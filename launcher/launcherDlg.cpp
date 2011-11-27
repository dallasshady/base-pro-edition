// launcherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "launcherDlg.h"
#include "VideoPlayer.h"
#include "RegDialog.h"
#include ".\launcherdlg.h"
#include "keyboard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ClauncherDlg dialog



ClauncherDlg::ClauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ClauncherDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void ClauncherDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ADAPTER, m_Adapter);
    DDX_Control(pDX, IDC_MODE, m_VideoMode);
    DDX_Control(pDX, IDC_FULLSCREEN, m_Fullscreen);
    DDX_Control(pDX, IDC_VSYNC, m_Sync);
    DDX_Control(pDX, IDC_AFTERFX, m_AfterFx);
    DDX_Control(pDX, IDC_SKINNEDMESH, m_SkinShader);
    DDX_Control(pDX, IDC_GRASS, m_Grass);
    DDX_Control(pDX, IDC_CROWD, m_Crowd);
    DDX_Control(pDX, IDC_TRAFFIC, m_Traffic);
    DDX_Control(pDX, IDC_SLIDER1, m_MusicVolume);
    DDX_Control(pDX, IDC_LEFT, mctrl_Left);
    DDX_Control(pDX, IDC_RIGHT, mctrl_Right);
    DDX_Control(pDX, IDC_UP, mctrl_Up);
    DDX_Control(pDX, IDC_DOWN, mctrl_Down);
    DDX_Control(pDX, IDC_LEFT_SKEW, mctrl_LeftSkew);
    DDX_Control(pDX, IDC_RIGHT_SKEW2, mctrl_RightSkew);
    DDX_Control(pDX, IDC_JUMP_PULL, mctrl_JumpPull);
    DDX_Control(pDX, IDC_RUN_TRACK, mctrl_RunTrack);
    DDX_Control(pDX, IDC_ALTIMETER_MODE, mctrl_AltimeterMode);
    DDX_Control(pDX, IDC_INCREASE_WARNING_ALTITUDE, mctrl_IncreaseWA);
    DDX_Control(pDX, IDC_DECREASE_WARNING_ALTITUDE, mctrl_DecreaseWA);
    DDX_Control(pDX, IDC_WLO_TOGGLES, mctrl_WLO);
    DDX_Control(pDX, IDC_HOOK_KNIFE, mctrl_Hook);
    DDX_Control(pDX, IDC_FIRST_PERSON_VIEW, mctrl_FirstPerson);
    DDX_Control(pDX, IDC_EXTERNAL_VIEW, mctrl_External);
    DDX_Control(pDX, IDC_FLYBY_VIEW, mctrl_Flyby);
    DDX_Control(pDX, IDC_SWITCH_HUD_MODE, mctrl_SwitchHUD);
    DDX_Control(pDX, IDC_INCREASE_TIMEFLOW, mctrl_IncreaseTime);
    DDX_Control(pDX, IDC_DECREASE_TIMEFLOW, mctrl_DecreaseTime);
    DDX_Control(pDX, IDC_INCREASE_MUSIC_VOLUME, mctrl_IncreaseMusic);
    DDX_Control(pDX, IDC_DECREASE_MUSIC_VOLUME, mctrl_DecreaseMusic);
    DDX_Control(pDX, IDC_START_GAME, m_StartButton);
}

BEGIN_MESSAGE_MAP(ClauncherDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_CBN_SELCHANGE(IDC_ADAPTER, OnCbnSelchangeAdapter)
    ON_WM_DESTROY()
    ON_CBN_SELCHANGE(IDC_MODE, OnCbnSelchangeMode)
    ON_BN_CLICKED(IDC_FULLSCREEN, OnBnClickedFullscreen)
    ON_BN_CLICKED(IDC_VSYNC, OnBnClickedVsync)
    ON_BN_CLICKED(IDC_AFTERFX, OnBnClickedAfterfx)
    ON_CBN_SELCHANGE(IDC_SKINNEDMESH, OnCbnSelchangeSkinnedmesh)
    ON_CBN_SELCHANGE(IDC_CROWD, OnCbnSelchangeCrowd)
    ON_CBN_SELCHANGE(IDC_TRAFFIC, OnCbnSelchangeTraffic)
    ON_BN_CLICKED(IDC_GRASS, OnBnClickedGrass)
    ON_BN_CLICKED(IDC_START_GAME, OnBnClickedStartGame)
    ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, OnNMReleasedcaptureSlider1)
    ON_CBN_SELCHANGE(IDC_LEFT, OnCbnSelchangeLeft)
    ON_CBN_SELCHANGE(IDC_UP, OnCbnSelchangeUp)
    ON_CBN_SELCHANGE(IDC_DOWN, OnCbnSelchangeDown)
    ON_CBN_SELCHANGE(IDC_LEFT_SKEW, OnCbnSelchangeLeftSkew)
    ON_CBN_SELCHANGE(IDC_RIGHT, OnCbnSelchangeRight)
    ON_CBN_SELCHANGE(IDC_RIGHT_SKEW2, OnCbnSelchangeRightSkew2)
    ON_CBN_SELCHANGE(IDC_JUMP_PULL, OnCbnSelchangeJumpPull)
    ON_CBN_SELCHANGE(IDC_RUN_TRACK, OnCbnSelchangeRunTrack)
    ON_CBN_SELCHANGE(IDC_ALTIMETER_MODE, OnCbnSelchangeAltimeterMode)
    ON_CBN_SELCHANGE(IDC_INCREASE_WARNING_ALTITUDE, OnCbnSelchangeIncreaseWarningAltitude)
    ON_CBN_SELCHANGE(IDC_DECREASE_WARNING_ALTITUDE, OnCbnSelchangeDecreaseWarningAltitude)
    ON_CBN_SELCHANGE(IDC_WLO_TOGGLES, OnCbnSelchangeWloToggles)
    ON_CBN_SELCHANGE(IDC_HOOK_KNIFE, OnCbnSelchangeHookKnife)
    ON_CBN_SELCHANGE(IDC_FIRST_PERSON_VIEW, OnCbnSelchangeFirstPersonView)
    ON_CBN_SELCHANGE(IDC_EXTERNAL_VIEW, OnCbnSelchangeExternalView)
    ON_CBN_SELCHANGE(IDC_FLYBY_VIEW, OnCbnSelchangeFlybyView)
    ON_CBN_SELCHANGE(IDC_SWITCH_HUD_MODE, OnCbnSelchangeSwitchHudMode)
    ON_CBN_SELCHANGE(IDC_INCREASE_TIMEFLOW, OnCbnSelchangeIncreaseTimeflow)
    ON_CBN_SELCHANGE(IDC_DECREASE_TIMEFLOW, OnCbnSelchangeDecreaseTimeflow)
    ON_CBN_SELCHANGE(IDC_INCREASE_MUSIC_VOLUME, OnCbnSelchangeIncreaseMusicVolume)
    ON_CBN_SELCHANGE(IDC_DECREASE_MUSIC_VOLUME, OnCbnSelchangeDecreaseMusicVolume)
END_MESSAGE_MAP()


// ClauncherDlg message handlers

CComboBox* ClauncherDlg::SetupMappingControl(CComboBox* comboBox, TiXmlElement* configElement, void* userData)
{
    assert( comboBox );
    assert( configElement );

    // fill combo box with mapping variants
    int stringId;
    for( int i=0; i<int(::getNumCodes()); i++ )
    {
        stringId = comboBox->AddString( ::getCodeHint( i ) );
        comboBox->SetItemData( stringId, ::getCode( i ) );
    }

    // determine current mapping code
    int code = 0;;
    configElement->Attribute( "code", &code );
    
    // show mapping
    int optionIsSelected = false;
    for( i=0; i<comboBox->GetCount(); i++ )
    {
        if( code == comboBox->GetItemData( i ) )
        {
            comboBox->SetCurSel( i );
            optionIsSelected = true;
            break;
        }
    }

    // select default option
    if( !optionIsSelected )
    {
        comboBox->SetCurSel( 0 );
        configElement->SetAttribute( "code", getCode( 0 ) );        
    }

    return comboBox;
}

void ClauncherDlg::Register(void)
{
    // check registration file is present
    FILE* licFile = fopen( "./usr/licence.txt", "r" );
    if( !licFile )
    {
        // start registration dialog
        CRegDialog regDialog( this );    
        regDialog.DoModal();
    }
    else
    {
        fclose( licFile );
    }
}

BOOL ClauncherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    // registar game copy (web-edition only)
    Register();

    // load config
    m_config = new TiXmlDocument( "./cfg/config.xml" );
    bool configIsLoaded = m_config->LoadFile();
    assert( configIsLoaded );

	// initialize directx
    m_iDirect3D9  = Direct3DCreate9( D3D_SDK_VERSION ); assert( m_iDirect3D9 );

    // enumerate adapters
    m_numAdapters = m_iDirect3D9->GetAdapterCount();
    for( int i=0; i<m_numAdapters; i++ )
    {
        // retrieve adapter ID
        D3DADAPTER_IDENTIFIER9 adapterIdentifier;
        m_iDirect3D9->GetAdapterIdentifier( i, 0, &adapterIdentifier );
        // fill checkbox item
        m_Adapter.AddString( adapterIdentifier.Description );
    }

    // configuration element 
    TiXmlElement* video = getConfigElement( "video" ); assert( video );
    int adapterId;
    video->Attribute( "adapter", &adapterId );

    // show adapter & mode
    m_Adapter.SetCurSel( adapterId );
    OnCbnSelchangeAdapter();

    // show fullscreen
    int windowed;
    video->Attribute( "windowed", &windowed );
    if( windowed == 0 )
    {
        m_Fullscreen.SetCheck( 1 );
    }
    else
    {
        m_Fullscreen.SetCheck( 0 );
    }

    // show sync
    int sync;
    video->Attribute( "sync", &sync );
    if( sync == 1 )
    {
        m_Sync.SetCheck( 1 );
    }
    else
    {
        m_Sync.SetCheck( 0 );
    }

    // show afterfx
    int afterfx;
    video->Attribute( "afterfx", &afterfx );
    if( afterfx == 1 )
    {
        m_AfterFx.SetCheck( 1 );
    }
    else
    {
        m_AfterFx.SetCheck( 0 );
    }

    // fill skin shader listbox
    int stringId = m_SkinShader.AddString( "4x" );
    m_SkinShader.SetItemData( stringId, DWORD_PTR( 4 ) );
    stringId = m_SkinShader.AddString( "8x" );
    m_SkinShader.SetItemData( stringId, DWORD_PTR( 8 ) );
    stringId = m_SkinShader.AddString( "16x" );
    m_SkinShader.SetItemData( stringId, DWORD_PTR( 16 ) );
    stringId = m_SkinShader.AddString( "32x" );
    m_SkinShader.SetItemData( stringId, DWORD_PTR( 32 ) );
    stringId = m_SkinShader.AddString( "64x" );
    m_SkinShader.SetItemData( stringId, DWORD_PTR( 64 ) );

    TiXmlElement* skinnedMesh = this->getConfigElement( "skinnedMesh" );
    int bonePalette;
    skinnedMesh->Attribute( "bonePalette", &bonePalette );
    
    // choose skin shader option
    int optionIsSelected = false;
    for( i=0; i<m_SkinShader.GetCount(); i++ )
    {
        if( bonePalette == int( m_SkinShader.GetItemData( i ) ) )
        {
            m_SkinShader.SetCurSel( i );
            optionIsSelected = true;
        }
    }

    // select default option
    if( !optionIsSelected )
    {
        m_SkinShader.SetCurSel( 0 );
        skinnedMesh->SetAttribute( "bonePalette", 4 );
        skinnedMesh->SetAttribute( "lightPalette", 2 );
        m_config->SaveFile();
    }

    // show grass option
    TiXmlElement* details = this->getConfigElement( "details" ); assert( details );
    int grass;
    details->Attribute( "grass", &grass );
    if( grass == 0 )
    {
        m_Grass.SetCheck( 0 );
    }
    else
    {
        m_Grass.SetCheck( 1 );
    }

    // fill crow density control
    stringId = m_Crowd.AddString( "0%" );
    m_Crowd.SetItemData( stringId, 0 );
    stringId = m_Crowd.AddString( "25%" );
    m_Crowd.SetItemData( stringId, 25 );
    stringId = m_Crowd.AddString( "50%" );
    m_Crowd.SetItemData( stringId, 50 );
    stringId = m_Crowd.AddString( "75%" );
    m_Crowd.SetItemData( stringId, 75 );
    stringId = m_Crowd.AddString( "100%" );
    m_Crowd.SetItemData( stringId, 100 );

    // fill traffic density control
    stringId = m_Traffic.AddString( "0%" );
    m_Traffic.SetItemData( stringId, 0 );
    stringId = m_Traffic.AddString( "25%" );
    m_Traffic.SetItemData( stringId, 25 );
    stringId = m_Traffic.AddString( "50%" );
    m_Traffic.SetItemData( stringId, 50 );
    stringId = m_Traffic.AddString( "75%" );
    m_Traffic.SetItemData( stringId, 75 );
    stringId = m_Traffic.AddString( "100%" );
    m_Traffic.SetItemData( stringId, 100 );

    // show crowd density
    double value;
    details->Attribute( "crowd", &value );
    float crowd = float( value );
    optionIsSelected = false;
    for( i=0; i<m_Crowd.GetCount(); i++ )
    {
        float itemData = float( m_Crowd.GetItemData( i ) ) / 100.0f;
        if( crowd == itemData )
        {
            m_Crowd.SetCurSel( i );
            optionIsSelected = true;
            break;
        }
    }
    if( !optionIsSelected )
    {
        m_Crowd.SetCurSel( 0 );
        
        char str[512];
        sprintf( str, "%3.2f", float( m_Crowd.GetItemData( 0 ) ) / 100.0f );
        details->SetAttribute( "crowd", str );
        m_config->SaveFile();
    }

    // show traffic density
    details->Attribute( "traffic", &value );
    float traffic = float( value );
    optionIsSelected = false;
    for( i=0; i<m_Traffic.GetCount(); i++ )
    {
        float itemData = float( m_Traffic.GetItemData( i ) ) / 100.0f;
        if( traffic == itemData )
        {
            m_Traffic.SetCurSel( i );
            optionIsSelected = true;
            break;
        }
    }
    if( !optionIsSelected )
    {
        m_Traffic.SetCurSel( 0 );

        char str[512];
        sprintf( str, "%3.2f", float( m_Crowd.GetItemData( 0 ) ) / 100.0f );
        details->SetAttribute( "traffic", str );
        m_config->SaveFile();
    }

    // show music volume
    TiXmlElement* sound = this->getConfigElement( "sound" ); assert( sound );
    value = 0.0f;
    sound->Attribute( "musicVolume", &value );
    if( value < 0 ) value = 0;
    if( value > 1 ) value = 1;
    m_MusicVolume.SetRange( 0, 100, TRUE );
    m_MusicVolume.SetPos( int( value * 100 ) );

    // show action mapping
    forAllMappings( SetupMappingControl, NULL );
    m_config->SaveFile();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void ClauncherDlg::OnPaint() 
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
        // bitmap.LoadBitmap( IDB_BITMAP1 ); // eng
        // bitmap.LoadBitmap( IDB_BITMAP3 ); // ru
        // bitmap.LoadBitmap( IDB_BITMAP4 ); // de
        bitmap.LoadBitmap( IDB_BITMAP5 ); // pl
        CBitmap *pbmOriginal = dcMemory.SelectObject(&bitmap);

        // draw bitmap
        GetDC()->BitBlt( 0,0,640,480, &dcMemory, 0,0, SRCCOPY );

        // invalidate all controls
        m_Adapter.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        m_VideoMode.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        m_Fullscreen.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        m_Sync.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        m_AfterFx.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        m_SkinShader.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        m_Grass.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        m_Crowd.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        m_Traffic.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        m_MusicVolume.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_Left.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_Right.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_Up.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_Down.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_LeftSkew.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_RightSkew.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_JumpPull.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_RunTrack.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_AltimeterMode.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_IncreaseWA.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_DecreaseWA.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_WLO.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_Hook.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_FirstPerson.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_External.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_Flyby.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_SwitchHUD.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_IncreaseTime.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_DecreaseTime.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_IncreaseMusic.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        mctrl_DecreaseMusic.RedrawWindow( NULL, NULL, RDW_INVALIDATE );
        m_StartButton.RedrawWindow( NULL, NULL, RDW_INVALIDATE );

        // draw all
        CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR ClauncherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void ClauncherDlg::OnCbnSelchangeAdapter()
{
    int adapterId = m_Adapter.GetCurSel();

    // reset list box
    m_VideoMode.ResetContent();

    // enumerate videomodes
    char str[512];
    D3DDISPLAYMODE displayMode;
    int numModes = m_iDirect3D9->GetAdapterModeCount( adapterId, D3DFMT_X8R8G8B8 );
    for( int i=0; i<numModes; i++ )
    {
        m_iDirect3D9->EnumAdapterModes( adapterId, D3DFMT_X8R8G8B8, i, &displayMode );
        if( displayMode.Width >= 640 && displayMode.Height >= 480 )
        {
            sprintf( str, "%dx%dx%d", displayMode.Width, displayMode.Height, displayMode.RefreshRate );
            int stringId = m_VideoMode.AddString( str );
            m_VideoMode.SetItemData( stringId, DWORD_PTR( i ) );
        }
    }

    // configuration element 
    TiXmlElement* video = getConfigElement( "video" ); assert( video );
    int width, height, rate;
    video->Attribute( "width", &width );
    video->Attribute( "height", &height );
    video->Attribute( "rate", &rate );

    // try to select corresponding videomode in list
    bool modeIsSelected = false;
    for( i=0; i<m_VideoMode.GetCount(); i++ )
    {
        m_iDirect3D9->EnumAdapterModes( 
            adapterId, 
            D3DFMT_X8R8G8B8, 
            int( m_VideoMode.GetItemData( i ) ), 
            &displayMode 
        );
        if( displayMode.Width == width && displayMode.Height == height && displayMode.RefreshRate == rate )
        {
            m_VideoMode.SetCurSel( i );
            modeIsSelected = true;
        }
    }

    // if can't select mode - choose first
    if( !modeIsSelected )
    {
        m_iDirect3D9->EnumAdapterModes( 
            adapterId, 
            D3DFMT_X8R8G8B8, 
            int( m_VideoMode.GetItemData( 0 ) ), 
            &displayMode 
        );
        video->SetAttribute( "width", displayMode.Width );
        video->SetAttribute( "height", displayMode.Height );
        video->SetAttribute( "rate", displayMode.RefreshRate );
        m_VideoMode.SetCurSel( 0 );
    }

    // save config
    m_config->SaveFile();
}

void ClauncherDlg::OnDestroy()
{
    m_iDirect3D9->Release();
    delete m_config;    

    CDialog::OnDestroy();
}

TiXmlElement* ClauncherDlg::getConfigElement(const char* name)
{
    TiXmlNode* child = m_config->FirstChild(); assert( child );
    if( child != NULL ) do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), name ) == 0 )
        {
            return static_cast<TiXmlElement*>( child );
        }
        child = child->NextSibling();
    }
    while( child != NULL );
    return NULL;
}

TiXmlElement* ClauncherDlg::getActionElement(const char* actionName)
{
    TiXmlNode* child = m_config->FirstChild(); assert( child );
    if( child != NULL ) do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "action" ) == 0 )
        {
            const char* name = static_cast<TiXmlElement*>( child )->Attribute( "name" );
            if( strcmp( name, actionName ) == 0 )
            {
                return static_cast<TiXmlElement*>( child );
            }
        }
        child = child->NextSibling();
    }
    while( child != NULL );
    return NULL;
}

void ClauncherDlg::OnCbnSelchangeMode()
{
    int adapterId = m_Adapter.GetCurSel();
    int modeId = int( m_VideoMode.GetItemData( m_VideoMode.GetCurSel() ) );    
    D3DDISPLAYMODE displayMode;

    m_iDirect3D9->EnumAdapterModes( adapterId, D3DFMT_X8R8G8B8, modeId, &displayMode );

    TiXmlElement* video = getConfigElement( "video" ); assert( video );
    video->SetAttribute( "width", displayMode.Width );
    video->SetAttribute( "height", displayMode.Height );
    video->SetAttribute( "rate", displayMode.RefreshRate );

    // save config
    m_config->SaveFile();
}

void ClauncherDlg::OnBnClickedFullscreen()
{
    // retrieve configuration element
    TiXmlElement* video = getConfigElement( "video" ); assert( video );    

    // change settings
    int checkStatus = m_Fullscreen.GetCheck();
    if( checkStatus )
    {
        video->SetAttribute( "windowed", 0 );
    }
    else
    {
        video->SetAttribute( "windowed", 1 );
    }

    // save config
    m_config->SaveFile();
}

void ClauncherDlg::OnBnClickedVsync()
{
    // retrieve configuration element
    TiXmlElement* video = getConfigElement( "video" ); assert( video );    

    // change settings
    int checkStatus = m_Sync.GetCheck();
    if( checkStatus )
    {
        video->SetAttribute( "sync", 1 );
    }
    else
    {
        video->SetAttribute( "sync", 0 );
    }

    // save config
    m_config->SaveFile();
}

void ClauncherDlg::OnBnClickedAfterfx()
{
    // retrieve configuration element
    TiXmlElement* video = getConfigElement( "video" ); assert( video );    

    // change settings
    int checkStatus = m_AfterFx.GetCheck();
    if( checkStatus )
    {
        video->SetAttribute( "afterfx", 1 );
    }
    else
    {
        video->SetAttribute( "afterfx", 0 );
    }

    // save config
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeSkinnedmesh()
{
    // retrieve configuration element
    TiXmlElement* skinnedMesh = getConfigElement( "skinnedMesh" ); assert( skinnedMesh );    

    int option = int( m_SkinShader.GetItemData( m_SkinShader.GetCurSel() ) );
    skinnedMesh->SetAttribute( "bonePalette", option );

    // save config
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeCrowd()
{
    // retrieve configuration element
    TiXmlElement* details = getConfigElement( "details" ); assert( details );    

    float option = float( m_Crowd.GetItemData( m_Crowd.GetCurSel() ) ) / 100.0f;
    char str[512];
    sprintf( str, "%3.2f", option );
    details->SetAttribute( "crowd", str );

    // save config
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeTraffic()
{
    // retrieve configuration element
    TiXmlElement* details = getConfigElement( "details" ); assert( details );    

    float option = float( m_Traffic.GetItemData( m_Traffic.GetCurSel() ) ) / 100.0f;
    char str[512];
    sprintf( str, "%3.2f", option );
    details->SetAttribute( "traffic", str );

    // save config
    m_config->SaveFile();
}

void ClauncherDlg::OnBnClickedGrass()
{
    // retrieve configuration element
    TiXmlElement* details = getConfigElement( "details" ); assert( details );

    // change settings
    int checkStatus = m_Grass.GetCheck();
    if( checkStatus )
    {
        details->SetAttribute( "grass", 1 );
    }
    else
    {
        details->SetAttribute( "grass", 0 );
    }

    // save config
    m_config->SaveFile();
}

void ClauncherDlg::OnBnClickedStartGame()
{
    // hide launcher
    ShowWindow( SW_HIDE );

    // play intro (release editions only)
    /*
    CVideoPlayer* videoPlayer = new CVideoPlayer( this );
    videoPlayer->DoModal();
    delete videoPlayer;
    */

    // exit launcher
    DestroyWindow();

    // start game
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    CreateProcess(
        NULL,
        "./sys/game-d.exe",
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi 
    );
}

void ClauncherDlg::OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
    // retrieve configuration element
    TiXmlElement* sound = getConfigElement( "sound" ); assert( sound );

    float volume = float( m_MusicVolume.GetPos() ) / 100.0f;
    if( volume < 0 ) volume = 0;
    if( volume > 1 ) volume = 1;

    char str[512];
    sprintf( str, "%3.2f", volume );
    sound->SetAttribute( "musicVolume", str );

    // save config
    m_config->SaveFile();

    *pResult = 0;
}


void ClauncherDlg::forAllMappings(MappingCallback callback, void* userData)
{   
    callback( &mctrl_Left, getActionElement( "iaLeft" ), userData );
    callback( &mctrl_Right, getActionElement( "iaRight" ), userData );
    callback( &mctrl_Up, getActionElement( "iaForward" ), userData );
    callback( &mctrl_Down, getActionElement( "iaBackward" ), userData );
    callback( &mctrl_LeftSkew, getActionElement( "iaLeftWarp" ), userData );
    callback( &mctrl_RightSkew, getActionElement( "iaRightWarp" ), userData );
    callback( &mctrl_JumpPull, getActionElement( "iaPhase" ), userData );
    callback( &mctrl_RunTrack, getActionElement( "iaModifier" ), userData );
    callback( &mctrl_AltimeterMode, getActionElement( "iaAltimeterMode" ), userData );
    callback( &mctrl_IncreaseWA, getActionElement( "iaIncreaseWarningAltitude" ), userData );
    callback( &mctrl_DecreaseWA, getActionElement( "iaDecreaseWarningAltitude" ), userData );
    callback( &mctrl_WLO, getActionElement( "iaWLO" ), userData );
    callback( &mctrl_Hook, getActionElement( "iaHook" ), userData );
    callback( &mctrl_FirstPerson, getActionElement( "iaCameraMode0" ), userData );
    callback( &mctrl_External, getActionElement( "iaCameraMode1" ), userData );
    callback( &mctrl_Flyby, getActionElement( "iaCameraMode2" ), userData );
    callback( &mctrl_SwitchHUD, getActionElement( "iaSwitchHUDMode" ), userData );
    callback( &mctrl_IncreaseTime, getActionElement( "iaAccelerateFlightTime" ), userData );
    callback( &mctrl_DecreaseTime, getActionElement( "iaDecelerateFlightTime" ), userData );
    callback( &mctrl_IncreaseMusic, getActionElement( "iaIncreaseMusicVolume" ), userData );
    callback( &mctrl_DecreaseMusic, getActionElement( "iaDecreaseMusicVolume" ), userData );
}

static CComboBox* checkCodeException = NULL;

CComboBox* ClauncherDlg::CheckCodeMapping(CComboBox* comboBox, TiXmlElement* configElement, void* userData)
{
    if( checkCodeException != comboBox )
    {
        unsigned int* code = reinterpret_cast<unsigned int*>( userData );
        if( *code != 0 )
        {
            if( *code == (unsigned int)comboBox->GetItemData( comboBox->GetCurSel() ) )
            {
                *code = 0;
            }
        }
    }
    return comboBox;
}

CComboBox* ClauncherDlg::ReadMappingControls(CComboBox* comboBox, TiXmlElement* configElement, void* userData)
{
    assert( comboBox );
    assert( configElement );

    ClauncherDlg* __this = reinterpret_cast<ClauncherDlg*>( userData ); assert( __this );

    // get combo selection & selection code
    int stringId = comboBox->GetCurSel();
    unsigned int code = (unsigned int)comboBox->GetItemData( stringId );
    assert( getCodeIndex( code ) != INVALID_CODE );

    // check code is not occupied
    unsigned int codeCheck = code;
    checkCodeException = comboBox;
    __this->forAllMappings( CheckCodeMapping, &codeCheck );
    if( codeCheck == 0 )
    {
        // beep
        MessageBeep( MB_ICONASTERISK );
        // back to current mapping
        int currentCode = 0;
        configElement->Attribute( "code", &currentCode );
    
        for( int i=0; i<comboBox->GetCount(); i++ )
        {
            if( currentCode == comboBox->GetItemData( i ) )
            {
                comboBox->SetCurSel( i );
                break;
            }
        }
    }
    else
    {
        // save current mapping
        configElement->SetAttribute( "code", code );
    }

    return comboBox;
}

void ClauncherDlg::OnCbnSelchangeLeft()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeUp()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeDown()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeLeftSkew()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeRight()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeRightSkew2()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeJumpPull()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeRunTrack()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeAltimeterMode()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeIncreaseWarningAltitude()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeDecreaseWarningAltitude()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeWloToggles()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeHookKnife()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeFirstPersonView()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeExternalView()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeFlybyView()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeSwitchHudMode()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeIncreaseTimeflow()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeDecreaseTimeflow()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeIncreaseMusicVolume()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}

void ClauncherDlg::OnCbnSelchangeDecreaseMusicVolume()
{
    forAllMappings( ReadMappingControls, this );
    m_config->SaveFile();
}
