#ifndef LOG_H
#define LOG_H

#include <exception>
#include <string>


namespace logger
{
    using message = std::string;
    using pathstr = std::string;

    
    extern pathstr     dir; // "./log" by default.
    extern pathstr     log; // "session.log" by default.
    extern std::string timestamp_format; // "%f %T%z" by default.  Uses std::strftime format.


    void write(message, pathstr out = log, std::string searchable = "");

    void info(message, pathstr out = log, std::string searchable = "");

    void error(message, pathstr out = log, std::string searchable = "");

    void exception(std::exception, pathstr out = log, std::string searchable = "");
    
    void debug(message, pathstr out = log, std::string searchable = "");

    void debug(const void*, size_t, pathstr out = log, std::string searchable = "");
}

#endif
