#ifndef PATHWINDOW_HPP
#define PATHWINDOW_HPP

#include <map>

#include <Path.hpp>

class PathWindow {
private:
    std::vector<const Path*> piw_; //< Pointers to paths in window.
    std::map<ulong, int> frqs; //< Path length frequencies.
    int sum_frqs; //< Sum of all path frequencies in the window (n paths).
public:
    /** Constructs a path window with paths that have paths lengths between
     *  lower (inclusive) and upper (exclusive) bound.
     *  @param l  Lower path length bound for this path group (inclusive).
     *  @param u  Upper path length bound for this path group (exclusive).
     *  @param sp Sorted pointers to paths according to path length in
     *            ascending order.
     * */
    PathWindow(ulong l, ulong u, const std::vector<const Path*>& sp);

    /** Returns a <PathLength, Frequency> pair for which has lowest frequency in
     * the frqs map. */
    std::pair<ulong, int> getLowestFrequencyEntry() const;

    /** Returns a <PathLength, Frequency> pair for which has highest frequency
     * in the frqs map. */
    std::pair<ulong, int> getHighestFrequencyEntry() const;
  
    /** Returns number of paths present in the window. In other words, returns
     * sum of all path frequencies that are contained in the window. */
    int getSumFreqs() const {return sum_frqs;}

    friend std::ostream& operator<< (std::ostream& s, const PathWindow& pw);
};

#endif
