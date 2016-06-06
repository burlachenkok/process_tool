#ifndef MOD_RUN_SUSPEND_H
#define MOD_RUN_SUSPEND_H

#include <windows.h>
#include <string>

namespace t1sdk{
    namespace tools{
        /** Run other process
        * @param theCmdline full cmdline
        * @param suspend create suspend process
        * @param wait wait end of the process
        * @param newConsole run cui in new console
        * @param executeLimitInSeconds execute limit in seconds if < 0 then not setup time constraint
        */
        int runProcess(const char* theCmdline, 
                        bool suspend = false, 
                        bool wait = false, 
                        bool newConsole = false, 
                        bool ignore_seh_exception = false,
                        unsigned long executeLimitInSeconds = -1,
			            unsigned long afinityMask = -1,
                        unsigned long stdoutFileHandle = 0,
                        unsigned long stderrFileHandle = 0,
                        unsigned long stdinFileHandle = 0
			);

        bool restrictProcess(HANDLE hProcess, unsigned int seconds, unsigned long afinityMask, bool ignore_seh_exception);

		bool terminateAllProcessesInJob();

		bool printAllProcessPidsInJob();

		bool printAllStatJobInfo();

		void setJobName(const std::string& name);
		
		void setWorkingDirectory2Run(const std::string& path);

	    void acquireJobAndSleep(int millisecsToSleep);
    }
}

#endif