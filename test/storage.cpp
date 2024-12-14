#include "../src/cpp/inc/Storage.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <unistd.h>

#define PRELOAD 0

int volatile t = 0;

int main()
{
    using namespace Storage;
    std::filesystem::remove_all("db"); // remove all files in db so that we can create it every time we run the
    // program
    std::cout << "starting..." << std::endl;
    Map<int, int> umap("db/testdb/prefix/", 2, [](const int &i) {
        std::vector<uint8_t> v((uint8_t *)&i, (uint8_t *)&i + sizeof(i));
        std::reverse(v.begin(), v.end());
        return v;
    },2048ULL); // set memory threshold here

    umap.insertOnlyMode(true);

#if PRELOAD
    std::cout << "preloading to cache..." << std::endl;
    umap.preloadToCache();
    std::cout << "preloading completed." << std::endl;
#endif
    // for (auto x : umap)
    // {
    //     std::cout << x.first << " " << x.second << std::endl;
    // }
    std::cout << "start to insert" << std::endl;
    for (int i = 0; i < 1000 * 20000; i += 1)
    {
        auto x = umap.insert(i, i << 1);
        auto y = umap.insert(i, i << 2);
        if (i % 100000 == 0){
            umap.commit();
        }
        // std::cout << (x == y ? "YES" : "NO") << std::endl;
    }
    sleep(3);
    std::string s;
    std::getline(std::cin, s);
    // umap.commit();
    umap.insertOnlyMode(false);
    std::getline(std::cin, s);

    // umap.commit();
    std::cout << "Total size: " << umap.size() << std::endl;

    auto c = umap.cfind(123000);
    for (auto x = c; x != umap.cend(); ++x)
    {
        // std::cout << x->first << " " << x->second << std::endl;
        auto a = x->first;
        auto b = x->second;
        t = a + b;
        a = t + b;
        b = a + t;
    }

    // fm->at(1) = 233;
    // fm->erase(2);
    // int n;
    // std::cin >> n;
    std::cout << "one last commit..." << std::endl;
    // umap.commit(); // auto commit in destructor
    std::getline(std::cin, s);
    return 0;
}