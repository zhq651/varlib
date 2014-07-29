// modified by Stulle
#include "stdafx.h"

#include "SystemInfo.h"

CSystemInfo::CSystemInfo(){
	CSysInfo::Init();
}

CSystemInfo::~CSystemInfo(){

}

int CSystemInfo::GetCpuUsage(){
	return cpu.GetUsage( &sys, &upTime );
}

unsigned int CSystemInfo::GetMemoryUsage()
{
	return CSysInfo::GetProcessMemoryUsageInt();
}

int CSystemInfo::GetGlobalCpuUsage(){
	return sys;
}

unsigned long long CSystemInfo::GetGlobalMemoryUsage()
{
	return CSysInfo::GetTotalPhys()-CSysInfo::GetAvailPhys();
}