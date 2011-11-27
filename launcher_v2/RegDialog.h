#pragma once
#include "afxwin.h"


// CRegDialog dialog

class CRegDialog : public CDialog
{
	DECLARE_DYNAMIC(CRegDialog)

public:
	CRegDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRegDialog();

// Dialog Data
	enum { IDD = IDD_REGISTER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    CEdit m_Keycode;
    afx_msg void OnBnClickedCancel();
};
