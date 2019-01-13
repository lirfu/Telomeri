#include <iostream>

#include <PathManager.hpp>
#include <Stopwatch.hpp>
#include <Scaffolder.hpp>

#define READS_FILE "reads"
#define CONTIGS_FILE "contigs"
#define RR_OVERLAP_FILE "rr_overlap.paf"
#define CR_OVERLAP_FILE "cr_overlap.paf"
#define RESULT_FILE "hera_result.fasta"


enum ParseState {
    NONE, OLL, OLP, OHL, OHP, RB_ATT, BT_ATT, LEN_THR, NN_THR, W_SIZE, R_THR
};

ulong try_parse_pos_num(const char *s) {
    try {
        long v = std::stol(s);

        if (v < 0) {
            std::cerr << "Value must be positive: " << s << std::endl;
            exit(1);
        }

        return static_cast<ulong>(v);
    } catch (std::invalid_argument& e) {
        std::cerr << "Invalid int value: " << s << std::endl;
    } catch (std::out_of_range& e) {
        std::cerr << "Provided value is outside of int range: " << s << std::endl;
    }

    exit(1);
}

float try_parse_perc(const char *s) {
    try {
        float v = std::stof(s);

        if (v < 0.0f || v > 1.0f) {
            std::cerr << "Value must be in range [0.0, 1.0]: " << s << std::endl;
            exit(1);
        }

        return v;
    } catch (std::invalid_argument& e) {
        std::cerr << "Invalid float value: " << s << std::endl;
    } catch (std::out_of_range& e) {
        std::cerr << "Provided value is outside of float range: " << s << std::endl;
    }

    exit(1);
}

int main(int argc, char **argv) {
    char *rr_file;
    char *cr_file;
    const char *mode_string = "AVG";
    OverlapGraph::FilterParameters filter_params = {
            OverlapGraph::AVG,
            0,
            0.0f,
            ULONG_MAX,
            1.0f
    };
    PathManager::Parameters pm_params = {
            500,
            30,
            10000ul,
            50,
            1000ul,
            0.9f
    };

    if (argc < 3) {
        std::cerr << "Please provide a path to read-read and contig-read files!\n"
                  << "Usage: hera [option...] <read-read_overlap_file>.paf "
                  << "<contig-read_overlap_file>.paf\n"
                  << "\n"
                  << "Available filter options:\n"
                  << "    --filter-avg         Use average length in filter comparisons (default value).\n"
                  << "    --filter-min         Use shorter length in filter comparisons.\n"
                  << "    --filter-max         Use longer length in filter comparisons.\n"
                  << "    --filter-sum         Use sum of lengths in filter comparisons.\n"
                  << "    --min-oll <value>    Minimum required overlap length (default = 0).\n"
                  << "    --min-olp <value>    Minimum required overlap percentage (default = 0).\n"
                  << "    --max-ohl <value>    Maximum allowed overhang length (default = INT_MAX).\n"
                  << "    --max-ohP <value>    Maximum allowed overhang percentage (default = 1.0).\n"
                  << "\n"
                  << "Available path construction options:\n"
                  << "    --rb-att  <value>    Number of path rebuild attempts if dead-end has been reached "
                  << "(default = 500).\n"
                  << "    --bt-att  <value>    Number of backtrack attempts when encountering a dead-end "
                  << "(default = 30).\n"
                  << "    --len-thr <value>    If difference between maximum and minimum path length is greater than "
                  << "this threshold, all paths go into same group (default = 10000).\n"
                  << "    --nn-thr <value>     Faster and simpler metric for length (default = 50).\n"
                  << "    --w-size <value>     Window size in path length (default = 1000).\n"
                  << "    --r-thr              Valley and peak ratio needed for splitting the paths into groups "
                  << "according to lowest path length frequency in the valley window (default = 0.9).\n"
                  << "\n"
                  << "Percentages are in range [0.0, 1.0].\n"
                  << std::endl;
        return 1;
    } else if (argc > 3) {
        ParseState parse_state = NONE;

        for (int i = 1; i < argc - 2; i++) {
            std::string arg = argv[i];
            switch (parse_state) {
                case NONE:
                    if (arg == "--filter-avg") {
                        filter_params.mode = OverlapGraph::AVG;
                        mode_string = "AVG";
                    } else if (arg == "--filter-min") {
                        filter_params.mode = OverlapGraph::MIN;
                        mode_string = "MIN";
                    } else if (arg == "--filter-max") {
                        filter_params.mode = OverlapGraph::MAX;
                        mode_string = "MAX";
                    } else if (arg == "--filter-sum") {
                        filter_params.mode = OverlapGraph::SUM;
                        mode_string = "SUM";
                    } else if (arg == "--min-oll") {
                        parse_state = OLL;
                    } else if (arg == "--min-olp") {
                        parse_state = OLP;
                    } else if (arg == "--max-ohl") {
                        parse_state = OHL;
                    } else if (arg == "--max-ohp") {
                        parse_state = OHP;
                    } else if (arg == "--rb-att") {
                        parse_state = RB_ATT;
                    } else if (arg == "--bt-att") {
                        parse_state = BT_ATT;
                    } else if (arg == "--len-thr") {
                        parse_state = LEN_THR;
                    } else if (arg == "--nn-thr") {
                        parse_state = NN_THR;
                    } else if (arg == "--w-size") {
                        parse_state = W_SIZE;
                    } else if (arg == "--r-thr") {
                        parse_state = R_THR;
                    } else {
                        std::cerr << "Unknown argument: " << arg << std::endl;
                        return 1;
                    }
                    break;
                case OLL:
                    filter_params.min_overlap_length = try_parse_pos_num(argv[i]);
                    parse_state = NONE;
                    break;
                case OLP:
                    filter_params.min_overlap_percentage = try_parse_perc(argv[i]);
                    parse_state = NONE;
                    break;
                case OHL:
                    filter_params.max_overhang_length = try_parse_pos_num(argv[i]);
                    parse_state = NONE;
                    break;
                case OHP:
                    filter_params.max_overhang_percentage = try_parse_perc(argv[i]);
                    parse_state = NONE;
                    break;
                case RB_ATT:
                    pm_params.rebuild_attempts = (int) try_parse_pos_num(argv[i]);
                    parse_state = NONE;
                    break;
                case BT_ATT:
                    pm_params.backtrack_attempts = (int) try_parse_pos_num(argv[i]);
                    parse_state = NONE;
                    break;
                case LEN_THR:
                    pm_params.len_threshold = (long) try_parse_pos_num(argv[i]);
                    parse_state = NONE;
                    break;
                case NN_THR:
                    pm_params.node_num_threshold = (long) try_parse_pos_num(argv[i]);
                    parse_state = NONE;
                    break;
                case W_SIZE:
                    pm_params.window_size = try_parse_pos_num(argv[i]);
                    parse_state = NONE;
                    break;
                case R_THR:
                    pm_params.ratio_threshold = try_parse_perc(argv[i]);
                    parse_state = NONE;
                    break;
            }
        }

        rr_file = argv[argc - 2];
        cr_file = argv[argc - 1];
    } else {
        rr_file = argv[1];
        cr_file = argv[2];
    }

    std::cout << "Filter:\n"
              << "    Mode: " << mode_string << "\n"
              << "    Min overlap length: " << filter_params.min_overlap_length << "\n"
              << "    Min overlap percentage: " << filter_params.min_overlap_percentage << "\n"
              << "    Max overhang length: " << filter_params.max_overhang_length << "\n"
              << "    Max overhang percentage: " << filter_params.max_overhang_percentage << std::endl;
    std::cout << "Path construction:\n"
              << "    Rebuild attempts: " << pm_params.rebuild_attempts << "\n"
              << "    Backtrack attempts: " << pm_params.backtrack_attempts << "\n"
              << "    Length threshold: " << pm_params.len_threshold << "\n"
              << "    Node number threshold: " << pm_params.node_num_threshold << "\n"
              << "    Window size: " << pm_params.window_size << "\n"
              << "    Ratio threshold: " << pm_params.ratio_threshold << "\n"
              << std::endl;

    Stopwatch timer;
    timer.start();

    // Construct overlap graph from both files.
    OverlapGraph graph;
    graph.filter_params_.mode = filter_params.mode;
    graph.filter_params_.min_overlap_length = filter_params.min_overlap_length;
    graph.filter_params_.min_overlap_percentage = filter_params.min_overlap_percentage;
    graph.filter_params_.max_overhang_length = filter_params.max_overhang_length;
    graph.filter_params_.max_overhang_percentage = filter_params.max_overhang_percentage;

    // FIXME Just for testing.
    graph.test_load_num_ = 100000;

    std::cout << "\nLoading overlaps..." << std::endl;
    if (!graph.load(rr_file, false) || !graph.load(cr_file, true)) {
        return 1;
    }
    std::cout << "Done (" << timer.lap() << "s)" << std::endl << graph.stats() << std::endl;

    // Construct paths with following heuristics.
    std::cout << "Calculating paths..." << std::endl;
    PathManager pm;
    pm.params_.rebuild_attempts = pm_params.rebuild_attempts;
    pm.params_.backtrack_attempts = pm_params.backtrack_attempts;
    pm.params_.len_threshold = pm_params.len_threshold;
    pm.params_.node_num_threshold = pm_params.node_num_threshold;
    pm.params_.window_size = pm_params.window_size;
    pm.params_.ratio_threshold = pm_params.ratio_threshold;
    pm.buildMonteCarlo(graph, Utils::Metrics::EXTENSION_SCORE);
    pm.buildMonteCarlo(graph, Utils::Metrics::EXTENSION_SCORE_SQRT);
    pm.buildMonteCarlo(graph, Utils::Metrics::OVERLAP_SCORE);
    pm.buildMonteCarlo(graph, Utils::Metrics::OVERLAP_SCORE_SQRT);
    pm.buildDeterministic(graph, Utils::Metrics::EXTENSION_SCORE);
    pm.buildDeterministic(graph, Utils::Metrics::EXTENSION_SCORE_SQRT);
    pm.buildDeterministic(graph, Utils::Metrics::OVERLAP_SCORE);
    pm.buildDeterministic(graph, Utils::Metrics::OVERLAP_SCORE_SQRT);

    // Filter uniques.
    pm.filterUnique();
    std::cout << "Done (" << timer.lap() << "s)" << std::endl << pm.stats() << std::endl;

    // Map of anchor pairs and paths between those two anchors: [anchor1, anchor2] => {path1, path2, ...}
    auto paths_between_anchors = pm.getPathsBetweenAnchors();

    // Map of path groups between those two anchors: [anchor1, anchor2] => {group1, group2, ...}
    std::map<std::pair<const OverlapGraph::Node*, const OverlapGraph::Node*>,
            std::vector<PathGroup>> groups_for_anchors;

    for (auto& pbai : paths_between_anchors) { // Construct groups and fill groups_for_anchors map.
        const OverlapGraph::Node& anchor1 = *pbai.first.first;  // Begin anchor.
        const OverlapGraph::Node& anchor2 = *pbai.first.second; // End anchor.
        std::vector<const Path*>& paths = pbai.second;          // Paths connecting begin and end anchor.

        std::cout << "====> Constructing groups for paths between anchor '" << anchor1.name
            << "' and anchor '" << anchor2.name << "'..." << std::endl;
        std::vector<PathGroup> pgs = PathManager::constructGroups(paths, pm.params_);
        for (size_t i = 0; i < pgs.size(); i++) {
            std::cout << "-- Group " << i << " lengths --\n" << pgs[i] << "\n---------------------\n";
        }
        std::cout << "<==== Finished constructing groups for paths between anchor '"
            << anchor1.name << "' and anchor '" << anchor2.name << "'!\n" << std::endl;

        groups_for_anchors[{&anchor1, &anchor2}] = pgs; // Store all groups for the anchor in the map of path groups.
    }

    // Find a consensus for each pair of anchors.
    std::map<std::pair<const OverlapGraph::Node*, const OverlapGraph::Node*>,
            const Path*> consensus_for_anchors;

    for (auto& anchors_groups_pair : groups_for_anchors) { // Iterate over map, ([a1, a2], path_groups) pairs.
        const OverlapGraph::Node& anchor1 = *anchors_groups_pair.first.first;  // Begin anchor.
        const OverlapGraph::Node& anchor2 = *anchors_groups_pair.first.second; // End anchor.
        std::cout << "====> Finding consensus path in each group between anchor '" << anchor1.name
            << "' and anchor '" << anchor2.name << "'..." << std::endl;
        std::vector<PathGroup>& pgs = anchors_groups_pair.second;
        
        std::vector<PathGroup*> pgswc;     // Path groups with consensus (not all have it). Filled in the following for loop.
        for (PathGroup& pg : pgs) {        // Iterate over path groups.
            pg.discardNotFrequent();       // Discard infrequent paths in each group.
            pg.calculateConsensusPath();   // Calculate consensus path for this group and set it in pg object.
            pg.calculateValidPathNumber(); // Count number of paths equal to group consensus.

            if (pg.consensus) { // Group has consensus.
                std::cout << "Consensus length: "  << pg.consensus->length() << "    "
                          << "Valid path number: " << pg.valid_path_number   << std::endl;
                pgswc.push_back(&pg);
            } else {              // Consensus could not be calculated for the group.
                std::cout << "No consensus sequence for group:\n" << pg << std::endl;
            }
        }
        std::cout << "Done finding consensus path in each group.\n";
        
        // Calculate consensus among groups for this pair of anchors (final sequence connecting the anchors).
        if (pgswc.empty()) { // No consensus between anchors.
            consensus_for_anchors[{&anchor1, &anchor2}] = nullptr;
        } else if (pgswc.size() == 1) { // Only one group between this pair of anchors has consensus.
            consensus_for_anchors[{&anchor1, &anchor2}] = pgswc[0]->consensus;
        } else {
            // Sort path groups by consensus length in descending order.
            std::sort(pgswc.begin(), pgswc.end(),
                    [](const PathGroup* pgp1, const PathGroup* pgp2) {
                        return pgp1->consensus->length() > pgp2->consensus->length(); 
                    });
            if (pgswc.size() == 2) { // Only two groups with consensus between this pair of anchors.
                // Use longer path length as consensus for this region.
                consensus_for_anchors[{&anchor1, &anchor2}] = pgswc[0]->consensus; 
            } else { // There are more than two path groups with calculated consensus.
                // Go over consecutive group pairs and compare those groups by consensus length.
                const PathGroup* longer = pgswc[0]; // Group with longer consensus. Initially, group with longest consensus.
                for (size_t i = 1, n = pgswc.size(); i < n; i++) { // Go over consecutive pairs of path groups.
                    const PathGroup* shorter = pgswc[i]; // Path group with shorter consensus (out of two).

                    // Check if valid path number in the longer group is less or equal to half of the number in the shorter group.
                    if (2 * longer->valid_path_number <= shorter->valid_path_number) {
                        longer = shorter; // Shorter is the winner of this comparison and goes into the next round.
                    }
                }
                consensus_for_anchors[{&anchor1, &anchor2}] = longer->consensus; 
            }
        }

        // Log the final consensus lenght to the standard output.
        const Path* consensus = consensus_for_anchors.at({&anchor1, &anchor2}); // Final consensus.
        if (consensus) std::cout << "Final consensus (consensus among groups) length: " << consensus->length() << '\n';
        else           std::cout << "Final consensus not found!\n";
        std::cout << "====> Done finding consensus between anchor '" << anchor1.name
            << "' and anchor '" << anchor2.name << "'!" << std::endl;
    }


    // TODO Re-group based on current group scores (this may be misinterpreted).
    // SomeOtherFunc f1;
    // pm.constructGroups(f1);

    // TODO Filter rare paths (this may be misinterpreted).
    // pm.filterRare();

    // TODO Construct consensus paths.
//    std::cout << "Building the scaffold..." << std::endl;
    Path scaffold;
//    scaffold = pm.constructConsensusPath();

    // Load sequences for the final scaffold.
    std::cout << "\nLoading files for scaffolding..." << std::endl;
    Scaffolder scaff(scaffold);
    if (!scaff.load(rr_file) || !scaff.load(cr_file)) {
        return 1;
    }
    std::cout << "Done (" << timer.lap() << "s)" << std::endl;

    // Test scaffolder. Result should be: "EFGHI1234"
//    Path scaffold;
//    scaffold.nodes_.push_back(new OverlapGraph::Node(true, 0, 5, "0"));
//    scaffold.nodes_.push_back(new OverlapGraph::Node(true, 1, 5, "1"));
//    scaffold.nodes_.push_back(new OverlapGraph::Node(true, 2, 5, "2"));
//    scaffold.nodes_.push_back(new OverlapGraph::Node(true, 3, 5, "3"));
//    scaffold.edges_.push_back(new OverlapGraph::Edge(0, 1, 3, 3, 0, 0, 0, 0, 0));
//    scaffold.edges_.push_back(new OverlapGraph::Edge(1, 2, 1, 2, 0, 1, 0, 0, 0));
//    scaffold.edges_.push_back(new OverlapGraph::Edge(2, 3, 3, 4, 3, 4, 0, 0, 0));
//
//    Scaffolder scaff(scaffold);
//    scaff.names_.push_back("0");
//    scaff.names_.push_back("1");
//    scaff.names_.push_back("2");
//    scaff.names_.push_back("3");
//    scaff.sequences_.push_back("01234");
//    scaff.sequences_.push_back("56789");
//    scaff.sequences_.push_back("0ABCD");
//    scaff.sequences_.push_back("EFGHI");

    // Write result to a file.
    std::cout << "Writing result to: " << RESULT_FILE << std::endl;
    if (!scaff.write(RESULT_FILE)) {
        return 1;
    }
    std::cout << "Done (" << timer.lap() << "s)" << std::endl;

    std::cout << "Total time: " << timer.stop() << "s" << std::endl;
    return 0;
}
