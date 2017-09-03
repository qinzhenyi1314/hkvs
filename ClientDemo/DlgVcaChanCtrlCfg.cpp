// DlgVcaChanCtrlCfg.cpp : implementation file
//

#include "stdafx.h"
#include "clientdemo.h"
#include "DlgVcaChanCtrlCfg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgVcaChanCtrlCfg dialog



CDlgVcaChanCtrlCfg::CDlgVcaChanCtrlCfg(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVcaChanCtrlCfg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgVcaChanCtrlCfg)
	m_bChkStreamWithVca = FALSE;
	m_bChkEnable = FALSE;
	//}}AFX_DATA_INIT
	m_iVcaChannel = -1;
	m_bSupportChanEx = FALSE;
	memset(&m_struVcaCtrlInfo, 0, sizeof(m_struVcaCtrlInfo));
}


void CDlgVcaChanCtrlCfg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgVcaChanCtrlCfg)
	DDX_Control(pDX, IDC_COMBO_VCA_MODE, m_comboVcaMode);
	DDX_Control(pDX, IDC_COMBO_CHAN_VCATYPE, m_comboChanVcaType);
	DDX_Control(pDX, IDC_COMBO_CHANNEL, m_comboChan);
	DDX_Check(pDX, IDC_CHK_STREAM_WITH_VCA, m_bChkStreamWithVca);
	DDX_Check(pDX, IDC_CHK_ENABLE, m_bChkEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgVcaChanCtrlCfg, CDialog)
	//{{AFX_MSG_MAP(CDlgVcaChanCtrlCfg)
	ON_BN_CLICKED(IDC_BTN_SURE, OnBtnSure)
	ON_BN_CLICKED(IDC_BTN_GET, OnBtnGet)
	ON_BN_CLICKED(IDC_BTN_SET, OnBtnSet)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNEL, OnSelchangeComboChannel)
	ON_CBN_SELCHANGE(IDC_COMBO_CHAN_VCATYPE, OnSelchangeComboChanVcatype)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgVcaChanCtrlCfg message handlers
BOOL CDlgVcaChanCtrlCfg::OnInitDialog()
{
	CDialog::OnInitDialog();
	char szLan[128] = {0};
    
	m_comboChan.ResetContent();
    for (int i = 0; i < m_iChannelNum; i++)
    {
        sprintf(szLan, "Chan%d", i+1);
        m_comboChan.InsertString(i,szLan);
        m_comboChan.SetItemData(i, i+1);
    }
	
	memset(&m_struVcaDevAbility, 0, sizeof(NET_VCA_DEV_ABILITY));
	if(!NET_DVR_GetDeviceAbility(g_struDeviceInfo[m_iDevIndex].lLoginID,  VCA_DEV_ABILITY, NULL, 0, (char*)&m_struVcaDevAbility, sizeof(NET_VCA_DEV_ABILITY)))
	{
		g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "VCA_DEV_ABILITY");
		g_StringLanType(szLan, "�豸��֧��", "Device does not support");
		AfxMessageBox(szLan);
		return FALSE;
	}
	else 
	{
		g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "VCA_DEV_ABILITY");
    }
	
	m_bSupportChanEx = (m_struVcaDevAbility.bySupport>>1)&1;

	m_comboChanVcaType.ResetContent();
    int index = 0;
    if (m_struVcaDevAbility.byBBaseChanNum > 0)
    {
        char szLan[128] = {0};
        g_StringLanType(szLan, "��Ϊ����������", "Behavior of basic");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_BEHAVIOR_BASE);
        index++;
    }
    if (m_struVcaDevAbility.byBAdvanceChanNum > 0)
    {
        char szLan[128] = {0};
        g_StringLanType(szLan, "��Ϊ�����߼���", "Behavior of advanced");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_BEHAVIOR_ADVANCE);
        index++;
    }
    if (m_struVcaDevAbility.byBFullChanNum > 0)
    {
        char szLan[128] = {0};
        g_StringLanType(szLan, "��Ϊ����������", "Behavior of full");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_BEHAVIOR_FULL);
        index++;
    }
    if (m_struVcaDevAbility.byPlateChanNum> 0)
    {   
		g_StringLanType(szLan, "����ʶ��", "Plate Recognition");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_PLATE);
        index++;
    }
    if (m_struVcaDevAbility.byATMChanNum > 0)
    {
        m_comboChanVcaType.InsertString(index, "ATM");
        m_comboChanVcaType.SetItemData(index, VCA_ATM);
        index++;
    }
	if (m_struVcaDevAbility.byPDCChanNum > 0)
    {   
		g_StringLanType(szLan, "����ͳ��", "PDC");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_PDC);
        index++;
    }
    if (m_struVcaDevAbility.byITSChanNum > 0)
    {;
		g_StringLanType(szLan, "��ͨ�¼�", "ITS");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_ITS);
        index++;
    }
    
    if (m_struVcaDevAbility.byBPrisonChanNum > 0)
    {   
		g_StringLanType(szLan, "��Ϊ������(����)", "Behavior of prison");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_BEHAVIOR_PRISON);
        index++;
    }	

	if (m_struVcaDevAbility.byFSnapChanNum > 0)
	{   
		g_StringLanType(szLan, "����ץ��", "Face Snap");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_FACE_SNAP);
        index++;
	}
	
	if (m_struVcaDevAbility.byFSnapRecogChanNum > 0)
	{   
		g_StringLanType(szLan, "����ץ����ʶ��", "Face Snap and Recognition");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_FACE_SNAPRECOG);
        index++;
	}
	
	if (m_struVcaDevAbility.byFRetrievalChanNum > 0)
	{
		g_StringLanType(szLan, "���������", "Face Retrieval");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_FACE_RETRIEVAL);
        index++;
	}
	
	if (m_struVcaDevAbility.byFRecogChanNum > 0)
	{   
		g_StringLanType(szLan, "����ʶ��", "Face Recognize");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_FACE_RECOG);
        index++;
	}

	if (m_struVcaDevAbility.byBPPerimeterChanNum > 0)
	{  
	    g_StringLanType(szLan, "��Ϊ������(�ܽ�)", "Prison perimeter");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_BEHAVIOR_PRISON_PERIMETER);
        index++;
	}
	
	if (m_struVcaDevAbility.byTPSChanNum > 0)
	{
		g_StringLanType(szLan, "��ͨ�յ�", "TPS");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_TPS);
        index++;
	}

	if (m_struVcaDevAbility.byTFSChanNum > 0)
	{   
		g_StringLanType(szLan, "��ͨΥ��ȡ֤", "TFS");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_TFS);
		index++;
	}
	
	if (m_struVcaDevAbility.byFSnapBFullChanNum > 0)
	{   
		g_StringLanType(szLan, "����ץ�ĺ���Ϊ����", "BEHAVIOR_FACESNA");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_BEHAVIOR_FACESNAP);
		index++;
	}
	
	if (m_struVcaDevAbility.byHeatMapChanNum > 0)
	{   
		g_StringLanType(szLan, "�ȶ�ͼ", "HEATMAP");
        m_comboChanVcaType.InsertString(index, szLan);
        m_comboChanVcaType.SetItemData(index, VCA_HEATMAP);
		index++;
	}

	if (!GetVcaCtrlCfg())
    {
        AfxMessageBox("Fail to get vca ctrl");
		CDialog::OnCancel();
        return FALSE;
    }
    
    m_comboChan.SetCurSel(0);
//	OnSelchangeComboChannel();
	
	return TRUE;
}

void CDlgVcaChanCtrlCfg::OnBtnSure() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_bSupportChanEx)
	{
		GetVcaCtrlInfoFromWnd(m_struVcaCtrlInfo);
	}
	else
	{
		GetVcaCtrlFromWnd(m_struVcaCtrlCfg.struCtrlInfo[m_iVcaChannel-1]);
	}
    
}

void CDlgVcaChanCtrlCfg::OnBtnGet() 
{
	// TODO: Add your control notification handler code here
	GetVcaCtrlCfg();
}

void CDlgVcaChanCtrlCfg::OnBtnSet() 
{
	// TODO: Add your control notification handler code here
	SetVcaCtrlCfg();
}


BOOL CDlgVcaChanCtrlCfg::SetVcaCtrlToWnd(NET_VCA_CTRLINFO const &struVcaCtrlInfo)
{
	
    for (int i = 0; i < m_comboChanVcaType.GetCount(); i++)
    {
        if (m_comboChanVcaType.GetItemData(i) == struVcaCtrlInfo.byVCAType)
        {
            m_comboChanVcaType.SetCurSel(i);
            break;
        }
    }
	
    m_bChkStreamWithVca = struVcaCtrlInfo.byStreamWithVCA;
    m_bChkEnable = struVcaCtrlInfo.byVCAEnable;

	ShowVcaChanModeParam(struVcaCtrlInfo.byVCAType,struVcaCtrlInfo.byMode);

	UpdateData(FALSE);
    return TRUE;
}

BOOL CDlgVcaChanCtrlCfg::GetVcaCtrlFromWnd(NET_VCA_CTRLINFO &struVcaCtrlInfo)
{
    struVcaCtrlInfo.byVCAEnable     = m_bChkEnable;
    struVcaCtrlInfo.byStreamWithVCA = m_bChkStreamWithVca;
    struVcaCtrlInfo.byVCAType       = m_comboChanVcaType.GetItemData(m_comboChanVcaType.GetCurSel());

	if (struVcaCtrlInfo.byVCAType == VCA_ATM ||
		struVcaCtrlInfo.byVCAType == VCA_TFS ||
		struVcaCtrlInfo.byVCAType == VCA_BEHAVIOR_FULL) 
	{   
		int nCurSelMode         = m_comboVcaMode.GetCurSel();
		struVcaCtrlInfo.byMode  = m_comboVcaMode.GetItemData(nCurSelMode);
	}

    return TRUE;
}

BOOL CDlgVcaChanCtrlCfg::SetVcaCtrlCfg()
{
	BOOL bRet = FALSE;
	UpdateData(TRUE);
    DWORD dwStatus = 0xffffffff; 
	
	if (m_bSupportChanEx)
	{
		NET_DVR_VCA_CTRLINFO_COND struCond = {0};
		struCond.dwSize = sizeof(struCond);
		struCond.struStreamInfo.dwChannel = m_iVcaChannel;
		m_struVcaCtrlInfo.dwSize = sizeof(NET_DVR_VCA_CTRLINFO_CFG);
		BOOL bRet = NET_DVR_SetDeviceConfig(m_lServerID, NET_DVR_SET_VCA_CTRLINFO_CFG, 1, &struCond,sizeof(struCond),&dwStatus, &m_struVcaCtrlInfo, sizeof(m_struVcaCtrlInfo));
		
		if (!(bRet && (0 == dwStatus)))
		{
			g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_SET_VCA_CTRLINFO_CFG");            
			bRet = FALSE;
		}
		else
		{
			g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_SET_VCA_CTRLINFO_CFG");            
			bRet = TRUE;
		}
	}
	else
	{
		if(!NET_DVR_SetDVRConfig(m_lServerID, NET_DVR_SET_VCA_CTRLCFG, m_iVcaChannel, &m_struVcaCtrlCfg, sizeof(m_struVcaCtrlCfg)))
		{
			g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_SET_VCA_CTRLCFG");            
			bRet = FALSE;
		}
		else
		{
			g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_SET_VCA_CTRLCFG");            
			bRet = TRUE;
		}
	}
    
    return bRet;
}

BOOL CDlgVcaChanCtrlCfg::GetVcaCtrlCfg()
{

	UpdateData(TRUE);
	BOOL bRet = FALSE;
    DWORD dwReturn = 0;
	if (m_bSupportChanEx)
	{
		DWORD dwStatus = 0;
		NET_DVR_VCA_CTRLINFO_COND struCond = {0};
		struCond.dwSize = sizeof(struCond);
		struCond.struStreamInfo.dwChannel = m_iVcaChannel;
		BOOL bRet = NET_DVR_GetDeviceConfig(m_lServerID, NET_DVR_GET_VCA_CTRLINFO_CFG, 1, &struCond,sizeof(struCond),&dwStatus, &m_struVcaCtrlInfo, sizeof(m_struVcaCtrlInfo));
		
		if (!(bRet && (0 == dwStatus)))
		{
			g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_GET_VCA_CTRLINFO_CFG");            
			bRet = FALSE;
			return bRet;
		}
		else
		{
			g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_GET_VCA_CTRLINFO_CFG");            
			SetVcaCtrlInfoToWnd(m_struVcaCtrlInfo);
			return TRUE;
		}	
	}
	else
	{
		if(!NET_DVR_GetDVRConfig(m_lServerID, NET_DVR_GET_VCA_CTRLCFG, m_iVcaChannel, &m_struVcaCtrlCfg, sizeof(m_struVcaCtrlCfg), &dwReturn))
		{
			g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_GET_VCA_CTRLCFG");            
			//bRet = FALSE;
			return bRet;
		}
		else
		{
			g_pMainDlg->AddLog(m_iDevIndex, OPERATION_SUCC_T, "NET_DVR_GET_VCA_CTRLCFG");            
			//bRet = TRUE;
			SetVcaCtrlToWnd(m_struVcaCtrlCfg.struCtrlInfo[m_iVcaChannel -1]);
			return TRUE;
		}
	}
	
	//   UpdateData(FALSE);
    return bRet;
}

void CDlgVcaChanCtrlCfg::OnSelchangeComboChannel() 
{
//	m_iVcaChannel = m_comboChan.GetItemData(m_comboChan.GetCurSel());
    SetVcaCtrlToWnd(m_struVcaCtrlCfg.struCtrlInfo[m_iVcaChannel-1]);
    UpdateData(FALSE);
}

BOOL CDlgVcaChanCtrlCfg::SetVcaCtrlInfoToWnd(NET_DVR_VCA_CTRLINFO_CFG const &struVcaCtrlInfo)
{
	
    for (int i = 0; i < m_comboChanVcaType.GetCount(); i++)
    {
        if (m_comboChanVcaType.GetItemData(i) == struVcaCtrlInfo.byVCAType )
        {
            m_comboChanVcaType.SetCurSel(i);
            break;
        }
    }
	
    m_bChkStreamWithVca = struVcaCtrlInfo.byStreamWithVCA;
    m_bChkEnable = struVcaCtrlInfo.byVCAEnable;

	ShowVcaChanModeParam(struVcaCtrlInfo.byVCAType,struVcaCtrlInfo.byMode);

	UpdateData(FALSE);
    return TRUE;
}

BOOL CDlgVcaChanCtrlCfg::GetVcaCtrlInfoFromWnd(NET_DVR_VCA_CTRLINFO_CFG &struVcaCtrlInfo)
{
    struVcaCtrlInfo.byVCAEnable = m_bChkEnable;
    struVcaCtrlInfo.byStreamWithVCA = m_bChkStreamWithVca;
    struVcaCtrlInfo.byVCAType = m_comboChanVcaType.GetItemData(m_comboChanVcaType.GetCurSel());
    struVcaCtrlInfo.byMode = m_comboVcaMode.GetCurSel();

	if (struVcaCtrlInfo.byVCAType == VCA_ATM ||
		struVcaCtrlInfo.byVCAType == VCA_TFS ||
		struVcaCtrlInfo.byVCAType == VCA_BEHAVIOR_FULL)
	{   
		int nCurSelMode         = m_comboVcaMode.GetCurSel();
		struVcaCtrlInfo.byMode  = m_comboVcaMode.GetItemData(nCurSelMode);
	}

    return TRUE;
}

void CDlgVcaChanCtrlCfg::OnSelchangeComboChanVcatype() 
{
	int nCurSelType = m_comboChanVcaType.GetCurSel();
	if (nCurSelType != CB_ERR)
	{
		int nVcaType = m_comboChanVcaType.GetItemData(nCurSelType);
		ShowVcaChanModeParam(nVcaType,0);
	}
}


void CDlgVcaChanCtrlCfg::ShowVcaChanModeParam(int nVcaChanType,int nChanMode)
{   
	m_comboVcaMode.ResetContent();
    
	BOOL bShowVcaModeWnd = FALSE;
	char szLan[128] = {0};
	if (nVcaChanType == VCA_ATM)
	{
		g_StringLanType(szLan, "���", "Panel");
		m_comboVcaMode.InsertString(0,szLan);
		m_comboVcaMode.SetItemData(0,VCA_ATM_PANEL);
		
		g_StringLanType(szLan, "����", "Surround");
		m_comboVcaMode.InsertString(1,szLan);
		m_comboVcaMode.SetItemData(1,VCA_ATM_SURROUND);

		g_StringLanType(szLan, "����", "Face");
		m_comboVcaMode.InsertString(2,szLan);
		m_comboVcaMode.SetItemData(2,VCA_ATM_FACE);

		g_StringLanType(szLan, "������", "SafetyCabin");
		m_comboVcaMode.InsertString(3,szLan);
		m_comboVcaMode.SetItemData(3,VCA_ATM_SAFETYCABIN);

		m_comboVcaMode.SetCurSel(0);
		bShowVcaModeWnd = TRUE;
	}
	else if (nVcaChanType == VCA_TFS)
	{
		g_StringLanType(szLan, "���е�·", "City road");
		m_comboVcaMode.InsertString(0,szLan);
		m_comboVcaMode.SetItemData(0,TFS_CITYROAD);
		
		g_StringLanType(szLan, "���ٵ�·", "Free way");
		m_comboVcaMode.InsertString(1,szLan);
		m_comboVcaMode.SetItemData(1,TFS_FREEWAY);

		m_comboVcaMode.SetCurSel(0);
		bShowVcaModeWnd = TRUE;
	}
	else if (nVcaChanType == VCA_BEHAVIOR_FULL)
	{
		g_StringLanType(szLan, "ϵͳĬ��", "Default");
		m_comboVcaMode.InsertString(0,szLan);
		m_comboVcaMode.SetItemData(0,BEHAVIOR_SCENE_DEFAULT);
		
		g_StringLanType(szLan, "Χǽ", "Wall");
		m_comboVcaMode.InsertString(1,szLan);
		m_comboVcaMode.SetItemData(1,BEHAVIOR_SCENE_WALL);

		g_StringLanType(szLan, "����", "Indoor");
		m_comboVcaMode.InsertString(2,szLan);
		m_comboVcaMode.SetItemData(2,BEHAVIOR_SCENE_INDOOR);
		
		m_comboVcaMode.SetCurSel(0);
		bShowVcaModeWnd = TRUE;
	}

	GetDlgItem(IDC_STATIC_VCA_MODE)->ShowWindow(bShowVcaModeWnd);
	GetDlgItem(IDC_COMBO_VCA_MODE)->ShowWindow(bShowVcaModeWnd);

	if (bShowVcaModeWnd)
	{
		int nCount = m_comboVcaMode.GetCount();
		for (int i = 0; i < nCount; i++)
		{
			if (m_comboVcaMode.GetItemData(i) == nChanMode)
			{
				m_comboVcaMode.SetCurSel(i);
				break;
			}
		}
	}
}