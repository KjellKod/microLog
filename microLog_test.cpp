/// microLog_test.cpp

// clang++ -std=c++11 ./microLog_test.cpp
// clang++ -std=c++11 -stdlib=libc++ ./microLog_test.cpp

// These macros can also be defined in the makefile, or in microLog_config.hpp:
/*
#define MICRO_LOG_ACTIVE 1
#define MICRO_LOG_MIN_LEVEL 4
*/

#ifdef MICRO_LOG_TEST

#define MICRO_LOG_ACTIVE 1
#include "microLog.hpp"

#include <iostream>
#include <string>

uLOG_INIT;     // microLog initialization


int main()
{
    std::string logPath;
    logPath.append("/Volumes/ramdisk/");
    logPath.append("myProg.log");

    std::cout << "Test version:  " << VERSION << std::endl;
    std::cout << "Log file path: " << logPath << std::endl;

    uLOG_START_APP(logPath);

    uLOG_DATE;              // date
    uLOG_TITLES(info);		// columns' titles

    uLog::LogLevels();

    uLog::MinLogLevel();
    uLog::minLogLevel = info;
    uLog::MinLogLevel();

    uLOG(info) << "Test log message number " << 2 << " with value " << 3.141 << uLOGE;
    uLOG(detail) << "Log not generated." << uLOGE;

    uLog::minLogLevel = warning;
    uLog::MinLogLevel();

/*
//+TODO
    uLOG(warning) << "Log made of separate tokens... ";
    uLOGT(warning) << "first token, ";
    uLOGT(warning) << "last token" << uLOGE;
*/
    uLOG(info) << "Log not generated." << uLOGE;
    uLOG(error) << "Test Log." << uLOGE;
    uLOG_(detail, logQSExperiment) << "Test minimum log levels for specific code areas: not generated." << uLOGE;
    uLOG_(error, logInfo) << "Test minimum log levels for specific code areas." << uLOGE;
    uLOG_(info, logInfo) << "Test minimum log levels for specific code areas." << uLOGE;
    uLOG_(detail, logGPSolver) << "Test minimum log levels for specific code areas." << uLOGE;

	for(int i = 1; i < 10; ++i) {
        uLOG(i) << "Test log message number " << i << " with value " << 1.23*i << uLOGE;
	}

    uLog::LogFields::SetVerbose();
    uLOG_TITLES(warning);		// columns' titles
    uLOG(error) << "Test unified logging " << 0.123 << uLOGE;
    uLOG_(error, logInfo) << "Test unified logging " << 1.23 << uLOGE;

    logLevelVar = detail;
    uLOG_(error, logLevelVar) << "Test variable log threshold " << 1 << uLOGE;

    logLevelVar = error;
    uLOG_(error, logLevelVar) << "Test variable log threshold " << 2 << uLOGE;



    uLog::Statistics::Log();
}

#endif // MICRO_LOG_TEST
