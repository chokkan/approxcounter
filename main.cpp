#include <iostream>
#include <string>
#include "spacesaving.h"

int main(int argc, char *argv[])
{
    std::istream& is = std::cin;
    std::ostream& os = std::cout;
    typedef stream_summary<std::string> counter_t;
    counter_t counter(1024);

    int n = 0;
    for (;;) {
        std::string line;
        std::getline(is, line);
        if (is.eof()) {
            break;
        }
        ++n;
        if (n == 297) {
            os << "OK";
        }
        counter.append(line);
    }

    counter_t::item_t *item = counter.top();
    while (item != NULL) {
        os << item->get_count() << '\t' << item->eps << '\t' << item->key << std::endl;
        item = counter.next(item);
    }

    return 0;
}
