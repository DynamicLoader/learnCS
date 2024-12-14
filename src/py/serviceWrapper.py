import random
import sys
import xml.sax
import hashlib
from timeUtil import InsertTimeHelper

sys.path.append("./build")

from xmldb import Entry
from dbwrapper import DBWrapper

import dblpHandler


def computeMd5(s: str) -> bytes:
    return hashlib.md5(s.encode()).digest()


insertTimer = InsertTimeHelper()


class ServiceWrapper:
    def __init__(self) -> None:
        self.db = DBWrapper()
        self.xmlHandler = dblpHandler.DblpHandler(self.entryHandler, self.write)
        self.xmlInput = ""
        self.entryCount = 0
        self.parse_topyear = True

    def write(self) -> None:
        self.db.commit()

        print(f"Successfully parsed {self.entryCount} entries.")

    def setInput(self, xmlInput: str) -> None:
        self.xmlInput = xmlInput

    def entryHandler(self, entry: Entry) -> None:
        primaryKey = self.entryCount.to_bytes(16, "big")
        self.db.insertFullTable(primaryKey, entry)

        currentTitle = entry.getProps("title")
        if len(currentTitle) == 0:
            currentTitle = "Untitled"
        else:
            currentTitle = currentTitle[0]

        titleKey = computeMd5(currentTitle)
        self.db.pushTitleTable(titleKey, primaryKey)

        authors = entry.getProps("author")
        try:
            year=entry.getProps("year")
        except IndexError:
            year = [0]
        if not year:
            year = [0]
        coauthorlist = []
        for a in authors:
            authorKey = computeMd5(a)
            self.db.insertAuthorName(authorKey, a)
            self.db.pushAuthorTable(authorKey, primaryKey)
            coauthorlist.append(authorKey)

        # print(coauthorlist)

        authorlist = []
        for a in authors:
            authorKey = computeMd5(a)
            self.db.pushCoauthorTable(authorKey, coauthorlist)
            authorlist.append(authorKey)
        
        
       

        if(not self.parse_topyear):
            try:
                year_value = int(year[0])
            except IndexError:
                year_value = 0
        else:
            year_value = 0
        self.db.computeKeyword(currentTitle, primaryKey, year_value)

        self.entryCount += 1
        if self.entryCount % 1000 == 0:
            insertTimer()
            
    def parse(self, parseTopYear = True) -> None:
        self.parse_topyear = parseTopYear
        parser = xml.sax.make_parser()
        parser.setContentHandler(self.xmlHandler)
        parser.parse(self.xmlInput)

    def queryByFullTitle(self, title: str) -> list:
        _title = computeMd5(title)
        return self.db.queryByFullTitle(_title)

    def queryByFullTitleRaw(self, title: str) -> list:
        _title = computeMd5(title)
        return self.db.queryByFullTitleRaw(_title)

    def queryByAuthor(self, author: str):
        _author = computeMd5(author)
        return self.db.queryByAuthor(_author)

    def queryCoauthor(self, author: str):
        """
        Args:
            author (str): The name of author.

        Returns:
            list[str]: int128_t key of coauthors
        """
        _author = computeMd5(author)
        return self.db.queryCoauthor(_author)

    def queryCoauthorRecurseImpl(
        self, author_key: str, depth: int, maxdepth: int, vertices: list, edges: list
    ):
        if depth > maxdepth:
            return

        adj = self.db.queryCoauthor(author_key)
        if depth > 1 and len(adj) > 5:
            adj = random.sample(adj, 5)

        stopRecurse = False
        if depth == 1 and len(adj) > 20:
            stopRecurse = True

        for a in adj:
            author_hash_string = bytes(a.to_bytes())

            if a not in vertices:
                vertices.append(author_hash_string)

            if (author_key, author_hash_string) not in edges and (
                author_hash_string,
                author_key,
            ) not in edges:
                edges.append((author_key, author_hash_string))

            if not stopRecurse:
                self.queryCoauthorRecurseImpl(
                    author_hash_string, depth + 1, maxdepth, vertices, edges
                )

    def queryCoauthorRecurse(self, author: str, depth: int):
        _author = computeMd5(author)

        _v = [_author]
        _e = []
        self.queryCoauthorRecurseImpl(_author, 1, depth, _v, _e)

        _v_dict = {}

        verticies = []
        edges = []

        for index, a in enumerate(_v):
            _v_dict[a] = str(index + 1)
            verticies.append((str(index + 1), self.db.getRealNameStr(a)))

        for a, b in _e:
            edges.append((_v_dict[a], _v_dict[b]))
        return verticies, edges

    def queryByKeyword(self, sentence: str):
        return self.db.queryByKeyword(sentence)

    def queryByKeywordRaw(self, sentence: str):
        return self.db.queryByKeywordRaw(sentence)

    def queryTopAuthor(self):
        print(self.db.queryTopAuthor())
        return self.db.queryTopAuthor()

    def queryTopKeyword(self):
        print(self.db.queryTopKeyword())
        return self.db.queryTopKeyword()

    def queryTopYearKeyword(self):
        print(self.db.queryTopYearKeyword())
        return self.db.queryTopYearKeyword()
    
    def preload(self):
        self.db.preload()

    def test_using(self) -> None:
        self.db.test_using()

    def hexString2result(self, keylist) -> list:
        return self.db.HexString2Result(keylist)

    def keyList2HexStringList(self, keylist) -> list:
        return self.db.keyList2HexStringList(keylist)

    def saveTopYearKeyword(self):
        return self.db.saveTopYearKeyword()
        
    def saveCli(self, max_k):
        return self.db.saveCli(max_k)

    def queryMaxCli(self):
        print(self.db.queryMaxCli())
        return self.db.queryMaxCli()
