#include <iostream>
#include <vector>
#include <argparser/ArgParser.hpp>

#include "ApiHandler.hpp"

int main(int argc, char** argv) {
    ArgumentParser::ArgParser argparser{"WayHome", "A tool for finding routes from city A to city B. "
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

    

    return EXIT_SUCCESS;
}
