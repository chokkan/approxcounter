#ifndef __SPACESAVING_H__
#define __SPACESAVING_H__

typedef unsigned int count_t;

#include <map>

template <class key_type>
class stream_summary
{
public:
    typedef key_type key_t;

    struct item_t;

protected:
    /**
     * Doubly-linked list of buckets.
     */
    struct bucket_t
    {
        count_t count;      ///< Item count.
        item_t *head;       ///< Pointer to the first item.
        item_t *tail;       ///< Pointer to the last item.
        bucket_t *prev;     ///< Pointer to the previous bucket.
        bucket_t *next;     ///< Pointer to the next bucket.

        /**
         * Constructs a bucket.
         *  @param  c       The count value.
         */
        bucket_t(count_t c=0) :
            count(c), head(NULL), tail(NULL), prev(NULL), next(NULL)
        {
        }
    };

public:
    /**
     * Doubly-linked list of items.
     */
    struct item_t
    {
        key_t key;          ///< The key
        count_t eps;        ///< Epsilon (the maximum error of the count)
        bucket_t *parent;   ///< Pointer to the bucket owning this item.
        item_t *prev;       ///< Pointer to the previous item.
        item_t *next;       ///< Pointer to the next item.

        /**
         * Constructs a bucket.
         *  @param  e       The epsilon value.
         */
        item_t(count_t e=0)
            : eps(e), parent(NULL), prev(NULL), next(NULL)
        {
        }

        /**
         * Constructs a bucket.
         *  @param  key     The key.
         *  @param  e       The epsilon value.
         */
        item_t(const key_t& k, count_t e=0)
            : key(k), eps(e), parent(NULL), prev(NULL), next(NULL)
        {
        }

        count_t get_count()
        {
            return parent->count;
        }
    };

public:
    typedef key_type key_t;
    typedef std::map<key_t, item_t*> keys_t;

    count_t m_m;
    keys_t m_keys;
    bucket_t *m_root;

public:
    stream_summary(count_t m=4) : m_root(NULL), m_m(m)
    {
    }

    virtual ~stream_summary()
    {
    }

protected:
    void increment(item_t* item)
    {
        // The bucket storing the item.
        bucket_t *bucket = item->parent;

        // Detach the item from the bucket.
        detach_item(item);

        // Incremented count of the item.
        count_t count = bucket->count+1;

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
            erase_bucket(bucket);
            delete bucket;
        }
    }

public:
    void append(const key_t& key)
    {
        keys_t::iterator it = m_keys.find(key);
        if (it != m_keys.end()) {
            // Increment the counter.
            this->increment(it->second);
        } else if (m_keys.size() < m_m) {
            // Create an item and insert it into the root bucket.
            if (m_root == NULL || 1 < m_root->count) {
                // Create the root (count=1) bucket.
                m_root = new bucket_t(1);
            }
            item_t *item = new item_t(key);
            append_item(m_root, item);
            m_keys[key] = item;
        } else {
            // The replacement step.
            bucket_t *bucket = m_root;
            item_t *item = bucket->head;
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
        for (keys_t::const_iterator it = m_keys.begin();it != m_keys.end();++it) {
            os << it->first << ": " << it->second->parent->count << "(" << it->second->eps << ")" << std::endl;
        }

        bucket_t *bucket = m_root;
        os << "items {" << std::endl;
        for (;bucket != NULL;bucket = bucket->next) {
            os << "  count " << bucket->count << " {" << std::endl;
            item_t *item = bucket->head;
            for (;item != NULL;item = item->next) {
                os << "    " << item->key << std::endl;
            }
            os << "  }" << std::endl;
        }
        os << "}" << std::endl;
    }

    item_t *top()
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

    item_t *next(item_t *cur)
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
    void detach_item(item_t *item)
    {
        item_t *prev = item->prev;
        item_t *next = item->next;
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

    void append_item(bucket_t *parent, item_t *item)
    {
        item_t *head = parent->head;
        item_t *tail = parent->tail;
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
        if (first != NULL) {
            second->next = first->next;
            first->next = second;
            second->prev = first;
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
