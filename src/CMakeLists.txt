add_executable(${PROJECT_NAME} 
    main.cpp
    Route.cpp
    ApiHandler.cpp
    RoutesHandler.cpp
    CacheHandler.cpp
    CodeSearcher.cpp
    WayHome.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/libs)

target_link_libraries(${PROJECT_NAME} PRIVATE argparser)

include(FetchContent)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git)
FetchContent_MakeAvailable(cpr)

target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
