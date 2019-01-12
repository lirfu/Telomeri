//
// Created by lirfu on 10.01.19..
//

#include <Scaffolder.hpp>
#include <fstream>
#include <iostream>
#include <Utils.hpp>


bool Scaffolder::load(const char *filename) {
    bool fastq = Utils::fileExtensionMatches(filename, ".fastq");

    std::fstream filestream;
    filestream.open(filename, std::fstream::in);
    if (!filestream) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }

    std::string name;
    std::string sequence;
    while (filestream >> name) {
        filestream >> sequence;
        names_.emplace_back(name.begin() + 1, name.end());
        sequences_.emplace_back(sequence.begin() + 1, sequence.end());
        if (fastq) { // Skip quality lines.
            filestream >> name;
            filestream >> name;
        }
    }
    filestream.close();
    return true;
}

bool Scaffolder::write(const char *filename) {
    std::fstream filestream;
    filestream.open(filename, std::fstream::out);
    if (!filestream) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }

    // TODO Watch out for screwed-up strands (reversed or inverted or relative_strand or whatever).
    for (uint i = 0; i < p_.nodes_.size(); i++) {
        const OverlapGraph::Node *n = p_.nodes_[i];
        ulong index;
        for (index = 0; index < names_.size(); index++) {
            if (n->name == names_[index]) {
                break;
            }
        }
        std::string &seq = sequences_[index];
        // TODO take a subsequence matching the information in the edge.
        std::reverse(seq.begin(), seq.end());
        filestream << seq;
    }

    filestream.close();
    return true;
}

void Scaffolder::swapBases(std::string &str) {
    for (ulong i = 0; i < str.size(); i++) {
        switch (str[i]) {
            case 'A':
                str[i] = 'T';
                break;
            case 'T':
                str[i] = 'A';
                break;
            case 'G':
                str[i] = 'C';
                break;
            case 'C':
                str[i] = 'G';
                break;
            default:;
        }
    }
}



