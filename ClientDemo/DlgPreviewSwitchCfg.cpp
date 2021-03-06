// DlgPreviewSwitchCfg.cpp : implementation file
//

#include "stdafx.h"
#include "clientdemo.h"
#include "DlgPreviewSwitchCfg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgPreviewSwitchCfg dialog


CDlgPreviewSwitchCfg::CDlgPreviewSwitchCfg(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPreviewSwitchCfg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgPreviewSwitchCfg)
	m_byGroup = 0;
	//}}AFX_DATA_INIT
	m_lUserID = -1;
	m_dwDevIndex = 0;
}


void CDlgPreviewSwitchCfg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgPreviewSwitchCfg)
	DDX_Control(pDX, IDC_PREVIEW_NUMBER, m_cmbPreviewNumber);
	DDX_Control(pDX, IDC_SWITCH_TIME, m_cmbSwitchTime);
	DDX_Control(pDX, IDC_ENABLE_AUDIO, m_cmbAudioPreview);
	DDX_Control(pDX, IDC_WND1, m_cmbWnd1);
	DDX_Control(pDX, IDC_WND2, m_cmbWnd2);
	DDX_Control(pDX, IDC_WND3, m_cmbWnd3);
	DDX_Control(pDX, IDC_WND4, m_cmbWnd4);
	DDX_Control(pDX, IDC_WND5, m_cmbWnd5);
	DDX_Control(pDX, IDC_WND6, m_cmbWnd6);
	DDX_Control(pDX, IDC_WND7, m_cmbWnd7);
	DDX_Control(pDX, IDC_WND8, m_cmbWnd8);
	DDX_Control(pDX, IDC_WND9, m_cmbWnd9);
	DDX_Control(pDX, IDC_WND10, m_cmbWnd10);
	DDX_Control(pDX, IDC_WND11, m_cmbWnd11);
	DDX_Control(pDX, IDC_WND12, m_cmbWnd12);
	DDX_Control(pDX, IDC_WND13, m_cmbWnd13);
	DDX_Control(pDX, IDC_WND14, m_cmbWnd14);
	DDX_Control(pDX, IDC_WND15, m_cmbWnd15);
	DDX_Control(pDX, IDC_WND16, m_cmbWnd16);
	DDX_Control(pDX, IDC_WND17, m_cmbWnd17);
	DDX_Control(pDX, IDC_WND18, m_cmbWnd18);
	DDX_Control(pDX, IDC_WND19, m_cmbWnd19);
	DDX_Control(pDX, IDC_WND20, m_cmbWnd20);
	DDX_Control(pDX, IDC_WND21, m_cmbWnd21);
	DDX_Control(pDX, IDC_WND22, m_cmbWnd22);
	DDX_Control(pDX, IDC_WND23, m_cmbWnd23);
	DDX_Control(pDX, IDC_WND24, m_cmbWnd24);
	DDX_Control(pDX, IDC_WND25, m_cmbWnd25);
	DDX_Control(pDX, IDC_WND26, m_cmbWnd26);
	DDX_Control(pDX, IDC_WND27, m_cmbWnd27);
	DDX_Control(pDX, IDC_WND28, m_cmbWnd28);
	DDX_Control(pDX, IDC_WND29, m_cmbWnd29);
	DDX_Control(pDX, IDC_WND30, m_cmbWnd30);
	DDX_Control(pDX, IDC_WND31, m_cmbWnd31);
	DDX_Control(pDX, IDC_WND32, m_cmbWnd32);
	DDX_Control(pDX, IDC_WND33, m_cmbWnd33);
	DDX_Control(pDX, IDC_WND34, m_cmbWnd34);
	DDX_Control(pDX, IDC_WND35, m_cmbWnd35);
	DDX_Control(pDX, IDC_WND36, m_cmbWnd36);
	DDX_Control(pDX, IDC_WND37, m_cmbWnd37);
	DDX_Control(pDX, IDC_WND38, m_cmbWnd38);
	DDX_Control(pDX, IDC_WND39, m_cmbWnd39);
	DDX_Control(pDX, IDC_WND40, m_cmbWnd40);
	DDX_Control(pDX, IDC_WND41, m_cmbWnd41);
	DDX_Control(pDX, IDC_WND42, m_cmbWnd42);
	DDX_Control(pDX, IDC_WND43, m_cmbWnd43);
	DDX_Control(pDX, IDC_WND44, m_cmbWnd44);
	DDX_Control(pDX, IDC_WND45, m_cmbWnd45);
	DDX_Control(pDX, IDC_WND46, m_cmbWnd46);
	DDX_Control(pDX, IDC_WND47, m_cmbWnd47);
	DDX_Control(pDX, IDC_WND48, m_cmbWnd48);
	DDX_Control(pDX, IDC_WND49, m_cmbWnd49);
	DDX_Control(pDX, IDC_WND50, m_cmbWnd50);
	DDX_Control(pDX, IDC_WND51, m_cmbWnd51);
	DDX_Control(pDX, IDC_WND52, m_cmbWnd52);
	DDX_Control(pDX, IDC_WND53, m_cmbWnd53);
	DDX_Control(pDX, IDC_WND54, m_cmbWnd54);
	DDX_Control(pDX, IDC_WND55, m_cmbWnd55);
	DDX_Control(pDX, IDC_WND56, m_cmbWnd56);
	DDX_Control(pDX, IDC_WND57, m_cmbWnd57);
	DDX_Control(pDX, IDC_WND58, m_cmbWnd58);
	DDX_Control(pDX, IDC_WND59, m_cmbWnd59);
	DDX_Control(pDX, IDC_WND60, m_cmbWnd60);
	DDX_Control(pDX, IDC_WND61, m_cmbWnd61);
	DDX_Control(pDX, IDC_WND62, m_cmbWnd62);
	DDX_Control(pDX, IDC_WND63, m_cmbWnd63);
	DDX_Control(pDX, IDC_WND64, m_cmbWnd64);
	DDX_Control(pDX, IDC_VIDEO_OUT_TYPE, m_cmbVideoOutType);
	DDX_Text(pDX, IDC_EDIT_GROUP, m_byGroup);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgPreviewSwitchCfg, CDialog)
	//{{AFX_MSG_MAP(CDlgPreviewSwitchCfg)
	ON_BN_CLICKED(IDC_BTN_GET_PREVIEWCFG, OnBtnGetPreviewcfg)
	ON_BN_CLICKED(IDC_BTN_SET_PREVIEWCFG, OnBtnSetPreviewcfg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPreviewSwitchCfg message handlers

BOOL CDlgPreviewSwitchCfg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_cmbVideoOutType.SetCurSel(0);
	m_cmbSwitchTime.SetCurSel(0);
	m_cmbPreviewNumber.SetCurSel(0);
	m_cmbAudioPreview.SetCurSel(0);

	AddChanInfoToCmb(&m_cmbWnd1);
    AddChanInfoToCmb(&m_cmbWnd2);
    AddChanInfoToCmb(&m_cmbWnd3);
    AddChanInfoToCmb(&m_cmbWnd4);
    AddChanInfoToCmb(&m_cmbWnd5);
    AddChanInfoToCmb(&m_cmbWnd6);
    AddChanInfoToCmb(&m_cmbWnd7);
    AddChanInfoToCmb(&m_cmbWnd8);
    AddChanInfoToCmb(&m_cmbWnd9);
	
    AddChanInfoToCmb(&m_cmbWnd10);
    AddChanInfoToCmb(&m_cmbWnd11);
    AddChanInfoToCmb(&m_cmbWnd12);
    AddChanInfoToCmb(&m_cmbWnd13);
    AddChanInfoToCmb(&m_cmbWnd14);
    AddChanInfoToCmb(&m_cmbWnd15);
    AddChanInfoToCmb(&m_cmbWnd16);
    AddChanInfoToCmb(&m_cmbWnd17);
    AddChanInfoToCmb(&m_cmbWnd18);
    AddChanInfoToCmb(&m_cmbWnd19);
	
	AddChanInfoToCmb(&m_cmbWnd20);
    AddChanInfoToCmb(&m_cmbWnd21);
    AddChanInfoToCmb(&m_cmbWnd22);
    AddChanInfoToCmb(&m_cmbWnd23);
    AddChanInfoToCmb(&m_cmbWnd24);
    AddChanInfoToCmb(&m_cmbWnd25);
    AddChanInfoToCmb(&m_cmbWnd26);
    AddChanInfoToCmb(&m_cmbWnd27);
    AddChanInfoToCmb(&m_cmbWnd28);
    AddChanInfoToCmb(&m_cmbWnd29);
	
	AddChanInfoToCmb(&m_cmbWnd30);
    AddChanInfoToCmb(&m_cmbWnd31);
    AddChanInfoToCmb(&m_cmbWnd32);
    AddChanInfoToCmb(&m_cmbWnd33);
    AddChanInfoToCmb(&m_cmbWnd34);
    AddChanInfoToCmb(&m_cmbWnd35);
    AddChanInfoToCmb(&m_cmbWnd36);
    AddChanInfoToCmb(&m_cmbWnd37);
    AddChanInfoToCmb(&m_cmbWnd38);
    AddChanInfoToCmb(&m_cmbWnd39);
	AddChanInfoToCmb(&m_cmbWnd40);
    AddChanInfoToCmb(&m_cmbWnd41);
    AddChanInfoToCmb(&m_cmbWnd42);
    AddChanInfoToCmb(&m_cmbWnd43);
    AddChanInfoToCmb(&m_cmbWnd44);
    AddChanInfoToCmb(&m_cmbWnd45);
    AddChanInfoToCmb(&m_cmbWnd46);
    AddChanInfoToCmb(&m_cmbWnd47);
    AddChanInfoToCmb(&m_cmbWnd48);
    AddChanInfoToCmb(&m_cmbWnd49);
	AddChanInfoToCmb(&m_cmbWnd50);
    AddChanInfoToCmb(&m_cmbWnd51);
    AddChanInfoToCmb(&m_cmbWnd52);
    AddChanInfoToCmb(&m_cmbWnd53);
    AddChanInfoToCmb(&m_cmbWnd54);
    AddChanInfoToCmb(&m_cmbWnd55);
    AddChanInfoToCmb(&m_cmbWnd56);
    AddChanInfoToCmb(&m_cmbWnd57);
    AddChanInfoToCmb(&m_cmbWnd58);
    AddChanInfoToCmb(&m_cmbWnd59);
	
    AddChanInfoToCmb(&m_cmbWnd60);
    AddChanInfoToCmb(&m_cmbWnd61);
    AddChanInfoToCmb(&m_cmbWnd62);
    AddChanInfoToCmb(&m_cmbWnd63);
    AddChanInfoToCmb(&m_cmbWnd64);

	m_cmbWnd1.SetCurSel(0);
	m_cmbWnd2.SetCurSel(0);
	m_cmbWnd3.SetCurSel(0);
	m_cmbWnd4.SetCurSel(0);
	m_cmbWnd5.SetCurSel(0);
	m_cmbWnd6.SetCurSel(0);
	m_cmbWnd7.SetCurSel(0);
	m_cmbWnd8.SetCurSel(0);
	m_cmbWnd9.SetCurSel(0);
	m_cmbWnd10.SetCurSel(0);
	m_cmbWnd11.SetCurSel(0);
	m_cmbWnd12.SetCurSel(0);
	m_cmbWnd13.SetCurSel(0);
	m_cmbWnd14.SetCurSel(0);
	m_cmbWnd15.SetCurSel(0);
	m_cmbWnd16.SetCurSel(0);
	m_cmbWnd17.SetCurSel(0);
	m_cmbWnd18.SetCurSel(0);
	m_cmbWnd19.SetCurSel(0);
	m_cmbWnd20.SetCurSel(0);
	m_cmbWnd21.SetCurSel(0);
	m_cmbWnd22.SetCurSel(0);
	m_cmbWnd23.SetCurSel(0);
	m_cmbWnd24.SetCurSel(0);
	m_cmbWnd25.SetCurSel(0);
	m_cmbWnd26.SetCurSel(0);
	m_cmbWnd27.SetCurSel(0);
	m_cmbWnd28.SetCurSel(0);
	m_cmbWnd29.SetCurSel(0);
	m_cmbWnd30.SetCurSel(0);
	m_cmbWnd31.SetCurSel(0);
	m_cmbWnd32.SetCurSel(0);
	m_cmbWnd33.SetCurSel(0);
	m_cmbWnd34.SetCurSel(0);
	m_cmbWnd35.SetCurSel(0);
	m_cmbWnd36.SetCurSel(0);
	m_cmbWnd37.SetCurSel(0);
	m_cmbWnd38.SetCurSel(0);
	m_cmbWnd39.SetCurSel(0);
	m_cmbWnd40.SetCurSel(0);
	m_cmbWnd41.SetCurSel(0);
	m_cmbWnd42.SetCurSel(0);
	m_cmbWnd43.SetCurSel(0);
	m_cmbWnd44.SetCurSel(0);
	m_cmbWnd45.SetCurSel(0);
	m_cmbWnd46.SetCurSel(0);
	m_cmbWnd47.SetCurSel(0);
	m_cmbWnd48.SetCurSel(0);
	m_cmbWnd49.SetCurSel(0);
	m_cmbWnd50.SetCurSel(0);
	m_cmbWnd51.SetCurSel(0);
	m_cmbWnd52.SetCurSel(0);
	m_cmbWnd53.SetCurSel(0);
	m_cmbWnd54.SetCurSel(0);
	m_cmbWnd55.SetCurSel(0);
	m_cmbWnd56.SetCurSel(0);
	m_cmbWnd57.SetCurSel(0);
	m_cmbWnd58.SetCurSel(0);
	m_cmbWnd59.SetCurSel(0);
	m_cmbWnd60.SetCurSel(0);
	m_cmbWnd61.SetCurSel(0);
	m_cmbWnd62.SetCurSel(0);
	m_cmbWnd63.SetCurSel(0);
	m_cmbWnd64.SetCurSel(0);

	OnBtnGetPreviewcfg();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgPreviewSwitchCfg::OnBtnGetPreviewcfg() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	NET_DVR_PREVIEW_SWITCH_COND struPreviewSwitchCond = {0};
	NET_DVR_PREVIEW_SWITCH_CFG struPreviewSwitchCfg = {0};
	struPreviewSwitchCond.dwSize = sizeof(NET_DVR_PREVIEW_SWITCH_COND);
	struPreviewSwitchCond.byGroup = m_byGroup;
	struPreviewSwitchCond.byVideoOutType = m_cmbVideoOutType.GetCurSel();
	if (!NET_DVR_GetDeviceConfig(m_lUserID, NET_DVR_GET_PREVIEW_SWITCH_CFG, 0, &struPreviewSwitchCond, sizeof(struPreviewSwitchCond), NULL, &struPreviewSwitchCfg, sizeof(struPreviewSwitchCfg)))
	{
		char szLan[256] = {0};
		g_StringLanType(szLan, "��ȡԤ���л�����ʧ��!", "Fail to Get Preview Switch!");
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_FAIL_T, "NET_DVR_GET_PREVIEW_SWITCH_CFG");
		AfxMessageBox(szLan);
		return;
	}
	else
	{
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_SUCC_T, "NET_DVR_GET_PREVIEW_SWITCH_CFG");
	}

	m_cmbPreviewNumber.SetCurSel(struPreviewSwitchCfg.byPreviewNumber);
	m_cmbAudioPreview.SetCurSel(struPreviewSwitchCfg.byEnableAudio);
	m_cmbSwitchTime.SetCurSel(struPreviewSwitchCfg.bySwitchTime);

	//0xFFFF cmbox����0��
// 	for (int i=0; i<64; i++)
// 	{
// 		if (0xFFFF == struPreviewSwitchCfg.wSwitchSeq[i])
// 		{
// 			struPreviewSwitchCfg.wSwitchSeq[i] = 0;
// 		}
// 	}

	AddChanInfoToCmb(&m_cmbWnd1,struPreviewSwitchCfg.wSwitchSeq[0] );
    AddChanInfoToCmb(&m_cmbWnd2,struPreviewSwitchCfg.wSwitchSeq[1] );
    AddChanInfoToCmb(&m_cmbWnd3,struPreviewSwitchCfg.wSwitchSeq[2] );
    AddChanInfoToCmb(&m_cmbWnd4,struPreviewSwitchCfg.wSwitchSeq[3] );
    AddChanInfoToCmb(&m_cmbWnd5,struPreviewSwitchCfg.wSwitchSeq[4] );
    AddChanInfoToCmb(&m_cmbWnd6,struPreviewSwitchCfg.wSwitchSeq[5] );
    AddChanInfoToCmb(&m_cmbWnd7,struPreviewSwitchCfg.wSwitchSeq[6] );
    AddChanInfoToCmb(&m_cmbWnd8,struPreviewSwitchCfg.wSwitchSeq[7] );
    AddChanInfoToCmb(&m_cmbWnd9,struPreviewSwitchCfg.wSwitchSeq[8] );
    AddChanInfoToCmb(&m_cmbWnd10,struPreviewSwitchCfg.wSwitchSeq[9] );
    AddChanInfoToCmb(&m_cmbWnd11,struPreviewSwitchCfg.wSwitchSeq[10] );
    AddChanInfoToCmb(&m_cmbWnd12,struPreviewSwitchCfg.wSwitchSeq[11] );
    AddChanInfoToCmb(&m_cmbWnd13,struPreviewSwitchCfg.wSwitchSeq[12] );
    AddChanInfoToCmb(&m_cmbWnd14,struPreviewSwitchCfg.wSwitchSeq[13] );
    AddChanInfoToCmb(&m_cmbWnd15,struPreviewSwitchCfg.wSwitchSeq[14] );
    AddChanInfoToCmb(&m_cmbWnd16,struPreviewSwitchCfg.wSwitchSeq[15] );
    AddChanInfoToCmb(&m_cmbWnd17,struPreviewSwitchCfg.wSwitchSeq[16] );
    AddChanInfoToCmb(&m_cmbWnd18,struPreviewSwitchCfg.wSwitchSeq[17] );
    AddChanInfoToCmb(&m_cmbWnd19,struPreviewSwitchCfg.wSwitchSeq[18] );
	AddChanInfoToCmb(&m_cmbWnd20,struPreviewSwitchCfg.wSwitchSeq[19] );
	AddChanInfoToCmb(&m_cmbWnd21,struPreviewSwitchCfg.wSwitchSeq[20] );
	AddChanInfoToCmb(&m_cmbWnd22,struPreviewSwitchCfg.wSwitchSeq[21] );
	AddChanInfoToCmb(&m_cmbWnd23,struPreviewSwitchCfg.wSwitchSeq[22] );
	AddChanInfoToCmb(&m_cmbWnd24,struPreviewSwitchCfg.wSwitchSeq[23] );
	AddChanInfoToCmb(&m_cmbWnd25,struPreviewSwitchCfg.wSwitchSeq[24] );
	AddChanInfoToCmb(&m_cmbWnd26,struPreviewSwitchCfg.wSwitchSeq[25] );
	AddChanInfoToCmb(&m_cmbWnd27,struPreviewSwitchCfg.wSwitchSeq[26] );
	AddChanInfoToCmb(&m_cmbWnd28,struPreviewSwitchCfg.wSwitchSeq[27] );
	AddChanInfoToCmb(&m_cmbWnd29,struPreviewSwitchCfg.wSwitchSeq[28] );
	AddChanInfoToCmb(&m_cmbWnd30,struPreviewSwitchCfg.wSwitchSeq[29] );
    AddChanInfoToCmb(&m_cmbWnd31,struPreviewSwitchCfg.wSwitchSeq[30] );
	
    AddChanInfoToCmb(&m_cmbWnd32,struPreviewSwitchCfg.wSwitchSeq[31] );
    AddChanInfoToCmb(&m_cmbWnd33,struPreviewSwitchCfg.wSwitchSeq[32] );
    AddChanInfoToCmb(&m_cmbWnd34,struPreviewSwitchCfg.wSwitchSeq[33] );
    AddChanInfoToCmb(&m_cmbWnd35,struPreviewSwitchCfg.wSwitchSeq[34] );
    AddChanInfoToCmb(&m_cmbWnd36,struPreviewSwitchCfg.wSwitchSeq[35] );
    AddChanInfoToCmb(&m_cmbWnd37,struPreviewSwitchCfg.wSwitchSeq[36] );
    AddChanInfoToCmb(&m_cmbWnd38,struPreviewSwitchCfg.wSwitchSeq[37] );
    AddChanInfoToCmb(&m_cmbWnd39,struPreviewSwitchCfg.wSwitchSeq[38] );
    AddChanInfoToCmb(&m_cmbWnd40,struPreviewSwitchCfg.wSwitchSeq[39] );
    AddChanInfoToCmb(&m_cmbWnd41,struPreviewSwitchCfg.wSwitchSeq[40] );
    AddChanInfoToCmb(&m_cmbWnd42,struPreviewSwitchCfg.wSwitchSeq[41] );
    AddChanInfoToCmb(&m_cmbWnd43,struPreviewSwitchCfg.wSwitchSeq[42] );
    AddChanInfoToCmb(&m_cmbWnd44,struPreviewSwitchCfg.wSwitchSeq[43] );
    AddChanInfoToCmb(&m_cmbWnd45,struPreviewSwitchCfg.wSwitchSeq[44] );
    AddChanInfoToCmb(&m_cmbWnd46,struPreviewSwitchCfg.wSwitchSeq[45] );
    AddChanInfoToCmb(&m_cmbWnd47,struPreviewSwitchCfg.wSwitchSeq[46] );
    AddChanInfoToCmb(&m_cmbWnd48,struPreviewSwitchCfg.wSwitchSeq[47] );
    AddChanInfoToCmb(&m_cmbWnd49,struPreviewSwitchCfg.wSwitchSeq[48] );
    AddChanInfoToCmb(&m_cmbWnd50,struPreviewSwitchCfg.wSwitchSeq[49] );
    AddChanInfoToCmb(&m_cmbWnd51,struPreviewSwitchCfg.wSwitchSeq[50] );
    AddChanInfoToCmb(&m_cmbWnd52,struPreviewSwitchCfg.wSwitchSeq[51] );
    AddChanInfoToCmb(&m_cmbWnd53,struPreviewSwitchCfg.wSwitchSeq[52] );
    AddChanInfoToCmb(&m_cmbWnd54,struPreviewSwitchCfg.wSwitchSeq[53] );
    AddChanInfoToCmb(&m_cmbWnd55,struPreviewSwitchCfg.wSwitchSeq[54] );
    AddChanInfoToCmb(&m_cmbWnd56,struPreviewSwitchCfg.wSwitchSeq[55] );
    AddChanInfoToCmb(&m_cmbWnd57,struPreviewSwitchCfg.wSwitchSeq[56] );
    AddChanInfoToCmb(&m_cmbWnd58,struPreviewSwitchCfg.wSwitchSeq[57] );
    AddChanInfoToCmb(&m_cmbWnd59,struPreviewSwitchCfg.wSwitchSeq[58] );
    AddChanInfoToCmb(&m_cmbWnd60,struPreviewSwitchCfg.wSwitchSeq[59] );
    AddChanInfoToCmb(&m_cmbWnd61,struPreviewSwitchCfg.wSwitchSeq[60] );
    AddChanInfoToCmb(&m_cmbWnd62,struPreviewSwitchCfg.wSwitchSeq[61] );
    AddChanInfoToCmb(&m_cmbWnd63,struPreviewSwitchCfg.wSwitchSeq[62] );
    AddChanInfoToCmb(&m_cmbWnd64,struPreviewSwitchCfg.wSwitchSeq[63] );

/*	m_cmbWnd1.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[0]);
	m_cmbWnd2.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[1]);
	m_cmbWnd3.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[2]);
	m_cmbWnd4.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[3]);
	m_cmbWnd5.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[4]);
	m_cmbWnd6.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[5]);
	m_cmbWnd7.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[6]);
	m_cmbWnd8.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[7]);
	m_cmbWnd9.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[8]);
	m_cmbWnd10.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[9]);
	m_cmbWnd11.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[10]);
	m_cmbWnd12.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[11]);
	m_cmbWnd13.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[12]);
	m_cmbWnd14.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[13]);
	m_cmbWnd15.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[14]);
	m_cmbWnd16.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[15]);
	m_cmbWnd17.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[16]);
	m_cmbWnd18.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[17]);
	m_cmbWnd19.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[18]);
	m_cmbWnd20.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[19]);
	m_cmbWnd21.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[20]);
	m_cmbWnd22.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[21]);
	m_cmbWnd23.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[22]);
	m_cmbWnd24.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[23]);
	m_cmbWnd25.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[24]);
	m_cmbWnd26.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[25]);
	m_cmbWnd27.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[26]);
	m_cmbWnd28.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[27]);
	m_cmbWnd29.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[28]);
	m_cmbWnd30.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[29]);
	m_cmbWnd31.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[30]);
	m_cmbWnd32.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[31]);
	m_cmbWnd33.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[32]);
	m_cmbWnd34.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[33]);
	m_cmbWnd35.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[34]);
	m_cmbWnd36.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[35]);
	m_cmbWnd37.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[36]);
	m_cmbWnd38.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[37]);
	m_cmbWnd39.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[38]);
	m_cmbWnd40.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[39]);
	m_cmbWnd41.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[40]);
	m_cmbWnd42.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[41]);
	m_cmbWnd43.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[42]);
	m_cmbWnd44.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[43]);
	m_cmbWnd45.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[44]);
	m_cmbWnd46.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[45]);
	m_cmbWnd47.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[46]);
	m_cmbWnd48.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[47]);
	m_cmbWnd49.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[48]);
	m_cmbWnd50.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[49]);
	m_cmbWnd51.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[50]);
	m_cmbWnd52.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[51]);
	m_cmbWnd53.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[52]);
	m_cmbWnd54.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[53]);
	m_cmbWnd55.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[54]);
	m_cmbWnd56.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[55]);
	m_cmbWnd57.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[56]);
	m_cmbWnd58.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[57]);
	m_cmbWnd59.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[58]);
	m_cmbWnd60.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[59]);
	m_cmbWnd61.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[60]);
	m_cmbWnd62.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[61]);
	m_cmbWnd63.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[62]);
	m_cmbWnd64.SetCurSel(struPreviewSwitchCfg.wSwitchSeq[63]);
*/
	UpdateData(FALSE);
}

void CDlgPreviewSwitchCfg::OnBtnSetPreviewcfg() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	NET_DVR_PREVIEW_SWITCH_COND struPreviewSwitchCond = {0};
	NET_DVR_PREVIEW_SWITCH_CFG struPreviewSwitchCfg = {0};
	struPreviewSwitchCond.dwSize = sizeof(NET_DVR_PREVIEW_SWITCH_COND);
	struPreviewSwitchCond.byGroup = m_byGroup;
	struPreviewSwitchCond.byVideoOutType = m_cmbVideoOutType.GetCurSel();

	struPreviewSwitchCfg.dwSize = sizeof(NET_DVR_PREVIEW_SWITCH_CFG);
	struPreviewSwitchCfg.byEnableAudio = m_cmbAudioPreview.GetCurSel();
	struPreviewSwitchCfg.byPreviewNumber = m_cmbPreviewNumber.GetCurSel();
	struPreviewSwitchCfg.bySwitchTime = m_cmbSwitchTime.GetCurSel();

	struPreviewSwitchCfg.wSwitchSeq[0] = m_cmbWnd1.GetItemData(m_cmbWnd1.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[1] = m_cmbWnd2.GetItemData(m_cmbWnd2.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[2] = m_cmbWnd3.GetItemData(m_cmbWnd3.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[3] = m_cmbWnd4.GetItemData(m_cmbWnd4.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[4] = m_cmbWnd5.GetItemData(m_cmbWnd5.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[5] = m_cmbWnd6.GetItemData(m_cmbWnd6.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[6] = m_cmbWnd7.GetItemData(m_cmbWnd7.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[7] = m_cmbWnd8.GetItemData(m_cmbWnd8.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[8] = m_cmbWnd9.GetItemData(m_cmbWnd9.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[9] = m_cmbWnd10.GetItemData(m_cmbWnd10.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[10] = m_cmbWnd11.GetItemData(m_cmbWnd11.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[11] = m_cmbWnd12.GetItemData(m_cmbWnd12.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[12] = m_cmbWnd13.GetItemData(m_cmbWnd13.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[13] = m_cmbWnd14.GetItemData(m_cmbWnd14.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[14] = m_cmbWnd15.GetItemData(m_cmbWnd15.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[15] = m_cmbWnd16.GetItemData(m_cmbWnd16.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[16] = m_cmbWnd17.GetItemData(m_cmbWnd17.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[17] = m_cmbWnd18.GetItemData(m_cmbWnd18.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[18] = m_cmbWnd19.GetItemData(m_cmbWnd19.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[19] = m_cmbWnd20.GetItemData(m_cmbWnd20.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[20] = m_cmbWnd21.GetItemData(m_cmbWnd21.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[21] = m_cmbWnd22.GetItemData(m_cmbWnd22.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[22] = m_cmbWnd23.GetItemData(m_cmbWnd23.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[23] = m_cmbWnd24.GetItemData(m_cmbWnd24.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[24] = m_cmbWnd25.GetItemData(m_cmbWnd25.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[25] = m_cmbWnd26.GetItemData(m_cmbWnd26.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[26] = m_cmbWnd27.GetItemData(m_cmbWnd27.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[27] = m_cmbWnd28.GetItemData(m_cmbWnd28.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[28] = m_cmbWnd29.GetItemData(m_cmbWnd29.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[29] = m_cmbWnd30.GetItemData(m_cmbWnd30.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[30] = m_cmbWnd31.GetItemData(m_cmbWnd31.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[31] = m_cmbWnd32.GetItemData(m_cmbWnd32.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[32] = m_cmbWnd33.GetItemData(m_cmbWnd33.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[33] = m_cmbWnd34.GetItemData(m_cmbWnd34.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[34] = m_cmbWnd35.GetItemData(m_cmbWnd35.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[35] = m_cmbWnd36.GetItemData(m_cmbWnd36.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[36] = m_cmbWnd37.GetItemData(m_cmbWnd37.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[37] = m_cmbWnd38.GetItemData(m_cmbWnd38.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[38] = m_cmbWnd39.GetItemData(m_cmbWnd39.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[39] = m_cmbWnd40.GetItemData(m_cmbWnd40.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[40] = m_cmbWnd41.GetItemData(m_cmbWnd41.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[41] = m_cmbWnd42.GetItemData(m_cmbWnd42.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[42] = m_cmbWnd43.GetItemData(m_cmbWnd43.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[43] = m_cmbWnd44.GetItemData(m_cmbWnd44.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[44] = m_cmbWnd45.GetItemData(m_cmbWnd45.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[45] = m_cmbWnd46.GetItemData(m_cmbWnd46.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[46] = m_cmbWnd47.GetItemData(m_cmbWnd47.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[47] = m_cmbWnd48.GetItemData(m_cmbWnd48.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[48] = m_cmbWnd49.GetItemData(m_cmbWnd49.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[49] = m_cmbWnd50.GetItemData(m_cmbWnd50.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[50] = m_cmbWnd51.GetItemData(m_cmbWnd51.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[51] = m_cmbWnd52.GetItemData(m_cmbWnd52.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[52] = m_cmbWnd53.GetItemData(m_cmbWnd53.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[53] = m_cmbWnd54.GetItemData(m_cmbWnd54.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[54] = m_cmbWnd55.GetItemData(m_cmbWnd55.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[55] = m_cmbWnd56.GetItemData(m_cmbWnd56.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[56] = m_cmbWnd57.GetItemData(m_cmbWnd57.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[57] = m_cmbWnd58.GetItemData(m_cmbWnd58.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[58] = m_cmbWnd59.GetItemData(m_cmbWnd59.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[59] = m_cmbWnd60.GetItemData(m_cmbWnd60.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[60] = m_cmbWnd61.GetItemData(m_cmbWnd61.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[61] = m_cmbWnd62.GetItemData(m_cmbWnd62.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[62] = m_cmbWnd63.GetItemData(m_cmbWnd63.GetCurSel());
	struPreviewSwitchCfg.wSwitchSeq[63] = m_cmbWnd64.GetItemData(m_cmbWnd64.GetCurSel());

	//0xFFFF cmbox����0��
	for (int i=0; i<64; i++)
	{
		if (0 == struPreviewSwitchCfg.wSwitchSeq[i])
		{
			struPreviewSwitchCfg.wSwitchSeq[i] = 0xFFFF;
		}
	}

	if (!NET_DVR_SetDeviceConfig(m_lUserID, NET_DVR_SET_PREVIEW_SWITCH_CFG, 0, &struPreviewSwitchCond, sizeof(struPreviewSwitchCond), NULL, &struPreviewSwitchCfg, sizeof(struPreviewSwitchCfg)))
	{
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_FAIL_T, "NET_DVR_SET_PREVIEW_SWITCH_CFG");
		char szLan[256] = {0};
		g_StringLanType(szLan, "����Ԥ���л�����ʧ��!", "Fail to Set Preview Switch!");
		AfxMessageBox(szLan);
		return;
	}
	else
	{
		g_pMainDlg->AddLog(m_dwDevIndex, OPERATION_SUCC_T, "NET_DVR_SET_PREVIEW_SWITCH_CFG");	
	}
}

void CDlgPreviewSwitchCfg::AddChanInfoToCmb(CComboBox* cmbWnd1)
{
    int i = 0;
    CString strTemp =_T("");
	CString strChanTmp = _T("");
	
    DWORD dwCmbIndex = 0;
    
    cmbWnd1->ResetContent();
    strTemp.Format("%s", "0xFFFF");
    cmbWnd1->AddString(strTemp);
    cmbWnd1->SetItemData(dwCmbIndex,65535);
    
    dwCmbIndex++;
    for(i = 0; i < g_struDeviceInfo[m_dwDevIndex].iDeviceChanNum ; i++)
    {
        if(i < g_struDeviceInfo[m_dwDevIndex].iAnalogChanNum)
        {
			//             if (g_struDeviceInfo[m_dwDevIndex].pStruChanInfo[i].bEnable)
			//             {
			strTemp.Format(ANALOG_C_FORMAT, i+1);   
            //}
        }
        else
        {
			//             if (g_struDeviceInfo[m_dwDevIndex].pStruChanInfo[i].bEnable)
			//             {
			//                 
			strTemp.Format(IP_CAMERA_NAME, i - g_struDeviceInfo[m_dwDevIndex].iAnalogChanNum + g_struDeviceInfo[m_dwDevIndex].iStartChan);
			/*    }*/
        }
        
        cmbWnd1->AddString(strTemp);
        cmbWnd1->SetItemData(dwCmbIndex,g_struDeviceInfo[m_dwDevIndex].pStruChanInfo[i].iChannelNO);
        dwCmbIndex++;
    }
}

void CDlgPreviewSwitchCfg::AddChanInfoToCmb(CComboBox* cmbWnd, DWORD dwItemData)
{
    for (int i=0;i < cmbWnd->GetCount();i++)
    {
        if (cmbWnd->GetItemData(i) == dwItemData)
        {
            cmbWnd->SetCurSel(i);
        }
    }
	
    return ;
}