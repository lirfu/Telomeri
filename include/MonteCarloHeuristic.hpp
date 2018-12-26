//
// Created by lirfu on 14.12.18..
//

#ifndef TELOMERI_HEURISTICMC_H
#define TELOMERI_HEURISTICMC_H

#include "PathBuilder.hpp"
#include <OverlapGraph.hpp>
#include <Utils.hpp>

#define REBUILD_ATTEMPTS 3

class MonteCarloHeuristic : public PathBuilder {
private:
public:
    MonteCarloHeuristic(const OverlapGraph &g, int repeat_num, const Utils::Metrics &metric);

    float getMetric(const OverlapGraph::Edge &e, const Utils::Metrics &metric) const;
};


#endif //TELOMERI_HEURISTICMC_H
