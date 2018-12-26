//
// Created by lirfu on 14.12.18..
//

#include <OverlapGraph.hpp>
#include <Utils.hpp>
#include <MonteCarloHeuristic.hpp>
#include <iostream>

float MonteCarloHeuristic::getMetric(const OverlapGraph::Edge &e, const Utils::Metrics &metric) const {
    switch (metric) {
        case Utils::Metrics::EXTENSION_SCORE:
            return e.extension_score;
        case Utils::Metrics::OVERLAP_SCORE:
            return e.overlap_score;
    }
}

MonteCarloHeuristic::MonteCarloHeuristic(const OverlapGraph &g, int repeat_num, const Utils::Metrics &metric) {
    std::srand(42);

    // For each anchor-node as starting point.
    for (const OverlapGraph::Node &start_node : g.nodes_) {
#ifdef DEBUG
        std::cout << "MCHeur node: " << start_node.index << std::endl;
#endif
        // Skip read-nodes.
        if (!start_node.anchor) {
            continue;
        }

        // Used for monitoring the number of repeated path re-build attempts.
        // Blocks the possibility of +oo loops when re-building paths.
        // After N attempts, path is just dropped.
        int rebuilds = 0;

        // Repeat path building from this starting point.
        for (int i = 0; i < repeat_num; i++) {
            Path p;
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
                        all_ok = false;
                        break;
                    } else {
// TODO Read-node is a dead-end, so trace back the stack. Recurse up the stack until an alternate path is found.

                        if (rebuilds < REBUILD_ATTEMPTS) {
                            // Retry path building.
                            ++rebuilds;
                            --i;
                        }
                        // Drop the path.
                        all_ok = false;
                        break;
                    }
                }

                // Sum-up the metric values.
                float sum = 0;
                for (const OverlapGraph::Edge &e : n->edges) {
                    if (!Utils::contains(p.edges_, &e)) { // Skip visited edges.
                        sum += getMetric(e, metric);
                    }
                }

                // Dead-end detected, drop the path.
                if (sum == 0) {
                    all_ok = false;
                    break;
                }

                // Select a random number from [0, sum]
                float random = std::rand() * sum;

                // Find the selected edge.
                const OverlapGraph::Edge *edge = nullptr;
                sum = 1e-9;
                for (const OverlapGraph::Edge &e : n->edges) {
                    if (!Utils::contains(p.edges_, &e) && sum >= random) { // Skip visited edges.
                        edge = &e;
                    }
                    sum += getMetric(e, metric);
                }

                // Add the selected edge.
                p.edges_.push_back(edge);

                // Replace current node with target node and add it to nodes.
                n = &edge->n2;
                p.nodes_.push_back(n);

                // If target node is anchor, add the node and break.
                if (n->anchor) {
                    break;
                }
            }

            if (all_ok) {
                rebuilds = 0;
                paths_.push_back(p);
#ifdef DEBUG
                std::cout << "MCHeur created path#" << paths_.size() << ": " << p.str() << std::endl;
#endif
            }
        }
    }

}
