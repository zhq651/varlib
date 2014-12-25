// XBrowseForFolder.h  Version 1.2
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// This software is released into the public domain.  You are free to use
// it in any way you like, except that you may not sell this source code.
//
// This software is provided "as is" with no expressed or implied warranty.
// I accept no liability for any damage or loss of business that this
// software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XBROWSEFORFOLDER_H
#define XBROWSEFORFOLDER_H
//打开文件夹
BOOL XBrowseForFolder(HWND hWnd,
					  LPCTSTR lpszInitialFolder,
					  int nFolder,
					  LPCTSTR lpszCaption,
					  LPTSTR lpszBuf,
					  DWORD dwBufSize,
					  BOOL bEditBox = FALSE);

//example

// void CXBrowseForFolderTestDlg::OnSHBrowseForFolder() 
// {
	// UpdateData(TRUE);

	// m_edtReturnCode.SetWindowText(_T(""));
	// m_edtFolder.SetWindowText(_T(""));

	// BROWSEINFO bi = { 0 };

	// bi.hwndOwner = m_hWnd;
	// bi.ulFlags   = BIF_RETURNONLYFSDIRS;
	// if (m_bEditBox)
		// bi.ulFlags |= BIF_EDITBOX;
	// bi.ulFlags |= BIF_NONEWFOLDERBUTTON;
	// int nRootFolder = m_nCsidl[m_nRootIndex].nCsidl;
	// LPITEMIDLIST pidlRoot = NULL;
	// if (SUCCEEDED(SHGetSpecialFolderLocation(m_hWnd, nRootFolder, &pidlRoot)))
		// bi.pidlRoot = pidlRoot;

	// LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	// BOOL bRet = FALSE;

	// TCHAR szFolder[MAX_PATH*2] = { _T('\0') };

	// if (pidl)
	// {
		// if (SHGetPathFromIDList(pidl, szFolder))
		// {
			// bRet = TRUE;
		// }

		// IMalloc *pMalloc = NULL; 
		// if (SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc) 
		// {  
			// pMalloc->Free(pidl);  
			// pMalloc->Release(); 
		// }
	// }

	// m_edtReturnCode.SetWindowText(bRet ? _T("TRUE") : _T("FALSE"));
	// m_edtFolder.SetWindowText(szFolder);
// }

// void CXBrowseForFolderTestDlg::OnXBrowseForFolder() 
// {
	// UpdateData(TRUE);

	// m_edtReturnCode.SetWindowText(_T(""));
	// m_edtFolder.SetWindowText(_T(""));

	// TCHAR szFolder[MAX_PATH*2] = { _T('\0') };

	// LPCTSTR lpszInitial = m_strInitialFolder;
	// if (m_nInitial)
	// {
		// int nFolder = m_nCsidl[m_nInitialIndex].nCsidl;
		// TCHAR szInitialPath[MAX_PATH*2] = { _T('\0') };
		// SHGetSpecialFolderPath(m_hWnd, szInitialPath, nFolder, FALSE);
		// if (szInitialPath[0] == _T('\0'))
		// {
			// AfxMessageBox(_T("The initial CSIDL must be a real folder.  \r\n\r\n")
						  // _T("Please select another CSIDL."),
						  // MB_OK);
			// return;
		// }
		// lpszInitial = (LPCTSTR) nFolder;
	// }

	// BOOL bRet = XBrowseForFolder(m_hWnd,
								 // lpszInitial,
								 // m_nCsidl[m_nRootIndex].nCsidl,
								 // m_strCaption,
								 // szFolder,
								 // sizeof(szFolder)/sizeof(TCHAR)-2,
								 // m_bEditBox);

	// m_edtReturnCode.SetWindowText(bRet ? _T("TRUE") : _T("FALSE"));
	// m_edtFolder.SetWindowText(szFolder);
// }

#endif //XBROWSEFORFOLDER_H
