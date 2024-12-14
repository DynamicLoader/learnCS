import sys

sys.path.append("build")
sys.path.append("src/py")

from xmldb import Entry
from dbwrapper import DBWrapper

import hashlib
import random
import string
import tqdm


def generate_random_string(length):
    letters = string.ascii_letters
    return "".join(random.choice(letters) for _ in range(length))


props_name = [
    "editor",
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

attrs_name = [
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


def computeMd5(s: str) -> bytes:
    return hashlib.md5(s.encode()).digest()


def genPrimaryKey(entry):
    result = computeMd5(entry.get_key() + entry.get_title()[0])
    return result


def entry_generater():
    current_entry = Entry()

    current_entry.setEntryType(1)
    # Generate random attributes
    num_attrs = random.randint(1, len(attrs_name))
    selected_attrs = random.sample(attrs_name, num_attrs)

    for attr in selected_attrs:
        current_entry.setAttr(attr, "itsmygoaaaaa")

    current_entry.pushProps("title", generate_random_string(random.randint(2, 100)))
    for _ in range(random.randint(1, 20)):
        current_entry.pushProps(
            "author", generate_random_string(random.randint(10, 50))
        )

    num_props = random.randint(3, 8)
    selected_props = random.sample(props_name, num_props)
    for p in selected_props:
        for _ in range(random.randint(1, 5)):
            current_entry.pushProps(p, "aaaaaaaaaa")

    return current_entry


class Test:
    def __init__(self) -> None:
        self.db = DBWrapper()
        self.entryCount = 0

    def entryHandler(self, entry: Entry) -> None:
        primaryKey = genPrimaryKey(entry)
        self.db.insertFullTable(primaryKey, entry)

        currentTitle = entry.get_title()[0]
        titleKey = computeMd5(currentTitle)
        self.db.pushTitleTable(titleKey, primaryKey)

        authors = entry.get_author()
        coauthorlist = []
        for a in authors:
            authorKey = computeMd5(a)
            self.db.insertAuthorName(authorKey, a)
            self.db.pushAuthorTable(authorKey, primaryKey)
            self.db.updateTopAuthor(authorKey)
            coauthorlist.append(authorKey)

        # print(coauthorlist)

        authorlist = []
        for a in authors:
            authorKey = computeMd5(a)
            self.db.pushCoauthorTable(authorKey, coauthorlist)
            authorlist.append(authorKey)

    def run(self, n=10**7):
        e = entry_generater()
        for _ in tqdm.tqdm(range(n)):
            self.entryHandler(e)


if __name__ == "__main__":
    test = Test()
    test.run()
