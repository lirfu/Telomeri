//
// Created by lirfu on 14.12.18..
//

#ifndef TELOMERI_HEURISTICMC_H
#define TELOMERI_HEURISTICMC_H

#include "PathBuilder.hpp"
#include <Utils.hpp>

class MonteCarloHeuristic : public PathBuilder {
public:
    MonteCarloHeuristic(const OverlapGraph &g, const Utils::Metrics &metric);
};


#endif //TELOMERI_HEURISTICMC_H
