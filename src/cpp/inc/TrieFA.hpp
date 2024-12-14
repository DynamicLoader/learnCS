#ifndef __TRIE_FA_HPP
#define __TRIE_FA_HPP

#include <string>
#include <vector>
#include <array>
#include <iostream>

struct Node
{
    Node *next[256];
    bool isEnd;
    Node()
    {
        for (int i = 0; i < 256; i++)
            next[i] = nullptr;
        isEnd = false;
    }
};

class TrieFA
{
  private:
    Node *root;
    Node *blackhole;

    Node *current;

  public:
    TrieFA()
    {
        root = new Node();
        blackhole = new Node();
        current = root;
    }

    void addPattern(const std::string &pattern)
    {
        auto p = root;

        for (const auto &c : pattern)
        {
            if (p->next[static_cast<unsigned char>(c)] == nullptr)
                p->next[static_cast<unsigned char>(c)] = new Node();
            p = p->next[static_cast<unsigned char>(c)];
        }
        p->isEnd = true;
    }

    void reset()
    {
        current = root;
    }

    // 状态机向前一步走
    void advance(const char c)
    {
        if (current == blackhole)
            return;

        if (current->next[static_cast<unsigned char>(c)] != nullptr)
        {
            current = current->next[static_cast<unsigned char>(c)];
        }
        else
            current = blackhole;
    }

    bool isMatched()
    {
        return current->isEnd;
    }
};

class KeywordFA
{
    using Result_t = std::vector<std::string>;

    TrieFA trie;

    const std::vector<std::string> ignoreWords{"is",  "are",  "was", "were", "be",   "being", "been", "am",
                                               "a",   "an",   "the", "me",   "he",   "him",   "she",  "her",
                                               "we",  "they", "in",  "on",   "at",   "of",    "with", "for",
                                               "and", "or",   "to",  "all",  "some", "very",  "many"};

    const std::vector<char> punctuations{',',  '.', ':', ';', '!', '?', '(', ')', '[',  ']',  '{',
                                         '}',  '&', '*', '@', '#', '$', '%', '^', '<',  '>',  '/',
                                         '\\', '|', '-', '_', '+', '=', '~', '`', '\'', '\"', ' '};

    std::array<bool, 256> puncLut{false};

    bool isPunctuation(const char c)
    {
        return puncLut[static_cast<unsigned char>(c)];
    }

    char toLower(const char c)
    {
        if (c >= 'A' && c <= 'Z')
            return c - 'A' + 'a';
        return c;
    }

  public:
    KeywordFA()
    {
        for (const auto &str : ignoreWords)
            trie.addPattern(str);
        for (const auto c : punctuations)
            puncLut[static_cast<unsigned char>(c)] = true;
    }

    Result_t compute(std::string title)
    {
        Result_t result{};

        trie.reset();
        std::string buffer;

        for (auto c : title)
        {
            if (isPunctuation(c))
            {
                if (buffer.size() > 1 && !trie.isMatched())
                    result.emplace_back(buffer);
                buffer.clear();
                trie.reset();
            }
            else
            {
                c = toLower(c);
                buffer.push_back(c);
                trie.advance(c);
            }
        }

        if (buffer.size() > 1 && !trie.isMatched())
            result.emplace_back(buffer);
        return result;
    }
};

#endif