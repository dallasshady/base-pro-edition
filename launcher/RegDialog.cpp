// RegDialog.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "RegDialog.h"
#include ".\regdialog.h"
#include <string>


// CRegDialog dialog

IMPLEMENT_DYNAMIC(CRegDialog, CDialog)
CRegDialog::CRegDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CRegDialog::IDD, pParent)
{
}

CRegDialog::~CRegDialog()
{
}

void CRegDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_Keycode);
}


BEGIN_MESSAGE_MAP(CRegDialog, CDialog)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CRegDialog message handlers

void CRegDialog::OnBnClickedOk()
{
    // retrieve keycode
    CString string;
    m_Keycode.GetWindowText( string );    
    std::string keycode = string;

    // check keycode was entered
    if( !keycode.size() )
    {
        AfxMessageBox( "You must enter registration code!", IDOK, 0 );
        return;
    }

    // save keycode
    FILE* licFile = fopen( "./usr/licence.txt", "w" );
    fwrite( keycode.c_str(), keycode.size(), 1, licFile );
    fclose( licFile );

    OnOK();
}

void CRegDialog::OnBnClickedCancel()
{
    AfxMessageBox( "You must register your copy before you can play.", IDOK, 0 );
    OnCancel();
}
