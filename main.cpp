#include <iostream>
#include <string>
#include <unordered_map>
#include <stdint.h>

#include "optparse.h"
#include "exact.h"
#include "spacesaving.h"
#include "tokenize.h"

class option : public optparse
{
public:
    bool help;
    std::string algorithm;
    std::string type;
    int epsilon;
    int token_field;
    int freq_field;
    double support;
    bool absolute_support;

public:
    option()
        : help(false), algorithm("exact"), type("uint32"), epsilon(1024),
        token_field(1), freq_field(2),
        support(0.), absolute_support(false)
    {
    }

    BEGIN_OPTION_MAP_INLINE()
        ON_OPTION_WITH_ARG(SHORTOPT('a') || LONGOPT("algorithm"))
            algorithm = arg;

        ON_OPTION_WITH_ARG(SHORTOPT('c') || LONGOPT("type"))
            type = arg;

        ON_OPTION_WITH_ARG(SHORTOPT('t') || LONGOPT("token-field"))
            token_field = std::atoi(arg);

        ON_OPTION_WITH_ARG(SHORTOPT('f') || LONGOPT("freq-field"))
            freq_field = std::atoi(arg);

        ON_OPTION_WITH_ARG(SHORTOPT('s') || LONGOPT("support"))
            support = std::atof(arg);
            absolute_support = false;

        ON_OPTION_WITH_ARG(SHORTOPT('S') || LONGOPT("absolute-support"))
            support = std::atof(arg);
            absolute_support = true;

        ON_OPTION_WITH_ARG(SHORTOPT('e') || LONGOPT("epsilon"))
            epsilon = std::atoi(arg);

        ON_OPTION(SHORTOPT('h') || LONGOPT("help"))
            help = true;

    END_OPTION_MAP()
};


template <class counter_class>
void count_data(counter_class& counter, std::istream& is)
{
    for (;;) {
        std::string line;
        std::getline(is, line);
        if (is.eof()) {
            break;
        }
        counter.append(line);
    }
}

template <class count_type>
int count_exact(const option& opt)
{
    typedef exact<std::string, count_type> counter_t;
    counter_t counter;
    count_data(counter, std::cin);

    double threshold = opt.absolute_support ? opt.support : opt.support * counter.total();
    for (typename counter_t::const_iterator it = counter.begin();it != counter.end();++it) {
        if (it->second  >= threshold) {
            std::cout << it->first << '\t' << it->second << std::endl;
        }
    }

    return 0;
}

template <class count_type>
int count_spacesaving(const option& opt)
{
    typedef spacesaving<std::string, count_type> counter_t;
    typename counter_t::item_type *item = NULL;
    counter_t counter(opt.epsilon);

    count_data(counter, std::cin);
    double threshold = opt.absolute_support ? opt.support : opt.support * counter.total();
    for (item = counter.top();item != counter.back();item = counter.next(item)) {
        std::cout <<
            item->get_key() << '\t' <<
            item->get_count() << '\t' <<
            item->get_epsilon() << std::endl;
    }

template <class count_type>
int do_sum_spacesaving(const option& opt)
{
    typedef spacesaving_PriorityQ<std::string, count_type> counter_t;
    typename counter_t::item_type *item = NULL;
    counter_t counter(opt.epsilon);
	count_type n = 0;
	
	for (;;) {
		//input data
        std::string line;
        std::getline(std::cin, line);
        if (std::cin.eof()) {
            break;
        }
		std::string token;
        int k = 1, freq = 0;
		//freq = 1; //for debug
        tokenizer fields(line, '\t');
        for (tokenizer::iterator it = fields.begin();it != fields.end();++it) {
            if (k == opt.token_field) {
                token = *it;
            }
            if (k == opt.freq_field) {
                freq = std::atoi(it->c_str());
            }
            ++k;
        }

		counter.append(token,freq);
		//std::cout<<token<<std::endl;
		//counter.debug();
		n+=freq;
	}
	
	double threshold = opt.absolute_support ? opt.support : opt.support * n;
	counter.showresult();
    return 0;
}

template <class count_type>
int do_sum(const option& opt)
{
    typedef std::unordered_map<std::string, count_type> counter_t;
    counter_t counter;
    count_type n = 0;

    for (;;) {
        std::string line;
        std::getline(std::cin, line);
        if (std::cin.eof()) {
            break;
        }

        std::string token;
        int k = 1, freq = 0;
        tokenizer fields(line, '\t');
        for (tokenizer::iterator it = fields.begin();it != fields.end();++it) {
            if (k == opt.token_field) {
                token = *it;
            }
            if (k == opt.freq_field) {
                freq = std::atoi(it->c_str());
            }
            ++k;
        }
        
        typename counter_t::iterator it = counter.find(token);
        if (it != counter.end()) {
            it->second += freq;
        } else {
            counter.insert(typename counter_t::value_type(token, freq));
        }

        n += freq;
    }

    double threshold = opt.absolute_support ? opt.support : opt.support * n;
    typename counter_t::const_iterator it;
    for (it = counter.begin();it != counter.end();++it) {
        if (it->second  >= threshold) {
            std::cout << it->first << '\t' << it->second << std::endl;
        }
    }

    return 0;
}

template <class count_type>
int count(const option& opt)
{
    if (opt.algorithm == "exact") {
        return count_exact<count_type>(opt);
    } else if (opt.algorithm == "sum") {
        return do_sum<count_type>(opt);
    } else if (opt.algorithm == "spacesaving") {
        return count_spacesaving<count_type>(opt);
	} else if (opt.algorithm == "sum_spacesaving") {
		return do_sum_spacesaving<count_type>(opt);
    } else {
        std::cerr << "ERROR: unrecognized algorithm: " << opt.algorithm << std::endl;
        return 1;
    }
}

int main(int argc, char *argv[])
{
    option opt;

    try { 
        int arg_used = opt.parse(argv, argc);
    } catch (const optparse::unrecognized_option& e) {
        std::cerr << "ERROR: unrecognized option: " << e.what() << std::endl;
        return 1;
    } catch (const optparse::invalid_value& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    if (opt.type == "uint16") {
        return count<uint16_t>(opt);
    } else if (opt.type == "uint32") {
        return count<uint32_t>(opt);
    } else if (opt.type == "uint64") {
        return count<uint64_t>(opt);
    } else {
        std::cerr << "ERROR: unrecognized type: " << opt.type << std::endl;
        return 1;
    }

    return 1;
}
