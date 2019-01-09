#include <PathGroup.hpp>

#include <iostream>
#include <algorithm>

 PathGroup::PathGroup(std::vector<const Path*>::const_iterator begin,
        std::vector<const Path*>::const_iterator end)
        : pig_(begin, end)
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
    for (const auto pp : pg.pig_) {
        s << '[' << pp->length() << ']' << ' ';
        if (++i % 5 == 0) { // Print out new line every now and then.
            s << '\n'; 
        }
    }
    return s;
}


static void deleteAtIndices(std::vector<const Path*>& data,
        const std::vector<int>& deleteIndices);

void PathGroup::discardNotFrequent() {
    int highest_plf = getHighestFrequencyEntry().second;
    int threshold_plf = highest_plf / 2; // If path has frequency less than this, erase it.
    if (threshold_plf == 0) return; // Speed return since no paths will be removed.

    std::vector<int> deleteIndices; // Indices of paths that should be removed from group.
    for (int i = 0, n = static_cast<int>(pig_.size()); i < n; i++) { // Iterate over paths.
        ulong path_length = pig_[i]->length(); // Get path length for current path.
        int plf = frqs[path_length]; // Get path length frequency of length of current path.

        if (plf < threshold_plf) { // Path length frequency is lower than threshold.
            deleteIndices.push_back(i); // Remember wich path to remove from group.
            frqs.erase(path_length); // Remove this path length from the map (if exists).
        }
    }

#ifdef DEBUG
    std::cout << "Threshold path length frequency: " << threshold_plf <<
        "\n  Removing " << deleteIndices.size() << " path from group." << std::endl;
#endif

    deleteAtIndices(pig_, deleteIndices); // Delete paths at found indices.
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


/** Deletes path ponters in the provided data vector at the indices provided
 * in the deleteIndices vector. */
static void deleteAtIndices(std::vector<const Path*>& data,
        const std::vector<int>& deleteIndices)
{
    std::vector<bool> markedElements(data.size(), false);
    std::vector<const Path*> tempBuffer;
    tempBuffer.reserve(data.size()-deleteIndices.size());

    for (std::vector<int>::const_iterator itDel = deleteIndices.begin();
            itDel != deleteIndices.end();
            itDel++) {
        markedElements[*itDel] = true;
    }

    for (size_t i=0; i<data.size(); i++) {
        if (!markedElements[i]) tempBuffer.push_back(data[i]);
    }
    data = tempBuffer;
}
