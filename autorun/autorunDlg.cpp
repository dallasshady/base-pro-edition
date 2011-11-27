// autorunDlg.cpp : implementation file
//

#include "stdafx.h"
#include "autorun.h"
#include "autorunDlg.h"
#include ".\autorundlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ----------------------------------------------------------------------------------------
// CautorunButton

CautorunButton::CautorunButton(UINT iId, UINT aId, UINT left, UINT top, UINT right, UINT bottom)
{
    _iId    = iId;
    _aId    = aId;
    _left   = left;
    _top    = top;
    _right  = right;
    _bottom = bottom;
    
    _iBitmap = new CBitmap;
    BOOL iBitmapIsLoaded = _iBitmap->LoadBitmap( iId ); assert( iBitmapIsLoaded );
    
    _aBitmap = new CBitmap;
    BOOL aBitmapIsLoaded = _aBitmap->LoadBitmap( aId ); assert( aBitmapIsLoaded );
}

CautorunButton::~CautorunButton()
{
    delete _iBitmap;
    delete _aBitmap;
}

void CautorunButton::render(CDC* windc, CDC* dcmemory, UINT cx, UINT cy)
{
    CBitmap *pbmOriginal;
    
    if( isInside( cx, cy ) )
    {
        pbmOriginal = dcmemory->SelectObject( _aBitmap );
    }
    else
    {
        pbmOriginal = dcmemory->SelectObject( _iBitmap );
    }

    windc->BitBlt( _left, _top, _right - _left, _bottom - _top, dcmemory, 0, 0, SRCPAINT );
}

bool CautorunButton::isInside(UINT cx, UINT cy)
{
    return ( cx >= _left && cx <= _right && cy >= _top && cy <= _bottom );
}

// ----------------------------------------------------------------------------------------
// CCloseButton

CCloseButton::CCloseButton() : CautorunButton( IDB_CLOSEI, IDB_CLOSEA, 608, 8, 631, 31 )
{
}

void CCloseButton::onClick(void)
{
    exit( 0 );
}

// ----------------------------------------------------------------------------------------
// CInstallCodecButton

CInstallCodecButton::CInstallCodecButton() : CautorunButton( IDB_INSTALLCODECI, IDB_INSTALLCODECA, 96, 96, 303, 119 )
{
}

void CInstallCodecButton::onClick(void)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    CreateProcess( NULL, ".\\codec\\wmv9VCMsetup.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi  );
}

// ----------------------------------------------------------------------------------------
// CInstallNetFrameworkButton

CInstallNetFrameworkButton::CInstallNetFrameworkButton() : CautorunButton( IDB_RU_INSTALLNETFRAMEWORKI, IDB_RU_INSTALLNETFRAMEWORKA, 96, 136, 303, 159 )
{
}

void CInstallNetFrameworkButton::onClick(void)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    CreateProcess( NULL, ".\\prerequisites\\netframework.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi  );
}

// ----------------------------------------------------------------------------------------
// CInstallGameButton

CInstallGameButton::CInstallGameButton() : CautorunButton( IDB_RU_INSTALLGAMEI, IDB_RU_INSTALLGAMEA, 96, 176, 303, 199 )
{
}

void CInstallGameButton::onClick(void)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    CreateProcess( NULL, "setup.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi  );

    // prevent CD-ROM bounding
    exit( 0 );
}

// ----------------------------------------------------------------------------------------
// CInstallDirectXButton

CInstallDirectXButton::CInstallDirectXButton() : CautorunButton( IDB_RU_INSTALLDXI, IDB_RU_INSTALLDXA, 96, 216, 303, 239 )
{
}

void CInstallDirectXButton::onClick(void)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    CreateProcess( NULL, ".\\directx\\dxsetup.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi  );
}

// ----------------------------------------------------------------------------------------
// CInstallAgeiaButton

CInstallAgeiaButton::CInstallAgeiaButton() : CautorunButton( IDB_RU_INSTALLAGEIAI, IDB_RU_INSTALLAGEIAA, 96, 256, 303, 279 )
{
}

void CInstallAgeiaButton::onClick(void)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    CreateProcess( NULL, ".\\ageia\\physx.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi  );
}

// ----------------------------------------------------------------------------------------
// CBonusButton

CBonusButton::CBonusButton() : CautorunButton( IDB_RU_BONUSI, IDB_RU_BONUSA, 96, 288, 303, 311 )
{
}

void CBonusButton::onClick(void)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    CreateProcess( NULL, "explorer \".\\bonus\\\"", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi  );
}

// ----------------------------------------------------------------------------------------
// CReadmeButton

CReadmeButton::CReadmeButton() : CautorunButton( IDB_RU_READMEI, IDB_RU_READMEA, 96, 320, 303, 343 )
{
}

void CReadmeButton::onClick(void)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    CreateProcess( NULL, "notepad readme.txt", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi  );
}

// ----------------------------------------------------------------------------------------
// CautorunDlg dialog

CautorunDlg::CautorunDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CautorunDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CautorunDlg::~CautorunDlg()
{
    while( m_Buttons.size() )
    {
        delete ( *m_Buttons.begin() );
        m_Buttons.erase( m_Buttons.begin() );
    }
}

void CautorunDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CautorunDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_WM_CLOSE()
    ON_WM_MOUSEMOVE()
    ON_WM_MBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CautorunDlg message handlers

BOOL CautorunDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    SetWindowText( "B.A.S.E." );
    m_ButtonUnderCursor = NULL;
    m_MousePos.x = m_MousePos.y = 0;
    m_Buttons.push_back( new CCloseButton() );
    m_Buttons.push_back( new CInstallCodecButton() );
    m_Buttons.push_back( new CInstallNetFrameworkButton() );
    m_Buttons.push_back( new CInstallGameButton() );
    m_Buttons.push_back( new CInstallDirectXButton() );
    m_Buttons.push_back( new CInstallAgeiaButton() );
    m_Buttons.push_back( new CBonusButton() );
    m_Buttons.push_back( new CReadmeButton() );
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CautorunDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		// draw background image
        CDC dcMemory;
        dcMemory.CreateCompatibleDC(GetDC());

        CBitmap bitmap;
        bitmap.LoadBitmap( IDB_WALLPAPER );
        CBitmap *pbmOriginal = dcMemory.SelectObject(&bitmap);

        GetDC()->BitBlt( 0,0,640,480, &dcMemory, 0,0, SRCCOPY );

        // draw buttons
        for( CautorunButtonI buttonI = m_Buttons.begin();
                             buttonI != m_Buttons.end();
                             buttonI++ )
        {
            (*buttonI)->render( GetDC(), &dcMemory, m_MousePos.x, m_MousePos.y );
        }

        // draw all
        CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CautorunDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CautorunDlg::OnClose()
{
    CDialog::OnClose();
}

void CautorunDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    // save point
    m_MousePos = point;

    // detect button under cursor
    CautorunButton* currentlyUnderCursor = NULL;
    for( CautorunButtonI buttonI = m_Buttons.begin();
                         buttonI != m_Buttons.end();
                         buttonI++ )
    {
        if( (*buttonI)->isInside( m_MousePos.x, m_MousePos.y ) )
        {
            currentlyUnderCursor = (*buttonI);
            break;
        }
    }
    if( currentlyUnderCursor != m_ButtonUnderCursor )
    {
        m_ButtonUnderCursor = currentlyUnderCursor;
        OnPaint();
    }

    CDialog::OnMouseMove(nFlags, point);
}

void CautorunDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
    CDialog::OnMButtonDown(nFlags, point);
}

void CautorunDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    if( m_ButtonUnderCursor ) m_ButtonUnderCursor->onClick();

    CDialog::OnLButtonDown(nFlags, point);
}
