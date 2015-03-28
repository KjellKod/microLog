/// microLog_test.cpp

// clang++ -std=c++11 ./microLog_test.cpp
// clang++ -std=c++11 -stdlib=libc++ ./microLog_test.cpp

// These macros can also be defined in the makefile, or in microLog_config.hpp:
/*
#define MICRO_LOG_ACTIVE 1
#define MICRO_LOG_MIN_LEVEL 4
*/

#ifdef MICRO_LOG_TEST

#include "microLog.hpp"

#include <cmath>
#include <iostream>
#include <string>

uLOG_INIT;     // microLog initialization



int Test_microLog(std::string logPath, int nTestCases = 1)
{
    /// Tests:
    // - Plain tests
    // - Multithreading tests - TODO
    // - Complex tests - TODO
    // - Border line tests - TODO
    // - Performace tests - TODO

    using std::sin;

    ///--- TEST INIT ---

    uLOG_START_APP(logPath);

    // Test with custom log file
    std::ofstream custom_ofs("/Volumes/ramdisk/custom.log");

    uLOG_DATE;              // date

    uLog::LogLevels();

    uLog::minLogLevel = nolog;
    uLog::MinLogLevel();

    ///--- TEST CODE ---

    for(size_t n = 0; n < nTestCases; ++n)
    {
        uLog::LogFields::SetSystem();
        uLOG_TITLES(info);		// columns' titles

        uLog::minLogLevel = nolog;

        for(size_t l = nolog; l <= fatal; ++l)
        {
            uLOG(l) << "Test log message with level " << l + 1 << "." << uLOGE;
        }

        uLOG(info) << "Test insertion operator: " << char((n + 65)%255) << " " << n << " " << sin(n + 1.0) << uLOGE;

        uLog::minLogLevel = warning;

        uLOG(detail) << "Log not generated, since below the minimum log level." << uLOGE;
        uLOG(warning) << "Log generated, since above the minimum log level." << uLOGE;

        uLog::minLogLevel = warning;
        uLog::logLevelVar = warning;
        uLOG_(detail, MICRO_LOG_LEVEL1) << "Test minimum log levels for specific code areas with macros: not generated." << uLOGE;
        uLOG_(detail, uLog::logConstLevel1) << "Test minimum log levels for specific code areas with constants: not generated." << uLOGE;
        uLOG_(detail, uLog::logLevelVar) << "Test minimum log levels for specific code areas with variables: not generated." << uLOGE;

        uLog::minLogLevel = warning;
        uLog::logLevelVar = detail;
        uLOG_(detail, MICRO_LOG_LEVEL2) << "Test minimum log levels for specific code areas with macros." << uLOGE;
        uLOG_(detail, uLog::logConstLevel2) << "Test minimum log levels for specific code areas with constants." << uLOGE;
        uLOG_(detail, uLog::logLevelVar) << "Test minimum log levels for specific code areas with variables." << uLOGE;

        /*
        //+TODO
            uLOG(warning) << "Log made of separate tokens... ";
            uLOGT(warning) << "first token, ";
            uLOGT(warning) << "last token" << uLOGE;
        */

        // Test with custom log file
        uLOG_TITLES_S(custom_ofs, warning);
        uLOGS(custom_ofs, warning) << "Test log on a different file." << uLOGE;
    }

    return 0;
}


int main()
{
    int testResult = 0;
    int nErrors = 0;
    int nWarnings = 0;

    std::cout << "\n--- microLog test ---\n" << std::endl;

    std::string logPath;
    std::string ramDiskPath = "/Volumes/ramdisk/";

    char pathOpt = '2';

    if(pathOpt == '0')
    {
        std::cout << "Select log file path:\n"
                  << "1. Local directory.\n"
                  << "2. Ram disk (" << ramDiskPath << ").\n"
                  << "   Note: check you have a ram disk on your system, a set its path in the source code (microLog_test.cpp).\n" << std::endl;

        std::cin >> pathOpt;
    }

    if(pathOpt == '2')
        logPath.append("/Volumes/ramdisk/");

    logPath.append("myProg.log");

    std::cout << "Test version:      " << VERSION << "\n";
    std::cout << "microLog version:  " << MICRO_LOG_VERSION << "\n";
    std::cout << "Log file path:     " << logPath << std::endl;

    testResult = Test_microLog(logPath);

    uLog::Statistics::Log();

    std::cout << "\nTest completed." << std::endl;

    if(testResult == 0)
        std::cout << "\nTest passed." << std::endl;
    else
        std::cout << "\nTest FAILED." << std::endl;

    return testResult;
}

#endif // MICRO_LOG_TEST
