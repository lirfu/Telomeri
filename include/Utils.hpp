//
// Created by lirfu on 14.12.18..
//

#ifndef TELOMERI_UTILS_H
#define TELOMERI_UTILS_H

#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>
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
        for (int i = 0; i < static_cast<int>(temp.length()); i++) {
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
            default:
                return 0.0f;
        }
    }

    struct Path {
        std::vector<const OverlapGraph::Node *> nodes_;
        std::vector<const OverlapGraph::Edge *> edges_;

        static bool equals(const Path &p1, const Path &p2) {
            return p1.nodes_.size() == p2.nodes_.size()
                   && std::equal(p1.nodes_.begin(), p1.nodes_.end(), p2.nodes_.begin());
        }

        ulong length() const {
            ulong l = 0;
            for (const OverlapGraph::Edge *e:edges_) {
                l += e->overlap_length; // FIXME Must calculate the extension lengths as well.
            }
            return l;
        }

        std::string str() const {
            std::stringstream str;
            bool e = false;
            int ni = 0;
            for (int i = 0, n = static_cast<int>(nodes_.size() + edges_.size()); i < n; i++) {
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

    class Stopwatch {
    private:
        std::chrono::time_point<std::chrono::system_clock> start_;
        std::chrono::time_point<std::chrono::system_clock> lapStart_;
    public:
        /** Start the timer. Resets internals to the current time point. */
        void start() {
            start_ = lapStart_ = std::chrono::system_clock::now();
        }

        /** Stop the timer and return total time. @return Total elapsed time.*/
        double stop() {
            auto now = std::chrono::system_clock::now();
            std::chrono::duration<double> diff = now - start_;
            return diff.count();
        }

        /** Return lap time and start new lap. Doesn't affect total time. @return Lap time. */
        double lap() {
            auto now = std::chrono::system_clock::now();
            std::chrono::duration<double> diff = now - lapStart_;
            lapStart_ = now;
            return diff.count();
        }
    };
}


#endif //TELOMERI_UTILS_H
