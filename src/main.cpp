#include "TrussDecomposition.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

struct Options {
    std::string graph_file;
    std::string output_file;
    bool one_based = false;
};

void printUsage(const char* program) {
    std::cerr << "Usage: " << program << " <graph-file> [--output file] [--one-based]\n";
}

Options parseArgs(int argc, char** argv) {
    if (argc < 2) {
        throw std::invalid_argument("missing required graph file");
    }

    Options options;
    options.graph_file = argv[1];
    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--output") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--output requires a file path");
            }
            options.output_file = argv[++i];
        } else if (arg == "--one-based") {
            options.one_based = true;
        } else if (arg == "--zero-based") {
            options.one_based = false;
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            std::exit(0);
        } else {
            throw std::invalid_argument("unknown argument: " + arg);
        }
    }
    return options;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const Options options = parseArgs(argc, argv);
        const auto start = std::chrono::steady_clock::now();
        const ktruss::GraphData graph = ktruss::readGraphFile(options.graph_file, options.one_based);
        const ktruss::DecompositionResult result = ktruss::decompose(graph);
        const auto elapsed = std::chrono::steady_clock::now() - start;
        const double milliseconds = std::chrono::duration<double, std::milli>(elapsed).count();

        std::cout << "|V|\t|E|\ttime(ms)\tkmax\n";
        std::cout << result.vertex_count << '\t'
                  << result.edge_count << '\t'
                  << std::fixed << std::setprecision(3) << milliseconds << '\t'
                  << result.kmax << '\n';

        if (!options.output_file.empty()) {
            ktruss::writeEdgeTrussFile(options.output_file, result);
            std::cout << "Wrote edge truss values to " << options.output_file << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}
