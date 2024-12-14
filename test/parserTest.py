import sys
import xml.sax

sys.path.append("src/py/")

import dblpHandler
import xml


cnt = 0


def dummyHandler(entry):
    global cnt
    cnt += 1

    if cnt % 10000 == 0:
        print(f"Parsed {cnt} entries.")


def cb():
    print("Done!")


h = dblpHandler.DblpHandler(dummyHandler, cb)

parser = xml.sax.make_parser()
parser.setContentHandler(h)
parser.parse("./xml/dblp.xml")

# breaks at
#
#
# ...
# Chun-Yi Liu 0002 </author>
