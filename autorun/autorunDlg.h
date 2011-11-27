// autorunDlg.h : header file
//

#pragma once

#include <list>

/**
 * Cautorun button
 */

class CautorunButton
{
private:
    UINT _iId;    // resource for inactive state 
    UINT _aId;    // resource for active state    
    UINT _left;   // left corner
    UINT _top;    // top corner
    UINT _right;  // width of button
    UINT _bottom; // height of button
    CBitmap* _iBitmap;
    CBitmap* _aBitmap;
public:
    // abstraction layer
    virtual void onClick(void) {}
public:
    // class implementation
    CautorunButton(UINT iId, UINT aId, UINT left, UINT top, UINT right, UINT bottom);
    ~CautorunButton();
    // common methods
    void render(CDC* windc, CDC* dcmemory, UINT cx, UINT cy);
    bool isInside(UINT cx, UINT cy);
};

typedef std::list<CautorunButton*> CautorunButtons;
typedef CautorunButtons::iterator CautorunButtonI;

/**
 * buttons
 */

class CCloseButton : public CautorunButton
{
public:    
    CCloseButton();
    virtual void onClick(void);
};

class CInstallCodecButton : public CautorunButton
{
public:    
    CInstallCodecButton();
    virtual void onClick(void);
};

class CInstallNetFrameworkButton : public CautorunButton
{
public:    
    CInstallNetFrameworkButton();
    virtual void onClick(void);
};

class CInstallGameButton : public CautorunButton
{
public:    
    CInstallGameButton();
    virtual void onClick(void);
};

class CInstallDirectXButton : public CautorunButton
{
public:    
    CInstallDirectXButton();
    virtual void onClick(void);
};

class CInstallAgeiaButton : public CautorunButton
{
public:    
    CInstallAgeiaButton();
    virtual void onClick(void);
};

class CBonusButton : public CautorunButton
{
public:    
    CBonusButton();
    virtual void onClick(void);
};

class CReadmeButton : public CautorunButton
{
public:    
    CReadmeButton();
    virtual void onClick(void);
};

/**
 * CautorunDlg dialog
 */

class CautorunDlg : public CDialog
{
// Construction
public:
	CautorunDlg(CWnd* pParent = NULL);	// standard constructor
    virtual ~CautorunDlg();

// Dialog Data
	enum { IDD = IDD_AUTORUN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON            m_hIcon;
    CautorunButtons  m_Buttons;
    CPoint           m_MousePos;
    CautorunButton*  m_ButtonUnderCursor;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnClose();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
