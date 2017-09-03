// DlgDevServer.cpp : implementation file
//

#include "stdafx.h"
#include "clientdemo.h"
#include "DlgDevServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgDevServer dialog


CDlgDevServer::CDlgDevServer(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDevServer::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgDevServer)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	memset(&m_struDevServerCfg, 0, sizeof(m_struDevServerCfg));
	memset(&m_struSecurityCfg, 0, sizeof(m_struSecurityCfg));
	m_lServerID = -1;
}


void CDlgDevServer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgDevServer)
	DDX_Control(pDX, IDC_COMBO_RTSP_AUTHEN, m_comRtspAuthentication);
	DDX_Control(pDX, IDC_COMBO_TELNET_SERVER_NEW, m_comboTelnetServerNew);
	DDX_Control(pDX, IDC_COMBO_WEB_AUTHEN, m_comWebAuthentication);
	DDX_Control(pDX, IDC_COMBO_SSH_SERVER, m_comSSHServer);
	DDX_Control(pDX, IDC_COMBO_AUTO_DEFOG, m_comAutoDefog);
	DDX_Control(pDX, IDC_COMBO_ABF_SERVER, m_comboAbfServer);
	DDX_Control(pDX, IDC_COMBO_TELNET_SERVER, m_comboTelnetServer);
	DDX_Control(pDX, IDC_COMBO_IRLAMP_SERVER, m_comboIRLampServer);
	DDX_Control(pDX, IDC_COMBO_ENABLESTATUS, m_comboEnableLEDStatus);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgDevServer, CDialog)
	//{{AFX_MSG_MAP(CDlgDevServer)
	ON_BN_CLICKED(IDC_BTN_SET, OnBtnSet)
	ON_BN_CLICKED(IDC_BTN_GET, OnBtnGet)
	ON_WM_CANCELMODE()
	ON_BN_CLICKED(IDC_BTN_SEC_SET, OnBtnSecSet)
	ON_BN_CLICKED(IDC_BTN_SEC_GET, OnBtnSecGet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgDevServer message handlers

void CDlgDevServer::OnBtnSet() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	char szLan[128] = {0};
	m_struDevServerCfg.byIrLampServer = m_comboIRLampServer.GetCurSel();
	m_struDevServerCfg.bytelnetServer = m_comboTelnetServer.GetCurSel();
	m_struDevServerCfg.byABFServer = m_comboAbfServer.GetCurSel();
	m_struDevServerCfg.dwSize = sizeof(m_struDevServerCfg);
	m_struDevServerCfg.byEnableLEDStatus = m_comboEnableLEDStatus.GetCurSel();
	//2014-03-21 ����
	m_struDevServerCfg.byEnableAutoDefog = m_comAutoDefog.GetCurSel();

	if (!NET_DVR_SetDVRConfig(m_lServerID,NET_DVR_SET_DEVSERVER_CFG, m_nChannel,&m_struDevServerCfg, sizeof(m_struDevServerCfg)))	
	{
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_FAIL_T, "NET_DVR_SET_DEVSERVER_CFG %d",NET_DVR_GetLastError());
		g_StringLanType(szLan, "��������ʧ��", "Save failed");
		AfxMessageBox(szLan);
	}
	else
	{
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_SUCC_T, "NET_DVR_SET_DEVSERVER_CFG %d",NET_DVR_GetLastError());
	}
}

void CDlgDevServer::OnBtnGet() 
{
	// TODO: Add your control notification handler code here
	DWORD dwReturn = 0;
	char szLan[128] = {0};
	
    if (!NET_DVR_GetDVRConfig(m_lServerID, NET_DVR_GET_DEVSERVER_CFG, m_nChannel, &m_struDevServerCfg,sizeof(m_struDevServerCfg), &dwReturn))
    {
        g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_FAIL_T, "NET_DVR_GET_DEVSERVER_CFG %d",NET_DVR_GetLastError());
		g_StringLanType(szLan, "������ȡʧ��", "Get failed");
		AfxMessageBox(szLan);
    }
	else
	{
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_SUCC_T, "NET_DVR_GET_DEVSERVER_CFG %d",NET_DVR_GetLastError());
	}
	m_comboIRLampServer.SetCurSel(m_struDevServerCfg.byIrLampServer);
	m_comboTelnetServer.SetCurSel(m_struDevServerCfg.bytelnetServer);
	m_comboAbfServer.SetCurSel(m_struDevServerCfg.byABFServer);
	m_comboEnableLEDStatus.SetCurSel(m_struDevServerCfg.byEnableLEDStatus);
	//2014-03-21 ����
	m_comAutoDefog.SetCurSel(m_struDevServerCfg.byEnableAutoDefog);

	UpdateData(FALSE);
}

BOOL CDlgDevServer::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	OnBtnGet();
	OnBtnSecGet();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgDevServer::OnCancelMode() 
{
	CDialog::OnCancelMode();
	
	// TODO: Add your message handler code here
	
}

void CDlgDevServer::OnBtnSecSet() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	char szLan[128] = {0};
	m_struSecurityCfg.dwSize = sizeof(m_struSecurityCfg);
	m_struSecurityCfg.bySSHServer = m_comSSHServer.GetCurSel();
	m_struSecurityCfg.byWebAuthentication = m_comWebAuthentication.GetCurSel();
	m_struSecurityCfg.byTelnetServer = m_comboTelnetServerNew.GetCurSel();
	m_struSecurityCfg.byRtspAuthentication = m_comRtspAuthentication.GetCurSel();
		
	if (!NET_DVR_SetDVRConfig(m_lServerID,NET_DVR_SET_SECURITY_CFG, m_nChannel,&m_struSecurityCfg, sizeof(m_struSecurityCfg)))	
	{
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_FAIL_T, "NET_DVR_SET_SECURITY_CFG %d",NET_DVR_GetLastError());
		g_StringLanType(szLan, "��������ʧ��", "Save failed");
		AfxMessageBox(szLan);
	}
	else
	{
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_SUCC_T, "NET_DVR_SET_SECURITY_CFG %d",NET_DVR_GetLastError());
	}
}

void CDlgDevServer::OnBtnSecGet() 
{
	// TODO: Add your control notification handler code here
	DWORD dwReturn = 0;
	char szLan[128] = {0};
	
    if (!NET_DVR_GetDVRConfig(m_lServerID, NET_DVR_GET_SECURITY_CFG, m_nChannel, &m_struSecurityCfg,sizeof(m_struSecurityCfg), &dwReturn))
    {
        g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_FAIL_T, "NET_DVR_GET_SECURITY_CFG %d",NET_DVR_GetLastError());
		g_StringLanType(szLan, "������ȡʧ��", "Get failed");
		AfxMessageBox(szLan);
    }
	else
	{
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_SUCC_T, "NET_DVR_GET_SECURITY_CFG %d",NET_DVR_GetLastError());
	}

	m_comSSHServer.SetCurSel(m_struSecurityCfg.bySSHServer);
	m_comWebAuthentication.SetCurSel(m_struSecurityCfg.byWebAuthentication);
	m_comboTelnetServerNew.SetCurSel(m_struSecurityCfg.byTelnetServer);
	m_comRtspAuthentication.SetCurSel(m_struSecurityCfg.byRtspAuthentication);
	UpdateData(FALSE);
}
