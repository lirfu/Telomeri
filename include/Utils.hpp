//
// Created by lirfu on 14.12.18..
//

#ifndef TELOMERI_UTILS_H
#define TELOMERI_UTILS_H

#include <string>
#include <sstream>
#include <algorithm>
#include "OverlapGraph.hpp"

namespace Utils {
    /** Checks if string starts with given template, insensitive to case (toLower).
     * @param str String to check.
     * @param temp Template to check with.
     * @return true - if string starts with given template. */
    static bool startsWithInsensitive(const std::string &str, const std::string &temp) {
        if (str.length() < temp.length()) {
            return false;
        }
        for (int i = 0; i < temp.length(); i++) {
            if (std::tolower(str[i]) != std::tolower(temp[i])) {
                return false;
            }
        }
        return true;
    }

    template<typename T>
    static bool contains(const std::vector<T> &v, const T &e) {
        return std::find(v.begin(), v.end(), e) != v.end();
    }

    enum class Metrics {
        EXTENSION_SCORE, OVERLAP_SCORE
    };

    static float getMetric(const OverlapGraph::Edge &e, const Utils::Metrics &metric) {
        switch (metric) {
            case Utils::Metrics::EXTENSION_SCORE:
                return e.extension_score;
            case Utils::Metrics::OVERLAP_SCORE:
                return e.overlap_score;
        }
    }

    struct Path {
        std::vector<const OverlapGraph::Node *> nodes_;
        std::vector<const OverlapGraph::Edge *> edges_;

        int compare(Path p) {
            // TODO hash compare or informativeness(entropy) or ...
        }

        std::string str() const {
            std::stringstream str;
            bool e = false;
            int ni = 0;
            for (int i = 0; i < nodes_.size() + edges_.size(); i++) {
                if (e) {
                    str << "-";
                    e = false;
                } else {
                    if (nodes_[ni]->anchor) {
                        str << '*';
                    }
                    str << 'n' << nodes_[ni]->index;
                    ni++;
                    e = true;
                }
            }
            return str.str();
        }
    };
};


#endif //TELOMERI_UTILS_H
