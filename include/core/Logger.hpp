#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

namespace ullfh::core {

/**
 * Global logger instance for the feed handler.
 * Thread-safe singleton wrapper around spdlog.
 */
class Logger {
   public:
    /**
     * Initialize the global logger with spdlog.
     * Should be called once at startup before any logging.
     *
     * @param name        logger name (default: "ullfh")
     * @param level       log level (default: info)
     */
    static void init(const std::string& name = "ullfh",
                     spdlog::level::level_enum level = spdlog::level::info) noexcept {
        try {
            auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            sink->set_level(level);
            
            std::vector<spdlog::sink_ptr> sinks{sink};
            auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
            logger->set_level(level);
            logger->flush_on(spdlog::level::err);
            
            spdlog::register_logger(logger);
            spdlog::set_default_logger(logger);
        } catch (const spdlog::spdlog_ex& ex) {
            // Silent: (I don't want logging infrastructure to crash the feed handler)
        }
    }

    /**
     * Get the global logger instance.
     */
    static std::shared_ptr<spdlog::logger> get() noexcept {
        auto logger = spdlog::get("ullfh");
        if (!logger) {
            logger = spdlog::stdout_color_mt("ullfh");
        }
        return logger;
    }

    /**
     * Shutdown and flush all loggers.
     */
    static void shutdown() noexcept {
        spdlog::shutdown();
    }
};

}  // namespace ullfh::core

#define ULLFH_LOG_TRACE(...) \
    ::ullfh::core::Logger::get()->trace(__VA_ARGS__)

#define ULLFH_LOG_DEBUG(...) \
    ::ullfh::core::Logger::get()->debug(__VA_ARGS__)

#define ULLFH_LOG_INFO(...) \
    ::ullfh::core::Logger::get()->info(__VA_ARGS__)

#define ULLFH_LOG_WARN(...) \
    ::ullfh::core::Logger::get()->warn(__VA_ARGS__)

#define ULLFH_LOG_ERROR(...) \
    ::ullfh::core::Logger::get()->error(__VA_ARGS__)

#define ULLFH_LOG_CRITICAL(...) \
    ::ullfh::core::Logger::get()->critical(__VA_ARGS__)

// Error-specific macros
#define ULLFH_LOG_PARSE_ERROR(code, msg) \
    ::ullfh::core::Logger::get()->warn("Parse error: {} - {}", \
        ::ullfh::core::error_string(code), msg)

#define ULLFH_LOG_QUEUE_ERROR(code) \
    ::ullfh::core::Logger::get()->warn("Queue error: {}", \
        ::ullfh::core::error_string(code))

#define ULLFH_LOG_BUFFER_ERROR(code) \
    ::ullfh::core::Logger::get()->warn("Buffer error: {}", \
        ::ullfh::core::error_string(code))
