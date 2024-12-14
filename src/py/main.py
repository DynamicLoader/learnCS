import serviceWrapper
import sys
import argparse
import os
import flask
from timeUtil import QueryTimeHelper
import http.server
import socket
import threading
import urllib.request

parser = argparse.ArgumentParser()
parser.add_argument(
    "-i", "--input", help="Path to the input file for creating db", required=False
)
parser.add_argument(
    "-p",
    "--preload",
    help='Preload "FullTable" into RAM. Recommended on machine with >= 16G RAM avail.',
    required=False,
    action="store_true",
)
parser.add_argument(
    "-g",
    "--gen",
    help="Generate additional stats, including max cliques and yearly keywords.",
    required=False,
    action="store_true",
)
parser.add_argument(
    "--standalone",
    help="Frontend and backend are running on the same machine. DO NOT USE THIS IN PRODUCTION!",
    required=False,
    action="store_true",
)

args = parser.parse_args(sys.argv[1:])


s = serviceWrapper.ServiceWrapper()

url_prefix = ""
app = flask.Flask(__name__)


time_helper = QueryTimeHelper()


def unpackEntry(e):
    return {"attrs": e.attributes, "props": e.properties}


def entrySummary(entry):
    """
    提取条目的简要信息，用于搜索页展示等
    """
    return {
        "title": entry.getProps("title"),
        "author": entry.getProps("author"),
        "year": entry.getProps("year"),
    }


@app.route(url_prefix + "/query", methods=["POST"])
def query():
    data = flask.request.get_json()
    body = dict()

    queryContent = data["query"]

    print(f'Query "{queryContent}" started')
    time_helper.start()

    body["keyword"] = []
    authorQueryResult = s.queryByAuthor(queryContent)
    print(f"Author query costs {time_helper.time() * 1000} ms.")

    if len(authorQueryResult) > 0:
        authorInfo = dict()

        authorInfo["name"] = queryContent
        authorInfo["workcount"] = len(authorQueryResult)
        authorInfo["work"] = s.keyList2HexStringList(authorQueryResult)

        body["author"] = authorInfo

        return flask.jsonify(body)

    body["keyword"] = s.keyList2HexStringList(
        s.queryByKeywordRaw(queryContent))
    print(f"Keyword query costs {time_helper.time() * 1000} ms.")

    fulltitleQuery = s.queryByFullTitleRaw(queryContent)
    if len(fulltitleQuery) <= 10:
        body["fullmatch"] = s.keyList2HexStringList(fulltitleQuery)
    else:
        body["fullmatch"] = []

    return flask.jsonify(body)


@app.route(url_prefix + "/hexkeylist2entry", methods=["POST"])
def hexkeylist2entry():
    data = flask.request.get_json()
    if "keylist" not in data:
        return flask.jsonify({})
    keylist = data["keylist"]

    body = {}
    body["result"] = []

    result = s.hexString2result(keylist)
    for index, e in enumerate(result):
        __entry = entrySummary(e)
        __entry["pkey"] = keylist[index]
        body["result"].append(__entry)

    return flask.jsonify(body)


@app.route(url_prefix + "/fullentry", methods=["POST"])
def fullentry():
    data = flask.request.get_json()
    q = data["query"]
    res = s.hexString2result([q])[0]

    return flask.jsonify([unpackEntry(res)])


@app.route(url_prefix + "/feature", methods=["POST"])
def feature():
    # data = flask.request.get_json()
    # q = data["query"]
    res = dict()
    topauthors = s.queryTopAuthor()
    topkeywords = s.queryTopKeyword()
    topyearkeywords = s.queryTopYearKeyword()
    MaxCli = s.queryMaxCli()
    res["topauthors"] = topauthors
    res["topkeywords"] = topkeywords
    res["MaxCli"] = MaxCli
    res["topyearkeywords"] = topyearkeywords
    return flask.jsonify(res)


@app.route(url_prefix + "/colab", methods=["POST"])
def colab():
    data = flask.request.get_json()
    a = data["query"]

    # !!! IMPORTANT 不要调大参数depth 不然会导致查询非常慢 !!!
    verticies, edges = s.queryCoauthorRecurse(a, 2)

    return flask.jsonify({"verticies": verticies, "edges": edges})


def get_resource_path(relative_path):
    try:
        # PyInstaller creates a temp folder and stores path in _MEIPASS
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")

    return os.path.join(base_path, relative_path)


class ProxyHandler(http.server.SimpleHTTPRequestHandler):
    def proxy(self, method):
        try:
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length)

            # 转发请求到另一个服务
            upstream_host = ('127.0.0.1', 5000)  # 假设上游服务器是本地的8001端口
            upstream_connection = http.client.HTTPConnection(*upstream_host)

            # 重新发送请求到上游服务器
            upstream_connection.request(
                method, self.path[4:], post_data, self.headers)
            response = upstream_connection.getresponse()

            # 发送响应到客户端
            self.send_response(response.status)
            for header in response.getheaders():
                self.send_header(*header)
            self.end_headers()
            self.wfile.write(response.read())

            upstream_connection.close()
        except:
            pass

    def do_GET(self):
        if self.path.startswith('/api'):
            # Forward the request to the Flask server
            self.proxy('GET')
        else:
            # Serve static files
            super().do_GET()

    def do_POST(self):
        if self.path.startswith('/api'):
            # Forward the request to the Flask server
            self.proxy('POST')
        else:
            self.send_error(http.HTTPStatus.METHOD_NOT_ALLOWED)

    def send_error(self, code: int, message: str | None = None, explain: str | None = None) -> None:
        if (code == 404):
            self.path = '/'
            self.do_GET()
        else:
            super().send_error(code, message, explain)

    def __init__(self, request, client_address, server):
        super().__init__(request, client_address, server,
                         directory=get_resource_path('src/web/dist'))


def main():
    if args.input is None:
        if args.preload:
            print("Preloading...")
            s.preload()
    else:
        if (os.path.isdir("db")) and len(os.listdir("db")) > 0:
            print("DB may already exist. Skipping DB creation.")
        else:
            s.setInput(args.input)
            s.parse(True if args.gen else False)
            print("DB created.")

    if args.gen:
        s.saveCli(10)  # this method calculates maximal cliques.
        s.saveTopYearKeyword()

    if args.standalone:
        print("You are using standalone mode. DO NOT USE THIS IN PRODUCTION!")
        httpd = http.server.ThreadingHTTPServer(('', 6767), ProxyHandler)
        server_thread = threading.Thread(target=httpd.serve_forever)
        server_thread.daemon = True
        server_thread.start()
        print("Frontend server started at http://localhost:6767")

    app.run(host="0.0.0.0")


if __name__ == "__main__":
    main()
