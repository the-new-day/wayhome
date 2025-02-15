#include <iostream>
#include <vector>
#include <argparser/ArgParser.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "ApiHandler.hpp"
#include "RoutesHandler.hpp"

#include <fstream>

int main(int argc, char** argv) {
    //setlocale(LC_ALL, "Russian");

    // std::ofstream f("test.txt");
    // f << "привет";

    // return 0;
    
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

    std::ofstream f("test.json");

    routes_handler.DumpRoutesToJson(f);
    return 0;

    std::cout << "ROUTES (" << routes_handler.GetRoutes().size() << ")\n";

    uint32_t transfers_limit = *argparser.GetValue<uint32_t>("limit");

    for (const WayHome::Route& route : routes_handler.GetRoutes()) {
        const WayHome::RoutePoint& start_point = route.GetStartPoint();
        const WayHome::RoutePoint& end_point = route.GetEndPoint();

        f << start_point.title << " - ";

        const std::vector<WayHome::Transfer>& transfers = route.GetTransfers();

        if (route.GetTransfersAmount() <= transfers_limit) {
            for (const WayHome::Transfer& transfer : transfers) {
                f << "(" << transfer.transfer_point.title << ": "
                    << transfer.station1.title;

                if (transfer.station2.code != transfer.station1.code) {
                    f << " - " << transfer.station2.title;
                }

                f << " - " << transfer.duration << " seconds transfer)";
                f << " - ";
            }
        }

        f << end_point.title;

        if (transfers.size() != 0) {
            f << " (" << transfers.size() << " transfer";
            if (transfers.size() != 1) {
                f << 's';
            }

            f << ')';
        }
        
        f << '\n';

        std::cout << start_point.title << " - " << end_point.title << std::endl;
        std::cout << '\t' << route.GetDepartureTime() << " - " << route.GetArrivalTime() << std::endl;
    }

    return EXIT_SUCCESS;
}
