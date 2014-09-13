/// microLog_test.cpp

// clang++ -std=c++11 ./microLog_test.cpp
// clang++ -std=c++11 -stdlib=libc++ ./microLog_test.cpp

// These macros can also be defined in the makefile, or in microLog_config.hpp:
/*
#define MICRO_LOG_ACTIVE 1
#define MICRO_LOG_FILE_NAME "test.log"
#define MICRO_LOG_MIN_LEVEL 4
*/

#define MICRO_LOG_ACTIVE 1
#include "microLog.hpp"

#include <iostream>

using namespace uLog;

MICRO_LOG_INIT;

int main()
{
//    MICRO_LOG_FORMAT();

//	MICRO_LOG_START("custom.log");
	MICRO_LOG_START_APPEND("custom.log");

//+B	uLOG(std::cerr, 5) << "Test log message number " << 2 << " with value " << 3.141 << uLOGE;

	uLOGB(5);		// separator
	uLOGD(5);		// date

	uLOG(5) << "Test log message number " << 2 << " with value " << 3.141 << uLOGE;

	uLOG(warning) << "Log made of separate tokens... ";
	uLOGT(warning) << "first token, ";
	uLOGT(warning) << "last token" << uLOGE;

	uLOG(error) << "Test log message number 1." << uLOGE;
	
	for(int i = 1; i < 10; ++i) {
		uLOG(i) << "Test log message number " << i << " with value " << 1.23*i << uLOGE;
	}

}
