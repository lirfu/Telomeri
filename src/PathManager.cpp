#include <cmath>
#include <OverlapGraph.hpp>
#include <Utils.hpp>
#include <PathManager.hpp>
#include <iostream>
#include <climits>
#include <map>

void PathManager::buildMonteCarlo(const OverlapGraph &g, int repeat_num, const Utils::Metrics &metric) {
    std::srand(42);

    // For each anchor-node as starting point.
    for (const OverlapGraph::Node &start_node : g.nodes_) {
        // Skip read-nodes.
        if (!start_node.anchor) {
            continue;
        }

        // Used for monitoring the number of path re-build attempts (dead-ends).
        // Blocks the possibility of +oo loops when re-building paths.
        // After N attempts, path is just dropped.
        int rebuilds = 0;

        // Repeat path building from this starting point.
        for (int i = 0; i < repeat_num; i++) {
            Utils::Path p;
            const OverlapGraph::Node *n = &start_node;

            // Store the starting node.
            p.nodes_.push_back(n);

            // Defines will the path be added (considered).
            bool all_ok = true;

            // Construct the path.
            while (true) {
                // Dead end.
                if (n->edges.size() == 0) {
                    if (n->anchor) {
                        // This is just a lonely anchor, drop it as it doesn't represent a path.
#ifdef DEBUG
                        std::cout << "Dead anchor: " << p.str() << std::endl;
#endif
                        all_ok = false;
                        break;
                    } else {
#ifdef DEBUG
                        std::cout << "Dead read: " << p.str() << std::endl;
#endif
                        if (rebuilds < REBUILD_ATTEMPTS) {
                            // Retry path building.
#ifdef DEBUG
                            std::cout << "Retry building." << std::endl;
#endif
                            ++rebuilds;
                            --i;
                        }
                        // Drop the path.
                        all_ok = false;
                        break;
                    }
                }

                // Sum-up the metric values.
                double sum = 0;
                for (const OverlapGraph::Edge &e : n->edges) {
                    if (!Utils::contains(p.edges_, &e)) { // Skip visited edges.
                        if (&e == nullptr) {
                            std::cout << n << std::endl;
                            std::cout << n->name << std::endl;
                            std::cout << n->anchor << std::endl;
                            std::cout << n->edges.size() << std::endl;
                        }
                        sum += getMetric(e, metric);
                    }
                }

                // Dead-end detected, drop the path.
                if (sum == 0) {
#ifdef DEBUG
                    std::cout << "Metrics sum to 0: " << p.str() << std::endl;
#endif
                    all_ok = false;
                    break;
                }

                // Select a random number from [0, sum]
                double random = std::rand() / (double) RAND_MAX * sum;

                // Find the selected edge.
                const OverlapGraph::Edge *edge = nullptr;
                sum = 0;
                for (const OverlapGraph::Edge &e : n->edges) {
                    if (!Utils::contains(p.edges_, &e)) { // Skip visited edges.
                        sum += getMetric(e, metric);

                        if (sum >= random) { // Node found.
                            edge = &e;
                            break;
                        }
                    }
                }

                // Add the selected edge.
                p.edges_.push_back(edge);

                // Replace current node with target node and add it to nodes.
                n = &g.nodes_[(n->index == edge->n2) ? edge->n1 : edge->n2];
                p.nodes_.push_back(n);

                // If target node is anchor, add the node and break.
                if (n->anchor) {
#ifdef DEBUG
                    std::cout << "New anchor found, path finished: " << p.str() << std::endl;
#endif
                    break;
                }
            }

            if (all_ok) {
                rebuilds = 0;
                paths_.push_back(p);
#ifdef DEBUG
                std::cout << "Added path #" << paths_.size() << ": " << p.str() << std::endl;
#endif
            }
        }
    }
}

void PathManager::buildDeterministic(const OverlapGraph &g, const Utils::Metrics &metric) {
    // TODO
}

void PathManager::filterUnique() {
    auto last = std::unique(paths_.begin(), paths_.end(), Utils::Path::equals);
    paths_.erase(last, paths_.end()); // Reallocate vector and remove undefined empty slots.
}

std::string PathManager::stats() {
    std::stringstream str;

    std::tuple<ulong, ulong, ulong> mms = getMinMaxSumPathLength();
    ulong min_len = std::get<0>(mms);
    ulong max_len = std::get<1>(mms);
    ulong sum_len = std::get<2>(mms);

    str << "Paths"                                    << '\n'
        << "- total_num: " << paths_.size()           << '\n'
        << "-   min_len: " << min_len                 << '\n'
        << "-   max_len: " << max_len                 << '\n'
        << "-   avg_len: " << sum_len / paths_.size() << std::endl;

    return str.str();
}

std::tuple<ulong, ulong, ulong> PathManager::getMinMaxSumPathLength() {
    ulong min_len = ULONG_MAX, max_len = 0, sum_len = 0;
    for (const Utils::Path &p : paths_) {
        ulong l = p.length();
        sum_len += l;
        if (min_len > l) {
            min_len = l;
        }
        if (max_len < l) {
            max_len = l;
        }
    }
    return std::tuple<ulong, ulong, ulong>(min_len, max_len, sum_len);
}


class PathWindow {
private:
    ulong lower; //< Lower path length bound (inclusive).
    ulong upper; //< upper path length bound (exclusive).
    std::vector<const Utils::Path*> piw_; //< Pointers to paths in window.
    std::map<ulong, int> frqs; //< Path length frequencies.
    int sum_frqs;
public:
    /** Constructs a path window with paths that have paths lengths between
     *  lower (inclusive) and upper (exclusive) bound.
     *  @param l  Lower path length bound for this path group (inclusive).
     *  @param u  Upper path length bound for this path group (exclusive).
     *  @param sp Sorted pointers to paths according to path lenght in
     *            ascending order.
     * */
    PathWindow(ulong l, ulong u, const std::vector<const Utils::Path*>& sp)
            : lower(l), upper(u), sum_frqs(0) {
        for (size_t i = 0, n = sp.size(); i < n; i++) {
            if (sp[i]->length() >= u) break; // Passed upper limit.
            if (sp[i]->length() >= l) {
                piw_.emplace_back(sp[i]);
                
                // If this is first time seeing this path length, set it to one. Otherwise increment.
                frqs[sp[i]->length()] = frqs.count(sp[i]->length()) == 0 ?
                    1 : frqs[sp[i]->length()] + 1;

                // Increase total number of paths in this window (sum frequencies).
                sum_frqs++;
            }
        }
    }

    friend std::ostream& operator<< (std::ostream& s, const PathWindow& pw) {
        for (const auto pp : pw.piw_) {
            s << pp->str() << '[' << pp->length() << ']' << ' ';
        }
        return s;
    }

    /** Returns a <PathLength, Frequency> pair for which has lowest frequency in the frqs map. */
    std::pair<ulong, int> getLowestFrequencyEntry() const {
        return *std::min_element(std::begin(frqs), std::end(frqs), // Returns an iterator, hence dereference.
            [] (const std::pair<ulong, int>& p1, const std::pair<ulong, int> & p2) {
                return p1.second < p2.second;
            });
    }

    /** Returns a <PathLength, Frequency> pair for which has highest frequency in the frqs map. */
    std::pair<ulong, int> getHighestFrequencyEntry() const {
        return *std::max_element(std::begin(frqs), std::end(frqs), // Returns an iterator, hence dereference.
            [] (const std::pair<ulong, int>& p1, const std::pair<ulong, int> & p2) {
                return p1.second < p2.second;
            });
    }
    
    ulong getLowerBound() const {return lower;}
    ulong getUpperBound() const {return upper;}
    int getSumFreqs() const {return sum_frqs;}
};


std::vector<ulong> getBorderPathLengths(const std::vector<PathWindow>& pws, float ratio_threshold);
std::vector<PathGroup> PathManager::constructGroups() {
    // Get min and max path lengths.
    std::tuple<ulong, ulong, ulong> mms = getMinMaxSumPathLength();
    ulong min_len = std::get<0>(mms);
    ulong max_len = std::get<1>(mms);

    // Create a sorted view (ascending) over paths_ array.
    std::vector<const Utils::Path*> v(paths_.size(), nullptr);
    for (size_t i = 0, n = v.size(); i < n; i++) {
        v[i] = &paths_[i];
    }
    std::sort(v.begin(), v.end(),
            [](const Utils::Path* a, const Utils::Path* b) {
                return a->length() < b->length();
            });

    // Create path groups.
    std::vector<PathGroup> pgs;
    if (max_len - min_len < LEN_THRESHOLD) {  // Check if all paths should go in one group.
        pgs.emplace_back(v.begin(), v.end()); // Insert all elements from 'v' into first (and only) group.
    } else {
        // Create path windows.
        std::vector<PathWindow> pws;
        for (ulong lower = min_len, upper = lower + WINDOW_SIZE;
                lower <= max_len;
                lower = upper, upper += WINDOW_SIZE) {
            pws.emplace_back(lower, upper, v);         // Create a window with paths in the [lower, upper> range.
            if ((pws.end() - 1)->getSumFreqs() == 0) { // Check if window is empty.
                pws.pop_back();                        // Remove empty window.
            }
        }
//#ifdef DEBUG
        //std::cout << "Created " << pws.size() << " non-empty path windows.\n";
        //for (size_t i = 0, n = pws.size(); i < n; i++) {
            //std::cout << "  window " << i << " [" << pws[i].getLowerBound() << ','
                //<< pws[i].getUpperBound() << "]: " << pws[i] << '\n'; 
        //}
//#endif

        // Get path lengths that divide all paths into groups.
        std::vector<ulong> bs = getBorderPathLengths(pws, RATIO_THRESHOLD);
#ifdef DEBUG
        std::cout << "Dividing path lengths: ";
        for_each(bs.begin(), bs.end(), [] (ulong el) {std::cout << el << ' ';});
        std::cout << std::endl << std::endl;
#endif
        if (bs.empty()) { // No dividing path lengths has been found. All paths go into same group.
            pgs.emplace_back(v.begin(), v.end()); // Insert all elements from 'v' into first (and only) group.
        } else {
            std::vector<const Utils::Path*>::const_iterator begin = v.begin(); // Start of the group (inclusve).
            std::vector<const Utils::Path*>::const_iterator end;               // End of the group (exclusive).
            for (ulong cur_border : bs) {  // Current border. 
                for (end = begin; (*end)->length() < cur_border; end++); // Find first outside the border.
                pgs.emplace_back(begin, end); // Create group: [PreviousBorder, CurrentBorder>.
                begin = end;                  // Set begining of the next group to current border (current group end).
            }
            pgs.emplace_back(begin, v.end()); // Create final group: [LastBorder, v.end>.
        }
    }
    
    return pgs;
}

std::vector<ulong> getBorderPathLengths(const std::vector<PathWindow>& pws, float ratio_threshold) {
    std::vector<ulong> dividing_path_lengths; // Path lengths used to divide the set into groups.
    dividing_path_lengths.reserve(10);        // Reserve some space to prevent some copies.

    for(size_t i = 1, n = pws.size(); i < n - 1; i++) {
        // Shortcut references to left(l), center/current(c) and right(r) window.
        const PathWindow &l = pws[i-1], &c = pws[i], &r = pws[i+1];

        // Calculate valley and peak windows. 
        const PathWindow& valley = std::min({l, c, r},
                [](const PathWindow& a, const PathWindow& b){
                    return a.getSumFreqs() < b.getSumFreqs();
                });
        const PathWindow& peak = std::max({l, c, r},
                [](const PathWindow& a, const PathWindow& b){
                    return a.getSumFreqs() < b.getSumFreqs();
                });

        // Get <PathLength, Frequency> entries from the valley and peak windows.
        std::pair<ulong, int> valley_pair = valley.getLowestFrequencyEntry();
        std::pair<ulong, int> peak_pair = peak.getHighestFrequencyEntry();

        // If lowest frequency in the valley is 'significantly' smaller than highest frequency in the
        // peak, use the PathLength with the lowest frequency in the valley to divide the set into groups.
        if (valley_pair.second < ratio_threshold * peak_pair.second) {
            dividing_path_lengths.push_back(valley_pair.first);             
        }
    } 
    return dividing_path_lengths;
}

PathGroup::PathGroup(std::vector<const Utils::Path*>::const_iterator begin,
        std::vector<const Utils::Path*>::const_iterator end)
    : pig_(begin, end) {}
