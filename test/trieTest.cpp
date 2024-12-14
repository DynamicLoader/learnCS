#include "TrieFA.hpp"
#include <iostream>
#include <string>
#include <vector>

std::string story{
    "In a charming village, there was an annual kite-flying festival. Lily, a spirited girl, longed to soar like the "
    "kites above. With her meticulously crafted kite, she joined the festivities. As her creation danced in the sky, "
    "Lily felt a f sense of f-reedom and connection, reminding her of life\'s simple joys"};

const std::vector<std::string> ignoreWords{
    "is", "are",  "was", "were", "be", "being", "been", "am",  "a",   "an", "the", "me",  "he",   "him",  "she", "her",
    "we", "they", "in",  "on",   "at", "of",    "with", "for", "and", "or", "to",  "all", "some", "very", "many"};

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

TrieFA trie;
KeywordFA k;

void testTrie()
{
    for (const auto c : punctuations)
        puncLut[static_cast<unsigned char>(c)] = true;

    for (auto word : ignoreWords)
        trie.addPattern(word);

    std::string buffer;

    for (auto c : story)
    {
        c = toLower(c);

        if (isPunctuation(c))
        {
            if (trie.isMatched())
                std::cout << "[" << buffer << "] ";
            else if (buffer.length())
                std::cout << buffer << " ";

            buffer = "";
            trie.reset();
        }
        else
        {
            trie.advance(c);
            buffer.push_back(c);
        }
    }
}

int main()
{
    testTrie();

    std::cout << "\n\n\n";

    auto res = k.compute(story);
    for (auto keyword : res)
        std::cout << keyword << ", ";

    return 0;
}