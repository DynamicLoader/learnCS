#include "DBWrapper.hpp"
#include "inc/intkey.hpp"
#include <climits>
#include <cstddef>
#include <ostream>
#include <stdexcept>

extern "C"
{
#include "misc.h"
#include "LinkedList.h"
#include "degeneracy_algorithm_cliques_A.h"
}

const Entry DBWrapper::emptyEntry = Entry();
const KeyList DBWrapper::emptyKeyList = KeyList();

double DBWrapper::calcMaxKClique(int max_k)
{
    std::cout << "calcMaxKClique" << std::endl;
    size_t n;
    std::vector<LinkedList *> ll;
    auto readin = [&]() {
        if (coauthorTable.size() > INT_MAX)
            throw std::runtime_error("Too many authors.  Please reduce the number of authors.");

        for (size_t i = 0; i < coauthorTable.size(); i++)
        {
            ll.push_back(createLinkedList());
        }
        n = ll.size();
        assert(n == coauthorTable.size());
        std::cout << "n = " << n << std::endl;

        std::map<intKey_t, int> authorIDMap;
        int idx = 0;
        coauthorTable.preloadToCache();
        for (auto it = coauthorTable.cbegin(); it != coauthorTable.cend(); it++)
        {
            int cidx = authorIDMap.insert(std::make_pair(it->first, idx))
                           .first->second; // If not inserted, returned the existing index.
            if (cidx == idx)
                idx++;
            // std::cout << "cidx = " << cidx << std::endl;
            for (auto x : it->second)
            {
                int didx = authorIDMap.insert(std::make_pair(x, idx))
                               .first->second; // If not inserted, returned the existing index.
                if (didx == idx)
                    idx++;
                assert(cidx < n && cidx > -1);
                assert(didx < n && didx > -1);
                assert(cidx != didx);
                addLast(ll[cidx], didx);
                // addLast(ll[didx], cidx); we don't need this since the coauthorship is bidirectional, and we will add
                // it later.
            }
        }
    };

    auto run = [&]() {
        std::cout << "Counting max cliques..." << std::endl;
        clock_t start = clock();
        double totalCliques = 0;
        int deg = 0, m = 0;

        NeighborListArray **orderingArray = computeDegeneracyOrderArray(ll.data(), n);
        // printf("Before for. After computeDegeneracy.\n");
        for (int i = 0; i < n; i++)
        {
            if (deg < orderingArray[i]->laterDegree)
                deg = orderingArray[i]->laterDegree;
            m += orderingArray[i]->laterDegree;
        }

        if (max_k == 0) // this will take a long time
            max_k = deg + 1;

        // A only
        double nCalls, sumP, sqP;

        double *cliqueCounts = (double *)Calloc((max_k) + 1, sizeof(double));
        listAllCliquesDegeneracy_A(cliqueCounts, orderingArray, n, max_k, &nCalls, &sumP, &sqP);
        clock_t end = clock();

        printf("time,nCalls,sumP,sqP,max_k,degen\n");
        printf("%lf,%lf,%lf,%lf,%d,%d\n\n", (double)(end - start) / (double)(CLOCKS_PER_SEC), nCalls, sumP, sqP, 0,
               deg); // max_k_in = 0

        for (int i = 0; i <= max_k; i++)
        {
            if (cliqueCounts[i] != 0)
            {
                printf("%d, %lf\n", i, cliqueCounts[i]);
                cliques.push_back(std::make_pair(i, cliqueCounts[i]));
                totalCliques += cliqueCounts[i];
            }
        }

        printf("\n%lf total cliques\n", totalCliques);

        Free(cliqueCounts);
        Free(orderingArray);

        return totalCliques;
    };

    auto destroy = [&]() {
        for (auto x : ll)
        {
            destroyLinkedList(x);
        }
    };

    cliques.clear();
    readin();
    auto ret = run();
    destroy();

    return ret;
}


