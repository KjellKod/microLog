/**  microLog.hpp
 *
 *    This is the only file needed to generate logs, plus a user written logger's 
 *    configuration file named: "microLog_config.hpp".
 *
 *    Author:    Pietro Mele
 *    Web site:  https://sites.google.com/site/pietrom16
 *    E-mail:    pietrom16@gmail.com
 */

/** A minimalistic logging utility.

	- A single file project (this one).
	- There are no implementation files, so makefiles do not have to be touched.
    - Thread safe.
	- It can be used when no debugger nor other logging system is available.
	- No objects to be defined.
	- To activate it, #define MICRO_LOG_ACTIVE 1
	- When not activated, or when a log message level is below the threshold, there is no generated binary code.
    - uLOG(level) only logs if level >= MICRO_LOG_MIN_LEVEL and level >= minLogLevel.
    - To flush the message use the uLOGE manipulator (stands for log-end) instead of std::endl!	
	- The output log file can be:
		- Unique for this executable, with a fixed preprocessor defined file name (MICRO_LOG_FILE_NAME).
		- Unique for this executable, with a global static variable file stream (microLog_ofs).
		- Custom, with a stream passed as a parameter to every log message.
	- Multithreading: the MICRO_LOG_LOCK/MICRO_LOG_UNLOCK macros delimit a critical section.
		- Their values depend on the adopted threading library, and can be defined in microLog_config.hpp.
		- Predefined values available for: C++11 threads, Boost threads, pthread.
	- For better performance, consider logging to a ramdisk.

	Usage:

		#include "microLog.h"

        MICRO_LOG_INIT;
        
        int main()
        {
        	MICRO_LOG_START("myProg.log");
            uLOGD(5);           // date
            uLOG_TITLES(5);		// columns' titles

            uLOG(info) << "Test log message number " << 2 << " with value " << 3.141 << uLOGE;

            uLog::minLogLevel = warning;

            uLOG(warning) << "Log made of separate tokens... ";
            uLOGT(warning) << "first token, ";
            uLOGT(warning) << "last token." << uLOGE;
        }

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


/* microLog is distributed under the following BSD license:

Copyright (c) 2011-2013, Pietro Mele
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


/*
//+TODO
    - Option to log the executable name; useful if the log server receives logs from multiple executables.
    - Store all the required details in a bit mask parameter.
	- Test:
		- Multithreading/Boost.
		- Multithreading/C++11.
		- On Windows.
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

#define MICRO_LOG_VERSION 5.3.0

// Standard threading libraries
#define MICRO_LOG_SINGLE_THREAD  1
//+TEST:
#define MICRO_LOG_CPP11_THREAD   2
#define MICRO_LOG_BOOST_THREAD   3
#define MICRO_LOG_PTHREAD        4

// Import logger's configuration
#include "microLog_config.hpp"

#if(MICRO_LOG_ACTIVE == 1)
    #include <bitset>
    #include <cstring>
    #include <ctime>
    #include <fstream>
    #include <iomanip>
    #include <string>
    #include <vector>
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

	// Log levels (a custom levels enum can be used)
	enum LogLevels { nolog, detail, info, warning, error, critical, fatal };


    extern int highestLevel;		// define it in entrypoint.cpp, set to nolog
    extern int minLogLevel;         // minimum level a message must have to be logged


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

	#if(MICRO_LOG_ACTIVE == 1)

		// Log stream: define it at global scope and open it (in append mode) before logging
		extern std::ofstream microLog_ofs;

		#ifndef MICRO_LOG_MIN_LEVEL
			#define MICRO_LOG_MIN_LEVEL 2
		#endif

		#ifndef MICRO_LOG_DETAIL
			#define MICRO_LOG_DETAIL 2
		#endif

        //+TODO++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        static const char fieldSeparator = ' ';

        // List of available fields for the logs:
        static const char logFieldsName[][9] = { "time", "level", "program", "PID", "user", "UID",
                                                "fileName", "filePath", "funcName", "funcSign", "line",
                                                "msg", "\0" };

#define PROG_RELEASE_VERSION            1
#define PROG_RELEASE_VERSION_PROCINFO   2
#define PROG_DEBUG_VERSION              3
#define PROG_DEBUG_VERSION_PROCINFO     4

        // Specify the position of the fields in the logs (0 means absent field):
#if(PROG_VERSION == PROG_RELEASE_VERSION)
        static int logFieldsPos[] = { 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 4 };     // release version
#elif(PROG_VERSION == PROG_RELEASE_VERSION_PROCINFO)
        static int logFieldsPos[] = { 1, 2, 3, 4, 5, 6, 0, 0, 0, 0, 0, 7 };     // release version, with proc info
#elif(PROG_VERSION == PROG_DEBUG_VERSION)
        static int logFieldsPos[] = { 1, 2, 3, 0, 0, 0, 4, 0, 5, 0, 6, 7 };     // debug/development version
#elif(PROG_VERSION == PROG_DEBUG_VERSION_PROCINFO)
        static int logFieldsPos[] = { 1, 2, 3, 0, 0, 0, 4, 0, 5, 0, 6, 7 };     // debug/development version, with proc info
#else
        // Default
        static int logFieldsPos[] = { 1, 2, 3, 4, 5, 6, 0, 0, 0, 0, 0, 7 };     // release version, with proc info
#endif

        //+TODO-END++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        // microLog initialization:
        // Use this macro once in the main()'s file at global scope
        #define MICRO_LOG_INIT_0                                            \
            using namespace uLog;                                           \
            int uLog::highestLevel = nolog;                                 \
            int uLog::minLogLevel = MICRO_LOG_MIN_LEVEL;                    \
            std::ofstream uLog::microLog_ofs

        // Multithreading: macros used to define a critical section
        //                 according to the adopted threading library:

		// Single threaded program
        #if(MICRO_LOG_THREADING == MICRO_LOG_SINGLE_THREAD)
            
            #define MICRO_LOG_INIT MICRO_LOG_INIT_0
            #define MICRO_LOG_START(logFile)                                \
            	microLog_ofs.open((logFile))
            #define MICRO_LOG_START_APPEND(logFile)                         \
            	microLog_ofs.open((logFile), std::fstream::app)
            #define MICRO_LOG_LOCK {
            #define MICRO_LOG_UNLOCK }

		// C++11 Thread library      //+TEST
        #elif(MICRO_LOG_THREADING == MICRO_LOG_CPP11_THREAD)
            
            #include <mutex>
            #define MICRO_LOG_INIT                                          \
                MICRO_LOG_INIT_0;                                           \
                ;

            // Use this macro once in the main() function, before any log
            #define MICRO_LOG_START(logFile)                                \
            	microLog_ofs.open((logFile))
            #define MICRO_LOG_START_APPEND(logFile)                         \
            	microLog_ofs.open((logFile), std::fstream::app)

            #define MICRO_LOG_LOCK                                          \
        		{                                                           \
					std::mutex ulog_mutex;                                  \
					ulog_mutex.lock();
            #define MICRO_LOG_UNLOCK                                        \
                    ulog_mutex.unlock();                                    \
                }

		// Boost Thread library      //+TODO
        #elif(MICRO_LOG_THREADING == MICRO_LOG_BOOST_THREAD)
            
            #include <boost/thread/thread.hpp>
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
            
            #include <pthread.h>
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

		// DFS::Infrastructure::CThread library
        #elif(MICRO_LOG_THREADING == MICRO_LOG_CTHREAD)
            
            #define MICRO_LOG_INIT MICRO_LOG_INIT_0
            #define MICRO_LOG_START(logFile)                                \
            	microLog_ofs.open((logFile))
            #define MICRO_LOG_START_APPEND(logFile)                         \
            	microLog_ofs.open((logFile), std::fstream::app)
            
            #define MICRO_LOG_LOCK                                          \
            {                                                               \
                DFS::Infrastructure::CCriticalSection logCS;                \
                DFS::Infrastructure::CCriticalSectionLock L(logCS)
            #define MICRO_LOG_UNLOCK  }

        #endif


        static std::time_t microLog_time;

        static const char bar[] = "--------------------------------------------------";

		template <class charT, class traits>
		std::basic_ostream<charT, traits>& endm(std::basic_ostream<charT, traits>& os)
		{
			os << std::endl;
			return os;
		}

		inline bool CheckLogLevel(int _level)
		{
			if(highestLevel < _level)
				highestLevel = _level;

            if(_level < minLogLevel || _level < MICRO_LOG_MIN_LEVEL)
                return false;

			return true;
		}


//+TODO++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
#define uLOG(level) \
    if(CheckLogLevel(level)) \
        MICRO_LOG_LOCK; \
        microLog_ofs \

            << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
            << level
            << " [" << __FILE__ << ":" << __PRETTY_FUNCTION__ << ":" << __LINE__ << "] "


            << " [" << (strrchr(__FILE__, MICRO_LOG_DIR_SLASH) ? strrchr(__FILE__, MICRO_LOG_DIR_SLASH) + 1 : __FILE__) \
            << ":" << __func__ << ":" << __LINE__ << "] "

            << " [User=" << getlogin() << "; PID=" << getpid() << "] " //UNIX

            << " [User=" << username << "; PID=" << _getpid() << "] "  //WIN
*/
//+TODO+END++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



		#if MICRO_LOG_DETAIL == 0
			// level log

			#define uLOG(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << level << ": "

			#define uLOG_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << level << ": "

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
			// time level log

			#define uLOG(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
                                 << level << ": "

			#define uLOG_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
					    << level << ": "

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
			// time level file_name func_name line log

			#define uLOG(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
					             << level << " [" \
					             << (strrchr(__FILE__, MICRO_LOG_DIR_SLASH) ? strrchr(__FILE__, MICRO_LOG_DIR_SLASH) + 1 : __FILE__) \
					             << ":" << __func__ << ":" << __LINE__ << "] "

			#define uLOG_(ofs, level) \
				if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
					    << level << " [" \
					    << (strrchr(__FILE__, MICRO_LOG_DIR_SLASH) ? strrchr(__FILE__, MICRO_LOG_DIR_SLASH) + 1 : __FILE__) \
					    << ":" << __func__ << ":" << __LINE__ << "] "

            #define uLOG_TITLES(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << bar << "\n" << "Time  Level  File  Function  Line  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

            #define uLOG_TITLES_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << bar << "\n" << "Time  Level  File  Function  Line  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

        #elif MICRO_LOG_DETAIL == 3
			// time level file_path func_sig line log

			#define uLOG(level) \
				if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
					             << level << " [" << __FILE__ << ":" << __PRETTY_FUNCTION__ << ":" << __LINE__ << "] "

			#define uLOG_(ofs, level) \
				if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
					    << level << " [" << __FILE__ << ":" << __PRETTY_FUNCTION__ << ":" << __LINE__ << "] "

            #define uLOG_TITLES(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << bar << "\n" << "Time  Level  FilePath  FunctionSig  Line  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

            #define uLOG_TITLES_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << bar << "\n" << "Time  Level  FilePath  FunctionSig  Line  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

        #elif MICRO_LOG_DETAIL == 4
			// time level uname PID log

			#if !defined(WIN32)

				#define uLOG(level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
						             << level << " [User=" << getlogin() << "; PID=" << getpid() << "] "

				#define uLOG_(ofs, level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
						    << level << " [User=" << getlogin() << "; PID=" << getpid() << "] "

            #else // WIN32

				#define uLOG(level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                    	GetUserName(username, UNLEN+1);
                        microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
						             << level << " [User=" << username << "; PID=" << _getpid() << "] "

				#define uLOG_(ofs, level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        GetUserName(username, UNLEN+1);
                        ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
						    << level << " [User=" << username << "; PID=" << _getpid() << "] "

			#endif // WIN32

            #define uLOG_TITLES(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << bar << "\n" << "Time  Level  User  PID  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

            #define uLOG_TITLES_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << bar << "\n" << "Time  Level  User  PID  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

        #elif MICRO_LOG_DETAIL >= 5
			// time level UID PID log

			#if !defined(WIN32)

				#define uLOG(level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
						             << level << " [UID=" << getuid() << "; PID=" << getpid() << "] "

				#define uLOG_(ofs, level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
						    << level << " [UID=" << getuid() << "; PID=" << getpid() << "] "

			#else // WIN32

				#define uLOG(level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        microLog_ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
						             << level << " [UID=" << "?" << "; PID=" << _getpid() << "] "

				#define uLOG_(ofs, level) \
					if(CheckLogLevel(level)) \
                        MICRO_LOG_LOCK; \
                        ofs << std::fixed << std::setprecision(3) << float(std::clock())/CLOCKS_PER_SEC << " " \
						    << level << " [UID=" << "?" << "; PID=" << _getpid() << "] "

			#endif // WIN32

            #define uLOG_TITLES(level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    microLog_ofs << bar << "\n" << "Time  Level  UID  PID  Log \n" << bar << endm; \
                    MICRO_LOG_UNLOCK

            #define uLOG_TITLES_(ofs, level) \
                if(CheckLogLevel(level)) \
                    MICRO_LOG_LOCK; \
                    ofs << bar << "\n" << "Time  Level  UID  PID  Log \n" << bar << endm; \
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

	#else // MICRO_LOG_ACTIVE

		struct nullstream {
			void open(const char *filename = 0, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
			{}
		};

		template <typename T>
		nullstream& operator<<(nullstream& s, T const&) { return s; }

		inline nullstream& operator<<(nullstream &s, std::ostream &(std::ostream&)) { return s; }

		inline nullstream& endm(nullstream& os) { return os; }

        // microLog initialization:
        // Use this macro once in the main()'s file
        #define MICRO_LOG_INIT                 \
            using namespace uLog;              \
            nullstream uLog::microLog_ofs

        extern nullstream microLog_ofs;

    	#define MICRO_LOG_START(logFile)
    	#define MICRO_LOG_START_APPEND(logFile)
        #define uLOG_TITLES(level)       if(0) microLog_ofs
        #define uLOG_TITLES_(ofs, level) if(0) ofs
        #define uLOGE                    ""
        #define uLOG(level)              if(0) microLog_ofs
        #define uLOG_(ofs, level)        if(0) ofs
        #define uLOGT(level)             if(0) microLog_ofs
        #define uLOGT_(ofs, level)       if(0) ofs
        #define uLOGD(level)
        #define uLOGD_(ofs, level)
        #define uLOGB(level)
        #define uLOGB_(ofs, level)

	#endif // MICRO_LOG_ACTIVE


} // namespace uLog

#endif // MICRO_LOG_HPP
