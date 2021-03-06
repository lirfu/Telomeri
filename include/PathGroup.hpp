#ifndef PATHGROUP_HPP
#define PATHGROUP_HPP

#include <Path.hpp>

#include <map>

class PathGroup {
public:
    std::vector<const Path*> pig_; //< Pointers to paths in group (sorted).
    std::map<ulong, int> frqs; //< Path length frequencies of paths in group.
    const Path* consensus; //< Group consensus sequence.
    int valid_path_number; //< Number of paths in group equal to consensus.
public:
    /** Constructs a path group with path pointers defined with provided
     * iterators.
     * @param begin Beginning of the path pointers collection (inclusive).
     * @param end End of the path pointers collection (exclusive). */
    PathGroup(std::vector<const Path*>::const_iterator begin,
            std::vector<const Path*>::const_iterator end);

    /** Discards paths with path length frequency lower than half of the
     * highest path length frequency in the group. */
    void discardNotFrequent();
    
    /** Calculates consensus path among all paths in the group. Sets sequence
     * of the group if consensus can be made, nullptr otherwise. */
    void calculateConsensusPath();
    
    /** Calculates value path number of the group. Counts number of paths in the
     * pig_ vector matching the consensus sequence. */
    void calculateValidPathNumber();
    
private:
    /** Returns a <PathLength, Frequency> pair for which has lowest frequency in
     * the frqs map. */
    std::pair<ulong, int> getLowestFrequencyEntry() const;

    /** Returns a <PathLength, Frequency> pair for which has highest frequency
     * in the frqs map. */
    std::pair<ulong, int> getHighestFrequencyEntry() const;


    /** Removes paths that have path length frequency below provided threshold
     * from the pig_ vector. In other words, these paths have their lengths
     * removed from the frqs map. */
    void deletePathsBelowThreshold();

    friend std::ostream& operator<< (std::ostream& s, const PathGroup& pg);

private:
    /** If paths lengths within a single group are distributed across a window
     * wider than this threshold, then this region cannot be resolved and group
     * cannot provide consensus sequence. */
    static constexpr ulong CONSENSUS_THRESHOLD = 100000;
};

#endif
