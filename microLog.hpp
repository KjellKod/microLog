/**  microLog.hpp
 *
 *   Version 6.2.0
 *   2014.11.09
 *
 *   It was the smallest logger in the universe ...
 *   ... but it still fits in a single header ;-)
 *
 *   This is the only file needed to generate logs, plus a user written logger's 
 *   configuration file named: "microLog_config.hpp".
 *
 *   Author:    Pietro Mele
 *   Web site:  https://sites.google.com/site/pietrom16
 *   E-mail:    pietrom16@gmail.com
 */

/* microLog is distributed under the following BSD license:

Copyright (c) 2011-2014, Pietro Mele
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the <organization> nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/** A compact logging utility.

    - A single header file (this one).
	- There are no implementation files, so makefiles do not have to be touched.
    - Thread safe (C++11 threads, Boost threads or pthread).
	- It can be used when no debugger nor other logging system is available.
	- No objects to be defined.
	- To activate it, #define MICRO_LOG_ACTIVE.
	- When not activated, or when a log message level is below the threshold, there is no generated binary code.
    - uLOG(level) only logs if level >= MICRO_LOG_MIN_LEVEL and level >= ULog::minLogLevel.
    - uLOG_(level, localLevel) logs if level >= MICRO_LOG_MIN_LEVEL and (level >= ULog::minLogLevel or level >= localLevel).
          This allows to specify different minimum log levels for different kinds of log messages;
          localLevel can be a const or a variable.
    - To flush the message use the uLOGE manipulator (stands for log-end) instead of std::endl!
	- The output log file can be:
		- Unique for this executable, with a global static variable file stream (microLog_ofs).
        - Custom, with a stream passed as a parameter to every log message (TODO).
	- Multithreading: the MICRO_LOG_LOCK/MICRO_LOG_UNLOCK macros delimit a critical section.
		- Their values depend on the adopted threading library, and can be defined in microLog_config.hpp.
		- Predefined values available for: C++11 threads, Boost threads, pthread.
    - For better performance, log to a ramdisk and use an external utility to periodically move the logs to
          a permanent data storage.

	Usage: See microLog_test.cpp as an example.

	Settings:

		Define these macros in the makefile or as arguments to the compiler:

		MICRO_LOG_ACTIVE      to add the logger calls to the executable (if set to 1).
        MICRO_LOG_MIN_LEVEL   to set a minimum log level below which the logger call will not be added.
*/


/*
//+TODO

    - Error management:
        . Report an error if the log file cannot be opened.
        . If an error has been detected, disable logging.
        - Detect errors when logging, especially the disk full one.
            http://www.boost.org/doc/libs/1_57_0/libs/filesystem/doc/reference.html#space
            http://theboostcpplibraries.com/boost.filesystem-files-and-directories#ex.filesystem_13

    - Use tabs to separate fields in the logs.

    - Complete the code when the logger is disabled.

    - Testing.

    - Test:
        - Multithreading/C++11.
        - Multithreading/Boost.
        - On Linux/Windows.

    - Rolling logs.
        - Let an external utility deal with it.
        - Just check there is space on the disk partition.

    - Ram-disk: add utility that moves the logs from the ram-disk based log file
        to a hard disk, either at fixed time intervals or when space on the
        ram-disk falls below a certain threshold.

    Low priority:
    - Add log levels based on function names.
    - Make microLog as compatible as possible (regarding logging syntax) with g3log.
    - Store all the required details in a bit mask parameter.
    - In Windows, DLLs have troubles with static variables. Now static variables are removed when dealing with DLLs. Use dllexport/dllimport to fix this issue.
    - Log to multiple different log files simultaneously, with different log levels, details, ...
        - Tipically one of these files will be on ram-disk.
        - This can be done by an external utility (log server) which 
          gets the log from the logger and then copies it to the different log files.
          Pros: this makes the logger independent from the log files structure, and 
                allows to have the performance heavy task of dealing with the file
                system to another executable.
*/

#ifndef MICRO_LOG_HPP
#define MICRO_LOG_HPP

#define MICRO_LOG_VERSION 7.0.0

// Standard threading libraries
#define MICRO_LOG_SINGLE_THREAD  1
#define MICRO_LOG_CPP11_THREAD   2
#define MICRO_LOG_BOOST_THREAD   3
#define MICRO_LOG_PTHREAD        4

namespace uLog {
    // Log levels (a custom levels enum can be used)
    static const int nLogLevels = 8;
    enum             LogLevels                   {  nolog = 0, verbose,    detail,     info,       warning,    error,      critical,   fatal      };
    const char       logLevelTags[nLogLevels][9] { "  ----  ", "VERBOSE ", "DETAIL  ", "INFO    ", "WARNING ", "ERROR   ", "CRITICAL", "FATAL   " };
}

// Import logger's configuration
#include "microLog_config.hpp"

#ifdef MICRO_LOG_ACTIVE

    #include <bitset>
    #include <cstdio>
    #include <cstring>
    #include <ctime>
    #include <fstream>
    #include <iomanip>
    #include <iostream>
    #include <string>
    #include <vector>
    
    #ifndef MICRO_LOG_EXECUTABLE_NAME
        #define MICRO_LOG_EXECUTABLE_NAME ""
    #endif

    #if(MICRO_LOG_THREADING == MICRO_LOG_CPP11_THREAD)
        #include <mutex>
    #elif(MICRO_LOG_THREADING == MICRO_LOG_BOOST_THREAD)
        #include <boost/thread/thread.hpp>
    #elif(MICRO_LOG_THREADING == MICRO_LOG_PTHREAD)
        #include <pthread.h>
    #endif

    #ifndef MICRO_LOG_BOOST         // Boost used by default
        #define MICRO_LOG_BOOST 1
    #endif

    #if(MICRO_LOG_BOOST == 1)
        #include <boost/filesystem.hpp>
    #endif

    #ifndef WIN32
        #include <unistd.h>
    #else
        #include <process.h>
        #include <Lmcons.h>
    #endif
#else  // MICRO_LOG_ACTIVE
    #include <ios>
    #include <iosfwd>
#endif  // MICRO_LOG_ACTIVE


namespace uLog {

    struct Statistics
    {
        #ifndef MICRO_LOG_DLL
        static int nLogs;
        static int nNoLogs, nVerboseLogs, nDetailLogs, nInfoLogs, nWarningLogs, nErrorLogs, nCriticalLogs, nFatalLogs;
        static int highestLevel;
        #endif
        static void Update(int level);
        static void Log();
    };
    
    extern int minLogLevel;                  // minimum level a message must have to be logged
    extern int loggerStatus;                 // OK=0, error otherwise
    extern std::string logFilename;

    static const size_t maxLogSize = 1024;      // max length of a log message (bytes)

	#ifdef MICRO_LOG_ACTIVE

        // Begin: Platform specific
        #ifndef WIN32
            #define MICRO_LOG_DIR_SLASH '/'
        #else
            #define MICRO_LOG_DIR_SLASH '\\'
            static char username[UNLEN+1];
        #endif

        #ifdef _MSC_VER
            #define __func__ __FUNCTION__
            #define __PRETTY_FUNCTION__ __FUNCSIG__
        #endif
        // End: Platform specific

        // Log stream: define it at global scope and open it (in append mode) before logging
		extern std::ofstream microLog_ofs;

		#ifndef MICRO_LOG_MIN_LEVEL
			#define MICRO_LOG_MIN_LEVEL 2
		#endif

        // microLog initialization:
        // Use this macro once in the main()'s file at global scope

        #ifndef MICRO_LOG_DLL
            #define uLOG_INIT_0                                \
                using namespace uLog;                          \
                int uLog::minLogLevel = MICRO_LOG_MIN_LEVEL;   \
                int uLog::loggerStatus = 0;                    \
                std::string uLog::logFilename;                 \
                std::ofstream uLog::microLog_ofs;              \
                int Statistics::nLogs = 0, Statistics::nNoLogs = 0, Statistics::nVerboseLogs = 0, Statistics::nDetailLogs = 0, Statistics::nInfoLogs = 0, Statistics::nWarningLogs = 0, Statistics::nErrorLogs = 0, Statistics::nCriticalLogs = 0, Statistics::nFatalLogs = 0; \
                int Statistics::highestLevel = 0;              \
                bool LogFields::time = false, LogFields::date = true, LogFields::llevel = true, LogFields::exec = false, \
                     LogFields::uid = false, LogFields::uname = false, LogFields::pid = false, \
                     LogFields::fileName = false, LogFields::filePath = false, LogFields::funcName = false, LogFields::funcSig = false, \
                     LogFields::line = false, LogFields::log = true
        #else
            #define uLOG_INIT_0                                \
                using namespace uLog;                          \
                int uLog::minLogLevel = MICRO_LOG_MIN_LEVEL;   \
                int uLog::loggerStatus = 0;                    \
                std::string uLog::logFilename;                 \
                std::ofstream uLog::microLog_ofs
        #endif

        // microLog start:

        #define uLOG_START(logFilename_)            \
            uLog::logFilename = logFilename_;       \
            uLog::loggerStatus = 0;                 \
            microLog_ofs.open(logFilename);         \
            if(!microLog_ofs) {                     \
                uLog::loggerStatus = -1;            \
                std::cerr << "Error opening log file. Cannot produce logs. Check if disk space is available." << std::endl;  \
            }

        #define uLOG_START_APP(logFilename_)                        \
            uLog::logFilename = logFilename_;                       \
            uLog::loggerStatus = 0;                                 \
            microLog_ofs.open(logFilename, std::fstream::app);      \
            if(!microLog_ofs) {                                     \
                uLog::loggerStatus = -1;                            \
                std::cerr << "Error opening log file. Cannot produce logs. Check if disk space is available." << std::endl;  \
            }

        // Multithreading: macros used to define a critical section
        //                 according to the adopted threading library:

        #ifndef MICRO_LOG_THREADING
            #define MICRO_LOG_THREADING MICRO_LOG_SINGLE_THREAD
        #endif

		// Single threaded program
        #if(MICRO_LOG_THREADING == MICRO_LOG_SINGLE_THREAD)
            
            #define uLOG_INIT uLOG_INIT_0
            #define MICRO_LOG_LOCK {
            #define MICRO_LOG_UNLOCK }

		// C++11 Thread library
        #elif(MICRO_LOG_THREADING == MICRO_LOG_CPP11_THREAD)
            
            #define uLOG_INIT uLOG_INIT_0

            #define MICRO_LOG_LOCK                                          \
        		{                                                           \
					std::mutex ulog_mutex;                                  \
					ulog_mutex.lock();
            #define MICRO_LOG_UNLOCK                                        \
                    ulog_mutex.unlock();                                    \
                }

		// Boost Thread library      //+TODO
        #elif(MICRO_LOG_THREADING == MICRO_LOG_BOOST_THREAD)
            
            #define MICRO_LOG_INIT                                          \
                MICRO_LOG_INIT_0;                                           \
                ;

            // Use this macro once in the main() function, before any log
            #define MICRO_LOG_START(logFile)                                \
            	microLog_ofs.open((logFile))
            #define MICRO_LOG_START_APPEND(logFile)                         \
            	microLog_ofs.open((logFile), std::fstream::app)

            #define MICRO_LOG_LOCK                                          \
                ;
            #define MICRO_LOG_UNLOCK                                        \
                ;

		// PThread library      //+TEST
        #elif(MICRO_LOG_THREADING == MICRO_LOG_PTHREAD)
            
            #define MICRO_LOG_INIT                                          \
                MICRO_LOG_INIT_0;                                           \
                pthread_mutex_t ulog_mutex;

            // Use this macro once in the main() function, before any log
            #define MICRO_LOG_START(logFile)                                \
            	pthread_mutex_init(&ulog_mutex, NULL);                      \
            	microLog_ofs.open((logFile))
            #define MICRO_LOG_START_APPEND(logFile)                         \
            	pthread_mutex_init(&ulog_mutex, NULL);                      \
            	microLog_ofs.open((logFile), std::fstream::app)

            #define MICRO_LOG_LOCK                                          \
                pthread_mutex_lock(&ulog_mutex)
            #define MICRO_LOG_UNLOCK                                        \
                pthread_mutex_unlock(&ulog_mutex)

        #endif


        static std::time_t microLog_time;

        static const char bar[] = "--------------------------------------------------";

		template <class charT, class traits>
		std::basic_ostream<charT, traits>& endm(std::basic_ostream<charT, traits>& os)
		{
			os << std::endl;
			return os;
		}

        inline bool CheckLogLevel(int _level, int _localLevel = nolog)
        {
            #ifndef MICRO_LOG_DLL
                Statistics::Update(_level);
            #endif

            if(loggerStatus != 0) {        // cannot log if status is not clean
                if(_level > error)
                    std::cerr << "Error " << loggerStatus << ": logger disabled, and a critical error has been generated!" << std::endl;
                return false;
            }

            if(_level < MICRO_LOG_MIN_LEVEL || _level < _localLevel)
                return false;

            if(_localLevel == nolog && _level < uLog::minLogLevel)
                return false;

            return true;
        }

        inline bool CheckAvailableSpace()
            // Check if the next log message can fit in the remaining available space
        {
        #if(MICRO_LOG_BOOST == 1)
            //+TODO
            boost::system::error_code errCode;
            boost::filesystem::space_info space = boost::filesystem::space(uLog::logFilename, errCode);
            if(space.available < maxLogSize) {
                std::cerr << "Logger error: not enough space available in the current partition (" << space.available << " bytes)." << std::endl;
                return false;
            }
            else { //+T+
                std::cout << "Logger info: space available in the current partition (" << space.available << " bytes)." << std::endl;
            }
        #endif
            return true;
        }

        // Run time fields selection

        struct LogFields
            /// Flags to enable/disable log message fields
        {
            static bool time, date, llevel,
                        exec, pid,
                        uid, uname,
                        fileName, filePath, funcName, funcSig,
                        line, log;

            LogFields() {
                SetDefault();
            }

            static void SetDefault() {
                time = false; date = true;
                llevel = true;
                exec = false; pid = false;
                uid = false; uname = false;
                fileName = false; filePath = false;
                funcName = false; funcSig = false; line = false;
                log = true;
            }

            static void SetDetailed() {
                time = true; date = true;
                llevel = true;
                exec = true; pid = false;
                uid = false; uname = false;
                fileName = false; filePath = false;
                funcName = false; funcSig = false; line = false;
                log = true;
            }

            static void SetSystem() {
                time = false; date = true;
                llevel = true;
                exec = true; pid = true;
                uid = true; uname = true;
                fileName = true; filePath = true;
                funcName = false; funcSig = false; line = false;
                log = true;
            }

            static void SetDebug() {
                time = false; date = false;
                llevel = true;
                exec = true; pid = false;
                uid = false; uname = false;
                fileName = true; filePath = false;
                funcName = true; funcSig = false; line = true;
                log = true;
            }

            static void SetVerbose() {
                time = true; date = true;
                llevel = true;
                exec = true; pid = true;
                uid = true; uname = true;
                fileName = true; filePath = true;
                funcName = true; funcSig = true; line = true;
                log = true;
            }
        };

        inline std::string LogTime() {
            float t = float(std::clock())/CLOCKS_PER_SEC;
            const size_t sz = 16;
            char ct[sz];
            std::snprintf(ct, sz, "% 7.3f  ", t);
            return std::string(ct);
        }

        inline std::string LogDate() {
            std::time_t t = std::time(nullptr);
            char mbstr[32];
            std::strftime(mbstr, sizeof(mbstr), "%F %T  ", std::localtime(&t));
            return std::string(mbstr);
        }

        inline std::string GetPID() {
            #ifdef _POSIX_VERSION
                return std::to_string(getpid());
            #elif defined WIN32
                return std::to_string(_getpid());
            #else
                return "?";
            #endif
        }

        inline std::string GetUID() {
            #ifdef _POSIX_VERSION
                return std::to_string(getuid());
            #elif defined WIN32
                return "?";
            #else
                return "?";
            #endif
        }

        inline std::string GetUserName() {
            #ifdef _POSIX_VERSION
                return getlogin();
            #elif defined WIN32
                GetUserName(username, UNLEN+1);
                return username;
            #else
                return "?";
            #endif
        }


        #define uLOG_(level, localMinLevel)                                     \
            if(CheckLogLevel(level, localMinLevel) && CheckAvailableSpace())    \
                MICRO_LOG_LOCK;                                                 \
                microLog_ofs                                                    \
                    << (LogFields::time?LogTime():"")                           \
                    << (LogFields::date?LogDate():"")                           \
                    << (LogFields::llevel?logLevelTags[level]:"")               \
                    << (LogFields::llevel?"  ":"")                              \
                    << (LogFields::exec?MICRO_LOG_EXECUTABLE_NAME:"")           \
                    << (LogFields::exec?"  ":"")                                \
                    << (LogFields::pid?GetPID():"")                             \
                    << (LogFields::pid?"  ":"")                                 \
                    << (LogFields::uid?GetUID():"")                             \
                    << (LogFields::uid?"  ":"")                                 \
                    << (LogFields::uname?GetUserName():"")                      \
                    << (LogFields::uname?"  ":"")                               \
                    << (LogFields::fileName?(strrchr(__FILE__, MICRO_LOG_DIR_SLASH) ? strrchr(__FILE__, MICRO_LOG_DIR_SLASH) + 1 : __FILE__):"")  \
                    << (LogFields::fileName?"  ":"")                            \
                    << (LogFields::filePath?__FILE__:"")                        \
                    << (LogFields::filePath?"  ":"")                            \
                    << (LogFields::funcName?__func__:"")                        \
                    << (LogFields::funcName?"  ":"")                            \
                    << (LogFields::funcSig?__PRETTY_FUNCTION__:"")              \
                    << (LogFields::funcSig?"  ":"")                             \
                    << (LogFields::line?std::to_string(__LINE__):"")            \
                    << (LogFields::line?"  ":"")                                \
                    << ": "

        #define uLOG(level)  uLOG_(level, nolog)

        #define uLOG_TITLES(level)                                              \
            if(CheckLogLevel(level))                                            \
                MICRO_LOG_LOCK;                                                 \
                microLog_ofs                                                    \
                    << bar << "\n"                                              \
                    << (LogFields::time?"Time  ":"")                            \
                    << (LogFields::date?"Date  ":"")                            \
                    << (LogFields::llevel?"Level  ":"")                         \
                    << (LogFields::exec?"Executable  ":"")                      \
                    << (LogFields::pid?"PID  ":"")                              \
                    << (LogFields::uid?"UID  ":"")                              \
                    << (LogFields::uname?"User  ":"")                           \
                    << (LogFields::fileName?"Filename  ":"")                    \
                    << (LogFields::filePath?"Filepath  ":"")                    \
                    << (LogFields::funcName?"Function  ":"")                    \
                    << (LogFields::funcSig?"Function_signature  ":"")           \
                    << (LogFields::line?"Line  ":"")                            \
                    << "\n" << bar << endm;                                     \
                MICRO_LOG_UNLOCK

        // uLOG log terminator
        #define uLOGE endm; MICRO_LOG_UNLOCK

		#define uLOGT(level) \
			if(CheckLogLevel(level)) \
				microLog_ofs

        #define uLOG_DATE \
			if(std::time(&microLog_time)) \
                microLog_ofs << "\nDate: " << std::ctime(&microLog_time)

		#define uLOGD(level) \
			if(std::time(&microLog_time), CheckLogLevel(level)) \
                microLog_ofs << "\nDate: " << std::ctime(&microLog_time)

		#define uLOGB(level) \
			if(CheckLogLevel(level)) \
                microLog_ofs << bar << endm

        #ifndef MICRO_LOG_DLL
        void LogLevels() {
            microLog_ofs << "Log levels: ";
            for(size_t i = 0; i < nLogLevels; ++i) microLog_ofs << logLevelTags[i] << " ";
            microLog_ofs << std::endl;
        }

        void MinLogLevel() {
            microLog_ofs << "Minimum log level to be logged: " << logLevelTags[minLogLevel] << std::endl;
        }

        void Statistics::Update(int level) {
            ++nLogs;
            if(level > highestLevel) highestLevel = level;
            switch (level) {
            case nolog:    ++nNoLogs;       break;
            case verbose:  ++nVerboseLogs;  break;
            case detail:   ++nDetailLogs;   break;
            case info:     ++nInfoLogs;     break;
            case warning:  ++nWarningLogs;  break;
            case error:    ++nErrorLogs;    break;
            case critical: ++nCriticalLogs; break;
            case fatal:    ++nFatalLogs;    break;
            }
        }

        void Statistics::Log() {
            microLog_ofs << "Log statistics:"
                << "\n\tNumber of logs: " << nLogs
                << "\n\tNumber of 'fatal' logs:    " << nFatalLogs
                << "\n\tNumber of 'critical' logs: " << nCriticalLogs
                << "\n\tNumber of 'error' logs:    " << nErrorLogs
                << "\n\tNumber of 'warning' logs:  " << nWarningLogs
                << "\n\tNumber of 'info' logs:     " << nInfoLogs
                << "\n\tNumber of 'detail' logs:   " << nDetailLogs
                << "\n\tNumber of 'verbose' logs:  " << nVerboseLogs
                << "\n\tNumber of 'null' logs:     " << nNoLogs << std::endl;
            microLog_ofs << "Highest log level: " << highestLevel << std::endl;
        }

        #endif // MICRO_LOG_DLL


#else // MICRO_LOG_ACTIVE

		struct nullstream {
			void open(const char *filename = 0, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
			{}
		};

		template <typename T>
		nullstream& operator<<(nullstream& s, T const&) { return s; }

		inline nullstream& operator<<(nullstream &s, std::ostream &(std::ostream&)) { return s; }

		inline nullstream& endm(nullstream& os) { return os; }

        #ifndef MICRO_LOG_DLL
            #define uLOG_INIT                                  \
                using namespace uLog;                          \
                int uLog::minLogLevel = MICRO_LOG_MIN_LEVEL;   \
                nullstream uLog::microLog_ofs;                 \
                int Statistics::nLogs = 0, Statistics::nNoLogs = 0, Statistics::nVerboseLogs = 0, Statistics::nDetailLogs = 0, Statistics::nInfoLogs = 0, Statistics::nWarningLogs = 0, Statistics::nErrorLogs = 0, Statistics::nCriticalLogs = 0, Statistics::nFatalLogs = 0; \
                int Statistics::highestLevel = 0;
        #else
            #define uLOG_INIT                                  \
                using namespace uLog;                          \
                int uLog::minLogLevel = MICRO_LOG_MIN_LEVEL;   \
                nullstream uLog::microLog_ofs;
        #endif

        #define uLOG_START(logFilename)
        #define uLOG_START_APP(logFilename)

        #ifdef MICRO_LOG_FILE_NAME
            static nullstream microLog_ofs;
        #else
            extern nullstream microLog_ofs;
            // Define it at global scope and open it (in append mode) before logging.
        #endif

        #define uLOG_DATE                    if(0) microLog_ofs
        #define uLOG_TITLES(level)           if(0) microLog_ofs
        #define uLOG_TITLES_(ofs, level)     if(0) ofs
        #define uLOGE                        ""
        #define uLOG(level)                  if(0) microLog_ofs
        #define uLOGL(level, localMinLevel)  if(0) microLog_ofs
        #define uLOG_(ofs, level)            if(0) ofs
        #define uLOGT(level)                 if(0) microLog_ofs
        #define uLOGT_(ofs, level)           if(0) ofs
        #define uLOGD(level)                 if(0) microLog_ofs
        #define uLOGD_(ofs, level)           if(0) ofs
        #define uLOGB(level)                 if(0) microLog_ofs
        #define uLOGB_(ofs, level)           if(0) ofs
        #define uLOG_LEVEL                   if(0) microLog_ofs

        #ifndef MICRO_LOG_DLL
        void LogLevels() {}
        void MinLogLevel() {}
        void Statistics::Update(int level) {}
        void Statistics::Log() {}
        #endif

	#endif // MICRO_LOG_ACTIVE


} // namespace uLog

#endif // MICRO_LOG_HPP
