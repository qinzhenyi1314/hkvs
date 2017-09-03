/**********************************************************
FileName:    DlgPlayRemoteTime.cpp
Description: remote play by time      
Date:        2008/05/17
Note: 		 <global>struct, refer to GeneralDef.h, global variants and functions refer to ClientDemo.cpp  
Modification History:      
    <version> <time>         <desc>
    <1.0    > <2008/05/17>       <created>
***********************************************************/


#include "stdafx.h"
#include "ClientDemo.h"
#include "DlgPlayRemoteTime.h"
#include ".\dlgplayremotetime.h"
#include "DlgPlayConvert.h"
CDlgPlayRemoteTime* g_pDlgRemoteTime;
HWND g_hPlayBackWnd = NULL;

void CALLBACK cbFunction(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
    BOOL bRet = FALSE;
	TRACE("datasize = %d", nSize);
	return;
	switch (pFrameInfo->nType)
	{
	case NET_DVR_SYSHEAD:
		bRet = PlayM4_SetOverlayMode(nPort, FALSE, COLORREF(0));//not play on overplay
		bRet = PlayM4_SetStreamOpenMode(nPort, STREAME_REALTIME);
		
		if(strcmp(g_sSecretKey, "StreamNotEncrypt") != 0)
		{
			if(!PlayM4_SetSecretKey(nPort, 1, g_sSecretKey, 128))
			{
				TRACE("ERR PlayM4_SetSecretKey%d!\n", nPort);
			}
		}
		if (nSize > 0)
		{
			bRet = PlayM4_OpenStream(nPort, (BYTE *)pBuf, nSize, 600*1000);//SOURCE_BUF_MIN50*1000
			if (bRet)
			{
				bRet = PlayM4_Play(nPort, g_hPlayBackWnd);
				if (!bRet)
				{
					TRACE("PlayM4_Play [%d]!\n", nPort);
				}
			}
			else
			{
				TRACE("ERR PlayM4_OpenStream%d!\n", nPort);
			}
		}
		break;
	case NET_DVR_AUDIOSTREAMDATA:
		if (nSize > 0)
		{
			bRet = PlayM4_InputVideoData(nPort, (BYTE *)pBuf, nSize);
			if (!bRet)
			{
				TRACE("ERR PlayM4_InputVideoData[%d]!\n", nPort);
			}
			else
			{
				TRACE("cbFunction:PlayM4_InputVideoData[%d]!\n", nPort);
			}
		}
		break;
	case NET_DVR_STREAMDATA:
		TRACE("cbFunction:PlayM4_InputVideoData[%d]!\n", nPort);
		if (nSize > 0)
		{
			bRet = PlayM4_InputData(nPort, (BYTE *)pBuf, nSize);
			if (!bRet)
			{
				TRACE("err! cbFunction:PlayM4_InputData  [%d]\n", nPort);
			}
			else
			{
				TRACE("cbFunction:PlayM4_InputVideoData[%d]!\n", nPort);
			}
		}
		break;
	default:
		TRACE("cbFunction:type[%d]!\n", pFrameInfo->nType);
		break;
	}
}

// CDlgPlayRemoteTime dialog
/*********************************************************
  Function:	CDlgPlayRemoteTime
  Desc:		Constructor
  Input:	
  Output:	
  Return:	
**********************************************************/
IMPLEMENT_DYNAMIC(CDlgPlayRemoteTime, CDialog)
CDlgPlayRemoteTime::CDlgPlayRemoteTime(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPlayRemoteTime::IDD, pParent)
	, m_ctDateStart(0)
	, m_ctTimeStart(0)
	, m_ctDateStop(0)
	, m_ctTimeStop(0)
	, m_iChanIndex(-1)
	, m_iDeviceIndex(-1)
	, m_bFullScreen(FALSE)
    , m_ctDateSet(0)
    , m_ctTimeSet(0)
    , m_bBackuping(FALSE)
    , m_lBackupHandle(-1)
    , m_hBackupThread(NULL)
	, m_iDownloadSpeed(0)
	, m_bDrawFrame(FALSE)
	, m_dwFileIndex(0)
	,m_dwVolumeNum(0)
	,m_bDownLoad(FALSE)
{
    memset(&m_struBackupTimeParam, 0, sizeof(m_struBackupTimeParam));
    memset(&m_struDiskList ,0, sizeof(m_struDiskList));

	m_hPlayEnableIcon =			AfxGetApp()->LoadIcon(IDI_PLAY_ENABLE);
	m_hPlayDisableIcon =		AfxGetApp()->LoadIcon(IDI_PLAY_DISABLE);
	m_hPauseEnableIcon =		AfxGetApp()->LoadIcon(IDI_PAUSE_ENABLE);
	m_hPauseDisableIcon =		AfxGetApp()->LoadIcon(IDI_PAUSE_DISABLE);
	m_hStopEnableIcon =			AfxGetApp()->LoadIcon(IDI_STOP_ENABLE);
	m_hStopDisableIcon =		AfxGetApp()->LoadIcon(IDI_STOP_DISABLE);
	m_hSoundStartIcon=			AfxGetApp()->LoadIcon(IDI_SOUND_ENABLE);
	m_hSoundStopIcon=			AfxGetApp()->LoadIcon(IDI_SOUND_DISABLE);
	m_hGotoHeadDisableIcon =	AfxGetApp()->LoadIcon(IDI_GOTOSTART_DISABLE);
	m_hGotoTailDisableIcon =	AfxGetApp()->LoadIcon(IDI_GOTOEND_DISABLE);
	m_hFastDisableIcon =		AfxGetApp()->LoadIcon(IDI_FASTFORWARD_DISABLE);
	m_hSlowDisableIcon=			AfxGetApp()->LoadIcon(IDI_FASTBACKWARD_DISABLE);
	m_hGotoHeadEnableIcon =		AfxGetApp()->LoadIcon(IDI_GOTOSTART_ENABLE);
	m_hGotoTailEnableIcon =		AfxGetApp()->LoadIcon(IDI_GOTOEND_ENABLE);
	m_hFastEnableIcon =			AfxGetApp()->LoadIcon(IDI_FASTFORWARD_ENABLE);
	m_hSlowEnableIcon =			AfxGetApp()->LoadIcon(IDI_FASTBACKWARD_ENABLE);
	m_hStepEnableIcon=			AfxGetApp()->LoadIcon(IDI_STEP_ENABLE);
	m_hStepDisableIcon =		AfxGetApp()->LoadIcon(IDI_STEP_DISABLE);
	m_hStepBackEnableIcon =		AfxGetApp()->LoadIcon(IDI_STEPBACK_ENABLE);
	m_hStepBackDisableIcon =	AfxGetApp()->LoadIcon(IDI_STEPBACK_DISABLE);
	m_hCaptureIcon	=			AfxGetApp()->LoadIcon(IDI_PIC);	
}

/*********************************************************
  Function:	~CDlgPlayRemoteTime
  Desc:		destructor
  Input:	
  Output:	
  Return:	
**********************************************************/
CDlgPlayRemoteTime::~CDlgPlayRemoteTime()
{
}

/*********************************************************
Function:	DoDataExchange
Desc:		the map between control and variable
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteTime::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgPlayRemoteTime)
	DDX_Control(pDX, IDC_COMBO_TRANS_TYPE2, m_comboTransType);
	DDX_Control(pDX, IDC_CMB_DECODEFRAME_TYPE, m_cmbDecodeFrameType);
	DDX_Control(pDX, IDC_COMBO_VOLUME_TYPE, m_comboVolumeType);
	DDX_Control(pDX, IDC_SLIDER_REMOTE_TIME_PROGRESS, m_SliderTimeProgress);
	DDX_Control(pDX, IDC_COMBO_DISK_LIST, m_comboDiskList);
	DDX_DateTimeCtrl(pDX, IDC_DATE_RT_START, m_ctDateStart);
	DDX_DateTimeCtrl(pDX, IDC_TIME_RT_START, m_ctTimeStart);
	DDX_DateTimeCtrl(pDX, IDC_DATE_RT_STOP, m_ctDateStop);
	DDX_DateTimeCtrl(pDX, IDC_TIME_RT_STOP, m_ctTimeStop);
	DDX_Control(pDX, IDC_STATIC_RT_WND, m_staticWnd);
	DDX_Control(pDX, IDC_SLIDER_RT_VOLUME, m_sliderVolume);
	DDX_Text(pDX, IDC_EDIT_FILE_SIZE, m_csTotalLen);
	DDX_DateTimeCtrl(pDX, IDC_COMBO_REMOTE_TIME_SET, m_ctTimeSet);
	DDX_Check(pDX, IDC_CHK_BACKUP_PLAYER, m_bBackupPlayer);
	DDX_Text(pDX, IDC_EDIT_DOWNLOAD_SPEED, m_iDownloadSpeed);
	DDX_Check(pDX, IDC_CHECK_DRAW_FRAME, m_bDrawFrame);
	DDX_Text(pDX, IDC_EDIT_FILEINDEX, m_dwFileIndex);
	DDX_Check(pDX, IDC_CHECK_DOWNLOAD, m_bDownLoad);
	DDX_Text(pDX, IDC_EDIT_VOLUME_NUM, m_dwVolumeNum);
	DDX_DateTimeCtrl(pDX, IDC_COMBO_REMOTE_DATE_SET, m_ctDateSet);
	DDX_Check(pDX, IDC_CHECK_1, m_bCheck1);
	DDX_Check(pDX, IDC_CHECK10, m_bCheck10);
	DDX_Check(pDX, IDC_CHECK11, m_bCheck11);
	DDX_Check(pDX, IDC_CHECK12, m_bCheck12);
	DDX_Check(pDX, IDC_CHECK13, m_bCheck13);
	DDX_Check(pDX, IDC_CHECK14, m_bCheck14);
	DDX_Check(pDX, IDC_CHECK15, m_bCheck15);
	DDX_Check(pDX, IDC_CHECK16, m_bCheck16);
	DDX_Check(pDX, IDC_CHECK17, m_bCheck17);
	DDX_Check(pDX, IDC_CHECK18, m_bCheck18);
	DDX_Check(pDX, IDC_CHECK19, m_bCheck19);
	DDX_Check(pDX, IDC_CHECK2, m_bCheck2);
	DDX_Check(pDX, IDC_CHECK20, m_bCheck20);
	DDX_Check(pDX, IDC_CHECK21, m_bCheck21);
	DDX_Check(pDX, IDC_CHECK22, m_bCheck22);
	DDX_Check(pDX, IDC_CHECK23, m_bCheck23);
	DDX_Check(pDX, IDC_CHECK24, m_bCheck24);
	DDX_Check(pDX, IDC_CHECK25, m_bCheck25);
	DDX_Check(pDX, IDC_CHECK26, m_bCheck26);
	DDX_Check(pDX, IDC_CHECK27, m_bCheck27);
	DDX_Check(pDX, IDC_CHECK28, m_bCheck28);
	DDX_Check(pDX, IDC_CHECK29, m_bCheck29);
	DDX_Check(pDX, IDC_CHECK3, m_bCheck3);
	DDX_Check(pDX, IDC_CHECK30, m_bCheck30);
	DDX_Check(pDX, IDC_CHECK31, m_bCheck31);
	DDX_Check(pDX, IDC_CHECK4, m_bCheck4);
	DDX_Check(pDX, IDC_CHECK5, m_bCheck5);
	DDX_Check(pDX, IDC_CHECK6, m_bCheck6);
	DDX_Check(pDX, IDC_CHECK7, m_bCheck7);
	DDX_Check(pDX, IDC_CHECK8, m_bCheck8);
	DDX_Check(pDX, IDC_CHECK9, m_bCheck9);
	DDX_Check(pDX, IDC_CHECKE1, m_bECheck1);
	DDX_Check(pDX, IDC_CHECKE10, m_bECheck10);
	DDX_Check(pDX, IDC_CHECKE11, m_bECheck11);
	DDX_Check(pDX, IDC_CHECKE12, m_bECheck12);
	DDX_Check(pDX, IDC_CHECKE13, m_bECheck13);
	DDX_Check(pDX, IDC_CHECKE14, m_bECheck14);
	DDX_Check(pDX, IDC_CHECKE16, m_bECheck16);
	DDX_Check(pDX, IDC_CHECKE18, m_bECheck18);
	DDX_Check(pDX, IDC_CHECKE19, m_bECheck19);
	DDX_Check(pDX, IDC_CHECKE2, m_bECheck2);
	DDX_Check(pDX, IDC_CHECKE20, m_bECheck20);
	DDX_Check(pDX, IDC_CHECKE21, m_bECheck21);
	DDX_Check(pDX, IDC_CHECKE22, m_bECheck22);
	DDX_Check(pDX, IDC_CHECKE23, m_bECheck23);
	DDX_Check(pDX, IDC_CHECKE24, m_bECheck24);
	DDX_Check(pDX, IDC_CHECKE25, m_bECheck25);
	DDX_Check(pDX, IDC_CHECKE26, m_bECheck26);
	DDX_Check(pDX, IDC_CHECKE27, m_bECheck27);
	DDX_Check(pDX, IDC_CHECKE28, m_bECheck28);
	DDX_Check(pDX, IDC_CHECKE29, m_bECheck29);
	DDX_Check(pDX, IDC_CHECKE3, m_bECheck3);
	DDX_Check(pDX, IDC_CHECKE30, m_bECheck30);
	DDX_Check(pDX, IDC_CHECKE31, m_bECheck31);
	DDX_Check(pDX, IDC_CHECKE4, m_bECheck4);
	DDX_Check(pDX, IDC_CHECKE5, m_bECheck5);
	DDX_Check(pDX, IDC_CHECKE6, m_bECheck6);
	DDX_Check(pDX, IDC_CHECKE7, m_bECheck7);
	DDX_Check(pDX, IDC_CHECKE8, m_bECheck8);
	DDX_Check(pDX, IDC_CHECKE9, m_bECheck9);
	DDX_Check(pDX, IDC_CHECKE15, m_bECheck15);
	DDX_Check(pDX, IDC_CHECKE17, m_bECheck17);
	//}}AFX_DATA_MAP

}


/*********************************************************
Function:	BEGIN_MESSAGE_MAP
Desc:		the map between control and function
Input:	
Output:	
Return:	
**********************************************************/
BEGIN_MESSAGE_MAP(CDlgPlayRemoteTime, CDialog)
	//{{AFX_MSG_MAP(CDlgPlayRemoteTime)	
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_TIME_PLAY, OnBnClickedBtnTimePlay)
	ON_BN_CLICKED(IDC_BTN_TIME_STOP, OnBnClickedBtnTimeStop)
	ON_BN_CLICKED(IDC_BTN_TIME_SLOW, OnBnClickedBtnTimeSlow)
	ON_BN_CLICKED(IDC_BTN_TIME_FAST, OnBnClickedBtnTimeFast)
	ON_BN_CLICKED(IDC_BTN_TIME_PIC_SNATCH, OnBnClickedBtnTimePicSnatch)
	ON_BN_CLICKED(IDC_BTN_TIME_SOUND, OnBnClickedBtnTimeSound)
	ON_BN_CLICKED(IDC_BTN_RT_DOWNLOAD, OnBnClickedBtnRtDownload)
	ON_BN_CLICKED(IDC_BTN_RT_PLAY_SAVE, OnBnClickedBtnRtPlaySave)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_SET_TIME, OnBtnSetTime)
	ON_BN_CLICKED(IDC_BTN_BACKUP, OnBtnBackup)
	ON_CBN_SELCHANGE(IDC_COMBO_DISK_LIST, OnSelchangeComboDiskList)
	ON_BN_CLICKED(IDC_BTN_DOWNLOAD_SPEED, OnBtnDownloadSpeed)
	ON_BN_CLICKED(IDC_RADIO_FORWARD_TIME, OnRadioForward)
	ON_BN_CLICKED(IDC_RADIO_REVERSE_TIME, OnRadioReverse)
	ON_CBN_SELCHANGE(IDC_COMBO_VOLUME_TYPE, OnSelchangeComboVolumeType)
	ON_BN_CLICKED(IDC_BUTTON_MDR_SEARCH, OnButtonMdrSearch)
	ON_BN_CLICKED(IDC_BTN_SET_DECODEFRAME_TYPE, OnBtnSetDecodeframeType)
	ON_CBN_SELCHANGE(IDC_CMB_DECODEFRAME_TYPE, OnSelchangeCmbDecodeframeType)
	ON_BN_CLICKED(IDC_PLAY_CONVERT, OnPlayConvert)
	//}}AFX_MSG_MAP
	
END_MESSAGE_MAP()


// CDlgPlayRemoteTime message handlers
/*********************************************************
Function:	SetParentWnd
Desc:		transmit handle of parent window 
Input:	    hhWnd,pointer to handle array
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteTime::SetParentWnd(HWND *hhWnd, int iParentNum)
{
	VERIFY(iParentNum==2);
	m_hPareDlgWnd=hhWnd[0];
	m_hPareTabWnd=hhWnd[1];
}

/*********************************************************
Function:	OnInitDialog
Desc:		Initialize the dialog
Input:	
Output:	
Return:	
**********************************************************/
BOOL CDlgPlayRemoteTime::OnInitDialog()
{
	CDialog::OnInitDialog();
    g_pDlgRemoteTime = this;

	CRect rc(0, 0, 0, 0);
	GetParent()->GetClientRect(&rc);
	((CTabCtrl*)GetParent())->AdjustRect(FALSE, &rc);
	MoveWindow(&rc);

	InitUI();
	m_bPause=FALSE;
	m_bDown = FALSE;
	m_bTimeSave = FALSE;
	m_PlayConvert = FALSE;
	m_sliderVolume.SetRange(1, 100, TRUE);
	m_sliderVolume.SetPos(50);
    m_SliderTimeProgress.SetRange(0,100);
    m_SliderTimeProgress.SetPos(0);
	m_lPlayHandle = -1;
	m_lLoadHandle = -1;
	m_bSound = FALSE;
	m_bFullScreen = FALSE;
	//m_hPareDlgWnd = NULL;
	//m_hPareTabWnd = NULL;
	m_lStartChan = 0;
	GetDlgItem(IDC_STATIC_RT_WND)->GetWindowRect(m_rcWnd);
	ScreenToClient(m_rcWnd);
	m_rcScreen=CRect(0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));

	m_comboVolumeType.SetCurSel(0);
	ShowVolumeNumInfo(FALSE);

	CTime timeCur = CTime::GetCurrentTime();
	CTime timeStart(timeCur.GetYear(),timeCur.GetMonth(),timeCur.GetDay(),0,0,0);
	CTime timeStop(timeCur.GetYear(),timeCur.GetMonth(),timeCur.GetDay(),23,59,59);
	m_ctDateStart = timeStart;
	m_ctTimeStart = timeStart;
	m_ctDateStop = timeStop;
	m_ctTimeStop = timeStop;
    m_ctDateSet = timeStart;
    m_ctTimeSet = timeStart;
	//�����ؼ���ʼ��
	m_bCheck1 = FALSE;
	m_bCheck2 = FALSE;
	m_bCheck3 = FALSE;
	m_bCheck4 = FALSE;
	m_bCheck5 = FALSE;
	m_bCheck6 = FALSE;
	m_bCheck7 = FALSE;
	m_bCheck8 = FALSE;
	m_bCheck9 = FALSE;
	m_bCheck10 = FALSE;
	m_bCheck11 = FALSE;
	m_bCheck12 = FALSE;
	m_bCheck13 = FALSE;
	m_bCheck14 = FALSE;
	m_bCheck15 = FALSE;
	m_bCheck16 = FALSE;
	m_bCheck17 = FALSE;
	m_bCheck18 = FALSE;
	m_bCheck19 = FALSE;
	m_bCheck20 = FALSE;
	m_bCheck21= FALSE;
	m_bCheck22 = FALSE;
	m_bCheck23 = FALSE;
	m_bCheck24 = FALSE;
	m_bCheck25 = FALSE;
	m_bCheck26 = FALSE;
	m_bCheck27 = FALSE;
	m_bCheck28 = FALSE;
	m_bCheck29 = FALSE;
	m_bCheck30 = FALSE;
	m_bCheck31 = FALSE;
	UpdateData(FALSE);
	return TRUE;
}

/*********************************************************
  Function:	CheckInitParam
  Desc:		check device parameters, and initialize dialog parameters
  Input:	
  Output:	
  Return:	TRUE, device parameter correct;FALSE,not initialized
**********************************************************/
BOOL CDlgPlayRemoteTime::CheckInitParam()
{
	((CButton *)GetDlgItem(IDC_RADIO_FORWARD_TIME))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_REVERSE_TIME))->SetCheck(FALSE);

	int iDeviceIndex = g_pMainDlg->GetCurDeviceIndex();
	if (iDeviceIndex == -1)
	{
		UpdateData(FALSE);
		return FALSE;
	}

	int iChanIndex = g_pMainDlg->GetCurChanIndex();
	if (iChanIndex == -1)
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "set the channel index = 0");
		iChanIndex = 0;
	}
	GetDlgItem(IDC_BTN_TIME_PLAY)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_DOWN)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_RT_PLAY_SAVE)->EnableWindow(TRUE);
	if ( m_iDeviceIndex == -1 || m_iDeviceIndex != iDeviceIndex)
	{
		m_iChanIndex = -1;//reset channel number if device switched
	}

	m_iDeviceIndex = iDeviceIndex;
	m_lLoginID = g_struDeviceInfo[m_iDeviceIndex].lLoginID;
	m_lStartChan = g_struDeviceInfo[m_iDeviceIndex].iStartChan;
	
	//reset parameters while channel is switched
	if (m_iChanIndex != iChanIndex)
	{
		m_iChanIndex = iChanIndex;
		m_iChanShowNum = g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].iChannelNO;
		if (m_bDown)
		{
		}
		if (m_lPlayHandle>=0)
		{
			StopPlay();
		}
	}

    GetDiskList();
    if (m_struDiskList.dwNodeNum > 0)
    {
        m_comboDiskList.SetCurSel(0);
        OnSelchangeComboDiskList();
    }

	return TRUE;
}

/*********************************************************
Function:	OnBnClickedBtnTimePlay
Desc:		start to play by time
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteTime::OnBnClickedBtnTimePlay()
{
	char szLan[128] = {0};
	UpdateData(TRUE);

	if (m_lPlayHandle == -1)
	{
		NET_DVR_VOD_PARA struVodPara = {0};

		m_iChanShowNum = g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].iChannelNO;//m_iChanIndex+m_lStartChan;
		struVodPara.struBeginTime.dwYear = m_ctDateStart.GetYear();
		struVodPara.struBeginTime.dwMonth = m_ctDateStart.GetMonth();
		struVodPara.struBeginTime.dwDay = m_ctDateStart.GetDay();
		struVodPara.struBeginTime.dwHour = m_ctTimeStart.GetHour();
		struVodPara.struBeginTime.dwMinute = m_ctTimeStart.GetMinute();
		struVodPara.struBeginTime.dwSecond = m_ctTimeStart.GetSecond();
		struVodPara.struEndTime.dwYear = m_ctDateStop.GetYear();
		struVodPara.struEndTime.dwMonth = m_ctDateStop.GetMonth();
		struVodPara.struEndTime.dwDay = m_ctDateStop.GetDay();
		struVodPara.struEndTime.dwHour = m_ctTimeStop.GetHour();
		struVodPara.struEndTime.dwMinute = m_ctTimeStop.GetMinute();
		struVodPara.struEndTime.dwSecond = m_ctTimeStop.GetSecond();
		struVodPara.struIDInfo.dwChannel = m_iChanShowNum;
		struVodPara.byVolumeType = m_comboVolumeType.GetCurSel();
		if (struVodPara.byVolumeType)
		{
			struVodPara.byVolumeNum = m_dwVolumeNum;
		}
		struVodPara.byDrawFrame = m_bDrawFrame;
		struVodPara.dwFileIndex = m_dwFileIndex;
		struVodPara.hWnd = m_staticWnd.GetSafeHwnd();
 

		NET_DVR_PLAYCOND struPlayCon = {0};
		struPlayCon.byDrawFrame = m_bDrawFrame;
		struPlayCon.dwChannel = m_iChanShowNum;
		struPlayCon.struStartTime.dwYear = m_ctDateStart.GetYear();
		struPlayCon.struStartTime.dwMonth = m_ctDateStart.GetMonth();
		struPlayCon.struStartTime.dwDay = m_ctDateStart.GetDay();
		struPlayCon.struStartTime.dwHour = m_ctTimeStart.GetHour();
		struPlayCon.struStartTime.dwMinute = m_ctTimeStart.GetMinute();
		struPlayCon.struStartTime.dwSecond = m_ctTimeStart.GetSecond();
		struPlayCon.struStopTime.dwYear = m_ctDateStop.GetYear();
		struPlayCon.struStopTime.dwMonth = m_ctDateStop.GetMonth();
		struPlayCon.struStopTime.dwDay = m_ctDateStop.GetDay();
		struPlayCon.struStopTime.dwHour = m_ctTimeStop.GetHour();
		struPlayCon.struStopTime.dwMinute = m_ctTimeStop.GetMinute();
		struPlayCon.struStopTime.dwSecond = m_ctTimeStop.GetSecond();
	
	

		g_hPlayBackWnd = m_staticWnd.GetSafeHwnd();

		if (((CButton *)GetDlgItem(IDC_RADIO_FORWARD_TIME))->GetCheck())
		{	
			m_lPlayHandle = NET_DVR_PlayBackByTime_V40(m_lLoginID, &struVodPara);
		}
		else
		{
			m_lPlayHandle = NET_DVR_PlayBackReverseByTime_V40(m_lLoginID, m_staticWnd.GetSafeHwnd(), &struPlayCon);		
		}
		
		if (m_lPlayHandle == -1)
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PlayBackByTime_V40 ChanNum[%d]", m_iChanShowNum);
			g_StringLanType(szLan, "Զ�̻ط�ʧ��", "Fail to play back remote");
			AfxMessageBox(szLan);
			return;
		}

 		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PlayBackByTime_V40 ChanNum[%d]", m_iChanShowNum);

		//����ת������
		int iTransType = m_comboTransType.GetCurSel() + 1;
		if (NET_DVR_PlayBackControl_V40(m_lPlayHandle, NET_DVR_SET_TRANS_TYPE, &iTransType, NULL))
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_SET_TRANS_TYPE TransType[%d]", iTransType);
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_SET_TRANS_TYPE TransType[%d]", iTransType);
		}

		NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSTART, 0, NULL);

		if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSTARTAUDIO, 0, NULL))
		{
			m_bSound=TRUE;
			NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYAUDIOVOLUME, (0xffff)/2, NULL);
			m_sliderVolume.SetPos(50);
			((CButton *)GetDlgItem(IDC_BTN_TIME_SOUND))->SetIcon(m_hSoundStartIcon);
		}
		else
		{
			m_bSound=FALSE;
			((CButton *)GetDlgItem(IDC_BTN_TIME_SOUND))->SetIcon(m_hSoundStopIcon);
		}
		SetPlayState();
		SetTimer(PLAYBYTIME_TIMER, 500, NULL);

        //2010-01-16
        unsigned __int64 nTotalLen;
        if (NET_DVR_PlayBackControl_V40(m_lPlayHandle, NET_DVR_PLAYGETTOTALLEN, NULL, 0, &nTotalLen, NULL))
        {
            char ch[32] = {0};
            sprintf(ch, "0x%x%x", (UINT)(nTotalLen>>32), UINT(nTotalLen));
            m_csTotalLen = ch;
            UpdateData(FALSE);
        }
        else
        {
  			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYGETTOTALLEN");   
        }
	}
	else
	{
		if (m_bPause)
		{
			if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYRESTART, 0, NULL))
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYRESTART");
				m_bPause=FALSE;
				SetPlayState();
			}
		}
		else
		{
			if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYPAUSE, 0, NULL))
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYPAUSE");
				m_bPause=TRUE;
				CButton *pButton;	
				pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_PLAY);
				pButton->SetIcon(m_hPlayEnableIcon);
				pButton->EnableWindow(TRUE);
			}
		}
	}	
    

}

/*********************************************************
Function:	OnBnClickedBtnTimeStop
Desc:		stop play by time
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteTime::OnBnClickedBtnTimeStop()
{
	StopPlay();
}

/*********************************************************
Function:	StopPlay
Desc:		stop play
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteTime::StopPlay()
{
	CButton *pButton;
	char szLan[128] = {0};
	if (m_lPlayHandle >= 0)
	{
		if (m_bFullScreen)
		{
			m_bFullScreen = FALSE;
			ShowNormal();
		}
		if (m_bTimeSave)
		{
			NET_DVR_StopPlayBackSave(m_lPlayHandle);
			m_bTimeSave = FALSE;
			g_StringLanType(szLan, "����", "Save");
			GetDlgItem(IDC_BTN_RT_PLAY_SAVE)->SetWindowText(szLan);
		}
		int idx = NET_DVR_GetPlayBackPlayerIndex(m_lPlayHandle);

		PlayM4_SetDecCallBack(idx, NULL);
		NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSTOPAUDIO, 0, NULL);
		NET_DVR_StopPlayBack(m_lPlayHandle);
		m_lPlayHandle = -1;

		Invalidate(TRUE);
		UpdateData(FALSE);
		KillTimer(PLAYBYTIME_TIMER);
	}
	m_bSound=FALSE;
	m_bPause = FALSE;
	m_sliderVolume.EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_PLAY);
	pButton->SetIcon(m_hPlayEnableIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_STOP);
	pButton->SetIcon(m_hStopDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_FAST);
	pButton->SetIcon(m_hFastDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_SLOW);
	pButton->SetIcon(m_hSlowDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_STEP);
	pButton->SetIcon(m_hStepDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_GOTO_BEGINNING);
	pButton->SetIcon(m_hGotoHeadDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_PIC_SNATCH);
	pButton->SetIcon(m_hCaptureIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_SOUND);
	pButton->SetIcon(m_hSoundStopIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_STEP);
	pButton->SetIcon(m_hStepDisableIcon);
	pButton->EnableWindow(FALSE);

    m_SliderTimeProgress.SetPos(0);
}

/*********************************************************
  Function:	OnBnClickedBtnTimeSlow
  Desc:		slow motion
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteTime::OnBnClickedBtnTimeSlow()
{
	if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSLOW, 0, NULL))
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYSLOW");
	}
	else
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYSLOW");
	}
}

/*********************************************************
Function:	OnBnClickedBtnTimeFast
Desc:		fast play
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteTime::OnBnClickedBtnTimeFast()
{
	if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYFAST, 0, NULL))
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYFAST");
	}
	else
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYFAST");
	}
}

/*********************************************************
  Function:	OnBnClickedBtnTimePicSnatch
  Desc:		capture
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteTime::OnBnClickedBtnTimePicSnatch()
{
	char cFilename[256];

	sprintf(cFilename, "%s\\%d_%d.bmp",g_struLocalParam.chPictureSavePath,m_iChanShowNum, GetTickCount());
	if (NET_DVR_PlayBackCaptureFile(m_lPlayHandle,cFilename))
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "%s NET_DVR_PlayBackCaptureFile",cFilename);
		return;
	}
	else
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PlayBackCaptureFile");
	}
}

/*********************************************************
  Function:	OnBnClickedBtnTimeSound
  Desc:		enable audio
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteTime::OnBnClickedBtnTimeSound()
{
	CButton *pButton;
	if (m_lPlayHandle >= 0)
	{
		if (m_bSound)
		{
			if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSTOPAUDIO, 0, NULL))
			{
				m_bSound=FALSE;
				pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_SOUND);
				pButton->SetIcon(m_hSoundStopIcon);
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYSTOPAUDIO %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName); 
			}
			else
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYSTOPAUDIO %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName); 
			}
		}
		else
		{
			if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSTARTAUDIO, 0, NULL))
			{
				pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_SOUND);
				pButton->SetIcon(m_hSoundStartIcon);
				m_bSound=TRUE;
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYSTARTAUDIO %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanShowNum-1].chChanName); 
			}
			else
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYSTARTAUDIO %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanShowNum-1].chChanName); 
			}
		}	
	}
}

/*********************************************************
  Function:	OnHScroll
  Desc:		draging scroll to play
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteTime::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	if (GetDlgItem(IDC_VOLUME_SLIDER) == pScrollBar)
	{
		char volume;
		WORD temp, volumevalue;
		temp = (0xffff)/100;
		volume = (char)m_sliderVolume.GetPos();
		volumevalue = (WORD)(volume*temp);
		if (m_lPlayHandle >= 0)
		{
			if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYAUDIOVOLUME, volumevalue, NULL))
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYAUDIOVOLUME %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName);
			}
			else
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYAUDIOVOLUME %s", g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].chChanName);
			}
		}
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

/*********************************************************
  Function:	OnLButtonDblClk
  Desc:		double click screen to enlarge image
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteTime::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if ((IsInWnd(point)) && (m_lPlayHandle >= 0))
	{
		m_bFullScreen=!m_bFullScreen;
		g_pMainDlg->FullScreen(m_bFullScreen);
		if (m_bFullScreen)
		{
			ShowFullScreen();
		}
		else
		{
			ShowNormal();
		}
		//NET_DVR_RefreshPlay(m_lPlayHandle);
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}

/*********************************************************
  Function:	IsInWnd
  Desc:		check mouse double click playback screen
  Input:	
  Output:	
  Return:	
**********************************************************/
BOOL CDlgPlayRemoteTime::IsInWnd(CPoint point)
{
	CRect rc;
	GetDlgItem(IDC_STATIC_RT_WND)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	if (PtInRect(&rc,point))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*********************************************************
  Function:	ShowFullScreen
  Desc:		playback by time in full screen
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteTime::ShowFullScreen()
{
	EnableFlashWnd(FALSE);

	CRect rc,rcClient;
	::GetWindowRect(m_hPareDlgWnd,m_rcPareDlg);

	::GetWindowRect(m_hPareTabWnd,m_rcPareTab);
	::ScreenToClient(m_hPareDlgWnd,(POINT*)(&m_rcPareTab));
	::ScreenToClient(m_hPareDlgWnd,(POINT*)(&m_rcPareTab)+1);

	::GetWindowRect(m_hWnd,m_rcParent);
	::ScreenToClient(m_hPareTabWnd,(POINT*)(&m_rcParent));
	::ScreenToClient(m_hPareTabWnd,(POINT*)(&m_rcParent)+1);


	::SetWindowPos(m_hPareDlgWnd,NULL,m_rcScreen.left-SCREEN_EDGE,m_rcScreen.top-SCREEN_EDGE,m_rcScreen.Width()+2*SCREEN_EDGE,m_rcScreen.Height()+2*SCREEN_EDGE,SWP_SHOWWINDOW);

	::GetClientRect(m_hPareDlgWnd,rc);
	::SetWindowPos(m_hPareTabWnd,NULL,rc.left,rc.top,rc.Width(),rc.Height(),SWP_SHOWWINDOW);

	::GetClientRect(m_hPareTabWnd,rc);
	::SetWindowPos(m_hWnd,NULL,rc.left,rc.top,rc.Width(),rc.Height(),SWP_SHOWWINDOW);

	GetDlgItem(IDC_STATIC_RT_WND)->GetClientRect(rcClient);
	GetDlgItem(IDC_STATIC_RT_WND)->GetWindowRect(rc);
	int iEdge=(rc.Width()-rcClient.Width())/2;
	rc=m_rcScreen;
	rc.left-=iEdge;
	rc.right+=iEdge;
	rc.top-=iEdge;
	rc.bottom+=iEdge;
	this->ScreenToClient(rc);
	::SetWindowPos(GetDlgItem(IDC_STATIC_RT_WND)->m_hWnd,NULL,rc.left,rc.top,rc.Width(),rc.Height(),SWP_SHOWWINDOW);
	Invalidate(FALSE);
	UpdateWindow();
}

/*********************************************************
  Function:	ShowNormal
  Desc:		return to original size while in full screen
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteTime::ShowNormal()
{
	GetDlgItem(IDC_STATIC_RT_WND)->SetWindowPos(NULL,m_rcWnd.left,m_rcWnd.top,m_rcWnd.Width(),m_rcWnd.Height(),SWP_SHOWWINDOW);
	::SetWindowPos(m_hWnd,NULL,m_rcParent.left,m_rcParent.top,m_rcParent.Width(),m_rcParent.Height(),SWP_SHOWWINDOW);
	::SetWindowPos(m_hPareTabWnd,NULL,m_rcPareTab.left,m_rcPareTab.top,m_rcPareTab.Width(),m_rcPareTab.Height(),SWP_SHOWWINDOW);
	::SetWindowPos(m_hPareDlgWnd,NULL,m_rcPareDlg.left,m_rcPareDlg.top,m_rcPareDlg.Width(),m_rcPareDlg.Height(),SWP_SHOWWINDOW);
	EnableFlashWnd(TRUE);
	Invalidate(FALSE);
	UpdateWindow();
}

/*********************************************************
  Function:	EnableFlashWnd
  Desc:		refresh conrol state
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteTime::EnableFlashWnd(BOOL bEnable)
{
	if (bEnable)
	{
		GetDlgItem(IDC_DATE_RT_START)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_DATE_RT_STOP)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_TIME_RT_STOP)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_TIME_RT_START)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_TIME_PLAY)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_TIME_STOP)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_TIME_SLOW)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_TIME_FAST)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_TIME_STEP)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_TIME_GOTO_BEGINNING)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_TIME_PIC_SNATCH)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_BTN_TIME_SOUND)->ModifyStyle(0,WS_VISIBLE,0);
		GetDlgItem(IDC_VOLUME_SLIDER)->ModifyStyle(0,WS_VISIBLE,0);
	}
	else
	{
		//THE same to multiplay
		GetDlgItem(IDC_DATE_RT_START)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_DATE_RT_STOP)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_TIME_RT_STOP)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_TIME_RT_START)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_TIME_PLAY)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_TIME_STOP)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_TIME_SLOW)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_TIME_FAST)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_TIME_STEP)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_TIME_GOTO_BEGINNING)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_TIME_PIC_SNATCH)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_BTN_TIME_SOUND)->ModifyStyle(WS_VISIBLE,0,0);
		GetDlgItem(IDC_VOLUME_SLIDER)->ModifyStyle(WS_VISIBLE,0,0);
	}
}

/*********************************************************
Function:	InitUI
Desc:		intialize UI
Input:	
Output:	
Return:	
**********************************************************/
void CDlgPlayRemoteTime::InitUI()
{
	CButton *pButton;

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_PLAY);
	pButton->SetIcon(m_hPlayEnableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_STOP);
	pButton->SetIcon(m_hStopDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_FAST);
	pButton->SetIcon(m_hFastDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_SLOW);
	pButton->SetIcon(m_hSlowDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_STEP);
	pButton->SetIcon(m_hStepDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_GOTO_BEGINNING);
	pButton->SetIcon(m_hGotoHeadDisableIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_PIC_SNATCH);
	pButton->SetIcon(m_hCaptureIcon);
	pButton->EnableWindow(FALSE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_SOUND);
	pButton->SetIcon(m_hSoundStopIcon);
	pButton->EnableWindow(FALSE);

	GetDlgItem(IDC_BTN_RT_PLAY_SAVE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_RT_DOWNLOAD)->EnableWindow(FALSE);
}

/*********************************************************
  Function:	SetPlayState
  Desc:	    set control state of playback
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteTime::SetPlayState()
{
	m_sliderVolume.EnableWindow(TRUE);

	CButton *pButton;

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_PLAY);
	pButton->SetIcon(m_hPauseEnableIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_STOP);
	pButton->SetIcon(m_hStopEnableIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_FAST);
	pButton->SetIcon(m_hFastEnableIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_SLOW);
	pButton->SetIcon(m_hSlowEnableIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_PIC_SNATCH);
	pButton->SetIcon(m_hCaptureIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_SOUND);
	if (m_bSound)
		pButton->SetIcon(m_hSoundStartIcon);
	else
		pButton->SetIcon(m_hSoundStopIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_STEP);
	pButton->SetIcon(m_hStepEnableIcon);
	pButton->EnableWindow(TRUE);

	pButton = (CButton *)GetDlgItem(IDC_BTN_TIME_GOTO_BEGINNING);
	pButton->SetIcon(m_hGotoHeadEnableIcon);
	pButton->EnableWindow(TRUE);
}

/*********************************************************
  Function:	OnTimer
  Desc:		start timer
  Input:	
  Output:	
  Return:	
**********************************************************/
#if (_MSC_VER >= 1500)	//vs2008
void CDlgPlayRemoteTime::OnTimer(UINT_PTR nIDEvent)
#else
void CDlgPlayRemoteTime::OnTimer(UINT nIDEvent) 
#endif
{
	// TODO: Add your message handler code here and/or call default
	DWORD nPos;
	char szLan[128] = {0};
//	NET_DVR_TIME struOsdTime;
	if (nIDEvent == PLAYBYTIME_TIMER)
	{
		if (m_lPlayHandle >= 0)
		{
			if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYGETPOS, 0, &nPos))
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYGETPOS pos[%d]", nPos);
                m_SliderTimeProgress.SetPos(nPos);
			}
			else
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYGETPOS");
			}
     			
			if (nPos > 100)
			{
				StopPlay();
				g_StringLanType(szLan, "��������ԭ���DVRæ,�ط��쳣��ֹ!", " Due to network reasons or DVR is busy, playback abnormal termination");
				AfxMessageBox(szLan);
			}
			if ((((CButton *)GetDlgItem(IDC_RADIO_FORWARD_TIME))->GetCheck())? (nPos == 100):(nPos == 0))
			{				
				StopPlay();
				g_StringLanType(szLan, "��ʱ��طŽ���", "playback by time over");
				AfxMessageBox(szLan);
			}	
		}
	}
	else if (nIDEvent == DOWNBYTIME_TIMER)
	{
		if (m_bDown)
		{
			NET_DVR_PlayBackControl(m_lLoadHandle, NET_DVR_PLAYGETPOS, 0, &nPos);
			if (nPos > 100)
			{
				m_bDown=FALSE;				
				NET_DVR_StopGetFile(m_lLoadHandle);
				g_StringLanType(szLan, "����", "Download");
				GetDlgItem(IDC_BTN_RT_DOWNLOAD)->SetWindowText(szLan);
				g_StringLanType(szLan, "��������ԭ���DVRæ,�����쳣��ֹ!", " Due to network reasons or DVR is busy, download abnormal termination");
				AfxMessageBox(szLan);
			}
			if (nPos == 100)
			{
				m_bDown=FALSE;				
				NET_DVR_StopGetFile(m_lLoadHandle);
				g_StringLanType(szLan, "����", "Download");
				GetDlgItem(IDC_BTN_RT_DOWNLOAD)->SetWindowText(szLan);
				g_StringLanType(szLan, "��ʱ�����ؽ���", "download by time over");
				AfxMessageBox(szLan);
			}		
            m_SliderTimeProgress.SetPos(nPos);
		}	
	}	
	CDialog::OnTimer(nIDEvent);
}

/*********************************************************
  Function:	OnBnClickedBtnRtDownload
  Desc:		download recorded files from device
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteTime::OnBnClickedBtnRtDownload()
{
	CString csFileName;
	CString csStartTime, csStopTime;
	CString csTemp,csDir;
	char szFileName[256] = {0};
	char szLan[128] = {0};

	UpdateData(TRUE);
	g_CreateLocalDir();
	if (!m_bDown)
	{
		csStartTime.Format("%04d%02d%02d%02d%02d%02d",m_ctDateStart.GetYear(),m_ctDateStart.GetMonth(), m_ctDateStart.GetDay(), m_ctTimeStart.GetHour(), m_ctTimeStart.GetMinute(), m_ctTimeStart.GetSecond());
		csStopTime.Format("%04d%02d%02d%02d%02d%02d",m_ctDateStop.GetYear(),m_ctDateStop.GetMonth(), m_ctDateStop.GetDay(), m_ctTimeStop.GetHour(), m_ctTimeStop.GetMinute(), m_ctTimeStop.GetSecond());

		csDir.Format("%s\\",g_struLocalParam.chDownLoadPath);
		sprintf(szFileName,"%s%s_%02d_D%s_%s.mp4",csDir,g_struDeviceInfo[m_iDeviceIndex].chDeviceIPInFileName, m_iChanShowNum,csStartTime,csStopTime);
		
		NET_DVR_PLAYCOND struDownloadCond = {0};
		UpdateData(TRUE);

		struDownloadCond.struStartTime.dwYear = m_ctDateStart.GetYear();
		struDownloadCond.struStartTime.dwMonth = m_ctDateStart.GetMonth();
		struDownloadCond.struStartTime.dwDay = m_ctDateStart.GetDay();
		struDownloadCond.struStartTime.dwHour = m_ctTimeStart.GetHour();
		struDownloadCond.struStartTime.dwMinute = m_ctTimeStart.GetMinute();
		struDownloadCond.struStartTime.dwSecond = m_ctTimeStart.GetSecond();
		struDownloadCond.struStopTime.dwYear = m_ctDateStop.GetYear();
		struDownloadCond.struStopTime.dwMonth = m_ctDateStop.GetMonth();
		struDownloadCond.struStopTime.dwDay = m_ctDateStop.GetDay();
		struDownloadCond.struStopTime.dwHour = m_ctTimeStop.GetHour();
		struDownloadCond.struStopTime.dwMinute = m_ctTimeStop.GetMinute();
		struDownloadCond.struStopTime.dwSecond = m_ctTimeStop.GetSecond();
  
		struDownloadCond.dwChannel = m_iChanShowNum;
		struDownloadCond.byDrawFrame = m_bDrawFrame;

		m_lLoadHandle = NET_DVR_GetFileByTime_V40(m_lLoginID, szFileName, &struDownloadCond);
		if (m_lLoadHandle >= 0)
		{
			//����ת������
			int iTransType = m_comboTransType.GetCurSel() + 1;
			if (NET_DVR_PlayBackControl_V40(m_lLoadHandle, NET_DVR_SET_TRANS_TYPE, &iTransType, NULL))
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_SET_TRANS_TYPE TransType[%d]", iTransType);
			}
			else
			{
				g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_SET_TRANS_TYPE TransType[%d]", iTransType);
			}

			NET_DVR_PlayBackControl(m_lLoadHandle, NET_DVR_PLAYSTART, 0, NULL);
			g_StringLanType(szLan, "ֹͣ����", "Stop Download");
			GetDlgItem(IDC_BTN_RT_DOWNLOAD)->SetWindowText(szLan);
			m_bDown=TRUE;
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GetFileByTime_V40 file[%s]", szFileName);
			SetTimer(DOWNBYTIME_TIMER,1000,NULL);
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GetFileByTime_V40 file[%s]", szFileName);
			g_StringLanType(szLan, "��ʱ������ʧ��", "download by time failed");
			AfxMessageBox(szLan);
			return;
		}
	}
	else 
	{
		NET_DVR_StopGetFile(m_lLoadHandle);
		m_bDown=FALSE;
		g_StringLanType(szLan, "����", "Download");
		GetDlgItem(IDC_BTN_RT_DOWNLOAD)->SetWindowText(szLan);
		KillTimer(DOWNBYTIME_TIMER);
		Invalidate(TRUE);
	}
	UpdateData(FALSE);
}

/*********************************************************
  Function:	OnBnClickedBtnRtPlaySave
  Desc:		save playing video file to local
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgPlayRemoteTime::OnBnClickedBtnRtPlaySave()
{
	char szLan[128] = {0};
	if (!m_bTimeSave)
	{
		static char szFilter[]="All File(*.*)|*.*||";
		char m_sFile[256] = "c:\\SavefileByTime.mp4";
		CString csFile;
		CFileDialog dlg(FALSE,"*.mp4","SavefileByTime.mp4",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
			szFilter);
		if (dlg.DoModal()==IDOK)
		{
			csFile = dlg.GetPathName();
			UpdateData(FALSE);
			sprintf(m_sFile, "%s", csFile);

			if (NET_DVR_PlayBackSaveData(m_lPlayHandle, m_sFile))
			{
				m_bTimeSave = TRUE;
				g_StringLanType(szLan, "ֹͣ����", "Stop saving");
				GetDlgItem(IDC_BTN_RT_PLAY_SAVE)->SetWindowText(szLan);
			}
		}
	}
	else
	{
		NET_DVR_StopPlayBackSave(m_lPlayHandle);
		m_bTimeSave = FALSE;
		g_StringLanType(szLan, "����", "Save");
		GetDlgItem(IDC_BTN_RT_PLAY_SAVE)->SetWindowText(szLan);
	}
}

HBRUSH CDlgPlayRemoteTime::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if (m_lPlayHandle < 0 || !m_bPause)
	{
		return hbr;
	}
	if (!NET_DVR_RefreshPlay(m_lPlayHandle))
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_RefreshPlay");
	}
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CDlgPlayRemoteTime::OnBtnSetTime() 
{
	// TODO: Add your control notification handler code here
    UpdateData(TRUE);
    NET_DVR_TIME struTime;
    struTime.dwYear = (DWORD)m_ctDateSet.GetYear();
    struTime.dwMonth = (DWORD)m_ctDateSet.GetMonth();
    struTime.dwDay = (DWORD)m_ctDateSet.GetDay();
    struTime.dwHour = (DWORD)m_ctTimeSet.GetHour();
    struTime.dwMinute = (DWORD)m_ctTimeSet.GetMinute();
    struTime.dwSecond = (DWORD)m_ctTimeSet.GetSecond();
    
    
    if (NET_DVR_PlayBackControl_V40(m_lPlayHandle, NET_DVR_PLAYSETTIME, &struTime, sizeof(struTime), NULL, 0))
    {
  		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAYSETTIME");      
    }
    else
    {
  		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYSETTIME");    
    }
    
}

DWORD  GetBackupTimeThread(LPVOID pParam)
{
    CDlgPlayRemoteTime *pDlgRemotPlayTime = (CDlgPlayRemoteTime*)pParam;
    pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_SHOW);
    DWORD dwState = 0;
    char szLan[256] = {0};
    while (1)
    {
        NET_DVR_GetBackupProgress(pDlgRemotPlayTime->m_lBackupHandle, &dwState);
        
        if (dwState == 100)
        {
            g_StringLanType(szLan, "�������", "Succ to backup");
            pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
            NET_DVR_StopBackup(pDlgRemotPlayTime->m_lBackupHandle);
            g_StringLanType(szLan, "�ļ�����", "Backup");
            pDlgRemotPlayTime->GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
            pDlgRemotPlayTime->m_bBackuping = FALSE;
			Sleep(1000);
            pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_HIDE);
            break;
        }
		else if(dwState ==  BACKUP_CHANGE_DEVICE)
		{
			g_StringLanType(szLan, "�����豸����, ������豸��������", "Device of backup is full, change another device and continue backuping");
			pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
			NET_DVR_StopBackup(pDlgRemotPlayTime->m_lBackupHandle);
			g_StringLanType(szLan, "����", "Backup");
			pDlgRemotPlayTime->GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
			pDlgRemotPlayTime->m_bBackuping = FALSE;
			Sleep(1000);
			pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_HIDE);
			break;
		}
        else if (dwState == 400)
        {
            g_StringLanType(szLan, "�����쳣", "backup exception");
            pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
            NET_DVR_StopBackup(pDlgRemotPlayTime->m_lBackupHandle);
            g_StringLanType(szLan, "�ļ�����", "Backup");
            pDlgRemotPlayTime->GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
            pDlgRemotPlayTime->m_bBackuping = FALSE;
			Sleep(1000);
            pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_HIDE);
            break;
        }
        else if (dwState == 500)
        {
            g_StringLanType(szLan, "����ʧ��", "Succ to backup");
            pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
            NET_DVR_StopBackup(pDlgRemotPlayTime->m_lBackupHandle);
            g_StringLanType(szLan, "�ļ�����", "Backup");
            pDlgRemotPlayTime->GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
            pDlgRemotPlayTime->m_bBackuping = FALSE;
			Sleep(1000);
            pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_HIDE);
            break;
        }
        else if (dwState >= 0 && dwState < 100)
        {
            char szLanCn[128] = {0};
            char szLanEn[128] = {0};
            sprintf(szLanCn, "���ڱ���[%d]", dwState);
            sprintf(szLanEn, "backuping[%d]", dwState);
            g_StringLanType(szLan, szLanCn, szLanCn);
            pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
        }
		//�м����
		else if(dwState == BACKUP_SEARCH_DEVICE)
		{
            g_StringLanType(szLan, "�������������豸", "searching backup device");
            pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
		}
		else if(dwState == BACKUP_SEARCH_FILE)
		{
            g_StringLanType(szLan, "��������¼���ļ�", "searching record files");
            pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
		}
		//����ֵ
		else if(dwState >= BACKUP_TIME_SEG_NO_FILE)
		{
            char szLanCn[128] = {0};
            char szLanEn[128] = {0};
            sprintf(szLanCn, "����ʧ��, ����ֵ[%d]", dwState);
            sprintf(szLanEn, "Backup failed, ErrorCode[%d]", dwState);
			g_StringLanType(szLan, szLanCn, szLanEn);
            pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->SetWindowText(szLan);
			Sleep(2000);
            NET_DVR_StopBackup(pDlgRemotPlayTime->m_lBackupHandle);
            g_StringLanType(szLan, "�ļ�����", "Backup");
            pDlgRemotPlayTime->GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
            pDlgRemotPlayTime->m_bBackuping = FALSE;
            pDlgRemotPlayTime->GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_HIDE);
            break;
		}
        Sleep(1000);
    }
    CloseHandle(g_pDlgRemoteTime->m_hBackupThread);
    g_pDlgRemoteTime->m_hBackupThread = NULL;
    
    return 0;
}

void CDlgPlayRemoteTime::OnBtnBackup() 
{
    UpdateData(TRUE);
    char szLan[256] = {0};
    if (!m_bBackuping)
    {
        CString csDiskDesc;
        if (m_comboDiskList.GetCurSel() != CB_ERR)
        {
            m_comboDiskList.GetLBText(m_comboDiskList.GetCurSel(), csDiskDesc);
        }
        else
        {
            g_StringLanType(szLan, "��ѡ����̱����б�", "Please select backup disk");
            AfxMessageBox(szLan);
        }
        strncpy((char*)m_struBackupTimeParam.byDiskDes, (char*)csDiskDesc.GetBuffer(0), sizeof(m_struBackupTimeParam.byDiskDes));
        
        m_struBackupTimeParam.byWithPlayer = m_bBackupPlayer;
        m_struBackupTimeParam.lChannel =  m_iChanShowNum;
        m_struBackupTimeParam.struStartTime.dwYear = m_ctDateStart.GetYear();
        m_struBackupTimeParam.struStartTime.dwMonth = m_ctDateStart.GetMonth();
        m_struBackupTimeParam.struStartTime.dwDay = m_ctDateStart.GetDay();
        m_struBackupTimeParam.struStartTime.dwHour = m_ctTimeStart.GetHour();
        m_struBackupTimeParam.struStartTime.dwMinute = m_ctTimeStart.GetMinute();
        m_struBackupTimeParam.struStartTime.dwSecond = m_ctTimeStart.GetSecond();
        m_struBackupTimeParam.struStopTime.dwYear = m_ctDateStop.GetYear();
        m_struBackupTimeParam.struStopTime.dwMonth = m_ctDateStop.GetMonth();
        m_struBackupTimeParam.struStopTime.dwDay = m_ctDateStop.GetDay();
        m_struBackupTimeParam.struStopTime.dwHour = m_ctTimeStop.GetHour();
        m_struBackupTimeParam.struStopTime.dwMinute = m_ctTimeStop.GetMinute();
		m_struBackupTimeParam.struStopTime.dwSecond = m_ctTimeStop.GetSecond();
        
		m_struBackupTimeParam.byDrawFrame = m_bDrawFrame;

        m_lBackupHandle = NET_DVR_BackupByTime(m_lLoginID, &m_struBackupTimeParam);
        if (m_lBackupHandle == -1)
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_BackupByTime");
            return;
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_BackupByTime");
        }
        
        DWORD dwThreadId = 0;
        if (m_hBackupThread == NULL)
        {
            m_hBackupThread = CreateThread(NULL,0,LPTHREAD_START_ROUTINE(GetBackupTimeThread),this,0,&dwThreadId);		
        }
        if (m_hBackupThread  == NULL)
        {
            char szLan[256] = {0};
            g_StringLanType(szLan, "�򿪱����߳�ʧ��!", "Fail to open backup thread!");
            AfxMessageBox(szLan);
            return;
        }
        g_StringLanType(szLan, "ֹͣ����", "Stop Bakcup");
        GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
        m_bBackuping = TRUE;
        GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_SHOW);
    }
    else
    {
        if (m_hBackupThread)
        {
            TerminateThread(m_hBackupThread, 0);
        }
        
        CloseHandle(m_hBackupThread);
        m_hBackupThread = NULL;
        NET_DVR_StopBackup(m_lBackupHandle);
        g_StringLanType(szLan, "�ļ�����", "Backup");
        GetDlgItem(IDC_BTN_BACKUP)->SetWindowText(szLan);
        m_bBackuping = FALSE;
        GetDlgItem(IDC_STATIC_BACK_STATE)->ShowWindow(SW_HIDE);
    }
	
}

void CDlgPlayRemoteTime::GetDiskList()
{
    if (!NET_DVR_GetDiskList(m_lLoginID, &m_struDiskList))
    {
		int i = NET_DVR_GetLastError();
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GetDiskList DISK_LIST");
    }
    else
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "   DISK_LIST");
    }
    
    m_comboDiskList.ResetContent();
    for (int i = 0; i < m_struDiskList.dwNodeNum; i++)
    {
        m_comboDiskList.AddString((char*)m_struDiskList.struDescNode[i].byDescribe);
        m_comboDiskList.SetItemData(i, m_struDiskList.struDescNode[i].iValue);
    }
}

void CDlgPlayRemoteTime::OnSelchangeComboDiskList() 
{
    DWORD dwDiskFreeSpace = m_struDiskList.struDescNode[m_comboDiskList.GetCurSel()].dwFreeSpace;
    char szLanCn[256] = {0};
    char szLanEn[256] = {0};
    char szLan[256] = {0};
    sprintf(szLanCn, "ʣ����̿ռ�%dM", dwDiskFreeSpace);
    sprintf(szLanEn, "Free Disk Spcace%dM", dwDiskFreeSpace);
    g_StringLanType(szLan, szLanCn, szLanEn);
    GetDlgItem(IDC_STATIC_BACKUP_DISK_SIZE)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_STATIC_BACKUP_DISK_SIZE)->SetWindowText(szLan);
}

void CDlgPlayRemoteTime::OnBtnDownloadSpeed() 
{
    UpdateData(TRUE);
    if (NET_DVR_PlayBackControl_V40(m_lLoadHandle, NET_DVR_SETSPEED, &m_iDownloadSpeed, sizeof(m_iDownloadSpeed), NULL, NULL))
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_SETSPEED");
    }
    else
    {
        g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_SETSPEED");
    }
}

void CDlgPlayRemoteTime::OnRadioForward() 
{
	UpdateData(TRUE);

	
	if(m_lPlayHandle == -1)
	{
		return;
	}
	else
	{
		if(NET_DVR_PlayBackControl_V40(m_lPlayHandle, NET_DVR_PLAY_FORWARD, NULL, 0, NULL, NULL))
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAY_FORWARD");
			((CButton *)GetDlgItem(IDC_RADIO_FORWARD_TIME))->SetCheck(TRUE);
			((CButton *)GetDlgItem(IDC_RADIO_REVERSE_TIME))->SetCheck(FALSE);
			UpdateData(FALSE);
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAY_FORWARD");
			((CButton *)GetDlgItem(IDC_RADIO_FORWARD_TIME))->SetCheck(FALSE);
			((CButton *)GetDlgItem(IDC_RADIO_REVERSE_TIME))->SetCheck(TRUE);
			UpdateData(FALSE);
		}	
	}


}

void CDlgPlayRemoteTime::OnRadioReverse() 
{
	UpdateData(TRUE);

	
	if(m_lPlayHandle == -1)
	{
		return;
	}
	else
	{
		if(NET_DVR_PlayBackControl_V40(m_lPlayHandle, NET_DVR_PLAY_REVERSE, NULL, 0, NULL, NULL))
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PLAY_REVERSE");
			((CButton *)GetDlgItem(IDC_RADIO_FORWARD_TIME))->SetCheck(FALSE);
			((CButton *)GetDlgItem(IDC_RADIO_REVERSE_TIME))->SetCheck(TRUE);
			UpdateData(FALSE);	
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAY_REVERSE");
			((CButton *)GetDlgItem(IDC_RADIO_FORWARD_TIME))->SetCheck(TRUE);
			((CButton *)GetDlgItem(IDC_RADIO_REVERSE_TIME))->SetCheck(FALSE);
			UpdateData(FALSE);
		}
	}
}

void CDlgPlayRemoteTime::ShowVolumeNumInfo(BOOL bShow)
{
	UpdateData(TRUE);
	GetDlgItem(IDC_EDIT_VOLUME_NUM)->ShowWindow(bShow);
	GetDlgItem(IDC_STATIC_VOLUME_NUM)->ShowWindow(bShow);
	UpdateData(FALSE);
}

void CDlgPlayRemoteTime::OnSelchangeComboVolumeType() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	ShowVolumeNumInfo(m_comboVolumeType.GetCurSel());	
}

void CDlgPlayRemoteTime::OnButtonMdrSearch() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	NET_DVR_MRD_SEARCH_PARAM struSearchParam = {0};
	NET_DVR_MRD_SEARCH_RESULT struSearchResult = {0};
	struSearchParam.dwSize = sizeof(NET_DVR_MRD_SEARCH_PARAM);
	struSearchParam.wYear = m_ctDateStart.GetYear();
	struSearchParam.byMonth = m_ctDateStart.GetMonth();
	struSearchParam.struStreamInfo.dwChannel = m_iChanShowNum;
	struSearchParam.byDrawFrame = m_bDrawFrame;
	if (!NET_DVR_GetDeviceConfig(m_lLoginID, NET_DVR_GET_MONTHLY_RECORD_DISTRIBUTION, 0, &struSearchParam, sizeof(struSearchParam), NULL, &struSearchResult, sizeof(struSearchResult)))
	{
		char szLan[256] = {0};
		g_StringLanType(szLan, "��������ʧ��!", "Fail to Monthly Search!");
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GET_MONTHLY_RECORD_DISTRIBUTION");
        AfxMessageBox(szLan);
		return;
	}
	else
	{
		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GET_MONTHLY_RECORD_DISTRIBUTION");
	}
	
	m_bCheck1 = struSearchResult.byRecordDistribution[0];
	m_bCheck2 = struSearchResult.byRecordDistribution[1];
	m_bCheck3 = struSearchResult.byRecordDistribution[2];
	m_bCheck4 = struSearchResult.byRecordDistribution[3];
	m_bCheck5 = struSearchResult.byRecordDistribution[4];
	m_bCheck6 = struSearchResult.byRecordDistribution[5];
	m_bCheck7 = struSearchResult.byRecordDistribution[6];
	m_bCheck8 = struSearchResult.byRecordDistribution[7];
	m_bCheck9 = struSearchResult.byRecordDistribution[8];
	m_bCheck10 = struSearchResult.byRecordDistribution[9];
	m_bCheck11 = struSearchResult.byRecordDistribution[10];
	m_bCheck12 = struSearchResult.byRecordDistribution[11];
	m_bCheck13 = struSearchResult.byRecordDistribution[12];
	m_bCheck14 = struSearchResult.byRecordDistribution[13];
	m_bCheck15 = struSearchResult.byRecordDistribution[14];
	m_bCheck16 = struSearchResult.byRecordDistribution[15];
	m_bCheck17 = struSearchResult.byRecordDistribution[16];
	m_bCheck18 = struSearchResult.byRecordDistribution[17];
	m_bCheck19 = struSearchResult.byRecordDistribution[18];
	m_bCheck20 = struSearchResult.byRecordDistribution[19];
	m_bCheck21 = struSearchResult.byRecordDistribution[20];
	m_bCheck22 = struSearchResult.byRecordDistribution[21];
	m_bCheck23 = struSearchResult.byRecordDistribution[22];
	m_bCheck24 = struSearchResult.byRecordDistribution[23];
	m_bCheck25 = struSearchResult.byRecordDistribution[24];
	m_bCheck26 = struSearchResult.byRecordDistribution[25];
	m_bCheck27 = struSearchResult.byRecordDistribution[26];
	m_bCheck28 = struSearchResult.byRecordDistribution[27];
	m_bCheck29 = struSearchResult.byRecordDistribution[28];
	m_bCheck30 = struSearchResult.byRecordDistribution[29];
	m_bCheck31 = struSearchResult.byRecordDistribution[30];
	
	m_bECheck1 = struSearchResult.byHasEventRecode[0];
	m_bECheck2 = struSearchResult.byHasEventRecode[1];
	m_bECheck3 = struSearchResult.byHasEventRecode[2];
	m_bECheck4 = struSearchResult.byHasEventRecode[3];
	m_bECheck5 = struSearchResult.byHasEventRecode[4];
	m_bECheck6 = struSearchResult.byHasEventRecode[5];
	m_bECheck7 = struSearchResult.byHasEventRecode[6];
	m_bECheck8 = struSearchResult.byHasEventRecode[7];
	m_bECheck9 = struSearchResult.byHasEventRecode[8];
	m_bECheck10 = struSearchResult.byHasEventRecode[9];
	m_bECheck11 = struSearchResult.byHasEventRecode[10];
	m_bECheck12 = struSearchResult.byHasEventRecode[11];
	m_bECheck13 = struSearchResult.byHasEventRecode[12];
	m_bECheck14 = struSearchResult.byHasEventRecode[13];
	m_bECheck15 = struSearchResult.byHasEventRecode[14];
	m_bECheck16 = struSearchResult.byHasEventRecode[15];
	m_bECheck17 = struSearchResult.byHasEventRecode[16];
	m_bECheck18 = struSearchResult.byHasEventRecode[17];
	m_bECheck19 = struSearchResult.byHasEventRecode[18];
	m_bECheck20 = struSearchResult.byHasEventRecode[19];
	m_bECheck21 = struSearchResult.byHasEventRecode[20];
	m_bECheck22 = struSearchResult.byHasEventRecode[21];
	m_bECheck23 = struSearchResult.byHasEventRecode[22];
	m_bECheck24 = struSearchResult.byHasEventRecode[23];
	m_bECheck25 = struSearchResult.byHasEventRecode[24];
	m_bECheck26 = struSearchResult.byHasEventRecode[25];
	m_bECheck27 = struSearchResult.byHasEventRecode[26];
	m_bECheck28 = struSearchResult.byHasEventRecode[27];
	m_bECheck29 = struSearchResult.byHasEventRecode[28];
	m_bECheck30 = struSearchResult.byHasEventRecode[29];
	m_bECheck31 = struSearchResult.byHasEventRecode[30];
	UpdateData(FALSE);
}


void CDlgPlayRemoteTime::OnBtnSetDecodeframeType() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	int iDecodeFrameType = m_cmbDecodeFrameType.GetCurSel();
	if (iDecodeFrameType == -1)
	{
		//AfxMessageBox(")
	}
	else
	{
		if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_SET_DECODEFFRAMETYPE, iDecodeFrameType, NULL))
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_SET_DECODEFFRAMETYPE");
		}
		else
		{
			g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_SET_DECODEFFRAMETYPE");
		}
	}
}

void CDlgPlayRemoteTime::OnSelchangeCmbDecodeframeType() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	if (m_cmbDecodeFrameType.GetCurSel() == -1)
	{
		GetDlgItem(IDC_BTN_SET_DECODEFRAME_TYPE)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_BTN_SET_DECODEFRAME_TYPE)->EnableWindow(TRUE);
	}
}


void CDlgPlayRemoteTime::OnPlayConvert() 
{
	// TODO: Add your control notification handler code here
	char szLan[128] = {0};
	UpdateData(TRUE);
	NET_DVR_VOD_PARA struVodPara = {0};
	
	m_iChanShowNum = g_struDeviceInfo[m_iDeviceIndex].pStruChanInfo[m_iChanIndex].iChannelNO;//m_iChanIndex+m_lStartChan;
	struVodPara.struBeginTime.dwYear = m_ctDateStart.GetYear();
	struVodPara.struBeginTime.dwMonth = m_ctDateStart.GetMonth();
	struVodPara.struBeginTime.dwDay = m_ctDateStart.GetDay();
	struVodPara.struBeginTime.dwHour = m_ctTimeStart.GetHour();
	struVodPara.struBeginTime.dwMinute = m_ctTimeStart.GetMinute();
	struVodPara.struBeginTime.dwSecond = m_ctTimeStart.GetSecond();
	struVodPara.struEndTime.dwYear = m_ctDateStop.GetYear();
	struVodPara.struEndTime.dwMonth = m_ctDateStop.GetMonth();
	struVodPara.struEndTime.dwDay = m_ctDateStop.GetDay();
	struVodPara.struEndTime.dwHour = m_ctTimeStop.GetHour();
	struVodPara.struEndTime.dwMinute = m_ctTimeStop.GetMinute();
	struVodPara.struEndTime.dwSecond = m_ctTimeStop.GetSecond();
	struVodPara.struIDInfo.dwChannel = m_iChanShowNum;
	struVodPara.byVolumeType = m_comboVolumeType.GetCurSel();
	if (struVodPara.byVolumeType)
	{
		struVodPara.byVolumeNum = m_dwVolumeNum;
	}
	struVodPara.byDrawFrame = m_bDrawFrame;
	struVodPara.dwFileIndex = m_dwFileIndex;
	struVodPara.hWnd = m_staticWnd.GetSafeHwnd();
	
	
	NET_DVR_PLAYCOND struPlayCon = {0};
	struPlayCon.byDrawFrame = m_bDrawFrame;
	struPlayCon.dwChannel = m_iChanShowNum;
	struPlayCon.struStartTime.dwYear = m_ctDateStart.GetYear();
	struPlayCon.struStartTime.dwMonth = m_ctDateStart.GetMonth();
	struPlayCon.struStartTime.dwDay = m_ctDateStart.GetDay();
	struPlayCon.struStartTime.dwHour = m_ctTimeStart.GetHour();
	struPlayCon.struStartTime.dwMinute = m_ctTimeStart.GetMinute();
	struPlayCon.struStartTime.dwSecond = m_ctTimeStart.GetSecond();
	struPlayCon.struStopTime.dwYear = m_ctDateStop.GetYear();
	struPlayCon.struStopTime.dwMonth = m_ctDateStop.GetMonth();
	struPlayCon.struStopTime.dwDay = m_ctDateStop.GetDay();
	struPlayCon.struStopTime.dwHour = m_ctTimeStop.GetHour();
	struPlayCon.struStopTime.dwMinute = m_ctTimeStop.GetMinute();
	struPlayCon.struStopTime.dwSecond = m_ctTimeStop.GetSecond();
	
	
	
	
 	g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_PlayBackByTime_V40 ChanNum[%d]", m_iChanShowNum);
	DlgPlayConvert dlg;
	dlg.Init(m_lPlayHandle);
    dlg.m_dwCurChanNo = m_iChanShowNum;
	dlg.DoModal();
	NET_DVR_COMPRESSION_INFO_V30 struCompressionInfoV30;
	memset(&struCompressionInfoV30,0,sizeof(struCompressionInfoV30));
	dlg.getComPressionInfoV30(&struCompressionInfoV30);

	if (m_lPlayHandle == -1)
	{
		g_hPlayBackWnd = m_staticWnd.GetSafeHwnd();
		int iResult = ((CButton *)GetDlgItem(IDC_RADIO_FORWARD_TIME))->GetCheck();
		if (iResult)
		{	
			m_lPlayHandle = NET_DVR_PlayBackByTime_V40(m_lLoginID, &struVodPara);
		}
		else
		{
			m_lPlayHandle = NET_DVR_PlayBackReverseByTime_V40(m_lLoginID, g_hPlayBackWnd, &struPlayCon);		
		}
		
		if (m_lPlayHandle == -1)
		{
			g_StringLanType(szLan, "Զ�̻ط�ʧ��", "Fail to play back remote");
			AfxMessageBox(szLan);
			return;
		}
        else
        {
            g_pMainDlg->AddLog(m_lPlayHandle, OPERATION_SUCC_T, "NET_DVR_PlayBackReverseByTime_V40");
        }
	}

	if(NET_DVR_PlayBackControl_V40(m_lPlayHandle,NET_DVR_PLAY_CONVERT,&struCompressionInfoV30,sizeof(struCompressionInfoV30),NULL,NULL))
	{
		g_pMainDlg->AddLog(m_lPlayHandle, OPERATION_SUCC_T, "NET_DVR_PLAY_CONVERT");
	}
	else
	{
		g_pMainDlg->AddLog(m_lPlayHandle, OPERATION_FAIL_T, "NET_DVR_PLAY_CONVERT");
		g_StringLanType(szLan, "ת��ʧ��", "Fail to play convert");
		AfxMessageBox(szLan);
        m_lPlayHandle = -1;
        return;
	}
	
	if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSTART, 0, NULL))
	{
        g_pMainDlg->AddLog(m_lPlayHandle, OPERATION_SUCC_T, "NET_DVR_PLAYSTART");
	}
    else
    {
        g_pMainDlg->AddLog(m_lPlayHandle, OPERATION_FAIL_T, "NET_DVR_PLAYSTART");
         return;
    }
	
	if (NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYSTARTAUDIO, 0, NULL))
	{
		m_bSound=TRUE;
		NET_DVR_PlayBackControl(m_lPlayHandle, NET_DVR_PLAYAUDIOVOLUME, (0xffff)/2, NULL);
		m_sliderVolume.SetPos(50);
		((CButton *)GetDlgItem(IDC_BTN_TIME_SOUND))->SetIcon(m_hSoundStartIcon);
	}
	else
	{
		m_bSound=FALSE;
		((CButton *)GetDlgItem(IDC_BTN_TIME_SOUND))->SetIcon(m_hSoundStopIcon);
	}
	SetPlayState();
	SetTimer(PLAYBYTIME_TIMER, 500, NULL);
	
	//2010-01-16
// 	unsigned __int64 nTotalLen;
// 	if (NET_DVR_PlayBackControl_V40(m_lPlayHandle, NET_DVR_PLAYGETTOTALLEN, NULL, 0, &nTotalLen, NULL))
// 	{
// 		char ch[32] = {0};
// 		sprintf(ch, "0x%x%x", (UINT)(nTotalLen>>32), UINT(nTotalLen));
// 		m_csTotalLen = ch;
// 		UpdateData(FALSE);
// 	}
// 	else
// 	{
// 		g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_PLAYGETTOTALLEN");   
// 	}
}