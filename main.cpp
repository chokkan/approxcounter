#include <iostream>
#include <string>
#include "spacesaving.h"

int main(int argc, char *argv[])
{
    std::istream& is = std::cin;
    std::ostream& os = std::cout;
    typedef spacesaving<std::string> counter_t;
    counter_t counter(1024);

    int n = 0;
    for (;;) {
        std::string line;
        std::getline(is, line);
        if (is.eof()) {
            break;
        }
        ++n;
        counter.append(line);
    }

    for (counter_t::item_type *item = counter.top();item != counter.back();item = counter.next(item)) {
        os << item->get_count() << '\t' << item->get_epsilon() << '\t' << item->get_key() << std::endl;
    }

    return 0;
}
