#include "WayHome.hpp"

#include <argparser/ArgParser.hpp>

#include <iostream>
#include <vector>

void SetParserAgruments(ArgumentParser::ArgParser& argparser, WayHome::ApiRouteParameters& params);
bool HandleParserErrors(const ArgumentParser::ArgParser& argparser);

int main(int argc, char** argv) {
    WayHome::ApiRouteParameters params;

    ArgumentParser::ArgParser argparser{"WayHome", "A util for finding routes from city A to city B. "
        "Uses Yandex Schedules API (http://rasp.yandex.ru/)"};
        
    SetParserAgruments(argparser, params);
    argparser.Parse(argc, argv);

    if (argparser.Help() || argc == 1) {
        std::cout << argparser.HelpDescription() << std::endl;
        return EXIT_SUCCESS;
    }
    
    if (!HandleParserErrors(argparser)) {
        return EXIT_FAILURE;
    }

    WayHome::WayHome wayhome{params};

    if (*argparser.GetValue<bool>("clear-cache")) {
        wayhome.ClearAllCache();
        if (!wayhome.HasError()) {
            std::cout << "Cache cleared successfully\n";
        }
    }

    if (*argparser.GetValue<bool>("update-cache")) {
        wayhome.UpdateRoutesWithAPI();
    } else {
        wayhome.CalculateRoutes();
    }

    if (*argparser.GetValuesSet("file") != 0) {
        wayhome.DumpRoutesToJson(*argparser.GetValue<std::string>("file"));
        std::cout << "Routes have successfully been saved to " + *argparser.GetValue<std::string>("file") << std::endl;
    }

    wayhome.DumpRoutesPretty(std::cout);

    if (wayhome.HasError()) {
        std::cerr << wayhome.GetError().message << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void SetParserAgruments(ArgumentParser::ArgParser& argparser, WayHome::ApiRouteParameters& params) {
    argparser.AddArgument<std::string>("from", "Departure point: Yandex Schedules code")
        .StoreValue(params.from);

    argparser.AddArgument<std::string>("to", "Arrival point: Yandex Schedules code")
        .StoreValue(params.to);

    argparser.AddArgument<std::string>("date", "Date of departure in \"YYYY-MM-DD\" format")
        .StoreValue(params.date);

    argparser.AddArgument<uint32_t>("transfers", "Maximum number of transfers")
        .Default(1)
        .StoreValue(params.max_transfers);
    
    argparser.AddArgument<std::string>("transport", "Transport type")
        .Default("")
        .StoreValue(params.transport_type)
        .SetDefaultValueString("all");

    argparser.AddArgument<std::string>("file", "Name of the JSON file where the routes will be stored rather than printed")
        .Default("none");

    argparser.AddFlag("update-cache", "Force to make a new call to API even if suitable routes are cached");
    argparser.AddFlag("clear-cache", "Clear all cache before calculation");
        
    argparser.AddHelp("help", "Show help and exit");
}

bool HandleParserErrors(const ArgumentParser::ArgParser& argparser) {
    if (!argparser.HasError()) {
        return true;
    }

    ArgumentParser::ParsingError error = argparser.GetError();

    if (error.status == ArgumentParser::ParsingErrorType::kUnknownArgument) {
        std::cerr << "Unknown argument: " << error.argument_string << std::endl;
        return false;
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

    return false;
}
