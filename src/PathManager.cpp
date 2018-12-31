//
// Created by lirfu on 26.12.18..
//

#include <cmath>
#include <OverlapGraph.hpp>
#include <Utils.hpp>
#include <PathManager.hpp>
#include <iostream>
#include <climits>

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

std::vector<PathGroup> PathManager::constructGroups() {
    std::vector<PathGroup> path_groups;

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

    if (max_len - min_len < LEN_THRESHOLD) { // Check if all paths should go in one group.
        path_groups.emplace_back(min_len, max_len + 1ul, v); // Add one because upper is exclusive.
    } else {
        for (ulong lower = min_len, upper = lower + WINDOW_SIZE;
                lower <= max_len;
                lower = upper, upper += WINDOW_SIZE) {
            path_groups.emplace_back(lower, upper, v);
        }
    }

    return path_groups;
}

PathGroup::PathGroup(ulong l, ulong u, const std::vector<const Utils::Path*>& sp)
        : lower(l), upper(u) {
    for (size_t i = 0, n = sp.size(); i < n; i++) {
        if (sp[i]->length() >= upper) break; // Passed upper limit.
        if (sp[i]->length() >= lower) pig_.emplace_back(sp[i]);
    }
}
