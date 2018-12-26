#include <iostream>
#include <OverlapGraph.hpp>
#include <Utils.hpp>
#include <PathBuilder.hpp>
#include <MonteCarloHeuristic.hpp>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Please provide a path to read-read and contig-read files!" << std::endl;
        std::cerr << "Usage: hera <read-read_overlap_file>.paf <contig-read_overlap_file>.paf" << std::endl;
        return 1;
    }

    // Construct overlap graph from both files.
    std::cout << "Loading files..." << std::endl;
    OverlapGraph graph;

    // FIXME Just for testing.
    graph.test_load_num_ = 10;

    if (!graph.load(argv[1], false) || !graph.load(argv[2], true)) {
        return 1;
    }

    // Construct paths with following heuristics.
    std::cout << "Calculating paths..." << std::endl;
//    DeterministicHeuristic h_os(graph, Utils::Metrics::OVERLAP_SCORE);
//    DeterministicHeuristic h_es(graph, Utils::Metrics::EXTENSION_SCORE);
    MonteCarloHeuristic h_mc(graph, 3, Utils::Metrics::EXTENSION_SCORE);

    // TODO Aggregate paths into a single object. Why not make heuristics build paths to the same path vector?
    // PathManager pm (h_os, h_es, h_mc);

    // TODO Filter uniques.
    // pm.filterUnique();

    // TODO Construct groups.
    // std::cout << "Constructing groups..." << std::endl;
    // SomeFunc f;
    // pm.constructGroups(f);

    // TODO Re-group based on current group scores (this may be misinterpreted).
    // SomeOtherFunc f1;
    // pm.constructGroups(f1);

    // TODO Filter rare paths (this may be misinterpreted).
    // pm.filterRare();

    // TODO Construct consensus paths.
    // std::cout << "Building the scaffold..." << std::endl;
    // PathBuilder::Path consensus = pm.constructConsensusPath();

    // TODO Construct final result/scaffold (may need to load original files after all, but only here).
    // char *scaffold = Utils::buildScaffold(consensus, reads_file, contigs_file);

    // TODO Write result to a file.
    // std::cout << "Writing result to: " << output_file << std::endl;
    // Utils::write(scaffold, output_file);

    std::cout << "Done!" << std::endl;
    return 0;
}
