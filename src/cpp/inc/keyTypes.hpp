#pragma once
#include "Storage.hpp"
#include "Entry.hpp"

#include "intkey.hpp"
#include <string>

using Key_t = intKey_t;
using StrKey_t = std::string;
using KeyList = std::vector<Key_t>;
using StrKeyList = std::vector<StrKey_t>;

using FullTable = Storage::UnorderedMap<Key_t, Entry, 12>;
using AuthorName = Storage::UnorderedMap<Key_t, std::string>;
using AuthorTable = Storage::UnorderedMap<Key_t, KeyList>;
using CoauthorTable = Storage::UnorderedMap<Key_t, KeyList>;
using KeywordTable = Storage::UnorderedMap<StrKey_t, KeyList>;
using TitleTable = Storage::UnorderedMap<Key_t, KeyList>;
using TopAuthortable = Storage::UnorderedMap<Key_t, KeyList>;

// for top100 author
// using topAuthorTable = Storage::Map<Key_t, int>;
using AuthorWork = std::pair<int, std::string>;
// using SetIterator = std::set<AuthorWork>::iterator;

// for top100 keyword
// using topKeywordTable = Storage::Map<std::string, int>;
using KeywordCount = std::pair<int, std::string>;
// keyword被提到的次数 keyword本身
// 等等上面的似乎没有用上其实））））

using Result = std::vector<Entry>;