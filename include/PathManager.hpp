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

    std::vector<PathGroup> constructGroups();
private:
    /** Number of path rebuild attempts if dead-end has been reached. */
    static constexpr int REBUILD_ATTEMPTS = 100;
    /** Number of backtrack attempts when encountering a dead-end. */
    static constexpr int BACKTRACK_ATTEMPTS = 10;
    /** If difference between maximum and minimum path length is greater than
     *  this threshold,, all paths go into same group. */
    static constexpr long LEN_THRESHOLD = 10000000000ul;
    /** Faster and simpler metric for length. */
    static constexpr long NODE_NUM_THRESHOLD = 500;
    /** Window size in path length. */
    static constexpr ulong WINDOW_SIZE = 1000ul;
    /** Valley and peak ratio needed for splitting the paths into groups
     * according to lowest path length frequency in the valley window. */
    static constexpr float RATIO_THRESHOLD = 0.9f;
};


#endif
