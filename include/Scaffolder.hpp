//
// Created by lirfu on 10.01.19..
//

#ifndef TELOMERI_SCAFFOLDER_HPP
#define TELOMERI_SCAFFOLDER_HPP


#include "Path.hpp"

class Scaffolder {
public:
    const Path &p_;
    std::vector<std::string> names_;
    std::vector<std::string> sequences_;

    explicit Scaffolder(const Path &p) : p_(p) {}

    /** Load names and sequences from given filename. */
    bool load(const char *filename);

    /** Get substring from start (inclusive) to end (inclusive). */
    std::string substring(std::string &str, ulong start, ulong end);

    std::string &reverse(std::string &);
    std::string reverse(std::string );

    std::string &getSequenceFrom(const std::string &name);

    /** Construct resulting sequence and write to file. */
    bool write(const char *filename);

    void swapBases(std::string &str);
};


#endif //TELOMERI_SCAFFOLDER_HPP
