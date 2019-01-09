#ifndef TELOMERI_PATHMANAGER_HPP
#define TELOMERI_PATHMANAGER_HPP

#include <sstream>
#include <tuple>

#include <OverlapGraph.hpp>
#include <Path.hpp>
#include <PathGroup.hpp>
#include <Utils.hpp>

class PathManager {
private:
    std::vector<Path> paths_;
public:
    void buildMonteCarlo(const OverlapGraph &g, const Utils::Metrics &metric);

    void buildDeterministic(const OverlapGraph &g,
            const Utils::Metrics &metric);

    void filterUnique();

    std::tuple<ulong, ulong, ulong> getMinMaxSumPathLength();

    std::string stats();

    /** Constructs groups from the paths passed in. This should be paths that
     * connect a pair of anchors.
     * @param v Paths connecting two anchors. Sorted when this function returns.
     * @return Groups of paths between two anchors. */
    static std::vector<PathGroup> constructGroups(std::vector<const Path*>& v);

    static std::pair<ulong, ulong> getMinMaxPathLength(std::vector<const Path*>& v);

    /** Returns map that maps anchor pair to all paths beetween those achors:
     * [anchor1, achor2] => {path1, path2,...}
     *@return Map of anchor pair and paths between those anchors. */
    std::map<std::pair<const OverlapGraph::Node*, const OverlapGraph::Node*>,
    std::vector<const Path*>>
    getPathsBetweenAnchors();

private:
    /** Number of path rebuild attempts if dead-end has been reached. */
    static constexpr int REBUILD_ATTEMPTS = 500;
    /** Number of backtrack attempts when encountering a dead-end. */
    static constexpr int BACKTRACK_ATTEMPTS = 30;
    /** If difference between maximum and minimum path length is greater than
     *  this threshold, all paths go into same group. */
    static constexpr long LEN_THRESHOLD = 10000ul;
    /** Faster and simpler metric for length. */
    static constexpr long NODE_NUM_THRESHOLD = 50;
    /** Window size in path length. */
    static constexpr ulong WINDOW_SIZE = 1000ul;
    /** Valley and peak ratio needed for splitting the paths into groups
     * according to lowest path length frequency in the valley window. */
    static constexpr float RATIO_THRESHOLD = 0.9f;
};


#endif
