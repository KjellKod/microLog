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


/** A minimalistic logging utility.

    - A single file project (this one).
	- There are no implementation files, so makefiles do not have to be touched.
    - Thread safe (C++11 threads, Boost threads or pthread).
	- It can be used when no debugger nor other logging system is available.
	- No objects to be defined.
	- To activate it, #define MICRO_LOG_ACTIVE.
	- When not activated, or when a log message level is below the threshold, there is no generated binary code.
    - uLOG(level) only logs if level >= MICRO_LOG_MIN_LEVEL and level >= ULog::minLogLevel.
    - uLOG(level, localLevel) logs if level >= MICRO_LOG_MIN_LEVEL and (level >= ULog::minLogLevel or level >= localLevel).
    - To flush the message use the uLOGE manipulator (stands for log-end) instead of std::endl!
	- The output log file can be:
		- Unique for this executable, with a fixed preprocessor defined file name (MICRO_LOG_FILE_NAME).
		- Unique for this executable, with a global static variable file stream (microLog_ofs).
		- Custom, with a stream passed as a parameter to every log message.
	- Multithreading: the MICRO_LOG_LOCK/MICRO_LOG_UNLOCK macros delimit a critical section.
		- Their values depend on the adopted threading library, and can be defined in microLog_config.hpp.
		- Predefined values available for: C++11 threads, Boost threads, pthread.
	- For better performance, consider logging to a ramdisk.

	Usage: See microLog_test.cpp as an example.

	Settings:

		Define these macros in the makefile or as arguments to the compiler:

		MICRO_LOG_ACTIVE      to add the logger calls to the executable (if set to 1).
		MICRO_LOG_FILE_NAME   to set a log file name different from the default.
		MICRO_LOG_MIN_LEVEL   to set a minimum log level below which the logger call will not be added.
		MICRO_LOG_DETAIL      to set the amount of details to be generated (0-5).

	Log message structures:

        MICRO_LOG_DETAIL = 0
		[loglevel] [Log message]

        MICRO_LOG_DETAIL = 1
		[Time since program launch in seconds] [loglevel] [Log message]

        MICRO_LOG_DETAIL = 2
		[Time since program launch in seconds] [loglevel] [file name:function_name:line] [Log message]

        MICRO_LOG_DETAIL = 3
		[Time since program launch in seconds] [loglevel] [file path:function_signature:line] [Log message]

        MICRO_LOG_DETAIL = 4
		[Time since program launch in seconds] [loglevel] [uname; PID] [Log message]

        MICRO_LOG_DETAIL = 5
		[Time since program launch in seconds] [loglevel] [UID; PID] [Log message]

	Output log examples with different values for MICRO_LOG_DETAIL:

		----------------------------------------
		Date: Fri Dec 09 11:44:42 2012
		4 Test log message number 2 with value 3.141
        0.025 4 Test log message number 2 with value 3.141
        0.025 4 [microLog_test.cpp:TestFunc:16] Test log message number 2 with value 3.141
        0.025 4 [path/to/microLog_test.cpp:void TestFunc():16] Test log message number 2 with value 3.141
        0.025 4 [User=mele; PID=123] Test log message number 2 with value 3.141
        0.025 4 [UID=456; PID=123] Test log message number 2 with value 3.141

*/


/*
//+TODO
    - Add log levels based on function names.
    - Make microLog as compatible as possible (regarding logging syntax) with g3log.
    - Store all the required details in a bit mask parameter.
	- Test:
		- Multithreading/C++11.
        - Multithreading/Boost.
        . On Windows.
    - In Windows, DLLs have troubles with static variables. Now static variables are removed when dealing with DLLs. Use dllexport/dllimport to fix this issue.
    - Log to multiple different log files simultaneously, with different log levels, details, ...
        - Tipically one of these files will be on ram-disk.
        - This can be done by an external utility (log server) which 
          gets the log from the logger and then copies it to the different log files.
          Pros: this makes the logger independent from the log files structure, and 
                allows to have the performance heavy task of dealing with the file
                system to another executable.
    - Utility that periodically checks the size of the log file, and when it 
      reaches a certain value, it copies it somewhere else (e.g. on a different
      file, appendig it to an existing log, ...). This is useful, e.g., in case 
      a small partition is available on a ram-disk. (part of the log server)
*/

#ifndef MICRO_LOG_HPP
#define MICRO_LOG_HPP

#define MICRO_LOG_VERSION 6.2.0

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
    
    extern int minLogLevel;         // minimum level a message must have to be logged

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

		#ifndef MICRO_LOG_DETAIL
			#define MICRO_LOG_DETAIL 2
		#endif

        // microLog initialization:
        // Use this macro once in the main()'s file at global scope

        #ifndef MICRO_LOG_DLL
            #define uLOG_INIT_0                                \
                using namespace uLog;                          \
                int uLog::minLogLevel = MICRO_LOG_MIN_LEVEL;   \
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
                std::ofstream uLog::microLog_ofs
        #endif

        // microLog start:
        #define uLOG_START(logFilename)     microLog_ofs.open(logFilename)
        #define uLOG_START_APP(logFilename) microLog_ofs.open(logFilename, std::fstream::app)


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

            if(_level < MICRO_LOG_MIN_LEVEL || _level < _localLevel)
                return false;

            if(_localLevel == nolog && _level < uLog::minLogLevel)
                return false;

            return true;
        }

        //+TODO
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

        #define uLOGR(level, localMinLevel)                                     \
            if(CheckLogLevel(level, localMinLevel))                             \
                MICRO_LOG_LOCK;                                                 \
                microLog_ofs                                                    \
                    << (LogFields::time?LogTime():"")                           \
                    << (LogFields::date?LogDate():"")                           \
                    << (LogFields::llevel?logLevelTags[level]:"")               \
                    << (LogFields::llevel?"  ":"")                              \
                    << (LogFields::exec?MICRO_LOG_EXECUTABLE_NAME:"")           \
                    << (LogFields::exec?"  ":"")                                \
                    << (LogFields::pid?std::to_string(getpid()):"")             \
                    << (LogFields::pid?"  ":"")                                 \
                    << (LogFields::uid?std::to_string(getuid()):"")             \
                    << (LogFields::uid?"  ":"")                                 \
                    << (LogFields::uname?getlogin():"")                         \
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



        #if MICRO_LOG_DETAIL == 0
			// level Executable log

			#define uLOG(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << ": "

            #define uLOGL(level, localMinLevel) \
                if(CheckLogLevel(level, localMinLevel)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << ": "

            #define uLOG_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << ": "

            #define uLOG_TITLES(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << bar << "\n" << "Level  Log \n" << bar << endm; \
                	MICRO_LOG_UNLOCK

            #define uLOG_TITLES_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << bar << "\n" << "Level  Log \n" << bar << endm; \
                	MICRO_LOG_UNLOCK

        #elif MICRO_LOG_DETAIL == 1
			// time level Executable log

			#define uLOG(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                 << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << ": "

            #define uLOGL(level, localMinLevel) \
                if(CheckLogLevel(level, localMinLevel)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                 << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << ": "

			#define uLOG_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                        << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << ": "

            #define uLOG_TITLES(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << bar << "\n" << "Time  Level  Log \n" << bar << endm; \
                	MICRO_LOG_UNLOCK

            #define uLOG_TITLES_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << bar << "\n" << "Time  Level  Log \n" << bar << endm; \
                	MICRO_LOG_UNLOCK

		#elif MICRO_LOG_DETAIL == 2
			// time level Executable file_name func_name line log

			#define uLOG(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                 << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [" \
					             << (strrchr(__FILE__, MICRO_LOG_DIR_SLASH) ? strrchr(__FILE__, MICRO_LOG_DIR_SLASH) + 1 : __FILE__) \
					             << ":" << __func__ << ":" << __LINE__ << "] "

            #define uLOGL(level, localMinLevel) \
                if(CheckLogLevel(level, localMinLevel)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                 << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [" \
					             << (strrchr(__FILE__, MICRO_LOG_DIR_SLASH) ? strrchr(__FILE__, MICRO_LOG_DIR_SLASH) + 1 : __FILE__) \
					             << ":" << __func__ << ":" << __LINE__ << "] "

			#define uLOG_(ofs, level) \
				if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                        << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [" \
					    << (strrchr(__FILE__, MICRO_LOG_DIR_SLASH) ? strrchr(__FILE__, MICRO_LOG_DIR_SLASH) + 1 : __FILE__) \
					    << ":" << __func__ << ":" << __LINE__ << "] "

            #define uLOG_TITLES(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
					microLog_ofs << bar << "\n" << "Time  Level  Executable  File  Function  Line  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

            #define uLOG_TITLES_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
					ofs << bar << "\n" << "Time  Level  Executable  File  Function  Line  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

        #elif MICRO_LOG_DETAIL == 3
			// time level Executable file_path func_sig line log

			#define uLOG(level) \
				if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                 << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [" << __FILE__ << ":" << __PRETTY_FUNCTION__ << ":" << __LINE__ << "] "

            #define uLOGL(level, localMinLevel) \
				if(CheckLogLevel(level, localMinLevel)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                 << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [" << __FILE__ << ":" << __PRETTY_FUNCTION__ << ":" << __LINE__ << "] "

			#define uLOG_(ofs, level) \
				if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                        << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [" << __FILE__ << ":" << __PRETTY_FUNCTION__ << ":" << __LINE__ << "] "

            #define uLOG_TITLES(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
					microLog_ofs << bar << "\n" << "Time  Level  Executable  FilePath  FunctionSig  Line  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

            #define uLOG_TITLES_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
					ofs << bar << "\n" << "Time  Level  Executable  FilePath  FunctionSig  Line  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

        #elif MICRO_LOG_DETAIL == 4
			// time level Executable uname PID log

			#if !defined(WIN32)

				#define uLOG(level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                     << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [User=" << getlogin() << "; PID=" << getpid() << "] "

                #define uLOGL(level, localMinLevel) \
					if(CheckLogLevel(level, localMinLevel)) \
                        MICRO_LOG_LOCK; \
                        microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                     << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [User=" << getlogin() << "; PID=" << getpid() << "] "

				#define uLOG_(ofs, level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                            << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [User=" << getlogin() << "; PID=" << getpid() << "] "

            #else // WIN32

				#define uLOG(level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                    	GetUserName(username, UNLEN+1); \
                        microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                     << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [User=" << username << "; PID=" << _getpid() << "] "

                #define uLOGL(level, localMinLevel) \
					if(CheckLogLevel(level, localMinLevel)) \
                        MICRO_LOG_LOCK; \
                    	GetUserName(username, UNLEN+1); \
                        microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                     << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [User=" << username << "; PID=" << _getpid() << "] "

                #define uLOG_(ofs, level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        GetUserName(username, UNLEN+1); \
                        ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                            << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [User=" << username << "; PID=" << _getpid() << "] "

			#endif // WIN32

            #define uLOG_TITLES(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
					microLog_ofs << bar << "\n" << "Time  Level  Executable  User  PID  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

            #define uLOG_TITLES_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
					ofs << bar << "\n" << "Time  Level  Executable  User  PID  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

        #elif MICRO_LOG_DETAIL >= 5
			// time level Executable UID PID log

			#if !defined(WIN32)

				#define uLOG(level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                     << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [UID=" << getuid() << "; PID=" << getpid() << "] "

                #define uLOGL(level, localMinLevel) \
					if(CheckLogLevel(level, localMinLevel)) \
                        MICRO_LOG_LOCK; \
                        microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                     << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [UID=" << getuid() << "; PID=" << getpid() << "] "

				#define uLOG_(ofs, level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                            << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [UID=" << getuid() << "; PID=" << getpid() << "] "

			#else // WIN32

				#define uLOG(level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                     << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [UID=" << "?" << "; PID=" << _getpid() << "] "

                #define uLOGL(level, localMinLevel) \
					if(CheckLogLevel(level, localMinLevel)) \
                        MICRO_LOG_LOCK; \
                        microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                     << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [UID=" << "?" << "; PID=" << _getpid() << "] "

				#define uLOG_(ofs, level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                            << logLevelTags[level] << " - " << MICRO_LOG_EXECUTABLE_NAME << " [UID=" << "?" << "; PID=" << _getpid() << "] "

			#endif // WIN32

            #define uLOG_TITLES(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
					microLog_ofs << bar << "\n" << "Time  Level  Executable  UID  PID  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

            #define uLOG_TITLES_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
					ofs << bar << "\n" << "Time  Level  Executable  UID  PID  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

        #endif

        // uLOG log terminator
        #define uLOGE endm; MICRO_LOG_UNLOCK

		#define uLOGT(level) \
			if(CheckLogLevel(level)) \
				microLog_ofs

		#define uLOGT_(ofs, level) \
			if(CheckLogLevel(level)) \
				ofs

        #define uLOG_DATE \
			if(std::time(&microLog_time)) \
                microLog_ofs << "\nDate: " << std::ctime(&microLog_time)

		#define uLOGD(level) \
			if(std::time(&microLog_time), CheckLogLevel(level)) \
                microLog_ofs << "\nDate: " << std::ctime(&microLog_time)

		#define uLOGD_(ofs, level) \
			if(std::time(&microLog_time), CheckLogLevel(level)) \
                ofs << "\nDate: " << std::ctime(&microLog_time)

		#define uLOGB(level) \
			if(CheckLogLevel(level)) \
                microLog_ofs << bar << endm

		#define uLOGB_(ofs, level) \
			if(CheckLogLevel(level)) \
                ofs << bar << endm

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
