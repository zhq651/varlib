// modified by Stulle
#pragma once

#include "SysInfo.h"
#include "CPU.h"

class CSystemInfo : public CSysInfo
{
public:
	CSystemInfo();
	~CSystemInfo();

	int		GetCpuUsage();
	unsigned int GetMemoryUsage();
	int		GetGlobalCpuUsage();
	unsigned long long GetGlobalMemoryUsage();

private:
	CPU cpu;
    int sys;
    TKTime upTime;
};
