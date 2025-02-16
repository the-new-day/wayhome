#include "WayHome.hpp"

#include <argparser/ArgParser.hpp>

#include <iostream>
#include <vector>
#include <fstream>

int main(int argc, char** argv) {
    WayHome::ApiRouteParameters params;

    ArgumentParser::ArgParser argparser{"WayHome", "A util for finding routes from city A to city B. "
        "Uses Yandex Schedules API (http://rasp.yandex.ru/)"};

    argparser.AddArgument<std::string>("from", "Departure point: Yandex Schedules code")
        .StoreValue(params.from);

    argparser.AddArgument<std::string>("to", "Arrival point: Yandex Schedules code")
        .StoreValue(params.to);

    argparser.AddArgument<std::string>("date", "Date of departure in \"YYYY-MM-DD\" format")
        .StoreValue(params.date);

    argparser.AddArgument<uint32_t>("limit", "Maximum number of routes in the response")
        .Default(10)
        .StoreValue(params.limit);

    argparser.AddArgument<uint32_t>("transfers", "Maximum number of transfers")
        .Default(1)
        .StoreValue(params.max_transfers);
    
    argparser.AddArgument<std::string>("transport", "Transport type")
        .Default("")
        .StoreValue(params.transport_type)
        .SetDefaultValueString("all");

    argparser.AddArgument<std::string>("file", "Name of the JSON file routes will be stored to")
        .Default(WayHome::kResultFilename);

    argparser.AddFlag("update-cache", "Force to make a new call to API even if suitable routes are cached");
    argparser.AddFlag("clear-cache", "Clear all cache");
        
    argparser.AddHelp("help", "Show help and exit");

    if (!argparser.Parse(argc, argv)) {
        if (argc == 1) {
            std::cout << argparser.HelpDescription() << std::endl;
            return EXIT_SUCCESS;
        }

        ArgumentParser::ParsingError error = argparser.GetError();

        if (error.status == ArgumentParser::ParsingErrorType::kUnknownArgument) {
            std::cerr << "Unknown argument: " << error.argument_string << std::endl;
            return 1;
        }

        std::cerr << "An error occured while parsing the following argument: " 
            << (error.argument_name) << std::endl;

        if (error.status == ArgumentParser::ParsingErrorType::kInvalidArgument) {
            std::cerr << "Unable to parse: " << error.argument_string << std::endl;
        } else if (error.status == ArgumentParser::ParsingErrorType::kNoArgument) {
            std::cerr << "No value was specified" << std::endl;
        } else if (error.status == ArgumentParser::ParsingErrorType::kInsufficent) {
            std::cerr << "Not enough values were specified" << std::endl;
        }

        return EXIT_FAILURE;
    } else if (argparser.Help()) {
        std::cout << argparser.HelpDescription() << std::endl;
        return EXIT_SUCCESS;
    }

    WayHome::WayHome wayhome{params};
    wayhome.CalculateRoutes();
    wayhome.DumpRoutesToJson(*argparser.GetValue<std::string>("file"));

    if (wayhome.HasError()) {
        std::cerr << "An error occured: " << wayhome.GetError().message << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
