#ifndef __STORAGE_HPP__
#define __STORAGE_HPP__

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <type_traits>
#include <utility>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <shared_mutex>
#include <mutex>
#include <thread>

#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/archives/portable_binary.hpp>

namespace Storage
{

constexpr uint8_t TIMPL_MAP = 1;
constexpr uint8_t TIMPL_MULTI_MAP = 2;
constexpr uint8_t TIMPL_UNORDERED_MAP = 3;
constexpr uint8_t TIMPL_UNORDERED_MULTI_MAP = 4;

// constexpr uint8_t MAX_LEN = 32;

namespace internal
{

inline std::string codeToString(const std::vector<uint8_t> &data)
{
    constexpr char CHAR_TABLE[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', //
                                     'a', 'b', 'c', 'd', 'e', 'f'};
    std::string encoded;
    for (auto a : data)
    {
        encoded += CHAR_TABLE[(a >> 4) & 0xf];
        encoded += CHAR_TABLE[a & 0xf];
    }

    return encoded;
}

template <uint8_t TIMPL> struct MapItemMeta
{
    char magic[3] = {'D', 'T', 'I'};
    uint8_t impl = TIMPL;

    size_t count;

    // Normally the block on the disk will not be less than 256 bytes, here we also reserved some bytes for used in
    // cereal
    uint8_t _reserved[240 - sizeof(magic) + sizeof(impl) + sizeof(count)];

    template <class Archive> void serialize(Archive &archive)
    {
        archive(magic);
        if (magic[0] != 'D' || magic[1] != 'T' || magic[2] != 'I')
        {
            throw std::runtime_error("Invalid magic");
        }
        archive(impl);
        if (impl != TIMPL)
        {
            throw std::runtime_error("Invalid impl");
        }
        archive(count);

        archive(_reserved);
    }
};

template <typename TKey, typename TVal, uint8_t TIMPL> struct MapItem : MapItemMeta<TIMPL>
{

    using TMAP = std::conditional_t<
        TIMPL == TIMPL_MAP, std::map<TKey, TVal>,
        std::conditional_t<TIMPL == TIMPL_MULTI_MAP, std::multimap<TKey, TVal>,
                           std::conditional_t<TIMPL == TIMPL_UNORDERED_MAP, std::unordered_map<TKey, TVal>,
                                              std::conditional_t<TIMPL == TIMPL_UNORDERED_MULTI_MAP,
                                                                 std::unordered_multimap<TKey, TVal>, void>>>>;
    std::shared_ptr<TMAP> pdata = std::make_shared<TMAP>();

    template <class Archive> void serialize(Archive &archive)
    {
        MapItemMeta<TIMPL>::serialize(archive);
        archive(pdata);
    }

    void updateMeta()
    {
        this->count = pdata->size();
    }
};

/**
 * @brief Map Implement
 *
 * @tparam TKey Type of Key, should implement serialization function used by cereal
 * @tparam TVal Type of Value, should implement serialization function used by cereal
 * @tparam TIMPL Internal Map implement ID
 * @tparam TlevelBits The count of bits used in level, must be multiple of 4
 */
template <typename TKey, typename TVal, uint8_t TIMPL, uint8_t TlevelBits = 8> class MapImpl
{
  private:
    static_assert((TlevelBits & 0x3) == 0, "TlevelBits must be multiple of 4");
    constexpr static uint8_t TlevelLen = TlevelBits >> 2;

    size_t _memory_usage = 0; // Current memory usage
    size_t _memory_usage_per_element = 0;
    size_t _memory_threshold =
        2048ULL * 1024 * 1024; // Memory threshold(default to 2048MB), if exceeded, perform GC
    std::shared_mutex _mutex;  // Add a shared mutex, allow multiple reads while only one write
    std::string _pathPrefix;
    uint8_t _maxLevel;
    std::function<std::vector<uint8_t>(const TKey &)> _KeySer;
    using MapType = MapItem<TKey, TVal, TIMPL>;

    struct cache_item_t
    {
        MapType item;
        // std::shared_ptr<std::fstream> pofs;
        bool modified = false;
    };

    using cache_t = std::map<std::string, cache_item_t>;
    cache_t _cache;

    std::set<std::string> _dbfile_list;
    size_t _total_records = 0;

    bool _insertOnlyMode = false;
    std::vector<MapImpl<TKey, TVal, TIMPL, TlevelBits> *> _insertOnlyMaps;

    auto _getpath(const std::string &id)
    {
        auto path = _pathPrefix;
        for (int i = 0; i < (int)_maxLevel - 1; i++)
        {
            path += id.substr(i * TlevelLen, TlevelLen) + "/";
        }
        auto filename = id.substr((_maxLevel - 1) * TlevelLen, TlevelLen) + ".gcd";
        path += filename;
        // std::cout << path << std::endl;
        return path;
    }

    // inline size_t _calculateIncrementSize(const TKey& key, const TVal& value) // rough estimation
    // {
    //     size_t incrementSize = 0;
    //     incrementSize += sizeof(key) + sizeof(value) + sizeof(std::shared_ptr<std::fstream>);
    //     return incrementSize;
    // }

    constexpr size_t _calculateIncrementSize()
    {
        return sizeof(TKey) + sizeof(TVal) + sizeof(std::shared_ptr<typename MapType::TMAP>);
    }

    size_t _calculateTotalElements()
    {
        size_t total_count = 0;
        for (auto &cache_pair : _cache)
        {
            total_count += cache_pair.second.item.pdata->size();
        }
        // std::cout<<"Total count is: "<<total_count<<std::endl; // debug
        return total_count;
    }

    size_t _garbageCollect()
    {
        size_t initialSize = _calculateTotalElements();
        // std::cout<<"The number of elements in cache is: "<<initialSize<<std::endl; // debug

        int n = _cache.size();
        // std::cout<<"The number of blocks in cache is: "<<n<<std::endl; // debug

        int num = n / 4;
        std::vector<decltype(_cache.begin())> unmodified;

        // Find all unmodified elements
        for (auto it = _cache.begin(); it != _cache.end(); ++it)
        {
            if (!it->second.modified)
            {
                unmodified.push_back(it);
            }
        }

        // Store and remove unmodified elements
        std::vector<decltype(_cache.begin())> deathNote;
        for (int i = 0; i < num && !unmodified.empty(); ++i)
        {
            auto it = unmodified.back();
            unmodified.pop_back();

            if (!_store(it->first))
            {
                std::cout << "Error saving " << it->first << std::endl;
                return 0;
            }

            deathNote.push_back(it);
        }

        // If not enough unmodified elements, remove elements from the beginning
        for (auto it = _cache.begin(); it != _cache.end() && num > 0; ++it, --num)
        {
            if (!_store(it->first))
            {
                std::cout << "Error saving " << it->first << std::endl;
                return 0;
            }
            it->second.modified = false;
            deathNote.push_back(it);
            if (--num == 0)
            {
                break;
            }
        }

        for (auto it : deathNote)
        {
            _cache.erase(it);
        }

        size_t finalSize = _calculateTotalElements();
        size_t numReleased = initialSize - finalSize;

        return numReleased * _memory_usage_per_element; // Return the amount of released memory(bytes)
    }

    // When load_cache is false, the returned cache_item_t should be deleted manually!!!
    auto &_load(const std::string &id, bool load_cache = true)
    {
        auto path = _getpath(id);
        if (load_cache)
        {
            auto x = _cache.find(path);
            if (x != _cache.end())
            {
                return x->second;
            }

            _memory_usage = _calculateTotalElements() * _memory_usage_per_element;
            // std::cout<<"Memory usage now is: "<<_memory_usage / (1024 * 1024) << "MB."<<std::endl; // debug
            if (_memory_usage > _memory_threshold)
            {
                std::cout << "Memory usage exceeds threshold(about " << _memory_threshold / (1024 * 1024) << "MB data)."
                          << std::endl; // debug

                if (size_t TEMP = _garbageCollect())
                {
                    std::cout << "Memory usage has reduced about " << TEMP / (1024 * 1024) << "MB data)."
                              << std::endl;                                                // debug
                    _memory_usage = _calculateTotalElements() * _memory_usage_per_element; // update memory usage
                    std::cout << "Memory usage now is: " << _memory_usage / (1024 * 1024) << "MB."
                              << std::endl; // debug
                }
                else
                {
                    throw std::runtime_error("Error while performing GC.");
                }
            }
        }

        cache_item_t c;

        std::ifstream ofs(path, std::ios::binary);

        if (ofs.is_open()) // exists, reading back into cache
        {

            try
            {
                cereal::PortableBinaryInputArchive archive(ofs);
                archive(c.item);
            }
            catch (const std::runtime_error &e)
            {
                std::cerr << "Error loading " << path << ": " << e.what() << '\n';
                ofs.close();
                if (load_cache)
                {
                    auto it = _cache.insert(std::make_pair(path, c));
                    _dbfile_list.insert(id.substr(0, _maxLevel * TlevelLen)); // update dbfile list; if exists, no change should be made accoring to the standard
                    return it.first->second;
                }
                else
                {
                    return *(new cache_item_t(c));
                }
            }
            ofs.close();
            if (load_cache)
            {
                auto it = _cache.insert(std::make_pair(path, c));
                _dbfile_list.insert(id.substr(0, _maxLevel * TlevelLen));
                return it.first->second;
            }
            else
            {
                return *(new cache_item_t(c));
            }
        }
        else
        {
            if (load_cache)
            {
                auto it = _cache.insert(std::make_pair(path, c));
                _dbfile_list.insert(id.substr(0, _maxLevel * TlevelLen));
                return it.first->second;
            }
            else
            {
                return *(new cache_item_t(c));
            }
        }
    }

    bool _store(const std::string &path, cache_item_t &c)
    {
        std::ofstream ofs(path, std::ios::binary);
        if (!ofs.is_open())
        { // first try failed, may be the directory does not exist
            std::filesystem::create_directories(std::filesystem::path(path).parent_path());
            ofs.open(path, std::ios::binary);
            if (!ofs.is_open()) // second try failed
                return false;
        }
        c.item.updateMeta(); // Update meta before saving

        try
        {
            cereal::PortableBinaryOutputArchive archive(ofs);
            archive(c.item);
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "Error saving " << path << ": " << e.what() << '\n';
            ofs.close();
            return false;
        }
        ofs.close();
        c.modified = false;

        return true;
    }

    bool _store(const std::string &path)
    {
        // std::cout << "Saving " << path << "\n";
        auto x = _cache.find(path);
        if (x == _cache.end())
        {
            return false;
        }
        auto &c = x->second;

        return _store(path, c);
    }

    std::string _key2id(const TKey &key)
    {

        auto k = codeToString(_KeySer(key));
        if (k.size() < TlevelLen * _maxLevel)
            k = std::string(TlevelLen * _maxLevel - k.size(), '0') + k;
        return k;
    }

    bool _isValidId(const std::string &id)
    {
        if (id.size() != TlevelLen)
            return false;
        for (auto x : id)
        {
            if (x < '0' || x > 'f')
                return false;
        }
        return true;
    }

    void _initMeta()
    {
        auto path = _pathPrefix;
        size_t vf = 0;
        if (std::filesystem::is_directory(path) == false)
        {
            return;
        }
        for (auto const &dir_entry : std::filesystem::recursive_directory_iterator(path))
        {

            if (dir_entry.is_regular_file())
            {
                if (dir_entry.path().extension() != ".gcd")
                    continue;
                // std::cout << dir_entry << '\n';
                auto par = dir_entry.path().lexically_relative(path).parent_path();
                // std::cout << "Got file in dir:" << par << std::endl;
                int l = 0;
                std::string id;
                for (auto x : par)
                {
                    // std::cout << x << std::endl;
                    if (!_isValidId(x.string()))
                        break;
                    l++;
                    id += x.string();
                }
                if (l != _maxLevel - 1)
                {
                    continue;
                }
                if (!_isValidId(dir_entry.path().stem().string()))
                    continue;
                id += dir_entry.path().stem().string();
                std::cout << "Got file:" << dir_entry.path() << std::endl;
                std::ifstream ifs(dir_entry.path(), std::ios::binary);
                if (!ifs.is_open())
                    throw std::runtime_error("Failed to open file: " + dir_entry.path().string());

                try
                {
                    cereal::PortableBinaryInputArchive archive(ifs);
                    MapItemMeta<TIMPL> meta;
                    archive(meta);
                    _total_records += meta.count;
                    ifs.close();
                    vf++;
                    // we add the file into db file list
                    _dbfile_list.insert(id);
                }
                catch (const std::runtime_error &e)
                {
                    std::cerr << "Error loading " << dir_entry.path() << ": " << e.what() << '\n';
                    ifs.close();
                    continue;
                }
            }
        }
        std::cout << "Retrived " << _total_records << " records from " << vf << " files" << std::endl;
    }

  public:
    template <bool isConst>
    class iterator_impl : protected std::conditional_t<isConst, typename MapType::TMAP::const_iterator,
                                                       typename MapType::TMAP::iterator>
    {
      private:
        std::shared_ptr<typename MapType::TMAP> _thisMap;
        // size_t _idx;
        using dbf_idx_t = decltype(_dbfile_list.begin());
        dbf_idx_t _idx;
        MapImpl<TKey, TVal, TIMPL, TlevelBits>
            *_impl; // Raw pointer is used since shared_ptr will require full decl of MapImpl for template construction
        using _base_type =
            std::conditional_t<isConst, typename MapType::TMAP::const_iterator, typename MapType::TMAP::iterator>;

        bool _lazy_load = false;
        TKey _key;

      protected:
        iterator_impl(MapImpl<TKey, TVal, TIMPL, TlevelBits> *impl, std::shared_ptr<typename MapType::TMAP> thisMap,
                      dbf_idx_t idx, _base_type it = {})
            : _impl(impl), _thisMap(thisMap), _idx(idx)
        {
            if (thisMap != nullptr)
            {
                _base_type::operator=(it == _base_type{} ? thisMap->begin() : it);
            }
        }

        iterator_impl(MapImpl<TKey, TVal, TIMPL, TlevelBits> *impl, std::shared_ptr<typename MapType::TMAP> thisMap,
                      const TKey &dbf, _base_type it = {})
            : _impl(impl), _thisMap(thisMap), _key(dbf), _lazy_load(true)
        {
            if (thisMap != nullptr)
            {
                _base_type::operator=(it == _base_type{} ? thisMap->begin() : it);
            }
        }

      public:
        // Override ++x
        iterator_impl &operator++()
        {
            if (_thisMap == nullptr) // already last block
            {
                std::cerr << "Attempt to increment an end iterator\n";
                return *this;
            }

            if (_lazy_load)
            {
                _idx = _impl->_dbfile_list.find(_impl->_key2id(_key).substr(0, _impl->_maxLevel * TlevelLen));
                _lazy_load = false;
            }

            _base_type::operator++();
            while (static_cast<_base_type>(*this) ==
                   _thisMap->end()) // Continous increment until we find a valid iterator, or end of entire map
            {
                _idx++;
                // Go into next map
                if (_idx == _impl->_dbfile_list.end())
                {
                    _thisMap = nullptr;
                    return *this; // No more, just hang here
                }
                auto &c = _impl->_load(*_idx);
                if constexpr (!isConst) // not a const iterator, mark as modified
                {
                    c.modified = true;
                }
                _thisMap = c.item.pdata;
                _base_type::operator=(_thisMap->begin());
            }
            return *this;
        }

        // Override x++
        iterator_impl operator++(int)
        {
            auto orig = *this;
            this->operator++();
            return orig;
        }

        // Override ==
        bool operator==(const iterator_impl &rhs) const
        {
            return (this->_thisMap == nullptr && rhs._thisMap == nullptr) ||
                   this->_thisMap == rhs._thisMap && static_cast<_base_type>(*this) == static_cast<_base_type>(rhs);
        }

        // Override !=, used in range-for
        bool operator!=(const iterator_impl &rhs) const
        {
            return !(*this == rhs);
        }

        // Export access functions
        using _base_type::operator*;
        using _base_type::operator->;

        friend class MapImpl<TKey, TVal, TIMPL, TlevelBits>;
    };

    using iterator = iterator_impl<false>;
    using const_iterator = iterator_impl<true>;
    friend iterator;
    friend const_iterator;

    iterator begin()
    {
        auto x = _dbfile_list.begin();
        return empty() ? end()
                       : iterator(this, _dbfile_list.empty() ? nullptr : ({
                                      auto &c = _load(*x);
                                      c.modified = true;
                                      c.item.pdata;
                                  }),
                                  x);
    }

    iterator end()
    {
        return iterator(this, nullptr, _dbfile_list.end());
    }

    const_iterator cbegin()
    {
        auto x = _dbfile_list.begin();
        return empty() ? cend() : const_iterator(this, _dbfile_list.empty() ? nullptr : _load(*x).item.pdata, x);
    }

    const_iterator cend()
    {
        return const_iterator(this, nullptr, _dbfile_list.end());
    }

    /**
     * @brief Construct a new Map Impl object
     *
     * @param pathPrefix Path prefix of the storage, must be ended with '/'
     * @param maxLevel At most maxLevel -1 levels of directories will be created
     * @param KeySer A function that serialize the key to a vector of uint8_t, used in the file name after converting
     * to hex string
     * @param memory_threshold_in_mb The memory threshold in MB. When the memory usage exceeds this threshold, 
     * garbage collection will be performed. Default is 2048 MB.
     */
    MapImpl(
        const std::string &pathPrefix, uint8_t maxLevel,
        std::function<std::vector<uint8_t>(const TKey &)> KeySer =
            [](const TKey &key) { return std::vector<uint8_t>((uint8_t *)&key, (uint8_t *)&key + sizeof(TKey)); },
        size_t memory_threshold_in_mb = 2048ULL)
        : _pathPrefix(pathPrefix), _maxLevel(maxLevel), _KeySer(KeySer), 
          _memory_threshold(memory_threshold_in_mb * 1024 * 1024) // Convert to bytes
    {
        if (maxLevel > 5)
        {
            throw std::runtime_error("maxLevel must be <= 5");
        }
        if (maxLevel < 1)
        {
            throw std::runtime_error("maxLevel must be >= 1");
        }
        _memory_usage_per_element = _calculateIncrementSize() << 1; // rough estimation
        // std::cout<<"Expected memory usage per element is: "<<_memory_usage_per_element<<std::endl; // debug
        // Initialize the meta data by enumerating all the files
        _initMeta();
    }

    ~MapImpl()
    {
        if (_insertOnlyMode)
        {
            insertOnlyMode(false);
            return;
        }

        commit(true); // silent commit on destruction
    }

    /**
     * @brief Insert only mode, used for huge amount of data insertion
     * @details While enabeling the mode, no data can be access from the map, and when commiting, a new sub-map will be
     created to be used. After disable the mode, all data will be merged into the main map.
     *
     * @param enable true  to enable, false to disable
     * @warning All iterator previously obtained will be invalid after enabling the mode!
     */
    void insertOnlyMode(bool enable)
    {
        if (enable)
        {
            if (!_insertOnlyMode)
            { // only do this when not in insert only mode
                commit(true);
                _cache.clear();
                _insertOnlyMaps.push_back(new MapImpl<TKey, TVal, TIMPL, TlevelBits>(
                    _pathPrefix + ".tmp" + std::to_string(_insertOnlyMaps.size()) + "/", _maxLevel, _KeySer));
                _insertOnlyMode = true;
                std::cout << "Insert only mode enabled" << std::endl;
            }
        }
        else
        {
            if (_insertOnlyMode)
            {
                std::cout << "Preparing to merge..." << std::endl;
                // commit all

                std::thread trd[8];
                for (int i = 0; i < 8; i++)
                {
                    auto t = _insertOnlyMaps.size() >> 3;
                    auto s = i * t;
                    auto e = i == 7 ? _insertOnlyMaps.size() : (i + 1) * t;
                    if (s < e)
                    {
                        trd[i] = std::thread([this, s, e]() {
                            for (int j = s; j < e; j++)
                            {
                                _insertOnlyMaps[j]->commit(true);
                                _insertOnlyMaps[j]->_cache.clear(); // must clear cache manually
                            }
                        });
                    }
                }
                for (int i = 0; i < 8; i++)
                    if (trd[i].joinable())
                        trd[i].join();

                for(auto x: _insertOnlyMaps){ // update dbfile list
                    _dbfile_list.merge(x->_dbfile_list);
                }
                std::cout << "Merging(" << _dbfile_list.size() << " files in total)";
                _total_records = 0;
                
                size_t working_total_recoreds[8] = {0};
                std::vector<std::string> dbf;
                for (auto id : _dbfile_list)
                    dbf.push_back(id);
                for (int i = 0; i < 8; i++)
                {
                    auto t = dbf.size() >> 3;
                    auto s = i * t;
                    auto e = i == 7 ? dbf.size() : (i + 1) * t;

                    if (s < e)
                    {
                        trd[i] = std::thread([this, &dbf, s, e, i, &working_total_recoreds]() {
                            for (int j = s; j < e; j++)
                            {
                                std::vector<cache_item_t *> cbs;
                                for (auto m : _insertOnlyMaps)
                                {
                                    auto &c = m->_load(dbf[j], false);
                                    if (c.item.pdata->size() > 0)
                                    {
                                        cbs.push_back(&c);
                                    }
                                    else
                                    {
                                        delete &c;
                                    }
                                }
                                auto &m = _load(dbf[j], false);
                                for (auto &c : cbs)
                                {
                                    m.item.pdata->merge(*(*c).item.pdata);
                                }
                                if (!_store(_getpath(dbf[j]), m))
                                { // just store it
                                    // throw std::runtime_error("Error saving " + id);
                                    std::cerr << "Error saving " << dbf[j] << std::endl;
                                }
                                else
                                {
                                    working_total_recoreds[i] += m.item.pdata->size();
                                }
                                for (auto x : cbs)
                                {
                                    delete x;
                                }
                                delete &m;
                            }
                        });
                    }
                }
                for (int i = 0; i < 8; i++) // wait for threads to finish
                    if (trd[i].joinable())
                        trd[i].join();

                for (auto x : working_total_recoreds)
                {
                    _total_records += x;
                }
                std::cout << "OK!" << std::endl;

                // delete all (tmp)
                std::cout << "Cleaning up...";
                for (auto m : _insertOnlyMaps)
                {
                    auto path = m->_pathPrefix;
                    delete m;
                    std::filesystem::remove_all(path);
                }
                _insertOnlyMaps.clear();
                _insertOnlyMode = false;
                std::cout << "OK!\nInsert only mode disabled" << std::endl;
            }
        }
    }

    bool insertOnlyMode() const
    {
        return _insertOnlyMode;
    }

    /**
     * @brief Preload all files into cache
     */
    void preloadToCache()
    {
        if (_insertOnlyMode)
            throw std::runtime_error("Cannot preload in insert only mode");

        auto path = _pathPrefix;
        size_t vf = 0;
        if (std::filesystem::is_directory(path) == false)
        {
            return;
        }
        for (auto const &dir_entry : std::filesystem::recursive_directory_iterator(path))
        {
            if (dir_entry.is_regular_file())
            {
                if (dir_entry.path().extension() != ".gcd")
                    continue;
                auto par = dir_entry.path().lexically_relative(path).parent_path();
                int l = 0;
                std::string id;
                for (auto x : par)
                {
                    if (!_isValidId(x.string()))
                        break;
                    l++;
                    id += x.string();
                }
                if (l != _maxLevel - 1)
                {
                    continue;
                }
                if (!_isValidId(dir_entry.path().stem().string()))
                    continue;
                id += dir_entry.path().stem().string();
                std::cout << "Got file:" << dir_entry.path() << std::endl;
                std::ifstream ifs(dir_entry.path(), std::ios::binary);
                if (!ifs.is_open())
                    throw std::runtime_error("Failed to open file: " + dir_entry.path().string());

                auto pathID = _getpath(id); // Get the path of the file

                try
                {
                    cereal::PortableBinaryInputArchive archive(ifs);
                    cache_item_t item;
                    archive(item.item);
                    _cache.insert(std::make_pair(pathID, item));

                    ifs.close();
                    vf++;
                    // we add the file into db file list
                    _dbfile_list.insert(id);
                }
                catch (const std::runtime_error &e)
                {
                    std::cerr << "Error loading " << dir_entry.path() << ": " << e.what() << '\n';
                    ifs.close();
                    continue;
                }
            }
        }
        std::cout << "Loaded " << _total_records << " records from " << vf << " files" << std::endl;
    }

    /**
     * @brief insert a key-value pair at random location
     *
     * @param key
     * @param val
     */
    std::pair<iterator, bool> insert(const TKey &key, const TVal &val)
    {
        std::unique_lock<std::shared_mutex> lock(_mutex); // Add a unique lock
        auto id = _key2id(key);
        if (_insertOnlyMode)
        {
            auto ret = _insertOnlyMaps.back()->insert(key, val).second;
            // _dbfile_list.insert(id.substr(0, _maxLevel * TlevelLen));
            return std::make_pair(iterator(this, nullptr, _dbfile_list.end()),
                                  ret); // no iterator should be returned; ret will not be accurate since it can be
                                        // overwrite from later insert. it can only be used to determind if the insert
                                        // in current map is successful
        }

        auto &c = _load(id);
        // auto idx = _dbfile_list.insert(id.substr(0, _maxLevel * TlevelLen)).first;
        auto ret = c.item.pdata->insert(std::make_pair(key, val));
        c.modified = true;

        // static std::string last_id;

        // if(last_id != *idx){
        //     std::cout << "Inserting  at " << *idx << "\n"; // debug
        //     last_id = *idx;
        // }

        _total_records += ret.second ? 1 : 0;
        return std::make_pair(iterator(this, c.item.pdata, key, ret.first), ret.second); // lazy load dbfile iterator
    }

    /**
     * @brief Erase an element with the given iterator
     *
     * @param it iterator to erase
     */
    void erase(const iterator &it)
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        if (_insertOnlyMode)
            throw std::runtime_error("Cannot erase in insert only mode");

        auto sz = it._thisMap->size();
        it._thisMap->erase(it);
        _load(*(it._idx)).modified = true;
        if (sz - it._thisMap->size() != 1)
        {
            throw std::runtime_error("Internal error: erase failed -> erased more than one element of single iterator");
        }
        _total_records -= 1;
    }

    /**
     * @brief Erase all elements with the given key
     *
     * @param key key to erase
     */
    void erase(const TKey &key)
    {
        std::unique_lock<std::shared_mutex> lock(_mutex); // Add a unique lock
        if (_insertOnlyMode)
            throw std::runtime_error("Cannot erase in insert only mode");
        auto &c = _load(_key2id(key));
        auto sz = c.item.pdata->size();
        c.pdata->erase(key);
        c.modified = true;
        _total_records -= sz - c.item.pdata->size();
    }

    /**
     * @brief Find any element with the given key and mark as MODIFIED
     *
     * @param key you know...
     * @return iterator The iterator of the element found, or end() if not found
     */
    iterator find(const TKey &key)
    {
        if (_insertOnlyMode)
            throw std::runtime_error("Cannot find in insert only mode");
        auto &c = _load(_key2id(key));
        auto x = c.item.pdata->find(key);
        if (x != c.item.pdata->end())
        {
            c.modified = true;
            // auto dbf = _dbfile_list.find(_key2id(key).substr(0, _maxLevel * TlevelLen));
            // if (dbf == _dbfile_list.end())
            // {
            //     throw std::runtime_error("Internal error: db file not found");
            // }
            return iterator(this, c.item.pdata, key, x); // lazy load dbfile iterator
        }
        return end();
    }

    /**
     * @brief Find any element with the given key and mark as MODIFIED
     *
     * @param key you know...
     * @return a const iterator The iterator of the element found, or end() if not found
     */
    const_iterator cfind(const TKey &key)
    {
        if (_insertOnlyMode)
            throw std::runtime_error("Cannot find in insert only mode");
        auto c = _load(_key2id(key));
        auto x = c.item.pdata->find(key);
        if (x != c.item.pdata->end())
        {
            // auto dbf = _dbfile_list.find(_key2id(key).substr(0, _maxLevel * TlevelLen));
            // if (dbf == _dbfile_list.end())
            // {
            //     throw std::runtime_error("Internal error: db file not found");
            // }
            return const_iterator(this, c.item.pdata, key, x);
        }
        return cend();
    }

    /**
     * @brief Find any element with the given key and mark as MODIFIED
     *
     * @param key key to find
     * @return auto A shared_ptr<Map> of Map in memory that contains the key. If not found, an empty map is returned
     * @note The iterator cannot be used to index the next element of another key,
     *       since the elements can be stored in other files
     */
    auto findMap(const TKey &key)
    {
        std::shared_lock<std::shared_mutex> lock(_mutex); // Add shared lock
        if (_insertOnlyMode)
            throw std::runtime_error("Cannot find in insert only mode");
        auto &c = _load(_key2id(key));
        auto x = c.item.pdata->count(key) <= 0 ? std::make_shared<typename MapType::TMAP>() : c.item.pdata;
        if (x->size())
        {
            c.modified = true;
        }
        return x;
    }

    /**
     * @brief Find any element with the given key without modifying the cache
     *
     * @param key key to find
     * @return const auto Map item found
     */
    auto findMapConst(const TKey &key)
    {
        std::shared_lock<std::shared_mutex> lock(_mutex); // Add shared lock
        if (_insertOnlyMode)
            throw std::runtime_error("Cannot find in insert only mode");
        auto c = _load(_key2id(key)).item.pdata;
        auto x = c->count(key) <= 0 ? std::make_shared<const typename MapType::TMAP>()
                                    : (std::shared_ptr<const typename MapType::TMAP>)c;
        return x;
    }

    bool empty() const
    {
        return _total_records == 0;
    }
    size_t size() const
    {
        return _total_records;
    }

    /**
     * @brief Commits the changes made to the storage.
     *
     * Saves the modified data to the storage. If the storage is in insert-only mode,
     * it commits the changes to the current insert-only map and creates a new one.
     * If the storage is not in insert-only mode, it saves the modified data to the storage file.
     *
     * @param silent Flag indicating whether to print status messages during the commit process. Default is false.
     * @return True if the commit is successful, false otherwise.
     */
    bool commit(bool silent = false)
    {
        std::unique_lock<std::shared_mutex> lock(_mutex); // protect that sub map is commiting in another thread
        size_t memory_before = _calculateTotalElements() * _memory_usage_per_element;

#define __SILENT_COMMIT(...)                                                                                           \
    if (!silent)                                                                                                       \
    {                                                                                                                  \
        __VA_ARGS__                                                                                                    \
    }

        if (_insertOnlyMode)
        {
            auto a = _insertOnlyMaps.back();
            std::thread trd([a]() { a->commit(true); });
            trd.detach();
            // renew one
            _insertOnlyMaps.push_back(new MapImpl<TKey, TVal, TIMPL, TlevelBits>(
                _pathPrefix + ".tmp" + std::to_string(_insertOnlyMaps.size()) + "/", _maxLevel, _KeySer));
            return true;
        }
        else
        {
            __SILENT_COMMIT(std::cout << "Saving data with prefix " << _pathPrefix << " ...";)
        }

        for (auto it = _cache.begin(); it != _cache.end(); ++it)
        {
            if (it->second.modified && !_store(it->first))
            {
                std::cerr << "Error saving " << it->first << std::endl;
                return false;
            }
            it->second.modified = false; // Reset flag
        }
        _cache.clear(); // Clear cache
        _memory_usage = _calculateTotalElements() * _memory_usage_per_element;
        __SILENT_COMMIT(std::cout << "OK! Released about " << (memory_before - _memory_usage) / (1024 * 1024)
                                  << "MB memory." << std::endl;)
        // std::cout<<"Memory usage now is: "<< _memory_usage / (1024 * 1024) << "MB." <<std::endl; // debug
        return true;
#undef __SILENT_COMMIT
    }
};

} // namespace internal

/// @brief A on disk map using std::map as internal implement
template <typename TKey, typename TVal, uint8_t TlevelBits = 8>
using MultiMap = internal::MapImpl<TKey, TVal, TIMPL_MULTI_MAP, TlevelBits>;

/// @brief A on disk map using std::map as internal implement
template <typename TKey, typename TVal, uint8_t TlevelBits = 8>
using Map = internal::MapImpl<TKey, TVal, TIMPL_MAP, TlevelBits>;

/// @brief A on disk map using std::unordered_map as internal implement
template <typename TKey, typename TVal, uint8_t TlevelBits = 8>
using UnorderedMap = internal::MapImpl<TKey, TVal, TIMPL_UNORDERED_MAP, TlevelBits>;

/// @brief A on disk map using std::unordered_multimap as internal implement
template <typename TKey, typename TVal, uint8_t TlevelBits = 8>
using UnorderedMultiMap = internal::MapImpl<TKey, TVal, TIMPL_UNORDERED_MULTI_MAP, TlevelBits>;

} // namespace Storage

#endif