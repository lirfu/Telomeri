#ifndef PATHGROUP_HPP
#define PATHGROUP_HPP

#include <Path.hpp>

#include <map>

class PathGroup {
public:
    std::vector<const Path*> pig_; //< Pointers to paths in group.
    std::map<ulong, int> frqs; //< Path length frequencies of paths in group.
public:
    /** Constructs a path group with path pointers defined with provided
     * iterators.
     * @param begin Begining of the path pointers collection (inclusive).
     * @param end End of the path pointers collection (exclusive). */
    PathGroup(std::vector<const Path*>::const_iterator begin,
            std::vector<const Path*>::const_iterator end);

    /** Discards paths with path length frequency lower than half of the
     * highest path length frequency in the group. */
    void discardNotFrequent();
    
private:
    /** Returns a <PathLength, Frequency> pair for which has lowest frequency in
     * the frqs map. */
    std::pair<ulong, int> getLowestFrequencyEntry() const;

    /** Returns a <PathLength, Frequency> pair for which has highest frequency
     * in the frqs map. */
    std::pair<ulong, int> getHighestFrequencyEntry() const;


    friend std::ostream& operator<< (std::ostream& s, const PathGroup& pg);
};

#endif
