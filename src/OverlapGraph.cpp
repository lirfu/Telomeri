#include <iostream>
#include <fstream>

#include "OverlapGraph.hpp"

bool OverlapGraph::load(char *filepath) {
    std::fstream filestream;
    filestream.open(filepath, std::fstream::in);
    if (!filestream) {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return false;
    }

    PAFOverlap o;
    int i = 0;
    while (filestream
            >> o.query_name
            >> o.query_len
            >> o.query_start
            >> o.query_end
            >> o.relative_strand
            >> o.target_name
            >> o.target_len
            >> o.target_start
            >> o.target_end
            >> o.residue_matches_num
            >> o.alignment_block_len
            >> o.mapping_quality) {
//        TODO Remove, this for testing.
//        if (i++ > 0) break;
//        std::cout << o.query_name << "," << o.query_len << "," << o.query_start << "," << o.query_end << ","
//                  << o.relative_strand << ","
//                  << o.target_name << "," << o.target_len << "," << o.target_start << "," << o.target_end << ","
//                  << o.residue_matches_num << "," << o.alignment_block_len << "," << o.mapping_quality << std::endl;
        if (!filter(o)) {
            buildFrom(o);
        }
    }

    return true;
}

bool OverlapGraph::filter(const OverlapGraph::PAFOverlap &overlap) {
    // TODO Domagoj
    return true;
}

void OverlapGraph::buildFrom(const OverlapGraph::PAFOverlap &overlap) {
    // TODO Rudolf
}
