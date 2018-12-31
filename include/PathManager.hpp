//
// Created by lirfu on 26.12.18..
//

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
};


class PathGroup {
private:
    ulong lower; //< Lower path length bound (inclusive).
    ulong upper; //< upper path length bound (exclusive).
    std::vector<const Utils::Path*> pig_; //< Pointers to paths in group.
public:
    /** Constructs a path group with paths that have paths lengths between lower
     *  (inclusive) and upper (exclusive) bound.
     *
     *  @param l  Lower path length bound for this path group (inclusive).
     *  @param u  Upper path length bound for this path group (exclusive).
     *  @param sp Sorted pointers to paths according to path lenght in
     *            ascending order.
     * */
    PathGroup(ulong l, ulong u, const std::vector<const Utils::Path*>& sp);

    std::string str() const {
        std::stringstream ss;
        for (const auto pp : pig_) {
            ss << pp->str() << '[' << pp->length() << ']' << "  ";
        }
        return ss.str();
    }

    ulong getLowerBound() {return lower;}
    ulong getUpperBound() {return upper;}
};


#endif //TELOMERI_PATHMANAGER_HPP
