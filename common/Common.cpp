/**
 * @file Common.cpp
 * @brief some useful functions collected from other open source
 * @author liangsijian
 * @version 
 * @date 2015-04-26
 */
#include <windows.h>
#include <vector>
#include <ShlObj.h>
#include <shlwapi.h>
#include "Common.h"
#pragma comment(lib,"shlwapi.lib")
#pragma warning(suppress:4996)
/* string {{{1 */
std::vector<generic_string> tokenizeString(const generic_string & tokenString, const char delim) {
	//Vector is created on stack and copied on return
	std::vector<generic_string> tokens;

    // Skip delimiters at beginning.
	generic_string::size_type lastPos = tokenString.find_first_not_of(delim, 0);
    // Find first "non-delimiter".
    generic_string::size_type pos     = tokenString.find_first_of(delim, lastPos);

    while (pos != std::string::npos || lastPos != std::string::npos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(tokenString.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = tokenString.find_first_not_of(delim, pos);
        // Find next "non-delimiter"
        pos = tokenString.find_first_of(delim, lastPos);
    }
	return tokens;
}

/* search {{{2 */
bool isInList(const TCHAR *token, const TCHAR *list) {
	if ((!token) || (!list))
		return false;
	TCHAR word[64];
	int i = 0;
	int j = 0;
	for (; i <= int(lstrlen(list)) ; i++)
	{
		if ((list[i] == ' ')||(list[i] == '\0'))
		{
			if (j != 0)
			{
				word[j] = '\0';
				j = 0;
				
				if (!generic_stricmp(token, word))
					return true;
			}
		}
		else 
		{
			word[j] = list[i];
			j++;
		}
	}
	return false;
}

/* }}} search */
/* other {{{2 */

std::wstring string2wstring(const std::string & rString, UINT codepage)
{
	int len = MultiByteToWideChar(codepage, 0, rString.c_str(), -1, NULL, 0);
	if(len > 0)
	{		
		std::vector<wchar_t> vw(len);
		MultiByteToWideChar(codepage, 0, rString.c_str(), -1, &vw[0], len);
		return &vw[0];
	}
	else
		return L"";
}

std::string wstring2string(const std::wstring & rwString, UINT codepage/*  = CP_ACP */)
{
	int len = WideCharToMultiByte(codepage, 0, rwString.c_str(), -1, NULL, 0, NULL, NULL);
	if(len > 0)
	{		
		std::vector<char> vw(len);
		WideCharToMultiByte(codepage, 0, rwString.c_str(), -1, &vw[0], len, NULL, NULL);
		return &vw[0];
	}
	else
		return "";
}
/* }}} other */
/* }}} string */



/* file {{{1 */
void writeLog(const TCHAR *logFileName, const char *log2write)
{	
	FILE *f = generic_fopen(logFileName, TEXT("a+"));
	fwrite(log2write, sizeof(log2write[0]), strlen(log2write), f);
	fputc('\n', f);
	fflush(f);
	fclose(f);
}

/* }}} file */

/* gui {{{1 */

// Set a call back with the handle after init to set the path.
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/callbackfunctions/browsecallbackproc.asp
static int __stdcall BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM, LPARAM pData)
{
	if (uMsg == BFFM_INITIALIZED)
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
	return 0;
};
void folderBrowser(HWND parent, int outputCtrlID, const TCHAR* title, const TCHAR *defaultStr, TCHAR * rsDir)
{
    // This code was copied and slightly modifed from:
    // http://www.bcbdev.com/faqs/faq62.htm

    // SHBrowseForFolder returns a PIDL. The memory for the PIDL is
    // allocated by the shell. Eventually, we will need to free this
    // memory, so we need to get a pointer to the shell malloc COM
    // object that will free the PIDL later on.
    LPMALLOC pShellMalloc = 0;
    if (::SHGetMalloc(&pShellMalloc) == NO_ERROR)
    {
        // If we were able to get the shell malloc object,
        // then proceed by initializing the BROWSEINFO stuct
        BROWSEINFO info;
        memset(&info, 0, sizeof(info));
        info.hwndOwner = parent;
        info.pidlRoot = NULL;
        TCHAR szDisplayName[MAX_PATH];
        info.pszDisplayName = szDisplayName;
        info.lpszTitle = title;
        info.ulFlags = 0;
        info.lpfn = BrowseCallbackProc;
        TCHAR directory[MAX_PATH];
        if (parent)
            ::GetDlgItemText(parent, outputCtrlID, directory, sizeof(directory));
        if (!directory[0] && defaultStr)
            info.lParam = reinterpret_cast<LPARAM>(defaultStr);
        else
            info.lParam = reinterpret_cast<LPARAM>(directory);

        // Execute the browsing dialog.
        LPITEMIDLIST pidl = ::SHBrowseForFolder(&info);

        // pidl will be null if they cancel the browse dialog.
        // pidl will be not null when they select a folder.
        if (pidl)
        {
            // Try to convert the pidl to a display generic_string.
            // Return is true if success.
            TCHAR szDir[MAX_PATH];
            if (::SHGetPathFromIDList(pidl, szDir))
                // Set edit control to the directory path.
            if (parent)
                ::SetDlgItemText(parent, outputCtrlID, szDir);
            if (rsDir) {
               wcscpy(rsDir, szDir);
            }
            pShellMalloc->Free(pidl);
        }
        pShellMalloc->Release();
    }
}
void ClientRectToScreenRect(HWND hWnd, RECT* rect)
{
	POINT		pt;

	pt.x		 = rect->left;
	pt.y		 = rect->top;
	::ClientToScreen( hWnd, &pt );
	rect->left   = pt.x;
	rect->top    = pt.y;

	pt.x		 = rect->right;
	pt.y		 = rect->bottom;
	::ClientToScreen( hWnd, &pt );
	rect->right  = pt.x;
	rect->bottom = pt.y;
};


/* }}} gui */

/* util {{{1 */
/* directrion {{{2 */

int IsValidDirectory(const TCHAR *path)
{

    int Rc;
    Rc = GetFileAttributes(path);
    return Rc == INVALID_FILE_ATTRIBUTES ? FALSE : Rc & FILE_ATTRIBUTE_DIRECTORY;

} // IsValidDirectory


////////////////////////
// Creates a line of the dir.txt file
// use a callback function as argument since ScanDir is used either to
// create dir.txt or to dispaly the dir window

void ScanDir(int(*f)(TCHAR *s, DWORD dw), DWORD dwParam, const TCHAR *szDirectory)
{
    WIN32_FIND_DATA  FindData;
    FILETIME    FtLocal;
    SYSTEMTIME  SysTime;
    TCHAR        szLine[256], szFileSpec[_MAX_PATH + 5];
    TCHAR        szDate[sizeof "jj/mm/aaaa"];
    HANDLE      hFind;

    szFileSpec[_MAX_PATH - 1] = 0;
    lstrcpyn(szFileSpec, szDirectory, _MAX_PATH);
    lstrcat(szFileSpec, TEXT("\\*.*"));
    hFind = FindFirstFile(szFileSpec, &FindData);
    if (hFind != INVALID_HANDLE_VALUE)
    do
    {
        // display only files, skip directories
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  continue;
        FileTimeToLocalFileTime(&FindData.ftCreationTime, &FtLocal);
        FileTimeToSystemTime(&FtLocal, &SysTime);
        GetDateFormat(LOCALE_SYSTEM_DEFAULT,
            DATE_SHORTDATE,
            &SysTime,
            NULL,
            szDate, sizeof szDate);
        szDate[sizeof "jj/mm/aaaa" - 1] = 0;    // truncate date
        FindData.cFileName[62] = 0;      // truncate file name if needed
        // dialog structure allow up to 64 char
        if (FindData.nFileSizeLow < 0) {
            continue;
        }
        wsprintf(szLine, TEXT("%s\t%s\t%u"),
            FindData.cFileName, szDate, FindData.nFileSizeLow);

        (*f) (szLine, dwParam);
    } while (FindNextFile(hFind, &FindData));

    FindClose(hFind);

}  // ScanDir
/* }}}  directrion */

/* parse commandLine {{{2 */

typedef std::vector<const TCHAR*> ParamVector;
bool checkSingleFile(const TCHAR * commandLine) {
    TCHAR fullpath[MAX_PATH];
    ::GetFullPathName(commandLine, MAX_PATH, fullpath, NULL);
    if (::PathFileExists(fullpath)) {
        return true;
    }

    return false;
}
void parseCommandLine(TCHAR * commandLine, ParamVector & paramVector) {
    //params.erase(params.begin());	
    //remove the first element, since thats the path the the executable (GetCommandLine does that)
    //TCHAR stopChar = TEXT(' ');
    //if (commandLine[0] == TEXT('\"')) {
    //    stopChar = TEXT('\"');
    //    commandLine++;
    //}
    ////while this is not really DBCS compliant, space and quote are in the lower 127 ASCII range
    //while (commandLine[0] && commandLine[0] != stopChar)
    //{
    //    commandLine++;
    //}

    //// For unknown reason, the following command :
    //// c:\NppDir>notepad++
    //// (without quote) will give string "notepad++\0notepad++\0"
    //// To avoid the unexpected behaviour we check the end of string before increasing the pointer
    //if (commandLine[0] != '\0')
    //    commandLine++;	//advance past stopChar

    ////kill remaining spaces
    //while (commandLine[0] == TEXT(' '))
    //    commandLine++;

    //bool isFile = checkSingleFile(commandLine);	//if the commandline specifies only a file, open it as such
    //if (isFile) {
    //    paramVector.push_back(commandLine);
    //    return;
    //}
    bool isInFile = false;
    bool isInWhiteSpace = true;
    paramVector.clear();
    size_t commandLength = lstrlen(commandLine);
    for (size_t i = 0; i < commandLength; i++) {
        switch (commandLine[i]) {
        case '\"': {										//quoted filename, ignore any following whitespace
                       if (!isInFile) {	//" will always be treated as start or end of param, in case the user forgot to add an space
                           paramVector.push_back(commandLine + i + 1);	//add next param(since zero terminated generic_string original, no overflow of +1)
                       }
                       isInFile = !isInFile;
                       isInWhiteSpace = false;
                       //because we dont want to leave in any quotes in the filename, remove them now (with zero terminator)
                       commandLine[i] = 0;
                       break; }
        case '\t':	//also treat tab as whitespace
        case ' ': {
                      isInWhiteSpace = true;
                      if (!isInFile)
                          commandLine[i] = 0;		//zap spaces into zero terminators, unless its part of a filename
                      break; }
        default: {											//default TCHAR, if beginning of word, add it
                     if (!isInFile && isInWhiteSpace) {
                         paramVector.push_back(commandLine + i);	//add next param 
                         isInWhiteSpace = false;
                     }
                     break; }
        }
    }
    //the commandline generic_string is now a list of zero terminated strings concatenated, and the vector contains all the substrings
}

static int argc = 0;
static LPSTR argv[32];

static int SplitCmdLine(LPSTR lpszCmdLine)
{
    LPSTR p, q;
    argv[0] = "{{application}}";
    p = lpszCmdLine;
    while (p != NULL &&  *p)
    {  // for each argument 
        while ((*p) && (*p == ' ')) p++; // skip over leading spaces 
        if (*p == '"')
        {
            q = ++p; // skip double quotes
            // scan to end of argument 
            while ((*p) && (*p != '"'))  p++;
            argv[++argc] = q;
            if (*p)  *p++ = 0;
        }
        else if (*p)
        { // delimited by spaces 
            q = p;
            while ((*p) && (*p != ' '))  p++;
            argv[++argc] = q;
            if (*p)  *p++ = 0;
        }
    }  // parse all "words"
    // create empty 
    argv[++argc] = NULL;
    return argc;
} // SplitCmdLine
/* }}} parse commandLine */

#include <time.h>
#define MAXTIMESTR  20  //"mm/dd/yyyy/00:00:00"/0
char* timetoa(time_t t)
{
    static char timebuf[MAXTIMESTR];
    struct tm *tt = localtime(&t);
    if (tt == NULL)  timebuf[0] = 0;
    else           sprintf(timebuf, "%02d/%02d/%04d/%02d:%02d:%02d",
        tt->tm_mon + 1, tt->tm_mday, tt->tm_year + 1900,
        tt->tm_hour, tt->tm_min, tt->tm_sec);
    return timebuf;
} 

/* }}} util */
