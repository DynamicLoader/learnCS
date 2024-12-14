#ifndef __DB_WRAPPER_HPP__
#define __DB_WRAPPER_HPP__

#include "cereal/archives/portable_binary.hpp"
#include "cereal/details/helpers.hpp"
#include "intkey.hpp"
#include "keyTypes.hpp"
#include "TrieFA.hpp"
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/utility.hpp>
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <chrono>
#include <numeric>
#include <queue>
#define verbose 0

/*

需要实现的表

作者 -> 作者的所有论文 (包括长度)
作者 -> 所有合作者 (邻接表)

年份 关键词频率 (?)

*/

const int workerThread = 4;

template <typename T> auto intersectArray(const std::vector<T> &a, const std::vector<T> &b) -> std::vector<T>
{
    // We assume that the destination array is empty,
    // and the two source arrays are sorted.
    std::vector<T> dest{};
    dest.reserve(std::min(a.size(), b.size()));

    size_t i = 0;
    size_t j = 0;

    while (i < a.size() and j < b.size())
    {
        if (a[i] < b[j])
        {
            ++i;
        }
        else if (b[j] < a[i])
        {
            ++j;
        }
        else
        {
            dest.push_back(a.at(i));
            ++i;
            ++j;
        }
    }

    return dest;
}

class DBWrapper
{
  private:
    const int maxLevel = 1;
    const int topAuthorSize = 100;

    KeywordFA fa;

  public:
    FullTable fullTable = FullTable("db/fulltable/", maxLevel, Key_t::KeySerializer, 8192);
    AuthorName authorName = AuthorName("db/authorname/", maxLevel, Key_t::KeySerializer, 1024);
    AuthorTable authorTable = AuthorTable("db/author/", maxLevel, Key_t::KeySerializer, 1024);
    CoauthorTable coauthorTable = CoauthorTable("db/coauthor/", maxLevel, Key_t::KeySerializer, 1024);
    TitleTable titleTable = TitleTable("db/title/", maxLevel, Key_t::KeySerializer, 1024);
    KeywordTable keywordTable = KeywordTable(
        "db/keyword/", maxLevel,
        [](const StrKey_t &s) { return std::vector<unsigned char>{(unsigned char)s.front(), (unsigned char)s.back()}; },
        2048);

    const static Entry emptyEntry;
    const static KeyList emptyKeyList;
    std::vector<std::pair<int, double>> cliques;

    void insertFullTable(const StrKey_t &k, const Entry &v)
    {
        if (!fullTable.insertOnlyMode())
        {
            fullTable.insertOnlyMode(true);
        }
        static int cnt = 0;
        auto _k = Key_t(k);
        fullTable.insert(_k, v);
        cnt++;
        if ((cnt & ((1ULL << 19) - 1)) == 0)
        {
            fullTable.commit();
            cnt = 0;
        }
    }

    void insertAuthorName(const StrKey_t &k, const std::string &v)
    {
        auto _k = Key_t(k);
        authorName.insert(_k, v);
    }

    /**
     * @brief
     *
     * @param k 作者
     * @param v 文章
     */
    void pushAuthorTable(const StrKey_t &k, const StrKey_t &v)
    {
        auto _k = Key_t(k);
        auto _v = Key_t(v);
        auto x = authorTable.find(_k);

        if (x == authorTable.end())
        {
            auto ret = authorTable.insert(_k, KeyList({}));
            if (!ret.second)
                throw std::runtime_error("Error occured when inserting into authorTable.");

            x = ret.first;
        }

        x->second.push_back(_v);
    }

    bool isTopStore()
    {
        std::filesystem::path p("db");
        if (!std::filesystem::exists(p))
        {
            std::filesystem::create_directories(p);
        }

        p /= "feature";
        if (std::filesystem::exists(p))
        {
            return true;
        }
        if (!std::filesystem::exists(p))
        {
            std::filesystem::create_directories(p);
        }

        return std::filesystem::exists(p);
    }

    /**
     * @brief 更新作者合作关系邻接表
     *
     * @param k 主角作者
     * @param v 传入一个文章的作者列表。函数遍历列表，排除主角本身和重复项，插入到邻接表中。
     */
    void pushCoauthorTable(const StrKey_t &k, const StrKeyList &v)
    {
        auto _k = Key_t(k);
        auto x = coauthorTable.find(_k);

        if (x == coauthorTable.end())
        {
            auto ret = coauthorTable.insert(_k, KeyList({}));
            if (!ret.second)
                throw std::runtime_error("Error inserting coauthor table.");

            x = ret.first;
        }

        auto &adj = x->second;
        for (auto &author : v)
        {
            auto _author = Key_t(author);
            if (_author != _k)
                adj.push_back(author);
        }
    }

    /**
     * @brief 更新标题 -> 文章key映射表
     *
     * @param k
     * @param v
     */
    void pushTitleTable(const StrKey_t &k, const StrKey_t &v)
    {
        auto _k = Key_t(k);
        auto x = titleTable.find(_k);

        if (x == titleTable.end())
        {
            auto ret = titleTable.insert(_k, KeyList({}));
            if (!ret.second)
                throw std::runtime_error("Error pushing title table.");

            x = ret.first;
        }

        x->second.push_back(Key_t(v));
    }

    /**
     * @brief 建立keyword的表
     *
     * @param k 标题词汇（string）
     * @param v 文章
     */

    void pushKeywordTable(const StrKey_t &k, const StrKey_t &v)
    {
#if verbose
        std::cout << "[keyword] " << k << " <- " << v << std::endl;
#endif
        auto _v = Key_t(v);
        auto x = keywordTable.find(k);

        if (x == keywordTable.end())
        {
            auto ret = keywordTable.insert(k, KeyList({}));
            if (!ret.second)
                throw std::runtime_error("Error pushing keyword table.");

            x = ret.first;
        }

        x->second.push_back(Key_t(_v));
    }
    /**
     * @brief 更新top100keyword 类似于topauthor
     *
     * @param k keyword
     */
    const int topKeywordsize = 100;

    void computeTopKeyword()
    {
        std::map<int, std::string> keywordCount;
        for (auto it = keywordTable.cbegin(); it != keywordTable.cend(); it++)
        {
            auto num = it->second.size();
            if (keywordCount.size() < topKeywordsize)
            {
                keywordCount.insert(std::make_pair(num, it->first));
            }
            else if (num > keywordCount.begin()->first)
            {
                keywordCount.erase(keywordCount.begin()); // The first element is the least
                keywordCount.insert(std::make_pair(num, it->first));
            }
        }

        std::vector<std::string> result;
        for (auto it = keywordCount.rbegin(); it != keywordCount.rend(); it++)
        {
            result.push_back(it->second);
        }

        std::ofstream file("db/feature/topKeyword.gcd", std::ios::binary);
        if (!file.is_open())
        {
            std::filesystem::create_directories("db/feature");
            file.open("db/feature/topKeyword.gcd", std::ios::binary);
            if (!file)
                throw std::runtime_error("Error opening file.");
        }
        try
        {
            cereal::PortableBinaryOutputArchive oarchive(file);
            oarchive(result);
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "Error saving topKeyword: " << e.what() << std::endl;
        }
    }

    void computeTopAuthor()
    {
        std::map<int, Key_t> authorCount;

        for (auto it = authorTable.cbegin(); it != authorTable.cend(); it++)
        {
            auto num = it->second.size();
            if (authorCount.size() < topAuthorSize)
            {
                authorCount.insert(std::make_pair(num, it->first));
            }
            else if (num > authorCount.begin()->first)
            {
                authorCount.erase(authorCount.begin()); // The first element is the least
                authorCount.insert(std::make_pair(num, it->first));
            }
        }

        std::vector<std::string> result; // May change to pair (name, count) to provide more info.
        for (auto it = authorCount.rbegin(); it != authorCount.rend(); it++)
        {
            auto nit = authorName.cfind(it->second);
            if (nit == authorName.cend())
                throw std::runtime_error("Error: author name not found.");
            result.push_back(nit->second);
        }

        std::ofstream file("db/feature/topAuthor.gcd", std::ios::binary);
        if (!file.is_open())
        {
            std::filesystem::create_directories("db/feature");
            file.open("db/feature/topAuthor.gcd", std::ios::binary);
            if (!file)
                throw std::runtime_error("Error opening file.");
        }
        try
        {
            cereal::PortableBinaryOutputArchive oarchive(file);
            oarchive(result);
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "Error saving topAuthor: " << e.what() << std::endl;
        }
    }

    void computeKeyword(const std::string &title, const StrKey_t &k, const int year)
    {
        auto keywords = fa.compute(title);
        for (const auto &word : keywords)
        {
            pushKeywordTable(word, k);
            if(year != 0)
                pushYearKeyword(word, year);
        }
    }

    // map(int, map(string, int));
    std::map<int, std::map<std::string, int, std::greater<std::string>>> yearKeyword;

    void pushYearKeyword(std::string word, int year)
    {
        auto x = yearKeyword.find(year);
        if (x == yearKeyword.end())
        {
            // 如果year还没有对应的条目，创建一个并插入word
            yearKeyword[year][word] = 1; // 初始化计数为1
        }
        else
        {
            // 如果year已有条目，直接增加word的计数
            ++(x->second[word]); // 如果word已经存在，计数加1；否则会自动初始化为1
        }
    }

    void saveYearKeyword()
    {
        if (isyearKeyword())
        {
            std::ofstream file("db/year/yearkey.gcd", std::ios::binary);
            if (!file)
            {
                throw std::runtime_error("Error opening file.");
                return;
            }
            cereal::BinaryOutputArchive archive(file);
            // 或者 cereal::JSONOutputArchive 如果你想要JSON格式
            archive(yearKeyword);
            file.close();
            if (file.fail())
            {
                throw std::runtime_error("Error writing file.");
            }
        }
        else
        {
            std::cout << "Error: cannot open file" << std::endl;
        }
    }
    void loadYearKeyword()
    {
        std::ifstream file("db/year/yearkey.gcd", std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Error opening file.");
            return;
        }
        cereal::BinaryInputArchive archive(file);
        // 或者 cereal::JSONInputArchive 如果你的数据是JSON格式
        archive(yearKeyword); // 将 vec 替换为 yearKeyword
        file.close();
        if (file.fail())
        {
            throw std::runtime_error("Error reading file.");
        }
    }

    bool isyearKeyword()
    {
        std::filesystem::path p("db");
        if (!std::filesystem::exists(p))
        {
            std::filesystem::create_directories(p);
        }

        p /= "year";
        if (std::filesystem::exists(p))
        {
            return true;
        }
        if (!std::filesystem::exists(p))
        {
            std::filesystem::create_directories(p);
        }

        return std::filesystem::exists(p);
    }

    const int yearKeywordCountSize = 10;

    void saveTopYearKeyword()
    {
        loadYearKeyword();
        std::vector<std::string> res;
        int count = 1;
        for (const auto &yearEntry : yearKeyword)
        {
            int year = yearEntry.first;
            const auto &keywordCounts = yearEntry.second;

            // 使用优先队列找到每年计数最多的前N个关键词
            std::priority_queue<std::pair<int, std::string>, std::vector<std::pair<int, std::string>>,
                                std::greater<std::pair<int, std::string>>>
                pq;

            for (const auto &kwEntry : keywordCounts)
            {
                pq.push(std::make_pair(kwEntry.second, kwEntry.first)); // 推入计数和关键词，计数用于排序

                // 如果优先队列大小超过限制，弹出计数最少的一个
                if (pq.size() > yearKeywordCountSize)
                {
                    pq.pop();
                }
            }

            res.push_back(std::to_string(year));
            std::string combinedKeywords;
            while (!pq.empty())
            {
                combinedKeywords += pq.top().second; // 添加关键词到结果字符串
                pq.pop();

                // 如果不是最后一个关键词，添加分隔符
                if (!pq.empty())
                {
                    combinedKeywords += ", "; // 这里使用逗号和空格作为分隔符，可以根据需要修改
                }
            }
            res.push_back((combinedKeywords));
        }
        std::cout << "[cpp] Writing..." << std::endl;
        std::ofstream file("db/feature/topYearKeyword.gcd", std::ios::binary);
        if (!file.is_open())
        {
            std::filesystem::create_directories("db/feature");
            file.open("db/feature/topYearKeyword.gcd", std::ios::binary);
            if (!file)
                throw std::runtime_error("Error opening file.");
        }
        try
        {
            cereal::PortableBinaryOutputArchive oarchive(file);
            oarchive(res);
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "Error saving topYearKeyword: " << e.what() << std::endl;
        }

        std::cout << "[cpp] Yearly keyword success." << std::endl;
    }
    // void saveTopYearKeyword()
    // {
    // std::cout << "[cpp] Yearly keyword count started..." << std::endl;
    // std::map<std::string, std::map<int, int>> yearKeywordCount;
    //
    // // for (auto it = keywordTable.cbegin(); it != keywordTable.cend(); it++)
    // // {
    // // auto &keywordCount = yearKeywordCount[it->first];
    // // for (auto yit = it->second.begin(); yit != it->second.end(); yit++)
    // // {
    // // auto yearProp = fullTable.cfind(*yit)->second.getProps("year");
    // // if (yearProp.empty())
    // // continue;
    //
    // // int year = std::stoi(yearProp[0]); // assuming yearProp[0] is a string representing an integer
    // // keywordCount[year]++;
    // // }
    // // }
    //
    // // std::map<int, std::vector<std::pair<std::string, int>>> topYearKeywordCount;
    // // for (auto it = yearKeywordCount.begin(); it != yearKeywordCount.end(); it++)
    // // {
    // // for (auto yit = it->second.begin(); yit != it->second.end(); yit++)
    // // {
    // // auto &topKeywords = topYearKeywordCount[yit->first];
    // // if (topKeywords.size() < yearKeywordCountSize)
    // // {
    // // topKeywords.push_back({it->first, yit->second});
    // // }
    // // else
    // // {
    // // auto min = std::min_element(topKeywords.begin(), topKeywords.end(),
    // // [](const auto &a, const auto &b) { return a.second < b.second; });
    // // if (yit->second > min->second)
    // // {
    // // topKeywords.erase(min);
    // // topKeywords.push_back({it->first, yit->second});
    // // }
    // // }
    // // }
    // // }
    //
    // // std::vector<std::string> result;
    // // for (auto it = topYearKeywordCount.rbegin(); it != topYearKeywordCount.rend(); it++)
    // // {
    // // auto year = std::to_string(it->first);
    // // result.push_back(year);
    // // std::string combined =
    // // std::accumulate(std::next(it->second.begin()), it->second.end(),
    // // it->second.begin()->first, // start with first element's string part
    // // [](std::string a, const std::pair<std::string, int> &b) { return a + ',' + b.first; });
    // // result.push_back(combined);
    // // }
    //
    // std::cout << "[cpp] Writing..." << std::endl;
    // std::ofstream file("db/feature/topYearKeyword.gcd", std::ios::binary);
    // if (!file.is_open())
    // {
    // std::filesystem::create_directories("db/feature");
    // file.open("db/feature/topYearKeyword.gcd", std::ios::binary);
    // if (!file)
    // throw std::runtime_error("Error opening file.");
    // }
    // try
    // {
    // cereal::PortableBinaryOutputArchive oarchive(file);
    // oarchive(result);
    // }
    // catch (const std::runtime_error &e)
    // {
    // std::cerr << "Error saving topYearKeyword: " << e.what() << std::endl;
    // }
    //
    // std::cout << "[cpp] Yearly keyword success." << std::endl;
    // }

    std::vector<std::string> queryTopYearKeyword()
    {
        // std::vector<std::string> result;
        // for (auto it = topKeyword.begin(); it != topKeyword.end(); it++)
        // {
        // result.push_back(it->second);
        // }
        // return result;
        std::ifstream file("db/feature/topYearKeyword.gcd", std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Error opening file.");
        }
        cereal::PortableBinaryInputArchive iarchive(file);
        std::vector<std::string> vec;
        iarchive(vec);
        if (file.fail())
        {
            throw std::runtime_error("Error reading file.");
        }
        return vec;
    }

    void test_using()
    {
        printf("Hello, World!\n");
    }

    void commit()
    {
        std::cout << "Commit called" << std::endl;

        std::thread trd([this] {
            titleTable.commit();

            computeTopAuthor();
            authorName.commit();
            authorTable.commit();

            for (auto &it : keywordTable)
                if (not it.second.empty())
                    std::sort(it.second.begin(), it.second.end());
            computeTopKeyword();
            // std::cout<<"compute top keyword"<<std::endl;
            // std::cout<<"compute year keyword"<<std::endl;

            keywordTable.commit();
            saveYearKeyword();
            for (auto &it : coauthorTable)
                if (not it.second.empty())
                {
                    auto &adj = it.second;
                    std::sort(adj.begin(), adj.end());
                    adj.erase(std::unique(adj.begin(), adj.end()), adj.end());
                }
            // // Before coauthor commit, just calc the max clique
            // calcMaxKClique();

            coauthorTable.commit();
        });

        fullTable.insertOnlyMode(false); // this wiil commit automatically

        if (trd.joinable())
            trd.join();
    }

  private:
    Result keyList2Result(const KeyList &list)
    {
        auto start = std::chrono::high_resolution_clock::now();

        Result res;
        for (auto &key : list)
        {
            auto x = fullTable.cfind(key);
            if (x != fullTable.cend())
                res.emplace_back(x->second);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "Fulltable query time: " << duration / 1000.0 << " ms" << std::endl;

        return res;
    }

  public:
    std::vector<std::string> keyList2HexStringList(const KeyList &list)
    {
        std::vector<std::string> res;
        for (const auto &key : list)
            res.emplace_back(std::string(key));
        return res;
    }

    // 实现具体功能
    Result queryByFullTitle(const StrKey_t &title)
    {
        return keyList2Result(queryByFullTitleRaw(title));
    }

    KeyList queryByFullTitleRaw(const StrKey_t &title)
    {
        auto _k = Key_t(title);

        auto x = titleTable.cfind(_k);
        if (x == titleTable.cend())
            return {};

        return x->second;
    }

    KeyList queryByAuthor(const std::string &author)
    {
        auto _k = Key_t(author);

        auto x = authorTable.cfind(_k);
        if (x == authorTable.cend())
            return {};

        return x->second;
    }

    KeyList queryCoauthor(const std::string &author)
    // 似乎查找合作者不用将所有条目（即论文信息)写出来
    {
        auto _k = Key_t(author);
        auto x = coauthorTable.cfind(_k);

        if (x == coauthorTable.cend())
            return {};

        return x->second;
    }

    const KeyList &queryListByKeyword(const std::string &keyword)
    {
        auto x = keywordTable.cfind(keyword);

        if (x == keywordTable.cend())
            return emptyKeyList;
        return x->second;
    }

    KeyList queryByKeywordRaw(const std::string &sentence)
    {
        auto keywords = fa.compute(sentence);

        if (keywords.size() == 0)
            return KeyList();

        Result result;
        KeyList mergeList;

        auto it = keywords.begin();
        mergeList = queryListByKeyword(*it);

        for (it = keywords.begin() + 1; it != keywords.end(); it++)
        {
            auto dest = intersectArray(mergeList, queryListByKeyword(*it));
            mergeList = dest;

            if (mergeList.empty())
                break;
        }

        return mergeList;
    }

    Result HexString2Result(std::vector<std::string> list)
    {
        KeyList __list;
        for (auto &k : list)
            __list.emplace_back(Key_t::fromHexString(k));
        return keyList2Result(__list);
    }

    Result queryByKeyword(const std::string &sentence)
    {
        return keyList2Result(queryByKeywordRaw(sentence));
    }

    std::string getRealName(const Key_t &hashName)
    {
        auto x = authorName.cfind(hashName);
        if (x == authorName.cend())
            return "";
        return x->second;
    }

    std::string getRealNameStr(const StrKey_t &hashName)
    {
        return getRealName(Key_t(hashName));
    }

    std::vector<std::string> queryTopAuthor()
    {
        // std::vector<std::string> result;
        // for (auto it = topAuthor.begin(); it != topAuthor.end(); it++)
        // {
        //     result.push_back(it->second);
        // }
        // return result;
        std::ifstream file("db/feature/topAuthor.gcd", std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Error opening file.");
        }
        cereal::PortableBinaryInputArchive iarchive(file);

        std::vector<std::string> vec;
        iarchive(vec);
        file.close();
        if (file.fail())
        {
            throw std::runtime_error("Error reading file.");
        }
        return vec;
    }

    std::vector<std::string> queryTopKeyword()
    {
        // std::vector<std::string> result;
        // for (auto it = topKeyword.begin(); it != topKeyword.end(); it++)
        // {
        // result.push_back(it->second);
        // }
        // return result;
        std::ifstream file("db/feature/topKeyword.gcd", std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Error opening file.");
        }
        cereal::PortableBinaryInputArchive iarchive(file);
        std::vector<std::string> vec;
        iarchive(vec);
        if (file.fail())
        {
            throw std::runtime_error("Error reading file.");
        }
        return vec;
    }

    void preload()
    {
        keywordTable.preloadToCache();
        titleTable.preloadToCache();
    }

    double calcMaxKClique(int max_k); // To avoid cyclic dependency

    void saveCli(int k_max = 10)
    {
        std::vector<double> vec;
        auto cli = calcMaxKClique(k_max);
        for (const auto &element : cliques)
        {
            std::cout << element.first << " " << element.second << std::endl;
            vec.push_back(element.second); // 将set中的字符串添加到vector中
        }
        if (isTopStore())
        {
            std::ofstream file("db/feature/MaxCli.gcd", std::ios::binary);
            if (!file)
            {
                throw std::runtime_error("Error opening file.");
                return;
            }
            cereal::BinaryOutputArchive archive(file);
            // 或者 cereal::JSONOutputArchive 如果你想要JSON格式
            archive(vec);
            file.close();
            if (file.fail())
            {
                throw std::runtime_error("Error writing file.");
            }
        }
        else
        {
            std::cout << "Error: cannot open file" << std::endl;
        }
    }

    std::vector<double> queryMaxCli()
    {
        std::ifstream file("db/feature/MaxCli.gcd", std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Error opening file.");
        }
        cereal::BinaryInputArchive iarchive(file);
        std::vector<double> vec;
        iarchive(vec);
        file.close();
        if (file.fail())
        {
            throw std::runtime_error("Error reading file.");
        }
        return vec;
    }
};

#endif