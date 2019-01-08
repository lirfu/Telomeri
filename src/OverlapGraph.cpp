#include <OverlapGraph.hpp>

#include <iostream>
#include <fstream>

#include <Utils.hpp>
#include <cfloat>
#include <climits>

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

    ulong min_OL = ULONG_MAX, max_OL = 0;
    float min_OS = FLT_MAX, max_OS = 0;
    float min_ES = FLT_MAX, max_ES = 0;
    float min_SI = FLT_MAX, max_SI = 0;
    for (const Edge &e:edges_) {
        if (min_OS > e.overlap_score) {
            min_OS = e.overlap_score;
        }
        if (max_OS < e.overlap_score) {
            max_OS = e.overlap_score;
        }
        if (min_ES > e.extension_score) {
            min_ES = e.extension_score;
        }
        if (max_ES < e.extension_score) {
            max_ES = e.extension_score;
        }
        if (min_SI > e.sequence_identity) {
            min_SI = e.sequence_identity;
        }
        if (max_SI < e.sequence_identity) {
            max_SI = e.sequence_identity;
        }
        int ol = std::max(e.q_end - e.q_start, e.t_end - e.t_start);
        if (min_OL > ol) {
            min_OL = static_cast<ulong>(ol);
        }
        if (max_OL < ol) {
            max_OL = static_cast<ulong>(ol);
        }
    }

    str << "Nodes" << '\n'
        << "-  anchor: " << anchors << '\n'
        << "-    read: " << reads << '\n'
        << "-   total: " << nodes_.size() << '\n'
        << "- min_len: " << min_len << '\n'
        << "- max_len: " << max_len << '\n'
        << "- min_con: " << min_con << '\n'
        << "- max_con: " << max_con << '\n'
        << "Edges" << '\n'
        << "-   total: " << edges_.size() << '\n'
        << "-  min_OL: " << min_OL << '\n'
        << "-  max_OL: " << max_OL << '\n'
        << "-  min_OS: " << min_OS << '\n'
        << "-  max_OS: " << max_OS << '\n'
        << "-  min_ES: " << min_ES << '\n'
        << "-  max_ES: " << max_ES << '\n'
        << "-  min_SI: " << min_SI << '\n'
        << "-  max_SI: " << max_SI << '\n'
        << std::endl;

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
    ulong ctr = 0;
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
        ++ctr;

        ContigPosition pos;
        if (anchors) {
            // Checks the position of the contig by checking name starts with 'ctg'.
            // If none starts with this template, decision defaults to contig-read format.
            pos = Utils::startsWithInsensitive(o.query_name, "ctg") ?
                  ContigPosition::QUERY : ContigPosition::TARGET;
        } else {
            pos = ContigPosition::NONE;
        }

        if (filter(o)) {
            buildFrom(o, pos);
        }

        // When testing, load only N instances.
        if (test_load_num_ > 0 && test_load_num_ < ctr) {
            break;
        }
    }

    std::cout << "Loaded " << edges_.size() << '/' << ctr << " edges from " << filepath << std::endl;

    filestream.close();
    return true;
}

bool OverlapGraph::filter(const OverlapGraph::PAFOverlap &overlap) const {
    if (overlap.query_name == overlap.target_name) { // Skip edges to the same starting point.
        return false;
    }

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

    long qn_index = nodeIndex(overlap.query_name);
    if (qn_index < 0) { // Node doesn't exist yet, create it.
        qn_index = nodes_.size();
        nodes_.emplace_back(pos == ContigPosition::QUERY, qn_index, overlap.query_len, overlap.query_name);
    }

    long tn_index = nodeIndex(overlap.target_name);
    if (tn_index < 0) { // Node doesn't exist yet, create it.
        tn_index = nodes_.size();
        nodes_.emplace_back(pos == ContigPosition::TARGET, tn_index, overlap.target_len, overlap.target_name);
    }

    // Create edge and emplace it into internal vector.
    int query_OL = getQueryOverlapLength(overlap);
    int target_OL = getTargetOverlapLength(overlap);
    int query_EL = getQueryExtensionLength(overlap);
    int query_OH = getQueryOverhangLength(overlap);
    int target_OH = getTargetOverhangLength(overlap);

    float SI = getSequenceIdentity(overlap);
    float OS = (query_OL + target_OL) / 2.0f * SI;

    // TODO This shouldn't give negative values (or should it?).
    float ES = std::abs(OS + query_EL / 2.0f - (query_OH + target_OH) / 2.0f);

    edges_.emplace_back(
            qn_index, // Index of first node of the edge.
            tn_index, // Index of second node of the edge.
            overlap.query_start,
            overlap.query_end,
            overlap.target_start,
            overlap.target_end,
            OS, SI, ES);

    nodes_[qn_index].edges.push_back(edges_[edges_.size() - 1]);
    nodes_[tn_index].edges.push_back(edges_[edges_.size() - 1]);
}

long OverlapGraph::nodeIndex(const std::string &name) const {
    for (long i = nodes_.size() - 1; i >= 0; i--) {
        if (nodes_[i].name == name) {
            return i;
        }
    }
    return -1;
}

OverlapGraph::Node::Node(bool anchor, uint index, uint length, const std::string &name)
        : anchor(anchor), index(index), length(length), name(name) {}


bool OverlapGraph::Node::operator==(const Node &rhs) const {
    return name == rhs.name;
}

void OverlapGraph::Node::to_stream(std::ostream &s) const {
    s << (anchor ? '*' : ' ') << 'n' << index << "  len: " << length
      << "  name: " << name << "  edges: " << edges.size() << std::endl;
}

OverlapGraph::Edge::Edge(uint q_index, uint t_index, uint q_start, uint q_end, uint t_start, uint t_end,
                         float overlap_score, float sequence_identity, float extension_score)
        : q_index(q_index), t_index(t_index), q_start(q_start), q_end(q_end), t_start(t_start), t_end(t_end),
          overlap_score(overlap_score), sequence_identity(sequence_identity),
          extension_score(extension_score) {}

OverlapGraph::Edge &OverlapGraph::Edge::operator=(OverlapGraph::Edge &&e) noexcept {
    std::swap(q_index, e.q_index);
    std::swap(t_index, e.t_index);
    std::swap(q_start, e.q_start);
    std::swap(q_end, e.q_end);
    std::swap(t_start, e.t_start);
    std::swap(t_end, e.t_end);
    std::swap(overlap_score, e.overlap_score);
    std::swap(sequence_identity, e.sequence_identity);
    std::swap(extension_score, e.extension_score);
    return *this;
}

void OverlapGraph::Edge::to_stream(std::ostream &s) const {
    s << q_index << "-" << t_index << "  OS: " << overlap_score
      << "  ES: " << extension_score << "  SI: " << sequence_identity << std::endl;
}

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
    return overlap.residue_matches_num / (float)
            std::min(overlap.query_len, overlap.target_len);
}
