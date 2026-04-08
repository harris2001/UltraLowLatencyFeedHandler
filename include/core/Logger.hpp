#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <atomic>

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
     *                              (options: trace < debug < info < warn < error < critical < off) 
     */
    static void init(const std::string& name = "ullfh",
                     spdlog::level::level_enum level = spdlog::level::info) noexcept {
        try {
            auto sink = std::ma6ke_shared<spdlog::sinks::stdout_color_sink_st>();
            sink->set_level(level);
            
            std::vector<spdlog::sink_ptr> sinks{sink};
            auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
            logger->set_level(level);
            logger->flush_on(spdlog::level::err);
            
            spdlog::register_logger(logger);
            spdlog::set_default_logger(logger);
            // We need to use memory_order_release here to guarantee that any thread that reads s_logger 
            // will see a fully initialized logger instance.
            s_logger.store(logger.get(), std::memory_order_release);
            s_logger_owner = logger;

            logger->info("Logger initialized with level: {}", spdlog::level::to_string_view(level));
        } catch (const spdlog::spdlog_ex& ex) {
            // Silent: (I don't want logging infrastructure to crash the feed handler)
        }
    }

    /**
     * Get the global logger instance.
     */
    static std::shared_ptr<spdlog::logger> get() noexcept {
        if (s_logger_owner) [[likely]] {
            return s_logger_owner;
        }
        s_logger_owner = spdlog::stdout_color_st("ullfh");
        s_logger.store(s_logger_owner.get(), std::memory_order_release);
        return s_logger_owner;
    }

    /**
     * Get a raw (non-owning) pointer to the logger.
     */
    [[nodiscard]] static spdlog::logger* get_raw() noexcept {
        // And here memory_order_acquire ensures that the s_logger pointer is initialized before 
        // any thread acquire and read it.
        return s_logger.load(std::memory_order_acquire);
    }

    /**
     * Shutdown and flush all loggers.
     */
    static void shutdown() noexcept {
        if (s_logger_owner) { s_logger_owner->flush(); }
        s_logger.store(nullptr, std::memory_order_release);
        s_logger_owner.reset();
        spdlog::shutdown();
    }

   private:
    // Private constructor to prevent instantiation
    Logger() = delete;
    ~Logger() = delete;

    // Once init() is done, then use the raw s_logger pointer for zero-cost access.
    inline static std::atomic<spdlog::logger*> s_logger{nullptr};
    // s_logger_owner keeps the shared_ptr alive.
    inline static std::shared_ptr<spdlog::logger> s_logger_owner;
};

}  // namespace ullfh::core

// ---- Logging macros ----
// Use get_raw() instead of get() to avoid shared_ptr
// In Release builds we compile out TRACE/DEBUG/INFO/WARN entirely to remove overhead from the hot path.
// But we retain ERROR and CRITICAL to catch faults.

#ifdef ULLFH_DIAGNOSTICS
#define ULLFH_LOG_TRACE(...)    ::ullfh::core::Logger::get_raw()->trace(__VA_ARGS__)
#define ULLFH_LOG_DEBUG(...)    ::ullfh::core::Logger::get_raw()->debug(__VA_ARGS__)
#define ULLFH_LOG_INFO(...)     ::ullfh::core::Logger::get_raw()->info(__VA_ARGS__)
#define ULLFH_LOG_WARN(...)     ::ullfh::core::Logger::get_raw()->warn(__VA_ARGS__)
#else
#define ULLFH_LOG_TRACE(...)    do {} while(0)
#define ULLFH_LOG_DEBUG(...)    do {} while(0)
#define ULLFH_LOG_INFO(...)     do {} while(0)
#define ULLFH_LOG_WARN(...)     do {} while(0)
#endif

#define ULLFH_LOG_ERROR(...)    ::ullfh::core::Logger::get_raw()->error(__VA_ARGS__)
#define ULLFH_LOG_CRITICAL(...) ::ullfh::core::Logger::get_raw()->critical(__VA_ARGS__)

// Error-specific macros - only used for diagnostics
#ifdef ULLFH_DIAGNOSTICS
#define ULLFH_LOG_PARSE_ERROR(code, msg) \
    ::ullfh::core::Logger::get()->warn("Parse error: {} - {}", \
        ::ullfh::core::error_string(code), msg)
#define ULLFH_LOG_QUEUE_ERROR(code) \
    ::ullfh::core::Logger::get()->warn("Queue error: {}", \
        ::ullfh::core::error_string(code))
#define ULLFH_LOG_BUFFER_ERROR(code) \
    ::ullfh::core::Logger::get()->warn("Buffer error: {}", \
        ::ullfh::core::error_string(code))
#else
#define ULLFH_LOG_PARSE_ERROR(code, msg)  do {} while(0)
#define ULLFH_LOG_QUEUE_ERROR(code)       do {} while(0)
#define ULLFH_LOG_BUFFER_ERROR(code)      do {} while(0)
#endif
