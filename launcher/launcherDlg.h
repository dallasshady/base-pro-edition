// launcherDlg.h : header file
//

#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "afxcmn.h"

/**
 * DirectInput errors
 */

static void diReportResult(const char* file, int line, HRESULT result)
{
    const char* description = NULL;

    switch( result )
    {
    case DIERR_BETADIRECTINPUTVERSION:
        description = "The application was written for an unsupported prerelease version of DirectInput.";
        break;
    case DIERR_INVALIDPARAM:
        description = "An invalid parameter was passed to the returning function, or the object was not in a state that permitted the function to be called. This value is equal to the E_INVALIDARG standard COM return value.";
        break;
    case DIERR_OLDDIRECTINPUTVERSION:
        description = "The application requires a newer version of DirectInput.";
        break;
    case DIERR_OUTOFMEMORY:
        description = "The DirectInput subsystem couldn't allocate sufficient memory to complete the call. This value is equal to the E_OUTOFMEMORY standard COM return value.";
        break;
    case E_POINTER:
        description = "Invalid pointer";
        break;
    default: 
        description = "Unknown.";
        break;
    }

    char message[512];
    sprintf( message, "DirectInput interface method was failed with result %x\n\nDescription: \"%s\"\nFile: \"%s\" line %d", result, description, file, line );

    AfxMessageBox( message, MB_OK | MB_ICONEXCLAMATION, 0 );
}

static inline void diCheckResult(const char* file, int line, HRESULT result)
{
    if( result != DI_OK ) diReportResult( file, line, result );
}

#define _diCR(HRESULT) diCheckResult( __FILE__, __LINE__, HRESULT )

/**
 * windows stuff
 */

typedef CComboBox* (*MappingCallback)(CComboBox* comboBox, TiXmlElement* configElement, void* userData);

// ClauncherDlg dialog
class ClauncherDlg : public CDialog
{
// Construction
public:
	ClauncherDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LAUNCHER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON                m_hIcon;
    TiXmlDocument*       m_config;
    IDirect3D9*          m_iDirect3D9;
    int                  m_numAdapters;

    // registration
    void Register(void);

    // manual methods
    TiXmlElement* getConfigElement(const char* name);
    TiXmlElement* getActionElement(const char* actionName);

    // callback engine
    void forAllMappings(MappingCallback callback, void* userData);

    // callbacks
    static CComboBox* SetupMappingControl(CComboBox* comboBox, TiXmlElement* configElement, void* userData);
    static CComboBox* CheckCodeMapping(CComboBox* comboBox, TiXmlElement* configElement, void* userData);
    static CComboBox* ReadMappingControls(CComboBox* comboBox, TiXmlElement* configElement, void* userData);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_Adapter;
    afx_msg void OnCbnSelchangeAdapter();
    CComboBox m_VideoMode;
    afx_msg void OnDestroy();
    afx_msg void OnCbnSelchangeMode();
    CButton m_Fullscreen;
    afx_msg void OnBnClickedFullscreen();
    CButton m_Sync;
    afx_msg void OnBnClickedVsync();
    CButton m_AfterFx;
    afx_msg void OnBnClickedAfterfx();
    CComboBox m_SkinShader;
    afx_msg void OnCbnSelchangeSkinnedmesh();
    CButton m_Grass;
    CComboBox m_Crowd;
    CComboBox m_Traffic;
    afx_msg void OnCbnSelchangeCrowd();
    afx_msg void OnCbnSelchangeTraffic();
    afx_msg void OnBnClickedGrass();
    afx_msg void OnBnClickedStartGame();
    CSliderCtrl m_MusicVolume;
    afx_msg void OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult);
    CComboBox mctrl_Left;
    CComboBox mctrl_Right;
    CComboBox mctrl_Up;
    CComboBox mctrl_Down;
    CComboBox mctrl_LeftSkew;
    CComboBox mctrl_RightSkew;
    CComboBox mctrl_JumpPull;
    CComboBox mctrl_RunTrack;
    CComboBox mctrl_AltimeterMode;
    CComboBox mctrl_IncreaseWA;
    CComboBox mctrl_DecreaseWA;
    CComboBox mctrl_WLO;
    CComboBox mctrl_Hook;
    CComboBox mctrl_FirstPerson;
    CComboBox mctrl_External;
    CComboBox mctrl_Flyby;
    CComboBox mctrl_SwitchHUD;
    CComboBox mctrl_IncreaseTime;
    CComboBox mctrl_DecreaseTime;
    CComboBox mctrl_IncreaseMusic;
    CComboBox mctrl_DecreaseMusic;
    afx_msg void OnCbnSelchangeLeft();
    afx_msg void OnCbnSelchangeUp();
    afx_msg void OnCbnSelchangeDown();
    afx_msg void OnCbnSelchangeLeftSkew();
    afx_msg void OnCbnSelchangeRight();
    afx_msg void OnCbnSelchangeRightSkew2();
    afx_msg void OnCbnSelchangeJumpPull();
    afx_msg void OnCbnSelchangeRunTrack();
    afx_msg void OnCbnSelchangeAltimeterMode();
    afx_msg void OnCbnSelchangeIncreaseWarningAltitude();
    afx_msg void OnCbnSelchangeDecreaseWarningAltitude();
    afx_msg void OnCbnSelchangeWloToggles();
    afx_msg void OnCbnSelchangeHookKnife();
    afx_msg void OnCbnSelchangeFirstPersonView();
    afx_msg void OnCbnSelchangeExternalView();
    afx_msg void OnCbnSelchangeFlybyView();
    afx_msg void OnCbnSelchangeSwitchHudMode();
    afx_msg void OnCbnSelchangeIncreaseTimeflow();
    afx_msg void OnCbnSelchangeDecreaseTimeflow();
    afx_msg void OnCbnSelchangeIncreaseMusicVolume();
    afx_msg void OnCbnSelchangeDecreaseMusicVolume();
    CButton m_StartButton;
};
