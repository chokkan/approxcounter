#ifndef __EXACT_H__
#define __EXACT_H__

#include <unordered_map>

template <class key_type, class count_type=int>
class exact : public std::unordered_map<key_type, count_type>
{
public:
    typedef std::unordered_map<key_type, count_type> base_class;

    exact()
    {
    }

    virtual ~exact()
    {
    }

    void append(const key_type& key)
    {
        typename base_class::iterator it = this->find(key);
        if (it != this->end()) {
            ++it->second;
        } else {
            this->insert(typename base_class::value_type(key, 1));
        }
    }
};

#endif/*__EXACT_H__*/
