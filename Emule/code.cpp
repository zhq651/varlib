#include <windows.h>

//将byte转换为16进制字符串
//The ByteToStr function converts an array of BYTE values to a hexadecimal character string.
void ByteToStr(
    DWORD cb,
    void* pv,
    LPSTR sz)
//--------------------------------------------------------------------
// Parameters passed are:
//    pv is the array of BYTES to be converted.
//    cb is the number of BYTEs in the array.
//    sz is a pointer to the string to be returned.

{
//--------------------------------------------------------------------
//  Declare and initialize local variables.

    BYTE* pb = (BYTE*) pv; // local pointer to a BYTE in the BYTE array
    DWORD i;               // local loop counter
    int b;                 // local variable

//--------------------------------------------------------------------
//  Begin processing loop.

    for (i = 0; i<cb; i++)
    {
        b = (*pb & 0xF0) >> 4;
        *sz++ = (b <= 9) ? b + '0' : (b - 10) + 'A';
        b = *pb & 0x0F;
        *sz++ = (b <= 9) ? b + '0' : (b - 10) + 'A';
        pb++;
    }
    *sz++ = 0;
}

CString ipstr(unsigned int nIP)
{
	// following gives the same string as 'inet_ntoa(*(in_addr*)&nIP)' but is not restricted to ASCII strings
	const BYTE* pucIP = (BYTE*)&nIP;
	CString strIP;
	strIP.ReleaseBuffer(_stprintf(strIP.GetBuffer(3+1+3+1+3+1+3), _T("%u.%u.%u.%u"), pucIP[0], pucIP[1], pucIP[2], pucIP[3]));
	return strIP;
}

CString ipstr(uint32 nIP, uint16 nPort)
{
	// following gives the same string as 'inet_ntoa(*(in_addr*)&nIP)' but is not restricted to ASCII strings
	const BYTE* pucIP = (BYTE*)&nIP;
	CString strIP;
	strIP.ReleaseBuffer(_stprintf(strIP.GetBuffer(3+1+3+1+3+1+3+1+5), _T("%u.%u.%u.%u:%u"), pucIP[0], pucIP[1], pucIP[2], pucIP[3], nPort));
	return strIP;
}

CString ipstr(LPCTSTR pszAddress, uint16 nPort)
{
	CString strIPPort;
	strIPPort.Format(_T("%s:%u"), pszAddress, nPort);
	return strIPPort;
}

CStringA ipstrA(uint32 nIP)
{
	const BYTE* pucIP = (BYTE*)&nIP;
	CStringA strIP;
	strIP.ReleaseBuffer(sprintf(strIP.GetBuffer(3+1+3+1+3+1+3), "%u.%u.%u.%u", pucIP[0], pucIP[1], pucIP[2], pucIP[3]));
	return strIP;
}

void ipstrA(CHAR* pszAddress, int iMaxAddress, uint32 nIP)
{
	const BYTE* pucIP = (BYTE*)&nIP;
	_snprintf(pszAddress, iMaxAddress, "%u.%u.%u.%u", pucIP[0], pucIP[1], pucIP[2], pucIP[3]);
}

bool IsUnicodeFile(LPCTSTR pszFilePath)
{
	bool bResult = false;
	FILE* fp = _tfsopen(pszFilePath, _T("rb"), _SH_DENYWR);
	if (fp != NULL)
	{
		WORD wBOM = 0;
		bResult = (fread(&wBOM, sizeof(wBOM), 1, fp) == 1 && wBOM == 0xFEFF);
		fclose(fp);
	}
	return bResult;
}



uint16 GetRandomUInt16()
{
#if RAND_MAX == 0x7fff
	// get 2 random numbers
	UINT uRand0 = rand();
	UINT uRand1 = rand();

	// NOTE: if that assert fires, you have most likely called that function *without* calling 'srand' first.
	// NOTE: each spawned thread HAS to call 'srand' for itself to get real random numbers.
	ASSERT( !(uRand0 == 41 && uRand1 == 18467) );

	return (uint16)(uRand0 | ((uRand1 >= RAND_MAX/2) ? 0x8000 : 0x0000));
#else
#error "Implement it!"
#endif
}

uint32 GetRandomUInt32()
{
#if RAND_MAX == 0x7fff
	//return ((uint32)GetRandomUInt16() << 16) | (uint32)GetRandomUInt16();
	// this would give the receiver the following information:
	//	random number N
	//	random number N+1 is below or greater/equal than 0x8000
	//	random number N+2
	//	random number N+3 is below or greater/equal than 0x8000

	uint32 uRand0 = GetRandomUInt16();
	srand(GetTickCount() | uRand0);
	uint32 uRand1 = GetRandomUInt16();
	return (uRand0 << 16) | uRand1;
#else
#error "Implement it!"
#endif
}
