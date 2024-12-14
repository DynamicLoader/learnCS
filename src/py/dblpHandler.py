import xml
import sys

sys.path.append("build")

from xmldb import Entry

EntryType = {
    "article": 1,
    "inproceedings": 2,
    "proceedings": 3,
    "book": 4,
    "incollection": 5,
    "phdthesis": 6,
    "mastersthesis": 7,
    "www": 8,
    "person": 9,
    "data": 10,
    "__entry_type_max": 11,
}


class DblpHandler(xml.sax.ContentHandler):
    def _is_valid_tag(self, tag) -> bool:
        return tag in [
            "article",
            "inproceedings",
            "proceedings",
            "book",
            "incollection",
            "phdthesis",
            "mastersthesis",
            "www",
            "person",
            "data",
        ]

    def _is_valid_field(self, field) -> bool:
        return field in [
            "author",
            "editor",
            "title",
            "booktitle",
            "pages",
            "year",
            "address",
            "volume",
            "number",
            "month",
            "url",
            "ee",
            "cdrom",
            "cite",
            "publisher",
            "note",
            "crossref",
            "isbn",
            "series",
            "school",
            "chapter",
            "publnr",
            "stream",
            "rel",
        ]

    def _is_valid_attr(self, attr) -> bool:
        return attr in [
            "key",
            "mdate",
            "publtype",
            "reviewid",
            "rating",
            "cdate",
            "aux",
            "bibtex",
            "orcid",
            "label",
            "type",
            "href",
            "uri",
            "sort",
        ]

    def __init__(self, entryHandler, endCallback):
        self.entryHandler = entryHandler
        self.inEntry = False
        self.currentEntry = None
        self.charBuffer = ""
        self.tagStack = []
        self.cb = endCallback

    def startElement(self, name, attrs):
        if self._is_valid_tag(name):
            self.inEntry = True
            self.currentEntry = Entry()
            self.currentEntry.setEntryType(EntryType[name])

            for k, v in attrs.items():
                if self._is_valid_attr(k):
                    self.currentEntry.setAttr(k, v)
        elif self._is_valid_field(name):
            self.charBuffer = ""

        self.tagStack.append(name)

    def endElement(self, name):
        if self._is_valid_tag(name):
            self.entryHandler(self.currentEntry)
            self.currentEntry = None
        elif self._is_valid_field(name):
            self.currentEntry.pushProps(name, self.charBuffer)
        elif name == "dblp":
            self.cb()

        self.tagStack.pop()

    def characters(self, content):
        self.charBuffer += content
