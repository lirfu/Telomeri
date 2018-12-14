//
// Created by lirfu on 14.12.18..
//

#ifndef TELOMERI_UTILS_H
#define TELOMERI_UTILS_H

#include <string>

namespace Utils {
    /** Checks if string starts with given template, insensitive to case (toLower).
     * @param str String to check.
     * @param temp Template to check with.
     * @return true - if string starts with given template. */
    bool startsWithInsensitive(const std::string &str, const std::string &temp) {
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

    enum Metrics {
        EXTENSION_SCORE, OVERLAP_SCORE
    };
};


#endif //TELOMERI_UTILS_H
