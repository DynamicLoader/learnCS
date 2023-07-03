/**
 * @file VirtualMMU.hpp
 * @brief Virtual MMU implement of Storage::Interface
 * @version 0.1
 * @date 2022-12-06
 *
 */

#ifndef __HRCORE_VIRTUALMMU_HPP__
#define __HRCORE_VIRTUALMMU_HPP__

#include "../HRCore.h"

namespace HRCore {

#if HRCORE_ENABLE_STORAGE_VMMU
namespace Storage {

    /**
     * @brief An implement of Storage::Interface as Virtual MMU
     * @tparam N The number of items in each memory unit.
     */
    template <size_t N>
    class VirtualMMU : public Interface {
    private:
        struct data_t : public item_t {
            friend class VirtualMMU<N>;
        };

        struct node_t {
            ival_t data[N] = { 0 };
            ival_t protect;
        };

        node_t* _pool[HRCORE_STORAGE_VMMU_MAX_LEN] = { nullptr };
        size_t _count = 1; // "Actul count of items" from outside, initially it's 1.
        size_t _len = 1;

    public:
        VirtualMMU()
        {
            this->_pool[0] = new node_t;
        }

        virtual ~VirtualMMU()
        {
            while (this->_len)
                delete this->_pool[--this->_len];
            this->_count = 0;
        }

        virtual item_t at(size_t n)
        {
            data_t ret;
            if (n >= this->_count)
                return ret;
            size_t p = n / N;
            size_t f = n % N;
            auto x = this->_pool[p];
            if (!x)
                return ret;
            ret.data = x->data + f;
            ret.index = n;
            return ret;
        }

        virtual item_t begin() override { return this->at(0); }
        virtual item_t end() override { return this->at(this->_count - 1); }
        virtual size_t length() override { return this->_count; }

        virtual item_t next(item_t cur) override
        {
            data_t* helper = (data_t*)&cur;
            data_t ret;
            if (!cur || helper->index >= this->_count - 1 || helper->index + 1 < helper->index) // BUG3! When (!cur) should not get next
                return ret;
            helper->index++;
            size_t p = helper->index / N;
            size_t f = helper->index % N;
            auto x = this->_pool[p];
            if (!x)
                return ret;
            ret.index = helper->index;
            ret.data = x->data + f;
            return ret;
        }

        virtual item_t prev(item_t cur) override
        {
            data_t* helper = (data_t*)&cur;
            data_t ret;
            if (!cur || helper->index >= this->_count + 1 || helper->index - 1 > helper->index)
                return ret;
            helper->index--;
            size_t p = helper->index / N;
            size_t f = helper->index % N;
            auto x = this->_pool[p];
            if (!x)
                return ret;
            ret.index = helper->index;
            ret.data = x->data + f;
            return ret;
        }

        virtual bool shrink(size_t c) override
        {
            if (c > HRCORE_STORAGE_VMMU_MAX_LEN * N || c >= this->_count)
                return false;
            size_t rlen = c / N + (c % N ? 1 : 0);
            while (this->_len > rlen) {
                delete this->_pool[--(this->_len)];
                this->_pool[this->_len] = nullptr;
            }
            this->_count = c;
            return true;
        }

        virtual bool extend(size_t c) override
        {
            if (c > HRCORE_STORAGE_VMMU_MAX_LEN * N || c <= this->_count)
                return false;
            size_t rlen = c / N + (c % N ? 1 : 0);
            while (this->_len < rlen)
                this->_pool[this->_len++] = new node_t;
            this->_count = c;
            return true;
        }

        virtual Interface* newObject() override { return new VirtualMMU<N>(); }
        virtual void delObject(const Interface* ptr) override { delete static_cast<const VirtualMMU<N>*>(ptr); }
    };

} // namespace Storage

#endif

} // namespace HRCore

#endif