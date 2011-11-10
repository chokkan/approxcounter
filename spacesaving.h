/*
 *      Space Saving algorithm.
 *
 * Copyright (c) 2011 Naoaki Okazaki
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the authors nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SPACESAVING_H__
#define __SPACESAVING_H__

#include <unordered_map>
#include <cassert>

/**
 * Space-saving algorithm.
 *  @param  key_tmpl        Key type.
 *  @param  count_tmpl      Count type.
 */
template <class key_tmpl, class count_tmpl=int>
class spacesaving
{
public:
    /// Key type.
    typedef key_tmpl key_type;
    /// Count type.
    typedef count_tmpl count_type;
    /// This class.
    typedef spacesaving<key_tmpl, count_tmpl> this_type;

protected:
    struct bucket_t;

public:
    /**
     * A count item.
     *  This class implements a doubly-linked list of count items.
     */
    class item_type
    {
        friend class spacesaving<key_tmpl, count_tmpl>;

    protected:
        key_type key;       ///< The key
        count_type eps;     ///< Epsilon (maximum overestimation of the count)
        bucket_t *parent;   ///< Pointer to the bucket owning this item.
        item_type *prev;    ///< Pointer to the previous item.
        item_type *next;    ///< Pointer to the next item.

    public:
        /**
         * Constructs a count item.
         *  @param  e       The epsilon value.
         */
        item_type(count_type e=0)
            : eps(e), parent(NULL), prev(NULL), next(NULL)
        {
        }

        /**
         * Constructs a count item.
         *  @param  key     The key.
         *  @param  e       The epsilon value.
         */
        item_type(const key_type& k, count_type e=0)
            : key(k), eps(e), parent(NULL), prev(NULL), next(NULL)
        {
        }

        /**
         * Gets the key.
         *  @return key_type&   the reference to the key.
         */
        const key_type& get_key() const
        {
            return this->key;
        }

        /**
         * Gets the count of the key.
         *  @return count_type  the count.
         */
        count_type get_count() const
        {
            return parent->count;
        }

        /**
         * Gets the epsilon (maximum overestimation) of the count.
         *  @return count_type  the epsilon value.
         */
        count_type get_epsilon() const
        {
            return this->eps;
        }
    };

protected:
    /**
     * A bucket storing items with the same count.
     *  This class implements a doubly-linked list of buckets.
     */
    struct bucket_t
    {
        count_type count;   ///< Item count.
        item_type *head;    ///< Pointer to the first item.
        item_type *tail;    ///< Pointer to the last item.
        bucket_t *prev;     ///< Pointer to the previous bucket.
        bucket_t *next;     ///< Pointer to the next bucket.

        /**
         * Constructs a bucket.
         *  @param  c       The count value.
         */
        bucket_t(count_type c=0) :
            count(c), head(NULL), tail(NULL), prev(NULL), next(NULL)
        {
        }
    };

protected:
    /// A mapping type: key -> *item.
    typedef std::unordered_map<key_type, item_type*> keys_t;
    /// The mapping object: key -> *item.
    keys_t m_keys;
    /// The maximum number of counters.
    count_type m_m;
    /// The pointer to the first bucket.
    bucket_t *m_root;

public:
    /**
     * Constructs an object.
     *  @param  m       The maximum number of counters.
     */
    spacesaving(count_type m=4) : m_root(NULL), m_m(m)
    {
    }

    /**
     * Destructs the object.
     */
    virtual ~spacesaving()
    {
    }

protected:
    void increment(item_type* item)
    {
        // The bucket storing the item.
        bucket_t *bucket = item->parent;

        // Detach the item from the bucket.
        detach_item(item);

        // Incremented count of the item.
        count_type count = bucket->count+1;

        // Find the right bucket for storing the item (for the incremented count).
        if (bucket->next != NULL && bucket->next->count == count) {
            // Attach the item to the next bucket.
            append_item(bucket->next, item);
        } else {
            // Create a new bucket and insert it after the bucket.
            bucket_t *new_bucket = new bucket_t(count);
            insert_bucket(bucket, new_bucket);
            append_item(new_bucket, item);
        }

        // Remove the bucket if it is empty.
        if (bucket->head == NULL) {
            assert(bucket->tail == NULL);
            erase_bucket(bucket);
            delete bucket;
        }
    }

public:
    void append(const key_type& key)
    {
        typename keys_t::iterator it = m_keys.find(key);
        if (it != m_keys.end()) {
            // Increment the counter.
            this->increment(it->second);
        } else if ((count_type)m_keys.size() < m_m) {
            // Create an item and insert it into the root bucket.
            if (m_root == NULL || 1 < m_root->count) {
                // Create the root (count=1) bucket.
                m_root = new bucket_t(1);
            }
            item_type *item = new item_type(key);
            append_item(m_root, item);
            m_keys[key] = item;
        } else {
            // The replacement step.
            bucket_t *bucket = m_root;
            item_type *item = bucket->head;
            m_keys.erase(item->key);
            item->key = key;
            item->eps = bucket->count;
            this->increment(item);
            m_keys[key] = item;
        }
    }

    void debug(std::ostream& os)
    {
        os << "[keys]" << std::endl;
        for (typename keys_t::const_iterator it = m_keys.begin();it != m_keys.end();++it) {
            os << it->first << ": " << it->second->parent->count << "(" << it->second->eps << ")" << std::endl;
        }

        bucket_t *bucket = m_root;
        os << "items {" << std::endl;
        for (;bucket != NULL;bucket = bucket->next) {
            os << "  count " << bucket->count << " {" << std::endl;
            item_type *item = bucket->head;
            for (;item != NULL;item = item->next) {
                os << "    " << item->key << std::endl;
            }
            os << "  }" << std::endl;
        }
        os << "}" << std::endl;
    }

    item_type *top()
    {
        bucket_t *bucket = m_root;
        if (bucket != NULL) {
            while (bucket->next != NULL) {
                bucket = bucket->next;
            }
            return bucket->tail;
        } else {
            return NULL;
        }
    }

    item_type *back()
    {
        if (m_root != NULL) {
            return m_root->head;
        } else {
            return NULL;
        }
    }

    item_type *next(item_type *cur)
    {
        if (cur->prev != NULL) {
            return cur->prev;
        } else {
            bucket_t *bucket = cur->parent;
            if (bucket->prev != NULL) {
                return bucket->prev->tail;
            } else {
                return NULL;
            }
        }
    }

protected:
    void detach_item(item_type *item)
    {
        item_type *prev = item->prev;
        item_type *next = item->next;
        bucket_t *parent = item->parent;
        if (parent->head == item) {
            parent->head = next;
        }
        if (parent->tail == item) {
            parent->tail = prev;
        }
        if (prev != NULL) {
            prev->next = next;
        }
        if (next != NULL) {
            next->prev = prev;
        }
        item->parent = NULL;
        item->prev = NULL;
        item->next = NULL;
    }

    void append_item(bucket_t *parent, item_type *item)
    {
        item_type *head = parent->head;
        item_type *tail = parent->tail;
        if (tail == NULL) {
            item->prev = NULL;
            item->next = NULL;
            parent->head = item;
        } else {
            tail->next = item;
            item->prev = tail;
            item->next = NULL;
        }
        item->parent = parent;
        parent->tail = item;
    }

    void insert_bucket(bucket_t *first, bucket_t *second)
    {
        bucket_t *next = first->next;
        if (first != NULL) {
            second->next = next;
            first->next = second;
            second->prev = first;
            if (next != NULL) {
                next->prev = second;
            }
        } else {
            m_root = second;
            second->prev = NULL;
            second->next = NULL;
        }
    }

    void erase_bucket(bucket_t *bucket)
    {
        bucket_t *prev = bucket->prev;
        bucket_t *next = bucket->next;
        if (prev != NULL) {
            prev->next = next;
        }
        if (next != NULL) {
            next->prev = prev;
        }
        if (m_root == bucket) {
            m_root = next;
        }
    }
};

#endif/*__SPACESAVING_H__*/
