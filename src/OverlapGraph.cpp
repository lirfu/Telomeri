#include <iostream>
#include <fstream>
#include <Utils.hpp>

#include "OverlapGraph.hpp"

int getQueryOverlapLength(const OverlapGraph::PAFOverlap &overlap);
int getTargetOverlapLength(const OverlapGraph::PAFOverlap &overlap);
int getQueryOverhangLength(const OverlapGraph::PAFOverlap &overlap);
int getTargetOverhangLength(const OverlapGraph::PAFOverlap &overlap);
int getQueryExtensionLength(const OverlapGraph::PAFOverlap &overlap);
int getTargetExtensionLength(const OverlapGraph::PAFOverlap &overlap);

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
    int query_overlap_length = getQueryOverlapLength(overlap);
    int target_overlap_length = getTargetOverlapLength(overlap);
    int query_overhang_length = getQueryOverhangLength(overlap);
    int target_overhang_length = getTargetOverhangLength(overlap);
    float overlap_length;
    float overhang_length;
    float total_length;

    switch (filter_params_.mode) {
        case MIN:
            overlap_length = std::min(query_overlap_length, target_overlap_length);
            overhang_length = std::min(query_overhang_length, target_overhang_length);
            total_length = std::min(overlap.query_len, overlap.target_len);
            break;
        case MAX:
            overlap_length = std::max(query_overlap_length, target_overlap_length);
            overhang_length = std::max(query_overhang_length, target_overhang_length);
            total_length = std::max(overlap.query_len, overlap.target_len);
            break;
        case AVG:
            overlap_length = (query_overlap_length + target_overlap_length) / 2.0f;
            overhang_length = (query_overhang_length + target_overhang_length) / 2.0f;
            total_length = (overlap.query_len + overlap.target_len) / 2.0f;
            break;
        case SUM:
            overlap_length = query_overlap_length + target_overlap_length;
            overhang_length = query_overhang_length + target_overhang_length;
            total_length = overlap.query_len + overlap.target_len;
            break;
        default:
            return false;
    }

    if (overlap_length < filter_params_.min_overlap_length
        || (overlap_length / total_length) < filter_params_.min_overlap_percentage) {
        return false;
    }

    return overhang_length <= filter_params_.max_overhang_length
             && (overhang_length / overlap_length) <= filter_params_.max_overhang_percentage;

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

    { // Create edge and emplace it into internal vector.
        int query_overlap_length = getQueryOverlapLength(overlap);
        int target_overlap_length = getTargetOverlapLength(overlap);
        int query_overhang_length = getQueryOverhangLength(overlap);
        int target_overhang_length = getTargetOverhangLength(overlap);
        int qurey_extension_length = getQueryExtensionLength(overlap);
        int target_extension_length = getTargetExtensionLength(overlap);
        edges_.emplace_back(
                nodes_[qn_index], // First node of the edge.
                nodes_[tn_index], // Second node of the edge.
                0,     // TODO Calculate overlap length (OL).
                0.0f,  // TODO Calculate overlap score (OS).
                0.0f,  // TODO Calculate sequence identity (SI).
                0.0f); // TODO Calculate extension score (ES).
    }
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

int getQueryOverlapLength(const OverlapGraph::PAFOverlap &overlap) {
    return overlap.query_end - overlap.query_start;
}

int getTargetOverlapLength(const OverlapGraph::PAFOverlap &overlap) {
    return overlap.target_end - overlap.target_start;
}

int getQueryOverhangLength(const OverlapGraph::PAFOverlap &overlap) {
    return overlap.query_len - overlap.query_end; // TODO: CHECK IF CORRECT!
}

int getTargetOverhangLength(const OverlapGraph::PAFOverlap &overlap) {
    return overlap.target_len - overlap.target_end; // TODO: CHECK IF CORRECT!
}

int getQueryExtensionLength(const OverlapGraph::PAFOverlap &overlap) {
    return overlap.query_len - overlap.query_end;
}

int getTargetExtensionLength(const OverlapGraph::PAFOverlap &overlap) {
    return overlap.query_start;
}
