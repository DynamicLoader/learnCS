/**
 * @file StorageIF.hpp
 * @brief Storage Interface
 * @version 0.1
 * @date 2022-12-06
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __HRCORE_STORAGEIF__HPP__
#define __HRCORE_STORAGEIF__HPP__

#include "../HRCore.h"

namespace HRCore {

/**
 * @brief A namespace contains Storage interface and implements
 * @namespace HRCore::Storage
 */
namespace Storage {

    /**
     *  @class Interface
     *  @brief A class as a storage interface, pure virtual.
     */
    class Interface {
    public:

        /// @brief Storage item descriptor.
        struct item_t {
            ival_t* data = nullptr;

            /**
             * @brief Allows the syntax of "if(item)"
             * @return true if data != nullptr
             */
            explicit operator bool() const noexcept { return (data != nullptr); }
        protected:
            /// Store an appendix pointer
            void* appendix = nullptr;

            /// Store the index of data.
            size_t index = 0;
        };

        /**
         * @brief  Return the first item of storage.
         * @return item_t The first item.
         */
        virtual item_t begin() = 0;

        /**
         * @brief Returns the last item of storage.
         * @return item_t The last item.
         */
        virtual item_t end() = 0;

        /**
         * @brief Get next item.
         *
         * @param cur Current item.
         * @return item_t The next item.
         */
        virtual item_t next(item_t cur) = 0;

        /**
         * @brief
         *
         * @param cur Current item.
         * @return item_t The previous item.
         */
        virtual item_t prev(item_t cur) = 0;

        /**
         * @brief Access to storage[pos].
         * @param pos The index of item, starting from 0.
         * @return item_t
         */
        virtual item_t at(size_t pos) = 0;

        /**
         * @brief Get the length of storage.
         * @return size_t Length of storage
         */

        virtual size_t length() = 0;

        /**
         * @brief Remove data from the end to shrink the size to a given length.
         * @param len The length to be remained.
         * @return true if success
         * @return false if failed
         */
        virtual bool shrink(size_t len) = 0;

        /**
         * @brief Extend the storage to a given length.
         *
         * @param len Final length.
         * @return true if success
         * @return false if no space or ...
         */
        virtual bool extend(size_t len) = 0;

        /**
         * @brief Get a new object of same type
         * @return Interface* The pointer of new object
         */
        virtual Interface* newObject() = 0;

        /**
         * @brief Delete a object from newObject()
         * 
         * @param ptr The pointer of object to be deleted
         */
        virtual void delObject(const Interface* ptr) = 0;
        
    };

}
}

#endif