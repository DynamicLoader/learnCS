/**
 * @file LinkedList.hpp
 * @brief Linked List implement of Storage::Interface
 * @version 0.1
 * @date 2022-12-06
 *
 */

#ifndef __HRCORE_LINKEDLIST_HPP__
#define __HRCORE_LINKEDLIST_HPP__

#include "../HRCore.h"

namespace HRCore {
namespace Storage {

    /**
     * @namespace Storage
     * @class LinkedList
     * @brief An implement of Storage::Interface using linked list
     *
     * @tparam N Count of items per node.
     */
    template <size_t N>
    class LinkedList : public Interface {
    private:
        // Type definations
        struct node_t {
            ival_t data[N] = { 0 };
            node_t* prev = nullptr;
            node_t* next = nullptr;
        };
        struct data_t : public item_t {
            friend class LinkedList<N>; // Add friend defination to allow the access to item_t 's protected member
        };

        // Internal data
        node_t* _head = nullptr;
        node_t* _tail = nullptr;
        size_t _len = 1; // A node is required to be allocate when init.
        size_t _count = 1; // "Actul count of items" from outside, initially it's 1.

        // Add a empty node after pos
        bool _addNode(node_t* pos)
        {
            if (!pos)
                return false;
            node_t* tmp = new node_t;
            tmp->next = pos->next;
            tmp->prev = pos;
            pos->next = tmp;
            if (tmp->next) {
                tmp->next->prev = tmp;
            } else { // No next, is the tail node.
                this->_tail = tmp;
            }
            ++_len;
            return true;
        }

        // remove a node
        bool _removeNode(node_t* pos)
        {
            if (!pos || this->_len == 1) // the last one node; can't remove.
                return false;
            if (pos == this->_head && pos == this->_tail)
                return false;
            if (pos->prev) {
                pos->prev->next = pos->next;
            } else {
                this->_head = pos->next;
            }

            if (pos->next) {
                pos->next->prev = pos->prev;
            } else {
                this->_tail = pos->prev;
            }
            delete pos;
            --_len;
            return true;
        }

    public:
        LinkedList()
        {
            this->_head = new node_t;
            this->_tail = this->_head;
            this->_head->next = nullptr;
            this->_head->prev = nullptr;
        }

        virtual ~LinkedList()
        {
            for (node_t* tmp = nullptr; _head != nullptr; _head = tmp) {
                tmp = _head->next;
                delete _head;
            }
        }
        size_t length() override { return _count; }
        item_t begin() override { return this->at(0); }
        item_t end() override { return this->at(this->_count - 1); }

        item_t at(size_t pos) override
        {
            data_t ret;
            if (pos >= this->_count)
                return ret.data = nullptr, ret;
            size_t rpos = pos / N;
            if (rpos >= _len)
                return ret.data = nullptr, ret;
            size_t opos = pos % N;
            node_t* n = nullptr;
            // Binary search
            if (rpos >= this->_len / 2) {
                n = this->_tail;
                for (size_t i = 0; n != nullptr && i < this->_len - rpos - 1; ++i)
                    n = n->prev;
            } else {
                n = this->_head;
                for (size_t i = 0; n != nullptr && i < rpos; ++i)
                    n = n->next;
            }
            ret.data = n->data + opos;
            ret.appendix = (void*)n;
            ret.index = opos;
            return ret;
        }

        bool shrink(size_t len) override
        {
            size_t rlen = len / N + (len % N ? 1 : 0);
            if (rlen > this->_len) // Can not shrink to a bigger size
                return false;
            size_t rcount = this->_len - rlen;
            bool ret = true;
            for (size_t i = 0; i < rcount; ++i)
                ret &= this->_removeNode(this->_tail);
            this->_count = len;
            return ret;
        }

        bool extend(size_t len) override
        {
            size_t elen = len / N + (len % N ? 1 : 0);
            if (elen < this->_len)
                return false;
            size_t ecount = elen - this->_len;
            bool ret = true;
            for (size_t i = 0; i < ecount; ++i)
                ret &= this->_addNode(this->_tail);
            this->_count = len;
            return ret;
        }

        item_t next(item_t pos) override
        {
            data_t* tmp = (data_t*)&pos;
            if (!tmp->appendix || tmp->index >= N || (tmp->appendix == this->_tail && tmp->index + 1 >= this->_count - (this->_len - 1) * N)) // Illegal position
                return tmp->data = nullptr, *tmp;
            if (tmp->index < N - 1) { // Not overflow...
                tmp->data++;
                tmp->index++;
                return *tmp;
            } else {
                tmp->appendix = (void*)(((node_t*)(tmp->appendix))->next);
                tmp->data = (tmp->appendix ? ((node_t*)tmp->appendix)->data : nullptr);
                tmp->index = 0;
                return *tmp;
            }
        }

        item_t prev(item_t pos) override
        {
            data_t* tmp = (data_t*)&pos;
            if (!tmp->appendix || tmp->index >= N || (tmp->appendix == this->_head && tmp->index == 0))
                return tmp->data = nullptr, *tmp;
            if (tmp->index > 0) { // Not overflow...
                tmp->data--;
                tmp->index--;
                return *tmp;
            } else {
                tmp->appendix = (void*)(((node_t*)(tmp->appendix))->prev);
                tmp->data = (tmp->appendix ? ((node_t*)tmp->appendix)->data + N - 1 : nullptr);
                tmp->index = N - 1;
                return *tmp;
            }
        }

        Interface* newObject() override { return new LinkedList<N>(); }
        void delObject(const Interface* ptr) override { delete static_cast<const LinkedList<N>*>(ptr); }
    };

}
}

#endif