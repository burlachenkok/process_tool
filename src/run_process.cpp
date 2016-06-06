#include <windows.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <string>

static std::string jobName = std::string("testRestrictProcessMyJob2");
static std::string workDir = std::string();

namespace t1sdk{ namespace tools{

bool pushProcessToJob(HANDLE hProcess, HANDLE hJob)	
{
//    HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | // for CreateRemoteTnread
//                                  PROCESS_VM_OPERATION |  // for VirtualAllocEx/VirtualFreeEx
//                                  PROCESS_VM_WRITE,       // for WriteProcessMemory
//                                  FALSE, processId);    
    BOOL inJob = FALSE;
    IsProcessInJob(hProcess, NULL, &inJob);
    if (inJob)
    {
        printf("Process already in job...\n");
        return false;
    }
	AssignProcessToJobObject(hJob, hProcess);
    return true;
}

bool updateJobRestrictions(HANDLE hJob, unsigned int executionTimeLimit, unsigned long afinityMask, bool ignore_seh_exception)
{
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION li = {0};
	li.BasicLimitInformation.PriorityClass = NORMAL_PRIORITY_CLASS;  // process always run with this priority

	if (afinityMask != unsigned long(-1))
	{
		li.BasicLimitInformation.Affinity    = afinityMask;
		li.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_AFFINITY;
	}

	if (executionTimeLimit != unsigned long(-1))
	{
		
		li.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = executionTimeLimit * 10 * 1000;
		li.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_TIME;
		
		/*
		li.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = executionTimeLimit;
		li.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_TIME;
		*/

	}
	if (ignore_seh_exception)
	{
		li.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION;
	}
	SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &li, sizeof(li));	
	return true;
}

bool terminateAllProcessesInJob()
{
	HANDLE hJob = OpenJobObject(JOB_OBJECT_TERMINATE, false, jobName.c_str());
	if (hJob == NULL)
	{
		printf("Job '%s' was not found...\n", jobName.c_str());
		return false;
	}

	BOOL res = TerminateJobObject(hJob, 0);
	CloseHandle(hJob);
	return SUCCEEDED(res);
}

bool printAllProcessPidsInJob() {
	HANDLE hJob = OpenJobObject(JOB_OBJECT_QUERY, false, jobName.c_str());
	if (hJob == NULL)
	{
		printf("Job '%s' was not found...\n", jobName.c_str());
		return false;
	}

	const int kMaxProcessesInJob = 12;
	DWORD cb = sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST) + (kMaxProcessesInJob - 1) * sizeof(DWORD);
	PJOBOBJECT_BASIC_PROCESS_ID_LIST pjobpil = (PJOBOBJECT_BASIC_PROCESS_ID_LIST) malloc(cb);
	pjobpil->NumberOfAssignedProcesses = kMaxProcessesInJob;
	QueryInformationJobObject(hJob, JobObjectBasicProcessIdList, pjobpil, cb, &cb);

	printf("Job name: '%s'\n", jobName.c_str());
	printf("Process number in the job: %i\n", int(pjobpil->NumberOfProcessIdsInList));
	for (DWORD x = 0; x < pjobpil->NumberOfProcessIdsInList; x++) 
	{
		unsigned long pid = pjobpil->ProcessIdList[x];
		printf("Process #%i in the job. it's pid: %lu\n", int(x), pid);
	}
	free(pjobpil);
	CloseHandle(hJob);
	printf("\n");

	return true;
}

bool printAllStatJobInfo() {
	HANDLE hJob = OpenJobObject(JOB_OBJECT_QUERY, false, jobName.c_str());
	if (hJob == NULL)
	{
		printf("Job '%s' was not found...\n", jobName.c_str());
		return false;
	}

	JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION info = {};
	DWORD returnLength = 0; 
	QueryInformationJobObject(hJob, JobObjectBasicAndIoAccountingInformation, &info, sizeof(info), &returnLength);
	CloseHandle(hJob);

	typedef unsigned long long uint64_cast;
	printf("All prrocesses from the job consumed user time: %llu (milliseconds)\n", uint64_cast(info.BasicInfo.TotalUserTime.QuadPart/10000));
	printf("All prrocesses from the job consumed kernel time: %llu (milliseconds)\n", uint64_cast(info.BasicInfo.TotalKernelTime.QuadPart/10000));
	printf("Number of current active processes in job: %llu\n", uint64_cast(info.BasicInfo.ActiveProcesses));
	printf("Full count of active/terminated processes: %llu\n", uint64_cast(info.BasicInfo.TotalProcesses));
	printf("Number of terminated by break-limit processes in job: %llu\n", uint64_cast(info.BasicInfo.TotalTerminatedProcesses));

	printf("Read bytes for all processes: %llu\n", uint64_cast(info.IoInfo.ReadTransferCount));
	printf("Read operations count for all processes: %llu\n", uint64_cast(info.IoInfo.ReadOperationCount));
	printf("Write bytes for all processes: %llu\n", uint64_cast (info.IoInfo.WriteTransferCount));
	printf("Write operations count for all processes: %llu\n", uint64_cast(info.IoInfo.WriteOperationCount));
	printf("Other not read/write bytes for input-output: %llu\n", uint64_cast(info.IoInfo.OtherTransferCount));

	return true;
}

int runProcess(const char* theCmdline, 
               bool suspend, 
               bool wait, 
               bool newConsole, 
               bool ignore_seh_exception, 
               unsigned long executeLimitInSeconds, 
               unsigned long afinityMask, 
               unsigned long stdoutFileHandle,
               unsigned long stderrFileHandle,
               unsigned long stdinFileHandle
               )
{
    char cmdline[1024] = {};
    if (strlen(theCmdline) > sizeof(cmdline))
    {
        assert(!"Insufficient buffer size for store cmdline");
        return -1;
    }
    strcpy(cmdline, theCmdline);
    
    // Find executable image here (if it is not specified in cmdline):
    // 1. Directory which hold exe module of current process
    // 2. Current working directory
    // 3. Windows system directory (System32). See: GetSystemDirectory
    // 4. Major Windows directory
    // 5. Directories from PATH env.variable  
    // If there is no ".exe" prefix in cmdline

    bool inheritHandles = true; // copy all kernel-handle objects => into child process in same places with sama flags with same handle-value. 
                                // (handle-value are just <number to interal process kernel table> <couple bits for mask, include inheritance>)

    DWORD flags = 0;

    flags |= CREATE_SUSPENDED;     // Create process with suspended thread
    if (newConsole)
        flags |= CREATE_NEW_CONSOLE; // Create process with new console

    //flags |= DETACHED_PROCESS;          // Create cui process with detached (new) console window which will be created via AllocConsole
    //flags |= CREATE_NO_WINDOW;          // Create without new window
    //flags |= CREATE_DEFAULT_ERROR_MODE; // Use default system error mode for SetErrorMode
    const char* curDirectory = 0;               // Use parent directory

	if (!workDir.empty())
	{
		curDirectory = workDir.c_str();
	}

    STARTUPINFO si = {};
    si.cb = sizeof(si);
    si.hStdError = si.hStdInput = si.hStdOutput = INVALID_HANDLE_VALUE;

    if (stdoutFileHandle || stdinFileHandle || stderrFileHandle)
    {
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdInput =  GetStdHandle(STD_INPUT_HANDLE);
        si.hStdError =  GetStdHandle(STD_ERROR_HANDLE);
    }
    if (stdoutFileHandle != 0)
    {
        si.hStdOutput = (HANDLE)stdoutFileHandle;
    }
    if (stdinFileHandle != 0)
    {
        si.hStdInput = (HANDLE)stdinFileHandle;
    }
    if (stderrFileHandle != 0)
    {
        si.hStdError = (HANDLE)stderrFileHandle;
    }

    PROCESS_INFORMATION pi;
    HANDLE hJob = CreateJobObjectA(NULL, jobName.c_str()); // let child inherit job object handle
    SetHandleInformation(hJob, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	updateJobRestrictions(hJob, executeLimitInSeconds, afinityMask, ignore_seh_exception);

    BOOL newProcess = CreateProcessA(NULL, cmdline, NULL, NULL, inheritHandles, flags, NULL, curDirectory, &si, &pi);
    if (newProcess == TRUE)
    {
		pushProcessToJob(pi.hProcess, hJob);
		
        if(!suspend)
        {
            ResumeThread(pi.hThread);
        }

        // Close handle of child process and it's primary thread
        CloseHandle(pi.hThread);
        
        if (!wait) // Create detached process
        {
            CloseHandle(pi.hProcess);
            CloseHandle(hJob);
            return 0;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        printf("CHILD PROCESS CMDLINE: %s\n", cmdline);
        printf("CHILD PROCESS EXIT CODE: %u\n", unsigned int(exitCode));
        printf("CHILD PROCESS ID: %i\n", int(GetProcessId(pi.hProcess)));
        CloseHandle(pi.hProcess);
        CloseHandle(hJob);
        return exitCode;
    }
    else
    {
        printf("CAN NOT RUN CHILD PROCESS CMDLINE: %s\n", cmdline);
        DWORD lastError = GetLastError();
        printf("LAST ERROR: 0x%X\n", unsigned int(lastError));
        CloseHandle(hJob);
        return -1;
    }
}


void setJobName(const std::string& name)
{
	jobName = name;
}

void setWorkingDirectory2Run(const std::string& path)
{
	workDir = path;
}

void acquireJobAndSleep(int millisecsToSleep)
{
    HANDLE hJob = CreateJobObjectA(NULL, jobName.c_str()); // let child inherit job object handle
	printf("Acquire job name: '%s'\n", jobName.c_str());
	printf("Sleep: '%i milliseconds'\n", millisecsToSleep);
	Sleep(millisecsToSleep);
    CloseHandle(hJob);
}

}}
