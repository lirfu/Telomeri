#ifndef OVERLAP_GRAPH_HPP
#define OVERLAP_GRAPH_HPP

#include <vector>
#include <string>

enum class ContigPosition : char;

class OverlapGraph {
public:
    struct Edge;

    struct Node {
        bool anchor; /**< True if node is anchor (contig). */
        uint index; /**< Unique node ID. Equal to position in vector of nodes. */
        std::vector<Edge> edges; /**< Edges that connect to this node. */
        uint length; /**< Length of sequence. */
        std::string name; /**< Unique sequence (node) name. */

        Node(bool anchor, uint index, uint length, const std::string &name);

        bool operator==(const Node &rhs) const;
    };

    struct Edge {
        const Node &n1; /**< First node of the edge. */
        const Node &n2; /**< Second node of the edge. */
        int overlap_length;
        float overlap_score, sequence_identity, extension_score;

        Edge(const Node &n1, const Node &n2, int overlap_length,
             float overlap_score, float sequence_identity,
             float extension_score);
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

    enum LengthCalculationMode {
        MIN, /** Smaller of the lengths is used in filter comparisons */
        MAX, /** Larger of the lengths is used in filter comparisons */
        AVG, /** Average value of the lengths is used in filter comparisons */
        SUM /** Sum of lengths is used in filter comparisons */
    };

    struct FilterParameters {
        LengthCalculationMode mode; /** Mode of length calculation */
        int min_overlap_length; /** Minimum required overlap length */
        float min_overlap_percentage; /** Minimum required overlap length, as percentage to total length */
        int max_overhang_length; /** Maximum allowed overhang length */
        float max_overhang_percentage; /** Maximum allowed overhang length, as percentage of overlap length */
    };

    std::vector<Node> nodes_;
    std::vector<Edge> edges_;
    FilterParameters filter_params_;

    /**
     * Loads the given .paf file and constructs an overlap graph from it.
     * @param filepath String path to the file.
     * @param anchors The inputs will be anchors.
     * @return false if an error occurred.
     */
    bool load(char *filepath, bool anchors);

    /** Returns a string representation of the internal graph statistics. */
    std::string stats();

    /** Variable used to specify the number of lines read from a file.
     * Used to cut down the loading time when testing.
     * By default, loads the whole file (is equal to 0).*/
    uint test_load_num_ = 0;

private:
    /**
     * Descides if the overlap is usable based on its quality.
     * @param overlap
     * @return true if given overlap is usable.
     */
    bool filter(const PAFOverlap &overlap) const;

    /**
     * Constructs and edge and nodes (if they don't exist) and adds them to the
     * internal vectors.
     * @param overlap
     * @param pos flag signaling if and where contigs is in the overlap.
     */
    void buildFrom(const PAFOverlap &overlap, ContigPosition pos);

    /**
     * Check if node has already been seen. More specifically, the function
     * checks if node is present in internal node vector.
     * @param n node to check if it is present.
     * @return index of the node in internal vector if it is present in internal
     * vectors. If it is not present, function returns -1.
     */
    long nodeIndex(const std::string &name) const;
};

enum class ContigPosition : char {
    NONE,  /**< Contig is not part of overlap (overlap is between reads). */
    QUERY,  /**< Contig is in the query part of the overlap. */
    TARGET    /**< Contig is in the target part of the overlap. */
};

#endif
