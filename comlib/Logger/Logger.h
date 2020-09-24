#pragma once
#ifndef LOGGER_H
#define LOGGER_H

/*
Use it by including this header in every file required, and in your main function start a new log.
	Logger::startLog("Log.txt");
Use the various error levels by naming them and simply passing the info and what you want to output.
	Logger::logInfo("Some information.");
*/

#include <memory>
#include <fstream>
#include <string>
#include <mutex>



namespace Logger {


	// Initialize the log.
	void startLog(const std::string& filepath);

	// Log a message.
	void logInfo(const std::string& msg);
	void logWarning(const std::string& msg);
	void logError(const std::string& msg);

	// Logging class.
	class Log {
	public:
		Log();

		void logInfo(const std::string& msg);
		void logWarning(const std::string& msg);
		void logError(const std::string& msg);
		void initLog(const std::string& filepath);

		~Log();
	private:
		// File for logging.
		std::ofstream m_logfile;
		void addLog(const std::string& level, const std::string& msg);
		std::mutex mtx;
	};

	
	std::string ByteArrayToHex(BYTE* buf, DWORD buf_len);
}


#endif