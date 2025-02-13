#include <iostream>
#include <cpr/cpr.h>
#include "argparser/ArgParser.hpp"

int main(int argc, char** argv) {
    ArgumentParser::ArgParser argparser{"WayHome", "A tool for finding routes from city A to city B"};
    argparser.AddHelp("help", "Show help");

    if (!argparser.Parse(argc, argv)) {
        std::cout << "ERROR!" << std::endl;
    } else if (argparser.Help()) {
        std::cout << argparser.HelpDescription() << std::endl;
    }

    return EXIT_SUCCESS;
}
