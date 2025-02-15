#include "ApiHandler.hpp"
#include "RoutesHandler.hpp"

#include <argparser/ArgParser.hpp>

#include <iostream>
#include <vector>
#include <fstream>

int main(int argc, char** argv) {
    ArgumentParser::ArgParser argparser{"WayHome", "A util for finding routes from city A to city B. "
        "Uses Yandex Schedules API (http://rasp.yandex.ru/)"};

    argparser.AddArgument<std::string>("apikey", "Yandex Schedules API key");

    argparser.AddArgument<std::string>("from", "Departure point: Yandex Schedules code");
    argparser.AddArgument<std::string>("to", "Arrival point: Yandex Schedules code");
    argparser.AddArgument<std::string>("date", "Date of departure in \"YYYY-MM-DD\" format");

    argparser.AddArgument<uint32_t>("limit", "Maximum number of routes in the response")
        .Default(10);

    argparser.AddArgument<uint32_t>("transfers", "Maximum number of transfers")
        .Default(1);
    
    argparser.AddArgument<std::string>("transport", "Transport type")
        .Default("")
        .SetDefaultValueString("all");

    argparser.AddArgument<std::string>("file", "Name of the file routes will be stored to")
        .Default("wayhome_routes.json");
        
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

    WayHome::ApiHandler api{
        *argparser.GetValue<std::string>("apikey"),
        *argparser.GetValue<std::string>("from"),
        *argparser.GetValue<std::string>("to")
    };

    api.SetDate(*argparser.GetValue<std::string>("date"));
    api.SetTransportType(*argparser.GetValue<std::string>("transport"));

    api.SetMaxTransfers(*argparser.GetValue<uint32_t>("transfers"));
    api.SetRoutesLimit(*argparser.GetValue<uint32_t>("limit"));

    if (!api.AreParametersOk()) {
        std::cerr << "An error occured: invalid parameter values" << std::endl;
        return EXIT_FAILURE;
    }

    std::expected<json, WayHome::Error> request = api.MakeRequest();
    
    if (!request.has_value()) {
        std::cerr << "An error occured: " << request.error().message << std::endl;
        return EXIT_FAILURE;
    }

    WayHome::RoutesHandler routes_handler;
    
    if (!routes_handler.BuildFromJson(request.value())) {
        std::cerr << "An error occured: unable to parse routes from JSON" << std::endl;
        return EXIT_FAILURE;
    }

    std::ofstream f(*argparser.GetValue<std::string>("file"));

    if (!f.good()) {
        std::cerr << "An error occured: unable to open the file" << std::endl;
        return EXIT_FAILURE;
    }

    routes_handler.DumpRoutesToJson(f, *argparser.GetValue<uint32_t>("transfers"));
    return EXIT_SUCCESS;
}
