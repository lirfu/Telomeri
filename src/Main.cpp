#include <iostream>
#include <OverlapGraph.hpp>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Please provide a path to read-read and contig-read files!" << std::endl;
        std::cerr << "Usage: hera <read-read_overlap_file>.paf <contig-read_overlap_file>.paf" << std::endl;
        return 1;
    }

    // Construct overlap graph from both files.
    std::cout << "Loading files..." << std::endl;
    OverlapGraph graph;
    if (!graph.load(argv[1], false) || !graph.load(argv[2], true)) {
        return 1;
    }

    std::cout << "Done!" << std::endl;

    return 0;
}
