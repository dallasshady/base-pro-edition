#pragma once


// CVideoPlayer dialog

class CVideoPlayer : public CDialog
{
	DECLARE_DYNAMIC(CVideoPlayer)

private:
    unsigned int _pscc; // state change counter

public:
	CVideoPlayer(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoPlayer();

// Dialog Data
	enum { IDD = IDD_VIDEOPLAYER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    DECLARE_EVENTSINK_MAP()
    void PlayStateChangeOcx1(long NewState);
};
