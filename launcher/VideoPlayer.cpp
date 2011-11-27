// VideoPlayer.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "VideoPlayer.h"
#include ".\videoplayer.h"


// CVideoPlayer dialog

IMPLEMENT_DYNAMIC(CVideoPlayer, CDialog)
CVideoPlayer::CVideoPlayer(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoPlayer::IDD, pParent)
{
    _pscc = 0;
}

CVideoPlayer::~CVideoPlayer()
{
}

void CVideoPlayer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVideoPlayer, CDialog)
END_MESSAGE_MAP()


// CVideoPlayer message handlers
BEGIN_EVENTSINK_MAP(CVideoPlayer, CDialog)
    ON_EVENT(CVideoPlayer, IDC_OCX1, 5101, PlayStateChangeOcx1, VTS_I4)
END_EVENTSINK_MAP()


void CVideoPlayer::PlayStateChangeOcx1(long NewState)
{
    // increase state change counter
    _pscc++;

    // close videoplayer window is stream was played
    if( _pscc > 1 ) PostMessage( WM_CLOSE, 0, 0 );
}
