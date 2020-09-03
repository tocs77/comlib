#include <Windows.h>
#include <iomanip>

#include "Logger.h"


namespace Logger {
    inline Log* get_glog() {
        static std::unique_ptr<Log> glog = std::make_unique<Log>();
        return glog.get();
    }

    void startLog(const std::string& filepath) {
        Log* glog = get_glog();
        glog->initLog(filepath);
    }


    // log methods to log messages
    void logInfo(const std::string& msg) {
        
        get_glog()->logInfo(msg);
    }
    void logWarning(const std::string& msg) {

        get_glog()->logWarning(msg);
    }
    void logError(const std::string& msg) {

        get_glog()->logError(msg);
    }

  

    Log::Log() : m_logfile{} {
 
    }

    void Log::initLog(const std::string& filepath) {
        m_logfile.open(filepath);
        logInfo("Logging started");
    }

    // Add a message to our log.
    void Log::addLog(const std::string& level, const std::string& msg) {
        if (m_logfile.is_open()) {
            SYSTEMTIME time;
            GetLocalTime(&time);

            m_logfile << " [" << time.wYear << '.'
                << std::setfill('0') << std::setw(2) << time.wMonth << '.'
                << std::setfill('0') << std::setw(2) << time.wDay << ' '
                << std::setfill(' ') << std::setw(2) << time.wHour << ':'
                << std::setfill('0') << std::setw(2) << time.wMinute << ':'
                << std::setfill('0') << std::setw(2) << time.wSecond << '.'
                << std::setfill('0') << std::setw(3) << time.wMilliseconds << "] ";
            m_logfile << level << ": " << msg << std::endl;
        }
    }


    void Log::logInfo(const std::string& msg) {
        addLog("Information", msg);
    }

    void Log::logWarning(const std::string& msg) {
        addLog("Warning", msg);
    }

    void Log::logError(const std::string& msg) {
        addLog("Error", msg);
    }

    Log::~Log() {
        logInfo("Stopped logging system.");
        m_logfile.close();
    }
}