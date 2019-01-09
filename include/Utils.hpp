#ifndef TELOMERI_UTILS_H
#define TELOMERI_UTILS_H

#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <cmath>

#include <OverlapGraph.hpp>

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
        EXTENSION_SCORE, OVERLAP_SCORE, EXTENSION_SCORE_SQRT, OVERLAP_SCORE_SQRT
    };

    static float getMetric(const OverlapGraph::Edge &e, const Utils::Metrics &metric) {
        switch (metric) {
            case Utils::Metrics::EXTENSION_SCORE:
                return e.extension_score;
            case Utils::Metrics::EXTENSION_SCORE_SQRT:
                return std::sqrt(e.extension_score);
            case Utils::Metrics::OVERLAP_SCORE:
                return e.overlap_score;
            case Utils::Metrics::OVERLAP_SCORE_SQRT:
                return std::sqrt(e.overlap_score);
            default:
                return 0.0f;
        }
    }

    static char* getMetricName(const Utils::Metrics &metric){
        switch (metric) {
            case Utils::Metrics::EXTENSION_SCORE:
                return const_cast<char *>("EXTENSION_SCORE");
            case Utils::Metrics::EXTENSION_SCORE_SQRT:
                return const_cast<char *>("EXTENSION_SCORE_SQRT");
            case Utils::Metrics::OVERLAP_SCORE:
                return const_cast<char *>("OVERLAP_SCORE");
            case Utils::Metrics::OVERLAP_SCORE_SQRT:
                return const_cast<char *>("OVERLAP_SCORE_SQRT");
            default:
                return const_cast<char *>("");
        }
    }
}

#endif
