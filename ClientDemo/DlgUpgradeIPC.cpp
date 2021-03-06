// DlgUpgradeIPC.cpp : implementation file
//

#include "stdafx.h"
#include "clientdemo.h"
#include "DlgUpgradeIPC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgUpgradeIPC dialog

void __stdcall cbUpgradeIPCStatus(DWORD dwType, LPVOID lpBuffer, DWORD dwBufLen, LPVOID pUserData)
{
	CDlgUpgradeIPC *pThis = (CDlgUpgradeIPC*)pUserData;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	
	
	::SendMessage(hWnd, MSG_UPGRADE_IPC_STATUS, (WPARAM)dwType, (LPARAM)lpBuffer);
}

CDlgUpgradeIPC::CDlgUpgradeIPC(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgUpgradeIPC::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgUpgradeIPC)
	m_bCheckD1 = FALSE;
	m_strUpgradeFile = _T("");
	m_strOutput = _T("");
	//}}AFX_DATA_INIT

	m_lHandle = -1;
	m_dwSelectNum = 0;
}

/*********************************************************
  Function:	~CDlgUpgradeIPC
  Desc:		destructor
  Input:	
  Output:	
  Return:	
**********************************************************/
CDlgUpgradeIPC::~CDlgUpgradeIPC()
{
    if (m_pbUpdateChan != NULL)
    {
        delete[] m_pbUpdateChan;
        m_pbUpdateChan = NULL;
    }
}


void CDlgUpgradeIPC::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgUpgradeIPC)
	DDX_Check(pDX, IDC_CHECK_D1, m_bCheckD1);
	DDX_Check(pDX, IDC_CHECK_D2, m_bCheckD2);
	DDX_Check(pDX, IDC_CHECK_D3, m_bCheckD3);
	DDX_Check(pDX, IDC_CHECK_D4, m_bCheckD4);
	DDX_Check(pDX, IDC_CHECK_D5, m_bCheckD5);
	DDX_Check(pDX, IDC_CHECK_D6, m_bCheckD6);
	DDX_Check(pDX, IDC_CHECK_D7, m_bCheckD7);
	DDX_Check(pDX, IDC_CHECK_D8, m_bCheckD8);
	DDX_Check(pDX, IDC_CHECK_D9, m_bCheckD9);
	DDX_Check(pDX, IDC_CHECK_D10, m_bCheckD10);
	DDX_Check(pDX, IDC_CHECK_D11, m_bCheckD11);
	DDX_Check(pDX, IDC_CHECK_D12, m_bCheckD12);
	DDX_Check(pDX, IDC_CHECK_D13, m_bCheckD13);
	DDX_Check(pDX, IDC_CHECK_D14, m_bCheckD14);
	DDX_Check(pDX, IDC_CHECK_D15, m_bCheckD15);
	DDX_Check(pDX, IDC_CHECK_D16, m_bCheckD16);
	DDX_Check(pDX, IDC_CHECK_D17, m_bCheckD17);
	DDX_Check(pDX, IDC_CHECK_D18, m_bCheckD18);
	DDX_Check(pDX, IDC_CHECK_D19, m_bCheckD19);
	DDX_Check(pDX, IDC_CHECK_D20, m_bCheckD20);
	DDX_Check(pDX, IDC_CHECK_D21, m_bCheckD21);
	DDX_Check(pDX, IDC_CHECK_D22, m_bCheckD22);
	DDX_Check(pDX, IDC_CHECK_D23, m_bCheckD23);
	DDX_Check(pDX, IDC_CHECK_D24, m_bCheckD24);
	DDX_Check(pDX, IDC_CHECK_D25, m_bCheckD25);
	DDX_Check(pDX, IDC_CHECK_D26, m_bCheckD26);
	DDX_Check(pDX, IDC_CHECK_D27, m_bCheckD27);
	DDX_Check(pDX, IDC_CHECK_D28, m_bCheckD28);
	DDX_Check(pDX, IDC_CHECK_D29, m_bCheckD29);
	DDX_Check(pDX, IDC_CHECK_D30, m_bCheckD30);
	DDX_Check(pDX, IDC_CHECK_D31, m_bCheckD31);
	DDX_Check(pDX, IDC_CHECK_D32, m_bCheckD32);
	DDX_Check(pDX, IDC_CHECK_D33, m_bCheckD33);
	DDX_Check(pDX, IDC_CHECK_D34, m_bCheckD34);
	DDX_Check(pDX, IDC_CHECK_D35, m_bCheckD35);
	DDX_Check(pDX, IDC_CHECK_D36, m_bCheckD36);
	DDX_Check(pDX, IDC_CHECK_D37, m_bCheckD37);
	DDX_Check(pDX, IDC_CHECK_D38, m_bCheckD38);
	DDX_Check(pDX, IDC_CHECK_D39, m_bCheckD39);
	DDX_Check(pDX, IDC_CHECK_D40, m_bCheckD40);
	DDX_Check(pDX, IDC_CHECK_D41, m_bCheckD41);
	DDX_Check(pDX, IDC_CHECK_D42, m_bCheckD42);
	DDX_Check(pDX, IDC_CHECK_D43, m_bCheckD43);
	DDX_Check(pDX, IDC_CHECK_D44, m_bCheckD44);
	DDX_Check(pDX, IDC_CHECK_D45, m_bCheckD45);
	DDX_Check(pDX, IDC_CHECK_D46, m_bCheckD46);
	DDX_Check(pDX, IDC_CHECK_D47, m_bCheckD47);
	DDX_Check(pDX, IDC_CHECK_D48, m_bCheckD48);
	DDX_Check(pDX, IDC_CHECK_D49, m_bCheckD49);
	DDX_Check(pDX, IDC_CHECK_D50, m_bCheckD50);
	DDX_Check(pDX, IDC_CHECK_D51, m_bCheckD51);
	DDX_Check(pDX, IDC_CHECK_D52, m_bCheckD52);
	DDX_Check(pDX, IDC_CHECK_D53, m_bCheckD53);
	DDX_Check(pDX, IDC_CHECK_D54, m_bCheckD54);
	DDX_Check(pDX, IDC_CHECK_D55, m_bCheckD55);
	DDX_Check(pDX, IDC_CHECK_D56, m_bCheckD56);
	DDX_Check(pDX, IDC_CHECK_D57, m_bCheckD57);
	DDX_Check(pDX, IDC_CHECK_D58, m_bCheckD58);
	DDX_Check(pDX, IDC_CHECK_D59, m_bCheckD59);
	DDX_Check(pDX, IDC_CHECK_D60, m_bCheckD60);
	DDX_Check(pDX, IDC_CHECK_D61, m_bCheckD61);
	DDX_Check(pDX, IDC_CHECK_D62, m_bCheckD62);
	DDX_Check(pDX, IDC_CHECK_D63, m_bCheckD63);
	DDX_Check(pDX, IDC_CHECK_D64, m_bCheckD64);
	DDX_Text(pDX, IDC_EDIT_UPGRADE_FILE, m_strUpgradeFile);
	DDX_Text(pDX, IDC_EDIT_OUTPUT, m_strOutput);
	DDX_Control(pDX, IDC_TREE_CHAN, m_treeChan);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgUpgradeIPC, CDialog)
	//{{AFX_MSG_MAP(CDlgUpgradeIPC)
	ON_BN_CLICKED(IDC_BTN_BROWSE_FILE, OnBtnBrowseFile)
	ON_BN_CLICKED(IDC_BTN_UPGRADE_EXIT, OnBtnUpgradeExit)
	ON_BN_CLICKED(IDC_BTN_UPGRADE, OnBtnUpgrade)
	ON_WM_CLOSE()
	ON_MESSAGE(MSG_UPGRADE_IPC_STATUS, OnUpgradIPCStatusMessage)
	ON_BN_CLICKED(IDC_BTN_UPGRADE_STOP, OnBtnUpgradeStop)
	ON_NOTIFY(NM_CLICK, IDC_TREE_CHAN, OnClickTreeChan)
    ON_BN_CLICKED(IDC_BTN_SELECT_ALL, OnBtnSelectAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgUpgradeIPC message handlers


LRESULT CDlgUpgradeIPC::OnUpgradIPCStatusMessage(WPARAM wParam, LPARAM lParam)
{
	DWORD dwStatus = (DWORD)wParam;
	m_strOutput.Format("Status = %d\r\n", dwStatus);

	if (NET_SDK_CALLBACK_STATUS_SUCCESS == dwStatus)
    {
        m_strOutput.Format("Status[Update Success]\r\n");
		GetDlgItem(IDC_BTN_UPGRADE)->EnableWindow(TRUE);
    }
    else if (NET_SDK_CALLBACK_STATUS_FAILED == dwStatus)
    {	
        m_strOutput.Format("Status[update Failed]\r\n");
        if (lParam != NULL)
        {
            LPNET_DVR_UPGRADE_IPC_ERR_INFO lpErrInfo = (LPNET_DVR_UPGRADE_IPC_ERR_INFO)lParam;
            for (int i=0; i<64; i++)
            {
                CString strTemp;
                strTemp.Format("[%d] Channel=%d, ErrCode=%d \r\n",  i, lpErrInfo->struErrInfoSingle[i].struStreamInfo.dwChannel, lpErrInfo->struErrInfoSingle[i].wErrCode);
                m_strOutput += strTemp;
            }
        }
	}
	else if (NET_SDK_CALLBACK_STATUS_PROCESSING == dwStatus)
	{
        if (!m_bInProcessing)
        {
            m_strOutput.Format("Status[in Update  Processing...]\r\n");
            m_bInProcessing = TRUE;
        }
        
		GetDlgItem(IDC_BTN_UPGRADE)->EnableWindow(FALSE);
	}
	else if(NET_SDK_CALLBACK_STATUS_EXCEPTION == dwStatus)
	{
        m_strOutput.Format("Status[exception]\r\n");
		GetDlgItem(IDC_BTN_UPGRADE)->EnableWindow(TRUE);
	}
    else if(NET_SDK_CALLBACK_STATUS_LANGUAGE_MISMATCH == dwStatus)
    {
        m_strOutput.Format("Status[Language mismatch]\r\n");
        GetDlgItem(IDC_BTN_UPGRADE)->EnableWindow(TRUE);
	}
    else if(NET_SDK_CALLBACK_STATUS_DEV_TYPE_MISMATCH == dwStatus)
    {
        m_strOutput.Format("Status[Device Type Mismatch]\r\n");
        GetDlgItem(IDC_BTN_UPGRADE)->EnableWindow(TRUE);
    }
    else
    {
        m_strOutput.Format("UnKnown Status[%d]\r\n", dwStatus);
        GetDlgItem(IDC_BTN_UPGRADE)->EnableWindow(TRUE);
    }

	UpdateData(FALSE);
	return NULL;
}

void CDlgUpgradeIPC::OnBtnBrowseFile() 
{
	// TODO: Add your control notification handler code here
	static char szFilter[]="All File(*.*)|*.*||";
	CFileDialog dlg(TRUE,"*.*","digicap",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		szFilter);
	if (dlg.DoModal()==IDOK)
	{
		m_strUpgradeFile = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

void CDlgUpgradeIPC::OnBtnUpgradeExit() 
{
	// TODO: Add your control notification handler code here
	if (m_lHandle >= 0)
	{
		NET_DVR_StopRemoteConfig(m_lHandle);
		m_lHandle = -1;
	}
	
	CDialog::OnCancel();
}

void CDlgUpgradeIPC::OnBtnUpgrade() 
{
	// TODO: Add your control notification handler code here
	OnBtnUpgradeStop();
	UpdateData(TRUE);
	char szLan[128] = {0};
	NET_DVR_UPGRADE_IPC_PARAM struUpgradeIPCParam = {0};
	m_bInProcessing = FALSE;
    m_strOutput = "";
	int iIndex = 0;
/*	if (m_bCheckD1)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan;
		iIndex++;
	}

	if (m_bCheckD2)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 1;
		iIndex++;
	}

	if (m_bCheckD3)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 2;
		iIndex++;
	}


	if (m_bCheckD4)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 3;
		iIndex++;
	}

	if (m_bCheckD5)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 4;
		iIndex++;
	}

	if (m_bCheckD6)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 5;
		iIndex++;
	}

	if (m_bCheckD7)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 6;
		iIndex++;
	}

	if (m_bCheckD8)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 7;
		iIndex++;
	}
	
	if (m_bCheckD9)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 8;
		iIndex++;
	}
	
	if (m_bCheckD10)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 9;
		iIndex++;
	}
	
	if (m_bCheckD11)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 10;
		iIndex++;
	}
	
	if (m_bCheckD12)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 11;
		iIndex++;
	}
	
	if (m_bCheckD13)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 12;
		iIndex++;
	}
	
	if (m_bCheckD14)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 13;
		iIndex++;
	}
	
	if (m_bCheckD15)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 14;
		iIndex++;
	}

	if (m_bCheckD16)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 15;
		iIndex++;
	}
	
	if (m_bCheckD17)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 16;
		iIndex++;
	}
	
	if (m_bCheckD18)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 17;
		iIndex++;
	}
	
	if (m_bCheckD19)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 18;
		iIndex++;
	}
	
	if (m_bCheckD20)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 19;
		iIndex++;
	}
	
	if (m_bCheckD21)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 20;
		iIndex++;
	}
	
	if (m_bCheckD22)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 21;
		iIndex++;
	}
	
	if (m_bCheckD23)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 22;
		iIndex++;
	}

	if (m_bCheckD24)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 23;
		iIndex++;
	}
	
	if (m_bCheckD25)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 24;
		iIndex++;
	}
	
	if (m_bCheckD26)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 25;
		iIndex++;
	}
	
	if (m_bCheckD27)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 26;
		iIndex++;
	}
	
	if (m_bCheckD28)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 27;
		iIndex++;
	}
	
	if (m_bCheckD29)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 28;
		iIndex++;
	}
	
	if (m_bCheckD30)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 29;
		iIndex++;
	}
	
	if (m_bCheckD31)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 30;
		iIndex++;
	}

	if (m_bCheckD32)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 31;
		iIndex++;
	}
	
	if (m_bCheckD33)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 32;
		iIndex++;
	}
	
	if (m_bCheckD34)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 33;
		iIndex++;
	}
	
	if (m_bCheckD35)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 34;
		iIndex++;
	}
	
	if (m_bCheckD36)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 35;
		iIndex++;
	}
	
	if (m_bCheckD37)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 36;
		iIndex++;
	}
	
	if (m_bCheckD38)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 37;
		iIndex++;
	}
	
	if (m_bCheckD39)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 38;
		iIndex++;
	}

	if (m_bCheckD40)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 39;
		iIndex++;
	}
	
	if (m_bCheckD41)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 40;
		iIndex++;
	}
	
	if (m_bCheckD42)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 41;
		iIndex++;
	}
	
	if (m_bCheckD43)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 42;
		iIndex++;
	}
	
	if (m_bCheckD44)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 43;
		iIndex++;
	}
	
	if (m_bCheckD45)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 44;
		iIndex++;
	}
	
	if (m_bCheckD46)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 45;
		iIndex++;
	}
	
	if (m_bCheckD47)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 46;
		iIndex++;
	}

	if (m_bCheckD48)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 47;
		iIndex++;
	}
	
	if (m_bCheckD49)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 48;
		iIndex++;
	}
	
	if (m_bCheckD50)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 49;
		iIndex++;
	}
	
	if (m_bCheckD51)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 50;
		iIndex++;
	}
	
	if (m_bCheckD52)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 51;
		iIndex++;
	}
	
	if (m_bCheckD53)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 52;
		iIndex++;
	}
	
	if (m_bCheckD54)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 53;
		iIndex++;
	}
	
	if (m_bCheckD55)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 54;
		iIndex++;
	}

	if (m_bCheckD56)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 55;
		iIndex++;
	}
	
	if (m_bCheckD57)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 56;
		iIndex++;
	}
	
	if (m_bCheckD58)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 57;
		iIndex++;
	}
	
	if (m_bCheckD59)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 58;
		iIndex++;
	}
	
	if (m_bCheckD60)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 59;
		iIndex++;
	}
	
	if (m_bCheckD61)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 60;
		iIndex++;
	}
	
	if (m_bCheckD62)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 61;
		iIndex++;
	}
	
	if (m_bCheckD63)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 62;
		iIndex++;
	}

	if (m_bCheckD64)
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwChannel = m_struIPParamCfgV40.dwStartDChan + 63;
	}
*/
g_StringLanType(szLan,"一次只能升级64个通道", "You can only upgrade 64-channel Once!");
if (m_dwSelectNum >64)
{
	AfxMessageBox(szLan);
	return;
}

for (int i = 0; i < g_struDeviceInfo[m_dwDevIndex].iDeviceChanNum; i++)
{
	if (m_pbUpdateChan[i])
	{
		struUpgradeIPCParam.struStreamInfo[iIndex].dwSize = sizeof(NET_DVR_STREAM_INFO);
		struUpgradeIPCParam.struStreamInfo[iIndex++].dwChannel = i+1;
	}
}

	strcpy(struUpgradeIPCParam.szFileName, m_strUpgradeFile.GetBuffer(0));

	GetDlgItem(IDC_BTN_UPGRADE)->EnableWindow(FALSE);
	m_lHandle = NET_DVR_StartRemoteConfig(m_lUserID, NET_DVR_UPGRADE_IPC, &struUpgradeIPCParam, sizeof(NET_DVR_UPGRADE_IPC_PARAM), cbUpgradeIPCStatus, this);
	if (m_lHandle == -1)
	{
		g_StringLanType(szLan,"启动失败","Fail to start");
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_FAIL_T, "NET_DVR_StartRemoteConfig");
		AfxMessageBox(szLan);
		GetDlgItem(IDC_BTN_UPGRADE)->EnableWindow(TRUE);
		return;
	}
	else
	{
		g_StringLanType(szLan,"启动成功","Succeed to start");
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_SUCC_T, "NET_DVR_StartRemoteConfig");
	}
}

BOOL CDlgUpgradeIPC::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	DWORD dwReturned;
	if (!NET_DVR_GetDVRConfig(m_lUserID, NET_DVR_GET_IPPARACFG_V40, 0, &m_struIPParamCfgV40, sizeof(NET_DVR_IPPARACFG_V40), &dwReturned))
	{
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_FAIL_T, "NET_DVR_GET_IPPARACFG_V40");
	}
	else
	{
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_SUCC_T, "NET_DVR_GET_IPPARACFG_V40");
	}

	m_pbUpdateChan = new BOOL[MAX_CHANNUM_V40];
    if (m_pbUpdateChan != NULL)
    {
        memset(m_pbUpdateChan, 0, sizeof(BOOL)*MAX_CHANNUM_V40);
    }
    else
    {
        return FALSE;
    }
    CreateTree();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgUpgradeIPC::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	if (m_lHandle >= 0)
	{
		NET_DVR_StopRemoteConfig(m_lHandle);
		m_lHandle = -1;
	}
	CDialog::OnClose();
}

void CDlgUpgradeIPC::OnBtnUpgradeStop() 
{
	// TODO: Add your control notification handler code here
	if (m_lHandle >= 0)
	{
		if (!NET_DVR_StopRemoteConfig(m_lHandle))
		{
			g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_FAIL_T, "NET_DVR_StopRemoteConfig");
		}
		else
		{
			m_bInProcessing =FALSE;
            m_strOutput = "";
			g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_SUCC_T, "NET_DVR_StopRemoteConfig");
			GetDlgItem(IDC_BTN_UPGRADE)->EnableWindow(TRUE);
		}

		m_lHandle = -1;
	}
}

void CDlgUpgradeIPC::OnClickTreeChan(NMHDR* pNMHDR, LRESULT* pResult) 
{
    UpdateData(TRUE);
    CPoint pt(0,0);
    CRect rc(0,0,0,0);
    GetCursorPos(&pt);
    GetDlgItem(IDC_TREE_CHAN)->GetWindowRect(&rc);
    ScreenToClient(&rc);
    ScreenToClient(&pt);
    pt.x = pt.x - rc.left;
    pt.y = pt.y - rc.top;
    char szLan[1024];
    
    UINT uFlag = 0;
    HTREEITEM hSelect = m_treeChan.HitTest(pt, &uFlag);
    
    if (NULL == hSelect) 
    {
        return;
    }
    m_treeChan.SelectItem(hSelect);
    DWORD dwIndex = m_treeChan.GetItemData(hSelect)%1000;
    BOOL bCheck = m_treeChan.GetCheck(hSelect);
    m_pbUpdateChan[dwIndex] = !bCheck;
    //
    if (uFlag != LVHT_TOLEFT)
    {
        m_treeChan.SetCheck(hSelect, !bCheck);
        if ( m_dwSelectNum > 0 )
        {
            m_dwSelectNum--;
        }
        
    }
    else
    {
        if (m_dwSelectNum < 64)
        {
            m_treeChan.SetCheck(hSelect, bCheck);
            m_dwSelectNum++; 
        }
        else
        {
            g_StringLanType(szLan,"一次最多只能升级64个通道","You can only upgrade 64-channel Once!");
            AfxMessageBox(szLan);
        }
    }
    
    *pResult = 0;
}


/*********************************************************
  Function:	CreateTree
  Desc:		Create alarm output-video channel connection
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgUpgradeIPC::CreateTree(void)
{
	//m_treeAlarmOut.DeleteAllItems();
 	m_treeChan.DeleteAllItems();
    CreateChannelTree(m_treeChan, m_dwDevIndex, m_pbUpdateChan);
  
	UpdateData(FALSE);
}

/*********************************************************
  Function:	OnBtnSelectAll
  Desc:		select chan
  Input:	
  Output:	
  Return:	
**********************************************************/
void CDlgUpgradeIPC::OnBtnSelectAll(void)
{
 
    CString struChanText = "";
    BOOL bSetAll = FALSE;
    int  i = 0;
    GetDlgItem(IDC_BTN_SELECT_ALL)->GetWindowText(struChanText);
    if (struChanText.Find("全选") != -1)
    {
        GetDlgItem(IDC_BTN_SELECT_ALL)->SetWindowText("全不选");
        
        for (i = 0; i < MAX_CHANNUM_V40; i++)
        {
            if ((i <  g_struDeviceInfo[m_dwDevIndex].iDeviceChanNum) && g_struDeviceInfo[m_dwDevIndex].pStruChanInfo[i].bEnable)
            {
                m_pbUpdateChan[i] = 1;
            }
            else if (i >= MAX_ANALOG_CHANNUM && g_struDeviceInfo[m_dwDevIndex].pStruChanInfo[i].bEnable)
            {
                m_pbUpdateChan[i] = 1;
            }
        }
        
    }
    else   
    {
        GetDlgItem(IDC_BTN_SELECT_ALL)->SetWindowText("全选");
        for (i = 0; i < MAX_CHANNUM_V40; i++)
        {
            m_pbUpdateChan[i] = 0;
        }
    }
    
    CreateTree();
}

/************************
函数：CreateChannelTree
描述：创建通道树节点
输入：m_treeChannel  修改的通道树节点
      iDeviceIndex   设备的ID
      bChannelChecked 节点是否被选中
输出：
返回值：无
************************/
void CDlgUpgradeIPC::CreateChannelTree(CTreeCtrl &m_treeChannel, int iDeviceIndex, BOOL *bChannelChecked)
{
    if (iDeviceIndex == -1)
    {
        return;
    }
    m_treeChannel.DeleteAllItems();
    CString strTemp =_T("");
//    CString strChanTmp = _T("");
    int i = 0;
    HTREEITEM hChanItem = NULL;
    HTREEITEM hFirstItem = NULL;
    BOOL bChanChecked[MAX_CHANNUM_V40] = {0};

    for(i = 0; i < g_struDeviceInfo[iDeviceIndex].iDeviceChanNum ; i++)
    {
        if(i < g_struDeviceInfo[iDeviceIndex].iAnalogChanNum)
        {
            if (g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bEnable)
            {
                bChanChecked[i] = BOOL(*(bChannelChecked+i));
                strTemp.Format(ANALOG_C_FORMAT, i+1);
                hChanItem =  m_treeChannel.InsertItem(strTemp, 0, 0, TVI_ROOT);
                if (hFirstItem == NULL)
                {
                    hFirstItem = hChanItem;
                }
            
                m_treeChannel.SetItemData(hChanItem, 0*1000 + g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChannelNO);
                if (bChannelChecked[g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChannelNO -1])
                {
                    m_treeChannel.SetCheck(hChanItem, TRUE);
                }
            }
        }
        else
        {
            if (g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bEnable)
            {
                bChanChecked[i] = BOOL(*(bChannelChecked+i));
                strTemp.Format(IP_CAMERA_NAME, i+1 - g_struDeviceInfo[iDeviceIndex].iAnalogChanNum);
                hChanItem =  m_treeChannel.InsertItem(strTemp, 0, 0, TVI_ROOT);
                if (hFirstItem == NULL)
                {
                    hFirstItem = hChanItem;
                }
                m_treeChannel.SetItemData(hChanItem, 0*1000 + g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChannelNO);
                if (bChannelChecked[g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChannelNO -1])
                {
                    m_treeChannel.SetCheck(hChanItem, TRUE);
                }
            }
        }
    }
    m_treeChannel.SelectItem(hFirstItem);//avoid keeping clearing screen
	m_treeChannel.Expand(m_treeChannel.GetRootItem(),TVE_EXPAND);
}