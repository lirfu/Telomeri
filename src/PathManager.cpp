#include <PathManager.hpp>

#include <iostream>

#include <PathWindow.hpp>
#include <bitset>

void PathManager::buildMonteCarlo(const OverlapGraph &g, int paths_num,
                                  const Utils::Metrics &metric) {
    std::srand(40);
    std::cout << "> Monte Carlo heuristic" << std::endl;

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

        std::cout << "---> Building from node n" << start_node.index << std::endl;

        // Repeat path building from this starting point.
        for (int r = 0; r < paths_num; r++) {
            const OverlapGraph::Node *n = &start_node;
            Path p;

            std::cout << "...... Run " << (r + 1) << '/' << paths_num
                      << " rebuild " << rebuilds << '/' << REBUILD_ATTEMPTS << std::endl;

            // Store the starting node.
            p.nodes_.push_back(n);

            // Defines will the path be added (considered).
            bool acceptable = false;

            // Backtrack counter.
            int backtracks = 0;
            std::vector<const OverlapGraph::Node *> backtracked_nodes;

            // Construct the path.
            while (true) {
                // Sum-up the metric values.
                double sum = 0;
                const OverlapGraph::Edge *anchor_edge = nullptr;
                for (const OverlapGraph::Edge &e : n->edges) {
                    // Skip edges in wrong direction and visited nodes.
                    if (e.t_index == n->index
                        && !Utils::contains(p.nodes_, &(g.nodes_[e.q_index]))
                        && !Utils::contains(backtracked_nodes, &(g.nodes_[e.q_index]))) {
                        sum += getMetric(e, metric);
                        // If edge leads to anchor different from starting one, force select it.
                        if (g.nodes_[e.q_index].index != start_node.index && g.nodes_[e.q_index].anchor) {
                            anchor_edge = &e;
                            break;
                        }
                    }
                }

                const OverlapGraph::Edge *edge = nullptr;
                if (anchor_edge) {
                    edge = anchor_edge;
                } else { // Select one at random.
                    if (sum == 0) { // No edges available (dead-end) and anchor isn't found.
#ifdef DEBUG
                        std::cout << "Metric sum is 0: \t" << p << std::endl;
#endif
                        // Backtrack.
                        if (backtracks < BACKTRACK_ATTEMPTS && p.edges_.size() > 1) {
                            do {
                                n = p.nodes_.back();
                                p.nodes_.pop_back();
                                p.edges_.pop_back();
                            } while (!p.edges_.empty() && Utils::contains(backtracked_nodes, n));
                            if (p.edges_.empty()) {
#ifdef DEBUG
                                std::cout << "Nothing to backtrack to!" << std::endl;
#endif
                                break;
                            }
                            backtracked_nodes.push_back(n);
                            ++backtracks;
#ifdef DEBUG
                            std::cout << "Backtrack " << backtracks << "/"
                                      << BACKTRACK_ATTEMPTS << " to: \t" << p << std::endl;
#endif

                            n = p.nodes_.back();
                            p.updateLength();
                            continue;
                        }
                        break;
                    }

                    // Select a random number from [0, sum]
                    double random = std::rand() / (double) RAND_MAX * sum;

                    // Find the selected edge.
                    sum = 0;
                    for (const OverlapGraph::Edge &e : n->edges) {
                        // Skip edges in wrong direction and visited nodes.
                        if (e.t_index == n->index
                            && !Utils::contains(p.nodes_, &(g.nodes_[e.q_index]))
                            && !Utils::contains(backtracked_nodes, &(g.nodes_[e.q_index]))) {
                            sum += getMetric(e, metric);
                            if (sum >= random) { // Node found.
                                edge = &e;
                                break;
                            }
                        }
                    }
                }

                // Replace current node with next node.
                n = &(g.nodes_[edge->q_index]);

                // Add the selected edge and next node to path.
                p.nodes_.push_back(n);
                p.edges_.push_back(edge);

                // If node is anchor, add the node and break.
                if (n->anchor) {
                    acceptable = true; // Accept path.
                    break;
                }

                p.updateLength();
                // Length is too large.
                if ((p.length() >= 0 ? p.length() : -p.length()) > LEN_THRESHOLD) {
#ifdef DEBUG
                    std::cout << "Length too large (" << p.length() << "): " << p << std::endl;
#endif
                    break;
                }

            }

            // Path ready to be added.
            if (acceptable) {
                rebuilds = 0;
                p.updateLength();
                paths_.push_back(p);
                std::cout << "Found path #" << paths_.size() << ": " << p << std::endl;

            } else if (rebuilds < REBUILD_ATTEMPTS) {
                // Retry path building.
                ++rebuilds;
                --r;
#ifdef DEBUG
                std::cout << "Attempt re-build." << std::endl;
#endif
            } else {
                // Stop retrying.
#ifdef DEBUG
                std::cout << "Retry attempts wasted." << std::endl;
#endif
                break;
            }
        }
    }
}

void PathManager::buildDeterministic(const OverlapGraph &g,
                                     const Utils::Metrics &metric) {
    const size_t num_nodes = g.nodes_.size();
#ifdef DEBUG
    int outer = 0;
    int num_anchors = 0;
    for (const OverlapGraph::Node &start_node : g.nodes_) {
        if (start_node.anchor) {
            num_anchors += 1;
        }
    }
#endif
    // For each anchor node as starting point
    for (const OverlapGraph::Node &start_node : g.nodes_) {
        // Skip read-nodes.
        if (!start_node.anchor) {
            continue;
        }
#ifdef DEBUG
        outer += 1;
        int inner = 1;
#endif
        // Construct path for each node connected to start_node
        for (const OverlapGraph::Edge &first_edge : start_node.edges) {
#ifdef DEBUG
            std::cout << "Build [" << outer << " / " << num_anchors << "]-["
                      << inner << " / " << start_node.edges.size() << "]" << std::endl;
            inner += 1;
#endif
            Path path;
            // num_nodes of bools with initial value false
            std::vector<bool> visited_nodes(num_nodes, false);

            // Add first node, first edge and second node to the path
            visited_nodes[start_node.index] = true;
            path.nodes_.push_back(&start_node);
            path.edges_.push_back(&first_edge);
            // Get second node from which the path will be build
            const OverlapGraph::Node *node = &g.nodes_[
                    (start_node.index == first_edge.t_index) ? first_edge.q_index : first_edge.t_index
            ];


            // If second node was already an anchor, the path of length 2 is built
            if (node->anchor) {
                path.updateLength();
                path.nodes_.push_back(node);
                paths_.push_back(path);
                break;
            }

            // Defines will the path be added (considered).
            bool all_ok = true;
            int step_index = 0;
            // Defines how many of best-scoring nodes will be ignored for path construction.
            // 0 = use best-scoring node
            // 1 = use second-best node
            // ...and so on
            // This is used to go back a step when dead end is encountered.
            int skip_n_best = 0;

            // Construct the path
            while (true) {
                // Dead end, go back a step and try a different path
                if (node->edges.empty()) {
                    if (step_index == 0) {
                        // Cannot go back a step, drop this path
                        all_ok = false;
                        break;
                    } else {
                        // Go back a step
                        step_index -= 1;
                        skip_n_best += 1;
                        node = path.nodes_.back();
                        visited_nodes[node->index] = false;
                        path.nodes_.pop_back();
                        path.edges_.pop_back();
                        continue;
                    }
                }

                // If we need to skip all edges, then this is a dead end, go back a step
                if (skip_n_best >= node->edges.size() - 1) {
                    if (step_index == 0) {
                        // Cannot go back a step, drop this path
                        all_ok = false;
                        break;
                    }
                    step_index -= 1;
                    skip_n_best += 1;
                    node = path.nodes_.back();
                    visited_nodes[node->index] = false;
                    path.nodes_.pop_back();
                    path.edges_.pop_back();
                    continue;
                }

                // Sort (descending) edges by provided metric
                std::vector<OverlapGraph::Edge> sorted_edges(node->edges);
                std::stable_sort(
                        sorted_edges.begin(),
                        sorted_edges.end(),
                        [&](const OverlapGraph::Edge& a, const OverlapGraph::Edge& b) -> bool {
                            return Utils::getMetric(a, metric) > Utils::getMetric(b, metric);
                        }
                );

                // Find edge by skipping n best
                bool edge_found = false;
                const OverlapGraph::Edge *edge = nullptr;
                while (skip_n_best < node->edges.size() - 1) {
                    edge = &sorted_edges[skip_n_best];
                    // Get next node
                    const OverlapGraph::Node* nn = &g.nodes_[
                            (node->index == edge->t_index) ? edge->q_index : edge->t_index
                    ];

                    // Node was not visited yet, break the loop
                    if (!visited_nodes[nn->index]) {
                        edge_found = true;
                        break;
                    } else {
                        skip_n_best += 1;
                    }
                }

                // Yet again there are no available edges, go back a step
                if (!edge_found) {
                    if (step_index == 0) {
                        // Cannot go back a step, drop this path
                        all_ok = false;
                        break;
                    }
                    step_index -= 1;
                    skip_n_best += 1;
                    node = path.nodes_.back();
                    visited_nodes[node->index] = false;
                    path.nodes_.pop_back();
                    path.edges_.pop_back();
                    continue;
                }

                // Finally, the edge was found
                // Go to next node
                node = &g.nodes_[(node->index == edge->t_index) ? edge->q_index : edge->t_index];
                visited_nodes[node->index] = true;
                path.edges_.push_back(edge);
                path.nodes_.push_back(node);

                // If target node is anchor, add the node and break.
                if (node->anchor) {
                    break;
                }

                step_index += 1;
            }

            if (all_ok) {
                path.updateLength();
                paths_.push_back(path);
            }
        }
    }
}

void PathManager::filterUnique() {
    // Use equality operator to identify unique paths.
    auto last = std::unique(paths_.begin(), paths_.end());

    // Reallocate vector and remove undefined empty slots.
    paths_.erase(last, paths_.end());
}

std::string PathManager::stats() {
    std::stringstream str;

    std::tuple<ulong, ulong, ulong> mms = getMinMaxSumPathLength();
    ulong min_len = std::get<0>(mms);
    ulong max_len = std::get<1>(mms);
    ulong sum_len = std::get<2>(mms);

    str << "Paths"                          << '\n'
        << "- total_num: " << paths_.size() << '\n'
        << "-   min_len: " << min_len       << '\n'
        << "-   max_len: " << max_len       << '\n'
        << "-   avg_len: " << (paths_.size() > 0 ? sum_len / paths_.size() : 0) << std::endl;

    return str.str();
}

std::tuple<ulong, ulong, ulong> PathManager::getMinMaxSumPathLength() {
    ulong min_len = ULONG_MAX, max_len = 0, sum_len = 0;
    for (const Path &p : paths_) {
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




std::vector<ulong> getBorderPathLengths(const std::vector<PathWindow>& pws,
        float ratio_threshold);
std::vector<PathGroup> PathManager::constructGroups() {
    // Get min and max path lengths.
    std::tuple<ulong, ulong, ulong> mms = getMinMaxSumPathLength();
    ulong min_len = std::get<0>(mms);
    ulong max_len = std::get<1>(mms);

    // Create a sorted view (ascending) over paths_ array.
    std::vector<const Path*> v(paths_.size(), nullptr);
    for (size_t i = 0, n = v.size(); i < n; i++) {
        v[i] = &paths_[i];
    }
    std::sort(v.begin(), v.end(),
            [](const Path* a, const Path* b) {
                return a->length() < b->length();
            });

    // Create path groups.
    std::vector<PathGroup> pgs;
    if (max_len - min_len < LEN_THRESHOLD) {  // If all paths go in one group.
        // Insert all elements from 'v' into first (and only) group.
        pgs.emplace_back(v.begin(), v.end());
    } else {
        // Create path windows.
        std::vector<PathWindow> pws;
        for (ulong lower = min_len, upper = lower + WINDOW_SIZE;
                lower <= max_len;
                lower = upper, upper += WINDOW_SIZE) {
            // Create a window with paths in the [lower, upper> range.
            pws.emplace_back(lower, upper, v);
            if ((pws.end() - 1)->getSumFreqs() == 0) { // If window is empty.
                pws.pop_back();                        // Remove empty window.
            }
        }
//#ifdef DEBUG
        //std::cout << "Created " << pws.size() << " non-empty path windows.\n";
        //for (size_t i = 0, n = pws.size(); i < n; i++) {
            //std::cout << "  window " << i << " [" << pws[i].getLowerBound()
            //  << ',' << pws[i].getUpperBound() << "]: " << pws[i] << '\n';
        //}
//#endif

        // Get path lengths that divide all paths into groups.
        std::vector<ulong> bs = getBorderPathLengths(pws, RATIO_THRESHOLD);
#ifdef DEBUG
        std::cout << "Dividing path lengths: ";
        for_each(bs.begin(), bs.end(), [] (ulong el) {std::cout << el << ' ';});
        std::cout << std::endl << std::endl;
#endif
        if (bs.empty()) { // No dividing path lengths has been found.
            // Insert all elements from 'v' into first (and only) group.
            pgs.emplace_back(v.begin(), v.end());
        } else { // Dividing path lengths exist.
            // Start of the group (inclusve) and end of the group (exclusive).
            std::vector<const Path*>::const_iterator begin = v.begin();
            std::vector<const Path*>::const_iterator end;

            // Iterate over borders (dividing path lengths).
            for (ulong cur_border : bs) {
                // Find first outside the border.
                for (end = begin; (*end)->length() < cur_border; end++);

                // Create group: [PreviousBorder, CurrentBorder>.
                pgs.emplace_back(begin, end);

                // Set begining of next group to current group end.
                begin = end;
            }
            // Create final group: [LastBorder, v.end>.
            pgs.emplace_back(begin, v.end());
        }
    }

    return pgs;
}

std::vector<ulong> getBorderPathLengths(const std::vector<PathWindow>& pws,
        float ratio_threshold) {
    // Path lengths used to divide the set into groups.
    std::vector<ulong> dividing_path_lengths;
    dividing_path_lengths.reserve(10); // Reserve space to prevent some copies.

    for(size_t i = 1, n = pws.size(); i < n - 1; i++) {
        // Shortcut references: left(l), center/current(c) and right(r) window.
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

        // If lowest frequency in the valley is 'significantly' smaller than
        // highest frequency in the peak, use the PathLength with the lowest
        // frequency in the valley to divide the set into groups (dividing path
        // length has been found).
        if (valley_pair.second < ratio_threshold * peak_pair.second) {
            dividing_path_lengths.push_back(valley_pair.first);
        }
    }
    return dividing_path_lengths;
}


