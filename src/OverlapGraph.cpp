#include <iostream>
#include <fstream>
#include <Utils.hpp>

int getQueryOverlapLength(const OverlapGraph::PAFOverlap &overlap);

int getTargetOverlapLength(const OverlapGraph::PAFOverlap &overlap);

int getQueryOverhangLength(const OverlapGraph::PAFOverlap &overlap);

int getTargetOverhangLength(const OverlapGraph::PAFOverlap &overlap);

int getQueryExtensionLength(const OverlapGraph::PAFOverlap &overlap);

int getTargetExtensionLength(const OverlapGraph::PAFOverlap &overlap);

float getSequenceIdentity(const OverlapGraph::PAFOverlap &overlap);

std::string OverlapGraph::stats() {
    std::stringstream str;

    ulong min_len = static_cast<ulong>(-1), max_len = 0;
    ulong min_con = static_cast<ulong>(-1), max_con = 0;
    ulong anchors = 0, reads = 0;
    for (const Node &n:nodes_) {
        n.anchor ? anchors++ : reads++;
        if (min_len > n.length)
            min_len = n.length;
        if (max_len < n.length)
            max_len = n.length;
        if (min_con > n.edges.size())
            min_con = n.edges.size();
        if (max_con < n.edges.size())
            max_con = n.edges.size();
    }

    str << "Nodes" << std::endl;
    str << "-  anchor: " << anchors << std::endl;
    str << "-    read: " << reads << std::endl;
    str << "-   total: " << nodes_.size() << std::endl;
    str << "- min_len: " << min_len << std::endl;
    str << "- max_len: " << max_len << std::endl;
    str << "- min_con: " << min_con << std::endl;
    str << "- max_con: " << max_con << std::endl;
    str << "Edges" << std::endl;
    str << "-  total: " << edges_.size() << std::endl;

    return str.str();
}

bool OverlapGraph::load(char *filepath, bool anchors) {
    std::fstream filestream;
    filestream.open(filepath, std::fstream::in);
    if (!filestream) {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return false;
    }

    // Read line-by-line, check filter and build nodes.
    PAFOverlap o;
    std::string s;
    unsigned i = 0;
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

        // Ignore these last 4 unspecified outputs.
        filestream >> s;
        filestream >> s;
        filestream >> s;
        filestream >> s;

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

        // When testing, load only N instances.
        if (test_load_num_ > 0 && test_load_num_ < ++i) {
            break;
        }
    }

    std::cout << "Loaded " << nodes_.size() << '/' << i << " nodes from " << filepath << std::endl;

    filestream.close();
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
                nodes_.size(),    // Will be added at vector end.
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
                nodes_.size(),    // Will be added at vector end.
                overlap.target_len,
                overlap.target_name);
        tn_index = nodeIndex(tn); // Fetch node index in internal vector.
        if (tn_index < 0) {       // Check if node is present in vector.
            nodes_.push_back(tn); // Add node to end of internal vector.
            tn_index = tn.index;  // Remember node index.
        }
    }

    { // Create edge and emplace it into internal vector.
        int query_OL = getQueryOverlapLength(overlap);
        int target_OL = getTargetOverlapLength(overlap);
        int query_EL = getQueryExtensionLength(overlap);
        int query_OH = getQueryOverhangLength(overlap);
        int target_OH = getTargetOverhangLength(overlap);
        float SI = getSequenceIdentity(overlap);

        float OS = (query_OL + target_OL) * SI / 2.0f;

        // TODO This shouldn't give negative values (filtering problems).
        float ES = std::abs(OS + query_EL / 2.0f - (query_OH + target_OH) / 2.0f);

        edges_.emplace_back(
                nodes_[qn_index], // First node of the edge.
                nodes_[tn_index], // Second node of the edge.
                query_OL,
                OS,
                SI,
                ES);

        nodes_[qn_index].edges.push_back(edges_[edges_.size() - 1]);
        nodes_[tn_index].edges.push_back(edges_[edges_.size() - 1]);
    }
}

int OverlapGraph::nodeIndex(const Node &node) {
    for (int i = 0, n = static_cast<int>(nodes_.size()); i < n; i++) {
        if (nodes_[i] == node) {
            return i;
        }
    }
    return -1;
}

OverlapGraph::Node::Node(bool anchor, uint index, uint length,
                         const std::string &name)
        : anchor(anchor), index(index), length(length), name(name) {}


bool OverlapGraph::Node::operator==(const Node &rhs) const {
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
    return overlap.query_len - overlap.query_end;
}

int getTargetOverhangLength(const OverlapGraph::PAFOverlap &overlap) {
    return overlap.target_start;
}

int getQueryExtensionLength(const OverlapGraph::PAFOverlap &overlap) {
    return overlap.query_start;
}

int getTargetExtensionLength(const OverlapGraph::PAFOverlap &overlap) {
    return overlap.target_len - overlap.target_end;
}

float getSequenceIdentity(const OverlapGraph::PAFOverlap &overlap) {
    return overlap.residue_matches_num /
           std::min(overlap.query_len, overlap.target_len);
}
