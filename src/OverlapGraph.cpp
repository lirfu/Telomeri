#include <iostream>
#include <fstream>
#include <Utils.hpp>

#include "OverlapGraph.hpp"

bool OverlapGraph::load(char *filepath, bool anchors) {
    std::fstream filestream;
    filestream.open(filepath, std::fstream::in);
    if (!filestream) {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return false;
    }

    // Read line-by-line, check filter and build nodes.
    PAFOverlap o;
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

        ContigPosition pos;
        if (anchors) {
            // Checks the position of the contig by checking name starts with 'ctg'.
            // If none starts with this template, decision defaults to contig-read format.
            pos = Utils::startsWithInsensitive(o.target_name, "ctg") == 0 ?
                  ContigPosition::TARGET : ContigPosition::QUERY;
        } else {
            pos = ContigPosition::NONE;
        }

        if (!filter(o)) {
            buildFrom(o, pos);
        }
    }

    return true;
}

bool OverlapGraph::filter(const OverlapGraph::PAFOverlap &overlap) const {
    // TODO Domagoj
    return true;
}

void OverlapGraph::buildFrom(
        const OverlapGraph::PAFOverlap &overlap,
        ContigPosition pos) {

    int qn_index; // Query node index in internal vector. Needed for edge.
    { // Create query node.
        Node qn(pos == ContigPosition::QUERY,
                nodes_.size(),        // Will be added at vector end.
                overlap.query_len,
                overlap.query_name);
        qn_index = nodeIndex(qn); // Fetch node index in internal vector.
        if (qn_index < 0) {       // Check if node is present in vector.
            nodes_.push_back(qn); // Add node to end of internal vector.
            qn_index = qn.index;  // Remember node index.
        }
    }
    
    int tn_index; // Target node index in internal vector. Needed for edge.
    { // Create target node.
        Node tn(pos == ContigPosition::TARGET,
                nodes_.size(),        // Will be added at vector end.
                overlap.target_len,
                overlap.query_name);
        tn_index = nodeIndex(tn); // Fetch node index in internal vector.
        if (tn_index < 0) {       // Check if node is present in vector.
            nodes_.push_back(tn); // Add node to end of internal vector.
            tn_index = tn.index;  // Remember node index.
        }
    }

    // Create edge and emplace it into internal vector.
    edges_.emplace_back(
            nodes_[qn_index], // First node of the edge.
            nodes_[tn_index], // Second node of the edge.
            0,     // TODO Calculate overlap length (OL).
            0.0f,  // TODO Calculate overlap score (OS).
            0.0f,  // TODO Calculate sequence identity (SI).
            0.0f); // TODO Calculate extension score (ES).
}

int OverlapGraph::nodeIndex(const Node& node) {
    for (int i = 0, n = static_cast<int>(nodes_.size()); i < n; i++) {
        if (nodes_[i] == node) {
            return i;
        } 
    }
    return -1;
}


OverlapGraph::Node::Node(bool anchor, int index, int length,
        const std::string& name)
    : anchor(anchor), index(index), length(length), name(name) {}
 

bool OverlapGraph::Node::operator==(const Node& rhs) const {
    return name == rhs.name;
}

OverlapGraph::Edge::Edge(const Node &n1, const Node &n2, int overlap_length,
                float overlap_score, float sequence_identity,
                float extension_score)
        : n1(n1), n2(n2), overlap_length(overlap_length),
            overlap_score(overlap_score), sequence_identity(sequence_identity),
            extension_score(extension_score) {}

