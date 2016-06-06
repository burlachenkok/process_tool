#include "run_process.h"
#include <sstream>
#include <string>
#include <stdio.h>


// Job is container|sandbox to processes. The reasons to use this thing
// 1. If process create other child processes then they also will be killed by timeout
// 2. Is is impossible to process to leave the JOB
// 3. Is is possible to work with JOBS via OpenJobObject, 
// 4. Force process temination if unhandled SEH was happend
// 5. List process collection which was created in this job and kill them all
// 6. Take statistics: 

namespace
{
	class Base
	{
	public:
		Base()
		{
			g();
		}
		void g()
		{
			f();

		}
		virtual void f() = 0;
	};
	class  PureTest: public Base
	{
	public:
		PureTest()
		{
			f();
		}
		virtual void f() {}

	};
}

void pureCallTest()
{
	PureTest a;
}

void printHelp(int argc, char** argv)
{
	printf(
		"Run as:\n"
		"\n"
		"process_tool.exe <flags_for_launch> launch <cmdline>\n"
		"\t" "-jobname <name> - Use this to setupt asciiz jobname\n"
		"\t" "-new_console \t Create child process with separate console\n"
		"\t" "-suspend \t Create process and suspend it main thread\n"
		"\t" "-ignore_seh_exception \t Ignore SEH exception\n"
		"\t" "-wait \t Wait (or in posix terms join) for child process execution\n"
		"\t" "-timeExecLimit seconds \t Setup time execution limit per child process\n"
		"\t" "-afinity int_value_with_mask \t Setup affinity mask\n"
		"\t" "-workdir path \t Setup working directory for process\n"
        "\t" "-stdout_native path \t Setup stdout with WinApi inheritable HANDLE\n"
        "\t" "-stderr_native path \t Setup stderr with WinApi inheritable HANDLE\n"
        "\t" "-stdin_native path \t Setup stdin with WinApi inheritable HANDLE\n"
		"\n"
		"process_tool.exe <killall_flags> killall\n"
		"\t" "-jobname <name> \t Use this to setupt asciiz jobname\n"
		"\n"
		"process_tool.exe <jobinfo_flags> jobinfo\n"
		"\t" "-jobname <name> \t Use this to setupt asciiz jobname\n"
		"\n"
        "process_tool.exe purevcall\t\t(Emulate Pure Virtual Call)\n"
		"\n"
		"process_tool.exe <sleep_flags> sleep <milliceconds> \n"
		"optional sleep_flags:\n"
		"\t" "-jobname <name> \t Use this to setupt asciiz jobname\n"
		"\n\t\t\t\t\t\t(kburlachenko@gmail.com v4)\n"
		);
}

int main(int argc, char** argv)
{
	// Sleep(INFINITE);

    if (argc <= 1)
    {
		printHelp(argc, argv);
		return -1;
    }
    
    bool suspend = false;
    bool ignore_seh_exception = false;
    bool wait = false;
    bool new_console = false;
    bool launch = false;
    unsigned long int executeLimitInSeconds = -1;	
    unsigned long int afinityMask = -1;	
	std::string workingDir = "";	

    unsigned long stdoutFileHandle = 0;
    unsigned long stderrFileHandle = 0;
    unsigned long stdinFileHandle = 0;

    std::stringstream cmdline;

    for (int i = 1; i < argc; ++i)
    {
        std::string curArg(argv[i]);
        if (!launch)
        {
			if (curArg == "jobinfo")
			{
				t1sdk::tools::printAllProcessPidsInJob() &&
				t1sdk::tools::printAllStatJobInfo();

				return 0;
			}
			else if (curArg == "purevcall")
			{
				pureCallTest();
				//*(int*)0 = 11;
				return 1;
			}
			else if (curArg == "killall")
			{
				if (t1sdk::tools::terminateAllProcessesInJob())
				{
					printf("Successfully killed..\n");
				}
				return 0;
			}
			else if (curArg == "sleep")
			{
			    std::string nextArg(argv[++i]);
				int millisecsToSleep = 0;
			    std::stringstream(nextArg) >> millisecsToSleep;
 			    t1sdk::tools::acquireJobAndSleep(millisecsToSleep);
				return 0;
			}
			else if (curArg == "-jobname")
			{
				t1sdk::tools::setJobName(std::string(argv[++i]));
			}
			else if (curArg == "-workdir")
			{
				t1sdk::tools::setWorkingDirectory2Run(std::string(argv[++i]));
			}
			else if (curArg == "-timeExecLimit")
            {
                std::string nextArg(argv[++i]);
                std::stringstream(nextArg) >> executeLimitInSeconds;
            }
			else if (curArg == "-afinity")
			{
				std::string nextArg(argv[++i]);
				std::stringstream(nextArg) >> afinityMask;
			}
			else if (curArg == "-wait")
                wait = true;
            else if (curArg == "-suspend")
                suspend = true;
            else if (curArg == "-stdout_native")
            {
                std::string nextArg(argv[++i]);
                std::stringstream(nextArg) >> stdoutFileHandle;
            }
            else if (curArg == "-stderr_native")
            {
                std::string nextArg(argv[++i]);
                std::stringstream(nextArg) >> stderrFileHandle;
            }
            else if (curArg == "-stdin_native")
            {
                std::string nextArg(argv[++i]);
                std::stringstream(nextArg) >> stdinFileHandle;
            }
            else if (curArg == "-ignore_seh_exception")
                ignore_seh_exception = true;
            else if (curArg == "-new_console")
                new_console = true;
            else if (curArg == "launch")
                launch = true;
            else
            {
                printf("Unknown flag for process_tool: '%s'\n", curArg.c_str());
                return -1;
            }
        }
        else
        {
			if (!cmdline.str().empty())
				cmdline << " ";

			cmdline << curArg;
        }
    }

    return t1sdk::tools::runProcess(cmdline.str().c_str(), 
                                    suspend, wait, new_console, ignore_seh_exception, 
                                    executeLimitInSeconds, 
                                    afinityMask,
                                    stdoutFileHandle,
                                    stderrFileHandle,
                                    stdinFileHandle);
}
