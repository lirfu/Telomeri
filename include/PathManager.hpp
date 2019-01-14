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

    struct Parameters {
        /** Number of path rebuild attempts if dead-end has been reached. */
        int rebuild_attempts;
        /** Number of backtrack attempts when encountering a dead-end. */
        int backtrack_attempts;
        /** If difference between maximum and minimum path length is greater than
         *  this threshold, all paths go into same group. */
        long len_threshold;
        /** Faster and simpler metric for length. */
        long node_num_threshold;
        /** Window size in path length. */
        ulong window_size;
        /** Valley and peak ratio needed for splitting the paths into groups
         * according to lowest path length frequency in the valley window. */
        float ratio_threshold;
    };

    Parameters params_;

    /** Constructs groups from the paths passed in. This should be paths that
     * connect a pair of anchors.
     * @param v Paths connecting two anchors. Sorted when this function returns.
     * @param params Path manager parameters.
     * @return Groups of paths between two anchors. */
    static std::vector<PathGroup> constructGroups(std::vector<const Path*>& v, PathManager::Parameters params);

    static std::pair<ulong, ulong> getMinMaxPathLength(std::vector<const Path*>& v);

    /** Returns map that maps anchor pair to all paths beetween those achors:
     * [anchor1, achor2] => {path1, path2,...}
     *@return Map of anchor pair and paths between those anchors. */
    std::map<std::pair<const OverlapGraph::Node*, const OverlapGraph::Node*>,
    std::vector<const Path*>>
    getPathsBetweenAnchors();

    Path constructConsensusPath(
            const std::map<std::pair<const OverlapGraph::Node *, const OverlapGraph::Node *>,
                    std::vector<const Path *>> &,
            std::map<std::pair<const OverlapGraph::Node *,
                    const OverlapGraph::Node *>, const Path *>&);
};


#endif
