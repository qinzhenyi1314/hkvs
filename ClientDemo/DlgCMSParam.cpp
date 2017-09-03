// DlgCMSParam.cpp : implementation file
//

#include "stdafx.h"
#include "clientdemo.h"
#include "DlgCMSParam.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgCMSParam dialog


DlgCMSParam::DlgCMSParam(CWnd* pParent /*=NULL*/)
	: CDialog(DlgCMSParam::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgCMSParam)
	m_bRegister = FALSE;
	m_sDeviceID = _T("");
	m_sIP = _T("");
	m_Port = 0;
	m_sVersion = _T("");
	//}}AFX_DATA_INIT
	memset(&m_struCmsParam,0,sizeof(m_struCmsParam));
}


void DlgCMSParam::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgCMSParam)
	DDX_Control(pDX, IDC_PROTOCOL, m_ProtocolType);
	DDX_Control(pDX, IDC_INDEX, m_index);
	DDX_Check(pDX, IDC_REGISTER, m_bRegister);
	DDX_Text(pDX, IDC_DEVICEID, m_sDeviceID);
	DDX_Text(pDX, IDC_IP, m_sIP);
	DDX_Text(pDX, IDC_PORT, m_Port);
	DDX_Text(pDX, IDC_VERSION, m_sVersion);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgCMSParam, CDialog)
	//{{AFX_MSG_MAP(DlgCMSParam)
	ON_BN_CLICKED(IDC_SET, OnSet)
	ON_BN_CLICKED(IDC_GET, OnGet)
	ON_EN_CHANGE(IDC_DEVICEID, OnChangeDeviceid)
	ON_CBN_SELCHANGE(IDC_INDEX, OnSelchangeIndex)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgCMSParam message handlers

BOOL DlgCMSParam::OnInitDialog() 
{
	CDialog::OnInitDialog();

	UpdateData(TRUE);
	DWORD dwDeviceIndex = g_pMainDlg->GetCurDeviceIndex();
	char szLan[128] = {0};
	DWORD dwReturn;
	if(NET_DVR_GetDVRConfig(g_struDeviceInfo[dwDeviceIndex].lLoginID,NET_DVR_GET_CMS_CFG,1,&m_struCmsParam,sizeof(m_struCmsParam),&dwReturn))
	{
		g_pMainDlg->AddLog(g_struDeviceInfo[dwDeviceIndex].lLoginID, OPERATION_SUCC_T, "NET_DVR_GET_CMS_CFG");
		g_StringLanType(szLan, "��ȡ�ɹ�", "Get cms cfg successed!");
		AfxMessageBox(szLan);
	}
	else
	{
		g_pMainDlg->AddLog(g_struDeviceInfo[dwDeviceIndex].lLoginID, OPERATION_FAIL_T, "NET_DVR_GET_CMS_CFG");
		g_StringLanType(szLan, "��ȡʧ��", "Get cms cfg failed!");
		AfxMessageBox(szLan);
	}
	int sel = (int)m_struCmsParam.bySeverProtocolType-1;
	m_ProtocolType.SetCurSel(sel);
	m_bRegister = m_struCmsParam.byStatus - 1;
	m_sDeviceID.Format("%s",(char*)m_struCmsParam.sDeviceId);
	m_sIP.Format("%s",m_struCmsParam.struAddr.sIpV4);
	m_Port = m_struCmsParam.wServerPort;
	m_index.SetCurSel(0);
	m_sVersion.Format("%s",(char*)m_struCmsParam.sPlatformEhomeVersion);//2014-03-21
// 	GetDlgItem(IDC_PROTOCOL)->EnableWindow(FALSE);
// 	GetDlgItem(IDC_REGISTER)->EnableWindow(FALSE);
	UpdateData(FALSE);
	return TRUE;
}
void DlgCMSParam::OnSet() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_struCmsParam.bySeverProtocolType = m_ProtocolType.GetCurSel()+1;
	m_struCmsParam.byStatus = m_bRegister+1;
	sprintf((char*)m_struCmsParam.sDeviceId,"%s",m_sDeviceID.GetBuffer(m_sDeviceID.GetLength()));
	sprintf(m_struCmsParam.struAddr.sIpV4,"%s",m_sIP.GetBuffer(m_sIP.GetLength()));
	m_struCmsParam.wServerPort = m_Port;
	m_struCmsParam.dwSize = sizeof(m_struCmsParam);

	DWORD dwDeviceIndex = g_pMainDlg->GetCurDeviceIndex();
	char szLan[128] = {0};
	if(NET_DVR_SetDVRConfig(g_struDeviceInfo[dwDeviceIndex].lLoginID,NET_DVR_SET_CMS_CFG,m_index.GetCurSel()+1,&m_struCmsParam,sizeof(m_struCmsParam)))
	{
		g_pMainDlg->AddLog(g_struDeviceInfo[dwDeviceIndex].lLoginID, OPERATION_SUCC_T, "NET_DVR_SET_CMS_CFG");
		g_StringLanType(szLan, "���óɹ�", "Set cms cfg successed!");
		AfxMessageBox(szLan);
	}
	else
	{
		g_pMainDlg->AddLog(g_struDeviceInfo[dwDeviceIndex].lLoginID, OPERATION_FAIL_T, "NET_DVR_SET_CMS_CFG");
		g_StringLanType(szLan, "����ʧ��", "Set cms cfg failed!");
		AfxMessageBox(szLan);
	}
}

void DlgCMSParam::OnGet() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	DWORD dwDeviceIndex = g_pMainDlg->GetCurDeviceIndex();
	char szLan[128] = {0};
	DWORD dwReturn;
	if(NET_DVR_GetDVRConfig(g_struDeviceInfo[dwDeviceIndex].lLoginID,NET_DVR_GET_CMS_CFG,m_index.GetCurSel()+1,&m_struCmsParam,sizeof(m_struCmsParam),&dwReturn))
	{
		g_pMainDlg->AddLog(g_struDeviceInfo[dwDeviceIndex].lLoginID, OPERATION_SUCC_T, "NET_DVR_GET_CMS_CFG");
		g_StringLanType(szLan, "��ȡ�ɹ�", "Get cms cfg successed!");
		AfxMessageBox(szLan);
	}
	else
	{
		g_pMainDlg->AddLog(g_struDeviceInfo[dwDeviceIndex].lLoginID, OPERATION_FAIL_T, "NET_DVR_GET_CMS_CFG");
		g_StringLanType(szLan, "��ȡʧ��", "Get cms cfg failed!");
		AfxMessageBox(szLan);
	}
	int sel = (int)m_struCmsParam.bySeverProtocolType-1;
	m_ProtocolType.SetCurSel(sel);
	m_bRegister = m_struCmsParam.byStatus - 1;
	m_sDeviceID.Format("%s",(char*)m_struCmsParam.sDeviceId);
	m_sIP.Format("%s",m_struCmsParam.struAddr.sIpV4);
	m_Port = m_struCmsParam.wServerPort;
	m_sVersion.Format("%s",(char*)m_struCmsParam.sPlatformEhomeVersion);//2014-03-21
// 	GetDlgItem(IDC_PROTOCOL)->EnableWindow(FALSE);
// 	GetDlgItem(IDC_REGISTER)->EnableWindow(FALSE);
	UpdateData(FALSE);
}

void DlgCMSParam::OnChangeDeviceid() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void DlgCMSParam::OnSelchangeIndex() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	DWORD dwDeviceIndex = g_pMainDlg->GetCurDeviceIndex();
	char szLan[128] = {0};
	DWORD dwReturn;
	if(NET_DVR_GetDVRConfig(g_struDeviceInfo[dwDeviceIndex].lLoginID,NET_DVR_GET_CMS_CFG,m_index.GetCurSel()+1,&m_struCmsParam,sizeof(m_struCmsParam),&dwReturn))
	{
		g_pMainDlg->AddLog(g_struDeviceInfo[dwDeviceIndex].lLoginID, OPERATION_SUCC_T, "NET_DVR_GET_CMS_CFG");
		g_StringLanType(szLan, "��ȡ�ɹ�", "Get cms cfg successed!");
		AfxMessageBox(szLan);
	}
	else
	{
		g_pMainDlg->AddLog(g_struDeviceInfo[dwDeviceIndex].lLoginID, OPERATION_FAIL_T, "NET_DVR_GET_CMS_CFG");
		g_StringLanType(szLan, "��ȡʧ��", "Get cms cfg failed!");
		AfxMessageBox(szLan);
	}
	int sel = (int)m_struCmsParam.bySeverProtocolType-1;
	if(sel == 1)
		m_ProtocolType.SetCurSel(0);
	else if(sel == 0)
		m_ProtocolType.SetCurSel(1);
	UpdateData(FALSE);
	m_bRegister = m_struCmsParam.byStatus - 1;
	m_sDeviceID.Format("%s",(char*)m_struCmsParam.sDeviceId);
	m_sIP.Format("%s",m_struCmsParam.struAddr.sIpV4);
	m_Port = m_struCmsParam.wServerPort;
	m_sVersion.Format("%s",(char*)m_struCmsParam.sPlatformEhomeVersion);//2014-03-21
//	GetDlgItem(IDC_PROTOCOL_TYPE)->EnableWindow(FALSE);
// 	GetDlgItem(IDC_PROTOCOL)->EnableWindow(FALSE);
// 	GetDlgItem(IDC_REGISTER)->EnableWindow(FALSE);
	UpdateData(FALSE);
}
