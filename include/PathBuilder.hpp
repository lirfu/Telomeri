//
// Created by lirfu on 14.12.18..
//

#ifndef TELOMERI_PATHBUILDER_H
#define TELOMERI_PATHBUILDER_H


#include <vector>
#include "OverlapGraph.hpp"

class PathBuilder {
public:
    struct Path {
        std::vector<const OverlapGraph::Node *> nodes_;
        std::vector<const OverlapGraph::Edge *> edges_;

        int compare(Path p) {
            // TODO hash compare or informativeness(entropy) or ...
        }
    };

    std::vector<Path> paths_;
};


#endif //TELOMERI_PATHBUILDER_H
