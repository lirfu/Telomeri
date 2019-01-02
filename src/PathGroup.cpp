#include <PathGroup.hpp>

#include <ostream>

 PathGroup::PathGroup(std::vector<const Path*>::const_iterator begin,
        std::vector<const Path*>::const_iterator end)
        : pig_(begin, end) {}

std::ostream& operator<< (std::ostream& s, const PathGroup& pg) {
    int i = 0;
    for (const auto pp : pg.pig_) {
        s << *pp << '[' << pp->length() << ']' << ' ';
        if (++i % 3 == 0) { // Print out new line every now and then.
            s << '\n'; 
        }
    }
    return s;
}
