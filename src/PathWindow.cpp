#include <PathWindow.hpp>

#include <ostream>
#include <algorithm>

PathWindow::PathWindow(ulong l, ulong u,
        const std::vector<const Path*>& sp)
        : sum_frqs(0) {
    for (size_t i = 0, n = sp.size(); i < n; i++) {
        if (sp[i]->length() >= u) break; // Passed upper limit.
        if (sp[i]->length() >= l) {
            piw_.emplace_back(sp[i]);
            
            // If this is first time seeing this path length, set it to one.
            // Otherwise increment.
            frqs[sp[i]->length()] = frqs.count(sp[i]->length()) == 0 ?
                1 : frqs[sp[i]->length()] + 1;

            // Increase total number of paths in window (sum frequencies).
            sum_frqs++;
        }
    }
}

std::pair<ulong, int> PathWindow::getLowestFrequencyEntry() const {
    // Min element return an iteratior, hence dereference.
    return *std::min_element(std::begin(frqs), std::end(frqs),
        [] (const std::pair<ulong, int>& p1, const std::pair<ulong, int> & p2) {
            return p1.second < p2.second;
        });
}

std::pair<ulong, int> PathWindow::getHighestFrequencyEntry() const {
    // Max element return an iteratior, hence dereference.
    return *std::max_element(std::begin(frqs), std::end(frqs),
        [] (const std::pair<ulong, int>& p1, const std::pair<ulong, int> & p2) {
            return p1.second < p2.second;
        });
}
 
std::ostream& operator<< (std::ostream& s, const PathWindow& pw) {
    for (const auto pp : pw.piw_) {
        s << '[' << pp->length() << ']' << ' ';
    }
    return s;
}
