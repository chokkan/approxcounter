#ifndef __EXACT_H__
#define __EXACT_H__

#include <unordered_map>

template <class key_tmpl, class count_tmpl=int>
class exact : public std::unordered_map<key_tmpl, count_tmpl>
{
public:
    typedef std::unordered_map<key_tmpl, count_tmpl> this_class;

    exact()
    {
    }

    virtual ~exact()
    {
    }

    void append(const key_type& key)
    {
        iterator it = this->find(key);
        if (it != this->end()) {
            ++it->second;
        } else {
            this->insert(this_class::value_type(key, 1));
        }
    }
};

#endif/*__EXACT_H__*/
