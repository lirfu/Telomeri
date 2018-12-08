
#include <iostream>
#include <OverlapGraph.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Please provide a path to read-read and contig-read files!" << std::endl;
        std::cerr << "Usage: hera <read-read_overlap_file>.paf <contig-read_overlap_file>.paf" << std::endl;
        return 1;
    }

    // Construct overlap graph from both files.
    OverlapGraph graph;
    if (!graph.load(argv[1]) || graph.load(argv[2])) {
        return 1;
    }

    return 0;
}
