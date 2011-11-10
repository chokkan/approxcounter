#include <iostream>
#include <string>
#include "optparse.h"
#include "exact.h"
#include "spacesaving.h"

class option : public optparse
{
public:
    bool help;
    std::string algorithm;
    int epsilon;

public:
    option() : help(false), algorithm("exact"), epsilon(1024)
    {
    }

    BEGIN_OPTION_MAP_INLINE()
        ON_OPTION_WITH_ARG(SHORTOPT('a') || LONGOPT("algorithm"))
            algorithm = arg;

        ON_OPTION_WITH_ARG(SHORTOPT('e') || LONGOPT("epsilon"))
            epsilon = std::atoi(arg);

        ON_OPTION(SHORTOPT('h') || LONGOPT("help"))
            help = true;

    END_OPTION_MAP()
};


template <class counter_class>
void count(counter_class& counter, std::istream& is)
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

int do_exact(const option& opt)
{
    typedef exact<std::string> counter_t;
    counter_t counter;
    count(counter, std::cin);

    counter_t::const_iterator it;
    for (it = counter.begin();it != counter.end();++it) {
        std::cout << it->first << '\t' << it->second << std::endl;
    }

    return 0;
}

int do_spacesaving(const option& opt)
{
    typedef spacesaving<std::string> counter_t;
    counter_t::item_type *item = NULL;
    counter_t counter(opt.epsilon);

    count(counter, std::cin);
    for (item = counter.top();item != counter.back();item = counter.next(item)) {
        std::cout <<
            item->get_key() << '\t' <<
            item->get_count() << '\t' <<
            item->get_epsilon() << std::endl;
    }

    return 0;
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

    if (opt.algorithm == "exact") {
        do_exact(opt);
    } else if (opt.algorithm == "spacesaving") {
        do_spacesaving(opt);
    } else {
        std::cerr << "ERROR: unrecognized algorithm: " << opt.algorithm << std::endl;
    }

    return 0;
}
