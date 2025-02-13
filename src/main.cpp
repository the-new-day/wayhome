#include <iostream>
#include <vector>
#include <argparser/ArgParser.hpp>

#include "WayHome.hpp"

int main(int argc, char** argv) {
    ArgumentParser::ArgParser argparser{"WayHome", "A tool for finding routes from city A to city B. "
        "Uses Yandex Schedules API (http://rasp.yandex.ru/)"};

    argparser.AddArgument<std::string>("apikey", "Yandex Schedules API key");

    argparser.AddArgument<std::string>("from", "Departure point: Yandex Schedules code or a name");
    argparser.AddArgument<std::string>("to", "Arrival point: Yandex Schedules code or a name");
    argparser.AddArgument<std::string>("date", "Date of departure in ISO 8601 format, e.g. YYYY-MM-DD");

    argparser.AddArgument<int32_t>("limit", "Maximum number of routes in the response")
        .Default(10);
    argparser.AddArgument<int32_t>("transfers", "Maximum number of transfers")
        .Default(1);
    
    argparser.AddArgument<std::string>("transport", "Transport type")
        .Default("")
        .SetDefaultValueString("all");
        
    argparser.AddHelp("help", "Show help and exit");

    if (!argparser.Parse(argc, argv)) {
        if (argc == 1) {
            std::cout << argparser.HelpDescription() << std::endl;
            return EXIT_SUCCESS;
        }

        std::cout << "ERROR!" << std::endl;
        return EXIT_FAILURE;
    } else if (argparser.Help()) {
        std::cout << argparser.HelpDescription() << std::endl;
        return EXIT_SUCCESS;
    }

    WayHome::WayHome app(*argparser.GetValue<std::string>("apikey"));
    
    std::vector<std::vector<WayHome::ThreadPoint>> routes 
        = app.GetRoutes();

    return EXIT_SUCCESS;
}
