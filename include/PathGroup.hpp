#ifndef PATHGROUP_HPP
#define PATHGROUP_HPP

#include <Path.hpp>

class PathGroup {
public:
    std::vector<const Path*> pig_; //< Pointers to paths in group.
public:
    /** Constructs a path group with path pointers defined with provided
     * iterators.
     * @param begin Begining of the path pointers collection (inclusive).
     * @param end End of the path pointers collection (exclusive). */
    PathGroup(std::vector<const Path*>::const_iterator begin,
            std::vector<const Path*>::const_iterator end);
    
    friend std::ostream& operator<< (std::ostream& s, const PathGroup& pg);
};

#endif
