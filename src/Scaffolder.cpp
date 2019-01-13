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
        std::cerr << "Cannot write file: " << filename << std::endl;
        return false;
    }

    filestream << ">Resulting_HERA_scaffold" << std::endl;

    // TODO Watch out for screwed-up strands (reversed or inverted or relative_strand or whatever).
    std::string sequence, tmp;
    if (p_.nodes_.size() == 0) {
        std::cerr << "Scaffold is empty!" << std::endl;
        return false;
    } else if (p_.nodes_.size() == 1) { // Single node path.
        filestream << reverse(std::move(getSequenceFrom(p_.nodes_[0]->name)));
        return true;
    }

    // First sequence.
    tmp = getSequenceFrom(p_.nodes_[0]->name);
    sequence += reverse(substring(tmp, p_.edges_[0]->t_end + 1, tmp.size()));

    // Intermediate sequences.
    for (ulong i = 1; i < p_.edges_.size(); i++) {
        const OverlapGraph::Edge *e = p_.edges_[i - 1];
        const OverlapGraph::Edge *e_next = p_.edges_[i];

        ulong start = e_next->t_end;
        ulong end = e->q_end;

        if (start < end) {
            tmp = getSequenceFrom(p_.nodes_[i]->name);
            sequence += reverse(substring(tmp, start + 1, end));
            // The +1 is to not include the overlap area (next one will do that)

        } else if (start == end) {
            continue;

        } else { // Sequence indices is going backwards -> revert the afflicted area.
            sequence = substring(sequence, 0, sequence.length() - 1 - (start - end));
        }
    }

    // Last sequence.
    tmp = getSequenceFrom(p_.nodes_[p_.nodes_.size() - 1]->name);
    sequence += reverse(substring(tmp, 0, p_.edges_[p_.edges_.size() - 1]->q_end));

    std::reverse(sequence.begin(), sequence.end());
    filestream << sequence << std::endl;
    filestream.close();
    return true;
}

std::string &Scaffolder::getSequenceFrom(const std::string &name) {
    ulong index;
    for (index = 0; index < names_.size(); index++) {
        if (name == names_[index]) {
            break;
        }
    }
    return sequences_[index];
}

std::string Scaffolder::substring(std::string &str, ulong start, ulong end) {
    std::string s = str.substr(start, end - start + 1);
    return s;
}

std::string &Scaffolder::reverse(std::string &str) {
    std::reverse(str.begin(), str.end());
    return str;
}

std::string Scaffolder::reverse(std::string str) {
    std::reverse(str.begin(), str.end());
    return str;
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



