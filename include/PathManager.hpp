#ifndef TELOMERI_PATHMANAGER_HPP
#define TELOMERI_PATHMANAGER_HPP

#include <sstream>
#include "OverlapGraph.hpp"
#include "Utils.hpp"
#include <tuple>
#include <functional>

#define REBUILD_ATTEMPTS 3

class PathGroup;

class PathManager {
private:
    std::vector<Utils::Path> paths_;
public:
    void buildMonteCarlo(const OverlapGraph &g, int repeat_num, const Utils::Metrics &metric);

    void buildDeterministic(const OverlapGraph &g, const Utils::Metrics &metric);

    void filterUnique();

    std::tuple<ulong, ulong, ulong> getMinMaxSumPathLength();

    std::string stats();

    std::vector<PathGroup> constructGroups();
private:
    /** If difference between maximum and minimum path length is greater than
     *  this threshold,, all paths go into same group. */
    static constexpr ulong LEN_THRESHOLD = 10000ul;
    /** Window size in path length. */
    static constexpr ulong WINDOW_SIZE = 1000ul;
    /** Valley and peak ratio needed for splitting the paths into groups
     * according to lowest path length frequency in the valley window. */
    static constexpr float RATIO_THRESHOLD = 0.9f;
};


class PathGroup {
public:
    std::vector<const Utils::Path*> pig_; //< Pointers to paths in group.
public:
    PathGroup(std::vector<const Utils::Path*>::const_iterator begin,
        std::vector<const Utils::Path*>::const_iterator end);
    
    friend std::ostream& operator<< (std::ostream& s, const PathGroup& pg) {
        int i = 0;
        for (const auto pp : pg.pig_) {
            s << pp->str() << '[' << pp->length() << ']' << ' ';
            if (++i % 3 == 0) { // Print out new line every now and then.
                s << '\n'; 
            }
        }
        return s;
    }
};

#endif
