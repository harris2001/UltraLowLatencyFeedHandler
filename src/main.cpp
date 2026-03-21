#include <iostream>
#include <string>
#include <unordered_map>
#include "core/Logger.hpp"

#define VERSION "0.1"

spdlog::level::level_enum parse_log_level(const std::string& level_str) {
    static const std::unordered_map<std::string, spdlog::level::level_enum> level_map = {
        {"trace", spdlog::level::trace},
        {"debug", spdlog::level::debug},
        {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},
        {"err", spdlog::level::err},
        {"critical", spdlog::level::critical},
        {"off", spdlog::level::off}
    };
    
    auto it = level_map.find(level_str);
    return (it != level_map.end()) ? it->second : spdlog::level::info;
}

int main(int argc, char* argv[]) {
    std::string log_level = "info";
    if (argc > 1) {
        log_level = argv[1];
    }
    
	using namespace ullfh::core;
    Logger::init("ullfh", parse_log_level(log_level));
    
    auto logger = Logger::get();
    logger->info("Logger initialized successfully.");
    Logger::shutdown();
    return 0;
}
