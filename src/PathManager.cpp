#include <PathManager.hpp>

#include <iostream>
#include <iomanip>
#include <bitset>
#include <set>
#include <random>

#include <PathWindow.hpp>


void PathManager::buildMonteCarlo(const OverlapGraph &g, const Utils::Metrics &metric) {
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(0., 1.);

    std::cout << "> Monte Carlo heuristic: " << Utils::getMetricName(metric) << std::endl;
    ulong found = 0;

    // For each anchor-node as starting point.
    for (const OverlapGraph::Node &start_node : g.nodes_) {
        // Skip read-nodes.
        if (!start_node.anchor) {
            continue;
        }

//        std::cout << "---> Building from node n" << start_node.index << std::endl;

        // Repeat path building from this starting point.
        for (int r = 0; r < params_.rebuild_attempts; r++) {
            const OverlapGraph::Node *n = &start_node;
            Path p;
            std::vector<bool> visited_nodes(g.nodes_.size(), false);
            visited_nodes[n->index] = true;

//            std::cout << "...... Run " << (r + 1) << '/' << REBUILD_ATTEMPTS
//                      << "  (" << found << " found)" << std::endl;

            // Store the starting node.
            p.nodes_.push_back(n);

            // Defines will the path be added (considered).
            bool acceptable = false;

            // Backtrack counter.
            int backtracks = 0;

            // Construct the path.
            while (true) {
                // Sum-up the metric values.
                double sum = 0;
                const OverlapGraph::Edge *anchor_edge = nullptr;
                std::vector<const OverlapGraph::Edge *> appropriate_edges;
                for (const OverlapGraph::Edge &e : n->edges) {
                    const OverlapGraph::Node *q_n = &(g.nodes_[e.q_index]);

                    // If edge leads to anchor different from starting one, force select it.
                    if (q_n->anchor && q_n->index != start_node.index) {
                        anchor_edge = &e;
                        break;
                    }

                    // Skip visited nodes.
                    if (!visited_nodes[q_n->index]) {
                        sum += getMetric(e, metric);
                        appropriate_edges.push_back(&e);
                    }
                }

                const OverlapGraph::Edge *edge = nullptr;
                if (anchor_edge) {  // If points to anchor, use it.
                    edge = anchor_edge;
                } else if (appropriate_edges.empty()) {  // No edges available (dead-end), backtrack.
#ifdef DEBUG
                    std::cout << "No edges available for: \t" << p << '\n';
#endif
                    if (backtracks < params_.backtrack_attempts && p.edges_.size() > 1) {  // Backtrack possible.
                        bool t = false;
                        do {
                            p.nodes_.pop_back();
                            p.edges_.pop_back();
                            for (const OverlapGraph::Edge &e : p.nodes_.back()->edges) {
                                if (!visited_nodes[e.q_index]) {
                                    // Stop backing if you find an edge leading to unvisited node.
                                    t = true;
                                    break;
                                }
                            }
                        } while (!p.edges_.empty() && !t);

                        if (p.edges_.empty()) {
#ifdef DEBUG
                            std::cout << "Nothing to backtrack to: " << p << '\n';
#endif
                            break;
                        }
                        ++backtracks;

                        n = p.nodes_.back();
                        continue;
                    }
                    break;
                } else { // Select one at random.
                    // Select a random number from [0, sum]
                    double random = dis(gen) * sum;

                    // Find the selected edge.
                    sum = 0;
                    for (const OverlapGraph::Edge *e : appropriate_edges) {
                        sum += getMetric(*e, metric);
                        if (sum >= random) { // Node found.
                            edge = e;
                            break;
                        }
                    }
                }

                // Replace current node with next node.
                n = &(g.nodes_[edge->q_index]);

                // Add the selected edge and next node to path.
                p.nodes_.push_back(n);
                p.edges_.push_back(edge);
                visited_nodes[n->index] = true;

                // If node is anchor, add the node and break.
                if (n->anchor) {
                    acceptable = true; // Accept path.
                    break;
                }

                // Abort if length is too large.
                if (p.nodes_.size() >= params_.node_num_threshold) {
#ifdef DEBUG
                    std::cout << "Length too large (" << p.nodes_.size() << "): " << p << '\n';
#endif
                    break;
                }
            }

            // Path ready to be added.
            if (acceptable) {
                p.updateLength();
                paths_.push_back(p);
                ++found;
#ifdef DEBUG
                std::cout << "Found path #" << paths_.size() << " of " << p.nodes_.size() << " nodes and "
                          << p.length() << " paths: " << p << '\n';
#endif
            }
#ifdef DEBUG
            else {
                std::cout << "No path found!\n";
                p.updateLength();
                std::cout << "Aborted paths' length: " << p.length() << "bp, "
                          << p.nodes_.size() << " nodes.\n";
            }
#endif
        }
    }

    std::cout << "Found " << found << " paths." << std::endl;
#ifdef DEBUG
    //filterUnique();
    //std::cout << "Total new: " << (paths_.size() - orig_size) << std::endl;
#endif
}

void PathManager::buildDeterministic(const OverlapGraph &g,
                                     const Utils::Metrics &metric) {
    const size_t num_nodes = g.nodes_.size();
    ulong found = 0;
    std::cout << "> Deterministic heuristic: " << Utils::getMetricName(metric) << std::endl;
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
            const OverlapGraph::Node *node = &g.nodes_[first_edge.q_index];

            visited_nodes[node->index] = true;
            path.nodes_.push_back(node);
            // If second node was already an anchor, the path of length 2 is built
            if (node->anchor) {
                path.updateLength();
                paths_.push_back(path);
                ++found;
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
            bool went_back = false;

            // Construct the path
            while (true) {
                // Dead end, go back a step and try a different path
                if (node->edges.empty()) {
                    if (step_index == 0) {
                        // Cannot go back a step, drop this path
                        all_ok = false;
                        break;
                    } else {
                        // we will only go back a step once
                        if (went_back) {
                            all_ok = false;
                            break;
                        }
                        went_back = true;

                        // Go back a step
                        step_index -= 1;
                        skip_n_best += 1;
                        path.nodes_.pop_back();
                        path.edges_.pop_back();
                        node = path.nodes_.back();
                        visited_nodes[node->index] = false;
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

                    // we will only go back a step once
                    if (went_back) {
                        all_ok = false;
                        break;
                    }
                    went_back = true;

                    step_index -= 1;
                    skip_n_best += 1;
                    path.nodes_.pop_back();
                    path.edges_.pop_back();
                    node = path.nodes_.back();
                    visited_nodes[node->index] = false;
                    continue;
                }

                // Sort (descending) edges by provided metric
                std::vector<OverlapGraph::Edge> sorted_edges(node->edges);
                std::stable_sort(
                        sorted_edges.begin(),
                        sorted_edges.end(),
                        [&](const OverlapGraph::Edge &a, const OverlapGraph::Edge &b) -> bool {
                            return Utils::getMetric(a, metric) > Utils::getMetric(b, metric);
                        }
                );

                // Find edge by skipping n best
                bool edge_found = false;
                const OverlapGraph::Edge *edge = nullptr;
                int this_step_skips = skip_n_best;
                while (this_step_skips < node->edges.size() - 1) {
                    edge = &sorted_edges[this_step_skips];
                    // Get next node
                    const OverlapGraph::Node *nn = &g.nodes_[edge->q_index];

                    // Node was not visited yet, break the loop
                    if (!visited_nodes[nn->index]) {
                        edge_found = true;
                        break;
                    } else {
                        // skip to next edge in this step
                        this_step_skips += 1;
                    }
                }

                // Yet again there are no available edges, go back a step
                if (!edge_found) {
                    if (step_index == 0) {
                        // Cannot go back a step, drop this path
                        all_ok = false;
                        break;
                    }

                    // we will only go back a step once
                    if (went_back) {
                        all_ok = false;
                        break;
                    }
                    went_back = true;

                    step_index -= 1;
                    skip_n_best += 1;
                    path.nodes_.pop_back();
                    path.edges_.pop_back();
                    node = path.nodes_.back();
                    visited_nodes[node->index] = false;
                    continue;
                }

                // Finally, the edge was found
                bool pushed = false;
                for (const OverlapGraph::Edge &n_edge: node->edges) {
                    if (n_edge.q_index == edge->q_index && n_edge.t_index == edge->t_index) {
                        path.edges_.push_back(&n_edge);
                        pushed = true;
                        break;
                    }
                }

                if (!pushed) {
                    std::cout << "ERROR: no edge found!" << std::endl;
                    return;
                }

                // Go to next node
                visited_nodes[node->index] = true;
                node = &g.nodes_[edge->q_index];
                path.nodes_.push_back(node);
#ifdef DEBUG
                if (visited_nodes[node->index]) {
                    std::cout << "WARNING: duplicate node inserted!";
                }
#endif
                // If target node is anchor, add the node and break.
                if (node->anchor) {
                    break;
                }

                step_index += 1;
            }

            if (all_ok) {
                path.updateLength();
                paths_.push_back(path);
                ++found;
            }
        }
    }
    std::cout << "Found " << found << " paths." << std::endl;
#ifdef DEBUG
    std::cout << "Validating paths" << std::endl;

    for (const Path &p : paths_) {
        size_t num_nodes = p.nodes_.size();
        size_t num_edges = p.edges_.size();

        std::vector<bool> duplicates(g.nodes_.size(), false);

        if (num_nodes != (num_edges + 1)) {
            std::cout <<  "invalid num_nodes: " << num_nodes << ", num_edges: " << num_edges << std::endl;
        }

        for (int i = 0, j = 0; i < num_nodes && j < num_edges; i++, j++) {
            if (duplicates[p.nodes_[i]->index]) {
                std::cout << "found duplicate!" << std::endl;
            }

            duplicates[p.nodes_[i]->index] = true;

            if (p.nodes_[i]->index != p.edges_[j]->t_index) {
                std::cout <<  "t_index is invalid, i=" << i << std::endl;
            }

            if (i + 1 < num_nodes) {
                if(p.nodes_[i + 1]->index != p.edges_[j]->q_index) {
                    std::cout <<  "q_index is invalid, i=" << i  << std::endl;
                }
            }
            if (i == 0) {
                if(!(p.nodes_[i]->anchor)) {
                    std::cout <<  "start anchor is invalid" << std::endl;
                }
            }
            if (i == num_nodes - 1) {
                if(!(p.nodes_[i + 1]->anchor)) {
                    std::cout <<  "end anchor is invalid" << std::endl;
                }
            }
        }
    }
#endif
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

    str << "Paths" << '\n'
        << "- total_num: " << paths_.size() << '\n'
        << "-   min_len: " << min_len << '\n'
        << "-   max_len: " << max_len << '\n'
        << "-   avg_len: " << (paths_.size() > 0 ? sum_len / paths_.size() : 0) << std::endl;

    return str.str();
}

std::tuple<ulong, ulong, ulong> PathManager::getMinMaxSumPathLength() {
    ulong min_len = ULONG_MAX, max_len = 0, sum_len = 0;

#ifdef DEBUG
    size_t neg = 0, pos = 0;
#endif

    for (const Path &p : paths_) { // Iterate over all paths.

#ifdef DEBUG
        if (p.length() < 0) {
            std::cout << "NEG: " << p.length() << std::endl;
            neg++;
        } else {
            std::cout << "POSITIVE: " << p.length() << std::endl;
            pos++;
        }
#endif

        ulong l = p.length();
        sum_len += l;
        if (min_len > l) {
            min_len = l;
        }
        if (max_len < l) {
            max_len = l;
        }
    }

#ifdef DEBUG
    std::cout << "Number of negative path lengths: " << neg << '\n'
              << "Number of positive path lengths: " << pos << std::endl;
#endif

    return std::tuple<ulong, ulong, ulong>(min_len, max_len, sum_len);
}


std::vector<ulong> getBorderPathLengths(const std::vector<PathWindow> &pws,
                                        float ratio_threshold);

std::vector<PathGroup> PathManager::constructGroups(std::vector<const Path *> &v, PathManager::Parameters params) {
    // Get min and max path lengths.
    std::pair<ulong, ulong> mm = getMinMaxPathLength(v);
    ulong min_len = mm.first;
    ulong max_len = mm.second;

    // Sort paths for those two anchors in ascending order.
    std::sort(v.begin(), v.end(),
              [](const Path *a, const Path *b) {
                  return a->length() < b->length();
              });

    // Create path groups.
    std::vector<PathGroup> pgs;
    if (max_len - min_len < params.len_threshold) { // If all paths go in one group.
        // Insert all elements from 'v' into first (and only) group.
        pgs.emplace_back(v.begin(), v.end());
    } else {
        // Create path windows.
        std::vector<PathWindow> pws;
        for (ulong lower = min_len, upper = lower + params.window_size;
             lower <= max_len;
             lower = upper, upper += params.window_size) {
            // Create a window with paths in the [lower, upper> range.
            pws.emplace_back(lower, upper, v);
            if (pws.back().getSumFreqs() == 0) { // If window is empty.
                pws.pop_back();                  // Remove empty window.
            }
        }
#ifdef DEBUG
        std::cout << "Created " << pws.size() << " non-empty path windows.\n"
              << " [(Path length with lowest FQ, FQ), (Path length with highest FQ, FQ)]"
              << " => Lengths of paths inside the window\n";
        for (size_t i = 0, n = pws.size(); i < n; i++) {
            std::cout << "  window " << std::setw(2) << i << " [("
              << std::setw(6) << pws[i].getLowestFrequencyEntry().first << ','
              << std::setw(1) << pws[i].getLowestFrequencyEntry().second << ") ("
              << std::setw(6) << pws[i].getHighestFrequencyEntry().first << ','
              << std::setw(1) << pws[i].getHighestFrequencyEntry().second << ")] "
              << " => " << pws[i] << '\n';
        }
#endif

        // Get path lengths that divide all paths into groups.
        std::vector<ulong> bs = getBorderPathLengths(pws, params.ratio_threshold);
#ifdef DEBUG
        if (bs.size()) {
            std::cout << "Dividing path lengths: ";
            for_each(bs.begin(), bs.end(), [] (ulong el) {std::cout << el << ' ';});
            std::cout << std::endl;
        } else std::cout << "No dividing path lengths found!" << std::endl;
#endif
        if (bs.empty()) { // No dividing path lengths has been found.
            // Insert all elements from 'v' into first (and only) group.
            pgs.emplace_back(v.begin(), v.end());
        } else { // Dividing path lengths exist.
            // Start of the group (inclusve) and end of the group (exclusive).
            std::vector<const Path *>::const_iterator begin = v.begin();
            std::vector<const Path *>::const_iterator end;

            // Iterate over borders (dividing path lengths).
            for (ulong cur_border : bs) {
                // Find first outside the border.
                for (end = begin + 1; (*end)->length() < cur_border; end++);

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

std::vector<ulong> getBorderPathLengths(const std::vector<PathWindow> &pws,
                                        float ratio_threshold) {
    // Path lengths used to divide the set into groups.
    std::vector<ulong> dividing_path_lengths;
    dividing_path_lengths.reserve(10); // Reserve space to prevent some copies.

    for (size_t i = 1, n = pws.size(); i < n - 1; i++) {
        // Shortcut references: left(l), center/current(c) and right(r) window.
        const PathWindow &l = pws[i - 1], &c = pws[i], &r = pws[i + 1];

        // Calculate valley and peak windows. 
        const PathWindow &valley = std::min({l, c, r},
                                            [](const PathWindow &a, const PathWindow &b) {
                                                return a.getSumFreqs() < b.getSumFreqs();
                                            });
        const PathWindow &peak = std::max({l, c, r},
                                          [](const PathWindow &a, const PathWindow &b) {
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

    // Return only unique path lengths (eliminate edge cases where).
    std::set<ulong> s(dividing_path_lengths.begin(), dividing_path_lengths.end());
    dividing_path_lengths.assign(s.begin(), s.end());

    return dividing_path_lengths;
}

std::map<std::pair<const OverlapGraph::Node *, const OverlapGraph::Node *>,
        std::vector<const Path *>>
PathManager::getPathsBetweenAnchors() {
    // Map containing all paths between to anchors: [anchor1, achor2] => {path1, path2,...}
    std::map<std::pair<const OverlapGraph::Node *, const OverlapGraph::Node *>,
            std::vector<const Path *>> paths_between_anchors;

    // Iterate over paths and add each to entry for its anchors.
    for (const Path &p : paths_) {
        std::pair<const OverlapGraph::Node *, const OverlapGraph::Node *> anchors
                = {p.nodes_.front(), p.nodes_.back()};
        if (paths_between_anchors.count(anchors)) { // Entry exists for those two anchors.
            paths_between_anchors[anchors].push_back(&p);
        } else { // Entry for those two anchors does not exist in the map - create it.
            paths_between_anchors[anchors] = std::vector<const Path *>(1, &p);
        }
    }

    return paths_between_anchors;
}


std::pair<ulong, ulong> PathManager::getMinMaxPathLength(std::vector<const Path *> &v) {
    ulong min_len = ULONG_MAX, max_len = 0;

    for (const Path *p : v) { // Iterate over all paths.
        ulong l = p->length();
        if (min_len > l) {
            min_len = l;
        }
        if (max_len < l) {
            max_len = l;
        }
    }

    return {min_len, max_len};
}

Path PathManager::constructConsensusPath(
        const std::map<std::pair<const OverlapGraph::Node *, const OverlapGraph::Node *>,
                std::vector<const Path *>> &paths_between_contigs,
        std::map<std::pair<const OverlapGraph::Node *,
                const OverlapGraph::Node *>, const Path *> &consensus_paths, ulong min_path_num) {
    std::map<std::pair<const OverlapGraph::Node *, const OverlapGraph::Node *>, std::pair<const Path *, ulong>>
            filtered;
    std::vector<const OverlapGraph::Node *> nodes;
    ulong max_path_num = 0;
    std::pair<const OverlapGraph::Node *, const OverlapGraph::Node *> max_path_key;

    for (auto &pair:paths_between_contigs) {
        if (!consensus_paths[pair.first]) continue;
        // Filter out small ones.
        ulong path_num = pair.second.size();
        if (path_num >= min_path_num) {
            // Memorize unique nodes.
            if (!Utils::contains(nodes, pair.first.first)) {
                nodes.push_back(pair.first.first);
            }
            if (!Utils::contains(nodes, pair.first.second)) {
                nodes.push_back(pair.first.second);
            }
            // Add to filtered.
            filtered[pair.first] = {consensus_paths[pair.first], path_num};
            // Find max.
            if (max_path_num < path_num) {
                max_path_num = path_num;
                max_path_key = pair.first;
            }
        }
    }

    // Most often path is the seed.
    Path scaffold = *consensus_paths[max_path_key];

    const OverlapGraph::Node *left_anchor = max_path_key.first;
    const OverlapGraph::Node *right_anchor = max_path_key.second;

    // Don't repeat visited nodes.
    std::vector<const OverlapGraph::Node *> visited;
    visited.push_back(max_path_key.first);
    visited.push_back(max_path_key.second);


    std::pair<const OverlapGraph::Node *, const OverlapGraph::Node *> pair;
    // For each remaining consensus
    while (!filtered.empty()) {
        // Find best extension of scaffold.
        bool left = false, found = false;
        max_path_num = 0;
        for (const OverlapGraph::Node *n: nodes) {
            if (!Utils::contains(visited, n)) { // Don't visit twice.
                // Check left.
                pair = {n, left_anchor};
                if (max_path_num < filtered[pair].second) {
                    max_path_num = filtered[pair].second;
                    max_path_key = pair;
                    left = true;
                    found = true;
                }

                // Check right.
                pair = {right_anchor, n};
                if (max_path_num < filtered[pair].second) {
                    max_path_num = filtered[pair].second;
                    max_path_key = pair;
                    left = false;
                    found = true;
                }
            }
        }

        // If none is found, end.
        if (!found) {
            break;
        }

        // Extend it on correct side.
        const Path *p = filtered[max_path_key].first;
        if (left) {
            scaffold.nodes_.push_back(p->nodes_[0]);
            for (long i = 0; i < p->edges_.size(); i++) {
                scaffold.nodes_.push_back(p->nodes_[i + 1]);
                scaffold.edges_.push_back(p->edges_[i]);
            }
        } else {
            scaffold.nodes_.insert(scaffold.nodes_.begin(), p->nodes_[p->nodes_.size() - 1]);
            for (long i = p->edges_.size() - 1; i >= 0; i--) {
                scaffold.nodes_.insert(scaffold.nodes_.begin(), p->nodes_[i]);
                scaffold.edges_.insert(scaffold.edges_.begin(), p->edges_[i]);
            }
        }

        // Remove this consensus.
        filtered.erase(max_path_key);
    }

    return scaffold;
}
