#pragma once

#define TRACE(...)  utils::log(utils::LogLevel::LOG_TRACE,   __FILE__, __LINE__, __VA_ARGS__)
#define DBG(...)    utils::log(utils::LogLevel::LOG_DEBUG,   __FILE__, __LINE__, __VA_ARGS__)
#define WARN(...)   utils::log(utils::LogLevel::LOG_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define ERR(...)    utils::log(utils::LogLevel::LOG_ERROR,   __FILE__, __LINE__, __VA_ARGS__)

#ifndef POISON_LOG_SYNCHRONIZED
    #define POISON_LOG_SYNCHRONIZED 0
#endif


#include <sstream>
#include <boost/format.hpp>

#if POISON_LOG_SYNCHRONIZED
    #ifdef __ANDROID__
        #include <boost/thread/mutex.hpp>
        #include <boost/thread/lock_guard.hpp>
    #else
        #include <mutex>
    #endif
#endif

#ifdef __ANDROID__
    #include <android/log.h>
#endif

namespace poison { namespace utils {
    static LogLevel logLevel = LogLevel::LOG_DEBUG;

#if POISON_LOG_SYNCHRONIZED
    #ifdef __ANDROID__
        typedef boost::mutex Mutex;
        typedef boost::lock_guard<Mutex> Lock;
    #else
        typedef std::mutex Mutex;
        typedef std::lock_guard<Mutex> Lock;
    #endif

    static Mutex logMutex;
#endif

    class enum LogLevel {
        LOG_NULL = 0, 
        LOG_ERROR, 
        LOG_WARNING, 
        LOG_DEBUG, 
        LOG_TRACE
    };

    static void setLogLevel(LogLevel l) {
        logLevel = l;
    }

    static std::string formatString(boost::format& message) {
        return message.str();
    }
    
    template<typename TValue, typename... TArgs>
    static std::string formatString(boost::format& message, TValue arg, TArgs... args) {
        message % arg;
        return formatString(message, args...);
    }
    
    template<typename... TArgs>
    static std::string formatString(const std::string& fmt, TArgs... args) {
        boost::format message(fmt);
        return formatString(message, args...);
    }
    
    template<typename ...Args>
    static void log(LogLevel priority, const char * fileName, int lineNumber, const char * format, Args ... args) {
#if POISON_LOG_SYNCHRONIZED
        Lock lock(logMutex);
#endif

        if (priority > logLevel) {
            return;
        }
        
        static const char *LogLevel_str[] = {"", "E", "W", "D", "T"};

        std::ostringstream stream;
        stream  << currentDateTime().c_str() << " [ " << LogLevel_str[int(priority)] << "] " << formatString(format, args...) << std::endl;
        
#ifdef __APPLE__
        std::cout << stream.str();
#elif __ANDROID__

        android_LogPriority aPriority = android_LogPriority::ANDROID_LOG_UNKNOWN;
        switch(priority){
            case LogLevel::LOG_TRACE:
                aPriority = android_LogPriority::ANDROID_LOG_INFO;
            break;

            case LogLevel::LOG_DEBUG:
                aPriority = android_LogPriority::ANDROID_LOG_DEBUG;
            break;

            case LogLevel::LOG_WARNING:
                aPriority = android_LogPriority::ANDROID_LOG_WARN;
            break;

            case LogLevel::LOG_ERROR:
                aPriority = android_LogPriority::ANDROID_LOG_ERROR;
            break;

            default:
                aPriority = android_LogPriority::ANDROID_LOG_UNKNOWN;
            break;
        }

        __android_log_write(ANDROID_LOG_INFO, ndk_helper::JNIHelper::GetInstance()->GetAppName(), stream.str() );

#endif
        
    }

} }