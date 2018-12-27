//
// Created by lirfu on 26.12.18..
//

#ifndef TELOMERI_PATHMANAGER_HPP
#define TELOMERI_PATHMANAGER_HPP


#include <sstream>
#include "OverlapGraph.hpp"
#include "Utils.hpp"

#define REBUILD_ATTEMPTS 3

class PathManager {
private:
    std::vector<Utils::Path> paths_;
public:
    void buildMonteCarlo(const OverlapGraph &g, int repeat_num, const Utils::Metrics &metric);
    void buildDeterministic(const OverlapGraph &g, int repeat_num, const Utils::Metrics &metric);
    void filterUnique();
    std::string stats();
};


#endif //TELOMERI_PATHMANAGER_HPP
