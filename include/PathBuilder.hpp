//
// Created by lirfu on 14.12.18..
//

#ifndef TELOMERI_PATHBUILDER_H
#define TELOMERI_PATHBUILDER_H


#include <vector>
#include <sstream>
#include "OverlapGraph.hpp"

class PathBuilder {
public:
    struct Path {
        std::vector<const OverlapGraph::Node *> nodes_;
        std::vector<const OverlapGraph::Edge *> edges_;

        int compare(Path p) {
            // TODO hash compare or informativeness(entropy) or ...
        }

        std::string str() const {
            std::stringstream str;
            bool e = false;
            int ni = 0, ei = 0;
            for (int i = 0; i < nodes_.size() + edges_.size(); i++) {
                if (e) {
                    str << "-";
//                    ei++;
                    e = false;
                } else {
                    str << 'n' << nodes_[ni++]->index;
                    e = true;
                }
            }
            return str.str();
        }
    };

    std::vector<Path> paths_;
};


#endif //TELOMERI_PATHBUILDER_H
