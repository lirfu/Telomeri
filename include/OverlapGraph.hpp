//
// Created by lirfu on 08.12.18..
//

#pragma once

#include <vector>
#include <string>

class OverlapGraph {
public:
    struct Edge;

    struct Node {
        bool anchor;
        int index;
        std::vector<Edge> edges;
        int length;
        std::string name;
    };

    struct Edge {
        Node &n1, n2;
        int overlap_length;
        float overlap_score, sequence_identity, extension_score;
    };

    struct PAFOverlap {
        std::string query_name;
        int query_len, query_start, query_end;
        char relative_strand;
        std::string target_name;
        int target_len, target_start, target_end;
        int residue_matches_num;
        int alignment_block_len;
        int mapping_quality;
    };

private:
    std::vector<Node> nodes_;
    std::vector<Edge> edges_;

    /**
     * Descides if the overlap is usable based on its' quality.
     * @param overlap
     * @return true if given overlap is usable.
     */
    bool filter(const PAFOverlap &overlap);

    /**
     * Constructs and edge and nodes (if they don't exist) and adds them to the internal vectors.
     * @param overlap
     */
    void buildFrom(const PAFOverlap &overlap);

public:
    /**
     * Loads the given .paf file and constructs an overlap graph from it.
     * @param filepath String path to the file.
     * @return false if an error occurred.
     */
    bool load(char *filepath);
};
