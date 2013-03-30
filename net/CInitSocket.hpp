#ifndef C_INITSOCKET_H_
#define C_INITSOCKET_H_
#ifdef WIN32
#include <winsock2.h>
/*
 * class CInitSocket just is a init class for win32 socket,
 * you just define a object of CInitSocket as global var like
 * CInitSocket init;
 * int main(){
 * ......
 * }
 *
 */
class CInitSocket{
	CInitSocket(){
		WORD wVersionRequested;
		WSADATA wsaData;
		wVersionRequested = MAKEWORD( 2, 0 ); 
		int err = WSAStartup( wVersionRequested, &wsaData );
		if (err)
		{
			if (LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 0 )
			{
				WSACleanup( );
                /* There are error,exit(1) */
				exit(1);
			}
		}
	}
	~CInitSocket(){
		if (!WSACleanup())       //clean up
		{
			WSAGetLastError();
			return;
		}
	}
};
#else
class CInitSocket{
};
#endif
#endif
