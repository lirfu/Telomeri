#include <PathGroup.hpp>

#include <iostream>
#include <algorithm>
#include <numeric>
#include <stdexcept>

 PathGroup::PathGroup(std::vector<const Path*>::const_iterator begin,
        std::vector<const Path*>::const_iterator end)
        : pig_(begin, end), consensus(nullptr), valid_path_number(0)
{
    /* NOTE: This could be double work if group is made after path windows
     * are build since windows already contain frqs map for paths that are
     * in the window. If performance in this area shows to be critical, 
     * consider offering another constructor which does not calculate path
     * length frequencies all over again, but takes them from path windows. */
    for (const Path* path_p : pig_) {
        // If this is first time seeing this path length, set it to one.
        // Otherwise increment.
        frqs[path_p->length()] = frqs.count(path_p->length()) == 0 ?
                1 : frqs[path_p->length()] + 1;
    }
}


std::ostream& operator<< (std::ostream& s, const PathGroup& pg) {
    int i = 0; 
    s << '\t';
    for (const auto pp : pg.pig_) {
        s << '[' << pp->length() << ']' << ' ';
        if (++i % 5 == 0) { // Print out new line every now and then.
            s << '\n' << '\t'; 
        }
    }
    return s;
}



void PathGroup::discardNotFrequent() {
    int highest_plf = getHighestFrequencyEntry().second;
    int threshold_plf = highest_plf / 2; // If path has frequency less than this, erase it.
    if (threshold_plf == 0) return;      // Speed return since no paths will be removed.

    for (int i = 0, n = static_cast<int>(pig_.size()); i < n; i++) { // Iterate over paths.
        ulong path_length = pig_[i]->length(); // Get path length for current path.
        int plf = frqs[path_length]; // Get path length frequency of length of current path.

        if (plf < threshold_plf) {   // Path length frequency is lower than threshold.
            frqs.erase(path_length); // Remove this path length from the map (if exists).
        }
    }

    deletePathsBelowThreshold(); // Delete paths below threshold from the pig_ vector.
}


void PathGroup::calculateConsensusPath() {
    // If group has paths of higly different lengths, consensus cannot be made.
    if (pig_.back()->length() - pig_.front()->length() > CONSENSUS_THRESHOLD) {
        consensus = nullptr;
        return;
    }

    // Calculate average path length.
    size_t avg = std::accumulate(pig_.begin(), pig_.end(), 0,
            [] (size_t sum, const Path* p) { return sum + p->length(); })
        / pig_.size();

    // Return first element that has average or higher path length.
    for (const auto pp : pig_) {
        if (pp->length() >= avg) {
            consensus = pp;
            return;
        }
    }

    consensus = pig_.back(); // Return last element if none was found before.
}



void PathGroup::calculateValidPathNumber() {
    valid_path_number = 0;
    if (!consensus) return; // No consesus sequence found for the group.
    
    // Count number of paths equal to group consensus.
    for (const Path* pp : pig_) {
        if (*consensus == *pp){
            valid_path_number++;
        }
    } 
}


std::pair<ulong, int> PathGroup::getLowestFrequencyEntry() const {
    // Min element return an iteratior, hence dereference.
    return *std::min_element(std::begin(frqs), std::end(frqs),
        [] (const std::pair<ulong, int>& p1, const std::pair<ulong, int> & p2) {
            return p1.second < p2.second;
        });
}

std::pair<ulong, int> PathGroup::getHighestFrequencyEntry() const {
    // Max element return an iteratior, hence dereference.
    return *std::max_element(std::begin(frqs), std::end(frqs),
        [] (const std::pair<ulong, int>& p1, const std::pair<ulong, int> & p2) {
            return p1.second < p2.second;
        });
}


void PathGroup::deletePathsBelowThreshold()
{
    std::vector<const Path*> saved_paths; // Not deleted paths (saved paths).
    saved_paths.reserve(pig_.size());     // Reserve space for all paths.

    for (const Path* pp : pig_) {            // Iterate over paths in group.
        if (frqs.count(pp->length()) != 0) { // Paths path length is still present in map.
            saved_paths.push_back(pp);       // Keep the path (higer plf than threshold).
        }
    }

    pig_ = saved_paths; // Remember only saved paths.
}
