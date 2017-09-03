/**********************************************************
FileName:    ClientDemoDlg.cpp
Description: main dialog, show the main function      
Date:        2008/12/12
Note: 		the global Macro definition and structure is in "GeneralDef.h", global variable and function is in "ClientDemo.cpp"
Modification History:      
    <version> <time>         <desc>
    <1.0    > <2008/12/12>       <created>
***********************************************************/

#include "stdafx.h"
#include "string.h"
#include "ClientDemo.h"
#include ".\clientdemodlg.h"
#include "DlgAddDevice.h"
#include "DlgRemoteCfg.h"
#include "math.h"
#include "winver.h"
#include "DlgDeviceInfo.h"
#include "DlgUpgrade.h"
#include "DlgFormatHD.h"
#include "DlgRemoteKey.h"
#include "DlgDeviceState.h"
#include "DlgLocalLogConfig.h"
#include "DlgJPEGSnatchConfig.h"
#include "DlgBroadCast.h"
#include "DlgHardDiskCfg.h"
#include "DlgRemoteIPAccess.h"
#include "DlgVoiceTalk.h"
#include "DlgFortify.h"
#include "DlgLogSearch.h"
#include "DlgDeviceState.h"
#include "DlgCheckTime.h"
#include "DlgSerialTrans.h"
#include "DlgConfigFlash.h"
#include "DlgDoTest.h"
#include "process.h"
#include "DlgAtmFrameFormatV30.h"
#include "DlgInquestDvr.h"
#include "DlgPtzScope.h"
#include "DlgSensorAddPreview.h"
#include "DlgHolidayCfg.h"
#include "DlgNetServiceCfg.h"
#include "DlgCertInfo.h"
#include "DlgT1test.h"
#include "DlgDevServer.h"
#include "DlgProductionTest.h"
#include "DlgVedioAudioInCfg.h"
#include "DlgProductCfg.h"
#include "DlgVCADetionCfg.h"
#include "DlgTalkMREx.h"


using namespace std;

#pragma   comment   (lib,   "version.lib")  
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
static unsigned long ulAlarmPic = 0;

extern HINSTANCE g_hDSSDK;
CRITICAL_SECTION g_cs;

//Alarm host
typedef struct tagLOCAL_ALARM_INFO
{
	int iDeviceIndex;
	LONG lCommand;
	tagLOCAL_ALARM_INFO()
	{
		iDeviceIndex = -1;
		lCommand = -1;
	}
}LOCAL_ALARM_INFO, *LPLOCAL_ALARM_INFO;

/*********************************************************
Function:	g_ExceptionCallBack
Desc:		exception callback function
Input:	dwType: message type;lUserID: log-in device ID;lHandle, handle of connected channel ; pUser, user data��
Output:	
Return:	
**********************************************************/
void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
	UNREFERENCED_PARAMETER(pUser);
	if (g_bExitDemo)
	{
		return;
	}
	int i = 0, j=0;
	int iDeviceIndex = -1;
	CString sTemp;
	//	char szTmpBuf[256] = {0};
	
	for (i = 0; i < MAX_DEVICES ; i++)
	{				
		if (g_struDeviceInfo[i].lLoginID == lUserID)
		{
			iDeviceIndex = i;		
			break;
		}
	}
	
	switch (dwType) 
	{
	case EXCEPTION_EXCHANGE://the device is off line
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "EXCEPTION_EXCHANGE");
		break;
    case RESUME_EXCHANGE://the device is on line
        g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "RESUME_EXCHANGE");
		break;
	case EXCEPTION_AUDIOEXCHANGE:	//network exception while voice talk
		if (g_lVoiceHandle != -1)
		{	
			if (lHandle == g_lVoiceHandle)
			{//sdk do not reconnect, so close the exception thread
				g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "EXCEPTION_AUDIOEXCHANGE vt dev[%d]", iDeviceIndex);
				goto ExceptionOut;
			}			
		}
		else
		{
			for (i = 0; i<MAX_DEVICES; i++)
			{
				for (j = 0; j<MAX_AUDIO_V40; j++)
				{
					if (g_struDeviceInfo[i].lVoiceCom[j] == lHandle)
					{
						g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "AUDIOEXCHANGE mr dev[%d]exception[%d]", iDeviceIndex, dwType);
						if (NET_DVR_StopVoiceCom(lHandle))
						{
							g_pMainDlg->AddLog(i, OPERATION_SUCC_T, "NET_DVR_StopVoiceCom audioindex[%d]", j);
							g_struDeviceInfo[i].lVoiceCom[j] = -1;
						}
						else
						{
							g_pMainDlg->AddLog(i, OPERATION_FAIL_T, "NET_DVR_StopVoiceCom audioindex[%d]", j);
						}
						goto ExceptionOut;
					}
				}
			}
		}	
		if (g_struLocalParam.bBroadCast)
		{
			g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "AUDIOEXCHANGE BC dev[%d]handle[%d]exception[%d]", iDeviceIndex, lHandle, dwType);
			NET_DVR_DelDVR_V30(lHandle);
		}

		break;
	case EXCEPTION_ALARM:			//network exception while uploading alarm
		for (i = 0; i < MAX_DEVICES; i++)
		{				
			if (g_struDeviceInfo[i].lFortifyHandle >= 0)
			{
				if (lHandle == g_struDeviceInfo[i].lFortifyHandle)
				{
					//	if (NET_DVR_CloseAlarmChan_V30(g_struDeviceInfo[i].lFortifyHandle))
					//	{
					g_pMainDlg->AddLog(i, OPERATION_FAIL_T, "alarm fortify exception!");
					//		g_struDeviceInfo[i].lFortifyHandle = -1;
					//	}
					//	else
					//	{
					
					//	}			
					goto ExceptionOut;
				}
			}
		}	
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "ALARM dev[%d]exception[%d]", iDeviceIndex, dwType);
		break;
	case EXCEPTION_PREVIEW:			// exception while preview
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "PREVIEW dev[%d]exception[%d]", iDeviceIndex, dwType);
		for (i = 0; i < MAX_OUTPUTS; i++)
		{
			if (lHandle == g_dlgOutput[i].m_lPlayHandle)
			{
				g_dlgOutput[i].AddLog(OPERATION_FAIL_T, "preview exception!");
				if (g_dlgOutput[i].m_bLocalManualRec)
				{
					g_dlgOutput[i].StopLocalRecord();
				}
				g_dlgOutput[i].StopPlay();
				g_pMainDlg->ChangePlayBtnState();
				goto ExceptionOut;
			}
		}
		break;
	case EXCEPTION_SERIAL:			//exception while connecting in a transparent channel mode
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "EXCEPTION_SERIAL dev[%d]", iDeviceIndex);
		for (i = 0; i < MAX_DEVICES ; i++)
		{				
			if (g_struDeviceInfo[i].lTranHandle >= 0)
			{
				if (lHandle == g_struDeviceInfo[i].lTranHandle)
				{
					g_pMainDlg->AddLog(i, OPERATION_FAIL_T, "serial transparent exception!!");		
					goto ExceptionOut;
				}
			}
		}
		break;
	case EXCEPTION_RECONNECT:		//reconnect while preview	
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "preview RECONNECT dev[%d]", iDeviceIndex);
		for (i = 0; i < MAX_OUTPUTS; i++)
		{
			if (lHandle == g_dlgOutput[i].m_lPlayHandle)
			{
				g_dlgOutput[i].AddLog(OPERATION_SUCC_T, "preview reconneting!");//reconnect to network while preview
				goto ExceptionOut;
			}	
		}
		break;
	case PREVIEW_RECONNECTSUCCESS:		//reconnect successfully while preview				
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "PREVIEW_RECONNECTSUCCESS dev[%d]", iDeviceIndex);
		break;
	case EXCEPTION_ALARMRECONNECT://reconnect alarm guard channel
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "ALARMRECONNECT dev[%d]", iDeviceIndex);
		for (i = 0; i < MAX_DEVICES ; i++)
		{				
			if (g_struDeviceInfo[i].lFortifyHandle >= 0)
			{
				if (lHandle == g_struDeviceInfo[i].lFortifyHandle)
				{
					g_pMainDlg->AddLog(i, OPERATION_SUCC_T, "alarm Fortify reconnecting!!");		
					goto ExceptionOut;
				}
			}
		}
		break;
	case ALARM_RECONNECTSUCCESS://reconnect alarm guard channel successfully
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "ALARM_RECONNECTSUCCESS dev[%d]", iDeviceIndex);
		break;
	case EXCEPTION_SERIALRECONNECT://reconnect transparent channel
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "SERIALRECONNECT dev[%d]exception[0x%x]", iDeviceIndex, dwType);
		for (i = 0; i < MAX_DEVICES ; i++)
		{				
			if (g_struDeviceInfo[i].lLoginID == lUserID)
			{
				g_pMainDlg->AddLog(i, OPERATION_SUCC_T, "serial transparent reconnecting!");		
				goto ExceptionOut;
			}
		}
		break;
	case EXCEPTION_PLAYBACK:
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "EXCEPTION_PLAYBACK dev[%d] exception[0x%x] error[%d]", iDeviceIndex, dwType, NET_DVR_GetLastError());
		break;
	case EXCEPTION_DISKFMT:
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "EXCEPTION_DISKFMT dev[%d]DISKFMT exception[0x%x]", iDeviceIndex, dwType);
		break;
    case EXCEPTION_PASSIVEDECODE:
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "EXCEPTION_PASSIVEDECODE dev[%d]PASSIVEDECODE exception[0x%x]", iDeviceIndex, dwType);		
        //NET_DVR_MatrixStopPassiveDecode(lHandle);

		break;
	case SERIAL_RECONNECTSUCCESS:
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "SERIAL_RECONNECTSUCCESS dev[%d]", iDeviceIndex);
		break;
	case EXCEPTION_LOST_ALARM:
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "EXCEPTION_LOST_ALARM");
		break;
	case EXCEPTION_MAX_ALARM_INFO:
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "EXCEPTION_MAX_ALARM_INFO");
		break;
	default:
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "else exception dev[%d]exception[0x%x]", iDeviceIndex, dwType);
		break;
	}
ExceptionOut:
	return;
}

/*********************************************************
Function:	MessageCallback
Desc:		alarm, callback functions dealing with the info of card in ATM
Input:	lCommand,message type; pAlarmer,info of device which upload message;pAlarmInfo, message info content;dwBufLen,message length; pUser, user parameter
Output:	
Return:	
**********************************************************/
void CALLBACK MessageCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser)
{
	//Sleep(100000);
	UNREFERENCED_PARAMETER(pUser);	
	UINT iDeviceIndex = 0xffff;
	UINT i = 0;
	if (g_pMainDlg == NULL || g_bExitDemo)
	{
		TRACE("exit ...\n");
		//return;
	}
	 
	for (i=0; i<MAX_DEVICES; i++)
	{//IP address that client have acquired
		if (strcmp(g_struDeviceInfo[i].chDeviceIP, pAlarmer->sDeviceIP) == 0)
		{
			iDeviceIndex = i;
			break;
		}
	}
	


	if (iDeviceIndex >= MAX_DEVICES || iDeviceIndex < 0)
	{
		g_pMainDlg->AddLog(-1, ALARM_INFO_T, "alarm host get alarm[%x] from device[%s]", lCommand, pAlarmer->sDeviceIP);
		return;
	}
	else
	{
		g_pMainDlg->AddLog(-1, ALARM_INFO_T, "alarm host get alarm[%x] from device[%s] name[%s] mac:%x:%x:%x:%x:%x:%x serial:%s", \
			lCommand, pAlarmer->sDeviceIP, pAlarmer->sDeviceName, pAlarmer->byMacAddr[0], pAlarmer->byMacAddr[1], pAlarmer->byMacAddr[2],\
			pAlarmer->byMacAddr[3],  pAlarmer->byMacAddr[4],  pAlarmer->byMacAddr[5],
			pAlarmer->sSerialNumber);
	}
	try
	{
		char *pAlarmMsg = new char[dwBufLen];
		if (pAlarmMsg == NULL)
		{
			return;
		}
		memcpy(pAlarmMsg, pAlarmInfo, dwBufLen);
		if (lCommand == COMM_ALARM_RULE)
		{
			((LPNET_VCA_RULE_ALARM)pAlarmMsg)->pImage = new BYTE[((LPNET_VCA_RULE_ALARM)pAlarmMsg)->dwPicDataLen];
			memset(((LPNET_VCA_RULE_ALARM)pAlarmMsg)->pImage, 0, ((LPNET_VCA_RULE_ALARM)pAlarmMsg)->dwPicDataLen);
			memcpy(((LPNET_VCA_RULE_ALARM)pAlarmMsg)->pImage, ((LPNET_VCA_RULE_ALARM)pAlarmInfo)->pImage, ((LPNET_VCA_RULE_ALARM)pAlarmMsg)->dwPicDataLen);
		}
        else if (COMM_ALARM_AID == lCommand)
        {
            ((LPNET_DVR_AID_ALARM)pAlarmMsg)->pImage = new BYTE[((LPNET_DVR_AID_ALARM)pAlarmMsg)->dwPicDataLen];
            memset(((LPNET_DVR_AID_ALARM)pAlarmMsg)->pImage, 0, ((LPNET_DVR_AID_ALARM)pAlarmMsg)->dwPicDataLen);
            memcpy(((LPNET_DVR_AID_ALARM)pAlarmMsg)->pImage, ((LPNET_DVR_AID_ALARM)pAlarmInfo)->pImage, ((LPNET_DVR_AID_ALARM)pAlarmMsg)->dwPicDataLen);
        }
        else if (COMM_ALARM_FACE == lCommand)
        {
            ((LPNET_DVR_FACEDETECT_ALARM)pAlarmMsg)->pImage = new BYTE[((LPNET_DVR_FACEDETECT_ALARM)pAlarmMsg)->dwPicDataLen];
            memset(((LPNET_DVR_FACEDETECT_ALARM)pAlarmMsg)->pImage, 0, ((LPNET_DVR_FACEDETECT_ALARM)pAlarmMsg)->dwPicDataLen);
            memcpy(((LPNET_DVR_FACEDETECT_ALARM)pAlarmMsg)->pImage, ((LPNET_DVR_FACEDETECT_ALARM)pAlarmInfo)->pImage, ((LPNET_DVR_FACEDETECT_ALARM)pAlarmMsg)->dwPicDataLen);

			((LPNET_DVR_FACEDETECT_ALARM)pAlarmMsg)->pFaceImage = new BYTE[((LPNET_DVR_FACEDETECT_ALARM)pAlarmMsg)->dwFacePicDataLen];
            memset(((LPNET_DVR_FACEDETECT_ALARM)pAlarmMsg)->pFaceImage, 0, ((LPNET_DVR_FACEDETECT_ALARM)pAlarmMsg)->dwFacePicDataLen);
            memcpy(((LPNET_DVR_FACEDETECT_ALARM)pAlarmMsg)->pFaceImage, ((LPNET_DVR_FACEDETECT_ALARM)pAlarmInfo)->pFaceImage, ((LPNET_DVR_FACEDETECT_ALARM)pAlarmMsg)->dwFacePicDataLen);

        }
		else if (COMM_UPLOAD_FACESNAP_RESULT == lCommand)
		{
            ((LPNET_VCA_FACESNAP_RESULT)pAlarmMsg)->pBuffer1 = new BYTE[((LPNET_VCA_FACESNAP_RESULT)pAlarmMsg)->dwFacePicLen];
            memset(((LPNET_VCA_FACESNAP_RESULT)pAlarmMsg)->pBuffer1, 0, ((LPNET_VCA_FACESNAP_RESULT)pAlarmMsg)->dwFacePicLen);
            memcpy(((LPNET_VCA_FACESNAP_RESULT)pAlarmMsg)->pBuffer1, ((LPNET_VCA_FACESNAP_RESULT)pAlarmInfo)->pBuffer1, ((LPNET_VCA_FACESNAP_RESULT)pAlarmMsg)->dwFacePicLen);

			((LPNET_VCA_FACESNAP_RESULT)pAlarmMsg)->pBuffer2 = new BYTE[((LPNET_VCA_FACESNAP_RESULT)pAlarmMsg)->dwBackgroundPicLen];
			memset(((LPNET_VCA_FACESNAP_RESULT)pAlarmMsg)->pBuffer2, 0, ((LPNET_VCA_FACESNAP_RESULT)pAlarmMsg)->dwBackgroundPicLen);
			memcpy(((LPNET_VCA_FACESNAP_RESULT)pAlarmMsg)->pBuffer2, ((LPNET_VCA_FACESNAP_RESULT)pAlarmInfo)->pBuffer2, ((LPNET_VCA_FACESNAP_RESULT)pAlarmMsg)->dwBackgroundPicLen);

		}
		else if (COMM_SNAP_MATCH_ALARM == lCommand)
		{
            ((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmMsg)->struSnapInfo.pBuffer1 = new BYTE[((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmMsg)->struSnapInfo.dwSnapFacePicLen];
            memset(((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmMsg)->struSnapInfo.pBuffer1, 0, ((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmMsg)->struSnapInfo.dwSnapFacePicLen);
            memcpy(((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmMsg)->struSnapInfo.pBuffer1, ((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmInfo)->struSnapInfo.pBuffer1, ((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmMsg)->struSnapInfo.dwSnapFacePicLen);
			
			((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmMsg)->struBlackListInfo.pBuffer1 = new BYTE[((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmMsg)->struBlackListInfo.dwBlackListPicLen];
            memset(((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmMsg)->struBlackListInfo.pBuffer1, 0, ((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmMsg)->struBlackListInfo.dwBlackListPicLen);
            memcpy(((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmMsg)->struBlackListInfo.pBuffer1, ((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmInfo)->struBlackListInfo.pBuffer1, ((LPNET_VCA_FACESNAP_MATCH_ALARM)pAlarmMsg)->struBlackListInfo.dwBlackListPicLen);
		}
		else if (COMM_ITS_PLATE_RESULT == lCommand)
		{
			int i = 0;
			DWORD dwPrePicLen = 0;
			for (i = 0; i < 6; i++)
			{
				if (((LPNET_ITS_PLATE_RESULT)pAlarmMsg)->struPicInfo[i].dwDataLen > 0)
				{
					((LPNET_ITS_PLATE_RESULT)pAlarmMsg)->struPicInfo[i].pBuffer = (BYTE*)pAlarmMsg + sizeof(NET_ITS_PLATE_RESULT) + dwPrePicLen;
					dwPrePicLen += ((LPNET_ITS_PLATE_RESULT)pAlarmMsg)->struPicInfo[i].dwDataLen;
				}
			}
		}
		else if (COMM_ITS_GATE_VEHICLE == lCommand)
		{
			int i = 0;
			DWORD dwPrePicLen = 0;
			for (i = 0; i < 4; i++)
			{
				if (((LPNET_ITS_GATE_VEHICLE)pAlarmMsg)->struPicInfo[i].dwDataLen > 0)
				{	
					((LPNET_ITS_GATE_VEHICLE)pAlarmMsg)->struPicInfo[i].pBuffer = (BYTE*)pAlarmMsg + sizeof(NET_ITS_GATE_VEHICLE) + dwPrePicLen;
					dwPrePicLen += ((LPNET_ITS_GATE_VEHICLE)pAlarmMsg)->struPicInfo[i].dwDataLen;
				}	
			}
		}
		else if(COMM_ITS_GATE_FACE == lCommand)
		{
			((LPNET_ITS_GATE_FACE)pAlarmMsg)->struFaceInfo.pBuffer1 = new BYTE[((LPNET_ITS_GATE_FACE)pAlarmMsg)->struFaceInfo.dwFacePicLen];
            memset(((LPNET_ITS_GATE_FACE)pAlarmMsg)->struFaceInfo.pBuffer1, 0, ((LPNET_ITS_GATE_FACE)pAlarmMsg)->struFaceInfo.dwFacePicLen);
            memcpy(((LPNET_ITS_GATE_FACE)pAlarmMsg)->struFaceInfo.pBuffer1, ((LPNET_ITS_GATE_FACE)pAlarmInfo)->struFaceInfo.pBuffer1, \
				((LPNET_ITS_GATE_FACE)pAlarmMsg)->struFaceInfo.dwFacePicLen);
			
			((LPNET_ITS_GATE_FACE)pAlarmMsg)->struFaceInfo.pBuffer2 = new BYTE[((LPNET_ITS_GATE_FACE)pAlarmMsg)->struFaceInfo.dwBackgroundPicLen];
			memset(((LPNET_ITS_GATE_FACE)pAlarmMsg)->struFaceInfo.pBuffer2, 0, ((LPNET_ITS_GATE_FACE)pAlarmMsg)->struFaceInfo.dwBackgroundPicLen);
			memcpy(((LPNET_ITS_GATE_FACE)pAlarmMsg)->struFaceInfo.pBuffer2, ((LPNET_ITS_GATE_FACE)pAlarmInfo)->struFaceInfo.pBuffer2, \
				((LPNET_ITS_GATE_FACE)pAlarmMsg)->struFaceInfo.dwBackgroundPicLen);
			
		}
		else if(COMM_ITS_PARK_VEHICLE == lCommand)
		{
			int i = 0;
			DWORD dwPrePicLen = 0;
			for (i = 0; i < 2; i++)
			{
				if (((LPNET_ITS_PARK_VEHICLE)pAlarmMsg)->struPicInfo[i].dwDataLen > 0)
				{
					((LPNET_ITS_PARK_VEHICLE)pAlarmMsg)->struPicInfo[i].pBuffer = (BYTE*)pAlarmMsg + sizeof(NET_ITS_PARK_VEHICLE) + dwPrePicLen;
					dwPrePicLen += ((LPNET_ITS_PARK_VEHICLE)pAlarmMsg)->struPicInfo[i].dwDataLen; 
				}
			}
		}
		else if (COMM_ALARM_AID_V41 == lCommand)
		{
			((LPNET_DVR_AID_ALARM_V41)pAlarmMsg)->pImage = new BYTE[((LPNET_DVR_AID_ALARM_V41)pAlarmMsg)->dwPicDataLen];
            memset(((LPNET_DVR_AID_ALARM_V41)pAlarmMsg)->pImage, 0, ((LPNET_DVR_AID_ALARM_V41)pAlarmMsg)->dwPicDataLen);
            memcpy(((LPNET_DVR_AID_ALARM_V41)pAlarmMsg)->pImage, ((LPNET_DVR_AID_ALARM_V41)pAlarmInfo)->pImage, ((LPNET_DVR_AID_ALARM_V41)pAlarmMsg)->dwPicDataLen);
		}
		else if (COMM_ALARM_TFS == lCommand)
		{
			int i = 0;
			DWORD dwPrePicLen = 0;
			for (i = 0; i < 8; i++)
			{
				if (((LPNET_DVR_TFS_ALARM)pAlarmMsg)->struPicInfo[i].dwDataLen > 0)
				{
					((LPNET_DVR_TFS_ALARM)pAlarmMsg)->struPicInfo[i].pBuffer = (BYTE*)pAlarmMsg + sizeof(NET_DVR_TFS_ALARM) + dwPrePicLen;
					dwPrePicLen += ((LPNET_DVR_TFS_ALARM)pAlarmMsg)->struPicInfo[i].dwDataLen;
				}
			}
		}
		else if (COMM_ALARM_VQD_EX == lCommand)
		{   
			if (((LPNET_DVR_VQD_ALARM)pAlarmMsg)->dwPicDataLen > 0)
			{
				((LPNET_DVR_VQD_ALARM)pAlarmMsg)->pImage = new BYTE[((LPNET_DVR_VQD_ALARM)pAlarmMsg)->dwPicDataLen];
				memset(((LPNET_DVR_VQD_ALARM)pAlarmMsg)->pImage, 0, ((LPNET_DVR_VQD_ALARM)pAlarmMsg)->dwPicDataLen);
				memcpy(((LPNET_DVR_VQD_ALARM)pAlarmMsg)->pImage, ((LPNET_DVR_VQD_ALARM)pAlarmInfo)->pImage, ((LPNET_DVR_VQD_ALARM)pAlarmMsg)->dwPicDataLen);
			}
		}
		else if (COMM_UPLOAD_HEATMAP_RESULT == lCommand)
		{
			if ((((LPNET_DVR_HEATMAP_RESULT)pAlarmMsg)->wArrayColumn > 0) || (((LPNET_DVR_HEATMAP_RESULT)pAlarmMsg)->wArrayLine > 0))
			{ 
				int iLen = (((LPNET_DVR_HEATMAP_RESULT)pAlarmMsg)->wArrayColumn) * (((LPNET_DVR_HEATMAP_RESULT)pAlarmMsg)->wArrayLine);
				((LPNET_DVR_HEATMAP_RESULT)pAlarmMsg)->pBuffer = new BYTE[iLen];
				memset(((LPNET_DVR_HEATMAP_RESULT)pAlarmMsg)->pBuffer, 0, iLen);
				memcpy(((LPNET_DVR_HEATMAP_RESULT)pAlarmMsg)->pBuffer, ((LPNET_DVR_HEATMAP_RESULT)pAlarmInfo)->pBuffer, iLen);
			}
		}
		LPLOCAL_ALARM_INFO pAlarmDev = new LOCAL_ALARM_INFO;
		if (pAlarmDev == NULL)
		{
			delete [] pAlarmMsg;
			pAlarmMsg = NULL;
			return;
		}
		pAlarmDev->iDeviceIndex = iDeviceIndex;
		pAlarmDev->lCommand = lCommand;

		//the memery may be free.
		//g_pMainDlg->ProcDeviceAlarm((DWORD)pAlarmDev, (LONG)pAlarmMsg);

		::PostMessage(g_pMainDlg->m_hWnd, WM_PROC_ALARM, (DWORD)pAlarmDev, (LONG)pAlarmMsg);	
	}
	catch (...)
	{
		OutputDebugString("New Alarm Exception!\n");
	}
	
	return;
}

/*********************************************************
Function:	CClientDemoDlg
Desc:		Constructor
Input:	
Output:	
Return:	
**********************************************************/
IMPLEMENT_DYNAMIC(CClientDemoDlg, CDialog)
CClientDemoDlg::CClientDemoDlg(CWnd* pParent /*=NULL*/)
: CDialog(CClientDemoDlg::IDD, pParent)
, m_iCurDeviceIndex(-1)
, m_iCurChanIndex(-1)
, m_iCurWndIndex(0)//select current Output
, m_iMainType(PREVIEW_T)
, m_hCurDeviceItem(NULL)
, m_hCurChanItem(NULL)
, m_iCurRecDriver(0)
, m_pAutoRecTimer(NULL)
, m_pChkDevTimer(NULL)
, m_pCyclePlayTimer(NULL)
, m_pFreeSpaceTimer(NULL)
, m_pFileIntervalTimer(NULL)
, m_pSysTimer(NULL)
, m_iCardChanCount(0)
, m_iDriverNum(0)
, m_iTreeWidth(0)
, m_iRightWidth(0)
, m_iFunBtnHeight(0)
, m_iListLogHeight(0)
, m_bDrag(FALSE)
, m_iDragType(0)
, m_iDragChanIndex(-1)
, m_iDragParentIndex(-1)
, m_iCurWndNum(4)
, m_bListLogEnlarge(FALSE)
, m_bListAlarmEnlarge(FALSE)
, m_lListenHandle(-1)
, m_fLog(NULL)
, m_fAlarm(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    
	memset(&m_cHardDriver[0], 0, 26);
	memset(&m_struOldWndpl, 0, sizeof(WINDOWPLACEMENT));
    memset(&m_struSensorAlarm, 0, sizeof(m_struSensorAlarm));
	InitializeCriticalSection(&m_struLock);  //most thread might use log print function, especially alarm part, needing to add a lock
    InitializeCriticalSection(&g_cs);
	m_bSupportChanEx = FALSE;
	memset(&m_struVcaCtrlInfo, 0, sizeof(m_struVcaCtrlInfo));
	memset(&m_struIFrame, 0, sizeof(m_struIFrame));
}

/*********************************************************
Function:	~CClientDemoDlg
Desc:		destructor
Input:	
Output:	
Return:	
**********************************************************/
CClientDemoDlg::~CClientDemoDlg()
{
	if (m_fLog != NULL)
	{
		fclose(m_fLog);
	}
	if (m_fAlarm!= NULL)
	{
		fclose(m_fAlarm);
	}
	m_hIcon = NULL;
	m_hCurDeviceItem = NULL;
	m_hCurChanItem = NULL;
	g_pMainDlg = NULL;
	
	delete m_dlgRemoteCfg;
	
	delete m_dlgLocalCfg;
	
	delete m_dlgIPAccess;
	
	delete m_dlgHDConfig;
	
	delete m_dlgConfigAll;
	
	delete m_dlgOutputCtrl;
	
	delete m_dlgPlayBack;

	//delete m_pStreamIDDlg;

	delete m_dlgBatchCfg;

	delete m_dlgProductCfg;

	for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (g_struDeviceInfo[i].pStruChanInfo != NULL)
        {
            delete[] g_struDeviceInfo[i].pStruChanInfo;
            g_struDeviceInfo[i].pStruChanInfo = NULL;
        }
		
        if (g_struDeviceInfo[i].pStruIPParaCfgV40 != NULL)
        {
            delete[] g_struDeviceInfo[i].pStruIPParaCfgV40;
            g_struDeviceInfo[i].pStruIPParaCfgV40 = NULL;
        }     
        
        if (g_struDeviceInfo[i].pStruIPAlarmInCfgV40 != NULL)
        {
            delete[]  g_struDeviceInfo[i].pStruIPAlarmInCfgV40 ;
            g_struDeviceInfo[i].pStruIPAlarmInCfgV40 = NULL;
        }
        
        if (g_struDeviceInfo[i].pStruIPAlarmOutCfgV40 != NULL)
        {
            delete[] g_struDeviceInfo[i].pStruIPAlarmOutCfgV40;
            g_struDeviceInfo[i].pStruIPAlarmOutCfgV40 = NULL;
        }
		
		
    }
    
	DeleteCriticalSection(&g_cs);
	DeleteCriticalSection(&m_struLock);
}

/*********************************************************
Function:	DoDataExchange
Desc:		the map between control and variable
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClientDemoDlg)
	DDX_Control(pDX, IDC_BTN_PLAY, m_btnPlay);
	DDX_Control(pDX, IDC_BTN_RECORD, m_btnRecord);
	DDX_Control(pDX, IDC_TREE_DEVICE_LIST, m_treeDeviceList);
	DDX_Control(pDX, IDC_COMBO_WNDNUM, m_comboWinNum);
	DDX_Control(pDX, IDC_COMBO_NETDELAY, m_comboNetDelayBufNum);
	DDX_Control(pDX, IDC_LIST_ALL_LOG, m_listAllLog);
	DDX_Control(pDX, IDC_LIST_ALARM_INFO, m_listAlarmInfo);
	//}}AFX_DATA_MAP
}

/*********************************************************
Function:	BEGIN_MESSAGE_MAP
Desc:		the map between control and function
Input:	
Output:	
Return:	
**********************************************************/
BEGIN_MESSAGE_MAP(CClientDemoDlg, CDialog)
//{{AFX_MSG_MAP(CClientDemoDlg)
ON_WM_TIMER()
ON_WM_MOVE()
ON_NOTIFY(NM_CLICK, IDC_TREE_DEVICE_LIST, OnNMClickTreeDeviceList)
ON_NOTIFY(NM_RCLICK, IDC_TREE_DEVICE_LIST, OnNMRclickTreeDeviceList)
ON_NOTIFY(NM_DBLCLK, IDC_TREE_DEVICE_LIST, OnNMDblclkTreeDeviceList)
ON_CBN_SELCHANGE(IDC_COMBO_WNDNUM, OnCbnSelchangeComboWndNum)
ON_BN_CLICKED(IDC_BTN_CONFIG, OnBnClickedBtnConfig)
ON_BN_CLICKED(IDC_BTN_RECORD, OnBnClickedBtnRecord)
ON_CBN_SELCHANGE(IDC_COMBO_NETDELAY, OnCbnSelchangeComboNetdelay)
ON_BN_CLICKED(IDC_BTN_PLAY_BACK, OnBnClickedBtnPlayback)
ON_BN_CLICKED(IDC_BTN_MANAGE, OnBnClickedBtnManage)
ON_BN_CLICKED(IDC_BTN_LOG_CLEAR, OnBnClickedBtnLogClear)
ON_BN_CLICKED(IDC_BTN_LOG_SAVE, OnBnClickedBtnLogSave)
ON_BN_CLICKED(IDC_BTN_MAKE_IFRAME, OnBnClickedBtnMakeIframe)
ON_BN_CLICKED(IDC_BTN_LOG_CONFIG, OnBnClickedBtnLogConfig)
ON_NOTIFY(NM_CLICK, IDC_LIST_ALL_LOG, OnNMClickListAllLog)
ON_BN_CLICKED(IDC_BTN_PREVIEW, OnBnClickedBtnPreview)
ON_BN_CLICKED(IDC_BTN_EXIT, OnBnClickedBtnExit)
ON_BN_CLICKED(IDC_BTN_PLAY, OnBnClickedBtnPlay)
ON_BN_CLICKED(IDC_BTN_PICTURE_SNATCH, OnBnClickedBtnPictureSnatch)
ON_BN_CLICKED(IDC_BTN_ELSE, OnBnClickedBtnElse)
ON_NOTIFY(NM_DBLCLK, IDC_LIST_ALL_LOG, OnDblclkListAllLog)
ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_ALL_LOG, OnColumnclickListAllLog)
ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_ALARM_INFO, OnColumnclickListAlarmInfo)
ON_BN_CLICKED(IDC_BTN_TEST, OnBtnTest)
ON_BN_CLICKED(IDC_RADIO_LOCAL_LOG, OnRadioLocalLog)
ON_BN_CLICKED(IDC_RADIO_ALARM_INFO, OnRadioAlarmInfo)
ON_COMMAND(ID_MENU_SENSOR_ADD_PREVIEW, OnMenuSensorAddPreview)
ON_COMMAND(ID_MENU_HOLIDAY_CFG, OnMenuHolidayCfg)
ON_COMMAND(ID_MENU_T1Test, OnMENUT1Test)
ON_COMMAND(ID_MENU_T1Test, OnMENUT1Test)
ON_COMMAND(ID_MENU_PRODUCTION_LINE_TEST, OnMenuProductionLineTest)
ON_WM_HSCROLL()
ON_COMMAND(ID_MENU_DEVICE_ADD, OnMenuDeviceAdd)
ON_COMMAND(ID_MENU_DEVICE_DELETE, OnMenuDeviceDelete)
ON_COMMAND(ID_MENU_LOGOUT, OnMenuLogout)
ON_COMMAND(ID_MENU_DEVICE_INFO, OnMenuDeviceInfo)
ON_COMMAND(ID_MENU_CHANNEL_INFO, OnMenuChannelInfo)
ON_COMMAND(ID_MENU_JPEG_SNATCH_CONFIG, OnMenuJPEGSnatchConfig)
ON_COMMAND(ID_MENU_DEVICE_CHECK_TIME, OnMenuCheckTime)
ON_COMMAND(ID_MENU_UPGRADE, OnMenuUpgrade)
ON_COMMAND(ID_MENU_REBOOT, OnMenuReboot)
ON_COMMAND(ID_MENU_HD_FORMAT, OnMenuHDFormat)
ON_COMMAND(ID_MENU_POWEROFF, OnMenuPoweroff)
ON_COMMAND(ID_MENU_FORTIFY, OnMenuFortify)
ON_COMMAND(ID_MENU_VOICE_TALK, OnMenuVoiceTalk)
ON_COMMAND(ID_MENU_REMOTE_KEY, OnMenuRemoteKey)
ON_COMMAND(ID_MENU_VOICE_BROADCAST, OnMenuBroadCast)
ON_COMMAND(ID_MENU_DVR_LOCAL_RECORD, OnMenuDVRRecordStart)
ON_COMMAND(ID_MENU_LOG_SEARCH, OnMenuLogSearch)
ON_COMMAND(ID_MENU_WORK_STATE, OnMenuWorkState)
ON_COMMAND(ID_MENU_CONFIG_RESTORE, OnMenuDefaultConfig)
ON_COMMAND(ID_MENU_SERIAL_TRANS, OnMenuSerialTrans)
ON_COMMAND(ID_MENU_CONFIG_FLASH, OnMenuConfigFlash)
ON_COMMAND(ID_MENU_TALK_MR, OnMenuTalkMR)
ON_MESSAGE(WM_PROC_ALARM, OnWMProcAlarm)
ON_MESSAGE(WM_CHANGE_CHANNEL_ITEM_IMAGE,ChangeChannelItemImage)
ON_MESSAGE(WM_CHANGE_DEVICE_ITEM_IMAGE,ChangeDeviceItemImage)
ON_MESSAGE(WM_ADD_LOG, OnWMAddLog)
ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
ON_COMMAND(ID_MENU_PRODUCTION_LINE_TEST_EN, OnMenuProductionLineTest)
ON_COMMAND(IDM_MENU_VCA, OnMenuVca)
ON_BN_CLICKED(IDC_BTN_PRODUCT_CFG,OnBtntProductCfg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CClientDemoDlg message handlers
/*********************************************************
Function:	OnInitDialog
Desc:		Initialize the dialog
Input:	
Output:	
Return:	
**********************************************************/
BOOL CClientDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// Add "About..." menu item to system menu.
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	//CBrush cbr.CreateSolidBrush(COLORREF(0xffffff));//white
	// TODO: Add extra initialization here//////////////////////////////////////////////////////////////
	//
	//get path of configuration file
	char szLan[128] = {0};
	
	memset(g_szWorkDir, 0, MAX_PATH);
	GetCurrentPath(g_szWorkDir);
	//2012-06-21 �Ȱ���Ĭ��Ŀ¼���������򵱱��������ļ�������ʱ���ޱ���Ŀ¼����ʱ���������ļ���ʧ��
	g_CreateLocalDir();
	ReadLocalConfig();
	g_ReadDeviceConfig();
    g_ReadLocalReadSensorAdd();
	InitGlobalSysInfo();
	InitChildWindow();
	
	//initialize main window
	InitMainWin();
	
	LoadTreeImage();
	CreateTree();
	//for the demo log and alarm info
	CreateNewLogFile(g_struLocalParam.bAutoSaveLog);
	
	m_iCurRecDriver = m_cHardDriver[g_struLocalParam.iStartRecordDriver];
	
	m_pAutoRecTimer = SetTimer(AUTORECORD_TIMER, 2000, NULL);
	
	if (g_struLocalParam.bAutoCheckDeviceTime)
	{
		AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "Start DVR timing, the school time is %06d", g_struLocalParam.lCheckDeviceTime);
		m_pChkDevTimer = SetTimer(CHECK_DEVICE_TIMER, 1000, NULL);
	}
	m_pFreeSpaceTimer = SetTimer(FREESPACE_TIMER, 5000, NULL);
	m_pFileIntervalTimer = SetTimer(FILEINTERVAL_TIMER, 2000, NULL);
	
	m_pSysTimer = SetTimer(SYSTEM_TIMER, 1000, NULL);//refresh time on time
	
	//SDK uploads the info such as alarm and log sent from DVR by calling callback function 
	NET_DVR_SetDVRMessageCallBack_V30(MessageCallback, NULL);
	//SDK detects exception and uploads it, such as network disconnect
	
	NET_DVR_SetLogToFile(g_struLocalParam.nLogLevel, "c:\\SdkLog\\", g_struLocalParam.bCycleWriteLog);
	AddLog(-1, OPERATION_FAIL_T, "sdk v%x", NET_DVR_GetSDKBuildVersion());
	NET_DVR_SetExceptionCallBack_V30(WM_NULL/*WM_MYCOMMAND*/, NULL/*this->m_hWnd*/, g_ExceptionCallBack, NULL);
	NET_DVR_SetReconnect(5000, g_struLocalParam.bReconnect);
	if (!NET_DVR_SetShowMode(NORMALMODE, RGB(255, 255, 255)))
	{
		AddLog(-1, OPERATION_FAIL_T, "NET_DVR_SetShowMode");
	}
	PlayM4_InitDDrawDevice();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/*********************************************************
Function:	RegisterPlayer
Desc:		register the player or unregister
Input:	bRegister, TRUE=register, FALSE=unregister;
Output:	none
Return:	TRUE/FALSE
**********************************************************/
BOOL CClientDemoDlg::RegisterPlayer(BOOL bRegister)
{
	STARTUPINFO  si;  
	ZeroMemory(&si,  sizeof(si));  
	si.cb  =  sizeof  STARTUPINFO;  
	
	char* pCmdLine=new char[4096];
	
	PROCESS_INFORMATION  pi;  
	BOOL  res  =  FALSE;
	char szCurPath[256] = {0};
	GetCurrentPath(szCurPath);
	//sprintf(szCurPath, "%s\\", szCurPath);
	if (bRegister)
	{
	sprintf(pCmdLine, "regsvr32 %s\\PLAYSDK_DefSource.dll /s\
															  %s\\PLAYSDK_StreamSource.dll /s\
															  %s\\PLAYSDK_Sample.dll /s\
															  %s\\PLAYSDK_DefDemux.dll /s\
															  %s\\PLAYSDK_RFCDemux.dll /s\
															  %s\\PLAYSDK_MPEG2Source.dll /s\
															  %s\\PLAYSDK_MPEG2Demux.dll /s\
															  %s\\PLAYSDK_H264Dec.dll /s\
															  %s\\PLAYSDK_Mpeg4Dec.dll /s\
															  %s\\PLAYSDK_G722Dec.dll /s\
															  %s\\PLAYSDK_G711Dec.dll /s\
															  %s\\PLAYSDK_VTrans.dll /s\
															  %s\\PLAYSDK_ARenderer.dll /s\
															  %s\\PLAYSDK_VRenderer.dll /s\
															  %s\\PLAYSDK_Image.dll /s",\
															  szCurPath, szCurPath, szCurPath, szCurPath, szCurPath\
															  , szCurPath, szCurPath, szCurPath, szCurPath, szCurPath,\
															  szCurPath, szCurPath, szCurPath, szCurPath, szCurPath);
	}
	else
	{
	sprintf(pCmdLine, "regsvr32 -u %s\\PLAYSDK_DefSource.dll /s\
																 -u %s\\PLAYSDK_StreamSource.dll /s\
																 -u %s\\PLAYSDK_Sample.dll /s\
																 -u %s\\PLAYSDK_DefDemux.dll /s\
																 -u %s\\PLAYSDK_RFCDemux.dll /s\
																 -u %s\\PLAYSDK_MPEG2Source.dll /s\
																 -u %s\\PLAYSDK_MPEG2Demux.dll /s\
																 -u %s\\PLAYSDK_H264Dec.dll /s\
																 -u %s\\PLAYSDK_Mpeg4Dec.dll /s\
																 -u %s\\PLAYSDK_G722Dec.dll /s\
																 -u %s\\PLAYSDK_G711Dec.dll /s\
																 -u %s\\PLAYSDK_VTrans.dll /s\
																 -u %s\\PLAYSDK_ARenderer.dll /s\
																 -u %s\\PLAYSDK_VRenderer.dll /s\
																 -u %s\\PLAYSDK_Image.dll /s",\
																 szCurPath, szCurPath, szCurPath, szCurPath, szCurPath\
																 , szCurPath, szCurPath, szCurPath, szCurPath, szCurPath,\
																 szCurPath, szCurPath, szCurPath, szCurPath, szCurPath);
	}
	res = CreateProcess(NULL,  
		pCmdLine,  // execute dos command 
		NULL,  
		NULL,  
		NULL,  
		NORMAL_PRIORITY_CLASS,//    |  CREATE_NO_WINDOW,  
		NULL,  
		NULL,  
		&si,  
		&pi);  
	
	
	if  (TRUE == res)  
	{  
		CloseHandle(pi.hProcess);  
		CloseHandle(pi.hThread);  
	}
	else
	{
		g_pMainDlg->AddLog(-1, OPERATION_FAIL_T, "register player fail!");
	}
	if (pCmdLine != NULL)
	{
		delete [] pCmdLine;
	}
	return res;
}

/*********************************************************
Function:	OnPaint
Desc:	    refresh dialog
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting
		
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);
		
		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		
		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the szUser drags
//  the minimized window.
/*********************************************************
Function:	OnQueryDragIcon
Desc:	    drag mouse to initialize function
Input:	
Output:	
Return:	
**********************************************************/
HCURSOR CClientDemoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

/*********************************************************
Function:	InitHardDriver
Desc:		get pc basic info
Input:	
Output:	
Return:	
**********************************************************/
BOOL CClientDemoDlg::InitHardDriver()
{
	DWORD dw=GetLogicalDriveStrings(0,NULL);
	LPTSTR pAllDrivers=new char[dw];
	::GetLogicalDriveStrings(dw,pAllDrivers);
	LPTSTR pDriver=pAllDrivers;
	char tempDriver[26];
	DWORD nDriverNum=0;
	char *pTmp;
	
	while (pDriver[0]!=0)
	{
		tempDriver[nDriverNum++]=*pDriver;
		if ( (pTmp = _tcschr(pDriver,0)) != NULL)
		{
			pDriver=pTmp+1;          //locate to next driver
		}
		
	}
	
	//volume information
	TCHAR lpVolumeNameBuffer[200];
	DWORD dwVolumeSerialNumber,dwMaxComLength;
	DWORD dwFileSystemFlags;
	TCHAR lpFileSystemNameBuffer[50];
	
	DWORD nHardNum=0;
	for (DWORD i=0;i<nDriverNum;i++)
	{
		CString csRecRootPath;
		csRecRootPath.Format("%c%s",tempDriver[i],":\\");
		
		if (GetDriveType(csRecRootPath)==DRIVE_FIXED)
		{
			if (GetVolumeInformation(csRecRootPath,lpVolumeNameBuffer,sizeof(lpVolumeNameBuffer),&dwVolumeSerialNumber,
				&dwMaxComLength,&dwFileSystemFlags,lpFileSystemNameBuffer,sizeof(lpFileSystemNameBuffer)))
			{			
				this->m_cHardDriver[nHardNum++]=tempDriver[i];
			}
		}
	}
	m_iDriverNum=nHardNum;	
	delete[] pAllDrivers;
	return TRUE;
}

/*********************************************************
Function:	InitDecodeCard
Desc:		initialize decode card
Input:	
Output:	
Return:	TRUE,initialization succeed;FALSE,initialization failed;
**********************************************************/
BOOL CClientDemoDlg::InitDecodeCard()
{
	SetDecoderVideoExtOutputProc DLINK_SetDecoderVideoExtOutput;
	SetDisplayRegionProc DLINK_SetDisplayRegion;
	SetDisplayStandardProc DLINK_SetDisplayStandard;
	GetDisplayChannelCountProc DLINK_GetDisplayChannelCount;
	
	DLINK_SetDecoderVideoExtOutput = (SetDecoderVideoExtOutputProc)GetProcAddress(g_hDSSDK, "SetDecoderVideoExtOutput");
	DLINK_SetDisplayRegion = (SetDisplayRegionProc)GetProcAddress(g_hDSSDK, "SetDisplayRegion");
	DLINK_SetDisplayStandard = (SetDisplayStandardProc)GetProcAddress(g_hDSSDK, "SetDisplayStandard");
	DLINK_GetDisplayChannelCount = (GetDisplayChannelCountProc)GetProcAddress(g_hDSSDK, "GetDisplayChannelCount");

	int nDispNum = 0;
	char szLan[128] = {0};
	
	if (!NET_DVR_InitDDraw_Card(GetDlgItem(IDC_STATIC_PREVIEWBG)->m_hWnd,0xffffff))//dark blue 0xff2200
	{
		AddLog(-1, OPERATION_FAIL_T, "NET_DVR_InitDDraw_Card\n");
		return FALSE;
	}
	
	m_iCardChanCount = 0;
	if (!NET_DVR_InitDevice_Card(&m_iCardChanCount))
	{
		AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "NET_DVR_InitDevice_Card\n");
		return FALSE;
	}
	AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "Decoder Card Channel Num=%d", m_iCardChanCount);	
	if (m_iCardChanCount <= 0)
	{
		g_StringLanType(szLan, "�������뿨", "Please Insert Decoder Card");
		//AfxMessageBox(szLan);
		return FALSE;
	}
	else if (m_iCardChanCount > 16)
	{
		m_iCardChanCount = 16;
	}
	
	if (g_struLocalParam.bNTSC)
	{
		nDispNum = DLINK_GetDisplayChannelCount();
		if (nDispNum >= 2)
		{
			REGION_PARAM struDisplayRegion[MAX_DISPLAY_REGION];
			int i;
			memset(struDisplayRegion, 0, sizeof(struDisplayRegion));
			for (i=0; i<MAX_DISPLAY_REGION; i++)
				struDisplayRegion[i].color = RGB(i, i*8, i*16);
			for (i=0; i<nDispNum/2; i++)
			{
				DLINK_SetDisplayStandard(2*i,StandardNTSC);
				struDisplayRegion[0].left=0;	struDisplayRegion[0].top=0;		struDisplayRegion[0].width=352;		struDisplayRegion[0].height=240;
				struDisplayRegion[1].left=352;	struDisplayRegion[1].top=0;		struDisplayRegion[1].width=352;		struDisplayRegion[1].height=240;
				struDisplayRegion[2].left=0;	struDisplayRegion[2].top=240;	struDisplayRegion[2].width=352;		struDisplayRegion[2].height=240;
				struDisplayRegion[3].left=352;	struDisplayRegion[3].top=240;	struDisplayRegion[3].width=352;		struDisplayRegion[3].height=240;
				if (DLINK_SetDisplayRegion(2*i, 4, struDisplayRegion, 0) != HWERR_SUCCESS)
				{
					AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "SetDisplayRegion!");
				}				
				DLINK_SetDecoderVideoExtOutput(4*i, 0, 1, 2*i, 0, 0);
				DLINK_SetDecoderVideoExtOutput(4*i+1, 0, 1, 2*i, 1, 0);		
				DLINK_SetDecoderVideoExtOutput(4*i+2, 0, 1, 2*i, 2, 0);
				DLINK_SetDecoderVideoExtOutput(4*i+3, 0, 1, 2*i, 3, 0);
				
				DLINK_SetDisplayStandard(2*i+1,StandardNTSC);
				struDisplayRegion[0].left=0;	struDisplayRegion[0].top=0;		struDisplayRegion[0].width=704;		struDisplayRegion[0].height=480;
				if (DLINK_SetDisplayRegion(2*i+1, 1, struDisplayRegion, 0) != HWERR_SUCCESS)
				{
					AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "SetDisplayRegion failed!");
				}
				
				DLINK_SetDecoderVideoExtOutput(4*i, 1, 1, 2*i+1, 0, 0);		
			}			
		}
	}
	else
	{
		nDispNum = DLINK_GetDisplayChannelCount();
		if (nDispNum >= 2)
		{
			REGION_PARAM struDisplayRegion[MAX_DISPLAY_REGION];
			int i;
			memset(struDisplayRegion, 0, sizeof(struDisplayRegion));
			for (i=0; i<MAX_DISPLAY_REGION; i++)
				struDisplayRegion[i].color = RGB(i, i*8, i*16);
			for (i=0; i<nDispNum/2; i++)
			{
				DLINK_SetDisplayStandard(2*i,StandardPAL);
				struDisplayRegion[0].left=0;	struDisplayRegion[0].top=0;		struDisplayRegion[0].width=352;		struDisplayRegion[0].height=288;
				struDisplayRegion[1].left=352;	struDisplayRegion[1].top=0;		struDisplayRegion[1].width=352;		struDisplayRegion[1].height=288;
				struDisplayRegion[2].left=0;	struDisplayRegion[2].top=288;	struDisplayRegion[2].width=352;		struDisplayRegion[2].height=288;
				struDisplayRegion[3].left=352;	struDisplayRegion[3].top=288;	struDisplayRegion[3].width=352;		struDisplayRegion[3].height=288;
				if (DLINK_SetDisplayRegion(2*i, 4, struDisplayRegion, 0) != HWERR_SUCCESS)
				{
					AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "SetDisplayRegion failed!");
				}				
				DLINK_SetDecoderVideoExtOutput(4*i, 0, 1, 2*i, 0, 0);
				DLINK_SetDecoderVideoExtOutput(4*i+1, 0, 1, 2*i, 1, 0);		
				DLINK_SetDecoderVideoExtOutput(4*i+2, 0, 1, 2*i, 2, 0);
				DLINK_SetDecoderVideoExtOutput(4*i+3, 0, 1, 2*i, 3, 0);
				
				DLINK_SetDisplayStandard(2*i+1,StandardPAL);
				struDisplayRegion[0].left=0;	struDisplayRegion[0].top=0;		struDisplayRegion[0].width=704;		struDisplayRegion[0].height=576;
				if (DLINK_SetDisplayRegion(2*i+1, 1, struDisplayRegion, 0) != HWERR_SUCCESS)
				{
					AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "SetDisplayRegion failed!");
				}
				
				DLINK_SetDecoderVideoExtOutput(4*i, 1, 1, 2*i+1, 0, 0);		
			}			
		}
	}
	
	if (m_iCardChanCount==1)
	{
		m_iCurWndNum =1;
		m_comboWinNum.SetCurSel(0); 
	}
	else if (m_iCardChanCount<=4)
	{
		m_iCurWndNum = 4;
		m_comboWinNum.SetCurSel(1);
	}
	else if (m_iCardChanCount<=9)
	{
		m_iCurWndNum = 9;
		m_comboWinNum.SetCurSel(2);
	}
	else
	{
		m_iCurWndNum = 16;
		m_comboWinNum.SetCurSel(3);
	}
    m_comboWinNum.EnableWindow(FALSE);
	return TRUE;
}

/*********************************************************
Function:	InitGlobalSysInfo
Desc:		initialize global system info
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::InitGlobalSysInfo()
{
	InitHardDriver();
	
	g_pMainDlg = this;
	//initialize hardware decode background
	g_hPlayHardBgBrush = ::CreateSolidBrush(COLORREF(0xff2200));//dark blue0xff2200
	
	//initialize software decode background
	g_hPlaySoftBgBrush = ::CreateSolidBrush(COLORREF(0xffffff));//white
	if (g_struLocalParam.bUseCard)
	{
		if (InitDecodeCard())
		{
			g_bCardInit = TRUE;
			AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "InitDecodeCard success");
		}
		else
		{
			g_bCardInit = FALSE;
			AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "InitDecodeCard fail");
		}
	}
	
	//get current system resolution
	g_iCurScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	g_iCurScreenHeight = GetSystemMetrics(SM_CYSCREEN);
}

/*********************************************************
Function:	GetVersionInfor
Desc:		get version information
Input:	
Output:	
Return:	
**********************************************************/
typedef DWORD (WINAPI *VGetFileVersionInfoSize)(LPSTR lptstrFilename, LPDWORD lpdwHandle);  
typedef BOOL (WINAPI *VGetFileVersionInfo)(LPSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
typedef BOOL(WINAPI *VVerQueryValue)(const LPVOID pBlock,LPSTR lpSubBlock,LPVOID * lplpBuffer, PUINT puLen);
VGetFileVersionInfoSize vgetsize;
VGetFileVersionInfo vfileinfo;
VVerQueryValue vqureyvalue;
void CClientDemoDlg::GetVersionInfor()
{
	DWORD m_dwSize = 0;
	HMODULE handle = LoadLibrary("version.dll"); //there is no this library on win2000
	if (handle == NULL)
	{
		return;
	}
	vgetsize = (VGetFileVersionInfoSize)GetProcAddress(handle, "GetFileVersionInfoSizeA");
	m_dwSize = GetLastError();
	vfileinfo = (VGetFileVersionInfo)GetProcAddress(handle, "GetFileVersionInfoA");
	vqureyvalue = (VVerQueryValue)GetProcAddress(handle, "VerQueryValueA");
	char szInfo[128] = {0};
	m_dwSize = vgetsize("HCNetSDK.dll", NULL); 
	if (m_dwSize > 0)
	{
		char * m_lpBuffer   =   new char[m_dwSize];   
		char *lpSDKVersion = NULL;    
		//Get   the   versioninformation   block   
		BOOL bRet = vfileinfo("HCNetSDK.dll", 0, m_dwSize, m_lpBuffer);   
		
		struct LANGANDCODEPAGE {
		  WORD wLanguage;
		  WORD wCodePage;
		} *lpTranslate;
		DWORD cbTranslate = 0;
		vqureyvalue(m_lpBuffer, 
					  TEXT("\\VarFileInfo\\Translation"),
					  (LPVOID*)&lpTranslate,
					  (PUINT)&cbTranslate);
		wsprintf(szInfo, TEXT( "\\StringFileInfo\\%04x%04x\\FileVersion"), 
				lpTranslate-> wLanguage, 
				lpTranslate-> wCodePage); 

		bRet = vqureyvalue(m_lpBuffer,   
				szInfo,   
				(void **) &lpSDKVersion,   
				NULL); 
		char * m_lpSDKBuild = NULL;
		wsprintf(szInfo, TEXT( "\\StringFileInfo\\%04x%04x\\FileDescription"), 
				lpTranslate-> wLanguage, 
				lpTranslate-> wCodePage); 
		bRet = vqureyvalue(m_lpBuffer,   
				szInfo,  
				(void **) &m_lpSDKBuild,   
				NULL);   
		GetDlgItem(IDC_STATIC_DEMO_VERSION)->SetWindowText(m_lpSDKBuild);
		AddLog(-1, OPERATION_SUCC_T, "FileVersion%s %s", lpSDKVersion, m_lpSDKBuild);
		delete [] m_lpBuffer;
	}
	else
	{
		m_dwSize = GetLastError();
	}
}

/*********************************************************
Function:	InitMainWin
Desc:		demo main window layout
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::InitMainWin()
{
	m_iTreeWidth = 165;		//device tree width
	m_iRightWidth = 160;	//PTZ pic box width
	m_iFunBtnHeight = 85;	 //column height
	m_iListLogHeight = 110 ;//list log height
	char szLan[128] = {0};
	
	
	m_listAlarmInfo.InsertColumn(0,"",LVCFMT_LEFT,0,-1); 
	g_StringLanType(szLan, "ʱ��", "Time");
	m_listAlarmInfo.InsertColumn(1, szLan,LVCFMT_LEFT,150);
	g_StringLanType(szLan, "������Ϣ", "Alarm Info");
	m_listAlarmInfo.InsertColumn(2, szLan,LVCFMT_LEFT,400);
	g_StringLanType(szLan, "�豸��Ϣ", "Device Info");
	m_listAlarmInfo.InsertColumn(3, szLan,LVCFMT_LEFT,160);
	m_listAlarmInfo.SetExtendedStyle(m_listAlarmInfo.GetExtendedStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES);
	
	m_listAllLog.InsertColumn(0, "", LVCFMT_LEFT, 0, -1); 
	g_StringLanType(szLan, "ʱ��", "Time");
	m_listAllLog.InsertColumn(1, szLan, LVCFMT_LEFT, 120);
	g_StringLanType(szLan, "״̬", "State");
	m_listAllLog.InsertColumn(2, szLan, LVCFMT_LEFT, 40);
	g_StringLanType(szLan, "����", "Operation");
	m_listAllLog.InsertColumn(3, szLan, LVCFMT_LEFT, 300);
	g_StringLanType(szLan, "�豸��Ϣ", "Device Info");
	m_listAllLog.InsertColumn(4,szLan,LVCFMT_LEFT,140);
	g_StringLanType(szLan, "������Ϣ", "Error Info");
	m_listAllLog.InsertColumn(5,szLan,LVCFMT_LEFT,80);
	m_listAllLog.SetExtendedStyle(m_listAllLog.GetExtendedStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES);
	if (g_bInSide)
	{
		m_comboWinNum.ResetContent();
		m_comboWinNum.AddString("1");
		m_comboWinNum.AddString("4");
		m_comboWinNum.AddString("9");
		m_comboWinNum.AddString("16");
		m_comboWinNum.AddString("25");
		m_comboWinNum.AddString("36");
		m_comboWinNum.AddString("49");
		m_comboWinNum.AddString("64");
		m_comboWinNum.AddString("81");
	}
	
	m_iSelListType = 0;//local log
	((CButton *)GetDlgItem(IDC_RADIO_LOCAL_LOG))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_ALARM_INFO))->SetCheck(FALSE);
	OnSelchangeComboListType();
	MoveWindow(0, 0, DEMO_FULL_WIDTH, DEMO_FULL_HEIGHT, TRUE);

    GetVersionInfor();
	CString csTemp = _T("");
	DWORD dwVersion = NET_DVR_GetSDKBuildVersion();
	
	csTemp.Format("HCNetSDK V%d.%d.%d.%d", (0xff000000 & dwVersion)>>24, (0x00ff0000 & dwVersion)>>16,\
		(0x0000ff00 & dwVersion)>>8, (0x000000ff & dwVersion));//%.d,0x0000ff & dwVersion,build NO. do not expose
	GetDlgItem(IDC_STATIC_SDK_VERSION)->SetWindowText(csTemp);
	
	dwVersion = PlayM4_GetSdkVersion();
	csTemp.Format("PlayCtrl V%d.%d.%d.%d", (0xff000000 & dwVersion)>>24, (0x00ff0000 & dwVersion)>>16,\
		(0x0000ff00 & dwVersion)>>8, (0x000000ff & dwVersion));
	GetDlgItem(IDC_STATIC_PLAYM4_VERSION)->SetWindowText(csTemp);
	
	InitPreviewRect();//preview window, part of main window
}


/*********************************************************
Function:	InitChildWindow
Desc:		initialize child window
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::InitChildWindow(void)
{
	for (int i = 0; i < MAX_OUTPUTS; i++)
	{
		g_dlgOutput[i].m_iSubWndIndex = i;
		g_dlgOutput[i].Create(IDD_DLG_OUTPUT,this);
	}
	m_dlgRemoteCfg = new CDlgRemoteCfg;
	m_dlgRemoteCfg->Create(IDD_DLG_REMOTE_CFG,this);
	m_dlgRemoteCfg->ShowWindow(SW_HIDE);
	
	m_dlgLocalCfg = new CDlgLocalConfig;
	m_dlgLocalCfg->Create(IDD_DLG_LOCAL_CONFIG, this);
	m_dlgLocalCfg->ShowWindow(SW_HIDE);
	
	m_dlgIPAccess = new CDlgRemoteIPAccess;
	m_dlgIPAccess->Create(IDD_DLG_REMOTE_IP_ACCESS_CFG, this);
	m_dlgIPAccess->ShowWindow(SW_HIDE);
	
	m_dlgHDConfig = new CDlgHardDiskCfg;
	m_dlgHDConfig->Create(IDD_DLG_HARD_DISK_CFG, this);
	m_dlgHDConfig->ShowWindow(SW_HIDE);
	
	m_dlgConfigAll = new CDlgConfigAll;
	m_dlgConfigAll->Create(IDD_DLG_CONFIG_ALL, this);
	m_dlgConfigAll->ShowWindow(SW_HIDE);
	
	m_dlgOutputCtrl = new CDlgOutputCtrl;
	m_dlgOutputCtrl->Create(IDD_DLG_OUTPUT_CTRL, this);
	m_dlgOutputCtrl->ShowWindow(SW_HIDE);

	m_dlgPlayBack = new CDlgPlayBack;
	m_dlgPlayBack->Create(IDD_DLG_PLAY_BACK, this);
	m_dlgPlayBack->ShowWindow(SW_HIDE);

// 	m_pStreamIDDlg = new CDlgStreamID;
// 	m_pStreamIDDlg->Create(IDD_DLG_STREAM_ID, this);
// 	m_pStreamIDDlg->ShowWindow(SW_HIDE);

	m_dlgBatchCfg = new CDlgBatchConfig;
	m_dlgBatchCfg->Create(IDD_DLG_BATCH_CONFIG, this);
	m_dlgBatchCfg->ShowWindow(SW_HIDE);

	m_dlgProductCfg = new CDlgProductCfg;
	m_dlgProductCfg->Create(IDD_DLG_PRODUCT_CFG,this);
	m_dlgProductCfg->ShowWindow(SW_HIDE);
}
/*********************************************************
Function:	CreateTree
Desc:		create device tree
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::CreateTree(void)
{
	m_treeDeviceList.DeleteAllItems();
	char szLan[128] = {0};
	g_StringLanType(szLan, "�豸��(�һ�����豸)", "Device Tree(Right Click to Add Device)");
	HTREEITEM hRoot = m_treeDeviceList.InsertItem(szLan, TREE_ALL, TREE_ALL);
	CString strTemp =_T("");
	CString strChanTmp = _T("");
	int i = 0;
	int j = 0;
	HTREEITEM hDevice = NULL;
	HTREEITEM hChan = NULL;
	for (i = 0; i < MAX_DEVICES; i++)
	{
		if (g_struDeviceInfo[i].iDeviceChanNum != -1)
		{
			strTemp.Format("%s", g_struDeviceInfo[i].chLocalNodeName);
			hDevice = m_treeDeviceList.InsertItem(strTemp, g_struDeviceInfo[i].dwImageType, g_struDeviceInfo[i].dwImageType, hRoot);
			m_treeDeviceList.SetItemData(hDevice, DEVICETYPE*1000 + g_struDeviceInfo[i].iDeviceIndex);
			for (j=0; j< MAX_CHANNUM_V40; j++)
			{
				if (g_struDeviceInfo[i].pStruChanInfo[j].bEnable)
				{
					strChanTmp.Format("%s", g_struDeviceInfo[i].pStruChanInfo[j].chChanName);
					hChan = m_treeDeviceList.InsertItem(strChanTmp, g_struDeviceInfo[i].pStruChanInfo[j].dwImageType, g_struDeviceInfo[i].pStruChanInfo[j].dwImageType, hDevice);
					m_treeDeviceList.SetItemData(hChan,CHANNELTYPE * 1000 + g_struDeviceInfo[i].pStruChanInfo[j].iChanIndex);
				}
				else
				{
					g_struDeviceInfo[i].pStruChanInfo[j].iChanIndex = -1;
				}
			}
			if (g_struDeviceInfo[i].lLoginID >= 0)
			{
				m_treeDeviceList.Expand(hDevice,TVE_EXPAND);
			}
			else
			{
				m_treeDeviceList.Expand(hDevice,TVE_COLLAPSE);
			}
			
		}
	}
	m_treeDeviceList.Expand(hRoot,TVE_EXPAND);
	m_treeDeviceList.Expand(m_treeDeviceList.GetRootItem(),TVE_EXPAND);
}

/*********************************************************
Function:	OnBnClickedBtnPreview
Desc:		preview button response, switch to preview interface
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnBnClickedBtnPreview()
{
	if (m_iMainType != PREVIEW_T)
	{
		//modify preview flag
		m_iMainType = PREVIEW_T;
		
		
		//hide configuration and button control
		ConfigAllHide();
		m_dlgPlayBack->HideAll();
		if (g_struLocalParam.bUseCard)
		{
			if (!g_bCardInit)
			{
				if (InitDecodeCard())
				{
					g_bCardInit = TRUE;
				}
				else
				{
					g_bCardInit = FALSE;
				}
			}
		}
		else
		{
			if (g_bCardInit)
			{
				g_bCardInit = FALSE;
				NET_DVR_ReleaseDevice_Card();
				NET_DVR_ReleaseDDraw_Card();
			}
		}
		PreviewReferShow(TRUE);
	}
}

/*********************************************************
Function:	LoopSvc
Desc:		preview loop service
Input:	none
Output:	none
Return:	none
**********************************************************/
void *LoopSvc(void *pParam)
{
	int i = 0;
	while (1)
	{
		for (i=0; i<g_struLocalParam.iCycleTime; i++)
		{
			Sleep(1000);
			if (!g_struLocalParam.bCycling)
			{
				goto exitsvc;
			}
		}
		g_pMainDlg->CyclePlayAllDevicesLogined();
	}
exitsvc:
	g_pMainDlg->StopPlayAll();
	
	g_pMainDlg->ChangePlayBtnState();
	return NULL;
}


/*********************************************************
Function:	StartPlayAll
Desc:		play all, only log-in device
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::StartPlayAll(void)
{
	//stop all play window
	StopPlayAll();
	
	//get and save all surveillance points that can be previewed
	int i = 0;
	int j = 0;
	int iFirstPlayNum = 0;//first surveillance point
	int iDeviceIndex = -1;
	int iChanIndex = -1;
	int iSurveillanceNumLeft = 0;
	g_nSurveillanceTotalNum = 0;
	
	//get current surveillance to play
	for (i = 0; i < MAX_DEVICES; i++)
	{
		if (-1 != g_struDeviceInfo[i].iDeviceChanNum &&  g_struDeviceInfo[i].lLoginID != -1)
		{
			for (j = 0; j < MAX_CHANNUM_V30; j++)
			{	
				if (g_struDeviceInfo[i].pStruChanInfo[j].bEnable)
				{
					g_struPlayAllInfo[g_nSurveillanceTotalNum].iDeviceIndex = i;//serial starts from 0
					g_struPlayAllInfo[g_nSurveillanceTotalNum].iChanIndex = j;
					g_nSurveillanceTotalNum ++;
				}
			}
		}
	}
	
	if (g_nSurveillanceTotalNum == 0)
	{
		return;//no monitoring points that can be played
	}
	AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "StartPlayAll:Total Chan Number= %d", g_nSurveillanceTotalNum);
	
	if (g_nSurveillanceTotalNum > m_iCurWndNum)
	{//first group of mornitoring points
		iFirstPlayNum = m_iCurWndNum;
		iSurveillanceNumLeft = g_nSurveillanceTotalNum - m_iCurWndNum;
		g_iNextSurveillanceIndex = m_iCurWndNum;
		g_struLocalParam.bNextPage = TRUE;
		g_struLocalParam.bFrontPage =FALSE ;
	}
	else//only one group
	{
		iFirstPlayNum = g_nSurveillanceTotalNum;
		iSurveillanceNumLeft = 0;
		g_iNextSurveillanceIndex = g_nSurveillanceTotalNum;
		g_struLocalParam.bFrontPage = FALSE;
		g_struLocalParam.bNextPage = FALSE;
	}
	//preview first group
	for (i = 0; i < iFirstPlayNum; i++)
	{
		iDeviceIndex = g_struPlayAllInfo[i].iDeviceIndex;
		iChanIndex = g_struPlayAllInfo[i].iChanIndex;
		g_dlgOutput[i].StartPlay(&g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex], GetChanItem(iDeviceIndex, iChanIndex));
		Sleep(10);
	}
	
	//cycle play
	if (g_struLocalParam.bCyclePlay)
	{	
		if (!g_struLocalParam.bCycling)
		{
			g_struLocalParam.bCycling = TRUE;
			g_struLocalParam.bPaused = FALSE;
			_beginthread((void (__cdecl *)(void *))LoopSvc, 0, NULL);
		}
		// 		if (g_pCycleTimer == NULL)
		// 		{
		// 			g_pCycleTimer = SetTimer(CYCLE_PREVIEW_TIMER, g_struLocalParam.iCycleTime*1000, NULL);
		// 		}
		// 		g_struLocalParam.bCycling = TRUE;
		// 		g_struLocalParam.bPaused = FALSE;
	}
	
	if (iSurveillanceNumLeft != 0)
	{
		g_struLocalParam.bNextPage = TRUE;
	}
	else
	{
		g_struLocalParam.bNextPage = FALSE;
	}
	g_struLocalParam.bFrontPage = FALSE;
	g_bPlayAllDevice = TRUE;
	ChangePlayBtnState();
}

/*********************************************************
Function:	OnBnClickedBtnPlay
Desc:		play all or stop all
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnBnClickedBtnPlay()
{
	if (IsPlaying())//stop all playing windows
	{
		if (!g_struLocalParam.bCycling)
		{
			StopPlayAll();
			ChangePlayBtnState();
		}
		g_struLocalParam.bPaused = TRUE;
		g_bPlayAllDevice = FALSE;
		g_struLocalParam.bCycling = FALSE;
		// 		if (g_pCycleTimer != NULL)
		
		// 			KillTimer(CYCLE_PREVIEW_TIMER);
		// 			g_pCycleTimer = NULL;
		// 		}
		return;
	}
	
	StartPlayAll();	
}

/*********************************************************
Function:	OnBnClickedBtnMakeIframe
Desc:		mandatory I frame
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnBnClickedBtnMakeIframe()
{
	int i = m_iCurWndIndex;
	if (g_dlgOutput[i].m_lPlayHandle < 0)
	{
		return;
	}
	int iDeviceIndex = g_dlgOutput[i].m_struChanInfo.iDeviceIndex;
	int iChanIndex = g_dlgOutput[i].m_struChanInfo.iChanIndex;
	if (iDeviceIndex == -1 || iChanIndex == -1)
	{
		return;
	}
	
    if (iChanIndex < ZERO_CHAN_INDEX) // 500 ����Ϊ��ͨ��
    {
		if (((g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].dwProtocol>>31)&0x1) == 0)
		{
			//if (!NET_DVR_MakeKeyFrame(g_struDeviceInfo[iDeviceIndex].lLoginID,  g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].iChannelNO))
			memset(&m_struIFrame, 0, sizeof(m_struIFrame));
			m_struIFrame.dwSize = sizeof(NET_DVR_I_FRAME);
			m_struIFrame.dwChan = g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].iChannelNO;
			m_struIFrame.byStreamType = g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].dwStreamType;
			if(!NET_DVR_RemoteControl(g_struDeviceInfo[iDeviceIndex].lLoginID, NET_DVR_MAKE_I_FRAME, &m_struIFrame, sizeof(m_struIFrame)))
			{
				AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_MAKE_I_FRAME");
				char szLan[128] = {0};
				g_StringLanType(szLan, "ǿ��I֡ʧ�ܣ�", "Make I Frame Failed");
				AfxMessageBox(szLan);
			}
			else
			{
				AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_MAKE_I_FRAME");
			}
		}
	    else
		{
			//if (!NET_DVR_MakeKeyFrameSub(g_struDeviceInfo[iDeviceIndex].lLoginID,  g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].iChannelNO))
			memset(&m_struIFrame, 0, sizeof(m_struIFrame));
			m_struIFrame.dwSize = sizeof(NET_DVR_I_FRAME);
			m_struIFrame.dwChan = g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].iChannelNO;
			m_struIFrame.byStreamType = g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].dwStreamType;
			if(!NET_DVR_RemoteControl(g_struDeviceInfo[iDeviceIndex].lLoginID, NET_DVR_MAKE_I_FRAME, &m_struIFrame, sizeof(m_struIFrame)))
			{
				AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_MAKE_I_FRAME");
				char szLan[128] = {0};
				g_StringLanType(szLan, "ǿ��I֡ʧ�ܣ�", "Make I Frame Failed");
				AfxMessageBox(szLan);
			}
			else
			{
				AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_MAKE_I_FRAME");
			}
		}
	}
    else
    {
        int iZeroChannel = iChanIndex - ZERO_CHAN_INDEX + g_struDeviceInfo[iDeviceIndex].iStartChan;
        if (!NET_DVR_ZeroMakeKeyFrame(g_struDeviceInfo[iDeviceIndex].lLoginID, iChanIndex - ZERO_CHAN_INDEX + g_struDeviceInfo[iDeviceIndex].iStartChan))
        {
            AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_ZeroMakeKeyFrame Chan[%d]", iZeroChannel);
        }
        else
        {
            AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_ZeroMakeKeyFrame Chan[%d]", iZeroChannel);
        }
    }

}

/*********************************************************
Function:	PreviewReferShow
Desc:		display or hide control and dialog related to preview
Input:	bShow,TRUE,display,FALSE,hide;
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::PreviewReferShow(BOOL bShow)
{
	int iShowStat = bShow?SW_SHOW:SW_HIDE;
	
	if (bShow)
	{
		if (g_struLocalParam.bUseCard)
		{
			NET_DVR_ClearSurface_Card();
		}
		if (g_struLocalParam.bEnlarged)
		{
			ArrangeOutputs(1);
		}
		else
		{
			ArrangeOutputs(m_iCurWndNum);
		}	
	}
	else
	{
		if (g_struLocalParam.bUseCard)
		{
			NET_DVR_ClearSurface_Card();
		}
		for (int i=0; i<MAX_OUTPUTS; i++)
		{
			g_dlgOutput[i].ShowWindow(SW_HIDE);
		}
	}
	GetDlgItem(IDC_STATIC_PREVIEWBG)->ShowWindow(iShowStat);
	//GetDlgItem(IDC_STATIC_PREVIEWBG)->Invalidate(!bShow);
	GetDlgItem(IDC_BTN_PLAY)->ShowWindow(iShowStat);
	GetDlgItem(IDC_BTN_RECORD)->ShowWindow(iShowStat);
	GetDlgItem(IDC_BTN_PICTURE_SNATCH)->ShowWindow(iShowStat);
	GetDlgItem(IDC_BTN_MAKE_IFRAME)->ShowWindow(iShowStat);
	
	GetDlgItem(IDC_STATIC_PREVIEW1)->ShowWindow(iShowStat);
	GetDlgItem(IDC_STATIC_PREVIEW2)->ShowWindow(iShowStat);
	GetDlgItem(IDC_STATIC_PREVIEW3)->ShowWindow(iShowStat);
	GetDlgItem(IDC_STATIC_NETDELAY)->ShowWindow(iShowStat);
	GetDlgItem(IDC_STATIC_WIN_NUM)->ShowWindow(iShowStat);
	GetDlgItem(IDC_COMBONETDELAY)->ShowWindow(iShowStat);
	GetDlgItem(IDC_COMBO_WNDNUM)->ShowWindow(iShowStat);
	
	m_dlgOutputCtrl->ShowWindow(iShowStat);
}

/*********************************************************
Function:	InitPreviewRect
Desc:		initialize preview area
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::InitPreviewRect()
{
	m_rectPreviewBG.top    = m_iFunBtnHeight;
	m_rectPreviewBG.left   = m_iTreeWidth;
	m_rectPreviewBG.right  = DEMO_FULL_WIDTH - m_iRightWidth;
	m_rectPreviewBG.bottom = m_rectPreviewBG.Width()*3/4 + m_iFunBtnHeight+4;//+4
	
	m_rectRightArea.top    = m_iFunBtnHeight - 3;
	m_rectRightArea.left   = DEMO_FULL_WIDTH - m_iRightWidth + 5;//
	m_rectRightArea.right  = DEMO_FULL_WIDTH;//
	m_rectRightArea.bottom = m_rectPreviewBG.Width()*3/4 + m_iFunBtnHeight + 55; //+55button height
	
	GetDlgItem(IDC_STATIC_PREVIEWBG)->MoveWindow(&m_rectPreviewBG,TRUE);
	
	m_dlgOutputCtrl->MoveWindow(&m_rectRightArea,TRUE);
	m_dlgOutputCtrl->ShowWindow(SW_SHOW);
	
	if (!g_struLocalParam.bUseCard)
	{
		m_iCurWndNum = 4;//default window number is 4
		m_comboWinNum.SetCurSel(1);
		
	}
	m_comboNetDelayBufNum.SetCurSel(1);
	//output box layout
	ArrangeOutputs(m_iCurWndNum);//
}

/*********************************************************
Function:	LoadTreeImage
Desc:		load tree iamge
Input:	
Output:	
Return:	
**********************************************************/
#define MAX_BMPS 14
void CClientDemoDlg::LoadTreeImage(void)
{
	CBitmap cBmp[MAX_BMPS];
	
	m_imageTreeList.Create(16,16,ILC_COLOR32 | ILC_MASK,1,1);
	
	cBmp[TREE_ALL].LoadBitmap(IDB_BITMAP_TREE);
	m_imageTreeList.Add(&cBmp[TREE_ALL],RGB(1,1,1));
	
	cBmp[DEVICE_LOGOUT].LoadBitmap(IDB_BITMAP_LOGOUT);
	m_imageTreeList.Add(&cBmp[DEVICE_LOGOUT],RGB(1,1,1));
	cBmp[DEVICE_LOGIN].LoadBitmap(IDB_BITMAP_LOGIN);
	m_imageTreeList.Add(&cBmp[DEVICE_LOGIN],RGB(1,1,1));
	cBmp[DEVICE_FORTIFY].LoadBitmap(IDB_BITMAP_FORTIFY);
	m_imageTreeList.Add(&cBmp[DEVICE_FORTIFY],RGB(1,1,1));
	cBmp[DEVICE_ALARM].LoadBitmap(IDB_BITMAP_DEV_ALARM);
	m_imageTreeList.Add(&cBmp[DEVICE_ALARM],RGB(1,1,1));
	
	cBmp[DEVICE_FORTIFY_ALARM].LoadBitmap(IDB_BITMAP_FORTIFY_ALARM);
	m_imageTreeList.Add(&cBmp[DEVICE_FORTIFY_ALARM],RGB(1,1,1));
	
	cBmp[CHAN_ORIGINAL].LoadBitmap(IDB_BITMAP_CAMERA);
	m_imageTreeList.Add(&cBmp[CHAN_ORIGINAL],RGB(1,1,1));
	cBmp[CHAN_PLAY].LoadBitmap(IDB_BITMAP_PLAY);
	m_imageTreeList.Add(&cBmp[CHAN_PLAY],RGB(1,1,1));
	cBmp[CHAN_RECORD].LoadBitmap(IDB_BITMAP_REC);
	m_imageTreeList.Add(&cBmp[CHAN_RECORD],RGB(1,1,1));
	cBmp[CHAN_PLAY_RECORD].LoadBitmap(IDB_BITMAP_PLAYANDREC);
	m_imageTreeList.Add(&cBmp[CHAN_PLAY_RECORD],RGB(1,1,1));
	
	cBmp[CHAN_ALARM].LoadBitmap(IDB_BITMAP_ALARM);
	m_imageTreeList.Add(&cBmp[CHAN_ALARM],RGB(1,1,1));
	cBmp[CHAN_PLAY_ALARM].LoadBitmap(IDB_BITMAP_PLAY_ALARM);
	m_imageTreeList.Add(&cBmp[CHAN_PLAY_ALARM],RGB(1,1,1));
	
	cBmp[CHAN_PLAY_RECORD_ALARM].LoadBitmap(IDB_BITMAP_P_R_A);
	m_imageTreeList.Add(&cBmp[CHAN_PLAY_RECORD_ALARM],RGB(1,1,1));
	
	cBmp[CHAN_OFF_LINE].LoadBitmap(IDB_BITMAP_CHAN_OFF);
	m_imageTreeList.Add(&cBmp[CHAN_OFF_LINE],RGB(1,1,1));
	
	m_treeDeviceList.SetImageList(&m_imageTreeList, LVSIL_NORMAL);
}

/*********************************************************
Function:	IsPlaying
Desc:		whether any window is previewing
Input:	
Output:	
Return:	
**********************************************************/
BOOL CClientDemoDlg::IsPlaying(void)
{
	if (m_iMainType == PREVIEW_T)
	{
		for (int i = 0; i < MAX_OUTPUTS; i ++)
		{
			if (g_dlgOutput[i].m_lPlayHandle >= 0)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


/*********************************************************
Function:	ArrangeOutputs
Desc:		output layout
Input:	iNumber, current output number
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::ArrangeOutputs(int iNumber)
{
	if (iNumber == 0)
	{
		AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "ArrangeOutputs number=0!");
		return;
	}
	int i = 0;
	CRect crect;
	
	int iSqrtNum = 0;//sqrt value of window number
	int iWidth = 0;//window width
	int iHeight = 0;//window height
	
	iSqrtNum = (int)sqrt((double)iNumber);
	
	for (i = 0;i < MAX_OUTPUTS;i++)
	{
		g_dlgOutput[i].ShowWindow(SW_HIDE);	
	}
	
	if (g_struLocalParam.bMultiScreen)
	{//muti-window full screen
		iWidth = (g_iCurScreenWidth - OUTPUT_INTERVAL*(iSqrtNum-1))/iSqrtNum;
		iHeight = (g_iCurScreenHeight - OUTPUT_INTERVAL*(iSqrtNum-1))/iSqrtNum;
		
		for (i = 0; i < iNumber; i++)
		{
			g_dlgOutput[i].MoveWindow((i%iSqrtNum)*(iWidth+OUTPUT_INTERVAL),(i/iSqrtNum)*(iHeight+OUTPUT_INTERVAL),iWidth,iHeight,TRUE);
			g_dlgOutput[i].m_struCardPlayParam.nLeft = (i%iSqrtNum)*(iWidth+OUTPUT_INTERVAL);
			g_dlgOutput[i].m_struCardPlayParam.nTop = (i/iSqrtNum)*(iHeight+OUTPUT_INTERVAL);
			g_dlgOutput[i].m_struCardPlayParam.nWidth = iWidth;
			g_dlgOutput[i].m_struCardPlayParam.nHeight = iHeight;
			g_dlgOutput[i].ShowWindow(SW_SHOW);
		}
	}
	else
	{
		iWidth = (m_rectPreviewBG.Width()-OUTPUT_INTERVAL*(iSqrtNum-1))/iSqrtNum;//a single pic width in partition
		iHeight = (m_rectPreviewBG.Height()-OUTPUT_INTERVAL*(iSqrtNum-1))/iSqrtNum;//a single pic height in partition
		int iPlayIndex = 0;
		for (i = 0; i < iNumber; i++)
		{
			if (g_struLocalParam.bEnlarged)
			{//double click to zoom some pic, iNumber = 1
				iPlayIndex = m_iCurWndIndex;
			}
			else
			{
				iPlayIndex = i;
			}
			g_dlgOutput[iPlayIndex].MoveWindow(m_iTreeWidth + (i%iSqrtNum)*(iWidth+OUTPUT_INTERVAL),\
				m_iFunBtnHeight+(i/iSqrtNum)*(iHeight+OUTPUT_INTERVAL),iWidth,iHeight,TRUE);
			g_dlgOutput[iPlayIndex].m_struCardPlayParam.nLeft = (i%iSqrtNum)*(iWidth+OUTPUT_INTERVAL);//m_iTreeWidth+
			g_dlgOutput[iPlayIndex].m_struCardPlayParam.nTop = (i/iSqrtNum)*(iHeight+OUTPUT_INTERVAL);//m_iFunBtnHeight+
			g_dlgOutput[iPlayIndex].m_struCardPlayParam.nWidth = iWidth;
			g_dlgOutput[iPlayIndex].m_struCardPlayParam.nHeight = iHeight;
			g_dlgOutput[iPlayIndex].ShowWindow(SW_SHOW);
			g_dlgOutput[iPlayIndex].DrawOutputBorder();
		}
	}
	//hardware decode window set
	NET_DVR_DISPLAY_PARA struDisPlayParam;
	if (g_struLocalParam.bUseCard)
	{
		//clear and hide previous pic
		NET_DVR_ClearSurface_Card();
		for (i = 0; i < MAX_OUTPUTS; i++)
		{
			struDisPlayParam.bToScreen = 0;
			struDisPlayParam.bToVideoOut = 1;
			struDisPlayParam.nLeft = g_dlgOutput[i].m_struCardPlayParam.nLeft;
			struDisPlayParam.nTop = g_dlgOutput[i].m_struCardPlayParam.nTop;
			struDisPlayParam.nWidth = g_dlgOutput[i].m_struCardPlayParam.nWidth;
			struDisPlayParam.nHeight = g_dlgOutput[i].m_struCardPlayParam.nHeight;
			NET_DVR_ResetPara_Card(g_dlgOutput[i].m_lPlayHandle,&struDisPlayParam);
		}
		
		if (g_struLocalParam.bMultiScreen)
		{
			for (i = 0; i < m_iCurWndNum; i++)
			{
				struDisPlayParam.bToScreen = 1;
				struDisPlayParam.bToVideoOut = 1;
				struDisPlayParam.nLeft = g_dlgOutput[i].m_struCardPlayParam.nLeft;
				struDisPlayParam.nTop = g_dlgOutput[i].m_struCardPlayParam.nTop;
				struDisPlayParam.nWidth = g_dlgOutput[i].m_struCardPlayParam.nWidth;
				struDisPlayParam.nHeight = g_dlgOutput[i].m_struCardPlayParam.nHeight;
				NET_DVR_ResetPara_Card(g_dlgOutput[i].m_lPlayHandle,&struDisPlayParam);
			}
		}
		else
		{
			if (iNumber == 1)
			{//single channel hardware decode or single window zoom
				struDisPlayParam.bToScreen = 1;
				struDisPlayParam.bToVideoOut = 1;
				struDisPlayParam.nLeft = g_dlgOutput[m_iCurWndIndex].m_struCardPlayParam.nLeft;
				struDisPlayParam.nTop = g_dlgOutput[m_iCurWndIndex].m_struCardPlayParam.nTop;
				struDisPlayParam.nWidth = g_dlgOutput[m_iCurWndIndex].m_struCardPlayParam.nWidth;
				struDisPlayParam.nHeight = g_dlgOutput[m_iCurWndIndex].m_struCardPlayParam.nHeight;
				NET_DVR_ResetPara_Card(g_dlgOutput[m_iCurWndIndex].m_lPlayHandle,&struDisPlayParam);
			}
			else
			{//narrow
				for (i = 0; i < iNumber; i++)
				{
					struDisPlayParam.bToScreen = 1;
					struDisPlayParam.bToVideoOut = 1;
					struDisPlayParam.nLeft = g_dlgOutput[i].m_struCardPlayParam.nLeft;
					struDisPlayParam.nTop = g_dlgOutput[i].m_struCardPlayParam.nTop;
					struDisPlayParam.nWidth = g_dlgOutput[i].m_struCardPlayParam.nWidth;
					struDisPlayParam.nHeight = g_dlgOutput[i].m_struCardPlayParam.nHeight;
					NET_DVR_ResetPara_Card(g_dlgOutput[i].m_lPlayHandle,&struDisPlayParam);
				}
			}
		}
	}
}


/*********************************************************
Function:	FullScreen
Desc:		display/hide main control while single or multiple pic preview in full screen
Input:	bFullScreen: TRUE, full screen, hide some control, FALSE, recover display control and previous pic
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::FullScreen(BOOL bFullScreen)
{
	int iShowStat = bFullScreen?SW_HIDE:SW_SHOW;
	
	m_treeDeviceList.ShowWindow(iShowStat);
	
	GetDlgItem(IDC_BTN_LOG_SAVE)->ShowWindow(iShowStat);
	GetDlgItem(IDC_RADIO_LOCAL_LOG)->ShowWindow(iShowStat);
	GetDlgItem(IDC_RADIO_ALARM_INFO)->ShowWindow(iShowStat);
	// 	GetDlgItem(IDC_COMBO_LIST_TYPE)->ShowWindow(iShowStat);
	// 	GetDlgItem(IDC_STATIC_LIST_TYPE)->ShowWindow(iShowStat);
	
	GetDlgItem(IDC_BTN_LOG_CLEAR)->ShowWindow(iShowStat);
	GetDlgItem(IDC_BTN_LOG_CONFIG)->ShowWindow(iShowStat);
	
	GetDlgItem(IDC_STATIC_MAIN)->ShowWindow(iShowStat);
	GetDlgItem(IDC_BTN_CONFIG)->ShowWindow(iShowStat);
	GetDlgItem(IDC_BTN_PREVIEW)->ShowWindow(iShowStat);
	GetDlgItem(IDC_BTN_PLAY_BACK)->ShowWindow(iShowStat);
	GetDlgItem(IDC_BTN_MANAGE)->ShowWindow(iShowStat);
	GetDlgItem(IDC_BTN_ELSE)->ShowWindow(iShowStat);
	GetDlgItem(IDC_BTN_EXIT)->ShowWindow(iShowStat);
	
	GetDlgItem(IDC_STATIC_VERSION)->ShowWindow(iShowStat);
	//GetDlgItem(IDC_STATIC_SYS)->ShowWindow(iShowStat);
	GetDlgItem(IDC_STATIC_DEMO_VERSION)->ShowWindow(iShowStat);
	GetDlgItem(IDC_STATIC_SDK_VERSION)->ShowWindow(iShowStat);
	GetDlgItem(IDC_STATIC_PLAYM4_VERSION)->ShowWindow(iShowStat);
	
	if (bFullScreen)
	{
		//for full screen while backplay
		GetWindowPlacement(&m_struOldWndpl);
		
		CRect rectWholeDlg;//entire client(including title bar)
		CRect rectClient;//client area(not including title bar)
		CRect rectFullScreen;
		GetWindowRect(&rectWholeDlg);
		RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery, &rectClient);
		ClientToScreen(&rectClient);
		
		rectFullScreen.left = rectWholeDlg.left-rectClient.left;
		rectFullScreen.top = rectWholeDlg.top-rectClient.top;
		rectFullScreen.right = rectWholeDlg.right+g_iCurScreenWidth - rectClient.right;
		rectFullScreen.bottom = rectWholeDlg.bottom+g_iCurScreenHeight - rectClient.bottom;
		//enter into full screen;
		WINDOWPLACEMENT struWndpl;
		struWndpl.length = sizeof(WINDOWPLACEMENT);
		struWndpl.flags = 0;
		struWndpl.showCmd = SW_SHOWNORMAL;
		struWndpl.rcNormalPosition = rectFullScreen;
		SetWindowPlacement(&struWndpl);
	}
	else
	{
		SetWindowPlacement(&m_struOldWndpl);
	}
	
	if (m_iMainType == PREVIEW_T)//dealing while preview
	{
		//refresh backgroud box
		if (bFullScreen)
		{	
			GetDlgItem(IDC_STATIC_PREVIEWBG)->MoveWindow(0,0,g_iCurScreenWidth,g_iCurScreenHeight,true);	
		}
		else
		{
			GetDlgItem(IDC_STATIC_PREVIEWBG)->MoveWindow(&m_rectPreviewBG,true);	
		}
		PreviewReferShow(!bFullScreen);
		GetDlgItem(IDC_STATIC_PREVIEWBG)->ShowWindow(SW_SHOW);
		
		if (g_struLocalParam.bFullScreen)
		{
			if (g_struLocalParam.bUseCard)
			{
				NET_DVR_DISPLAY_PARA struDisPlayParam;
				struDisPlayParam.bToVideoOut = 1;
				struDisPlayParam.bToScreen = 1;
				struDisPlayParam.nLeft = 0;
				struDisPlayParam.nTop = 0;
				struDisPlayParam.nWidth = g_iCurScreenWidth;
				struDisPlayParam.nHeight = g_iCurScreenHeight;
				NET_DVR_ResetPara_Card(g_dlgOutput[m_iCurWndIndex].m_lPlayHandle,&struDisPlayParam);
				g_dlgOutput[m_iCurWndIndex].MoveWindow(0,0,g_iCurScreenWidth, g_iCurScreenHeight, TRUE);
				g_dlgOutput[m_iCurWndIndex].ShowWindow(SW_SHOW);
			}
			else
			{
				g_dlgOutput[m_iCurWndIndex].MoveWindow(0,0,g_iCurScreenWidth, g_iCurScreenHeight, TRUE);
				g_dlgOutput[m_iCurWndIndex].ShowWindow(SW_SHOW);
			}
		}
	}
	if (bFullScreen)
	{
		m_listAlarmInfo.ShowWindow(SW_HIDE);
		m_listAllLog.ShowWindow(SW_HIDE);
	}
	else
	{
		OnSelchangeComboListType();
	}
}


/*********************************************************
Function:	OnNMClickTreeDeviceList
Desc:		click tree node response function, type of node; device node, log in operation, clear device and its related alarm
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnNMClickTreeDeviceList(NMHDR *pNMHDR, LRESULT *pResult)
{
	//get node handle through mouse position
	CPoint pt(0,0);
	GetCursorPos(&pt);
	
	CRect rect(0,0,0,0);
	ScreenToClient(&pt);
	GetDlgItem(IDC_TREE_DEVICE_LIST)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	
	pt.x = pt.x-rect.left;
	pt.y = pt.y-rect.top;
	
	UINT uFlags = 0;
	HTREEITEM hSelect = m_treeDeviceList.HitTest(pt,&uFlags);
	if (hSelect == NULL)
	{
		AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "please click the right node!");
		return;
	}
	BOOL bLogined = FALSE;
	DWORD dwNoteData = m_treeDeviceList.GetItemData(hSelect);
	int iType = dwNoteData/1000;
	int iIndex = dwNoteData%1000;
	if (DEVICETYPE == iType)
	{
		m_iCurDeviceIndex = iIndex;
		m_hCurDeviceItem = hSelect;
		m_iCurChanIndex = -1;
		m_hCurChanItem = NULL;
		bLogined = DoLogin(m_iCurDeviceIndex, hSelect);
	}
	else if (CHANNELTYPE == iType)
	{
		m_iCurChanIndex = iIndex;
		m_hCurChanItem = hSelect;
		HTREEITEM hParent = m_treeDeviceList.GetParentItem(hSelect);
		if (hParent != NULL)
		{
			dwNoteData = m_treeDeviceList.GetItemData(hParent);
			iType = dwNoteData/1000;
			iIndex = dwNoteData%1000;
			if (DEVICETYPE == iType)
			{
				m_iCurDeviceIndex = iIndex;
				m_hCurDeviceItem = hParent;
				bLogined = DoLogin(m_iCurDeviceIndex, hParent);
			}
		}
		
	}
	else
	{
		m_iCurDeviceIndex = -1;
		m_hCurDeviceItem = NULL;
		m_iCurChanIndex = -1;
		m_hCurChanItem = NULL;
	}
	
	if (m_iMainType == CONFIG_ALL_T)
	{
		m_dlgConfigAll->ConfigWndUpdate();
	}
	else if (m_iMainType == PLAY_BACK_T)
	{
		m_dlgPlayBack->PlayBackWinUpdate();
	}
	else if (m_iMainType == PRODUCT_RELATE_T)
	{
		m_dlgProductCfg->UpdateWindowParam(GetCurDeviceIndex(),GetCurChanIndex());
	}
	
	//clear alarm status on manual
	if (bLogined)
	{
		ManualClearAlarm(m_iCurDeviceIndex, m_iCurChanIndex);
	}
	
	*pResult = 0;
}


/*********************************************************
Function:	OnNMDblclkTreeDeviceList
Desc:		double click tree node response function, play one channel or whole device
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnNMDblclkTreeDeviceList(NMHDR *pNMHDR, LRESULT *pResult)
{
	HTREEITEM hSelect = m_treeDeviceList.GetSelectedItem();
	if (NULL == hSelect || m_iMainType != PREVIEW_T)//it works to double click device tree only while preview
	{
		return;
	}
	
	if (g_bPlayAllDevice)//disable to single designated play while all play
	{
		char szLan[128] = {0};
		g_StringLanType(szLan, "����ֹͣȫ������", "Please stop all play first");
		AfxMessageBox(szLan);
		return;
	}
	
	DWORD dwNodeData = (DWORD)m_treeDeviceList.GetItemData(hSelect);
	HTREEITEM hParent = NULL;
	int iType = dwNodeData/1000;
	int iIndex = dwNodeData%1000;
	
	switch (iType)
	{
	case TREE_ALL_T:
		DblAllTree();
		break;
	case DEVICETYPE:
		m_iCurDeviceIndex = iIndex;
		m_hCurDeviceItem = hSelect;
		m_iCurChanIndex = -1;
		m_hCurChanItem = NULL;
		if (GetCurDeviceIndex() != -1)
		{
			DblPlayDevice(m_iCurDeviceIndex, m_iCurWndIndex);
		}		
		m_treeDeviceList.Expand(hSelect, TVE_COLLAPSE);//expend reverse operation
		//	m_treeDeviceList.Expand(hSelect,TVE_EXPAND);
		break;
	case CHANNELTYPE:  //double click channel
		m_iCurChanIndex = iIndex;
		m_hCurChanItem = hSelect;
		hParent = m_treeDeviceList.GetParentItem(hSelect);
		if (hParent != NULL)
		{
			if (DEVICETYPE == m_treeDeviceList.GetItemData(hParent)/1000)
			{
				m_iCurDeviceIndex = m_treeDeviceList.GetItemData(hParent)%1000;
				m_hCurDeviceItem = hParent;
			}
		}
        if (iIndex < ZERO_CHAN_INDEX)
        {
            DblPlayChan(m_iCurDeviceIndex,iIndex,hSelect);
        }
        else
        {
            PlayZeroChan(m_iCurDeviceIndex,iIndex,hSelect);
        }
		
		break;
	default:
		{
			m_iCurChanIndex = -1;
			m_hCurChanItem = NULL;
			m_iCurDeviceIndex = -1;
			m_hCurDeviceItem = NULL;
		}
		break;
	}
	*pResult = 0;
}

/*********************************************************
Function:	OnNMRclickTreeDeviceList
Desc:		right-click tree node, pop up related menu
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnNMRclickTreeDeviceList(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu pMenu;
	HTREEITEM hParent=NULL;
	CPoint pt(0,0);
	GetCursorPos(&pt);
	HTREEITEM hRoot = m_treeDeviceList.GetRootItem();
	if (hRoot == NULL)
	{
		if (!pMenu.LoadMenu(IDR_MENU_TREE_ALL))
		{
			return;
		}
	}
	else
	{
		CRect rect(0,0,0,0);
		ScreenToClient(&pt);
		GetDlgItem(IDC_TREE_DEVICE_LIST)->GetWindowRect(&rect);
		ScreenToClient(&rect);
		
		pt.x = pt.x-rect.left;
		pt.y = pt.y-rect.top;
		//while select first node or blank
		//HTREEITEM hSelect = m_treeDeviceList.GetSelectedItem();
		UINT uFlags = 0;
		HTREEITEM hSelect = m_treeDeviceList.HitTest(pt,&uFlags);
		if (hSelect == NULL )//|| m_iMainType != PREVIEW_T
		{
			return;
		}
		
		m_treeDeviceList.SelectItem(hSelect);
		
		int iType = int(m_treeDeviceList.GetItemData(hSelect) / 1000);
		
		switch (iType)
		{
		case TREE_ALL_T:
			// 			if (!pMenu.LoadMenu(IDR_MENU_TREE_ALL))
			// 			{
			// 				return;
			// 			}
			OnMenuDeviceAdd();
			return;
			break;
		case DEVICETYPE:
			m_iCurDeviceIndex = int(m_treeDeviceList.GetItemData(hSelect) % 1000);
			m_hCurDeviceItem = hSelect;
			m_iCurChanIndex = -1;
			m_hCurChanItem = NULL;
			if (!pMenu.LoadMenu(IDR_MENU_DEVICE))
			{
				return;
			}
			break;
		case CHANNELTYPE:
			m_iCurChanIndex = int(m_treeDeviceList.GetItemData(hSelect) % 1000);
			m_hCurChanItem = hSelect;
			
			hParent = m_treeDeviceList.GetParentItem(hSelect);
			if (hParent != NULL)
			{
				if (DEVICETYPE == m_treeDeviceList.GetItemData(hParent)/1000)
				{
					m_iCurDeviceIndex = m_treeDeviceList.GetItemData(hParent)%1000;
					m_hCurDeviceItem = hParent;
				}
			}
			if (!pMenu.LoadMenu(IDR_MENU_CHANNEL))
			{
				return;
			}
			
			break;
		default:
			break;
		}
	}
	//display menu
	GetCursorPos(&pt);
	pMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN,pt.x ,pt.y,this);	
	*pResult = 0;
}

/*********************************************************
Function:	GetAfterItem
Desc:		get device insert point, make sure new device insert afterword last node
Input:	hRoot:handle of item tree root
Output:	
Return:	
**********************************************************/
HTREEITEM CClientDemoDlg::GetAfterItem(HTREEITEM hRoot)
{
	HTREEITEM hReturn = TVI_FIRST;
	HTREEITEM hChild = m_treeDeviceList.GetChildItem(hRoot);
	
	while (hChild)
	{
		if (TREE_ALL_T ==  m_treeDeviceList.GetItemData(hChild) / 1000)   //break if it si device tree node
		{
			break;
		}
		
		hReturn = hChild;
		hChild = m_treeDeviceList.GetNextSiblingItem(hChild);
	}
	
	return hReturn;
}

/*********************************************************
Function:	OnMenuDeviceAdd
Desc:		menu,device adding response 
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuDeviceAdd()
{
	int i = 0;
    int j = 0;
	for (i = 0; i < MAX_DEVICES; i++)
	{
		if (-1 == g_struDeviceInfo[i].iDeviceChanNum)
		{
			break;
		}
	}
	char szLan[128] = {0};
	if (i == MAX_DEVICES)
	{
		g_StringLanType(szLan, "����ӵ��豸�Ѵﵽ�����", "Added device has reached the maximum number");
		AfxMessageBox(szLan);
		return;
	}
	
	HTREEITEM hSelect = m_treeDeviceList.GetSelectedItem();
	
	for (i = 0; i < MAX_DEVICES; i++)
	{
		if (g_struDeviceInfo[i].iDeviceChanNum == -1)
		{
			CDlgAddDevice dlg;
            dlg.m_iDeviceIndex = i;
            //memset(&g_struDeviceInfo[i], 0, sizeof(STRU_DEVICE_INFO));
            if (dlg.DoModal() == IDOK)
            {
                HTREEITEM hDevice = m_treeDeviceList.InsertItem(g_struDeviceInfo[i].chLocalNodeName, DEVICE_LOGIN, DEVICE_LOGIN,hSelect,GetAfterItem(hSelect));
                m_treeDeviceList.SetItemData(hDevice, DEVICETYPE * 1000 + g_struDeviceInfo[i].iDeviceIndex);//
                HTREEITEM hChannel = NULL;

				//analog channel & IP channel
                for (j= 0; j < MAX_CHANNUM_V40; j++)
                {
					if (g_struDeviceInfo[i].pStruChanInfo[j].iChanIndex != -1 )
					{
						 if (g_struDeviceInfo[i].pStruChanInfo[j].bEnable)
                         {
                             hChannel = m_treeDeviceList.InsertItem(g_struDeviceInfo[i].pStruChanInfo[j].chChanName,CHAN_ORIGINAL,CHAN_ORIGINAL,hDevice);
						 }
						 else
						 {
							 hChannel = m_treeDeviceList.InsertItem(g_struDeviceInfo[i].pStruChanInfo[j].chChanName,CHAN_OFF_LINE,CHAN_OFF_LINE,hDevice);                            
                         }

						 m_treeDeviceList.SetItemData(hChannel, CHANNELTYPE * 1000 + g_struDeviceInfo[i].pStruChanInfo[j].iChanIndex);
					}  
                }

				//zero
                for (j= 0; j < g_struDeviceInfo[i].byZeroChanNum; j++)
                {
                    hChannel = m_treeDeviceList.InsertItem(g_struDeviceInfo[i].struZeroChan[j].chChanName,CHAN_ORIGINAL,CHAN_ORIGINAL,hDevice);
                    m_treeDeviceList.SetItemData(hChannel, CHANNELTYPE * 1000 + g_struDeviceInfo[i].struZeroChan[j].iChanIndex);
                }
                m_iCurDeviceIndex = dlg.m_iDeviceIndex;
                m_treeDeviceList.Expand(hSelect,TVE_EXPAND);
                m_treeDeviceList.Expand(hDevice,TVE_EXPAND);
                ::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_DEVICE_ITEM_IMAGE, WPARAM(hDevice), LPARAM(i));
            }
            return;
        }
    }
    g_StringLanType(szLan, "��������豸��", "Exceeds the maximum number of Devices");
    AfxMessageBox(szLan);
}

/*********************************************************
Function:	DblPlayChan
Desc:		double click playing channel, stop if it is already playing
Input:		hChanItem,pic switch wile play status changes;iChanIndex:channel index;hChanItem,channel node pointer;
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::DblPlayChan(int iDeviceIndex, int iChanIndex,HTREEITEM hChanItem)
{
	if (!DoLogin(iDeviceIndex, m_treeDeviceList.GetParentItem(hChanItem)))
	{
		return;
	}
	char szLan[128] = {0};
	if (g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].lRealHandle < 0)
	{
		CHANNEL_INFO pStruChanInfo;
		memcpy(&pStruChanInfo, &g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex], sizeof(CHANNEL_INFO));
		
		if (g_dlgOutput[m_iCurWndIndex].StartPlay(&pStruChanInfo,hChanItem)<0)
		{
			g_StringLanType(szLan, "Ԥ��ʧ�ܣ�", "Preview Failed");
			AfxMessageBox(szLan);
		}
		
		ChangePlayBtnState();
		return;
	}
	
	//if the channel is already previewed, stop it
	if (!StopPlayedChan(iDeviceIndex,iChanIndex))
	{
		g_StringLanType(szLan, "ֹͣԤ��ʧ�ܣ�", "Stop previewing Failed");
		AfxMessageBox(szLan);
	}
	ChangePlayBtnState();
}


/*********************************************************
Function:	PlayZeroChan
Desc:		double click playing channel, stop if it is already playing
Input:		hChanItem,pic switch wile play status changes;iChanIndex:channel index;hChanItem,channel node pointer;
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::PlayZeroChan(int iDeviceIndex, int iChanIndex,HTREEITEM hChanItem)
{
    if (!DoLogin(iDeviceIndex, m_treeDeviceList.GetParentItem(hChanItem)))
    {
        return;
    }
    char szLan[128] = {0};
    
    if (g_struDeviceInfo[iDeviceIndex].struZeroChan[iChanIndex-ZERO_CHAN_INDEX].lRealHandle < 0)
    {
        CHANNEL_INFO struZeroChan;
        memcpy(&struZeroChan, &g_struDeviceInfo[iDeviceIndex].struZeroChan[iChanIndex-ZERO_CHAN_INDEX], sizeof(CHANNEL_INFO));
        
        if (g_dlgOutput[m_iCurWndIndex].ZeroStartPlay(&struZeroChan,hChanItem)<0)
        {
            g_StringLanType(szLan, "Ԥ��ʧ�ܣ�", "Preview Failed");
            //AddLog(iDeviceIndex, OPERATION_FAIL_T, "Zero play chan", iChanIndex-ZERO_CHAN_INDEX);
            AfxMessageBox(szLan);
        }
        
        ChangePlayBtnState();
        return;
    }
    
    //if the channel is already previewed, stop it
    if (!StopPlayedChan(iDeviceIndex,iChanIndex))
    {
        g_StringLanType(szLan, "ֹͣԤ��ʧ�ܣ�", "Stop previewing Failed");
        AfxMessageBox(szLan);
    }
    ChangePlayBtnState();
}

/*********************************************************
Function:	OnMenuLogout
Desc:		menu on logout response function
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuLogout()
{
	int iDeviceIndex = GetCurDeviceIndex();
	
	if (iDeviceIndex == -1)
	{
		return;
	}
	HTREEITEM hDeviceItem = m_treeDeviceList.GetSelectedItem();
	if (hDeviceItem == NULL)
	{
		AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "CClientDemoDlg::OnMenuLogout: hDeviceItem err\n");
		return;
	}
	DoLogout(iDeviceIndex, hDeviceItem);
	ManualClearAlarm(iDeviceIndex, -1);
}

/*********************************************************
Function:	DoLogout
Desc:		logout
Input:		iDeviceIndex:device serial number;hDeviceItem,device pointer;
Output:		
Return:		TRUE,log out successfullyFALSE,log out failed
**********************************************************/
BOOL CClientDemoDlg::DoLogout(int iDeviceIndex, HTREEITEM hDeviceItem)
{
	int i = 0;
	char szLan[128] = {0};
	g_StringLanType(szLan, "���豸��ͨ������Ԥ����¼��", "Some channels of this device is recording or preview");
	for (i = 0; i<g_struDeviceInfo[iDeviceIndex].iDeviceChanNum; i++)
	{
		if ((g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].lRealHandle >= 0 && i < MAX_CHANNUM_V30*2)|| g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bLocalManualRec )
		{
			AfxMessageBox(szLan);
			return FALSE;
		}
	}
	// 2011-11-16 silujie
	g_StringLanType(szLan, "���豸��ͨ�����ڱ�������", "some channels of this device is passive decode");
	for(i =0; i<256; i++)
	{
		if (g_struDeviceInfo[iDeviceIndex].struPassiveDecode[i].lPassiveHandle >=0 || g_struDeviceInfo[iDeviceIndex].struPassiveDecode[i].hFileThread != NULL)
		{
			AfxMessageBox(szLan);
			return FALSE;
		}
	}
	g_StringLanType(szLan, "���豸��ͨ�����������Խ�", "Some channels of this device is voice talk");
	if (g_lVoiceHandle >= 0)
	{
		AfxMessageBox(szLan);
		return FALSE;
	}
	if (g_struDeviceInfo[iDeviceIndex].lLoginID >= 0)
	{
		//remove guard
		if (g_struDeviceInfo[iDeviceIndex].lFortifyHandle >= 0)
		{
			if (NET_DVR_CloseAlarmChan_V30(g_struDeviceInfo[iDeviceIndex].lFortifyHandle))
			{
				g_struDeviceInfo[iDeviceIndex].lFortifyHandle = -1;	
			}
		}
		
		if (!NET_DVR_Logout_V30(g_struDeviceInfo[iDeviceIndex].lLoginID))
		{
			//g_struDeviceInfo[iDeviceIndex].lLoginID = -1;
			g_StringLanType(szLan, "ע��ʧ��", "Logout Failed");
			AfxMessageBox(szLan);
			::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_DEVICE_ITEM_IMAGE, WPARAM(hDeviceItem), LPARAM(iDeviceIndex));
			return FALSE;
		}
	}
	AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_Logout_V30");
	g_struDeviceInfo[iDeviceIndex].lLoginID	    = -1;
	g_struDeviceInfo[iDeviceIndex].bPlayDevice  = FALSE;	
	
	for (i = 0; i < g_struDeviceInfo[iDeviceIndex].iDeviceChanNum; i++)
	{
		g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].lRealHandle = -1;
		g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = FALSE;
		g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bLocalManualRec = FALSE;
	}
	::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_DEVICE_ITEM_IMAGE, WPARAM(hDeviceItem), LPARAM(iDeviceIndex));
	return TRUE;
}


/*********************************************************
Function:	OnMenuDeviceDelete
Desc:		delete devices, and initialize structure
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuDeviceDelete()
{
	char szLan[128] = {0};
	g_StringLanType(szLan, "ȷ��Ҫɾ���ý��", "Sure to delete this node");
	
	if (MessageBox(szLan,"Warning",MB_OKCANCEL) == IDOK)
	{
		int iDeviceIndex = GetCurDeviceIndex();
		HTREEITEM hDeviceItem = m_treeDeviceList.GetSelectedItem();
		if (DoLogout(iDeviceIndex, hDeviceItem))//��ע������豸
		{
			LOCAL_DEVICE_INFO struDevInfo;
			//memcpy(&(g_struDeviceInfo[iDeviceIndex]), &struDevInfo, sizeof(LOCAL_DEVICE_INFO));
			 sprintf(g_struDeviceInfo[iDeviceIndex].chLoginUserName , "%s", _T(""));
			 sprintf(g_struDeviceInfo[iDeviceIndex].chLoginPwd  , "%s", _T(""));
			 sprintf(g_struDeviceInfo[iDeviceIndex].chLocalNodeName      , "%s", _T(""));
			 sprintf(g_struDeviceInfo[iDeviceIndex].chDeviceIP, "%s", _T(""));
			// sprintf(g_struDeviceInfo[iDeviceIndex].chDevNetCard1IP, "%s", _T(""));
		     sprintf(g_struDeviceInfo[iDeviceIndex].chDeviceMultiIP   , "%s", _T(""));
		     sprintf(g_struDeviceInfo[iDeviceIndex].chSerialNumber    , "%s", _T(""));
		    // sprintf(g_struDeviceInfo[iDeviceIndex].chIPServerIP, "%s", _T(""));
	      	 g_struDeviceInfo[iDeviceIndex].lLoginID	    = -1;
	     	 g_struDeviceInfo[iDeviceIndex].iDeviceChanNum   = -1;
		     g_struDeviceInfo[iDeviceIndex].lDevicePort      = 8000;
	         g_struDeviceInfo[iDeviceIndex].iDeviceType      = 0;
		     g_struDeviceInfo[iDeviceIndex].iDeviceLoginType  = 0;
		
	     	for (int j = 0; j < g_struDeviceInfo[iDeviceIndex].iDeviceChanNum; j++)
			{
		    	sprintf(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[j].chChanName ,"%s", _T(""));
		        g_struDeviceInfo[iDeviceIndex].pStruChanInfo[j].iChanIndex    = -1;
			    g_struDeviceInfo[iDeviceIndex].pStruChanInfo[j].dwProtocol    = 0;
			    g_struDeviceInfo[iDeviceIndex].pStruChanInfo[j].iDeviceIndex  = -1;
			}
			
			m_treeDeviceList.DeleteItem(hDeviceItem);
			g_SaveDeviceConfig();
		}
		
	}
}

/*********************************************************
Function:	OnMenuDeviceInfo
Desc:		menu on device info response function
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuDeviceInfo()
{
	int iDeviceIndex  = GetCurDeviceIndex();
	if (iDeviceIndex == -1 )
	{
		return;
	}
	
	CDlgDeviceInfo dlgDeviceInfo;
	dlgDeviceInfo.m_iDeviceIndex = iDeviceIndex;
	
	if (IDOK == dlgDeviceInfo.DoModal())
	{
		HTREEITEM hSelect = m_treeDeviceList.GetSelectedItem();
		m_treeDeviceList.SetItemText(hSelect,g_struDeviceInfo[iDeviceIndex].chLocalNodeName);
	}	
}


/*********************************************************
Function:	OnMenuJPEGSnatchConfig
Desc:		local 'jpeg' capture pic configuration
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuJPEGSnatchConfig()
{
	
	int iDeviceIndex  = GetCurDeviceIndex();
	if (iDeviceIndex == -1)
	{
		return;
	}
	
	int iChanIndex = GetCurChanIndex();
	if (iChanIndex == -1)
	{
		char szLan[128] = {0};
		g_StringLanType(szLan, "ͨ���Ŵ���", "Channel Error");
		AfxMessageBox(szLan);
		return;
	}
    if (iChanIndex >= ZERO_CHAN_INDEX)
    {
        AfxMessageBox("Zero not support  NET_DVR_CaptureJPEGPicture");
    }
	CDlgJPEGSnatchConfig dlg;
	dlg.m_pChanInfo = &g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex];
	
	dlg.DoModal();
	
}

/*********************************************************
Function:	OnMenuChannelInfo
Desc:		menu on channel info response function
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuChannelInfo()
{
	int iDeviceIndex  = GetCurDeviceIndex();
	if (iDeviceIndex == -1)
	{
		return;
	}
	int iChanIndex = GetCurChanIndex();
	if (iChanIndex == -1)
	{
		char szLan[128] = {0};
		g_StringLanType(szLan, "ͨ���Ŵ���", "Channel Error");
		AfxMessageBox(szLan);
		return;
	}
	CDlgChannelInfo dlgChannelInfo;
    if (iChanIndex >= ZERO_CHAN_INDEX)
    {
        dlgChannelInfo.m_pChanInfo = &g_struDeviceInfo[iDeviceIndex].struZeroChan[iChanIndex - ZERO_CHAN_INDEX];
    }
    else if (iChanIndex >= 0 && iChanIndex <= MAX_CHANNUM_V30)
    {
	dlgChannelInfo.m_pChanInfo = &g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex];
    }
	
	if (IDOK == dlgChannelInfo.DoModal() && dlgChannelInfo.m_bChanged)
	{
		HTREEITEM hSelect = m_treeDeviceList.GetSelectedItem();
        if (iChanIndex >= ZERO_CHAN_INDEX)
        {
            m_treeDeviceList.SetItemText(hSelect,g_struDeviceInfo[iDeviceIndex].struZeroChan[iChanIndex-ZERO_CHAN_INDEX].chChanName);
        }
        else
        {
		m_treeDeviceList.SetItemText(hSelect,g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].chChanName);
	}	
        
    }	
}

/*********************************************************
Function:	ChangeChannelItemImage
Desc:		change status icon related to channel node, including play, record, alarm status
Input:	wParam, tree channel pointer;lParam,Pointer to channel structure;
Output:	
Return:	0 succeed
**********************************************************/
LRESULT CClientDemoDlg::ChangeChannelItemImage(WPARAM wParam, LPARAM lParam)
{
	int iDeviceIndex = int(wParam);
	int iChanIndex = int(lParam);
	if (iDeviceIndex < 0 || iDeviceIndex >MAX_DEVICES\
        || iChanIndex > 512 || iChanIndex < 0)
	{
		OutputDebugString("dev index and chan num err!\n");
		return NULL;
	}
	
	HTREEITEM hChanItem = GetChanItem(iDeviceIndex, iChanIndex);
	if (NULL == hChanItem)
	{
		OutputDebugString("chan item NULL!\n");
		return NULL;
	}
	int iImage = CHAN_ORIGINAL;
	CHANNEL_INFO pStruChanInfo;
	BOOL bPlaying = FALSE;
    if (iChanIndex >= ZERO_CHAN_INDEX)
    {
        memcpy(&pStruChanInfo, &g_struDeviceInfo[iDeviceIndex].struZeroChan[iChanIndex - ZERO_CHAN_INDEX], sizeof(pStruChanInfo));
    }
    else
    {
        memcpy(&pStruChanInfo, &g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex], sizeof(pStruChanInfo));
    }
    
	
	bPlaying = pStruChanInfo.lRealHandle >= 0? TRUE:FALSE;
	if (bPlaying && !pStruChanInfo.bLocalManualRec && !pStruChanInfo.bAlarm)
	{
		iImage = CHAN_PLAY;	
	}
	else if ( bPlaying && pStruChanInfo.bLocalManualRec && !pStruChanInfo.bAlarm)
	{
		iImage = CHAN_PLAY_RECORD;	
	}
	else if (!bPlaying && pStruChanInfo.bLocalManualRec && !pStruChanInfo.bAlarm)
	{
		iImage = CHAN_RECORD;	
	}
	else if (!bPlaying && !pStruChanInfo.bLocalManualRec && pStruChanInfo.bAlarm)
	{
		iImage = CHAN_ALARM;	
	}
	else if (bPlaying && !pStruChanInfo.bLocalManualRec && pStruChanInfo.bAlarm)
	{
		iImage = CHAN_PLAY_ALARM;
	}
	else if (bPlaying && pStruChanInfo.bLocalManualRec && pStruChanInfo.bAlarm)
	{
		iImage = CHAN_PLAY_RECORD_ALARM;	
	}
	else if (!bPlaying && !pStruChanInfo.bLocalManualRec && !pStruChanInfo.bAlarm)
	{
		iImage = CHAN_ORIGINAL;	
	}
	
	m_treeDeviceList.SetItemImage(hChanItem,iImage,iImage);	
	//modify status!!
    if (iChanIndex >= ZERO_CHAN_INDEX)
    {
		g_struDeviceInfo[pStruChanInfo.iDeviceIndex].struZeroChan[iChanIndex - ZERO_CHAN_INDEX].dwImageType = iImage;
    }
    else
    {
	g_struDeviceInfo[pStruChanInfo.iDeviceIndex].pStruChanInfo[pStruChanInfo.iChanIndex].dwImageType = iImage;
    }
    
	return 0;
}

/*********************************************************
Function:	ChangeDeviceItemImage
Desc:		�豸�ڵ�״̬�ı����Ϣ����,ά���豸"��½","����","�澯"��״̬
Input:	wParam,���豸�ڵ��ָ��;lParam,�豸�����к�;
Output:	
Return:	0 �ɹ�
**********************************************************/
LRESULT CClientDemoDlg::ChangeDeviceItemImage(WPARAM wParam, LPARAM lParam)
{
	HTREEITEM hDeviceItem     = HTREEITEM(wParam);
	int iDeviceIndex = int(lParam);
	
	if (NULL == hDeviceItem || iDeviceIndex < 0)
	{
		AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "change dev item");
		return NULL;
	}
	int iImage = DEVICE_LOGOUT;
	if (g_struDeviceInfo[iDeviceIndex].lLoginID >= 0 \
		&& g_struDeviceInfo[iDeviceIndex].lFortifyHandle < 0&& !g_struDeviceInfo[iDeviceIndex].bAlarm)
	{
		iImage = DEVICE_LOGIN;	
	}
	else if (g_struDeviceInfo[iDeviceIndex].lLoginID >= 0 \
		&& g_struDeviceInfo[iDeviceIndex].lFortifyHandle >= 0&& !g_struDeviceInfo[iDeviceIndex].bAlarm)
	{
		iImage = DEVICE_FORTIFY;
	}
	else if (g_struDeviceInfo[iDeviceIndex].lLoginID >= 0 \
		&& g_struDeviceInfo[iDeviceIndex].lFortifyHandle >= 0 && g_struDeviceInfo[iDeviceIndex].bAlarm)
	{
		iImage = DEVICE_FORTIFY_ALARM;
	}
	else if (g_struDeviceInfo[iDeviceIndex].lLoginID >= 0 \
		&& g_struDeviceInfo[iDeviceIndex].lFortifyHandle < 0 && g_struDeviceInfo[iDeviceIndex].bAlarm)
	{
		iImage = DEVICE_ALARM;
	}
	else
	{
		iImage = DEVICE_LOGOUT;
	}
	
	m_treeDeviceList.SetItemImage(hDeviceItem,iImage,iImage);	
	g_struDeviceInfo[iDeviceIndex].dwImageType = iImage;
	return 0;
}
typedef struct tagLOCAL_LOGIN_PARAM
{
    int iDeviceIndex;
	HTREEITEM hSelect;
	BOOL bMsgBox;
}LOCAL_LOGIN_PARAM, *LPLOCAL_LOGIN_PARAM;

/*********************************************************
Function:	DoLogin
Desc:		check the device has logged in, if not, log in 
Input:	iDeviceIndex,device serial number ;hDeviceItem pointer to device node
Output:	
Return:	FALSE,device log in failed;TRUE; logged in;
**********************************************************/
void *CClientDemoDlg::DoLoginEx(void *lpParam)
{
	if (lpParam == NULL)
	{
		return FALSE;
	}
	int iDeviceIndex = ((LPLOCAL_LOGIN_PARAM)lpParam)->iDeviceIndex;
	HTREEITEM hDeviceItem = ((LPLOCAL_LOGIN_PARAM)lpParam)->hSelect;
	BOOL bMsgBox = ((LPLOCAL_LOGIN_PARAM)lpParam)->bMsgBox;
	
	delete lpParam;
	lpParam = NULL;
	
	NET_DVR_NETCFG_V30* pStruNetCfg = new  NET_DVR_NETCFG_V30;
	NET_DVR_DEVICECFG_V40* pStruDevCfg = new NET_DVR_DEVICECFG_V40;
	
	if (pStruNetCfg == NULL || pStruDevCfg == NULL)
	{
		return NULL;
	}
	
	DWORD dwReturned = 0;
	CString csTemp;
	int i = 0;
	if (g_struDeviceInfo[iDeviceIndex].lLoginID < 0)
	{
		LONG lUserID = -1;
		NET_DVR_DEVICEINFO_V30 struDeviceInfo;
		//		char szDeviceSN[SERIALNO_LEN] = {0};
		memset(&struDeviceInfo, 0, sizeof(NET_DVR_DEVICEINFO_V30));
		
		lUserID = NET_DVR_Login_V30(g_struDeviceInfo[iDeviceIndex].chDeviceIP, (WORD)g_struDeviceInfo[iDeviceIndex].lDevicePort,\
			g_struDeviceInfo[iDeviceIndex].chLoginUserName, g_struDeviceInfo[iDeviceIndex].chLoginPwd, &struDeviceInfo);
		if (lUserID < 0)
		{
			char szLan[1024] = {0};
			g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_Login_V30");
			if (bMsgBox)
			{
				if (NET_DVR_GetLastError() == NET_DVR_PASSWORD_ERROR)
				{
					g_StringLanType(szLan, "�û����������!", "user name or password error!");
					AfxMessageBox(szLan);
				}
				else
				{				
					g_StringLanType(szLan, "��������ԭ���DVRæ, ע��ʧ��!", "net error or dvr is busy!");
					AfxMessageBox(szLan);
				}
			}
			if (pStruNetCfg != NULL)
            {
                delete pStruNetCfg ;
                pStruNetCfg = NULL;
            }
			
            if (pStruDevCfg != NULL)
            {
                delete pStruDevCfg;
                pStruDevCfg = NULL;
            }
			return NULL;
		}
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_Login_V30");
		LOCAL_DEVICE_INFO temp, temp1;
		//initialize the information of device
		memcpy(&temp1, &(g_struDeviceInfo[iDeviceIndex]), sizeof(LOCAL_DEVICE_INFO));
		//memcpy(&(g_struDeviceInfo[iDeviceIndex]), &temp, sizeof(LOCAL_DEVICE_INFO));
		
		g_struDeviceInfo[iDeviceIndex].lLoginID = lUserID;// located ID from re-log in
		g_struDeviceInfo[iDeviceIndex].iDeviceIndex = iDeviceIndex;
		sprintf(g_struDeviceInfo[iDeviceIndex].chDeviceIP, "%s", temp1.chDeviceIP);
		
		//if the device ip is in ipv6 version, change the ':' to '.' ,
		//for ':' is not allowed in Windows OS's file name 
		char sIPV6Addr[130] = {0};
		int i = 0; 
		for (;i < sizeof(g_struDeviceInfo[iDeviceIndex].chDeviceIP); i ++)
		{
			if(g_struDeviceInfo[iDeviceIndex].chDeviceIP[i] == '\0')
			{
				break;
			}
			else if(g_struDeviceInfo[iDeviceIndex].chDeviceIP[i] == ':')
			{
				sIPV6Addr[i] = '.';
			}
			else
			{
				sIPV6Addr[i] = g_struDeviceInfo[iDeviceIndex].chDeviceIP[i];
			}
		}
		strncpy(g_struDeviceInfo[iDeviceIndex].chDeviceIPInFileName, sIPV6Addr, sizeof(sIPV6Addr));

		sprintf(g_struDeviceInfo[iDeviceIndex].chLocalNodeName, "%s", temp1.chLocalNodeName);
		g_struDeviceInfo[iDeviceIndex].lDevicePort = temp1.lDevicePort;
		
		sprintf(g_struDeviceInfo[iDeviceIndex].chLoginPwd, "%s", temp1.chLoginPwd);
		sprintf(g_struDeviceInfo[iDeviceIndex].chLoginUserName, "%s", temp1.chLoginUserName);
		sprintf(g_struDeviceInfo[iDeviceIndex].chLoginUserName, "%s", temp1.chLoginUserName);
		
		g_struDeviceInfo[iDeviceIndex].dwImageType = DEVICE_LOGIN;
		g_struDeviceInfo[iDeviceIndex].lLoginID = lUserID;
		sprintf(g_struDeviceInfo[iDeviceIndex].chSerialNumber, "%s", struDeviceInfo.sSerialNumber);
		g_struDeviceInfo[iDeviceIndex].iDeviceType = struDeviceInfo.wDevType;  //��չ�ֶα�ʾ���ͣ�ԭΪbyDVRType
		g_struDeviceInfo[iDeviceIndex].iDeviceChanNum = struDeviceInfo.byChanNum + struDeviceInfo.byIPChanNum+ struDeviceInfo.byHighDChanNum *256;
		g_struDeviceInfo[iDeviceIndex].iStartChan = struDeviceInfo.byStartChan;
		g_struDeviceInfo[iDeviceIndex].iDiskNum = struDeviceInfo.byDiskNum;
		g_struDeviceInfo[iDeviceIndex].iAlarmOutNum = struDeviceInfo.byAlarmOutPortNum;
		g_struDeviceInfo[iDeviceIndex].iAlarmInNum = struDeviceInfo.byAlarmInPortNum;
		g_struDeviceInfo[iDeviceIndex].iAudioNum = struDeviceInfo.byAudioChanNum;
		g_struDeviceInfo[iDeviceIndex].iAnalogChanNum = struDeviceInfo.byChanNum;
		g_struDeviceInfo[iDeviceIndex].iIPChanNum = struDeviceInfo.byIPChanNum + struDeviceInfo.byHighDChanNum*256;
        g_struDeviceInfo[iDeviceIndex].byZeroChanNum = struDeviceInfo.byZeroChanNum;
		g_struDeviceInfo[iDeviceIndex].byStartDTalkChan = struDeviceInfo.byStartDTalkChan;
        g_struDeviceInfo[iDeviceIndex].byLanguageType = struDeviceInfo.byLanguageType;

        if (struDeviceInfo.bySupport & 0x80)
		{
			g_struDeviceInfo[iDeviceIndex].byMainProto = struDeviceInfo.byMainProto + 2;
			g_struDeviceInfo[iDeviceIndex].bySubProto = struDeviceInfo.bySubProto + 2;
		}
		else
		{
			g_struDeviceInfo[iDeviceIndex].byMainProto = struDeviceInfo.byMainProto;
			g_struDeviceInfo[iDeviceIndex].bySubProto = struDeviceInfo.bySubProto;
		}  
		
        g_struDeviceInfo[iDeviceIndex].bySupport = struDeviceInfo.bySupport;
		g_struDeviceInfo[iDeviceIndex].bySupport1 = struDeviceInfo.bySupport1;
        g_struDeviceInfo[iDeviceIndex].bySupport2 = struDeviceInfo.bySupport2;

		for (i=0; i<g_struDeviceInfo[iDeviceIndex].iDeviceChanNum; i++)
		{
			g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].dwProtocol = temp1.pStruChanInfo[i].dwProtocol;
			g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iPicResolution = temp1.pStruChanInfo[i].iPicResolution;
			g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iPicQuality = temp1.pStruChanInfo[i].iPicQuality;
			g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].dwPreviewMode = temp1.pStruChanInfo[i].dwPreviewMode;
			g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].nPreviewProtocolType = temp1.pStruChanInfo[i].nPreviewProtocolType;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChannelNO = temp1.pStruChanInfo[i].iChannelNO;
		}

        for (i = 0; i <  g_struDeviceInfo[iDeviceIndex].iAnalogChanNum; i++)
        {
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iDeviceIndex = iDeviceIndex;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChanIndex = i;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChannelNO = i + g_struDeviceInfo[iDeviceIndex].iStartChan;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bEnable = TRUE;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChanType = DEMO_CHANNEL_TYPE_ANALOG;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].dwImageType = CHAN_ORIGINAL;
            sprintf(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName, ANALOG_C_FORMAT, i+g_struDeviceInfo[iDeviceIndex].iStartChan);	
        }

        
        if (g_struDeviceInfo[iDeviceIndex].byZeroChanNum > 0)
        {
            for (i=0; i<g_struDeviceInfo[iDeviceIndex].byZeroChanNum; i++)
            {
                g_struDeviceInfo[iDeviceIndex].struZeroChan[i].iDeviceIndex = iDeviceIndex;
                g_struDeviceInfo[iDeviceIndex].struZeroChan[i].iChanIndex = i+ZERO_CHAN_INDEX;
                sprintf(g_struDeviceInfo[iDeviceIndex].struZeroChan[i].chChanName, "ZeroChan%d", i);	
                //analog devices
                if (1)
                {
                    g_struDeviceInfo[iDeviceIndex].struZeroChan[i].bEnable = TRUE;
                    g_struDeviceInfo[iDeviceIndex].struZeroChan[i].dwImageType = CHAN_ORIGINAL;
                    //				g_struDeviceInfo[iDeviceIndex].iEnableChanNum ++;
                }
                else
                {
                    g_struDeviceInfo[iDeviceIndex].struZeroChan[i].dwImageType = CHAN_OFF_LINE;
                    g_struDeviceInfo[iDeviceIndex].struZeroChan[i].bEnable = FALSE;		
                }
            }
    }

		
		if (!NET_DVR_GetDVRConfig(lUserID, NET_DVR_GET_NETCFG_V30, 0,pStruNetCfg, sizeof(NET_DVR_NETCFG_V30), &dwReturned))
		{
			g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GET_NETCFG_V30");	
		}
		else
		{
			//sprintf(g_struDeviceInfo[m_iDeviceIndex].chDevNetCard1IP, "%s", struNetCfg.struEtherNet[0].struDVRIP.sIpV4);
			
            if (!g_ValidIPv6((BYTE *)(g_struDeviceInfo[iDeviceIndex].chDeviceIP)))
            {
                sprintf(g_struDeviceInfo[iDeviceIndex].chDeviceMultiIP, "%s", pStruNetCfg->struMulticastIpAddr.sIpV4);
                g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "multi-cast ipv4[%s]", pStruNetCfg->struMulticastIpAddr.sIpV4);
			}
            else //ipv6
			{
				sprintf(g_struDeviceInfo[iDeviceIndex].chDeviceMultiIP, "%s", pStruNetCfg->struMulticastIpAddr.byIPv6);
				g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "multi-cast ipv6[%s]", pStruNetCfg->struMulticastIpAddr.byIPv6);
			}
#if 0
			//IPv6�ಥ���ԣ���ʱ�Ե�ַд��
			memset(g_struDeviceInfo[iDeviceIndex].chDeviceMultiIP, 0, sizeof(g_struDeviceInfo[iDeviceIndex].chDeviceMultiIP));
			sprintf(g_struDeviceInfo[iDeviceIndex].chDeviceMultiIP, "ff02::1234");
#endif
		}

// 		string strContent = "<?xml version=\"1.0\"><StorageServer version=\"1.0\"><PCNVRUI>1<PCNVRUI> </StorageServer>";
// 	    NET_DVR_SetTransparentParam(lUserID, (char*)strContent.c_str(), strContent.size());

		if (!NET_DVR_GetDVRConfig(lUserID,NET_DVR_GET_DEVICECFG_V40,0,pStruDevCfg,sizeof(NET_DVR_DEVICECFG_V40),&dwReturned))
		{
			g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GET_DEVICECFG_V40");
		}
		else
		{
			if (g_struDeviceInfo[iDeviceIndex].iDeviceType != (int)pStruDevCfg->wDevType)
			{
				char szLan[256] = {0};
				char szLanShow[256] = {0};
				g_StringLanType(szLan, "��½�����豸����ֵ���ȡ�豸���������豸����ֵ��ͬ", "returned device type is different between login and get device config");
                sprintf(szLanShow,"%s %d,%d",szLan,g_struDeviceInfo[iDeviceIndex].iDeviceType,pStruDevCfg->wDevType);
				AfxMessageBox(szLanShow);
			}
			
			sprintf(g_struDeviceInfo[iDeviceIndex].chDeviceName,"%s",pStruDevCfg->byDevTypeName);	
			g_struDeviceInfo[iDeviceIndex].dwDevSoftVer = pStruDevCfg->dwSoftwareVersion;
		}
		
		g_pMainDlg->m_treeDeviceList.Expand(hDeviceItem,TVE_EXPAND);
		//get the new device resource configure when login device/
		if (g_pMainDlg->DoGetDeviceResoureCfg(iDeviceIndex))
		{
			g_pMainDlg->DoGetIPAlarmInCfg(iDeviceIndex);
			g_pMainDlg->DoGetIPAlarmOutCfg(iDeviceIndex);			
		}
		g_pMainDlg->RedrawDeviceNode(iDeviceIndex);
		
		::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_DEVICE_ITEM_IMAGE, WPARAM(hDeviceItem), LPARAM(iDeviceIndex));	
	}

	//return if already logged in

	if (pStruNetCfg != NULL)
	{
		delete pStruNetCfg;
		pStruNetCfg = NULL;
	}
	
	if (pStruDevCfg  != NULL)
	{
		delete pStruDevCfg ;
		pStruDevCfg = NULL;
	}
	
	return NULL;
}

/*********************************************************
Function:	DoLogin
Desc:		check the device has logged in, if not, log in 
Input:	iDeviceIndex,device serial number ;hDeviceItem pointer to device node
Output:	
Return:	FALSE,device log in failed;TRUE; logged in;
**********************************************************/
BOOL CClientDemoDlg::DoLogin(int iDeviceIndex, HTREEITEM hDeviceItem)
{
	LPLOCAL_LOGIN_PARAM lpParam = new LOCAL_LOGIN_PARAM;
	if (lpParam == NULL)
    {
        return FALSE;
    }

	lpParam->iDeviceIndex = iDeviceIndex;
	lpParam->hSelect = hDeviceItem;
	lpParam->bMsgBox = TRUE;
	DoLoginEx(lpParam);	
	//return if already logged in
	return TRUE;
}

/*********************************************************
Function:	StopPlayedChan
Desc:		stop one play channel in preview
Input:	iDeviceIndex, device index, iChanIndex,channel index
Output:	
Return:	
**********************************************************/
BOOL CClientDemoDlg::StopPlayedChan(int iDeviceIndex, int iChanIndex)
{
	for (int i = 0; i < MAX_OUTPUTS; i ++)
	{
		if (g_dlgOutput[i].m_lPlayHandle < 0)
		{
			continue;
		}
		
		if (g_dlgOutput[i].m_struChanInfo.iDeviceIndex == iDeviceIndex && \
			g_dlgOutput[i].m_struChanInfo.iChanIndex == iChanIndex )
		{
            if (g_dlgOutput[i].m_struChanInfo.iChanIndex < ZERO_CHAN_INDEX)
            {
				return g_dlgOutput[i].StopPlay();
			}
            else
            {
                return g_dlgOutput[i].ZeroStopPlay();
            }            
        }
	}
	
	return TRUE;
}

/*********************************************************
Function:	CreateNewLogFile
Desc:		create new log file
Input:	bCreate, TRUE/FALSE
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::CreateNewLogFile(BOOL bCreate)
{
	CString csLogFile;
	
	// 	if ((LONG *)m_fLog.m_hFile != INVALID_HANDLE_VALUE)
	// 	{
	// 		TRACE("Close alarm file");	
	// 		m_fLog.Close();
	// 		m_fLog.Abort();
	// 	}
	// 
	// 	if ((LONG *)m_fAlarm.m_hFile != INVALID_HANDLE_VALUE)
	// 	{
	// 		TRACE("Close alarm file");
	// 		
	// 		m_fAlarm.Close();
	// 		m_fAlarm.Abort();
	// 	}
	if (m_fLog != NULL)
	{
		fclose(m_fLog);
	}
	if (m_fAlarm!= NULL)
	{
		fclose(m_fAlarm);
	}
	if (!bCreate)
	{
		return;
	}
	COleDateTime dtCurTime = COleDateTime::GetCurrentTime();
	csLogFile.Format("%s\\LogFile_%4d%02d%02d.txt",\
		g_struLocalParam.chRemoteCfgSavePath, dtCurTime.GetYear(), dtCurTime.GetMonth(),\
		dtCurTime.GetDay());
	if ((m_fLog = fopen(csLogFile.GetBuffer(0), "a+")) == NULL)
	{
		GetDlgItem(IDC_BTN_LOG_SAVE)->EnableWindow(FALSE);
		g_struLocalParam.bAutoSaveLog = FALSE;
	}		
	csLogFile.Format("%s\\AlarmInfo_%4d%02d%02d.txt",\
		g_struLocalParam.chRemoteCfgSavePath, dtCurTime.GetYear(), dtCurTime.GetMonth(),\
		dtCurTime.GetDay());
	if ((m_fAlarm = fopen(csLogFile.GetBuffer(0), "a+")) == NULL)
	{
		GetDlgItem(IDC_BTN_LOG_SAVE)->EnableWindow(FALSE);
		g_struLocalParam.bAutoSaveLog = FALSE;
	}
}

/*********************************************************
Function:	OnTimer
Desc:		timer response function
Input:	nIDEvent, ID event value
Output:	
Return:	
**********************************************************/
#if (_MSC_VER >= 1500)	//vs2008
void CClientDemoDlg::OnTimer(UINT_PTR nIDEvent)
#else
void CClientDemoDlg::OnTimer(UINT nIDEvent)
#endif
{
	// TODO: Add your message handler code here and/or call default
	char szLan[128] = {0};
	
	CString csTemp = _T("");
	int i = 0;
	int j = 0;
	time_t	lCurrentTime;
	CString	csRecRootPath;
	
	CTime ctCurTime = CTime::GetCurrentTime();
	int iWeekday = ctCurTime.GetDayOfWeek() - 1;
	WORD	iNowTime = (WORD)(ctCurTime.GetHour() * 100 + ctCurTime.GetMinute());
	NET_DVR_TIME CurTime;
	CurTime.dwYear = ctCurTime.GetYear();
	CurTime.dwMonth = ctCurTime.GetMonth();
	CurTime.dwDay = ctCurTime.GetDay();
	CurTime.dwHour = ctCurTime.GetHour();
	CurTime.dwMinute = ctCurTime.GetMinute();
	CurTime.dwSecond = ctCurTime.GetSecond();
	switch (nIDEvent)
	{
	case SYSTEM_TIMER:
		if (g_struLocalParam.bAutoSaveLog && ctCurTime.GetHour() == 0\
			&& ctCurTime.GetMinute() == 0 && ctCurTime.GetSecond() == 0)
		{//one day
			CreateNewLogFile(TRUE);
		}
		
		break;
	case CYCLE_PREVIEW_TIMER:
		CyclePlayAllDevicesLogined();
		break;
	case FREESPACE_TIMER:
		csRecRootPath.Format("%c:\\", m_iCurRecDriver);	
		ULARGE_INTEGER FreeSpace,CallerSpace,TotalSpace;
		GetDiskFreeSpaceEx(csRecRootPath,&CallerSpace,&TotalSpace,&FreeSpace);
		g_struLocalParam.bAllDiskFull = FALSE;
		if (CallerSpace.QuadPart < MINSPACE)
		{
			csTemp.Format("%4d-%02d-%02d %02d:%02d:%02d HD Partition%c is Full", CurTime.dwYear, CurTime.dwMonth,\
				CurTime.dwDay, CurTime.dwHour, CurTime.dwMinute, CurTime.dwSecond, m_iCurRecDriver);
			AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, csTemp.GetBuffer(0));
			if (IsDiskFull())		//delete first 8 record files if hard disk is full
			{
				if (g_struLocalParam.bCycleRecord)
				{
					DeleteClientRecordFile();
				}
				else
				{
					for (i = 0; i < MAX_OUTPUTS; i++)
					{
						if ((g_dlgOutput[i].m_lPlayHandle >= 0) && (g_dlgOutput[i].m_bLocalManualRec))
						{
							AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "CClientDemoDlg::FREESPACE_TIMER stop");
							g_dlgOutput[i].StopLocalRecord();
						}
					}
					g_StringLanType(szLan, "�ͻ���¼��Ӳ���������������޷�����¼��", "Client hard disk drive is full, unable to continue record");
					g_pMainDlg->AddLog(-1, OPERATION_FAIL_T, szLan);
					return;
				}
			}
			else
			{	//storage to other partition which is free
				if (MakeRecDir())
				{
					AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "switch to Hard disk partition%c", m_iCurRecDriver);
					for (i = 0; i < MAX_OUTPUTS; i++)
					{
						if ((g_dlgOutput[i].m_lPlayHandle >= 0) && (g_dlgOutput[i].m_bLocalManualRec))
						{
							g_dlgOutput[i].ChangeRecordFileName();
						}
					}
				}
			}
		}
		break;
	case FILEINTERVAL_TIMER:
		for (i = 0; i < MAX_OUTPUTS; i++)
		{
			if ((g_dlgOutput[i].m_lPlayHandle >= 0) && (g_dlgOutput[i].m_bLocalManualRec))
			{
				lCurrentTime = (time_t)ctCurTime.GetTime();
				if (lCurrentTime - g_dlgOutput[i].m_lChangeFileNameTime >= 0)
				{
					if (MakeRecDir())
					{
						g_dlgOutput[i].ChangeRecordFileName();
					}
				}
			}
		}
		break;
	case AUTORECORD_TIMER:
		if (g_struLocalParam.bCycling)
		{
			return;	//no record in client while cycle play
		}
		if (!g_struLocalParam.bAutoRecord)
		{
			return;
		}
		for (i = 0; i < 4; i++)
		{
			if ((iNowTime >= g_struLocalParam.struLocalRecordTime[iWeekday][i].iStartTime) && 
				(iNowTime < g_struLocalParam.struLocalRecordTime[iWeekday][i].iStopTime))
			{	//it is time auto record
				for (j = 0; j < MAX_OUTPUTS; j++)
				{
					if (g_dlgOutput[j].m_lPlayHandle >= 0)
					{
						if ( !g_dlgOutput[j].m_bLocalManualRec)
						{
							if (g_struLocalParam.bAllDiskFull)
							{
								AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "Client Hard Disk is full, pause recording");
								return;		//no record if client hard disk is full
							}
							if (MakeRecDir())
							{
								g_dlgOutput[j].AddLog(OPERATION_SUCC_T, "%02d:%02d start auto record", CurTime.dwHour, CurTime.dwMinute);
								g_dlgOutput[j].StartLocalRecord();
							}
						}
					}
				}
				break;
			}
			else
				i++;
		}
		if (i == 4)
		{
			for (j = 0; j < MAX_OUTPUTS; j++)
			{
				if (g_dlgOutput[j].m_bLocalManualRec)
				{
					g_dlgOutput[j].AddLog(OPERATION_SUCC_T, "%02d:%02d stop auto record", CurTime.dwHour, CurTime.dwMinute);
					g_dlgOutput[j].StopLocalRecord();
				}
			}
		}
		break;
	case CHECK_DEVICE_TIMER:
		if (g_struLocalParam.lCheckDeviceTime == (LONG)(CurTime.dwHour*10000 + CurTime.dwMinute*100 + CurTime.dwSecond))
		{
			for (i = 0; i < MAX_DEVICES; i++)
			{
				if (g_struDeviceInfo[i].lLoginID >= 0)
				{
					if (NET_DVR_SetDVRConfig(g_struDeviceInfo[i].lLoginID, NET_DVR_SET_TIMECFG, 0, &CurTime, sizeof(NET_DVR_TIME)))
					{
						AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "NET_DVR_SetDVRConfig");
					}
					else
					{
						AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "NET_DVR_SetDVRConfig");
					}
				}
			}
		}
		break;
	default:
		break;
	}
	
	CDialog::OnTimer(nIDEvent);
}

/*********************************************************
Function:	OnCbnSelchangeComboWndNum
Desc:		change pic number in preview
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnCbnSelchangeComboWndNum()
{
	int iIndex = m_comboWinNum.GetCurSel();
	m_iCurWndNum = (int)pow(double(iIndex+1), 2);//current window number
	if (!IsPlaying())
	{
		m_iCurWndIndex = 0;//initialize current window index while switch window
		ArrangeOutputs(m_iCurWndNum);
		GetDlgItem(IDC_STATIC_PREVIEWBG)->Invalidate(TRUE);
	}
	else
	{
		char szLan[128] = {0};
		g_StringLanType(szLan, "����ֹͣ����", "Please stop previewing");
		AfxMessageBox(szLan);
	}
}

/*********************************************************
Function:	OnWMProcAlarm
Desc:		process alarm information when 
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::ProcAlarm(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	//	int iRet = 0;
	BOOL bDisk = FALSE;
	int i = 0;
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
	char szLan[128] ={0};
	NET_DVR_ALARMINFO m_struAlarmInfo;
	memcpy(&m_struAlarmInfo, pAlarmInfo, sizeof(NET_DVR_ALARMINFO));
	if (!(g_struDeviceInfo[iDeviceIndex].bAlarm))
	{
		g_struDeviceInfo[iDeviceIndex].bAlarm = TRUE;
		::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_DEVICE_ITEM_IMAGE, WPARAM(g_pMainDlg->GetDeviceItem(iDeviceIndex)), LPARAM(iDeviceIndex));		
	}

	int iAChanNum = g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].dwAChanNum;

	switch (m_struAlarmInfo.dwAlarmType)
	{
	case 0://signal alarm
		g_StringLanType(szLan, "���������� ��������", "Switch alarm input");
		sprintf(szInfoBuf,"%s%d", szLan, m_struAlarmInfo.dwAlarmInputNumber+1); 
		g_StringLanType(szLan, ",����ͨ��¼��:", ", Invoke record Channel:");
		sprintf(szInfoBuf,"%s%s", szInfoBuf, szLan); 
		for (i=0; i<MAX_CHANNUM; i++)
		{
			if (m_struAlarmInfo.dwAlarmRelateChannel[i] == 1)
			{
				sprintf(szInfoBuf,"%s %d,", szInfoBuf, i+g_struDeviceInfo[iDeviceIndex].iStartChan); 
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T,szInfoBuf);
		break;
	case 1://hard disk full alarm
		g_StringLanType(szInfoBuf, "Ӳ��������:", "HDD is Full:");
		for (i=0; i<MAX_DISKNUM; i++)
		{
			if (m_struAlarmInfo.dwDiskNumber[i] == 1)
			{
				sprintf(szInfoBuf, "%s HDD%d ", szInfoBuf, i);
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T,szInfoBuf);
		break;
	case 2://video loss alarm
		g_StringLanType(szInfoBuf, "��Ƶ�źŶ�ʧ����:", "The video signal lost:");
		for (i=0; i<MAX_CHANNUM; i++)
		{
			if (m_struAlarmInfo.dwChannel[i] == 1)
			{
				if (!(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm))
				{
					g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
					::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));	
				}
				sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);				
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	case 3://motion detection alarm
		g_StringLanType(szInfoBuf, "�ƶ���ⱨ��", "motion detection");
		for (i=0; i<MAX_CHANNUM_V40; i++)
		{
			if (m_struAlarmInfo.dwChannel[i] == 1)
			{
				
				if (g_struDeviceInfo[iDeviceIndex].iStartChan ==0)
				{
					//ģ��ͨ��
					if (i < MAX_ANALOG_CHANNUM)
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));		
						}
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);
					}
					else   // ����ͨ����i
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i-32+iAChanNum].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i-32+iAChanNum].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i-32+iAChanNum));		
						}
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i-32+iAChanNum].chChanName);
					}
				}
				else
				{
					if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm)
					{
						g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
						::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i-g_struDeviceInfo[iDeviceIndex].iStartChan+iAChanNum));		
					}
					sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i-g_struDeviceInfo[iDeviceIndex].iStartChan+iAChanNum].chChanName);
				}
				
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	case 4://hard disk unformatted alarm
		g_StringLanType(szInfoBuf, "Ӳ��δ��ʽ������", "HDD not formatted:");
		for (i=0; i<MAX_DISKNUM; i++)
		{
			if (m_struAlarmInfo.dwDiskNumber[i] == 1)
			{
				sprintf(szInfoBuf, "%s HDD%d", szInfoBuf, i);
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	case 5://hard disk alarm
		for (i=0; i<MAX_DISKNUM; i++)
		{
			if (m_struAlarmInfo.dwDiskNumber[i] == 1)
			{
				bDisk = TRUE;
				g_StringLanType(szLan, "��дӲ�̳���", "failed to read and write HDD");
				sprintf(szInfoBuf, "%sHDD%d ", szLan, i);
				g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
			}
		}
		if (bDisk == FALSE)
		{
			g_StringLanType(szInfoBuf, "δ��Ӳ�̳�����", "no HDD");
			g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		}
		bDisk = FALSE;
		break;
	case 6://mask alarm
		g_StringLanType(szInfoBuf, "�����ڵ�����", "Mask alarm");
		for (i=0; i<MAX_CHANNUM; i++)
		{
			if (m_struAlarmInfo.dwChannel[i] == 1)
			{
				if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm)
				{
					g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
					::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));		
				}
				sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);
			}
		}	
		
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	case 7:	//type unmatched
		g_StringLanType(szInfoBuf, "��ʽ��ƥ��", "input and output video standard not match");
		for (i=0; i<MAX_CHANNUM; i++)
		{
			if (m_struAlarmInfo.dwChannel[i] == 1)
			{
				if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm)
				{
					g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
					::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));		
				}
				sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	case 8:
		g_StringLanType(szInfoBuf, "�Ƿ�����", "illegal access");
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);		
break;
	case 9:
        g_StringLanType(szInfoBuf, "��Ƶ�ź��쳣", "video exception");
        g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);		
		break;
    case 10:
        g_StringLanType(szInfoBuf, "¼���쳣", "record exception");
        g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);		
		break;
    case 11:
        g_StringLanType(szInfoBuf, "���ܳ����仯", "Vca scene change");
        g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T,szInfoBuf);
        break;
	default:
		g_StringLanType(szLan, "δ֪��������:", "Unknown Alarm Type:");
		sprintf(szInfoBuf, "%s 0x%x", szLan, m_struAlarmInfo.dwAlarmType);
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	}
}

/*********************************************************
Function:	ProcAlarmV30
Desc:		process alarm information for version 30
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::ProcAlarmV30(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	BOOL bDisk = FALSE;
	int i = 0;
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
	char szLan[128] ={0};
	NET_DVR_ALARMINFO_V30 m_struAlarmInfoV30;
	memcpy(&m_struAlarmInfoV30, pAlarmInfo, sizeof(NET_DVR_ALARMINFO_V30));
	if (!(g_struDeviceInfo[iDeviceIndex].bAlarm))
	{
		g_struDeviceInfo[iDeviceIndex].bAlarm = TRUE;
		::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_DEVICE_ITEM_IMAGE, WPARAM(g_pMainDlg->GetDeviceItem(iDeviceIndex)), LPARAM(iDeviceIndex));		
	}

	//ģ��ͨ����
	int iAChanNum = g_struDeviceInfo[iDeviceIndex].iAnalogChanNum;
	
	//����ͨ����
	int iDChanNum = g_struDeviceInfo[iDeviceIndex].iIPChanNum;

	switch (m_struAlarmInfoV30.dwAlarmType)
	{
	case 0://signal alarm
		g_StringLanType(szLan, "���������� ��������", "Switch alarm input");
		sprintf(szInfoBuf,"%s%d", szLan, m_struAlarmInfoV30.dwAlarmInputNumber+1); 
		g_StringLanType(szLan, ",����ͨ��¼��:", "Invoke record Channel:");
		sprintf(szInfoBuf,"%s%s", szInfoBuf, szLan); 
		for (i=0; i<MAX_CHANNUM_V30; i++)
		{
			if (m_struAlarmInfoV30.byAlarmRelateChannel[i] == 1)
			{
				// 					if (i<MAX_ANALOG_CHANNUM)
				// 					{
				sprintf(szInfoBuf,"%s %d,", szInfoBuf, i+g_struDeviceInfo[iDeviceIndex].iStartChan); 	
				// 					}
				// 					else
				// 					{
				// 						sprintf(szInfoBuf,"%s %s%d,", szInfoBuf,"IPChan", i+g_struDeviceInfo[iDeviceIndex].iStartChan); 
				// 					}
				
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T,szInfoBuf);
		break;
	case 1://hard disk full alarm
		g_StringLanType(szInfoBuf, "Ӳ��������", "HD is full:");
		for (i=0; i<MAX_DISKNUM_V30; i++)
		{
			if (m_struAlarmInfoV30.byDiskNumber[i] == 1)
			{
				sprintf(szInfoBuf, "%s HDD%d ", szInfoBuf, i+1);
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T,szInfoBuf);
		break;
	case 2://video loss alarm
		g_StringLanType(szInfoBuf, "��Ƶ�źŶ�ʧ����", "The signal lost");
		for (i=0; i<MAX_CHANNUM_V30; i++)
		{
			if (m_struAlarmInfoV30.byChannel[i] == 1)
			{
				//ģ��ͨ��
				if (i < MAX_ANALOG_CHANNUM)
				{
					if (!(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm))
					{
						g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
						::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));			
					}
					sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);	
				}
				else
				{
					if (!(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i-32+iAChanNum].bAlarm))
					{
						g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i-32+iAChanNum].bAlarm = TRUE;
						::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i-32+iAChanNum));			
					}
					sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i-32+iAChanNum].chChanName);	
				}

	
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T,szInfoBuf);
 		break;
	case 3://motion detection alarm
		g_StringLanType(szInfoBuf, "�ƶ���ⱨ��", "motion detection");
		for (i=0; i<MAX_CHANNUM_V30; i++)
		{
			if (m_struAlarmInfoV30.byChannel[i] == 1)
			{
				//��ģ��ͨ��������ͨ��
				if (0 == iDChanNum || 0 == iAChanNum)
				{
					if (!(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm))
					{
						g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
						::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));			
					}

					sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);	
				}
				else if(iAChanNum > 0 && iDChanNum > 0)  //���ͨ��
				{
					//ģ��ͨ��
					if (i < MAX_ANALOG_CHANNUM)
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));		
						}

						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);		
					}
					else //����ͨ��
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(iAChanNum+i-MAX_ANALOG_CHANNUM));		
						}

						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].chChanName);
					}
				}
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	case 4://hard disk unformatted alarm
		g_StringLanType(szInfoBuf, "Ӳ��δ��ʽ������", "not formatted");
		for (i=0; i<MAX_DISKNUM_V30; i++)
		{
			if (m_struAlarmInfoV30.byDiskNumber[i] == 1)
			{
				sprintf(szInfoBuf, "%s HDD%d ", szInfoBuf, i);
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	case 5://hard disk alarm
		for (i=0; i<MAX_DISKNUM_V30; i++)
		{
			if (m_struAlarmInfoV30.byDiskNumber[i] == 1)
			{
				bDisk = TRUE;
				g_StringLanType(szLan, "��дӲ�̳���", "failed to read and write HDD");
				sprintf(szInfoBuf, "%sHDD%d ", szLan, i+1);
				g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
			}
		}
		if (bDisk == FALSE)
		{
			g_StringLanType(szInfoBuf, "δ��Ӳ�̳�����", "No HDD");
			g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		}
		bDisk = FALSE;
		break;
	case 6://mask alarm
		g_StringLanType(szInfoBuf, "�����ڵ�����", "Mask alarm");
		for (i=0; i<MAX_CHANNUM_V30; i++)
		{
			if (m_struAlarmInfoV30.byChannel[i] == 1)
			{
				//��ģ��ͨ��������ͨ��
				if (0 == iDChanNum || 0 == iAChanNum)
				{
					if (!(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm))
					{
						g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
						::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));			
					}
					
					sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);	
				}
				else if(iAChanNum > 0 && iDChanNum > 0)  //���ͨ��
				{
					//ģ��ͨ��
					if (i < MAX_ANALOG_CHANNUM)
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);		
					}
					else //����ͨ��
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(iAChanNum+i-MAX_ANALOG_CHANNUM));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].chChanName);
					}
				}
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	case 7:	//type unmatched
		g_StringLanType(szInfoBuf, "��ʽ��ƥ��", "input and output video standard not match");
		for (i=0; i<MAX_CHANNUM_V30; i++)
		{
			if (m_struAlarmInfoV30.byChannel[i] == 1)
			{
				//��ģ��ͨ��������ͨ��
				if (0 == iDChanNum || 0 == iAChanNum)
				{
					if (!(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm))
					{
						g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
						::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));			
					}
					
					sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);	
				}
				else if(iAChanNum > 0 && iDChanNum > 0)  //���ͨ��
				{
					//ģ��ͨ��
					if (i < MAX_ANALOG_CHANNUM)
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);		
					}
					else //����ͨ��
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(iAChanNum+i-MAX_ANALOG_CHANNUM));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].chChanName);
					}
				}
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	case 8:
		g_StringLanType(szInfoBuf, "�Ƿ�����", "illegal access");
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);		
		break;
    case 9:
        g_StringLanType(szInfoBuf, "��Ƶ�ź��쳣", "video exception");
		for (i=0; i<MAX_CHANNUM_V30; i++)
		{
			if (m_struAlarmInfoV30.byChannel[i] == 1)
			{
				//��ģ��ͨ��������ͨ��
				if (0 == iDChanNum || 0 == iAChanNum)
				{
					if (!(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm))
					{
						g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
						::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));			
					}
					
					sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);	
				}
				else if(iAChanNum > 0 && iDChanNum > 0)  //���ͨ��
				{
					//ģ��ͨ��
					if (i < MAX_ANALOG_CHANNUM)
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);		
					}
					else //����ͨ��
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(iAChanNum+i-MAX_ANALOG_CHANNUM));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].chChanName);
					}
				}
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
    case 10:
        g_StringLanType(szInfoBuf, "¼���쳣", "record exception");
		for (i=0; i<MAX_CHANNUM_V30; i++)
		{
			if (m_struAlarmInfoV30.byChannel[i] == 1)
			{
				//��ģ��ͨ��������ͨ��
				if (0 == iDChanNum || 0 == iAChanNum)
				{
					if (!(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm))
					{
						g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
						::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));			
					}
					
					sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);	
				}
				else if(iAChanNum > 0 && iDChanNum > 0)  //���ͨ��
				{
					//ģ��ͨ��
					if (i < MAX_ANALOG_CHANNUM)
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);		
					}
					else //����ͨ��
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(iAChanNum+i-MAX_ANALOG_CHANNUM));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].chChanName);
					}
				}
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
    case 11:
        g_StringLanType(szInfoBuf, "���ܳ����仯", "Vca scene change");
		for (i=0; i<MAX_CHANNUM_V30; i++)
		{
			if (m_struAlarmInfoV30.byChannel[i] == 1)
			{
				//��ģ��ͨ��������ͨ��
				if (0 == iDChanNum || 0 == iAChanNum)
				{
					if (!(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm))
					{
						g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
						::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));			
					}
					
					sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);	
				}
				else if(iAChanNum > 0 && iDChanNum > 0)  //���ͨ��
				{
					//ģ��ͨ��
					if (i < MAX_ANALOG_CHANNUM)
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);		
					}
					else //����ͨ��
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(iAChanNum+i-MAX_ANALOG_CHANNUM));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].chChanName);
					}
				}
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
    case 12:
        g_StringLanType(szInfoBuf, "�����쳣", "Array exception");
        g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T,szInfoBuf);
        break;
	case 13:
		g_StringLanType(szInfoBuf, "ǰ��/¼��ֱ��ʲ�ƥ��", "resolution mismatch");
		for (i=0; i<MAX_CHANNUM_V30; i++)
		{
			if (m_struAlarmInfoV30.byChannel[i] == 1)
			{
				//��ģ��ͨ��������ͨ��
				if (0 == iDChanNum || 0 == iAChanNum)
				{
					if (!(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm))
					{
						g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
						::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));			
					}
					
					sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);	
				}
				else if(iAChanNum > 0 && iDChanNum > 0)  //���ͨ��
				{
					//ģ��ͨ��
					if (i < MAX_ANALOG_CHANNUM)
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);		
					}
					else //����ͨ��
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(iAChanNum+i-MAX_ANALOG_CHANNUM));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].chChanName);
					}
				}
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	case 14:
		g_StringLanType(szInfoBuf, "���������Դʧ��", "Alloc decoede resource fail");
		for (i=0; i<MAX_CHANNUM_V30; i++)
		{
			if (m_struAlarmInfoV30.byChannel[i] == 1)
			{
				//��ģ��ͨ��������ͨ��
				if (0 == iDChanNum || 0 == iAChanNum)
				{
					if (!(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm))
					{
						g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
						::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));			
					}
					
					sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);	
				}
				else if(iAChanNum > 0 && iDChanNum > 0)  //���ͨ��
				{
					//ģ��ͨ��
					if (i < MAX_ANALOG_CHANNUM)
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);		
					}
					else //����ͨ��
					{
						if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm)
						{
							g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].bAlarm = TRUE;
							::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(iAChanNum+i-MAX_ANALOG_CHANNUM));		
						}
						
						sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+i-MAX_ANALOG_CHANNUM].chChanName);
					}
				}
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
    case 15:
	    g_StringLanType(szInfoBuf, "�������", "VCA Detction");
	    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
    case 16:
        g_StringLanType(szInfoBuf, "POE�����쳣", "POE Power Exception");
        g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	default:
		g_StringLanType(szLan, "δ֪��������:", "Unknown Alarm Type:");
		sprintf(szInfoBuf, "%s 0x%x-htonl%d", szLan, m_struAlarmInfoV30.dwAlarmType, ntohl(m_struAlarmInfoV30.dwAlarmType));
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	}
}


/*********************************************************
Function:	ProcAlarmV40
Desc:		process alarm information for version 40
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::ProcAlarmV40(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	BOOL bDisk = FALSE;
	int i = 0;
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
	char szLan[128] ={0};
	NET_DVR_ALARMINFO_V40 m_struAlarmInfoV40;
	memcpy(&m_struAlarmInfoV40, pAlarmInfo, sizeof(NET_DVR_ALARMINFO_V40));
	if (!(g_struDeviceInfo[iDeviceIndex].bAlarm))
	{
		g_struDeviceInfo[iDeviceIndex].bAlarm = TRUE;
		::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_DEVICE_ITEM_IMAGE, WPARAM(g_pMainDlg->GetDeviceItem(iDeviceIndex)), LPARAM(iDeviceIndex));		
	}

	//ģ��ͨ����
	int iAChanNum = g_struDeviceInfo[iDeviceIndex].iAnalogChanNum;
	
	//����ͨ����
	int iDChanNum = g_struDeviceInfo[iDeviceIndex].iIPChanNum;

    int dwOffSetLen = 0;

    DWORD dwTemp = 0;
    BOOL bAlarmOut = FALSE;
	switch (m_struAlarmInfoV40.struAlarmFixedHeader.dwAlarmType)
	{
	case 0://signal alarm
		g_StringLanType(szLan, "���������� ���������", "Switch alarm input No.");
		sprintf(szInfoBuf,"%s%d", szLan, m_struAlarmInfoV40.struAlarmFixedHeader.uStruAlarm.struIOAlarm.dwAlarmInputNo +1); 
		g_StringLanType(szLan, ",����ͨ��¼��:", "Invoke record Channel:");
		sprintf(szInfoBuf,"%s%s", szInfoBuf, szLan); 
        dwOffSetLen += m_struAlarmInfoV40.struAlarmFixedHeader.uStruAlarm.struIOAlarm.dwTrigerAlarmOutNum;
		for (i=0; i<m_struAlarmInfoV40.struAlarmFixedHeader.uStruAlarm.struIOAlarm.dwTrigerRecordChanNum; i++)
		{
			if (*(m_struAlarmInfoV40.pAlarmData + dwOffSetLen) != 0xffffffff)
			{
                sprintf(szInfoBuf,"%s %d,", szInfoBuf, *(m_struAlarmInfoV40.pAlarmData + dwOffSetLen)); 
                dwOffSetLen++;
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T,szInfoBuf);
		break;
	case 1://hard disk full alarm
		g_StringLanType(szInfoBuf, "Ӳ��������", "HD is full:");
		for (i=0; i<m_struAlarmInfoV40.struAlarmFixedHeader.uStruAlarm.struAlarmHardDisk.dwAlarmHardDiskNum; i++)
		{
			if (*(m_struAlarmInfoV40.pAlarmData + dwOffSetLen) != 0xffffffff )
			{
				sprintf(szInfoBuf, "%s HDD%d ", szInfoBuf, *(m_struAlarmInfoV40.pAlarmData + dwOffSetLen));
                dwOffSetLen ++;
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T,szInfoBuf);
		break;	
	case 4://hard disk unformatted alarm
		g_StringLanType(szInfoBuf, "Ӳ��δ��ʽ������", "not formatted");
		for (i=0; i<m_struAlarmInfoV40.struAlarmFixedHeader.uStruAlarm.struAlarmHardDisk.dwAlarmHardDiskNum; i++)
		{
		
            dwTemp = *(m_struAlarmInfoV40.pAlarmData + dwOffSetLen);
			if (dwTemp > 0)
			{
				sprintf(szInfoBuf, "%s HDD%d ", szInfoBuf,dwTemp);
                dwOffSetLen++;
			}
            else
            {
                break;
            }
		}
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	case 5://hard disk alarm
		for (i=0; i<m_struAlarmInfoV40.struAlarmFixedHeader.uStruAlarm.struAlarmHardDisk.dwAlarmHardDiskNum; i++)
		{
            dwTemp = *(m_struAlarmInfoV40.pAlarmData + dwOffSetLen);
            if (dwTemp > 0)
            {
                bDisk = TRUE;
                g_StringLanType(szLan, "��дӲ�̳���", "failed to read and write HDD");
                sprintf(szInfoBuf, "%sHDD%d ", szLan, dwTemp);
				g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
                dwOffSetLen++;
            }
            else
            {
                break;
            }
		}

		if (bDisk == FALSE)
		{
			g_StringLanType(szInfoBuf, "δ��Ӳ�̵��³�����", "No HDD");
			g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		}
		bDisk = FALSE;
		break;
		
    case 2://video loss alarm
        bAlarmOut = TRUE;
        g_StringLanType(szInfoBuf, "��Ƶ�źŶ�ʧ����", "The signal lost");
    case 3://motion detection alarm
        bAlarmOut = TRUE;
        g_StringLanType(szInfoBuf, "�ƶ���ⱨ��", "motion detection");
    case 6://mask alarm
        bAlarmOut = TRUE;
        g_StringLanType(szInfoBuf, "�����ڵ�����", "Mask alarm");
    case 9:
        bAlarmOut = TRUE;
        g_StringLanType(szInfoBuf, "��Ƶ�ź��쳣", "video exception");
    case 10:
        bAlarmOut = TRUE;
        g_StringLanType(szInfoBuf, "¼���쳣", "record exception");
    case 11:
        bAlarmOut = TRUE;
        g_StringLanType(szInfoBuf, "���ܳ����仯", "Vca scene change");
    case 13:
        bAlarmOut = TRUE;
        g_StringLanType(szInfoBuf, "ǰ��/¼��ֱ��ʲ�ƥ��", "resolution mismatch");
		break;

    case 7:	//type unmatched
        g_StringLanType(szInfoBuf, "��ʽ��ƥ��", "input and output video standard not match");
        g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
        break;
    case 8:
        g_StringLanType(szInfoBuf, "�Ƿ�����", "illegal access");
        g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);		
		break;
    case 12:
        g_StringLanType(szInfoBuf, "�����쳣", "Array exception");
        g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T,szInfoBuf);
        break;
	case 14:
        g_StringLanType(szInfoBuf, "���������Դʧ��", "Alloc decoede resource fail");
        g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	default:
		g_StringLanType(szLan, "δ֪��������:", "Unknown Alarm Type:");
		sprintf(szInfoBuf, "%s 0x%x-htonl%d", szLan, m_struAlarmInfoV40.struAlarmFixedHeader.dwAlarmType, ntohl(m_struAlarmInfoV40.struAlarmFixedHeader.dwAlarmType));
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
		break;
	}
   
    if (bAlarmOut)
    {
        for (i=0; i<m_struAlarmInfoV40.struAlarmFixedHeader.uStruAlarm.struAlarmChannel.dwAlarmChanNum; i++)
        {
            dwTemp = *(m_struAlarmInfoV40.pAlarmData + dwOffSetLen);
            if (dwTemp >= 0 && dwTemp < MAX_CHANNUM_V40)
            {
                //��ģ��ͨ��������ͨ��
                if (0 == iDChanNum || 0 == iAChanNum)
                {
                    if (!(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwTemp- 1].bAlarm))
                    {
                        g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwTemp- 1].bAlarm = TRUE;
                        ::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(dwTemp- 1));			
                    }
                    
                    sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwTemp- 1].chChanName);	
                }
                else if(iAChanNum > 0 && iDChanNum > 0)  //���ͨ��
                {
                    //ģ��ͨ��
                    if (i < MAX_ANALOG_CHANNUM)
                    {
                        if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwTemp- 1].bAlarm)
                        {
                            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwTemp- 1].bAlarm = TRUE;
                            ::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(dwTemp- 1));		
                        }
                        
                        sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwTemp- 1].chChanName);		
                    }
                    else //����ͨ��
                    {
                        if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+dwTemp- 1-MAX_ANALOG_CHANNUM].bAlarm)
                        {
                            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+dwTemp- 1-MAX_ANALOG_CHANNUM].bAlarm = TRUE;
                            ::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(iAChanNum+dwTemp- 1-MAX_ANALOG_CHANNUM));		
                        }
                        
                        sprintf(szInfoBuf, "%s %s", szInfoBuf, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iAChanNum+dwTemp- 1-MAX_ANALOG_CHANNUM].chChanName);
                    }
                }
                dwOffSetLen++;
            }
            else
            {
                break;
            }
        }
        g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
    }
}


/*********************************************************
Function:	
Desc:		
Input:	void
Output:	void
Return:	0, same, else-difference
**********************************************************/
int CmpIPCfgAndIPCfgV31(LPNET_DVR_IPDEVINFO_V31 lpDevInfoV31, LPNET_DVR_IPDEVINFO lpDevInfo)
{
	if (lpDevInfo->dwEnable != lpDevInfoV31->byEnable)
	{
		return -1;
	}
	if (lpDevInfo->wDVRPort != lpDevInfoV31->wDVRPort)
	{
		return -1;
	}
	if (memcmp((char *)(lpDevInfoV31->sPassword), (char *)(lpDevInfoV31->sPassword), PASSWD_LEN) != 0)
	{
		return -1;
	}
	if (memcmp((char *)(lpDevInfo->sUserName), (char *)(lpDevInfoV31->sUserName), MAX_NAMELEN) != 0)
	{
		return -1;
	}
	if (memcmp((char *)&(lpDevInfo->struIP), (char *)&(lpDevInfoV31->struIP), sizeof(NET_DVR_IPDEVINFO)) != 0)
	{
		return -1;
	}
	memset((char *)(lpDevInfoV31->byDomain), 0, MAX_DOMAIN_NAME);//the old ipc parameter change need to clear domain information
	return 0;
}

/*********************************************************
Function:	ProcIPCfgAlarm
Desc:		process alarm information when 
Input:	void
Output:	void
Return:	void
**********************************************************/
void CClientDemoDlg::ProcIPCfgAlarm(WPARAM wParam, LPARAM lParam)
{
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
	int iRet = 0;
	int i = 0;
	NET_DVR_IPALARMINFO struIPAlarmInfo;
	memcpy(&struIPAlarmInfo, pAlarmInfo, sizeof(NET_DVR_IPALARMINFO));
    for (i = 0; i < ARRAY_SIZE(struIPAlarmInfo.struIPChanInfo); i++)
    {
        memcpy(&g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].struStreamMode[i].uGetStream.struChanInfo, \
           &struIPAlarmInfo.struIPChanInfo[i], sizeof(NET_DVR_IPCHANINFO));
    }
	iRet |= memcmp(g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].byAnalogChanEnable, struIPAlarmInfo.byAnalogChanEnable, sizeof(struIPAlarmInfo.byAnalogChanEnable));
	for (i=0; i < MAX_IP_DEVICE; i++)
	{
		iRet |= CmpIPCfgAndIPCfgV31(&(g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].struIPDevInfo[i]), &(struIPAlarmInfo.struIPDevInfo[i]));
	}
	
	if (iRet != 0)
	{
        for (i = 0; i < ARRAY_SIZE(struIPAlarmInfo.struIPChanInfo); i++)
        {
            memcpy(&g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].struStreamMode[i].uGetStream.struChanInfo, \
                &struIPAlarmInfo.struIPChanInfo[i], sizeof(NET_DVR_IPCHANINFO));
        }
		memcpy(g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].byAnalogChanEnable, struIPAlarmInfo.byAnalogChanEnable, sizeof(struIPAlarmInfo.byAnalogChanEnable));
		memcpy(g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].struIPDevInfo, struIPAlarmInfo.struIPDevInfo, sizeof(struIPAlarmInfo.struIPDevInfo));
	//	memcpy(g_struDeviceInfo[iDeviceIndex].struAlarmInCfg.struIPAlarmInInfo, struIPAlarmInfo.struIPAlarmInInfo, MAX_IP_ALARMIN*sizeof(NET_DVR_IPALARMININFO));
	//	memcpy(g_struDeviceInfo[iDeviceIndex].struAlarmOutCfg.struIPAlarmOutInfo, struIPAlarmInfo.struIPAlarmOutInfo, MAX_IP_ALARMOUT*sizeof(NET_DVR_IPALARMOUTINFO));
		g_pMainDlg->RefreshIPDevLocalCfg(iDeviceIndex);
		g_pMainDlg->RedrawDeviceNode(iDeviceIndex);
	}
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, "ip parameter is channged!");
}


/*********************************************************
Function:	ProcIPCfgAlarmV31
Desc:		process alarm information when 
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::ProcIPCfgAlarmV31(WPARAM wParam, LPARAM lParam)
{
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
	int iRet = 0;
	NET_DVR_IPALARMINFO_V31 struIPAlarmInfo;
	memcpy(&struIPAlarmInfo, pAlarmInfo, sizeof(NET_DVR_IPALARMINFO_V31));
    int i = 0;
    for (i = 0; i < ARRAY_SIZE(struIPAlarmInfo.struIPChanInfo); i++)
    {
        memcpy(&g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].struStreamMode[i].uGetStream.struChanInfo, \
            &struIPAlarmInfo.struIPChanInfo[i], sizeof(NET_DVR_IPCHANINFO));
    }
	iRet |= memcmp(g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].byAnalogChanEnable, struIPAlarmInfo.byAnalogChanEnable, sizeof(struIPAlarmInfo.byAnalogChanEnable));
	iRet |= memcmp(g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].struIPDevInfo, struIPAlarmInfo.struIPDevInfo, sizeof(struIPAlarmInfo.struIPDevInfo));//9000_1.1 need change
	if (iRet != 0)
	{
        for (i = 0; i < ARRAY_SIZE(struIPAlarmInfo.struIPChanInfo); i++)
        {
            memcpy(&g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].struStreamMode[i].uGetStream.struChanInfo, \
                &struIPAlarmInfo.struIPChanInfo[i], sizeof(NET_DVR_IPCHANINFO));
        }
        memcpy(g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].byAnalogChanEnable, struIPAlarmInfo.byAnalogChanEnable, sizeof(struIPAlarmInfo.struIPChanInfo));
		memcpy(g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].struIPDevInfo, struIPAlarmInfo.struIPDevInfo, sizeof(struIPAlarmInfo.struIPDevInfo));
		//memcpy(g_struDeviceInfo[iDeviceIndex].struAlarmInCfg.struIPAlarmInInfo, struIPAlarmInfo.struIPAlarmInInfo, MAX_IP_ALARMIN*sizeof(NET_DVR_IPALARMININFO));
		//memcpy(g_struDeviceInfo[iDeviceIndex].struAlarmOutCfg.struIPAlarmOutInfo, struIPAlarmInfo.struIPAlarmOutInfo, MAX_IP_ALARMOUT*sizeof(NET_DVR_IPALARMOUTINFO));
		g_pMainDlg->RefreshIPDevLocalCfg(iDeviceIndex);
		g_pMainDlg->RedrawDeviceNode(iDeviceIndex);
	}
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, "ip parameter v31 is channged!");
}


void CClientDemoDlg::ProcPdcAlarm(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iWinIndex = -1;
	int i = 0;
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
	
	NET_DVR_PDC_ALRAM_INFO  struPDCAlarmInfo;
    memset(&struPDCAlarmInfo, 0, sizeof(struPDCAlarmInfo));
	CTime time = CTime::GetCurrentTime();
	memcpy(&struPDCAlarmInfo, pAlarmInfo, sizeof(struPDCAlarmInfo));
    i = struPDCAlarmInfo.byChannel-g_struDeviceInfo[iDeviceIndex].iStartChan;
    g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
	::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));	

	
    if (struPDCAlarmInfo.byMode == 0 )
    {
        sprintf(szInfoBuf,"DevIP:[%s]Port[%d]Channel[%d]single frame:Channel[%d]LeaveNum[%d]EnterNum[%d]Smart[%d]", struPDCAlarmInfo.struDevInfo.struDevIP.sIpV4,
            struPDCAlarmInfo.struDevInfo.wPort,struPDCAlarmInfo.struDevInfo.byChannel,
            struPDCAlarmInfo.byChannel, struPDCAlarmInfo.dwLeaveNum,struPDCAlarmInfo.dwEnterNum, struPDCAlarmInfo.bySmart);//2014-03-21
    }
    else if (struPDCAlarmInfo.byMode == 1)
    {
		sprintf(szInfoBuf,"DevIP:[%s]Port[%d]Channel[%d]:time range:Start Y[%d]M[%d]D[%d]H[%d]M[%d]S[%d]  End Y[%d]M[%d]D[%d]H[%d]M[%d]S[%d]Channel[%d]LeaveNum[%d]EnterNum[%d]Smart[%d]", 
            struPDCAlarmInfo.struDevInfo.struDevIP.sIpV4,struPDCAlarmInfo.struDevInfo.wPort,struPDCAlarmInfo.struDevInfo.byChannel,
            struPDCAlarmInfo.uStatModeParam.struStatTime.tmStart.dwYear,struPDCAlarmInfo.uStatModeParam.struStatTime.tmStart.dwMonth,struPDCAlarmInfo.uStatModeParam.struStatTime.tmStart.dwDay,
            struPDCAlarmInfo.uStatModeParam.struStatTime.tmStart.dwHour,struPDCAlarmInfo.uStatModeParam.struStatTime.tmStart.dwMinute,struPDCAlarmInfo.uStatModeParam.struStatTime.tmStart.dwSecond,
            
            struPDCAlarmInfo.uStatModeParam.struStatTime.tmEnd.dwYear,struPDCAlarmInfo.uStatModeParam.struStatTime.tmEnd.dwMonth,struPDCAlarmInfo.uStatModeParam.struStatTime.tmEnd.dwDay,
            struPDCAlarmInfo.uStatModeParam.struStatTime.tmEnd.dwHour,struPDCAlarmInfo.uStatModeParam.struStatTime.tmEnd.dwMinute,struPDCAlarmInfo.uStatModeParam.struStatTime.tmEnd.dwSecond,
            
            struPDCAlarmInfo.byChannel,struPDCAlarmInfo.dwLeaveNum,struPDCAlarmInfo.dwEnterNum, struPDCAlarmInfo.bySmart);//2014-03-21
    }
    g_pMainDlg->AddLog(iDeviceIndex,ALARM_INFO_T,szInfoBuf);
	
}

/*********************************************************
Function:	ProcRuleAlarm
Desc:		process behavior alarm information
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::ProcRuleAlarm(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iWinIndex = -1;         
	int i = 0;
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_VCA_RULE_ALARM  struVcaRuleAlarm;
	CTime time = CTime::GetCurrentTime();
	memcpy(&struVcaRuleAlarm, pAlarmInfo, sizeof(NET_VCA_RULE_ALARM));
	i = struVcaRuleAlarm.struDevInfo.byIvmsChannel-g_struDeviceInfo[iDeviceIndex].iStartChan;
	g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
	::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));	
	
	if (struVcaRuleAlarm.struRuleInfo.wEventTypeEx == ENUM_VCA_EVENT_TRAVERSE_PLANE)
	{
		g_pMainDlg->AddLog(iDeviceIndex,ALARM_INFO_T,"traverse plane detection");
	}
	if (struVcaRuleAlarm.struRuleInfo.dwEventType == ENUM_VCA_EVENT_INTRUSION)
	{
		g_pMainDlg->AddLog(iDeviceIndex,ALARM_INFO_T,"field detection");
	}

	//��Ϊ��������ϱ��ṹ
	sprintf(szInfoBuf,"DevIP:[%s]TargetID[%d]RuleID[%d]RuleName[%s]AbsTime[%d]piclen[%d]PicType[%d]RelAlarmPicNum[%d]AlarmID[%d]IvmsChannel[%d]EventTypeEx[%d]",
		struVcaRuleAlarm.struDevInfo.struDevIP.sIpV4,struVcaRuleAlarm.struTargetInfo.dwID,
        struVcaRuleAlarm.struRuleInfo.byRuleID,struVcaRuleAlarm.struRuleInfo.byRuleName,
		struVcaRuleAlarm.dwAbsTime, struVcaRuleAlarm.dwPicDataLen, struVcaRuleAlarm.byPicType,struVcaRuleAlarm.byRelAlarmPicNum,struVcaRuleAlarm.dwAlarmID,
		struVcaRuleAlarm.struDevInfo.byIvmsChannel,struVcaRuleAlarm.struRuleInfo.wEventTypeEx);
    g_pMainDlg->AddLog(iDeviceIndex,ALARM_INFO_T,szInfoBuf);
	
	
	NET_VCA_DEV_INFO tmpDevInfo;
	memcpy(&tmpDevInfo, &struVcaRuleAlarm.struDevInfo, sizeof(NET_VCA_DEV_INFO));
	sprintf(szInfoBuf,"\nNET_VCA_DEV_INFO:DevIP[%s]Port[%d]Channel[%d]\n",
		tmpDevInfo.struDevIP.sIpV4, tmpDevInfo.wPort, tmpDevInfo.byChannel);
	OutputDebugString(szInfoBuf);
	
	NET_VCA_TARGET_INFO tmpTargetInfo;
	memcpy(&tmpTargetInfo, &struVcaRuleAlarm.struTargetInfo, sizeof(NET_VCA_TARGET_INFO));
	sprintf(szInfoBuf, "NET_VCA_TARGET_INFO:ID[%d]RECT[%f][%f][%f][%f]\n",
		tmpTargetInfo.dwID, tmpTargetInfo.struRect.fX, tmpTargetInfo.struRect.fY,
		tmpTargetInfo.struRect.fWidth, tmpTargetInfo.struRect.fHeight);
	OutputDebugString(szInfoBuf);
	
	
	switch (struVcaRuleAlarm.struRuleInfo.wEventTypeEx)
	{
		
	case ENUM_VCA_EVENT_TRAVERSE_PLANE://
		{
			sprintf(szInfoBuf,"ENUM_VCA_EVENT_TRAVERSE_PLANE:struRul:strart[%f][%f]end[%f][%f]\ndwCrossDirection:[%d]bySensitivity[%d]\n"
				,struVcaRuleAlarm.struRuleInfo.uEventParam.struTraversePlane.struPlaneBottom.struStart.fX,
				struVcaRuleAlarm.struRuleInfo.uEventParam.struTraversePlane.struPlaneBottom.struStart.fY,
				struVcaRuleAlarm.struRuleInfo.uEventParam.struTraversePlane.struPlaneBottom.struEnd.fX,
				struVcaRuleAlarm.struRuleInfo.uEventParam.struTraversePlane.struPlaneBottom.struEnd.fY,
				struVcaRuleAlarm.struRuleInfo.uEventParam.struTraversePlane.dwCrossDirection/*,
				struVcaRuleAlarm.struRuleInfo.uEventParam.struTraversePlane.bySensitivity*/);
				OutputDebugString(szInfoBuf);
		}
		break;
	case ENUM_VCA_EVENT_ENTER_AREA:
		{
			// 			sprintf(szInfoBuf,"VCA_ENTER_AREA:Senetity[%d]\nNET_VCA_POLYGON",struVcaRuleAlarm.struRuleInfo.uEventParam.struArea.bySensitivity);
			// 			OutputDebugString(szInfoBuf);
			NET_VCA_POLYGON tempPolygon;
			memcpy(&tempPolygon,&struVcaRuleAlarm.struRuleInfo.uEventParam.struArea.struRegion,sizeof(NET_VCA_POLYGON));
			
			for (i = 0; i < (int)tempPolygon.dwPointNum ;i ++)
			{
				sprintf(szInfoBuf,"[%f][%f]",tempPolygon.struPos[i].fX,tempPolygon.struPos[i].fY);
				OutputDebugString(szInfoBuf);
			}
			
			
		}
		break;
	case ENUM_VCA_EVENT_EXIT_AREA:
		{
			// 			sprintf(szInfoBuf,"VCA_EXIT_AREA:Senetity[%d]\nNET_VCA_POLYGON",struVcaRuleAlarm.struRuleInfo.uEventParam.struArea.bySensitivity);
			// 			OutputDebugString(szInfoBuf);
			NET_VCA_POLYGON tempPolygon;
			memcpy(&tempPolygon,&struVcaRuleAlarm.struRuleInfo.uEventParam.struArea.struRegion,sizeof(NET_VCA_POLYGON));
			
			for (i = 0; i < (int)tempPolygon.dwPointNum ;i ++)
			{
				sprintf(szInfoBuf,"[%f][%f]",tempPolygon.struPos[i].fX,tempPolygon.struPos[i].fY);
				OutputDebugString(szInfoBuf);
			}
			
		}
		break;
	case ENUM_VCA_EVENT_INTRUSION:
		{
			// 			sprintf(szInfoBuf,"VCA_INTRUSION:Sensitivity[%d]\nNET_VCA_POLYGON",
			// 				struVcaRuleAlarm.struRuleInfo.uEventParam.struIntrusion.bySensitivity);
			// 			OutputDebugString(szInfoBuf);
			NET_VCA_POLYGON tempPolygon;
			memcpy(&tempPolygon,&struVcaRuleAlarm.struRuleInfo.uEventParam.struIntrusion.struRegion,sizeof(NET_VCA_POLYGON));
			
			for (i = 0; i < (int)tempPolygon.dwPointNum ;i ++)
			{
				sprintf(szInfoBuf,"[%f][%f]",tempPolygon.struPos[i].fX,tempPolygon.struPos[i].fY);
				OutputDebugString(szInfoBuf);
			}
			
		}
		break;
	case ENUM_VCA_EVENT_LOITER:
		{
			// 			sprintf(szInfoBuf,"VCA_LOITER:Senetity[%d]\nNET_VCA_POLYGON",
			// 				struVcaRuleAlarm.struRuleInfo.uEventParam.struLoiter.bySensitivity);
			// 			OutputDebugString(szInfoBuf);
			NET_VCA_POLYGON tempPolygon;
			memcpy(&tempPolygon,&struVcaRuleAlarm.struRuleInfo.uEventParam.struLoiter.struRegion,sizeof(NET_VCA_POLYGON));
			
			for (i = 0; i < (int)tempPolygon.dwPointNum ;i ++)
			{
				sprintf(szInfoBuf,"[%f][%f]",tempPolygon.struPos[i].fX,tempPolygon.struPos[i].fY);
				OutputDebugString(szInfoBuf);
			}
			
		}
		break;
	case ENUM_VCA_EVENT_LEFT_TAKE:
		{
			// 			sprintf(szInfoBuf,"VCA_LEFT_TAKE:Senetity[%d]\nNET_VCA_POLYGON",
			// 				struVcaRuleAlarm.struRuleInfo.uEventParam.struTakeTeft.bySensitivity);
			// 			OutputDebugString(szInfoBuf);
			NET_VCA_POLYGON tempPolygon;
			memcpy(&tempPolygon,&struVcaRuleAlarm.struRuleInfo.uEventParam.struTakeTeft.struRegion,sizeof(NET_VCA_POLYGON));
			
			for (i = 0; i < (int)tempPolygon.dwPointNum ;i ++)
			{
				sprintf(szInfoBuf,"[%f][%f]",tempPolygon.struPos[i].fX,tempPolygon.struPos[i].fY);
				OutputDebugString(szInfoBuf);
			}
			
		}
		break;
	case ENUM_VCA_EVENT_PARKING:
		{
			// 			sprintf(szInfoBuf,"VCA_PARKING:Senetity[%d]\nNET_VCA_POLYGON",
			// 				struVcaRuleAlarm.struRuleInfo.uEventParam.struParking.bySensitivity);
			// 			OutputDebugString(szInfoBuf);
			NET_VCA_POLYGON tempPolygon;
			memcpy(&tempPolygon,&struVcaRuleAlarm.struRuleInfo.uEventParam.struParking.struRegion,sizeof(NET_VCA_POLYGON));
			
			for (i = 0; i < (int)tempPolygon.dwPointNum ;i ++)
			{
				sprintf(szInfoBuf,"[%f][%f]",tempPolygon.struPos[i].fX,tempPolygon.struPos[i].fY);
				OutputDebugString(szInfoBuf);
			}
			
		}
		break;
	case ENUM_VCA_EVENT_RUN:
		{
			// 			sprintf(szInfoBuf,"VCA_RUN:Senetity[%d]\nNET_VCA_POLYGON",
			// 				struVcaRuleAlarm.struRuleInfo.uEventParam.struRun.bySensitivity);
			// 			OutputDebugString(szInfoBuf);
			NET_VCA_POLYGON tempPolygon;
			memcpy(&tempPolygon,&struVcaRuleAlarm.struRuleInfo.uEventParam.struRun.struRegion,sizeof(NET_VCA_POLYGON));
			
			for (i = 0; i < (int)tempPolygon.dwPointNum ;i ++)
			{
				sprintf(szInfoBuf,"[%f][%f]",tempPolygon.struPos[i].fX,tempPolygon.struPos[i].fY);
				OutputDebugString(szInfoBuf);
			}
		}
		break;
	case ENUM_VCA_EVENT_HIGH_DENSITY:
		{
			// 			sprintf(szInfoBuf,"VCA_HIGH_DENSITY:Senetity[%d]\nNET_VCA_POLYGON",
			// 				struVcaRuleAlarm.struRuleInfo.uEventParam.struHighDensity.bySensitivity);
			// 			OutputDebugString(szInfoBuf);
			NET_VCA_POLYGON tempPolygon;
			memcpy(&tempPolygon,&struVcaRuleAlarm.struRuleInfo.uEventParam.struHighDensity.struRegion,sizeof(NET_VCA_POLYGON));
			
			for (i = 0; i < (int)tempPolygon.dwPointNum ;i ++)
			{
				sprintf(szInfoBuf,"[%f][%f]",tempPolygon.struPos[i].fX,tempPolygon.struPos[i].fY);
				OutputDebugString(szInfoBuf);
			}
			
		}
		break;
	case ENUM_VCA_EVENT_LECTURE:
	case VCA_LECTURE:
		{
			NET_VCA_POLYGON tempPolygon;
			memcpy(&tempPolygon,&struVcaRuleAlarm.struRuleInfo.uEventParam.struLecture.struRegion,sizeof(NET_VCA_POLYGON));
			
			for (i = 0; i < (int)tempPolygon.dwPointNum ;i ++)
			{
				if (struVcaRuleAlarm.struRuleInfo.uEventParam.struLecture.byAlarmState == 1)
				{
					sprintf(szInfoBuf,"Lecture  Event Start[%f][%f]",tempPolygon.struPos[i].fX,tempPolygon.struPos[i].fY);	
				}
				else if (struVcaRuleAlarm.struRuleInfo.uEventParam.struLecture.byAlarmState == 2)
				{
					sprintf(szInfoBuf,"Lecture  Event End[%f][%f]",tempPolygon.struPos[i].fX,tempPolygon.struPos[i].fY);
				}
				
				OutputDebugString(szInfoBuf);
				g_pMainDlg->AddLog(iDeviceIndex,ALARM_INFO_T,szInfoBuf);
			}
			
		}
		break;
	case ENUM_VCA_EVENT_ANSWER:
	case VCA_ANSWER:
		{
			NET_VCA_POLYGON tempPolygon;
			memcpy(&tempPolygon,&struVcaRuleAlarm.struRuleInfo.uEventParam.struAnswer.struRegion,sizeof(NET_VCA_POLYGON));
			
			for (i = 0; i < (int)tempPolygon.dwPointNum ;i ++)
			{
				if (struVcaRuleAlarm.struRuleInfo.uEventParam.struAnswer.byAlarmState == 1)
				{
					sprintf(szInfoBuf,"Answering Questions Event [%f][%f]",tempPolygon.struPos[i].fX,tempPolygon.struPos[i].fY);
				}
				else if(struVcaRuleAlarm.struRuleInfo.uEventParam.struAnswer.byAlarmState == 2)
				{
					sprintf(szInfoBuf,"Answering Questions Event [%f][%f]",tempPolygon.struPos[i].fX,tempPolygon.struPos[i].fY);
				}
				OutputDebugString(szInfoBuf);
				g_pMainDlg->AddLog(iDeviceIndex,ALARM_INFO_T,szInfoBuf);
			}
			
		}
		break;
	default:
		break;
	}
	
	if (struVcaRuleAlarm.dwPicDataLen > 0 && struVcaRuleAlarm.pImage != NULL)
	{
		int num = ulAlarmPic/1000+1;
		ulAlarmPic++;
		
		char cFilename[256] = {0};
		HANDLE hFile;
		DWORD dwReturn;
        
        SYSTEMTIME t;
        GetLocalTime(&t);
        char chTime[128];
        sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
		
		sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, tmpDevInfo.struDevIP.sIpV4/*g_struDeviceInfo[iDeviceIndex].chDeviceIPInFileName*/, num);
		if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
		{
			CreateDirectory(cFilename, NULL);
		}
        
		sprintf(cFilename, "%s\\LocalTime[%s]TargetID[%d]RuleName[%s]AlarmID[%d]_%d.jpg",cFilename, chTime, \
			struVcaRuleAlarm.struTargetInfo.dwID, struVcaRuleAlarm.struRuleInfo.byRuleName, struVcaRuleAlarm.dwAlarmID,rand() % GetTickCount());
		hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}
		WriteFile(hFile, struVcaRuleAlarm.pImage, struVcaRuleAlarm.dwPicDataLen, &dwReturn, NULL);
		CloseHandle(hFile);
		hFile = NULL;
	}
	if (struVcaRuleAlarm.pImage != NULL)
	{
		delete [] (struVcaRuleAlarm.pImage);
	}					
}

void CClientDemoDlg::ProcAidAlarm(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iWinIndex = -1;         
	int i = 0;
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_AID_ALARM  struAidAlarm = {0};
	CTime time = CTime::GetCurrentTime();
	memcpy(&struAidAlarm, pAlarmInfo, sizeof(struAidAlarm));
	i = struAidAlarm.struDevInfo.byIvmsChannel-g_struDeviceInfo[iDeviceIndex].iStartChan;
	g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
	::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));

    NET_DVR_TIME struAbsTime = {0};     
    struAbsTime.dwYear = GET_YEAR(struAidAlarm.dwAbsTime);
    struAbsTime.dwMonth = GET_MONTH(struAidAlarm.dwAbsTime);
    struAbsTime.dwDay = GET_DAY(struAidAlarm.dwAbsTime);
    struAbsTime.dwHour = GET_HOUR(struAidAlarm.dwAbsTime);
    struAbsTime.dwMinute = GET_MINUTE(struAidAlarm.dwAbsTime);
    struAbsTime.dwSecond = GET_SECOND(struAidAlarm.dwAbsTime);

    CString str = "";
    sprintf(szInfoBuf, "Aid Alarm IvmsChan[%d]ID[%d]Name[%s]Type[%d]Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d]Dev IP[%s]Port[%d]Chan[%d] \
        Direction Start[%0.3f,%0.3f]Stop[%0.3f,%0.3f]", 
        struAidAlarm.struDevInfo.byIvmsChannel, struAidAlarm.struAIDInfo.byRuleID, struAidAlarm.struAIDInfo.byRuleName,
        struAidAlarm.struAIDInfo.dwAIDType, struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay,
        struAbsTime.dwHour, struAbsTime.dwMinute, struAbsTime.dwSecond, struAidAlarm.struDevInfo.struDevIP.sIpV4,
        struAidAlarm.struDevInfo.wPort, struAidAlarm.struDevInfo.byChannel,
        struAidAlarm.struAIDInfo.struDirect.struStartPoint.fX, struAidAlarm.struAIDInfo.struDirect.struStartPoint.fY,
        struAidAlarm.struAIDInfo.struDirect.struEndPoint.fX, struAidAlarm.struAIDInfo.struDirect.struEndPoint.fY);

    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);

    if (struAidAlarm.dwPicDataLen > 0 && struAidAlarm.pImage != NULL)
	{
		int num = ulAlarmPic/1000+1;
		ulAlarmPic++;
		
		char cFilename[256] = {0};
		HANDLE hFile;
		DWORD dwReturn;
        
        SYSTEMTIME t;
        GetLocalTime(&t);
        char chTime[128];
        sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
		
		sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIPInFileName, num);
		if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
		{
			CreateDirectory(cFilename, NULL);
		}
        
		sprintf(cFilename, "%s\\LocalTime[%s].jpg",cFilename, chTime);
		hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}
		WriteFile(hFile, struAidAlarm.pImage, struAidAlarm.dwPicDataLen, &dwReturn, NULL);
		CloseHandle(hFile);
		hFile = NULL;
	}
	if (struAidAlarm.pImage != NULL)
	{
		delete [] (struAidAlarm.pImage);
        struAidAlarm.pImage = NULL;
	}		
}

void CClientDemoDlg::ProcTpsAlarm(WPARAM wParam, LPARAM lParam)
{

    char szInfoBuf[1024] = {0};
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iWinIndex = -1;
    unsigned int i = 0;
    int iDeviceIndex = pAlarmDev->iDeviceIndex;

    NET_DVR_TPS_ALARM struTpsAlarm = {0};
    memcpy(&struTpsAlarm, pAlarmInfo, sizeof(struTpsAlarm));
    i = struTpsAlarm.struDevInfo.byIvmsChannel - g_struDeviceInfo[iDeviceIndex].iStartChan;
    g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
    ::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));
    
    NET_DVR_TIME  struAbsTime = {0};
    struAbsTime.dwYear = GET_YEAR(struTpsAlarm.dwAbsTime);
    struAbsTime.dwMonth = GET_MONTH(struTpsAlarm.dwAbsTime);
    struAbsTime.dwDay = GET_DAY(struTpsAlarm.dwAbsTime);
    struAbsTime.dwHour = GET_HOUR(struTpsAlarm.dwAbsTime);
    struAbsTime.dwMinute = GET_MINUTE(struTpsAlarm.dwAbsTime);
    struAbsTime.dwSecond = GET_SECOND(struTpsAlarm.dwAbsTime);
    
    sprintf(szInfoBuf, "Tps alarm Dev ip[%s]port[%d]Chan[%d] IvmsChan[%d] Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d] LaneNum[%d]" , \
        struTpsAlarm.struDevInfo.struDevIP.sIpV4, struTpsAlarm.struDevInfo.wPort, struTpsAlarm.struDevInfo.byChannel,struTpsAlarm.struDevInfo.byIvmsChannel, 
        struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay, struAbsTime.dwHour, struAbsTime.dwMinute, struAbsTime.dwSecond);

    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);

    for (i = 0; i < struTpsAlarm.struTPSInfo.dwLanNum; i++)
    {
		sprintf(szInfoBuf, "ID[%d] Name[%s] VaryType[%d] LaneType[%d] Volume[%d] Velocity[%d] TimeHeadway[%d] SpaceHeadway[%d] SpceOccupy[%0.3f]",
            struTpsAlarm.struTPSInfo.struLaneParam[i].byRuleID, struTpsAlarm.struTPSInfo.struLaneParam[i].byRuleName,
            struTpsAlarm.struTPSInfo.struLaneParam[i].byVaryType, struTpsAlarm.struTPSInfo.struLaneParam[i].byLaneType,
            struTpsAlarm.struTPSInfo.struLaneParam[i].dwLaneVolume, struTpsAlarm.struTPSInfo.struLaneParam[i].dwLaneVelocity,
            struTpsAlarm.struTPSInfo.struLaneParam[i].dwTimeHeadway, struTpsAlarm.struTPSInfo.struLaneParam[i].dwSpaceHeadway,
            struTpsAlarm.struTPSInfo.struLaneParam[i].fSpaceOccupyRation);
        g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
    }
}

void CClientDemoDlg::ProcTpsAlarmV41(WPARAM wParam,LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo  = (char *)(lParam);		
    int iWinIndex     = -1;
    int i             = 0;
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
	
    NET_DVR_TPS_ALARM_V41 struTpsAlarm = {0};
    memcpy(&struTpsAlarm, pAlarmInfo, sizeof(struTpsAlarm));
    i = struTpsAlarm.struDevInfo.byIvmsChannel - g_struDeviceInfo[iDeviceIndex].iStartChan;
    g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
    ::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));
    
    NET_DVR_TIME  struAbsTime = {0};
    struAbsTime.dwYear   =  GET_YEAR(struTpsAlarm.dwAbsTime);
    struAbsTime.dwMonth  =  GET_MONTH(struTpsAlarm.dwAbsTime);
    struAbsTime.dwDay    =  GET_DAY(struTpsAlarm.dwAbsTime);
    struAbsTime.dwHour   =  GET_HOUR(struTpsAlarm.dwAbsTime);
    struAbsTime.dwMinute =  GET_MINUTE(struTpsAlarm.dwAbsTime);
    struAbsTime.dwSecond =  GET_SECOND(struTpsAlarm.dwAbsTime);

	NET_DVR_TIME  struStartTime = {0};
    struStartTime.dwYear   =  GET_YEAR(struTpsAlarm.dwStartTime);
    struStartTime.dwMonth  =  GET_MONTH(struTpsAlarm.dwStartTime);
    struStartTime.dwDay    =  GET_DAY(struTpsAlarm.dwStartTime);
    struStartTime.dwHour   =  GET_HOUR(struTpsAlarm.dwStartTime);
    struStartTime.dwMinute =  GET_MINUTE(struTpsAlarm.dwStartTime);
    struStartTime.dwSecond =  GET_SECOND(struTpsAlarm.dwStartTime);

	NET_DVR_TIME  struStopTime = {0};
    struStopTime.dwYear   =  GET_YEAR(struTpsAlarm.dwStopTime);
    struStopTime.dwMonth  =  GET_MONTH(struTpsAlarm.dwStopTime);
    struStopTime.dwDay    =  GET_DAY(struTpsAlarm.dwStopTime);
    struStopTime.dwHour   =  GET_HOUR(struTpsAlarm.dwStopTime);
    struStopTime.dwMinute =  GET_MINUTE(struTpsAlarm.dwStopTime);
    struStopTime.dwSecond =  GET_SECOND(struTpsAlarm.dwStopTime);

    
    sprintf(szInfoBuf, "Tps alarm v41 Dev ip[%s]port[%d]Chan[%d] IvmsChan[%d] Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d] LaneNum[%d] MonitoringSiteID[%s] DeviceID[%s] StartTime[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d] StopTime[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d]" , \
        struTpsAlarm.struDevInfo.struDevIP.sIpV4, struTpsAlarm.struDevInfo.wPort, struTpsAlarm.struDevInfo.byChannel,struTpsAlarm.struDevInfo.byIvmsChannel, 
        struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay, struAbsTime.dwHour, struAbsTime.dwMinute, struAbsTime.dwSecond,struTpsAlarm.struTPSInfo.dwLanNum,
		struTpsAlarm.byMonitoringSiteID, struTpsAlarm.byDeviceID, 
		struStartTime.dwYear, struStartTime.dwMonth, struStartTime.dwDay, struStartTime.dwHour, struStartTime.dwMinute, struStartTime.dwSecond,
		struStopTime.dwYear, struStopTime.dwMonth, struStopTime.dwDay, struStopTime.dwHour, struStopTime.dwMinute, struStopTime.dwSecond);
	
    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
	
    for (i = 0; i < struTpsAlarm.struTPSInfo.dwLanNum; i++)
    {
        sprintf(szInfoBuf, "ID[%d] Name[%s] VaryType[%d] LaneType[%d] Volume[%d] Velocity[%d] TimeHeadway[%d] SpaceHeadway[%d] SpaceOccupy[%0.3f] \
			TimeOccupy[%.3f] LightVehicle[%d] MidVehicle[%d] HeavyVehicle[%d] OversizeVehicle[%d] TrafficState[%d] LaneQueueHead[%0.3f,%0.3f] LaneQueueTail[%0.3f,%0.3f]  \
			LaneQueueLength[%d] LaneNo[%d]",
            struTpsAlarm.struTPSInfo.struLaneParam[i].byRuleID, struTpsAlarm.struTPSInfo.struLaneParam[i].byRuleName,
            struTpsAlarm.struTPSInfo.struLaneParam[i].dwVaryType, struTpsAlarm.struTPSInfo.struLaneParam[i].byLaneType,
            struTpsAlarm.struTPSInfo.struLaneParam[i].dwLaneVolume, struTpsAlarm.struTPSInfo.struLaneParam[i].dwLaneVelocity,
            struTpsAlarm.struTPSInfo.struLaneParam[i].dwTimeHeadway, struTpsAlarm.struTPSInfo.struLaneParam[i].dwSpaceHeadway,
            struTpsAlarm.struTPSInfo.struLaneParam[i].fSpaceOccupyRation, struTpsAlarm.struTPSInfo.struLaneParam[i].fTimeOccupyRation,
			struTpsAlarm.struTPSInfo.struLaneParam[i].dwLightVehicle, struTpsAlarm.struTPSInfo.struLaneParam[i].dwMidVehicle,
			struTpsAlarm.struTPSInfo.struLaneParam[i].dwHeavyVehicle, struTpsAlarm.struTPSInfo.struLaneParam[i].dwOversizeVehicle,
			struTpsAlarm.struTPSInfo.struLaneParam[i].byTrafficState,
			struTpsAlarm.struTPSInfo.struLaneParam[i].struLaneQueue.struHead.fX,struTpsAlarm.struTPSInfo.struLaneParam[i].struLaneQueue.struHead.fY,
			struTpsAlarm.struTPSInfo.struLaneParam[i].struLaneQueue.struTail.fX,struTpsAlarm.struTPSInfo.struLaneParam[i].struLaneQueue.struTail.fY,
			struTpsAlarm.struTPSInfo.struLaneParam[i].struLaneQueue.dwLength,
			struTpsAlarm.struTPSInfo.struLaneParam[i].byLaneNo);
        g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
    }
}


void CClientDemoDlg::ProcAidAlarmV41(WPARAM wParam,LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iWinIndex = -1;         
	int i = 0;
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_AID_ALARM_V41  struAidAlarm = {0};
	CTime time = CTime::GetCurrentTime();
	memcpy(&struAidAlarm, pAlarmInfo, sizeof(struAidAlarm));
	i = struAidAlarm.struDevInfo.byIvmsChannel-g_struDeviceInfo[iDeviceIndex].iStartChan;
	g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
	::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));
	
    NET_DVR_TIME struAbsTime = {0};     
    struAbsTime.dwYear   = GET_YEAR(struAidAlarm.dwAbsTime);
    struAbsTime.dwMonth  = GET_MONTH(struAidAlarm.dwAbsTime);
    struAbsTime.dwDay    = GET_DAY(struAidAlarm.dwAbsTime);
    struAbsTime.dwHour   = GET_HOUR(struAidAlarm.dwAbsTime);
    struAbsTime.dwMinute = GET_MINUTE(struAidAlarm.dwAbsTime);
    struAbsTime.dwSecond = GET_SECOND(struAidAlarm.dwAbsTime);
	
	CString str = "";
    sprintf(szInfoBuf, "Aid AlarmV41 IvmsChan[%d]ID[%d]Name[%s]Type[%d] LaneNo[%d] Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d]Dev IP[%s]Port[%d]Chan[%d] \
        Direction Start[%0.3f,%0.3f]Stop[%0.3f,%0.3f] SceneID[%d] SceneName[%s] Direction[%d] PTZ[%d,%d,%d] MonitoringSiteID[%s] DeviceID[%s]", 
        struAidAlarm.struDevInfo.byIvmsChannel, struAidAlarm.struAIDInfo.byRuleID, struAidAlarm.struAIDInfo.byRuleName,
        struAidAlarm.struAIDInfo.dwAIDType, struAidAlarm.byLaneNo, struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay,
        struAbsTime.dwHour, struAbsTime.dwMinute, struAbsTime.dwSecond, struAidAlarm.struDevInfo.struDevIP.sIpV4,
        struAidAlarm.struDevInfo.wPort, struAidAlarm.struDevInfo.byChannel,
        struAidAlarm.struAIDInfo.struDirect.struStartPoint.fX, struAidAlarm.struAIDInfo.struDirect.struStartPoint.fY,
        struAidAlarm.struAIDInfo.struDirect.struEndPoint.fX, struAidAlarm.struAIDInfo.struDirect.struEndPoint.fY,
		struAidAlarm.struSceneInfo.dwSceneID,struAidAlarm.struSceneInfo.bySceneName,struAidAlarm.struSceneInfo.byDirection,
		struAidAlarm.struSceneInfo.struPtzPos.wPanPos,struAidAlarm.struSceneInfo.struPtzPos.wTiltPos,struAidAlarm.struSceneInfo.struPtzPos.wZoomPos,
		struAidAlarm.byMonitoringSiteID, struAidAlarm.byDeviceID);
	
    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
	
    if (struAidAlarm.dwPicDataLen > 0 && struAidAlarm.pImage != NULL)
	{
		int num = ulAlarmPic/1000+1;
		ulAlarmPic++;
		
		char cFilename[256] = {0};
		HANDLE hFile;
		DWORD dwReturn;
        
        SYSTEMTIME t;
        GetLocalTime(&t);
        char chTime[128];
        sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
		
		sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIPInFileName, num);
		if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
		{
			CreateDirectory(cFilename, NULL);
		}
        
		if (struAidAlarm.byDataType == 0)
		{
			sprintf(cFilename, "%s\\LocalTime[%s].jpg",cFilename, chTime);
		} 
		else if (struAidAlarm.byDataType == 1)
		{
			sprintf(cFilename, "%s\\URL_LocalTime[%s].txt",cFilename, chTime);
		}

		//sprintf(cFilename, "%s\\LocalTime[%s].jpg",cFilename, chTime);
		hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}
		WriteFile(hFile, struAidAlarm.pImage, struAidAlarm.dwPicDataLen, &dwReturn, NULL);
		CloseHandle(hFile);
		hFile = NULL;
	}
	if (struAidAlarm.pImage != NULL)
	{
		delete [] (struAidAlarm.pImage);
        struAidAlarm.pImage = NULL;
	}		
}

void CClientDemoDlg::ProcTfsAlarm(WPARAM wParam,LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iWinIndex = -1;         
	int i = 0;
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_TFS_ALARM struTfsAlarm = {0};	
	memcpy(&struTfsAlarm, pAlarmInfo, sizeof(struTfsAlarm));
	i = struTfsAlarm.struDevInfo.byIvmsChannel-g_struDeviceInfo[iDeviceIndex].iStartChan;
	g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
	::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));
	
	NET_DVR_TIME struAbsTime = {0};     
    struAbsTime.dwYear = GET_YEAR(struTfsAlarm.dwAbsTime);
    struAbsTime.dwMonth = GET_MONTH(struTfsAlarm.dwAbsTime);
    struAbsTime.dwDay = GET_DAY(struTfsAlarm.dwAbsTime);
    struAbsTime.dwHour = GET_HOUR(struTfsAlarm.dwAbsTime);
    struAbsTime.dwMinute = GET_MINUTE(struTfsAlarm.dwAbsTime);
    struAbsTime.dwSecond = GET_SECOND(struTfsAlarm.dwAbsTime);
	
	CString str = "";
    sprintf(szInfoBuf, "Tfs Alarm IvmsChan[%d]ID[%d]Name[%s]Type[%d]Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d]Dev IP[%s]Port[%d]Chan[%d] SpecificVehicleType[%d]\
        LaneNo[%d] Direction Start[%0.3f,%0.3f]Stop[%0.3f,%0.3f] SceneID[%d] SceneName[%s] Direction[%d] PTZ[%d,%d,%d] IllegalType[%d] IllegalTime[%d]\
		MonitorSiteID[%s] DeviceID[%s] Licence[%s] Believe[%s]", 
        struTfsAlarm.struDevInfo.byIvmsChannel, struTfsAlarm.struAIDInfo.byRuleID, struTfsAlarm.struAIDInfo.byRuleName,
        struTfsAlarm.struAIDInfo.dwAIDType, struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay,
        struAbsTime.dwHour, struAbsTime.dwMinute, struAbsTime.dwSecond, struTfsAlarm.struDevInfo.struDevIP.sIpV4,
        struTfsAlarm.struDevInfo.wPort, struTfsAlarm.struDevInfo.byChannel,struTfsAlarm.bySpecificVehicleType,
		struTfsAlarm.byLaneNo,
        struTfsAlarm.struAIDInfo.struDirect.struStartPoint.fX, struTfsAlarm.struAIDInfo.struDirect.struStartPoint.fY,
        struTfsAlarm.struAIDInfo.struDirect.struEndPoint.fX, struTfsAlarm.struAIDInfo.struDirect.struEndPoint.fY,
		struTfsAlarm.struSceneInfo.dwSceneID,struTfsAlarm.struSceneInfo.bySceneName,struTfsAlarm.struSceneInfo.byDirection,
		struTfsAlarm.struSceneInfo.struPtzPos.wPanPos,struTfsAlarm.struSceneInfo.struPtzPos.wTiltPos,struTfsAlarm.struSceneInfo.struPtzPos.wZoomPos,
		struTfsAlarm.dwIllegalType,struTfsAlarm.dwIllegalDuration,struTfsAlarm.byMonitoringSiteID,struTfsAlarm.byDeviceID,
		struTfsAlarm.struPlateInfo.sLicense,struTfsAlarm.struPlateInfo.byBelieve);
	
    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
	
	for (i = 0; i < 8; i++)
	{
		if (struTfsAlarm.struPicInfo[i].dwDataLen > 0 && struTfsAlarm.struPicInfo[i].pBuffer != NULL)
		{
			sprintf(szInfoBuf, "TFS picture[%d] info Type[%d] ", i+1, struTfsAlarm.struPicInfo[i].byType);
			g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);	
			
			int num = ulAlarmPic/1000+1;
			ulAlarmPic++;
			
			char cFilename[256] = {0};
			HANDLE hFile;
			DWORD dwReturn;
			
			SYSTEMTIME t;
			GetLocalTime(&t);
			char chTime[128];
			sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
			
			sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIPInFileName, num);
			if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
			{
				CreateDirectory(cFilename, NULL);
			}
			
			if (struTfsAlarm.struPicInfo[i].byDataType == 0)
			{
				sprintf(cFilename, "%s\\LocalTime[%s]_%d.jpg",cFilename, chTime, i);
			} 
			else if(struTfsAlarm.struPicInfo[i].byDataType == 1)
			{
				sprintf(cFilename, "%s\\URL_LocalTime[%s]_%d.txt", cFilename, chTime, i);
			}

			//sprintf(cFilename, "%s\\LocalTime[%s]_%d.jpg",cFilename, chTime, i);
			hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				return;
			}
			WriteFile(hFile, struTfsAlarm.struPicInfo[i].pBuffer,struTfsAlarm.struPicInfo[i].dwDataLen, &dwReturn, NULL);
			CloseHandle(hFile);
			hFile = NULL;
		}
	} 
}

void CClientDemoDlg::ProcFaceDetectAlarm(WPARAM wParam, LPARAM lParam)
{
    char szInfoBuf[1024] = {0};
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_FACEDETECT_ALARM struFaceDetectAlarm = {0};
    memcpy(&struFaceDetectAlarm, pAlarmInfo, sizeof(struFaceDetectAlarm));
	
    int i = struFaceDetectAlarm.struDevInfo.byChannel - g_struDeviceInfo[iDeviceIndex].iStartChan;
    g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
    ::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));
//  ��ӡ��Ϣ
    NET_DVR_TIME  struAbsTime = {0};
    struAbsTime.dwYear = GET_YEAR(struFaceDetectAlarm.dwAbsTime);
    struAbsTime.dwMonth = GET_MONTH(struFaceDetectAlarm.dwAbsTime);
    struAbsTime.dwDay = GET_DAY(struFaceDetectAlarm.dwAbsTime);
    struAbsTime.dwHour = GET_HOUR(struFaceDetectAlarm.dwAbsTime);
    struAbsTime.dwMinute = GET_MINUTE(struFaceDetectAlarm.dwAbsTime);
    struAbsTime.dwSecond = GET_SECOND(struFaceDetectAlarm.dwAbsTime);
	
    sprintf(szInfoBuf, "Face alarm Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d] RuleName[%s] AlarmPicType[%d] PanelChan[%d] RelAlarmPicNum[%d] AlarmID[%d]",
        struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay, struAbsTime.dwHour,
        struAbsTime.dwMinute, struAbsTime.dwSecond, struFaceDetectAlarm.byRuleName,struFaceDetectAlarm.byAlarmPicType,struFaceDetectAlarm.byPanelChan,
		struFaceDetectAlarm.byRelAlarmPicNum,struFaceDetectAlarm.dwAlarmID);
    AddLog(m_iCurDeviceIndex, ALARM_INFO_T, szInfoBuf);
	
    if (struFaceDetectAlarm.dwPicDataLen > 0 && struFaceDetectAlarm.pImage != NULL)
    {
        int num = ulAlarmPic/1000+1;
        ulAlarmPic++;
        
        char cFilename[256] = {0};
        HANDLE hFile;
        DWORD dwReturn;
        
        SYSTEMTIME t;
        GetLocalTime(&t);
        char chTime[128];
        sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
        
        sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIPInFileName, num);
        if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
        {
            CreateDirectory(cFilename, NULL);
        }
        
		sprintf(cFilename, "%s\\LocalTime[%s]AlarmID[%d]_%d.jpg",cFilename, chTime,struFaceDetectAlarm.dwAlarmID,rand() % GetTickCount());
        hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            return;
        }
        WriteFile(hFile, struFaceDetectAlarm.pImage, struFaceDetectAlarm.dwPicDataLen, &dwReturn, NULL);
        CloseHandle(hFile);
        hFile = NULL;
	}
	
	if (struFaceDetectAlarm.dwFacePicDataLen > 0 && struFaceDetectAlarm.pFaceImage != NULL)
    {
        int num = ulAlarmPic/1000+1;
        ulAlarmPic++;
        
        char cFilename[256] = {0};
        HANDLE hFile;
        DWORD dwReturn;
        
        SYSTEMTIME t;
        GetLocalTime(&t);
        char chTime[128];
        sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
        
        sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIPInFileName, num);
        if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
        {
            CreateDirectory(cFilename, NULL);
        }
        
        sprintf(cFilename, "%s\\Face_LocalTime[%s]AlarmID[%d]_%d.jpg",cFilename, chTime,struFaceDetectAlarm.dwAlarmID,rand() % GetTickCount());
        hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            return;
        }
        WriteFile(hFile, struFaceDetectAlarm.pFaceImage, struFaceDetectAlarm.dwFacePicDataLen, &dwReturn, NULL);
        CloseHandle(hFile);
        hFile = NULL;
	}
	
    if (struFaceDetectAlarm.pImage != NULL)
    {
        delete [] (struFaceDetectAlarm.pImage);
        struFaceDetectAlarm.pImage = NULL;
	}
	
	if (struFaceDetectAlarm.pFaceImage != NULL)
    {
        delete [] (struFaceDetectAlarm.pFaceImage);
        struFaceDetectAlarm.pFaceImage = NULL;
	}
}

void CClientDemoDlg::ProcFaceSnapAlarm(WPARAM wParam, LPARAM lParam)
{
    char szInfoBuf[1024] = {0};
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_VCA_FACESNAP_RESULT struFaceSnapAlarm = {0};
    memcpy(&struFaceSnapAlarm, pAlarmInfo, sizeof(struFaceSnapAlarm));
	
    int i = struFaceSnapAlarm.struDevInfo.byChannel - g_struDeviceInfo[iDeviceIndex].iStartChan;
    g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
    ::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));
	//  ��ӡ��Ϣ
    NET_DVR_TIME  struAbsTime = {0};
    struAbsTime.dwYear = GET_YEAR(struFaceSnapAlarm.dwAbsTime);
    struAbsTime.dwMonth = GET_MONTH(struFaceSnapAlarm.dwAbsTime);
    struAbsTime.dwDay = GET_DAY(struFaceSnapAlarm.dwAbsTime);
    struAbsTime.dwHour = GET_HOUR(struFaceSnapAlarm.dwAbsTime);
    struAbsTime.dwMinute = GET_MINUTE(struFaceSnapAlarm.dwAbsTime);
    struAbsTime.dwSecond = GET_SECOND(struFaceSnapAlarm.dwAbsTime);
	
    sprintf(szInfoBuf, "Face Snap alarm Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d] FaceID[%d] Smart[%d] AgeGroup[%d] Sex[%d] EyeGlass[%d] StayDuration[%.3f] FaceScore[%d]\n",\
        struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay, struAbsTime.dwHour,struAbsTime.dwMinute, struAbsTime.dwSecond, \
		struFaceSnapAlarm.dwFacePicID, struFaceSnapAlarm.bySmart, struFaceSnapAlarm.struFeature.byAgeGroup, struFaceSnapAlarm.struFeature.bySex, \
		struFaceSnapAlarm.struFeature.byEyeGlass, struFaceSnapAlarm.fStayDuration, struFaceSnapAlarm.dwFaceScore);
    AddLog(m_iCurDeviceIndex, ALARM_INFO_T, szInfoBuf);
	
    if (struFaceSnapAlarm.dwFacePicLen > 0 && struFaceSnapAlarm.pBuffer1 != NULL)
    {
        int num = ulAlarmPic/1000+1;
        ulAlarmPic++;
        
        char cFilename[256] = {0};
        HANDLE hFile;
        DWORD dwReturn;
        
        SYSTEMTIME t;
        GetLocalTime(&t);
        char chTime[128];
        sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
        
        sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIP, num);
        if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
        {
            CreateDirectory(cFilename, NULL);
        }
        
        sprintf(cFilename, "%s\\SnapPic[%s].jpg",cFilename, chTime);
        hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            return;
        }
        WriteFile(hFile, struFaceSnapAlarm.pBuffer1, struFaceSnapAlarm.dwFacePicLen, &dwReturn, NULL);
        CloseHandle(hFile);
        hFile = NULL;
	}
	
	if (struFaceSnapAlarm.dwBackgroundPicLen > 0 && struFaceSnapAlarm.pBuffer2 != NULL)
    {
        int num = ulAlarmPic/1000+1;
        ulAlarmPic++;
        
        char cFilename[256] = {0};
        HANDLE hFile;
        DWORD dwReturn;
        
        SYSTEMTIME t;
        GetLocalTime(&t);
        char chTime[128];
        sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
        
        sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIP, num);
        if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
        {
            CreateDirectory(cFilename, NULL);
        }
        
        sprintf(cFilename, "%s\\BackgroundPic[%s].jpg",cFilename, chTime);
        hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            return;
        }
        WriteFile(hFile, struFaceSnapAlarm.pBuffer2, struFaceSnapAlarm.dwBackgroundPicLen, &dwReturn, NULL);
        CloseHandle(hFile);
        hFile = NULL;
	}
	
	if (struFaceSnapAlarm.pBuffer1 != NULL)
	{
		delete []struFaceSnapAlarm.pBuffer1;
		struFaceSnapAlarm.pBuffer1 = NULL;
	}
	
	if (struFaceSnapAlarm.pBuffer2 != NULL)
	{
		delete []struFaceSnapAlarm.pBuffer2;
		struFaceSnapAlarm.pBuffer2 = NULL;
	}
}

void CClientDemoDlg::ProcFaceSnapMatchAlarm(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_VCA_FACESNAP_MATCH_ALARM struFaceSnapMatchAlarm = {0};
    memcpy(&struFaceSnapMatchAlarm, pAlarmInfo, sizeof(struFaceSnapMatchAlarm));
	
	//  ��ӡ��Ϣ
    NET_DVR_TIME  struAbsTime = {0};
    struAbsTime.dwYear = GET_YEAR(struFaceSnapMatchAlarm.struSnapInfo.dwAbsTime);
    struAbsTime.dwMonth = GET_MONTH(struFaceSnapMatchAlarm.struSnapInfo.dwAbsTime);
    struAbsTime.dwDay = GET_DAY(struFaceSnapMatchAlarm.struSnapInfo.dwAbsTime);
    struAbsTime.dwHour = GET_HOUR(struFaceSnapMatchAlarm.struSnapInfo.dwAbsTime);
    struAbsTime.dwMinute = GET_MINUTE(struFaceSnapMatchAlarm.struSnapInfo.dwAbsTime);
    struAbsTime.dwSecond = GET_SECOND(struFaceSnapMatchAlarm.struSnapInfo.dwAbsTime);
	
    sprintf(szInfoBuf, "Face Snap Match alarm Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d] FaceID[%d] BlackList ID[%d]\n",
        struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay, struAbsTime.dwHour,
        struAbsTime.dwMinute, struAbsTime.dwSecond, struFaceSnapMatchAlarm.struSnapInfo.dwSnapFacePicID, struFaceSnapMatchAlarm.struBlackListInfo.struBlackListInfo.dwRegisterID);
    AddLog(m_iCurDeviceIndex, ALARM_INFO_T, szInfoBuf);
	
    if (struFaceSnapMatchAlarm.struSnapInfo.dwSnapFacePicLen > 0 && struFaceSnapMatchAlarm.struSnapInfo.pBuffer1 != NULL)
    {
        int num = ulAlarmPic/1000+1;
        ulAlarmPic++;
        
        char cFilename[256] = {0};
        HANDLE hFile;
        DWORD dwReturn;
        
        SYSTEMTIME t;
        GetLocalTime(&t);
        char chTime[128];
        sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
        
        sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIP, num);
        if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
        {
            CreateDirectory(cFilename, NULL);
        }
        
        sprintf(cFilename, "%s\\Match SnapPic[%s].jpg",cFilename, chTime);
        hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            return;
        }
        WriteFile(hFile, struFaceSnapMatchAlarm.struSnapInfo.pBuffer1, struFaceSnapMatchAlarm.struSnapInfo.dwSnapFacePicLen, &dwReturn, NULL);
        CloseHandle(hFile);
        hFile = NULL;
	}
	
	if (struFaceSnapMatchAlarm.struBlackListInfo.dwBlackListPicLen > 0 && struFaceSnapMatchAlarm.struBlackListInfo.pBuffer1 != NULL)
    {
        int num = ulAlarmPic/1000+1;
        ulAlarmPic++;
        
        char cFilename[256] = {0};
        HANDLE hFile;
        DWORD dwReturn;
        
        SYSTEMTIME t;
        GetLocalTime(&t);
        char chTime[128];
        sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
        
        sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIP, num);
        if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
        {
            CreateDirectory(cFilename, NULL);
        }
        
        sprintf(cFilename, "%s\\Match BlackList Pic[%s].jpg",cFilename, chTime);
        hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            return;
        }
        WriteFile(hFile, struFaceSnapMatchAlarm.struBlackListInfo.pBuffer1, struFaceSnapMatchAlarm.struBlackListInfo.dwBlackListPicLen, &dwReturn, NULL);
        CloseHandle(hFile);
        hFile = NULL;
	}
	
	if (struFaceSnapMatchAlarm.struSnapInfo.pBuffer1 != NULL)
	{
		delete []struFaceSnapMatchAlarm.struSnapInfo.pBuffer1;
		struFaceSnapMatchAlarm.struSnapInfo.pBuffer1 = NULL;
	}
	
	if (struFaceSnapMatchAlarm.struBlackListInfo.pBuffer1 != NULL)
	{
		delete []struFaceSnapMatchAlarm.struBlackListInfo.pBuffer1;
		struFaceSnapMatchAlarm.struBlackListInfo.pBuffer1 = NULL;
	}
}

void CClientDemoDlg::ProcRuleInfoUpload(WPARAM wParam, LPARAM lParam)
{
    char szInfoBuf[1024] = {0};
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);
    int iDeviceIndex = pAlarmDev->iDeviceIndex;

    NET_DVR_RULE_INFO_ALARM struRuleInfo = {0};
    memcpy(&struRuleInfo, pAlarmInfo, sizeof(struRuleInfo));
    
    int i = struRuleInfo.struDevInfo.byChannel - g_struDeviceInfo[iDeviceIndex].iStartChan;
    g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
    ::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));

    NET_DVR_TIME  struAbsTime = {0};
    struAbsTime.dwYear = GET_YEAR(struRuleInfo.dwAbsTime);
    struAbsTime.dwMonth = GET_MONTH(struRuleInfo.dwAbsTime);
    struAbsTime.dwDay = GET_DAY(struRuleInfo.dwAbsTime);
    struAbsTime.dwHour = GET_HOUR(struRuleInfo.dwAbsTime);
    struAbsTime.dwMinute = GET_MINUTE(struRuleInfo.dwAbsTime);
    struAbsTime.dwSecond = GET_SECOND(struRuleInfo.dwAbsTime);

    sprintf(szInfoBuf, "Rule info Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d]  relative[%d] DevInfo ip[%d]port[%d]chan[%d]",
    struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay, struAbsTime.dwHour,
    struAbsTime.dwMinute, struAbsTime.dwSecond, struRuleInfo.dwRelativeTime, 
    struRuleInfo.struDevInfo.struDevIP.sIpV4, struRuleInfo.struDevInfo.wPort, struRuleInfo.struDevInfo.byChannel);

    AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);

    NET_DVR_EVENT_INFO struEventInfo = {0};
    for ( i = 0; i < struRuleInfo.struEventInfoList.byNum; i++)
    {
        memcpy(&struEventInfo, &struRuleInfo.struEventInfoList.struEventInfo[i], sizeof(struEventInfo));
        if (VCA_HIGH_DENSITY == struEventInfo.dwEventType)
        {
            sprintf(szInfoBuf, "High density RuleID[%d]Name[%s]density[%f]", struEventInfo.byRuleID, struEventInfo.byRuleName, 
                struEventInfo.uEventParam.fCrowdDensity);
        }
        else if (VCA_HUMAN_ENTER == struEventInfo.dwEventType)
        {
            sprintf(szInfoBuf, "Human enter RuleID[%d]Name[%s]Human enter[%d]",struEventInfo.byRuleID,
                struEventInfo.byRuleName, struEventInfo.uEventParam.dwHumanIn);
        }
        else
        {
            sprintf(szInfoBuf, "event info RuleID[%d]Name[%s]", struEventInfo.byRuleID, struEventInfo.byRuleName);
        }
        
        AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
    }

}

void CClientDemoDlg::ProcSensorAlarm(WPARAM wParam, LPARAM lParam)
{
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_SENSOR_ALARM struSensorAlarm = {0};
    memcpy(&struSensorAlarm, pAlarmInfo, sizeof(struSensorAlarm));
    
    memcpy(&m_struSensorAlarm[struSensorAlarm.bySensorChannel - 1], &struSensorAlarm, sizeof(struSensorAlarm));
    
    // memcpy(&m_struSensorAlarm, &struSensorAlarm, sizeof(m_struSensorAlarm));
    char szLan[1024] = {0};
    
    sprintf(szLan, "Sensor Alarm:AbsTime[%d]Name[%s]SensorChan[%d]Type[%d]AlramType[%d]AlarmMode[%u]Value[%f]",
        struSensorAlarm.dwAbsTime, struSensorAlarm.byName, struSensorAlarm.bySensorChannel,
        struSensorAlarm.byType, struSensorAlarm.byAlarmType, struSensorAlarm.byAlarmMode, struSensorAlarm.fValue);
    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szLan);
    if (g_pDlgSensor != NULL)
    {
        g_pDlgSensor->SetSensorDate(struSensorAlarm);
    }
    
}

void CClientDemoDlg::ProcCVRAlarm(WPARAM wParam, LPARAM lParam)
{
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_CVR_ALARM struCVRAlarm = {0};
    memcpy(&struCVRAlarm, pAlarmInfo, sizeof(struCVRAlarm));
    
    char szLan[1024] = {0};
    
    sprintf(szLan, "CVR ALARM Time[%04d-%02d-%02d %02d-%02d-%02d] AlarmLevel[%d] Status[%d] FirstType[%s] FirstIndex[%s] SecondType[%s] SecondIndex[%s]  \
		ThirdType[%s] ThirdIndex[%s] FourthType[%s] FourthIndex[%s]", 
        struCVRAlarm.struTime.dwYear, struCVRAlarm.struTime.dwMonth, struCVRAlarm.struTime.dwDay,
        struCVRAlarm.struTime.dwHour, struCVRAlarm.struTime.dwMinute, struCVRAlarm.struTime.dwSecond, 
		struCVRAlarm.byAlarmLevel, struCVRAlarm.byStatus,
		struCVRAlarm.szFirstType, struCVRAlarm.szFirstIndex, struCVRAlarm.szSecondType, struCVRAlarm.szSecondIndex,
		struCVRAlarm.szThirdType, struCVRAlarm.szThirdIndex, struCVRAlarm.szFourthType, struCVRAlarm.szFourthIndex);

    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szLan);

    
}

void CClientDemoDlg::ProcSensorValueUpload(WPARAM wParam, LPARAM lParam)
{
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_SENSOR_ALARM struSensorValue = {0};
    memcpy(&struSensorValue, pAlarmInfo, sizeof(struSensorValue));

    memcpy(&m_struSensorAlarm[struSensorValue.bySensorChannel - 1], &struSensorValue, sizeof(struSensorValue));
    // memcpy(&m_struSensorAlarm, &struSensorValue, sizeof(m_struSensorAlarm));
    char szLan[1024] = {0};
    sprintf(szLan, "ValueUpload:AbsTime[%d]Name[%s]SensorChan[%d]Type[%d]Value[%f]",
        struSensorValue.dwAbsTime, struSensorValue.byName, struSensorValue.bySensorChannel,
        struSensorValue.byType,  struSensorValue.fValue);
    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szLan);
    if (g_pDlgSensor != NULL)
    {
        g_pDlgSensor->SetSensorDate(struSensorValue);
    }
}

void CClientDemoDlg::ProcSwitchAlarm(WPARAM wParam, LPARAM lParam)
{
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_SWITCH_ALARM struSwitchAlarm = {0};
    memcpy(&struSwitchAlarm, pAlarmInfo, sizeof(struSwitchAlarm));

    char szLan[1024] = {0};
	char szName[33] = {0};
	memcpy(szName, struSwitchAlarm.byName, NAME_LEN);
    //����

    CString tmp;
    if (struSwitchAlarm.byAlarmType == 0)
    {
        tmp = "normal";
    }
    else if (struSwitchAlarm.byAlarmType == 1)
    {
        tmp = "short circuit";
    }
    else if (struSwitchAlarm.byAlarmType == 2)
    {
        tmp = "broken circuit";
    }
    else if (struSwitchAlarm.byAlarmType == 3)
    {
        tmp = "exception";
    }
    else
    {
        tmp = "unknown";
    }
    sprintf(szLan, "Switch alarm[%s], Name[%s]AlarmIn[%d]"\
        , tmp, szName, struSwitchAlarm.wSwitchChannel);

	
    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szLan);
}

void CClientDemoDlg::ProcAlarmHostException(WPARAM wParam, LPARAM lParam)
{
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
	
    NET_DVR_ALARMHOST_EXCEPTION_ALARM struAlarmHostException = {0};
    memcpy(&struAlarmHostException, pAlarmInfo, sizeof(struAlarmHostException));
    CString tmType;
    // �쳣����  1-�豸���𱨾� 2-�豸������ָ����� 3-����Դ���籨�� 4-����Դ�����ָ����� 5-�ڲ�ͨ�Ź��ϱ��� 
    // 6-�ڲ�ͨ�Ź��Ϻ�ָ�����  7-�绰�߶��� 8-�绰�߶��߻ָ� 9-�Լ�ʧ�ܱ���  10-�Լ�ʧ�ܺ�ָ�����    
    // 11����Ƿѹ  12���ص�ѹ�ָ�������13-���ع��ϣ�14-MBUSģ����ߣ�15-MBUSģ����߻ָ���16-���̵��ߣ�
    //17-���̵��߻ָ���18-�豸���ƶ���19-�豸���ƶ���λ  20-485������·�ϱ���  21-485������·�Ϻ�ָ�����
    if (struAlarmHostException.dwExceptionType == 1)
    {
        tmType = "�豸���𱨾�";
    }
	else if (struAlarmHostException.dwExceptionType == 2)
	{
		tmType = "�豸������ָ�����";
	}
	else if (struAlarmHostException.dwExceptionType == 3)
	{
		tmType = "����Դ���籨��";
	}
	else if (struAlarmHostException.dwExceptionType == 4)
	{
		tmType = "����Դ�����ָ�����";
	}
	else if (struAlarmHostException.dwExceptionType == 5)
	{
		tmType = "�ڲ�ͨ�Ź��ϱ���";
	}
	else if (struAlarmHostException.dwExceptionType == 6)
	{
		tmType = "�ڲ�ͨ�Ź��Ϻ�ָ�����";
	}
	else if (struAlarmHostException.dwExceptionType == 7)
	{
		tmType = "�绰�߶���";
	}	
	else if (struAlarmHostException.dwExceptionType == 8)
	{
		tmType = "�绰�߶��߻ָ�";
	}
	else if (struAlarmHostException.dwExceptionType == 9)
	{
		tmType = "�Լ�ʧ�ܱ���";
	}
	else if (struAlarmHostException.dwExceptionType == 10)
	{
		tmType = "�Լ�ʧ�ܺ�ָ�����";
	}	
	else if (struAlarmHostException.dwExceptionType == 11)
	{
		tmType = "����Ƿѹ";
	}
	else if (struAlarmHostException.dwExceptionType == 12)
	{
		tmType = "���ص�ѹ�ָ�����";
	}
	else if (struAlarmHostException.dwExceptionType == 13)
	{
		tmType = "���ع���";
	}	
	else if (struAlarmHostException.dwExceptionType == 14)
	{
		tmType = "MBUSģ�����";
	}
	else if (struAlarmHostException.dwExceptionType == 15)
	{
		tmType = "MBUSģ����߻ָ�";
	}
	else if (struAlarmHostException.dwExceptionType == 16)
	{
		tmType = "���̵���";
	}	
	else if (struAlarmHostException.dwExceptionType == 17)
	{
		tmType = "���̵��߻ָ�";
	}
	else if (struAlarmHostException.dwExceptionType == 18)
	{
		tmType = "�豸���ƶ�";
	}
	else if (struAlarmHostException.dwExceptionType == 19)
	{
		tmType = "�豸���ƶ���λ";
	}
	else if (struAlarmHostException.dwExceptionType == 20)
	{
		tmType = "485������·�ϱ���";
	}
	else if (struAlarmHostException.dwExceptionType == 21)
	{
		tmType = "485������·�Ϻ�ָ�����";
	}
	else if (struAlarmHostException.dwExceptionType == 22)
	{
		tmType = "Ӳ����";
	}
	else if (struAlarmHostException.dwExceptionType == 23)
	{
		tmType = "Ӳ��δ��ʽ��";
	}
	else if (struAlarmHostException.dwExceptionType == 24)
	{
		tmType = "Ӳ�̶�д����";
	}

    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, tmType);
   
}

void CClientDemoDlg::ProcDeviceAlarm(WPARAM wParam, LPARAM lParam)
{
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	if (COMM_ALARM_DEVICE == pAlarmDev->lCommand)
	{
		char *pAlarmInfo = (char *)(lParam);		
		int iDeviceIndex = pAlarmDev->iDeviceIndex;

		NET_DVR_ALARMINFO_DEV struDeviceAlarm = {0};
		memcpy(&struDeviceAlarm, pAlarmInfo, sizeof(struDeviceAlarm));
    
		char szLan[1024] = {0};

		sprintf(szLan, "Device Alarm(CVR): type[%d], num[%d]", struDeviceAlarm.dwAlarmType, struDeviceAlarm.dwNumber);

		int iNumber = struDeviceAlarm.dwNumber;
		WORD *pContent = struDeviceAlarm.pNO; 
		for (int i = 0; i < iNumber && i < 10; i++)
		{
			sprintf(szLan, "%s %d", szLan, *(pContent));
			pContent = pContent + 1;
		}

		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szLan);
	}
}
void CClientDemoDlg::ProcAlarmHostSatetyCabinState(WPARAM wParam, LPARAM lParam)
{
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_ALARMHOST_SAFETYCABINSTATE struAlarm = {0};
    memcpy(&struAlarm, pAlarmInfo, sizeof(struAlarm));

	char szLan[1024] = {0};
	char szLan1[64] = {0};
	char szLan2[64] = {0};
	char szLan3[64] = {0};
	char szLan4[64] = {0};
	char szLan5[64] = {0};
	char szLan6[64] = {0};
	char szLan7[64] = {0};
	char szLan8[64] = {0};
	char szLan9[64] = {0};

	if (1 == struAlarm.byManState)
	{
		g_StringLanType(szLan1, "��������; ", "someone in the cabin;  ");
	}
	else
	{
		g_StringLanType(szLan1, "����û����; ", "no one in the cabin;  ");
	}

	if (1 == struAlarm.byEnterButton)
	{
		g_StringLanType(szLan2, "���Ű�ť: ����; ", "EnterButton: on;  ");
	}
	else
	{
		g_StringLanType(szLan2, "���Ű�ť: ������; ", "EnterButton: off;  ");
	}
	
	if (1 == struAlarm.byExitButton)
	{
		g_StringLanType(szLan3, "���Ű�ť: ����; ", "ExitButton: on;  ");
	}
	else
	{
		g_StringLanType(szLan3, "���Ű�ť: ������; ", "ExitButton: off;  ");
	}

	if (1 == struAlarm.byUrgencyButton)
	{
		g_StringLanType(szLan4, "������ť: ����; ", "UrgencyButton: off;  ");
	}
	else
	{
		g_StringLanType(szLan4, "������ť: ������; ", "UrgencyButton: off;  ");
	}

	if (0 == struAlarm.byDoorState)
	{
		g_StringLanType(szLan5, "�Ź�; ", "Door: close;  ");
	}
	else
	{
		g_StringLanType(szLan5, "�ſ�; ", "Door: open;  ");
	}

	if (0 == struAlarm.byLockState)
	{
		g_StringLanType(szLan6, "����; ", "locker: close;  ");
	}
	else
	{
		g_StringLanType(szLan6, "����; ", "locker: open;  ");
	}

	if (0 == struAlarm.byAbnormal)
	{
		g_StringLanType(szLan7, "����;", "Normal");
	}
	else
	{
		g_StringLanType(szLan7, "�쳣;", "Abnormal");
	}
	
	if (0 == struAlarm.byFanState)
	{
		g_StringLanType(szLan8, "���ȹ�; ", "Fan: off;  ");
	}
	else
	{
		g_StringLanType(szLan8, "���ȿ�; ", "Fan: on;  ");
	}
	
	if (0 == struAlarm.byLightState)
	{
		g_StringLanType(szLan9, "�ƹ��; ", "Light: off;  ");
	}
	else
	{
		g_StringLanType(szLan9, "�ƹ⿪; ", "Light: on;  ");
	}

    sprintf(szLan, "%s  %s  %s  %s  %s  %s  %s  %s  %s", szLan1, szLan2, szLan3, szLan4, szLan5, szLan6, szLan7,szLan8,szLan9);
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szLan);
}

void CClientDemoDlg::ProcAlarmHostAlarmoutStatus(WPARAM wParam, LPARAM lParam)
{
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_ALARMHOST_ALARMOUTSTATUS struAlarm = {0};
    memcpy(&struAlarm, pAlarmInfo, sizeof(struAlarm));
	char szLan[1024] = {0};
	char szLanCn[1024] = {0};
	char szLanEn[1024] = {0};
	switch(struAlarm.byAlarmType)
	{
	case 1: //���������״̬
		if(struAlarm.byAlarmStatus == 1)
		{
			sprintf(szLanCn, "��������� %d(%s) ����", struAlarm.wChan, struAlarm.byName);
			sprintf(szLanEn, "Get Alarm from AlarmOut channel %d(%s)", struAlarm.wChan, struAlarm.byName);
			g_StringLanType(szLan, szLanCn, szLanEn);
		}
		else
		{
			sprintf(szLanCn, "��������� %d(%s) ����ȡ��", struAlarm.wChan, struAlarm.byName);
			sprintf(szLanEn, "Alarm from AlarmOut channel %d(%s) clean", struAlarm.wChan, struAlarm.byName);
			g_StringLanType(szLan, szLanCn, szLanEn);
		}
	break;
	case 2: //����״̬
		if(struAlarm.byAlarmStatus == 1)
		{
			sprintf(szLanCn, "���� %d(%s) �������", struAlarm.wChan, struAlarm.byName);
			sprintf(szLanEn, "Get Alarm from siren  %d(%s)", struAlarm.wChan, struAlarm.byName);
			g_StringLanType(szLan, szLanCn, szLanEn);
		}
		else
		{
			sprintf(szLanCn, "���� %d(%s) ���Źر�", struAlarm.wChan, struAlarm.byName);
			sprintf(szLanEn, "Alarm from siren  %d(%s) clean", struAlarm.wChan, struAlarm.byName);
			g_StringLanType(szLan, szLanCn, szLanEn);
		}
	break;
	default:
	break;
	}
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szLan);
}

void CClientDemoDlg::ProcAlarmHostCIDAlarm(WPARAM wParam, LPARAM lParam)
{
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_CID_ALARM struAlarm = {0};
    memcpy(&struAlarm, pAlarmInfo, sizeof(struAlarm));

	char szLan[1024] = {0};
	char szLanCn[1024] = {0};
	char szLanEn[1024] = {0};

	switch (struAlarm.byReportType)
	{
	case NET_DVR_DEFENCE_ALARM:
		sprintf(szLanCn, "��������: ");
		sprintf(szLanEn, "Zone Alarm: ");
		break;
	case NET_DVR_VIDEO_ALARM:
		sprintf(szLanCn, "��Ƶ������");
		sprintf(szLanEn, "Video Alarm��");
		break;
	case NET_DVR_VIRTUAL_DEFENCE_ALARM:
		 sprintf(szLanCn, "�����������");
		 sprintf(szLanEn, "Soft Zone Alarm��");
		break;
	case NET_DVR_HOSTAGE_ALARM:
		sprintf(szLanCn, "Ю�ֱ���: ");
		sprintf(szLanEn, "Hostage Alarm��");
		break;
	case NET_DVR_KNOCK_DOWN_ALARM:
		sprintf(szLanCn, "���𱨾�: ");
		sprintf(szLanEn, "Tamper Alarm: ");
		break;
	case NET_DVR_OPERATE_ALARM:
		sprintf(szLanCn, "������");
		sprintf(szLanEn, "Operate��");
		break;
	case NET_DVR_OHTER_ABNORMAL_ALARM:
		sprintf(szLanCn, "�쳣��");
		sprintf(szLanEn, "Abnormal��");
		break;

	default:
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, "Unknown Alarm");
		return;
	}
	char sTemp1[5] = {0};
	char sTemp2[33] = {0};
	memcpy(sTemp1, struAlarm.sCIDCode, 4);
	memcpy(sTemp2, struAlarm.sCIDDescribe, NAME_LEN);
	sprintf(szLanCn, "%sCID�¼��ţ�%s, CID�¼�����%s", szLanCn, sTemp1, sTemp2);
	sprintf(szLanEn, "%sCID��%s, CID describe��%s", szLanEn, sTemp1, sTemp2);
	//�����˺�
	if (strcmp((char*)struAlarm.sCenterAccount, ""))
	{
		memset(sTemp2, 0, sizeof(sTemp2));
		memcpy(sTemp2, struAlarm.sCenterAccount, ACCOUNTNUM_LEN);
		sprintf(szLanCn, "%s, �����˺ţ�%s", szLanCn, sTemp2);
		sprintf(szLanEn, "%s, Center account��%s", szLanEn, sTemp2);
	}
	//�û���
	if (0xff != struAlarm.byUserType)
	{
		if (0 == struAlarm.byUserType)//�����û�
		{
			memset(sTemp2, 0, sizeof(sTemp2));
			memcpy(sTemp2, struAlarm.sUserName, NAME_LEN);
			sprintf(szLanCn, "%s�������û�����%s", szLanCn, struAlarm.sUserName);
			sprintf(szLanEn, "%s��Net User Name��%s", szLanEn, struAlarm.sUserName);
		}
		else if(1 == struAlarm.byUserType)//�����û�
		{
			sprintf(szLanCn, "%s, �����û��ţ�%d", szLanCn, struAlarm.wKeyUserNo);
			sprintf(szLanEn, "%s, Keypad UserID��%d", szLanEn, struAlarm.wKeyUserNo);
		}
		else
		{
			sprintf(szLanCn, "%s, δ֪�û�����", szLanCn);
			sprintf(szLanEn, "%s, Unknown User Type", szLanEn);
		}
	}
	
	//���̺�
	if (0xff != struAlarm.byKeypadNo)
	{
		sprintf(szLanCn, "%s, ���̺ţ�%d", szLanCn, struAlarm.byKeypadNo);
		sprintf(szLanEn, "%s, KeyPad No��%d", szLanEn, struAlarm.byKeypadNo);
	}
	//��ϵͳ��
	if (0xff != struAlarm.bySubSysNo)
	{
		sprintf(szLanCn, "%s, ��ϵͳ�ţ�%d", szLanCn, struAlarm.bySubSysNo);
		sprintf(szLanEn, "%s, Subsystem No��%d", szLanEn, struAlarm.bySubSysNo);
	}
	//������
	if (0xffff != struAlarm.wDefenceNo)
	{
		sprintf(szLanCn, "%s, �����ţ�%d", szLanCn, struAlarm.wDefenceNo);
		sprintf(szLanEn, "%s, Zone No��%d", szLanEn, struAlarm.wDefenceNo);
	}
	//��Ƶͨ����
	if (0xff != struAlarm.byVideoChanNo)
	{
		sprintf(szLanCn, "%s, ��Ƶͨ���ţ�%d", szLanCn, struAlarm.byVideoChanNo);
		sprintf(szLanEn, "%s, Viedo Chan No��%d", szLanEn, struAlarm.byVideoChanNo);
	}
	//Ӳ�̺�
	if (0xff != struAlarm.byDiskNo)
	{
		sprintf(szLanCn, "%s, Ӳ�̺ţ�%d", szLanCn, struAlarm.byDiskNo);
		sprintf(szLanEn, "%s, HD No��%d", szLanEn, struAlarm.byDiskNo);
	}
	//ģ���ַ��
	if (0xffff != struAlarm.wModuleAddr)
	{
		sprintf(szLanCn, "%s, ģ���ַ��%d", szLanCn, struAlarm.wModuleAddr);
		sprintf(szLanEn, "%s, Module Address��%d", szLanEn, struAlarm.wModuleAddr);
	}
	g_StringLanType(szLan, szLanCn, szLanEn);
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szLan);
	
	memset(szLanCn, 0, sizeof(szLanCn));
	memset(szLanEn, 0, sizeof(szLanEn));
	memset(szLan, 0, sizeof(szLan));
	sprintf(szLanCn, "��������ʱ�䣺%04d-%02d-%02d %02d:%02d:%02d, �ϴ�����ʱ�䣺%04d-%02d-%02d %02d:%02d:%02d", struAlarm.struTriggerTime.wYear, struAlarm.struTriggerTime.byMonth, struAlarm.struTriggerTime.byDay, struAlarm.struTriggerTime.byHour, \
		struAlarm.struTriggerTime.byMinute, struAlarm.struTriggerTime.bySecond, struAlarm.struUploadTime.wYear, struAlarm.struUploadTime.byMonth, struAlarm.struUploadTime.byDay, struAlarm.struUploadTime.byHour\
		, struAlarm.struUploadTime.byMinute, struAlarm.struUploadTime.bySecond);
	sprintf(szLanEn, "Trigger Time��%d-%d-%d %d:%d:%d, Upload Time��%d-%d-%d %d:%d:%d", struAlarm.struTriggerTime.wYear, struAlarm.struTriggerTime.byMonth, struAlarm.struTriggerTime.byDay, struAlarm.struTriggerTime.byHour, \
		struAlarm.struTriggerTime.byMinute, struAlarm.struTriggerTime.bySecond, struAlarm.struUploadTime.wYear, struAlarm.struUploadTime.byMonth, struAlarm.struUploadTime.byDay, struAlarm.struUploadTime.byHour\
		, struAlarm.struUploadTime.byMinute, struAlarm.struUploadTime.bySecond);	
	g_StringLanType(szLan, szLanCn, szLanEn);
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szLan);	
}

void CClientDemoDlg::ProcessAlarmhostUPSAlarm(char* szLanCn, char* szLanEn, LPNET_DVR_ALARMHOST_UPS_ALARMINFO lpAlarm)
{
	//Ĭ�ϳ���10
	DWORD dwDivisionCoefficient = 10;
	switch (lpAlarm->byAlarmType)
	{
	case 1:
		sprintf(szLanCn, "%s", "A�������ѹԽ��");
		break;
	case 2:
		sprintf(szLanCn, "%s", "B�������ѹԽ��");
		break;
	case 3:
		sprintf(szLanCn, "%s", "C�������ѹԽ��");
		break;
	case 4:
		sprintf(szLanCn, "%s", "A�������ѹԽ��5");
		break;
	case 5:
		sprintf(szLanCn, "%s", "B�������ѹԽ��");
		break;
	case 6:
		sprintf(szLanCn, "%s", "C�������ѹԽ��");
		break;		
	case 7:
		sprintf(szLanCn, "%s", "A������Ƶ��Խ��");
		break;
	case 8:
		sprintf(szLanCn, "%s", "B������Ƶ��Խ��");
		break;
	case 9:
		sprintf(szLanCn, "%s", "C������Ƶ��Խ��");
		break;
	case 10:
		sprintf(szLanCn, "%s", "��������ѹԽ�ޱ���");
		break;
	case 11:
		sprintf(szLanCn, "%s", "���ر���");
		break;
	case 12:
		sprintf(szLanCn, "%s", "��ص�ѹ�ͱ���");
		break;
	case 13:
		sprintf(szLanCn, "%s", "��غ�ʱ�䳬�ͱ���");
		dwDivisionCoefficient = 1;
		break;
	case 14:
		sprintf(szLanCn, "%s", "����¶ȳ��߱���");
		break;
	case 15:
		sprintf(szLanCn, "%s", "������رձ���");
		break;
	case 16:
		sprintf(szLanCn, "%s", "�Զ���·������");
		break;		
	case 17:
		sprintf(szLanCn, "%s", "���������ϱ���");
		break;
	case 18:
		sprintf(szLanCn, "%s", "��������ϱ���");
		break;
	case 19:
		sprintf(szLanCn, "%s", "��������ϱ���");
		break;
	case 20:
		sprintf(szLanCn, "%s", "��ع��ϱ���");
		break;
	case 21:
		sprintf(szLanCn, "%s", "�Զ���·���ϱ���");
		break;
	default:
		sprintf(szLanCn, "%s", "δ֪��������");
		break;
	}
	if (0xffffffff == lpAlarm->iAlarmValue)
	{
		sprintf(szLanCn, "%s, 485ͨ����%d, ��λ�ţ�%d", szLanCn, lpAlarm->byChanNo, lpAlarm->bySlotNo);
	}
	else
	{
		if (1 == dwDivisionCoefficient)
		{
			sprintf(szLanCn, "%s, 485ͨ����%d, ��λ�ţ�%d, ����ֵ��%d", szLanCn, lpAlarm->byChanNo, lpAlarm->bySlotNo, lpAlarm->iAlarmValue);
		}
		else	
		{
			sprintf(szLanCn, "%s, 485ͨ����%d, ��λ�ţ�%d, ����ֵ��%0.1f", szLanCn, lpAlarm->byChanNo, lpAlarm->bySlotNo, (float)lpAlarm->iAlarmValue/dwDivisionCoefficient);
		}
	}
}

void CClientDemoDlg::ProcessAlarmhostSwitchPowerAlarm(char* szLanCn, char* szLanEn, LPNET_DVR_ALARMHOST_SWITCH_POWER_SUPPLY_ALARMINFO lpAlarm)
{
	//Ĭ�ϳ���10
	DWORD dwDivisionCoefficient = 10;

		/*�������ͣ�1-��ز���ʧ�� 2-AC-DCת�������ϣ�3-������/���ѹAB/AԽ�ޣ�4-������/���ѹBC/BԽ�ޣ�5-������/���ѹCA/CԽ�ޣ�
	6-�������AԽ�ޣ�7-�������BԽ�ޣ�8-�������CԽ�ޣ�9����ģ����ϣ�10-���ȹ��ϣ�11-ֱ����ѹԽ��*/
	switch (lpAlarm->byAlarmType)
	{
	case 1:
		sprintf(szLanCn, "%s", "��ز���ʧ��");
		break;
	case 2:
		sprintf(szLanCn, "%s", "AC-DCת��������");
		break;
	case 3:
		sprintf(szLanCn, "%s", "������/���ѹAB/AԽ��");
		break;
	case 4:
		sprintf(szLanCn, "%s", "������/���ѹBC/BԽ��");
		break;
	case 5:
		sprintf(szLanCn, "%s", "������/���ѹCA/CԽ��");
		break;
	case 6:
		sprintf(szLanCn, "%s", "A���������Խ��");
		break;		
	case 7:
		sprintf(szLanCn, "%s", "B���������Խ��");
		break;
	case 8:
		sprintf(szLanCn, "%s", "C���������Խ��");
		break;
	case 9:
		sprintf(szLanCn, "%s", "����ģ�����");
		break;
	case 10:
		sprintf(szLanCn, "%s", "���ȹ���");
		break;
	case 11:
		sprintf(szLanCn, "%s", "ֱ����ѹԽ��");
		break;
	default:
		sprintf(szLanCn, "%s", "δ֪��������");
		break;
	}
	if (0xffffffff == lpAlarm->iAlarmValue)
	{
		sprintf(szLanCn, "%s, 485ͨ����%d, ��λ�ţ�%d", szLanCn, lpAlarm->byChanNo, lpAlarm->bySlotNo);
	}
	else
	{
		if (1 == dwDivisionCoefficient)
		{
			sprintf(szLanCn, "%s, 485ͨ����%d, ��λ�ţ�%d, ����ֵ��%d", szLanCn, lpAlarm->byChanNo, lpAlarm->bySlotNo, lpAlarm->iAlarmValue);
		}
		else	
		{
			sprintf(szLanCn, "%s, 485ͨ����%d, ��λ�ţ�%d, ����ֵ��%0.1f", szLanCn, lpAlarm->byChanNo, lpAlarm->bySlotNo, (float)lpAlarm->iAlarmValue/dwDivisionCoefficient);
		}
	}
}


void CClientDemoDlg::ProcessAlarmhostGasDetectAlarm(char* szLanCn, char* szLanEn, LPNET_DVR_ALARMHOST_GAS_DETECT_SYS_ALARMINFO lpAlarm)
{
	//Ĭ�ϳ���10
	DWORD dwDivisionCoefficient = 10;
	/*�������ͣ�1- SF6���庬���쳣��2-���������쳣��3-�¶�ֵ�쳣��4-ʪ��ֵ�쳣��5-����*/
	switch (lpAlarm->byAlarmType)
	{
	case 1:
		sprintf(szLanCn, "%s", "SF6���庬���쳣");
		dwDivisionCoefficient = 1;
		break;
	case 2:
		sprintf(szLanCn, "%s", "���������쳣");
		break;
	case 3:
		sprintf(szLanCn, "%s", "�¶�ֵ�쳣");
		break;
	case 4:
		sprintf(szLanCn, "%s", "ʪ��ֵ�쳣");
		break;
	case 5:
		sprintf(szLanCn, "%s", "����");
		dwDivisionCoefficient = 1;
		break;
	default:
		sprintf(szLanCn, "%s", "δ֪��������");
		break;
	}
	if (0xffffffff == lpAlarm->iAlarmValue)
	{
		sprintf(szLanCn, "%s, 485ͨ����%d, ��λ�ţ�%d", szLanCn, lpAlarm->byChanNo, lpAlarm->bySlotNo);
	}
	else
	{
		if (1 == dwDivisionCoefficient)
		{
			sprintf(szLanCn, "%s, 485ͨ����%d, ��λ�ţ�%d, ����ֵ��%d", szLanCn, lpAlarm->byChanNo, lpAlarm->bySlotNo, lpAlarm->iAlarmValue);
		}
		else	
		{
			sprintf(szLanCn, "%s, 485ͨ����%d, ��λ�ţ�%d, ����ֵ��%0.1f", szLanCn, lpAlarm->byChanNo, lpAlarm->bySlotNo, (float)lpAlarm->iAlarmValue/dwDivisionCoefficient);
		}
	}
}

void CClientDemoDlg::ProcessAlarmhostTempHumiSensorAlarm(char* szLanCn, char* szLanEn, LPNET_DVR_ALARMHOST_TEMP_HUMIDITY_SENSOR_ALARMINFO lpAlarm)
{
	//Ĭ�ϳ���10
	DWORD dwDivisionCoefficient = 10;
	//1-�¶��쳣��2-ʪ���쳣
	switch (lpAlarm->byAlarmType)
	{
	case 1:
		sprintf(szLanCn, "%s", "�¶��쳣");
		break;
	case 2:
		sprintf(szLanCn, "%s", "ʪ���쳣");
		break;
	default:
		sprintf(szLanCn, "%s", "δ֪��������");
		break;
	}
	if (0xffffffff == lpAlarm->iAlarmValue)
	{
		sprintf(szLanCn, "%s, 485ͨ����%d, ��λ�ţ�%d", szLanCn, lpAlarm->byChanNo, lpAlarm->bySlotNo);
	}
	else
	{
		if (1 == dwDivisionCoefficient)
		{
			sprintf(szLanCn, "%s, 485ͨ����%d, ��λ�ţ�%d, ����ֵ��%d", szLanCn, lpAlarm->byChanNo, lpAlarm->bySlotNo, lpAlarm->iAlarmValue);
		}
		else	
		{
			sprintf(szLanCn, "%s, 485ͨ����%d, ��λ�ţ�%d, ����ֵ��%0.1f", szLanCn, lpAlarm->byChanNo, lpAlarm->bySlotNo, (float)lpAlarm->iAlarmValue/dwDivisionCoefficient);
		}
	}
}

void CClientDemoDlg::ProcAlarmHostExternalDevAlarm(WPARAM wParam, LPARAM lParam)
{
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_485_EXTERNAL_DEVICE_ALARMINFO struAlarm = {0};
    memcpy(&struAlarm, pAlarmInfo, sizeof(struAlarm));

	char szLan[1024] = {0};
	char szLanCn[1024] = {0};
	char szLanEn[1024] = {0};

	switch (struAlarm.byAlarmType)
	{
	case 1:
		ProcessAlarmhostUPSAlarm(szLanCn, szLanEn, &struAlarm.struAlarmInfo.struUPSAlarm);
		break;
	case 2:
		ProcessAlarmhostSwitchPowerAlarm(szLanCn, szLanEn, &struAlarm.struAlarmInfo.struSwitchPowerAlarm);
		break;
	case 3:
		ProcessAlarmhostGasDetectAlarm(szLanCn, szLanEn, &struAlarm.struAlarmInfo.struGasDetectSystemAlarm);
		break;
	case 4:
		ProcessAlarmhostTempHumiSensorAlarm(szLanCn, szLanEn, &struAlarm.struAlarmInfo.struTempHumiditySensorAlarm);
		break;
	default:
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, "Unknown Alarm");
		return;
	}

	g_StringLanType(szLan, szLanCn, szLanEn);
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szLan);
}

void CClientDemoDlg::ProcessAlarmhostPointValue(char* szLanCn, char* szLanEn, LPNET_DVR_ALARMHOST_POINT_VALUE lpAlarm)
{
	//Ĭ�ϳ���10
	DWORD dwDivisionCoefficient = 1;
	if (1 == lpAlarm->byPointType)
	{
		dwDivisionCoefficient = 1000;
	}
	
	//1-����ģ����ͨ����2-���ؿ�����ͨ����3-485ͨ����4-����ͨ��
	switch (lpAlarm->byChanType)
	{
	case 1:
		sprintf(szLanCn, "����ģ����ͨ��%u,������:%u,���%u ����ֵ:%0.3f", lpAlarm->dwChanNo, lpAlarm->dwVariableNo, lpAlarm->dwPointNo, (float)((float)(lpAlarm->iValue)/dwDivisionCoefficient));
		break;
	case 2:
		sprintf(szLanCn, "���ؿ�����ͨ��%u,������:%u,���%u ����ֵ:%d", lpAlarm->dwChanNo, lpAlarm->dwVariableNo, lpAlarm->dwPointNo, lpAlarm->iValue);
		break;
	case 3:
		sprintf(szLanCn, "485ͨ��%u��λ%u,������:%u,���%u ����ֵ:%0.3f", lpAlarm->dwChanNo, lpAlarm->dwSubChanNo, lpAlarm->dwVariableNo, lpAlarm->dwPointNo, (float)((float)(lpAlarm->iValue)/dwDivisionCoefficient));
		break;
	case 4:
		sprintf(szLanCn, "����ͨ��%u,������:%u,���%u ����ֵ:%d", lpAlarm->dwChanNo, lpAlarm->dwVariableNo, lpAlarm->dwPointNo, (float)((float)(lpAlarm->iValue)/dwDivisionCoefficient));
		break;
	default:
		sprintf(szLanCn, "%s", "δ֪ͨ������");
		break;
	}
}

void CClientDemoDlg::ProcAlarmHostData(WPARAM wParam, LPARAM lParam)
{
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_ALARMHOST_DATA_UPLOAD struAlarm = {0};
    memcpy(&struAlarm, pAlarmInfo, sizeof(struAlarm));
	
	char szLan[1024] = {0};
	char szLanCn[1024] = {0};
	char szLanEn[1024] = {0};
	
	switch (struAlarm.byDataType)
	{
	case 1: //���������ϴ�
		ProcessAlarmhostPointValue(szLanCn, szLanEn, &struAlarm.struAlarmData.struPointValue);
		break;
	default:
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, "Unknown Alarm");
		return;
	}
	
	g_StringLanType(szLan, szLanCn, szLanEn);
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szLan);
}

void CClientDemoDlg::ProcGateAlarmHostData(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_GATE_ALARMINFO struAlarm = {0};
    memcpy(&struAlarm, pAlarmInfo, sizeof(struAlarm));	
	
	sprintf(szInfoBuf, "AlarmType:[%d] AlarmTime[%d:%d:%d %d:%d:%d]\n",\
		struAlarm.byAlarmType, struAlarm.struAlarmTime.wYear, struAlarm.struAlarmTime.byMonth, struAlarm.struAlarmTime.byDay, \
		struAlarm.struAlarmTime.byHour, struAlarm.struAlarmTime.byMinute, struAlarm.struAlarmTime.bySecond);
	
	if (struAlarm.byAlarmType == 0x01)
	{
		sprintf(szInfoBuf, "%s License[%s] VehicleType[%d]", szInfoBuf, struAlarm.uAlarmInfo.struVehicleInfo.sLicense, \
			struAlarm.uAlarmInfo.struVehicleInfo.byVehicleType);
	}
	
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
}

void CClientDemoDlg::ProcExternalControlAlarmHostData(WPARAM wParam, LPARAM lParam)//��������(��ر���)2013-06-09
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_EXTERNAL_CONTROL_ALARM struAlarm = {0};
    memcpy(&struAlarm, pAlarmInfo, sizeof(struAlarm));
	
	sprintf(szInfoBuf,"ChannelNo:[%d] Flicker:[%d] IONo:[%d] ParkingIndex:[%d] ExternalBeginTime:[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d] \n",
		struAlarm.dwChannelNo, struAlarm.struLampStateCtrl.byFlicker, struAlarm.struLampStateCtrl.dwIONo, struAlarm.struLampStateCtrl.byParkingIndex
		,struAlarm.struExternalBeginTime.dwYear, struAlarm.struExternalBeginTime.dwMonth, struAlarm.struExternalBeginTime.dwDay,
		struAlarm.struExternalBeginTime.dwHour, struAlarm.struExternalBeginTime.dwMinute,struAlarm.struExternalBeginTime.dwSecond);
	
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
	
}

void CClientDemoDlg::ProcSceneChangeDetection(WPARAM wParam, LPARAM lParam)//2013-07-17���������ⱨ��
{
	char szInfoBuf[1024] = {0};
	char szLan[128] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;	
	NET_DVR_SCENECHANGE_DETECTION_RESULT struSceneChange = {0};
	memcpy(&struSceneChange, pAlarmInfo, sizeof(struSceneChange));
	
	g_StringLanType(szLan, "���������ⱨ��","Scene Change Detection");

	sprintf(szInfoBuf, "%s: (Device IP:[%s]  Port:[%d]  Device Channel:[%d]  Ivms Channel:[%d])\n", szLan,
		struSceneChange.struDevInfo.struDevIP.sIpV4, struSceneChange.struDevInfo.wPort, 
		struSceneChange.struDevInfo.byChannel, struSceneChange.struDevInfo.byIvmsChannel);
	AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
}

void CClientDemoDlg::ProcHotSpare(WPARAM wParam, LPARAM lParam)//2013-07-17�ȱ��쳣����
{
	char szInfoBuf[1024] = {0};
	char szLan[128] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;	
	NET_DVR_ALARM_HOT_SPARE struAlarmHotSpare = {0};
	memcpy(&struAlarmHotSpare, pAlarmInfo, sizeof(struAlarmHotSpare));
	
	g_StringLanType(szLan, "�ȱ��쳣����","Scene Change Detection");
	switch (struAlarmHotSpare.dwExceptionCase)
	{
	case 0:
		sprintf(szInfoBuf, "%s: (�����쳣 Device IP:[%s])\n", szLan,
			struAlarmHotSpare.struDeviceIP.sIpV4);
	}
	
	AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
}

void CClientDemoDlg::ProcVehCostItem(WPARAM wParam, LPARAM lParam)//2013-11-19 ����ڹ����շ���ϸ
{
	char szInfoBuf1[1024] = {0};
	char szInfoBuf2[1024] = {0};
	char szInfoBuf3[1024] = {0};
	char szInfoBuf4[1024] = {0};
	char szLan[128] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;	
	NET_ITS_PASSVEHICLE_COST_ITEM struVehCostItem = {0};
	memset(&struVehCostItem, 0, sizeof(NET_ITS_PASSVEHICLE_COST_ITEM));
	memcpy(&struVehCostItem, pAlarmInfo, sizeof(NET_ITS_PASSVEHICLE_COST_ITEM));
	
	g_StringLanType(szLan, "����ڹ����շ���ϸ","Vehicle Cost Item");
	sprintf(szInfoBuf4,"%s End: \n", szLan);
	AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf4);
	
	sprintf(szInfoBuf3, "Operator Name:[%s]; Vehicle Type:[%d]; Pay Rule ID:[%d]; Free Rule ID:[%d]\n",struVehCostItem.byOperatorName,\
        struVehCostItem.byVehicleType, struVehCostItem.dwPayRuleID, struVehCostItem.dwFreeRuleID);
	AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf3);
	
	sprintf(szInfoBuf2, "Vehicle ID:[%d]; In Time:[%s]; Out Time:[%s]; Card No:[%s]; Plate No:[%s]; Pay Cost:[%0.2f]\n", struVehCostItem.dwPassVehicleID,\
		struVehCostItem.byIntime, struVehCostItem.byOuttime, struVehCostItem.byCardNo, struVehCostItem.byPlateInfo, struVehCostItem.fPayCost);
	AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf2);
	
	sprintf(szInfoBuf1,"%s Begin: \n", szLan);
	AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf1);
}

void CClientDemoDlg::ProcVehHandOver(WPARAM wParam, LPARAM lParam)//2013-11-19 ����ڽ��Ӱ�����
{
	char szInfoBuf[1024] = {0};
	char szLan[128] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;	
	NET_ITS_HANDOVER_INFO struHandOverInfo = {0};
	memset(&struHandOverInfo, 0, sizeof(NET_ITS_HANDOVER_INFO));
	memcpy(&struHandOverInfo, pAlarmInfo, sizeof(NET_ITS_HANDOVER_INFO));
	
	g_StringLanType(szLan, "����ڽ��Ӱ�����","Vehicle Handover Info");
	
	sprintf(szInfoBuf, "%s(Operator Name:[%s]; Operator Card:[%s]; Start Time:[%s]; End Time:[%s]; Total Pay:[%0.2f]; Total Records:[%d])\n", szLan,\
		struHandOverInfo.byOperatorName, struHandOverInfo.byOperatorCard, struHandOverInfo.byStartTime, struHandOverInfo.byEndTime,\
		struHandOverInfo.fTotal_Pay, struHandOverInfo.dwTotal_Records);
	AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
	
}


void CClientDemoDlg::ProcAudioException(WPARAM wParam, LPARAM lParam)//�����쳣�����ϴ� 2014-03-21
{
	char szInfoBuf[1024] = {0};
	char szLan[128] = {0};
	char szAlarmType[128] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;	
	NET_DVR_AUDIOEXCEPTION_ALARM struAudioException = {0};
	memset(&struAudioException, 0, sizeof(NET_DVR_AUDIOEXCEPTION_ALARM));
	memcpy(&struAudioException, pAlarmInfo, sizeof(NET_DVR_AUDIOEXCEPTION_ALARM));
	switch (struAudioException.byAlarmType)
	{
	case 1:
		g_StringLanType(szAlarmType, "��Ƶ�����쳣","Audio Input Exception");
		break;
	case 2:
		g_StringLanType(szAlarmType, "��Ƶ����ͻ��","Audio Input Mutation");
		break;
	case 3:
		g_StringLanType(szAlarmType, "��ǿ����","Audio Decibel Dropped");
		break;
	default:
		g_StringLanType(szAlarmType, "δ֪","Unknow");
		break;
	}
	
	g_StringLanType(szLan, "�����쳣����","Audio Exception");
	sprintf(szInfoBuf, "%s:[AlarmType:%s AudioDecibel:%d IP:%s Port:%d Channel:%d IvmsChannel:%d]\n", szLan,\
		szAlarmType, struAudioException.wAudioDecibel, struAudioException.struDevInfo.struDevIP.sIpV4, struAudioException.struDevInfo.wPort, \
		struAudioException.struDevInfo.byChannel, struAudioException.struDevInfo.byIvmsChannel);
	AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
}

void CClientDemoDlg::ProcHeatMapResult(WPARAM wParam, LPARAM lParam)//�ȶ�ͼ�����ϴ� 2014-03-21
{
	char szInfoBuf[1024] = {0};
	char szLan[128] = {0};
	char szAlarmType[128] = {0};
	char cFilename[256] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;	
	NET_DVR_HEATMAP_RESULT struHeatMapResult = {0};
	memset(&struHeatMapResult, 0, sizeof(NET_DVR_HEATMAP_RESULT));
	memcpy(&struHeatMapResult, pAlarmInfo, sizeof(NET_DVR_HEATMAP_RESULT));
	sprintf(cFilename, "No heat map");
	
	if (struHeatMapResult.wArrayColumn > 0 && struHeatMapResult.wArrayLine > 0 && struHeatMapResult.pBuffer != NULL)
	{
		sprintf(cFilename, "%s\\%s", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIPInFileName);
		if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
		{
			CreateDirectory(cFilename, NULL);
		}
		
		SYSTEMTIME t;
		GetLocalTime(&t);
		char chTime[128];
		sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
		
		sprintf(cFilename, "%s\\HeatMapValue[%s].txt", cFilename, chTime);
		HANDLE hFile;	
		DWORD dwReturn = 0;
		hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}
		WriteFile(hFile, struHeatMapResult.pBuffer, struHeatMapResult.wArrayColumn*struHeatMapResult.wArrayLine, &dwReturn, NULL);
		CloseHandle(hFile);
		hFile = NULL;
	}
	
	g_StringLanType(szLan, "�ȶ�ͼ����","Heat Map Result");
	sprintf(szInfoBuf, "%s:[IP:%s Port:%d Channel:%d IvmsChannel:%d StartTime%d-%d-%d %d:%d:%d EndTime:%d-%d-%d %d:%d:%d ArrayLine:%d ArrayColumn:%d HeatMapPath:%s]\n", \
		szLan, struHeatMapResult.struDevInfo.struDevIP.sIpV4, struHeatMapResult.struDevInfo.wPort, struHeatMapResult.struDevInfo.byChannel,\
		struHeatMapResult.struDevInfo.byIvmsChannel, struHeatMapResult.struStartTime.wYear, struHeatMapResult.struStartTime.byMonth,\
		struHeatMapResult.struStartTime.byDay, struHeatMapResult.struStartTime.byHour, struHeatMapResult.struStartTime.byMinute,\
		struHeatMapResult.struStartTime.bySecond, struHeatMapResult.struEndTime.wYear, struHeatMapResult.struEndTime.byMonth, struHeatMapResult.struEndTime.byDay,\
		struHeatMapResult.struEndTime.byHour, struHeatMapResult.struEndTime.byMinute, struHeatMapResult.struEndTime.bySecond, struHeatMapResult.wArrayLine,\
		struHeatMapResult.wArrayColumn, cFilename);
	AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
	
	if (struHeatMapResult.pBuffer != NULL)
	{
		delete [] (struHeatMapResult.pBuffer);
        struHeatMapResult.pBuffer = NULL;
	}
}

//ѹ�߱�����ʱ�����⿪�š�
// void CClientDemoDlg::ProcCrossLineData(WPARAM wParam, LPARAM lParam)//2013-09-27 ѹ�߱���
// {
// 	char szInfoBuf[1024] = {0};
// 	char szTriggerTime[128] = {0};
// 	char szLan[128] = {0};
// 	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
//     char *pAlarmInfo = (char *)(lParam);		
//     int iDeviceIndex = pAlarmDev->iDeviceIndex;	
// 	NET_DVR_CROSSLINE_ALARM struCrossLine = {0};
// 	memcpy(&struCrossLine, pAlarmInfo, sizeof(struCrossLine));
// 	
// 	g_StringLanType(szLan, "ѹ�߱���","Cross Line");
// 	
// 	sprintf(szTriggerTime, "%d-%d-%d %d:%d:%d", struCrossLine.struTriggerTime.wYear, 
// 		struCrossLine.struTriggerTime.byMonth, struCrossLine.struTriggerTime.byDay,
// 		struCrossLine.struTriggerTime.byHour, struCrossLine.struTriggerTime.byMinute,
// 		struCrossLine.struTriggerTime.byDay);
// 	
// 	sprintf(szInfoBuf, "%s: (Trigger Time:[%s]  Device IP:[%s]  Port:[%d]  Device Channel:[%d]  Ivms Channel:[%d]  Plate Number:[%s]  Park Index:[%s])\n", 
// 		szLan, szTriggerTime, struCrossLine.struDevInfo.struDevIP.sIpV4, struCrossLine.struDevInfo.wPort, 
// 		struCrossLine.struDevInfo.byChannel, struCrossLine.struDevInfo.byIvmsChannel, struCrossLine.struParkInfo.sPlateNo,
// 		struCrossLine.struParkInfo.sParkIndex);
// 	
// 	AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
// }


void CClientDemoDlg::ProcAuxAlarm(WPARAM wParam, LPARAM lParam)
{
    char szInfoBuf[1024] = {0};
	char sAlarmName[33] = {0};
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
	
	NET_IPC_AUXALARM_RESULT struAuxAlarm = {0};
	memcpy(&struAuxAlarm, pAlarmInfo, sizeof(struAuxAlarm));
	
	//     g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
	//     ::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));
	//  ��ӡ��Ϣ
    CTime time = CTime::GetCurrentTime();
    
	if (struAuxAlarm.byAlarmType == IPC_AUXALARM_PIR)
	{
		memcpy(sAlarmName, struAuxAlarm.struAuxAlarm.struPIRAlarm.byAlarmName, NAME_LEN);
	}
	else if (struAuxAlarm.byAlarmType == IPC_AUXALARM_WIRELESS)
	{
		memcpy(sAlarmName, struAuxAlarm.struAuxAlarm.struWirelessAlarm.byAlarmName, NAME_LEN);
	}
	else
	{
		strncpy(sAlarmName, "Call help", NAME_LEN);
	}
	
    sprintf(szInfoBuf, "AuxAlarm Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d] Type[%d] AlarmName[%s]\n",
        time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(),
        time.GetMinute(), time.GetSecond(), struAuxAlarm.byAlarmType, sAlarmName);
    AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
}

void CClientDemoDlg::ProcITSPlateAlarm(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iWinIndex = -1;         
	int i = 0;
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_ITS_PLATE_RESULT  struPlateResult = {0};	
	memcpy(&struPlateResult, pAlarmInfo, sizeof(struPlateResult));
	i = struPlateResult.byChanIndex;
	g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
	::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));
	
	// CString str = "";
	sprintf(szInfoBuf, "ITS Plate Alarm MatchNo[%d]DriveChan[%d]IllegalType[%d]IllegalSubType[%s]\
		MonitoringSiteID[%s] DeviceID[%s] Dir[%d] PicNum[%d] License[%s] ", 
		struPlateResult.dwMatchNo, struPlateResult.byDriveChan, struPlateResult.wIllegalType,
		struPlateResult.byIllegalSubType, struPlateResult.byMonitoringSiteID, struPlateResult.byDeviceID,
		struPlateResult.byDir, struPlateResult.dwPicNum,struPlateResult.struPlateInfo.sLicense);
	
    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);	
	for (i = 0; i < 6; i++)
	{
		if (struPlateResult.struPicInfo[i].dwDataLen > 0 && struPlateResult.struPicInfo[i].pBuffer != NULL)
		{
			sprintf(szInfoBuf, "ITS picture[%d] info Type[%d] RedLightTime[%d] AbsTime[%s]", i+1, struPlateResult.struPicInfo[i].byType,
				struPlateResult.struPicInfo[i].dwRedLightTime, struPlateResult.struPicInfo[i].byAbsTime);
			g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);	
			
			int num = ulAlarmPic/1000+1;
			ulAlarmPic++;
			
			char cFilename[256] = {0};
			HANDLE hFile;
			DWORD dwReturn;
			
			SYSTEMTIME t;
			GetLocalTime(&t);
			char chTime[128];
			sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
			
			sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIPInFileName, num);
			if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
			{
				CreateDirectory(cFilename, NULL);
			}

			if (struPlateResult.struPicInfo[i].byDataType == 0)
			{
				sprintf(cFilename, "%s\\LocalTime[%s]_%d.jpg",cFilename, chTime,rand() % GetTickCount());
			} 
			else if (struPlateResult.struPicInfo[i].byDataType == 1)
			{
				sprintf(cFilename, "%s\\URL_LocalTime[%s]_%d.txt",cFilename, chTime,rand() % GetTickCount());
			}
			
			//sprintf(cFilename, "%s\\LocalTime[%s]_%d.jpg",cFilename, chTime, i);
			hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				return;
			}
			WriteFile(hFile, struPlateResult.struPicInfo[i].pBuffer, struPlateResult.struPicInfo[i].dwDataLen, &dwReturn, NULL);
			CloseHandle(hFile);
			hFile = NULL;
		}
	}    		
}

void CClientDemoDlg::ProcITSGateVehicle(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		         
	int i = 0;
	int iDeviceIndex = pAlarmDev->iDeviceIndex;

	NET_ITS_GATE_VEHICLE  struGateVehicle = {0};
	memcpy(&struGateVehicle, pAlarmInfo, sizeof(struGateVehicle));
	
	sprintf(szInfoBuf, "ITS Gate Vehicle MatchNo[%d] LaneID[%d] CamLaneId[%d] ChanIndex[%d] MonitoringSiteID[%s] DeviceID[%s] Dir[%d] PicNum[%d]",\
		struGateVehicle.dwMatchNo, struGateVehicle.wLaneid, struGateVehicle.byCamLaneId, struGateVehicle.dwChanIndex,\
		struGateVehicle.byMonitoringSiteID, struGateVehicle.byDeviceID, struGateVehicle.byDir, struGateVehicle.dwPicNum);
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);

	for (i = 0; i < 4; i++)
	{
		if (struGateVehicle.struPicInfo[i].dwDataLen > 0 && struGateVehicle.struPicInfo[i].pBuffer != NULL)
		{
			sprintf(szInfoBuf, "ITS picture[%d] info Type[%d] RedLightTime[%d] AbsTime[%s]", i+1, struGateVehicle.struPicInfo[i].byType,
				struGateVehicle.struPicInfo[i].dwRedLightTime, struGateVehicle.struPicInfo[i].byAbsTime);
			g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);	
			
			int num = ulAlarmPic/1000+1;
			ulAlarmPic++;
			
			char cFilename[256] = {0};
			HANDLE hFile;
			DWORD dwReturn;
			
			SYSTEMTIME t;
			GetLocalTime(&t);
			char chTime[128];
			sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
			
			sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIPInFileName, num);
			if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
			{
				CreateDirectory(cFilename, NULL);
			}
			
			sprintf(cFilename, "%s\\ITSGateVehicle[%s]_%d.jpg",cFilename, chTime, i);
			hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			
			if (hFile == INVALID_HANDLE_VALUE)
			{
				return;
			}
			WriteFile(hFile, struGateVehicle.struPicInfo[i].pBuffer, struGateVehicle.struPicInfo[i].dwDataLen, &dwReturn, NULL);
			CloseHandle(hFile);
			hFile = NULL;
		}
	}
}

void CClientDemoDlg::ProcITSGateFace(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		         
	int i = 0;
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
	
	NET_ITS_GATE_FACE  struGateFace = {0};
	memcpy(&struGateFace, pAlarmInfo, sizeof(struGateFace));

	sprintf(szInfoBuf, "ITS Gate Face Laneid[%d] CamLaneId[%d] MonitoringSiteID[%s] DeviceID[%s]",\
		struGateFace.wLaneid, struGateFace.byCamLaneId, struGateFace.byMonitoringSiteID, struGateFace.byDeviceID);
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);

	//  ��ӡ��Ϣ
    NET_DVR_TIME  struAbsTime = {0};
    struAbsTime.dwYear = GET_YEAR(struGateFace.struFaceInfo.dwAbsTime);
    struAbsTime.dwMonth = GET_MONTH(struGateFace.struFaceInfo.dwAbsTime);
    struAbsTime.dwDay = GET_DAY(struGateFace.struFaceInfo.dwAbsTime);
    struAbsTime.dwHour = GET_HOUR(struGateFace.struFaceInfo.dwAbsTime);
    struAbsTime.dwMinute = GET_MINUTE(struGateFace.struFaceInfo.dwAbsTime);
    struAbsTime.dwSecond = GET_SECOND(struGateFace.struFaceInfo.dwAbsTime);
	
    sprintf(szInfoBuf, "Face Snap alarm Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d] FaceID[%d]\n",
        struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay, struAbsTime.dwHour,
        struAbsTime.dwMinute, struAbsTime.dwSecond, struGateFace.struFaceInfo.dwFacePicID);
    AddLog(m_iCurDeviceIndex, ALARM_INFO_T, szInfoBuf);
	
    if (struGateFace.struFaceInfo.dwFacePicLen > 0 && struGateFace.struFaceInfo.pBuffer1 != NULL)
    {
        int num = ulAlarmPic/1000+1;
        ulAlarmPic++;
        
        char cFilename[256] = {0};
        HANDLE hFile;
        DWORD dwReturn;
        
        SYSTEMTIME t;
        GetLocalTime(&t);
        char chTime[128];
        sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
        
        sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIP, num);
        if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
        {
            CreateDirectory(cFilename, NULL);
        }
        
        sprintf(cFilename, "%s\\ITSGateFace[%s].jpg",cFilename, chTime);
        hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            return;
        }
        WriteFile(hFile, struGateFace.struFaceInfo.pBuffer1, struGateFace.struFaceInfo.dwFacePicLen, &dwReturn, NULL);
        CloseHandle(hFile);
        hFile = NULL;
	}
	
	if (struGateFace.struFaceInfo.dwBackgroundPicLen > 0 && struGateFace.struFaceInfo.pBuffer2 != NULL)
    {
        int num = ulAlarmPic/1000+1;
        ulAlarmPic++;
        
        char cFilename[256] = {0};
        HANDLE hFile;
        DWORD dwReturn;
        
        SYSTEMTIME t;
        GetLocalTime(&t);
        char chTime[128];
        sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
        
        sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIP, num);
        if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
        {
            CreateDirectory(cFilename, NULL);
        }
        
        sprintf(cFilename, "%s\\BackgroundPic[%s].jpg",cFilename, chTime);
        hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            return;
        }
        WriteFile(hFile, struGateFace.struFaceInfo.pBuffer2, struGateFace.struFaceInfo.dwBackgroundPicLen, &dwReturn, NULL);
        CloseHandle(hFile);
        hFile = NULL;
	}
	
	if (struGateFace.struFaceInfo.pBuffer1 != NULL)
	{
		delete []struGateFace.struFaceInfo.pBuffer1;
		struGateFace.struFaceInfo.pBuffer1 = NULL;
	}
	
	if (struGateFace.struFaceInfo.pBuffer2 != NULL)
	{
		delete []struGateFace.struFaceInfo.pBuffer2;
		struGateFace.struFaceInfo.pBuffer2 = NULL;
	}
}

void CClientDemoDlg::ProcITSParkVehicle(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		         
	int i = 0;
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
	
	NET_ITS_PARK_VEHICLE  struParkVehicle = {0};
	memcpy(&struParkVehicle, pAlarmInfo, sizeof(struParkVehicle));

	sprintf(szInfoBuf, "ITS Park Vehicle LocationStatus[%d] MonitoringSiteID[%s] DeviceID[%s] PicNum[%d]",\
		struParkVehicle.byLocationStatus, struParkVehicle.byMonitoringSiteID, struParkVehicle.byDeviceID, struParkVehicle.dwPicNum);
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);

	for (i = 0; i < 2; i++)
	{
		if (struParkVehicle.struPicInfo[i].dwDataLen > 0 && struParkVehicle.struPicInfo[i].pBuffer != NULL)
		{
			sprintf(szInfoBuf, "ITS picture[%d] info Type[%d] RedLightTime[%d] AbsTime[%s]", i+1, struParkVehicle.struPicInfo[i].byType,
				struParkVehicle.struPicInfo[i].dwRedLightTime, struParkVehicle.struPicInfo[i].byAbsTime);
			g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);	
			
			int num = ulAlarmPic/1000+1;
			ulAlarmPic++;
			
			char cFilename[256] = {0};
			HANDLE hFile;
			DWORD dwReturn;
			
			SYSTEMTIME t;
			GetLocalTime(&t);
			char chTime[128];
			sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
			
			sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIPInFileName, num);
			if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
			{
				CreateDirectory(cFilename, NULL);
			}
			
			sprintf(cFilename, "%s\\LocalTimeITSParkVehicle[%s]_%d.jpg",cFilename, chTime, i);
			hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				return;
			}
			WriteFile(hFile, struParkVehicle.struPicInfo[i].pBuffer, struParkVehicle.struPicInfo[i].dwDataLen, &dwReturn, NULL);
			CloseHandle(hFile);
			hFile = NULL;
		}
	}
}

void CClientDemoDlg::ProcITSBalckListInfo(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		         
	int i = 0;
	int iDeviceIndex = pAlarmDev->iDeviceIndex;

	NET_ITS_ECT_BLACKLIST struBlackList = {0};
	memcpy(&struBlackList, pAlarmInfo, sizeof(struBlackList));
	
	sprintf(szInfoBuf, "ITS BLACK LIST INFO Channel[%d] LogicalLaneNo[%d] LaneName[%d] License[%s]",\
		struBlackList.dwChannel, struBlackList.bylogicalLaneNo, struBlackList.bylogicalLaneNo, struBlackList.struPlateInfo.sLicense);
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
}


void CClientDemoDlg::ProcITSRoadException(WPARAM wParam, LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iWinIndex = -1;         
	int i = 0;
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_ITS_ROADINFO  struResult = {0};	
	memcpy(&struResult, pAlarmInfo, sizeof(struResult));
	g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
	::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));
	
	// CString str = "";
    sprintf(szInfoBuf, "ITS Road Exception ChanNo[%d]DeviceNum[%d]", 
		struResult.dwChannel, struResult.dwDeviceNum);
	
    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);	
}

/*********************************************************
Function:	OnWMProcAlarm
Desc:		process alarm information when 
Input:	none
Output:	none
Return:	none
**********************************************************/
LRESULT CClientDemoDlg::OnWMProcAlarm(WPARAM wParam, LPARAM lParam)
{	
	//EnterCriticalSection(&g_cs);

	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
	if (iDeviceIndex < 0 && iDeviceIndex >= MAX_DEVICES)
	{
		delete [] pAlarmInfo;
		delete pAlarmDev;
		return NULL;
	}
	char szLan[128] ={0};
	char szInfoBuf[1024] = {0};
	switch (pAlarmDev->lCommand)
	{
	case COMM_IPC_AUXALARM_RESULT:
		ProcAuxAlarm(wParam, lParam);
		break;
	case COMM_ALARM:
		ProcAlarm(wParam, lParam);
		break;
	case COMM_ALARM_V30:
		ProcAlarmV30(wParam, lParam);
		break;
	case COMM_ALARM_V40:
        ProcAlarmV40(wParam, lParam);
        break;
	case COMM_TRADEINFO:
		NET_DVR_TRADEINFO m_TradeInfo;
		memcpy(&m_TradeInfo, pAlarmInfo, sizeof(NET_DVR_TRADEINFO));
		g_StringLanType(szLan, "�յ�������Ϣ", "receive trade card number");
		sprintf(szInfoBuf, "%s%s", m_TradeInfo.CardNumber);
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);  
		break;
	case COMM_IPCCFG://input parameters modified
		ProcIPCfgAlarm(wParam, lParam);
		break;
	case COMM_IPCCFG_V31:
		ProcIPCfgAlarmV31(wParam, lParam);
		break;
	case COMM_ALARM_RULE:
		ProcRuleAlarm(wParam, lParam);
		break;
    case COMM_ALARM_PDC:
        ProcPdcAlarm(wParam, lParam);
        break;
    case COMM_SENSOR_VALUE_UPLOAD:
        ProcSensorValueUpload(wParam, lParam);
        break;
    case COMM_SENSOR_ALARM:
        ProcSensorAlarm(wParam, lParam);
        break;
    case COMM_ALARM_AID:
        ProcAidAlarm(wParam, lParam);
        break;
    case COMM_ALARM_TPS:
        ProcTpsAlarm(wParam, lParam);
        break;
	case COMM_ALARM_AID_V41:
		ProcAidAlarmV41(wParam,lParam);
		break;
	case COMM_ALARM_TFS:
		ProcTfsAlarm(wParam,lParam);
		break;
	case COMM_ALARM_TPS_V41:
		ProcTpsAlarmV41(wParam,lParam);
		break;
    case COMM_ALARM_FACE:
        ProcFaceDetectAlarm(wParam, lParam);
        break;
	case COMM_UPLOAD_FACESNAP_RESULT:
		ProcFaceSnapAlarm(wParam, lParam);
		break;
	case COMM_SNAP_MATCH_ALARM:
		ProcFaceSnapMatchAlarm(wParam, lParam);
		break;
    case COMM_RULE_INFO_UPLOAD:
        ProcRuleInfoUpload(wParam, lParam);
        break;
	//2010-06-01
	case COMM_ALARM_VIDEOPLATFORM:
		memset(&m_struVideoPlatform, 0, sizeof(m_struVideoPlatform));
		memcpy(&m_struVideoPlatform, pAlarmInfo, sizeof(m_struVideoPlatform));

		if (m_struVideoPlatform.byAlarmType == 0)
		{
			g_StringLanType(szLan, "�յ���Ƶ�ۺ�ƽ̨�γ��Ӱ屨��,��ʼ��λ��:", "receive videoplatform pull board alarm, start slotnum:");
			sprintf(szInfoBuf, "%s%d", szLan, m_struVideoPlatform.bySubSystemChan[0]);
		}
		else if (m_struVideoPlatform.byAlarmType == 1)
		{
			g_StringLanType(szLan, "�յ���Ƶ�ۺ�ƽ̨�����Ӱ屨��,��ʼ��λ��:", "receive videoplatform push board alarm, start slotnum:");
			sprintf(szInfoBuf, "%s%d", szLan, m_struVideoPlatform.bySubSystemChan[0]);
		}
		else if (m_struVideoPlatform.byAlarmType == 2)
		{
			g_StringLanType(szLan, "�յ���Ƶ�ۺ�ƽ̨�¶ȳ����ޱ���,�¶�:", "receive videoplatform temperature uplimited alarm, temp:");
			sprintf(szInfoBuf, "%s%d.%d", szLan, (m_struVideoPlatform.iTemperature / 100), m_struVideoPlatform.iTemperature % 100);
		}
		else if (m_struVideoPlatform.byAlarmType == 3)
		{
			g_StringLanType(szLan, "�յ���Ƶ�ۺ�ƽ̨�¶ȳ����ޱ���,�¶�:", "receive videoplatform temperature downlimited alarm, temp:");
			sprintf(szInfoBuf, "%s%d.%d", szLan, (m_struVideoPlatform.iTemperature / 100), m_struVideoPlatform.iTemperature % 100);
		}
        else if (m_struVideoPlatform.byAlarmType == 4)
        {
            g_StringLanType(szLan, "�յ���Ƶ�ۺ�ƽ̨����״̬�쳣����,���Ⱥ�:", "receive videoplatform fan exception alarm, fan num:");
            sprintf(szInfoBuf, "%s", szLan);
            
            for (int i = 0; i < 32; i++)
            {
                sprintf(&szInfoBuf[i + strlen(szLan)], "%d", m_struVideoPlatform.byFanSequence[i]);                
            }
        }
        else if (m_struVideoPlatform.byAlarmType == 5)
        {
            g_StringLanType(szLan, "�յ���Ƶ�ۺ�ƽ̨��ϵͳ�쳣����,��ϵͳ��:", "receive videoplatform sub system exception alarm, sub system slot:");
            sprintf(szInfoBuf, "%s%d", szLan, m_struVideoPlatform.bySubSystemChan[0]);
        }
        else if (m_struVideoPlatform.byAlarmType == 6)
        {
            g_StringLanType(szLan, "�յ���Ƶ�ۺ�ƽ̨�Ƿ����ʱ���", "receive videoplatform illegal visit alarm");
            sprintf(szInfoBuf, "%s", szLan);
        }

		if (m_struVideoPlatform.byBoardNum != 0xff)
		{
			g_StringLanType(szLan, "�Ӱ��:", "Board number:");
			sprintf(szInfoBuf, "%s%s%d", szInfoBuf, szLan, m_struVideoPlatform.byBoardNum);
		}

		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);  
		break;
    case COMM_ALARM_ALARMHOST:
        //2010-06-30
        ProcAlarmHostAlarm(wParam, lParam);
        break;
    case COMM_SWITCH_ALARM:
        ProcSwitchAlarm(wParam, lParam);
        break;
    case COMM_ALARMHOST_EXCEPTION:
        ProcAlarmHostException(wParam, lParam);
        break;
	case COMM_ALARM_DEVICE:
		ProcDeviceAlarm(wParam, lParam);
		break;
	case COMM_ALARM_CVR:
		ProcCVRAlarm(wParam, lParam);
		break;
	case COMM_ALARMHOST_SAFETYCABINSTATE:
		ProcAlarmHostSatetyCabinState(wParam, lParam);
		break;
	case COMM_ALARMHOST_ALARMOUTSTATUS:
		ProcAlarmHostAlarmoutStatus(wParam, lParam);
		break;
	case COMM_ITS_PLATE_RESULT:
		ProcITSPlateAlarm(wParam, lParam);
		break;
	case COMM_ITS_ROAD_EXCEPTION:
		ProcITSRoadException(wParam, lParam);
		break;
	case COMM_ALARMHOST_CID_ALARM:
		ProcAlarmHostCIDAlarm(wParam, lParam);
		break;
	case COMM_ALARMHOST_EXTERNAL_DEVICE_ALARM:
		ProcAlarmHostExternalDevAlarm(wParam, lParam);
		break;
	case COMM_ALARM_VQD:
		ProcAlarmVQD(wParam,lParam);
		break;
	case COMM_ALARM_VQD_EX:
		ProcAlarmVQDEx(wParam,lParam);
		break;
	case COMM_ITS_GATE_VEHICLE:
		ProcITSGateVehicle(wParam, lParam);
		break;
	case COMM_ITS_GATE_FACE:
		ProcITSGateFace(wParam, lParam);
		break;
	case COMM_ITS_PARK_VEHICLE:
		ProcITSParkVehicle(wParam, lParam);
		break;
	case COMM_ITS_BLACKLIST_ALARM:
		ProcITSBalckListInfo(wParam, lParam);
		break;		
	case COMM_ALARMHOST_DATA_UPLOAD:
		ProcAlarmHostData(wParam, lParam);
		break;
	case COMM_ITS_GATE_ALARMINFO:
		ProcGateAlarmHostData(wParam, lParam);
		break;
	case COMM_ITS_EXTERNAL_CONTROL_ALARM://��������(��ر���)2013-06-09
		ProcExternalControlAlarmHostData(wParam, lParam);
		break;
	case COMM_SCENECHANGE_DETECTION_UPLOAD://2013-07-17���������ⱨ��
		ProcSceneChangeDetection(wParam, lParam);
		break;
//ѹ�߱�����ʱ�����⿪�š�
// 	case COMM_CROSSLINE_ALARM://ѹ�߱��� 2013-09-27
// 		ProcCrossLineData(wParam, lParam);
// 		break;
	case COMM_ALARM_HOT_SPARE:
		ProcHotSpare(wParam,lParam);
		break;
		//2013-11-19
	case COMM_ITS_GATE_COSTITEM://����ڹ����շ���ϸ
		ProcVehCostItem(wParam, lParam);
		break;
	case COMM_ITS_GATE_HANDOVER://����ڽ��Ӱ�����
		ProcVehHandOver(wParam, lParam);
		break;
	case COMM_ALARM_AUDIOEXCEPTION://�����쳣�����ϴ� 2014-03-21
		ProcAudioException(wParam, lParam);
		break;
	case COMM_UPLOAD_HEATMAP_RESULT://�ȶ�ͼ�����ϴ� 2014-03-21
		ProcHeatMapResult(wParam, lParam);
		break;
	default:
		break;
	}
	if (pAlarmInfo != NULL)
	{
		delete[] pAlarmInfo;
		pAlarmInfo = NULL;
	}
	if (pAlarmDev != NULL)
	{
		delete pAlarmDev;
		pAlarmDev = NULL;
	}
	//LeaveCriticalSection(&g_cs);
	return NULL;
}
//the structure of the log information
typedef struct tagLOCAL_LOG_INFO 
{
	int iLogType;
	char szTime[64];
	char szLogInfo[512];
	char szDevInfo[128];
	char szErrInfo[256];
	tagLOCAL_LOG_INFO ()
	{
		iLogType = 0;
		szTime[0] = '\0';
		szLogInfo[0] = '\0';
		szDevInfo[0] = '\0';
		szErrInfo[0] = '\0';
	}
}LOCAL_LOG_INFO, *LPLOCAL_LOG_INFO;

/*********************************************************
Function:	OnWMAddLog
Desc:		responding to the message WM_ADD_LOG
Input:	wParam, parameter 1;lParam, parameter 2;
Output:	none
Return:	result code
**********************************************************/
LRESULT CClientDemoDlg::OnWMAddLog(WPARAM wParam, LPARAM lParam)
{
	EnterCriticalSection(&g_cs);
	LPLOCAL_LOG_INFO pLogInfo = LPLOCAL_LOG_INFO(lParam);
	if (NULL == pLogInfo)
	{
		return 0;
	}
	char szLogType[32] = "FAIL";
	
	char szTime[64] = {0};
	char szLogInfo[512] = {0};
	char szDevInfo[128] = {0};
	char szErrInfo[256] = {0};
	memcpy(szTime, pLogInfo->szTime, 64);
	memcpy(szLogInfo, pLogInfo->szLogInfo, 512);
	memcpy(szDevInfo, pLogInfo->szDevInfo, 128);
	memcpy(szErrInfo, pLogInfo->szErrInfo, 256);
	if ( 5000 == m_listAllLog.GetItemCount())
	{
		m_listAllLog.DeleteAllItems();
	}
	
	switch (pLogInfo->iLogType)
	{
	case ALARM_INFO_T:
		sprintf(szLogType, "Alarm");
		m_listAlarmInfo.InsertItem(0, "", -1);
		m_listAlarmInfo.SetItemText(0, 1, szTime);
		m_listAlarmInfo.SetItemText(0, 2, szLogInfo);
		m_listAlarmInfo.SetItemText(0, 3, szDevInfo);
		break;
	case OPERATION_SUCC_T:
		sprintf(szLogType, "SUCC");	
		if (g_struLocalParam.bSuccLog)
		{
			m_listAllLog.InsertItem(0, "", -1);
			m_listAllLog.SetItemText(0, 1, szTime);
			m_listAllLog.SetItemText(0, 2, szLogType);
			m_listAllLog.SetItemText(0, 3, szLogInfo);
			m_listAllLog.SetItemText(0, 4, szDevInfo);
			m_listAllLog.SetItemText(0, 5, szErrInfo);
		}
		break;
	case OPERATION_FAIL_T:
		sprintf(szLogType, "FAIL");		
		if (g_struLocalParam.bFailLog)
		{
			m_listAllLog.InsertItem(0, "", -1);
			m_listAllLog.SetItemText(0, 1, szTime);
			m_listAllLog.SetItemText(0, 2, szLogType);
			m_listAllLog.SetItemText(0, 3, szLogInfo);
			m_listAllLog.SetItemText(0, 4, szDevInfo);
			m_listAllLog.SetItemText(0, 5, szErrInfo);
		}
		break;
	case PLAY_SUCC_T:
		sprintf(szLogType, "SUCC");
		if (g_struLocalParam.bSuccLog)
		{
			m_listAllLog.InsertItem(0, "", -1);
			m_listAllLog.SetItemText(0, 1, szTime);
			m_listAllLog.SetItemText(0, 2, szLogType);
			m_listAllLog.SetItemText(0, 3, szLogInfo);
			m_listAllLog.SetItemText(0, 4, szDevInfo);
			m_listAllLog.SetItemText(0, 5, szErrInfo);
		}
		break;
	case PLAY_FAIL_T:
		sprintf(szLogType, "FAIL");	
		if (g_struLocalParam.bFailLog)
		{
			m_listAllLog.InsertItem(0, "", -1);
			m_listAllLog.SetItemText(0, 1, szTime);
			m_listAllLog.SetItemText(0, 2, szLogType);
			m_listAllLog.SetItemText(0, 3, szLogInfo);
			m_listAllLog.SetItemText(0, 4, szDevInfo);
			m_listAllLog.SetItemText(0, 5, szErrInfo);
		}
		break;
	default:
		sprintf(szLogType, "FAIL");	
		if (g_struLocalParam.bFailLog)
		{
			m_listAllLog.InsertItem(0, "", -1);
			m_listAllLog.SetItemText(0, 1, szTime);
			m_listAllLog.SetItemText(0, 2, szLogType);
			m_listAllLog.SetItemText(0, 3, szLogInfo);
			m_listAllLog.SetItemText(0, 4, szDevInfo);
			m_listAllLog.SetItemText(0, 5, szErrInfo);
		}
		break;
	}
	if (pLogInfo != NULL)
	{
		delete pLogInfo;
		pLogInfo = NULL;
	}
	LeaveCriticalSection(&g_cs);
	return 0;
}

/*********************************************************
Function:	AddLog
Desc:		add local log
Input:	iLogType, log type, 0-alam, 1-operate log, 2-debug info; csLogInfo log info
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::AddLog(int iDeviceIndex, int iLogType, const char* format, ...)
{	
	if (g_bExitDemo)
	{
		return;
	}
	CTime  cTime = CTime::GetCurrentTime();
	int iLen = 0;
	char szTime[64] = {0};
	char szLogType[32] = "FAIL";
	char szLogInfo[1024] = {0};
	char szDevInfo[256] = {0};
	char szErrInfo[256] = {0};
	char szLog[1024] = {0};
	va_list arglist;
	va_start(arglist,format);
	vsprintf(szLogInfo,format,arglist); 
	va_end(arglist);
	
	sprintf(szTime, "%s", cTime.Format("%y-%m-%d %H:%M:%S").GetBuffer(0));
	
	if (iDeviceIndex != -1 && iDeviceIndex < 512)
	{
		sprintf(szDevInfo, "[%s]-[%s]", g_struDeviceInfo[iDeviceIndex].chDeviceIP, g_struDeviceInfo[iDeviceIndex].chLocalNodeName);
	}
	
	switch (iLogType)
	{
	case OPERATION_SUCC_T:
	case PLAY_SUCC_T:
		sprintf(szErrInfo, "");
		sprintf(szLogType, "SUCC");
		break;
	case PLAY_FAIL_T:
		sprintf(szErrInfo, "PLAY_M4 Eorror!!!");	
		break;
	case OPERATION_FAIL_T:
	default:
		sprintf(szErrInfo, "err[%d:%s]", NET_DVR_GetLastError(), NET_DVR_GetErrorMsg());	
		break;
	}
	//sprintf(szErrInfo, "err[%d:%s]", NET_DVR_GetLastError(), NET_DVR_GetErrorMsg());
	if (iLogType == ALARM_INFO_T )
	{
		iLen = sprintf(szLog, "%s %s\n", szTime, szLogInfo, szDevInfo);
		if (m_fAlarm != NULL && g_struLocalParam.bAutoSaveLog)
		{
			if (fwrite(szLog, 1, iLen, m_fAlarm) != (unsigned)iLen)
			{
				OutputDebugString("write err");
			}
		}
	}
	else
	{
		iLen = sprintf(szLog, "%s %s %s %s %s\n", szTime, szLogType, szLogInfo, szDevInfo, szErrInfo);
		if (m_fLog != NULL && g_struLocalParam.bAutoSaveLog)
		{
			if (fwrite(szLog, 1, iLen, m_fLog) != (unsigned)iLen)
			{
				OutputDebugString("write err");
			}
		}
	}
	
	if (g_struLocalParam.bOutputDebugString)
	{
		OutputDebugString(szLog);
	}
	if (!g_struLocalParam.bSuccLog && !g_struLocalParam.bFailLog && (iLogType != ALARM_INFO_T ))
	{
		return;
	}
	LPLOCAL_LOG_INFO pLogInfo = NULL;
	try
    {  
		
		pLogInfo = new LOCAL_LOG_INFO;
		memset(pLogInfo, 0, sizeof(LOCAL_LOG_INFO));
		if (pLogInfo == NULL)
		{
			return;
		}
		pLogInfo->iLogType = iLogType;
		memcpy(pLogInfo->szTime, szTime, 64);
		memcpy(pLogInfo->szLogInfo, szLogInfo, 512);
		memcpy(pLogInfo->szDevInfo, szDevInfo, 128);
		memcpy(pLogInfo->szErrInfo, szErrInfo, 256);
		::PostMessage(g_pMainDlg->m_hWnd, WM_ADD_LOG, iDeviceIndex, (LONG)pLogInfo);
	}
	catch (...)
	{
		if (pLogInfo != NULL)
		{
			return;
		}
		OutputDebugString("New Log Exception!!\n");
	}
}

/*************************************************
Function:    	DblAllTree
Description:	preview all device if some are not previewed, or stop all previewing devices
Input:          iDeviceIndex, device index; iStartOutputIndex, preview start index;
Return:		    ��
*************************************************/
void CClientDemoDlg::DblAllTree()
{
	HTREEITEM hSelect = m_treeDeviceList.GetSelectedItem();
	
	int i=0;
	int iDevIndex = 0;
	
	hSelect = m_treeDeviceList.GetChildItem(hSelect);
	
	for (i=0; i<MAX_DEVICES; i++)
	{
		//shut down all play channel
		
		if (g_struDeviceInfo[i].lLoginID < 0)
		{//play the window that is not previewed
			iDevIndex = m_treeDeviceList.GetItemData(hSelect)%1000;
			LPLOCAL_LOGIN_PARAM lpParam = new LOCAL_LOGIN_PARAM;
			lpParam->iDeviceIndex = iDevIndex;
			lpParam->hSelect = hSelect;
			lpParam->bMsgBox = FALSE;
			_beginthread((void (__cdecl *)(void *))DoLoginEx, 0, lpParam);
			Sleep(200);
			//LONG lReturn = DoLogin(iDevIndex, hSelect);
			hSelect = m_treeDeviceList.GetNextSiblingItem(hSelect);
			if (hSelect == NULL)
			{
				break;
			}
		}
	}
}
/*************************************************
Function:    	DblPlayDevice
Description:	preview all device if some are not previewed, or stop all previewing devices
Input:          iDeviceIndex, device index; iStartOutputIndex, preview start index;
Return:		    ��
*************************************************/
void CClientDemoDlg::DblPlayDevice(int iDeviceIndex, int iStartOutputIndex)
{
	HTREEITEM hSelect = m_treeDeviceList.GetSelectedItem();
	if (!DoLogin(iDeviceIndex, hSelect))
	{
		return;
	}
	
	int i=0;
	int iChanIndex = 0;
	
	HTREEITEM hChannel = m_treeDeviceList.GetChildItem(hSelect);
	
	if (hChannel ==  NULL)
	{
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "there is no channels!!");
		return;
	}
	
	if (g_struDeviceInfo[iDeviceIndex].bPlayDevice)
	{
		g_struDeviceInfo[iDeviceIndex].bPlayDevice = FALSE;
		//StopPlayAll();
		for (i=0; i< m_iCurWndNum; i++)
		{
			if (g_dlgOutput[i].m_struChanInfo.iDeviceIndex == iDeviceIndex)
			{
				g_dlgOutput[i].StopPlay();
			}	
		}
	}
	else
	{
		for (i=0; i<m_iCurWndNum; i++)
		{
			//shut down all play channel
			if (g_dlgOutput[i].m_struChanInfo.iDeviceIndex == iDeviceIndex)
			{
				g_dlgOutput[i].StopPlay();
			}	
			
			if (g_dlgOutput[i].m_lPlayHandle < 0)
			{//play the window that is not previewed
				iChanIndex = m_treeDeviceList.GetItemData(hChannel)%1000;
				LONG lReturn = g_dlgOutput[i].StartPlay(&g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex], hChannel);
				hChannel = m_treeDeviceList.GetNextSiblingItem(hChannel);
				if (hChannel == NULL)
				{
					break;
				}
			}
			
		}
		g_struDeviceInfo[iDeviceIndex].bPlayDevice = TRUE;
	}
	
	ChangePlayBtnState();
}

/*********************************************************
Function:	StopPlayAll
Desc:		stop all playing windows
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::StopPlayAll(void)
{
	int i = 0;
	for (i = 0; i < MAX_OUTPUTS; i ++)
	{
		if (g_dlgOutput[i].m_lPlayHandle < 0)
		{
			continue;
		}
		g_dlgOutput[i].StopPlay();
	}
}

/*********************************************************
Function:	ChangePlayBtnState
Desc:		change 'play' button text state
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::ChangePlayBtnState(void)
{
	int i = 0;
	char szLan[32] = {0};
	if (m_iMainType == PREVIEW_T)
	{
		for (i = 0; i < MAX_OUTPUTS; i++)
		{
			if (g_dlgOutput[i].m_lPlayHandle < 0)
			{
				if (!g_bPlayAllDevice && i == m_iCurWndIndex)
				{//change network delay
					m_comboNetDelayBufNum.EnableWindow(TRUE);
				}				
				continue;
			}
			
			if (i == m_iCurWndIndex)
			{
				m_comboNetDelayBufNum.EnableWindow(FALSE);
			}
			m_comboWinNum.EnableWindow(FALSE);
			g_StringLanType(szLan, "ͣ ֹ", "Stop");
			GetDlgItem(IDC_BTN_PLAY)->SetWindowText(szLan);
			return;
		}
	}
	
	m_comboWinNum.EnableWindow(TRUE);
	g_StringLanType(szLan, "�� ��", "Play");
	GetDlgItem(IDC_BTN_PLAY)->SetWindowText(szLan);
}

/*********************************************************
Function:	GetChanItem
Desc:		get channel item and its structure
Input:	iDeviceIndex,device serial number;iChanIndex,channel index;
Output:	
Return:	
**********************************************************/
HTREEITEM CClientDemoDlg::GetChanItem(int iDeviceIndex, int iChanIndex)
{
	HTREEITEM hRoot = m_treeDeviceList.GetRootItem();
	if (hRoot == NULL)
	{
		return NULL;
	}
	int iChanData = 0;
	HTREEITEM hChannel = NULL;
	
	HTREEITEM hDevItem = m_treeDeviceList.GetChildItem(hRoot);
	while (hDevItem != NULL)
	{
		if (DEVICETYPE == (int)m_treeDeviceList.GetItemData(hDevItem)/1000)
		{
			if (iDeviceIndex ==  (int)m_treeDeviceList.GetItemData(hDevItem)%1000)
			{
				hChannel = m_treeDeviceList.GetChildItem(hDevItem);	
				if (hChannel == NULL)
				{
					return NULL;
				}
				iChanData = m_treeDeviceList.GetItemData(hChannel);
				while (iChanData%1000 != iChanIndex)
				{
					hChannel = m_treeDeviceList.GetNextSiblingItem(hChannel);
					if (hChannel == NULL)
					{
						AddLog(iDeviceIndex, OPERATION_FAIL_T, "don't find chanindex[%d] node!!!", iChanIndex);
						return NULL;
					}
					else
					{
						iChanData = m_treeDeviceList.GetItemData(hChannel);
					}
					
				}
				return hChannel;
			}
		}
		hDevItem = m_treeDeviceList.GetNextVisibleItem(hDevItem);
	}//end while
	
	return NULL;
}


/*********************************************************
Function:	GetDeviceItem
Desc:		get device item and its structure
Input:	iDeviceIndex, device index
Output:	
Return:	
**********************************************************/
HTREEITEM CClientDemoDlg::GetDeviceItem(int iDeviceIndex)
{
	HTREEITEM hRoot = m_treeDeviceList.GetRootItem();
	if (hRoot == NULL)
	{
		return NULL;
	}
	HTREEITEM hItem = m_treeDeviceList.GetChildItem(hRoot);
	
	while (hItem)
	{
		if (DEVICETYPE == m_treeDeviceList.GetItemData(hItem)/1000)
		{
			if (iDeviceIndex ==  (int)m_treeDeviceList.GetItemData(hItem)%1000)
			{
				return hItem;
			}
		}//end else if
		
		hItem = m_treeDeviceList.GetNextVisibleItem(hItem);
	}//end while
	
	return NULL;
}

/*********************************************************
Function:	GetCurDeviceIndex
Desc:		check the channel, return device index related to this channel
Input:	
Output:	
Return:	
**********************************************************/
int CClientDemoDlg::GetCurDeviceIndex()
{
	if (m_iCurDeviceIndex < MAX_DEVICES && m_iCurDeviceIndex >= 0)
	{
		return m_iCurDeviceIndex;
	}
	AddLog(-1, OPERATION_SUCC_T, "please select a device at first!");
	return -1;
}

/*********************************************************
Function:	GetCurChanIndex
Desc:		Get current channel serial number.  When channel number < 64, this function is used.
Input:	
Output:	
Return:	
**********************************************************/
int CClientDemoDlg::GetCurChanIndex()
{
    if (m_iCurChanIndex >= 0 && m_iCurChanIndex < 512)
	{
		return m_iCurChanIndex;
	}

	//AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "please select a camera at first!");
	return -1;
} 

int CClientDemoDlg::GetCurChanIndexInTree()
{
    if (m_iCurChanIndex >= 0 && m_iCurChanIndex < 512)
	{	
		return m_iCurChanIndex;
	}
	//AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "please select a camera at first!");
	return -1; 
}

/*********************************************************
Function:	OnBnClickedBtnElse
Desc:		other operation
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnBnClickedBtnElse()
{
	CMenu pMenu;
	CRect rectBtnElse(0,0,0,0);
	GetDlgItem(IDC_BTN_ELSE)->GetWindowRect(&rectBtnElse);
	
	// 	int iDeviceIndex = GetCurDeviceIndex();
	// 	if (iDeviceIndex == -1)
	// 	{
	// 		AfxMessageBox("��ѡ�ж�Ӧ���豸!");
	// 		return;
	// 	}
	
	if (!pMenu.LoadMenu(IDR_MENU_ELSE))
	{
		return;
	}
	
	// 	if (g_struDeviceInfo[iDeviceIndex].iDiskNum > 0)
	// 	{//device local record on disk or other store devices
	// 		pMenu.GetSubMenu(0)->EnableMenuItem(5, MF_BYPOSITION | MF_ENABLED);		
	// 	}
	// 	else
	// 	{
	// 		pMenu.GetSubMenu(0)->EnableMenuItem(5, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	// 	}
	
	pMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN, rectBtnElse.left,rectBtnElse.bottom,this);
}

/*********************************************************
Function:	OnMenuRemoteKey
Desc:		menu on remote key response function
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuRemoteKey()
{
	int iDeviceIndex = GetCurDeviceIndex();
	if (iDeviceIndex == -1)
	{
		return;
	}
	CDlgRemoteKey dlgRemoteKey;
	dlgRemoteKey.m_lServerID = g_struDeviceInfo[iDeviceIndex].lLoginID;
	dlgRemoteKey.DoModal();
}

/*********************************************************
Function:	OnMenuCheckTime
Desc:		menu on time checking response function
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuCheckTime()
{
	CDlgCheckTime dlg;
	dlg.DoModal();
}

/*********************************************************
Function:	OnMenuWorkState
Desc:		get device work sate	
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::OnMenuWorkState()
{
	int iDeviceIndex = GetCurDeviceIndex();
	if (iDeviceIndex == -1)
	{
		return;
	}
	
	CDlgDeviceState dlg;
	dlg.m_lLoginID = g_struDeviceInfo[iDeviceIndex].lLoginID;
	dlg.DoModal();
}

/*********************************************************
Function:	OnMenuLogSearch
Desc:		dvr log search
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::OnMenuLogSearch()
{
	int iDeviceIndex = GetCurDeviceIndex();
	if (iDeviceIndex == -1)
	{
		return;
	}
	CDlgLogSearch dlg;
	dlg.m_lLoginID = g_struDeviceInfo[iDeviceIndex].lLoginID;
	dlg.DoModal();
}

/*********************************************************
Function:	OnMenuDVRRecordStart
Desc:		DVR local record
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuDVRRecordStart()
{
	int iDeviceIndex = GetCurDeviceIndex();
	int iChanIndex = GetCurChanIndex();
	int iChanShowNum = 1;
	int i= 0;
	CString sTemp;
	if (iDeviceIndex == -1)
	{
		return;
	}

	NET_DVR_WORKSTATE_V40 *pStruWorkStateV40 = new NET_DVR_WORKSTATE_V40;
	if (pStruWorkStateV40 == NULL)
	{
		char szLan[128] = {0};
		g_StringLanType(szLan, "�����ڴ�ʧ��", "alloc memory error");
		AfxMessageBox(szLan);
		return;
	}
	
	NET_DVR_GETWORKSTATE_COND struWorkStateCond = {0};
	struWorkStateCond.dwSize =  sizeof(NET_DVR_GETWORKSTATE_COND);
	struWorkStateCond.byFindChanByCond = 0;
	struWorkStateCond.byFindHardByCond = 0;
	
	DWORD dwList = 0;
	
	if (!NET_DVR_GetDeviceConfig(g_struDeviceInfo[iDeviceIndex].lLoginID, NET_DVR_GET_WORK_STATUS,1,\
		&struWorkStateCond,sizeof(NET_DVR_GETWORKSTATE_COND),&dwList,pStruWorkStateV40, sizeof(NET_DVR_WORKSTATE_V40)) ||(dwList != 0))
	{
		AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GET_WORK_STATUS");
		return;
	}

	
	if (iChanIndex == -1)  // All Cameras
	{
		for (i=0; i<MAX_CHANNUM_V40; i++)
		{
			if (!g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bEnable)
			{
				continue;
			}
			
			int iChannelNO = g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChannelNO;
			if (pStruWorkStateV40->struChanStatic[iChannelNO-1].byRecordStatic == 0) //
			{
				if (NET_DVR_StartDVRRecord(g_struDeviceInfo[iDeviceIndex].lLoginID, iChannelNO, 0))
				{
					AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StartDVRRecord %s", g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);
				}	
				else
				{
					AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StartDVRRecord %s", g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);
				}
			}
			else if(pStruWorkStateV40->struChanStatic[iChannelNO-1].byRecordStatic == 1) //
			{
				if (NET_DVR_StopDVRRecord(g_struDeviceInfo[iDeviceIndex].lLoginID, iChannelNO))
				{
					AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StopDVRRecord %s", g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);
				}
				else
				{
					AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StopDVRRecord %s", g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);	
				}
			}
		}
	}
	else
	{
		iChanShowNum = g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].iChannelNO;//iChanIndex+g_struDeviceInfo[iDeviceIndex].iStartChan;
		if (pStruWorkStateV40->struChanStatic[iChanShowNum-1].byRecordStatic == 0)
		{
			if (NET_DVR_StartDVRRecord(g_struDeviceInfo[iDeviceIndex].lLoginID, iChanShowNum, 0))
			{
				AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StartDVRRecord %s", g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].chChanName);
				char szLan[128] = {0};
				g_StringLanType(szLan, "��ʼDVR����¼��ɹ�", "Start Local Record successfully");
				AfxMessageBox(szLan);
			}	
			else
			{
				AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StartDVRRecord %s", g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].chChanName);
			}
		}
		else if (pStruWorkStateV40->struChanStatic[iChanShowNum-1].byRecordStatic == 1)
		{
			if (NET_DVR_StopDVRRecord(g_struDeviceInfo[iDeviceIndex].lLoginID, iChanShowNum))
			{
				AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "NET_DVR_StopDVRRecord %s", g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].chChanName);
				char szLan[128] = {0};
				g_StringLanType(szLan, "ֹͣDVR����¼��ɹ�", "Stop Local Record successfully");
				AfxMessageBox(szLan);
			}
			else
			{
				AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "NET_DVR_StopDVRRecord %s", g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].chChanName);
			}
		}
	}

	
	if (pStruWorkStateV40 != NULL)
	{
		delete pStruWorkStateV40 ;
		pStruWorkStateV40 = NULL;
	}
}

/*********************************************************
Function:	ConfigAllHide
Desc:		hide all configure box
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::ConfigAllHide()
{
	m_dlgConfigAll->ShowWindow(SW_HIDE);
	m_dlgRemoteCfg->ShowWindow(SW_HIDE);	
	m_dlgLocalCfg->ShowWindow(SW_HIDE);
	m_dlgIPAccess->ShowWindow(SW_HIDE);	
	m_dlgHDConfig->ShowWindow(SW_HIDE);
//	m_pStreamIDDlg->ShowWindow(SW_HIDE);
	m_dlgBatchCfg->ShowWindow(SW_HIDE);
	m_dlgProductCfg->ShowWindow(SW_HIDE);
	m_dlgPlayBack->ShowWindow(SW_HIDE);
}

/*********************************************************
Function:	OnCbnSelchangeComboNetdelay
Desc:		set current network delay
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnCbnSelchangeComboNetdelay()
{
	UpdateData(TRUE);
	int iTmp = m_comboNetDelayBufNum.GetCurSel();
	switch (iTmp) 
	{
	case 0:
		g_dlgOutput[m_iCurWndIndex].m_iNetDelayBufNum = 1;
		break;
	case 1:
		g_dlgOutput[m_iCurWndIndex].m_iNetDelayBufNum = 3;
		break;
	case 2:
		g_dlgOutput[m_iCurWndIndex].m_iNetDelayBufNum = 6;
		break;
	case 3:
		g_dlgOutput[m_iCurWndIndex].m_iNetDelayBufNum = 15;
		break;
	default:
		g_dlgOutput[m_iCurWndIndex].m_iNetDelayBufNum= 1;
		break;
	}
}
/*********************************************************
Function:	CyclePlayAllDevicesLogined
Desc:		ѭ�������Ѿ���½���豸
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::CyclePlayAllDevicesLogined()
{
	int i = 0;
	int iPlayNum = 0;//current monitoring points that will be played
	int iFirstIndex = -1;//current first monitoring point index 
	int iDeviceIndex = -1;
	int iChanIndex = -1;
	int iSurveillanceNumLeft = 0;
	
	if (g_nSurveillanceTotalNum<=m_iCurWndNum)
	{
		return;
	}
	
	if (g_nSurveillanceTotalNum == g_iNextSurveillanceIndex)		//turn to first page while cycle to last page
	{
		iPlayNum = m_iCurWndNum;
		iFirstIndex = 0;
	}
	else
	{
		iPlayNum = g_nSurveillanceTotalNum - g_iNextSurveillanceIndex;
		
		if (iPlayNum > m_iCurWndNum)
		{
			iPlayNum = m_iCurWndNum;
		}
		iFirstIndex = g_iNextSurveillanceIndex;
	}
	
	//stop all the windows which is playing
	StopPlayAll();
	
	for (i = 0; i < iPlayNum; i++)
	{
		if (i+iFirstIndex < MAX_PREVIEWS)
		{
			iDeviceIndex = g_struPlayAllInfo[i+iFirstIndex].iDeviceIndex;
			iChanIndex = g_struPlayAllInfo[i+iFirstIndex].iChanIndex;
			g_dlgOutput[i].StartPlay(&g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex], GetChanItem(iDeviceIndex, iChanIndex));
		}
	}
	
	iSurveillanceNumLeft = g_nSurveillanceTotalNum - g_iNextSurveillanceIndex;
	
	if (iSurveillanceNumLeft == 0)//restart afer a cycle
	{
		g_iNextSurveillanceIndex = m_iCurWndNum;
		g_struLocalParam.bFrontPage = FALSE;
		g_struLocalParam.bNextPage = TRUE;
	}
	else if (iSurveillanceNumLeft <= m_iCurWndNum)//last several monitoring points
	{
		g_iNextSurveillanceIndex = g_nSurveillanceTotalNum;
		g_struLocalParam.bNextPage = FALSE;
		g_struLocalParam.bFrontPage = TRUE;
	}	
	else
	{
		g_struLocalParam.bFrontPage = TRUE;
		g_struLocalParam.bNextPage = TRUE;
		g_iNextSurveillanceIndex += m_iCurWndNum;
	}
	
	//UpdateData(FALSE);
}

/*********************************************************
Function:	PlayNextPage
Desc:		preview next pic in play list
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::PlayNextPage()
{
	int iPlayNum = 0;
	int i = 0;
	int iDeviceIndex = -1;
	int iChanIndex = -1;
	int iSurveillanceLeftNum = 0;
	
	if ((g_struLocalParam.bEnlarged)||(g_struLocalParam.bFullScreen))
	{
		if (g_struLocalParam.bFullScreen)
		{
			g_struLocalParam.bFullScreen = FALSE;
			FullScreen(g_struLocalParam.bFullScreen);
		}
		g_struLocalParam.bEnlarged = FALSE;
		ArrangeOutputs(m_iCurWndNum);						
	}
	
	iSurveillanceLeftNum = g_nSurveillanceTotalNum - g_iNextSurveillanceIndex;
	if (iSurveillanceLeftNum > m_iCurWndNum)
	{// left monitoring points surpass current playing windows number
		iPlayNum = m_iCurWndNum;
		g_struLocalParam.bNextPage = TRUE;
	}
	else
	{
		iPlayNum = iSurveillanceLeftNum;
		g_struLocalParam.bNextPage = FALSE;
	}
	
	StopPlayAll();//shut down previous page preview
	
	for (i=0;i<iPlayNum;i++)
	{
		if (i+g_iNextSurveillanceIndex < MAX_PREVIEWS)
		{
			iDeviceIndex = g_struPlayAllInfo[i+g_iNextSurveillanceIndex].iDeviceIndex;
			iChanIndex = g_struPlayAllInfo[i+g_iNextSurveillanceIndex].iChanIndex;
			g_dlgOutput[i].StartPlay(&g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex], GetChanItem(iDeviceIndex, iChanIndex));
			Sleep(100);
		}
	}
	
	g_iNextSurveillanceIndex += iPlayNum;
	g_struLocalParam.bFrontPage = TRUE;
	ChangePlayBtnState();
	UpdateData(FALSE);
}

/*********************************************************
Function:	PlayFrontPage
Desc:		play front group of monitoring points
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::PlayFrontPage()
{
	int iPlayIndex = 0;
	int i = 0;
	int iDeviceIndex = -1;
	int iChanIndex = -1;
	
	if (g_iNextSurveillanceIndex <= m_iCurWndNum || m_iCurWndNum == 0)
	{
		return;
	}
	
	if ((g_struLocalParam.bEnlarged) || (g_struLocalParam.bFullScreen))
	{
		if (g_struLocalParam.bFullScreen)
		{
			g_struLocalParam.bFullScreen = FALSE;
			FullScreen(g_struLocalParam.bFullScreen);
		}
		g_struLocalParam.bEnlarged = FALSE;
		ArrangeOutputs(m_iCurWndNum);	//while windos is zoomed, turn to next pic and restore original window number				
	}
	
	if (g_iNextSurveillanceIndex < 2*m_iCurWndNum)
	{
		iPlayIndex = 0;//if front page is first index, set play index as 0
	}
	else
	{
		i = g_iNextSurveillanceIndex % m_iCurWndNum;
		if (i == 0)
		{
			iPlayIndex = g_iNextSurveillanceIndex - 2*m_iCurWndNum ;
		}
		else
		{
			iPlayIndex = g_iNextSurveillanceIndex - m_iCurWndNum - i;
		}
	}
	
	StopPlayAll();//stop all preview
	
	for (i=0;i<m_iCurWndNum;i++)
	{
		if (i+iPlayIndex < MAX_PREVIEWS)
		{
			iDeviceIndex = g_struPlayAllInfo[i+iPlayIndex].iDeviceIndex;
			iChanIndex = g_struPlayAllInfo[i+iPlayIndex].iChanIndex;
			g_dlgOutput[i].StartPlay(&g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex], GetChanItem(iDeviceIndex, iChanIndex));
			Sleep(100);
		}
	}
	
	if (g_iNextSurveillanceIndex <= 2*m_iCurWndNum)
	{
		g_iNextSurveillanceIndex = m_iCurWndNum;
		g_struLocalParam.bFrontPage = FALSE;		//no front page
	}
	else
	{
		i = g_iNextSurveillanceIndex % m_iCurWndNum;
		if (i == 0)
		{
			g_iNextSurveillanceIndex -= m_iCurWndNum;
		}
		else
		{
			g_iNextSurveillanceIndex -= i;
		}
		g_struLocalParam.bFrontPage = TRUE;
	}
	g_struLocalParam.bNextPage = TRUE;
}

/*********************************************************
Function:	CyclePause
Desc:		suspend or continue cycle
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::CyclePause()
{
	if (!g_struLocalParam.bPaused)
	{	
		g_struLocalParam.bPaused = TRUE;
		if (g_pCycleTimer != NULL)
		{
			KillTimer(CYCLE_PREVIEW_TIMER);
			g_pCycleTimer = NULL;	
		}
	}
	else//continue cycle
	{
		CyclePlayAllDevicesLogined();
		if (g_pCycleTimer == NULL)
		{
			g_pCycleTimer = SetTimer(CYCLE_PREVIEW_TIMER, g_struLocalParam.iCycleTime*1000,NULL);
		}
		g_struLocalParam.bPaused = FALSE;
	}
	ChangePlayBtnState();
}

/*********************************************************
Function:	OnMove
Desc:		dealing with preview and backplay in double screen
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);	
	
	GetWindowRect(&g_rcWindow);
	if (g_rcWindow.left < 0)
	{
		return;
	}
	int iTmpMonitorNum = 0;
	int i=0;
	//#if (WINVER > 0x0400)
	DWORD dwNewDeviceNum = 0;
	LONG lTmpHandle = -1;
	if (((DWORD)g_rcWindow.right + (DWORD)g_rcWindow.left > (DWORD)2 * g_iCurScreenWidth )||((DWORD)g_rcWindow.top + (DWORD)g_rcWindow.bottom > (DWORD)2 * g_iCurScreenHeight))
	{
		dwNewDeviceNum = 1;
	}
	else
	{
		dwNewDeviceNum = 0;	
	}
	if (dwNewDeviceNum != g_dwOldDeviceNum)
	{
		if (m_iMainType == PREVIEW_T)
		{
			int iPlayNum = 0;
			// 			if (g_nSurveillanceTotalNum == 0)
			// 			{
			// 				return;	
			// 			}
			if (g_nSurveillanceTotalNum < m_iCurWndNum)
			{
				iPlayNum = g_nSurveillanceTotalNum;
			}
			else
			{
				iPlayNum = g_nSurveillanceTotalNum;
			}
			iTmpMonitorNum = PlayM4_GetDDrawDeviceTotalNums();
			if (iTmpMonitorNum >= 1)
			{
				for (i=0; i<m_iCurWndNum; i++)
				{
					if (g_dlgOutput[i].m_lPlayHandle>=0)
					{
						LONG lPort = -1;
						if (g_dlgOutput[i].m_lPort >= 0)
						{
							lPort = g_dlgOutput[i].m_lPort;
						}
						else
						{
							lPort = NET_DVR_GetRealPlayerIndex(g_dlgOutput[i].m_lPlayHandle);
						}
						if (lPort < 0)
						{
							return;
						}
						if (PlayM4_SetDDrawDevice(lPort, dwNewDeviceNum + 1))//
						{
							AddLog(m_iCurDeviceIndex, PLAY_SUCC_T, "PlayM4_SetDDrawDevice output[%d]", i);
						}
						else
						{
							AddLog(m_iCurDeviceIndex, PLAY_FAIL_T, "PlayM4_SetDDrawDevice output[%d] err[%d]", i, PlayM4_GetLastError(NET_DVR_GetRealPlayerIndex(g_dlgOutput[i].m_lPlayHandle)));
						}
						
					}			
				}			
			}
			
		}
		else if (m_iMainType == PLAY_BACK_T)
		{
			switch (m_dlgPlayBack->m_iPlayBackType)
			{
			case 0://remote file
				if (m_dlgPlayBack->m_dlgPlayRemoteFile.m_nPlayHandle == -1)
				{
					return;		
				}	
				iTmpMonitorNum = PlayM4_GetDDrawDeviceTotalNums();
				lTmpHandle = NET_DVR_GetPlayBackPlayerIndex(m_dlgPlayBack->m_dlgPlayRemoteFile.m_nPlayHandle);
				AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "lTmpHandle is %d", lTmpHandle);
				PlayM4_SetDDrawDevice(lTmpHandle, dwNewDeviceNum + 1);
				g_dwOldDeviceNum = dwNewDeviceNum;		
				break;
			case 1://local
				iTmpMonitorNum = PlayM4_GetDDrawDeviceTotalNums();
				AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "iTmpMonitorNum is %d", iTmpMonitorNum);
				PlayM4_SetDDrawDevice(99, dwNewDeviceNum + 1);	
				break;
			case 2://play back by time
				if (m_dlgPlayBack->m_dlgPlayRemoteTime.m_lPlayHandle == -1)
				{
					return;
				}
				//nVal = NET_DVR_GetDDrawDeviceTotalNums();
				iTmpMonitorNum = PlayM4_GetDDrawDeviceTotalNums();
				lTmpHandle = NET_DVR_GetPlayBackPlayerIndex(m_dlgPlayBack->m_dlgPlayRemoteTime.m_lPlayHandle);
				AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "lTmpHandle is %d", lTmpHandle);
				//NET_DVR_SetDDrawDevice(temp, dwNewDeviceNum + 1);			
				PlayM4_SetDDrawDevice(lTmpHandle, dwNewDeviceNum + 1);			
				break;
			default:
				break;
			}
		}
		g_dwOldDeviceNum = dwNewDeviceNum;		
	}
	//#endif
	
	if (g_struLocalParam.bUseCard)
	{
		NET_DVR_RefreshSurface_Card();
	}
	UpdateWindow();
}

/*********************************************************
Function:	OnBnClickedBtnConfig
Desc:		configure operation
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnBnClickedBtnConfig()
{
	if (m_iMainType != CONFIG_ALL_T)
	{
		m_iMainType = CONFIG_ALL_T;
		PreviewReferShow(FALSE);
		ConfigAllHide();
		m_dlgPlayBack->HideAll();
		InitConfigAllRefer();
	}
}

/*********************************************************
Function:	InitConfigAllRefer
Desc:		initialize interface dealing related to configuration
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::InitConfigAllRefer()
{
	if (!m_dlgConfigAll->IsWindowVisible())
	{
		CRect rectConfig(0,0,0,0);	
		
		rectConfig.top    = m_iFunBtnHeight-2;
		rectConfig.left   = m_iTreeWidth-2;//-2 cover preview box
		rectConfig.right  = DEMO_FULL_WIDTH - m_iRightWidth ;
		rectConfig.bottom = DEMO_FULL_HEIGHT - m_iListLogHeight;//+4
		
		m_dlgLocalCfg->MoveWindow(&rectConfig,TRUE);
		m_dlgLocalCfg->ShowWindow(SW_HIDE);
		
		m_dlgRemoteCfg->MoveWindow(&rectConfig,TRUE);
		m_dlgRemoteCfg->ShowWindow(SW_SHOW);
		
		m_dlgIPAccess->MoveWindow(&rectConfig,TRUE);
		m_dlgIPAccess->ShowWindow(SW_SHOW);
		
		m_dlgHDConfig->MoveWindow(&rectConfig,TRUE);
		m_dlgHDConfig->ShowWindow(SW_SHOW);

		m_dlgBatchCfg->MoveWindow(&rectConfig,TRUE);
		m_dlgBatchCfg->ShowWindow(SW_SHOW);
		
		m_dlgConfigAll->MoveWindow(&m_rectRightArea,TRUE);
		m_dlgConfigAll->ShowWindow(SW_SHOW);
		m_dlgConfigAll->ConfigWndUpdate();
	}
}

/*********************************************************
Function:	OnBnClickedBtnPlayback
Desc:		switch to backplay interface
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnBnClickedBtnPlayback()
{
	if (m_iMainType != PLAY_BACK_T)
	{
		m_iMainType = PLAY_BACK_T;
		ListRestore();
		PreviewReferShow(FALSE);
		ConfigAllHide();
		InitPlayBackRefer();
	}
}

/*********************************************************
Function:	InitPlayBackRefer
Desc:		initialize backplay interface operation
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::InitPlayBackRefer()
{
	if (!m_dlgPlayBack->IsWindowVisible())
	{
		CRect rectPlayBack(0,0,0,0);	
		
		rectPlayBack.top    = m_iFunBtnHeight-2;
		rectPlayBack.left   = m_iTreeWidth-2;//-2 cover preview box
		rectPlayBack.right  = DEMO_FULL_WIDTH;
		rectPlayBack.bottom = DEMO_FULL_HEIGHT - m_iListLogHeight;//+4
		
		m_dlgPlayBack->MoveWindow(&rectPlayBack,TRUE);
		m_dlgPlayBack->PlayBackWinUpdate();
		m_dlgPlayBack->ShowWindow(SW_SHOW);
	}
}

/*********************************************************
Function:	OnBnClickedBtnRecord
Desc:		manually record uner preview interface
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnBnClickedBtnRecord()
{
	if (g_struLocalParam.bAutoRecord)
	{
		OnTimer(AUTORECORD_TIMER);
		return;
	}
	
	if ((g_dlgOutput[m_iCurWndIndex].m_lPlayHandle >= 0) && (!g_dlgOutput[m_iCurWndIndex].m_bLocalManualRec))
	{
		if (MakeRecDir())
		{
			g_dlgOutput[m_iCurWndIndex].StartLocalRecord();	
		}
	}
	else if (g_dlgOutput[m_iCurWndIndex].m_bLocalManualRec)
	{
		g_dlgOutput[m_iCurWndIndex].StopLocalRecord();
		g_dlgOutput[m_iCurWndIndex].m_bSaveHead = FALSE;
	}
}

/*********************************************************
Function:	MakeRecDir
Desc:		create record directory
Input:	
Output:	
Return:	TRUE,create successful;FALSE,create failed;
**********************************************************/
BOOL CClientDemoDlg::MakeRecDir()
{
	CString sTemp;
	DWORD dwDirAttr;
	CTime time = CTime::GetCurrentTime();
	char szLan[256] = {0};
	//m_iCurRecDriver = g_pMainDlg->m_cHardDriver[g_struLocalParam.iStartRecordDriver];
	sTemp.Format("%c:\\mpeg4record\\", m_iCurRecDriver);
	dwDirAttr = GetFileAttributes(sTemp);
	if ((dwDirAttr == -1) || !(dwDirAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
		if (!CreateDirectory(sTemp, NULL))
		{
			g_StringLanType(szLan, "����������Ŀ¼ʧ��", "create year-month-day directory failed");
			AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, szLan);
			AfxMessageBox(szLan);
			return FALSE;
		}	
	}
	else
	{
		sTemp.Format("%c:\\mpeg4record\\%4d-%02d-%02d\\", m_iCurRecDriver, time.GetYear(), time.GetMonth(), time.GetDay());
		dwDirAttr=GetFileAttributes(sTemp);
		if ((dwDirAttr == -1) || !(dwDirAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (!CreateDirectory(sTemp, NULL))
			{
				g_StringLanType(szLan, "����������Ŀ¼ʧ��", "create year-month-day directory failed");
				AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, szLan);
				AfxMessageBox(szLan);
				return FALSE;
			}
			else
			{
				sprintf(g_struLocalParam.chClientRecordPath, "%s", sTemp.GetBuffer(0));
				AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "create %c:\\mpeg4record\\%4d-%02d-%02d", m_iCurRecDriver, time.GetYear(), time.GetMonth(), time.GetDay());
				return TRUE;
			}
		}
		else
		{
			sprintf(g_struLocalParam.chClientRecordPath, "%s", sTemp.GetBuffer(0));
			AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "create directory:%s", sTemp.GetBuffer(0));
			return TRUE;	
		}
	}
	
	sTemp.Format("%c:\\mpeg4record\\%4d-%02d-%02d\\",m_iCurRecDriver, time.GetYear(), time.GetMonth(), time.GetDay());
	if (!CreateDirectory(sTemp, NULL))
	{
		return FALSE;
	}
	sprintf(g_struLocalParam.chClientRecordPath, "%s", sTemp.GetBuffer(0));
	AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "create %c:\\mpeg4record first, then create %4d-%02d-%02d", m_iCurRecDriver, time.GetYear(), time.GetMonth(), time.GetDay());
	return TRUE;
}

/*********************************************************
Function:	IsDiskFull
Desc:		check disk full
Input:	
Output:	
Return:	TRUE,disk is full;FALSE,disk is not full
**********************************************************/
BOOL CClientDemoDlg::IsDiskFull()
{
	int i;
	ULARGE_INTEGER FreeSpace,CallerSpace,TotalSpace;
	CString csRecRootPath;	
	
	for (i = g_struLocalParam.iStartRecordDriver; i < g_struLocalParam.iEndRecordDriver + 1; i++)
	{	
		csRecRootPath.Format("%c%s",m_cHardDriver[i],":\\");	//start from hard driver
		GetDiskFreeSpaceEx(csRecRootPath,&CallerSpace,&TotalSpace,&FreeSpace);
		if (CallerSpace.QuadPart > MINSPACE)
		{
			m_iCurRecDriver = m_cHardDriver[i];
			g_struLocalParam.bAllDiskFull = FALSE;
			return	FALSE;
		}
	}
	g_struLocalParam.bAllDiskFull = TRUE;
	return TRUE;
}

/*********************************************************
Function:	DeleteClientRecordFile
Desc:		delete local record file
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::DeleteClientRecordFile()
{
	CFileFind cFile,cFileMp4;
	BOOL bFineDir,bFileFind;
	CString sDeleteDir,sTempDelete;
	CString sDir;
	CString sDate;
	CString sFileName;
	CTime time,temptime;
	
	int i = 0;
	//	int iDelNum = 0;
	sDeleteDir.Format("0000-00-00");
	sTempDelete.Format("3000-00-00");
	
	time=CTime::GetCurrentTime();
	
	for (i = g_struLocalParam.iStartRecordDriver; i < (g_struLocalParam.iEndRecordDriver + 1); i++)
	{
		sDir.Format("%c:\\mpeg4record\\*.*",m_cHardDriver[i]);
		bFineDir=cFile.FindFile(sDir);//judge whether the current directory has files
		while (bFineDir)
		{
			bFineDir=cFile.FindNextFile();
			if (cFile.IsDots())//'.'mean current directory, & '..' means up directory.
			{
				continue;
			}
			if (cFile.IsDirectory())
			{
				if (cFile.GetFileName()==_T("download"))//not download record
				{
					continue;
				}
				if (cFile.GetFileName()==_T("alarm"))//not alarm record
				{
					continue;
				}
				sFileName.Format("%c:\\mpeg4record\\%s\\*.mp4", m_cHardDriver[i], cFile.GetFileName().GetBuffer(0));
				bFileFind=cFileMp4.FindFile(sFileName);
				if (!bFileFind)
				{
					RemoveDirectory(cFile.GetFilePath());
					AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, cFile.GetFilePath().GetBuffer(0));
				}
				while (bFileFind)
				{
					bFileFind=cFileMp4.FindNextFile();
					if (cFile.IsDots())
					{
						continue;
					}
					cFileMp4.GetCreationTime(temptime);
					if (temptime<time)
					{
						time=temptime;
						sTempDelete.Format("%s",cFileMp4.GetFilePath().GetBuffer(0));
					}
				}
			}			
		}
	}
	sFileName.Format("%s",sTempDelete.GetBuffer(0));
	sTempDelete.Format("delete[%s]",sFileName.GetBuffer(0));
	if (!DeleteFile(sFileName))
	{
		AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T,sTempDelete.GetBuffer(0));
		for (i = 0; i < MAX_OUTPUTS; i++)
		{
			if ((g_dlgOutput[i].m_lPlayHandle >= 0) || (g_dlgOutput[i].m_bLocalManualRec))
			{
				g_dlgOutput[i].StopLocalRecord();
			}
		}
		char szLan[128] = {0};
		g_StringLanType(szLan, "Ӳ���������㣬�Զ�ɾ���ļ�ʧ�ܣ�������Ӳ�̺����¿�ʼ¼��",\
			"Lack of hard drive capacity, automatic delete file failed, please clean up after the hard disk to start record!");
		AfxMessageBox(szLan);
	}
	else
	{
		AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, sTempDelete.GetBuffer(0));
		//iDelNum++;
	}
}

/*********************************************************
Function:	OnBnClickedBtnManage
Desc:		advanded management operantion
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnBnClickedBtnManage()
{
	CMenu pMenu;
	CRect cRectBtnManage(0,0,0,0);
	//GetCursorPos(&pt);
	GetDlgItem(IDC_BTN_MANAGE)->GetWindowRect(&cRectBtnManage);
	//select first node or blank
	
	if (!pMenu.LoadMenu(IDR_MENU_MANAGE))
	{
		return;
	}
	
	pMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN,cRectBtnManage.left ,cRectBtnManage.bottom,this);
}

/*********************************************************
Function:	OnMenuUpgrade
Desc:		device upgrade
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuUpgrade()
{
	int iDeviceIndex = GetCurDeviceIndex();
	if (iDeviceIndex == -1)
	{
		return;
	}
	
	
    int iChanIndex = GetCurChanIndex();
    if (iChanIndex < 0)
    {
        iChanIndex = 0;
    }

	CDlgUpgrade dlgUpgrade;
	dlgUpgrade.m_lServerID = g_struDeviceInfo[iDeviceIndex].lLoginID;
	dlgUpgrade.m_lChannel = iChanIndex + g_struDeviceInfo[iDeviceIndex].iStartChan;
	dlgUpgrade.m_iDeviceIndex = iDeviceIndex;
	dlgUpgrade.DoModal();
}

/*********************************************************
Function:	OnMenuReboot
Desc:		device reboot
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuReboot()
{
	int iDeviceIndex = GetCurDeviceIndex();
	int i = 0;
	char szLan[128] = {0};
	
	if (iDeviceIndex == -1)
	{
		return;
	}
	g_StringLanType(szLan, "ȷ��Ҫ�����豸?", "Sure to reboot device?");
	if (IDOK!=MessageBox(szLan, "Warning",IDOK))
	{
		return;
	}
	if (NET_DVR_RebootDVR(g_struDeviceInfo[iDeviceIndex].lLoginID))
	{
		g_struDeviceInfo[iDeviceIndex].bAlarm = FALSE;
		g_struDeviceInfo[iDeviceIndex].bPlayDevice = FALSE;
		g_struDeviceInfo[iDeviceIndex].bVoiceTalk = FALSE;
		//		g_struDeviceInfo[iDeviceIndex].dwImageType = DEVICE_LOGOUT;
		
		for (i = 0; i<MAX_CHANNUM_V30; i++)
		{
			g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = FALSE;
		}
		AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_RebootDVR");
		g_StringLanType(szLan, "�豸��������", "Device is rebooting");
		AfxMessageBox(szLan);
	}
	else
	{
		AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_RebootDVR");
		g_StringLanType(szLan, "�豸����ʧ��", "Reboot device failed");
		AfxMessageBox(szLan);	
	}
}

/*********************************************************
Function:	OnMenuHDFormat
Desc:		disk format
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuHDFormat()
{
	int iDeviceIndex = GetCurDeviceIndex();
	if (iDeviceIndex == -1)
	{
		return;
	}
	CDlgFormatHD dlgFormatDisk;
	dlgFormatDisk.m_iDeviceIndex = iDeviceIndex;
	dlgFormatDisk.m_dwDiskNum = g_struDeviceInfo[iDeviceIndex].iDiskNum;
	dlgFormatDisk.DoModal();
}

/*********************************************************
Function:	OnMenuPoweroff
Desc:		power off
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuPoweroff()
{
	
	int iDeviceIndex = GetCurDeviceIndex();
	char szLan[128] = {0};
	
	if (iDeviceIndex == -1)
	{
		return;
	}
	g_StringLanType(szLan, "ȷ��Ҫ�ر��豸?", "Sure to shut down device?");
	if (IDOK!=MessageBox(szLan, "Warning",IDOK))
	{
		return;
	}
	
	if (NET_DVR_ShutDownDVR(g_struDeviceInfo[iDeviceIndex].lLoginID))
	{
		AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_ShutDownDVR");
		g_StringLanType(szLan, "�豸���ڹػ�", "Device is shutting down");
		AfxMessageBox(szLan);
	}
	else
	{
		AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_ShutDownDVR");
		g_StringLanType(szLan, "�豸�ػ�ʧ��", "Shut down device failed");
		AfxMessageBox(szLan);
	}
}

/*********************************************************
Function:	OnMenuFortify
Desc:		switch between setting guard and removing guard 
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuFortify()
{
	CDlgFortify dlg;
	int i = 0;
	dlg.DoModal();
	if (dlg.m_bFortify)
	{
		for (i=0; i<MAX_DEVICES; i++)
		{
			if (g_struDeviceInfo[i].lLoginID >= 0)
			{
				::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_DEVICE_ITEM_IMAGE, WPARAM(GetDeviceItem(i)), LPARAM(i));
			}		
		}
	}
	
}

/*********************************************************
Function:	OnMenuVoiceTalk
Desc:		start or stop voice talk
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuVoiceTalk()
{
	int iDeviceIndex = GetCurDeviceIndex();
	if (iDeviceIndex == -1)
	{
		return;
	}
	CDlgVoiceTalk dlg;
	dlg.m_dwDeviceIndex = iDeviceIndex;
	dlg.DoModal();
}

/*********************************************************
Function:	OnMenuSerialTrans
Desc:		transport the data on serial ports
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::OnMenuSerialTrans()
{
	int iDeviceIndex = GetCurDeviceIndex();
	if (iDeviceIndex == -1)
	{
		return;
	}
	CTransparentTrans dlg;
	dlg.m_iDeviceIndex = iDeviceIndex;
	dlg.DoModal();
}

/*********************************************************
Function:	OnMenuDefaultConfig
Desc:		restore the default configure of the device
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::OnMenuDefaultConfig()
{
	int iDeviceIndex = GetCurDeviceIndex();
	char szLan[128] = {0};
	
	if (iDeviceIndex == -1)
	{
		return;
	}
	g_StringLanType(szLan, "ȷ��Ҫ�ָ�Ĭ�ϲ���?", "Sure to restore the default parameters?");
	if (IDOK!=MessageBox(szLan, "Warning",IDOK))
	{
		return;
	}
	
	
	if (g_struDeviceInfo[iDeviceIndex].lLoginID<0)
	{
		return;
	}
	
	if (NET_DVR_RestoreConfig(g_struDeviceInfo[iDeviceIndex].lLoginID))
	{
		AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_RestoreConfig");
		g_StringLanType(szLan, "�ָ�Ĭ��ֵ�ɹ���������", "Restore the default parameters successfully");
		AfxMessageBox(szLan);//DS9000 restore configure, reboot auto
	}
	else
	{
		AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_RestoreConfig");
		g_StringLanType(szLan, "�ָ�Ĭ��ֵʧ��", "Restore the default parameters failed");
		AfxMessageBox(szLan);	
	}
}


/*********************************************************
Function:	OnMenuConfigFlash
Desc:		set or get the configure file
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::OnMenuConfigFlash()
{
	CDlgConfigFlash dlg;
	dlg.DoModal();
}

/*********************************************************
Function:	OnMenuImExportIPCCfgFile
Desc:		set or get the configure file
Input:	none
Output:	none
Return:	none
**********************************************************/
// void CClientDemoDlg::OnMenuImExportIPCCfgFile()
// {
// 	DlgImexporIPCCfgFile dlg;
// 	dlg.DoModal();
// }

/*********************************************************
Function:	OnMenuTalkMR
Desc:		voice media redirect
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::OnMenuTalkMR()
{
	CDlgTalkMREx dlg;
	int iDevIndex = GetCurDeviceIndex();
	if (iDevIndex < 0)
	{
		return;
	}
	dlg.m_iDeviceIndex = iDevIndex;
	dlg.DoModal();
}

/*********************************************************
Function:	OnBnClickedBtnPictureSnatch
Desc:		capture pic in bmp under preview
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::PictureSnatchInSide()
{
	int i = m_iCurWndIndex;
	
	if (g_dlgOutput[i].m_lPlayHandle < 0)
	{
		return;
	}
	char cFilename[256];
	CString sTemp;
	CTime time = CTime::GetCurrentTime();
	sTemp.Format("%s\\", g_struLocalParam.chPictureSavePath);		
	DWORD dwRet = GetFileAttributes(sTemp);
	if ((dwRet == -1) || !(dwRet & FILE_ATTRIBUTE_DIRECTORY))
	{
		CreateDirectory(sTemp, NULL);
	}
	PCHANNEL_INFO pChanInfo = &(g_dlgOutput[i].m_struChanInfo);
	sprintf(cFilename, "%s\\Preview_%s_%02d_%4d%02d%02d_%02d%02d%02d_%d.bmp", \
		g_struLocalParam.chPictureSavePath, g_struDeviceInfo[pChanInfo->iDeviceIndex].chDeviceIPInFileName, \
		g_struDeviceInfo[pChanInfo->iDeviceIndex].iStartChan+pChanInfo->iChanIndex,	\
		time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond(), GetTickCount());
	if (g_struLocalParam.bUseCard)
	{			
		if (NET_DVR_CapturePicture_Card(g_dlgOutput[i].m_lPlayHandle, cFilename))
		{
			CString sTemp;	
			sTemp.Format("Capture picture succeed %s!",cFilename);
			AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, sTemp.GetBuffer(0));
			return;
		}
		else
		{
			AfxMessageBox("Failed to capture picture!");
		}
	}
	else
	{
		if (NET_DVR_CapturePicture(g_dlgOutput[i].m_lPlayHandle, cFilename))
		{
			AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "NET_DVR_CapturePicture[%s]", cFilename);
		}
		else
		{
			AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "NET_DVR_CapturePicture[%s]", cFilename);
			char szLan[128] = {0};
			g_StringLanType(szLan, "bmpץͼʧ��", "bmp snatch failed");
			AfxMessageBox(szLan);
		}
	}
}

void CClientDemoDlg::OnBnClickedBtnPictureSnatch()
{
	if (g_bInSide)
	{
		PictureSnatchInSide();
		return;
	}
	if (g_struLocalParam.bUseCard)
	{
		g_dlgOutput[m_iCurWndIndex].CapturePicture();
	}
	
	g_bCapPic[m_iCurWndIndex] = TRUE;
}

/*********************************************************
Function:	OnMenuBroadCast
Desc:		audio broadcast deal
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnMenuBroadCast()
{
	CBroadCast dlg;
	dlg.DoModal();
}

/*********************************************************
Function:	OnBnClickedBtnLogClear
Desc:		clear up all log items in list
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnBnClickedBtnLogClear()
{
	char szLan[128] = {0};
	g_StringLanType(szLan, "ȷ������б���?", "Sure to clear list?");
	if (IDOK==MessageBox(szLan,"Warning",IDOK))
	{
		if (m_iSelListType == 1)
		{
			m_listAlarmInfo.DeleteAllItems();
			for (int i = 0; i< MAX_DEVICES; i++)
			{
				if (g_struDeviceInfo[i].lFortifyHandle >= 0)
				{
					ManualClearAlarm(i, -1);
				}
			}
		}
		else
		{
			m_listAllLog.DeleteAllItems();
		}	
	}
}

/*********************************************************
Function:	OnBnClickedBtnLogSave
Desc:		save all the log items in list
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnBnClickedBtnLogSave()
{
	CString csFileName;
	CTime time;
	if (g_struLocalParam.bAutoSaveLog)
	{
		return;
	}
	int i;
	CString sTemp;
	CStdioFile myFile;
	
	int iCount = 0;
	
	time = CTime::GetCurrentTime();
	
	if (m_iSelListType == 1)
	{
		iCount = m_listAlarmInfo.GetItemCount();
		csFileName.Format("%s\\AlarmInfo_%4d%02d%02d_%02d%02d%02d.txt",\
			g_struLocalParam.chRemoteCfgSavePath, time.GetYear(), time.GetMonth(),\
			time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());
	}
	else
	{
		iCount = m_listAllLog.GetItemCount();
		csFileName.Format("%s\\LogFile_%4d%02d%02d_%02d%02d%02d.txt",\
			g_struLocalParam.chRemoteCfgSavePath, time.GetYear(), time.GetMonth(),\
			time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());
	}
	if (iCount <=  0)
	{
		return;
	}
	if (myFile.Open(csFileName, CFile::modeCreate|CFile::modeWrite) == FALSE)
	{
		char szLan[128] = {0};
		g_StringLanType(szLan, "�ļ�����ʧ��", "create file failed");
		AfxMessageBox(szLan);
		return;
	}
	for (i = 0; i < iCount; i++)
	{
		if (m_iSelListType == 1)
		{
			sTemp.Format("%s\t%s\n", m_listAllLog.GetItemText(i, 1).GetBuffer(0),\
				m_listAllLog.GetItemText(i, 2).GetBuffer(0));
		}
		else
		{
			sTemp.Format("%s\t%s\t%s\t%s\r\n", m_listAllLog.GetItemText(i, 1).GetBuffer(0),\
				m_listAllLog.GetItemText(i, 2).GetBuffer(0), m_listAllLog.GetItemText(i, 3).GetBuffer(0),\
				m_listAllLog.GetItemText(i, 4).GetBuffer(0));
		}
		
		myFile.WriteString(sTemp);
	}
	myFile.Close();
}

/*********************************************************
Function:	ManualClearAlarm
Desc:		clear alarm manually
Input:	iDeviceIndex,device index; iChanIndex,channel index, -1 clear all channel;
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::ManualClearAlarm(int iDeviceIndex, int iChanIndex)
{
	CString csTemp;
	int i=0;
	if (iChanIndex == -1)
	{//select device node
		//give out hints while switch alarm logo
		if (g_struDeviceInfo[iDeviceIndex].bAlarm)
		{
			g_struDeviceInfo[iDeviceIndex].bAlarm = FALSE;
			::PostMessage(m_hWnd, WM_CHANGE_DEVICE_ITEM_IMAGE, WPARAM(GetDeviceItem(iDeviceIndex)), LPARAM(iDeviceIndex));		
		}
		
		for (i=0; i<MAX_CHANNUM_V30; i++)
		{
			if (g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bEnable && g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm)
			{
				g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = FALSE;
				AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "ch[%d] clear alarm status", i+g_struDeviceInfo[iDeviceIndex].iStartChan);
				::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));	
			}
		}
	}
	else
	{
		if (g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].bAlarm)
		{
			g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].bAlarm = FALSE;
			AddLog(m_iCurDeviceIndex, OPERATION_SUCC_T, "ch[%d] clear alarm status", iChanIndex+g_struDeviceInfo[iDeviceIndex].iStartChan);
			::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(m_iCurDeviceIndex), LPARAM(iChanIndex));		
		}
	}
}

/*********************************************************
Function:	OnBnClickedBtnLogConfig
Desc:		local log configuration
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnBnClickedBtnLogConfig()
{
	CDlgLocalLogConfig dlg;
	dlg.DoModal();
}

/*********************************************************
Function:	IsCurDevMatDec
Desc:		check current device is decoder
Input:	
Output:	
Return:	
**********************************************************/
BOOL CClientDemoDlg::IsCurDevMatDec(int iDeviceIndex)
{
	BOOL bRet = FALSE;
	if (iDeviceIndex != -1)
	{
		if (DEC == g_struDeviceInfo[iDeviceIndex].iDeviceType||DEC_MAT == g_struDeviceInfo[iDeviceIndex].iDeviceType || \
			DS_6504D_B10 == g_struDeviceInfo[iDeviceIndex].iDeviceType||DS630X_D == g_struDeviceInfo[iDeviceIndex].iDeviceType
			|| DS640X_HD == g_struDeviceInfo[iDeviceIndex].iDeviceType \
			|| DS_B10_XY == g_struDeviceInfo[iDeviceIndex].iDeviceType || DS610X_D == g_struDeviceInfo[iDeviceIndex].iDeviceType) 
		{
			bRet = TRUE;
		}
	}
	return TRUE;
}

/*********************************************************
Function:	IsNeedRebootDevType
Desc:		some devices(8-ch,16-ch 7000,8000HS_S)need to reboot after modifying resolution
Input:	
Output:	
Return:	
**********************************************************/
BOOL CClientDemoDlg::IsNeedRebootDevType(void)
{
	int iDevIndex = GetCurDeviceIndex();
	BOOL bRet = FALSE;
	if (iDevIndex != -1)
	{
		if ((DVR_HS_S == g_struDeviceInfo[iDevIndex].iDeviceType)\
			||(( DVR_7000H== g_struDeviceInfo[iDevIndex].iDeviceType) && (g_struDeviceInfo[iDevIndex].iDeviceChanNum == 8))\
			||(( DVR_7000H== g_struDeviceInfo[iDevIndex].iDeviceType) && (g_struDeviceInfo[iDevIndex].iDeviceChanNum == 16)))
		{
			bRet = TRUE;
		}
	}
	return bRet;
}

/*********************************************************
Function:	OnNMClickListAllLog
Desc:		Copy the contents to the clipboard
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnNMClickListAllLog(NMHDR *pNMHDR, LRESULT *pResult)
{
	int iItemSel = 0;
	CString csTxt;
	
	POSITION  iPos = m_listAllLog.GetFirstSelectedItemPosition();
	if (iPos == NULL)
	{
		return;
	}
	iItemSel = m_listAllLog.GetNextSelectedItem(iPos);
	csTxt.Format("%s",m_listAllLog.GetItemText(iItemSel,3));
	if (OpenClipboard())   
	{   
		HGLOBAL   clipbuffer;   
		char   *   buffer;   
		EmptyClipboard();   
		clipbuffer   =   GlobalAlloc(GMEM_DDESHARE,  csTxt.GetLength()+1);   
		buffer   =   (char*)GlobalLock(clipbuffer);   
		strcpy(buffer,   LPCSTR(csTxt));   
		GlobalUnlock(clipbuffer);   
		SetClipboardData(CF_TEXT,clipbuffer);   
		CloseClipboard();   
	}   
	*pResult = 0;
}

/*********************************************************
Function:	PreTranslateMessage
Desc:		capture push enter button or esc button
Input:	none
Output:	none
Return:	none
**********************************************************/
BOOL CClientDemoDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_ESCAPE)
	{
		if (g_struLocalParam.bFullScreen)
		{
			g_struLocalParam.bEnlarged    = FALSE;
			g_struLocalParam.bMultiScreen = FALSE;
			g_struLocalParam.bFullScreen  = FALSE;
			g_pMainDlg->GetDlgItem(IDC_COMBO_WNDNUM)->EnableWindow(TRUE);
			g_pMainDlg->FullScreen(g_struLocalParam.bFullScreen);
			if (g_struLocalParam.bFullScreen)
			{
				g_pMainDlg->ArrangeOutputs(g_pMainDlg->m_iCurWndNum);
			}
			else//multi-pic zoom
			{
				g_pMainDlg->ArrangeOutputs(g_pMainDlg->m_iCurWndNum);
				
				g_pMainDlg->GetDlgItem(IDC_STATIC_PREVIEWBG)->Invalidate(TRUE);
			}
		}
		else
		{
			GetDlgItem(IDC_BTN_TEST)->ShowWindow(SW_HIDE);
		}
		return TRUE;
	}
	if (pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN)
	{
		GetDlgItem(IDC_BTN_TEST)->ShowWindow(SW_SHOW);
		return TRUE; 
	}
	else
	{
		return CDialog::PreTranslateMessage(pMsg);
	}
}

/*********************************************************
Function:	OnBnClickedBtnExit
Desc:		leave response function, release resource and exit demo
Input:	
Output:	
Return:	
**********************************************************/
void CClientDemoDlg::OnBnClickedBtnExit()
{
	char szLan[128] = {0};
	g_bExitDemo = TRUE;//make not post message
	g_StringLanType(szLan, "ȷ��Ҫ�˳���?", "Sure to exit?");
	if (IDOK != MessageBox(szLan,"Warning",IDOK))
	{
		return;
	}
	StopPlayAll();//stop all previews
	m_dlgPlayBack->FinishAll();//stop all play back
	if (m_pCyclePlayTimer)
	{
		KillTimer(CYCLE_PREVIEW_TIMER); 
		m_pCyclePlayTimer = 0;
	}
	if (m_pFreeSpaceTimer)
	{
		KillTimer(FREESPACE_TIMER);
		m_pFreeSpaceTimer = 0;
	}
	if (m_pFileIntervalTimer)
	{
		KillTimer(FILEINTERVAL_TIMER);
		m_pFileIntervalTimer = 0;
	}
	if (m_pChkDevTimer)
	{
		KillTimer(CHECK_DEVICE_TIMER);
		m_pChkDevTimer = 0;
	}
	if (m_pSysTimer)
	{
		KillTimer(SYSTEM_TIMER);
		m_pSysTimer=0;
	}
	
	if (g_struLocalParam.bUseCard)
	{
		NET_DVR_ReleaseDevice_Card();
		NET_DVR_ReleaseDDraw_Card();
	}
	
	if (m_lListenHandle > 0)
	{
		NET_DVR_StopListen_V30(m_lListenHandle); 
	}
	
	if (g_struLocalParam.bBroadCast)
	{
		NET_DVR_ClientAudioStop();
	}
	
	//remove guard, log out
	int i = 0, j = 0;
	for (i=0; i<MAX_DEVICES; i++)  
	{
		if (g_struDeviceInfo[i].lLoginID >= 0)
		{
			for (j=0; j<MAX_AUDIO_V40; j++)
			{
				if (g_struDeviceInfo[i].lVoiceCom[j] >= 0)
				{
					if (NET_DVR_StopVoiceCom(g_struDeviceInfo[i].lVoiceCom[j]))
					{
						g_struDeviceInfo[i].lVoiceCom[j] = -1;
					}
				}			
			}
			
			if (g_struDeviceInfo[i].lFortifyHandle >= 0)
			{
				if (NET_DVR_CloseAlarmChan(g_struDeviceInfo[i].lFortifyHandle))
				{
					g_struDeviceInfo[i].lFortifyHandle = -1;
				}
			}
			
			NET_DVR_Logout_V30(g_struDeviceInfo[i].lLoginID);
			
			g_struDeviceInfo[i].lLoginID = -1;
		}
	}

	//2011-11-16�˳���������
	int iPassiveChanIndex = 0;
	CString csStr;
	for (i=0; i<MAX_DEVICES; i++)
	{
		//64-T��������������ͨ��Ϊ32��
		for (int iPassiveChanIndex=0; iPassiveChanIndex<32; iPassiveChanIndex++)
		{
			if (g_struDeviceInfo[i].struPassiveDecode[iPassiveChanIndex].lRealHandle >= 0)
			{
				NET_DVR_SetRealDataCallBack(g_struDeviceInfo[i].struPassiveDecode[iPassiveChanIndex].lRealHandle, NULL, iPassiveChanIndex);
				if (!NET_DVR_StopRealPlay(g_struDeviceInfo[i].struPassiveDecode[iPassiveChanIndex].lRealHandle))
				{
					g_pMainDlg->AddLog(i, OPERATION_FAIL_T, "NET_DVR_StopRealPlay FAILED");
				}
				else
				{
					g_pMainDlg->AddLog(i, OPERATION_SUCC_T, "NET_DVR_StopRealPlay SUCC");
					g_struDeviceInfo[i].struPassiveDecode[iPassiveChanIndex].lRealHandle = -1;
				}
			}
			if (g_struDeviceInfo[i].struPassiveDecode[iPassiveChanIndex].lUserID >= 0)
			{
				if (!NET_DVR_Logout(g_struDeviceInfo[i].struPassiveDecode[iPassiveChanIndex].lUserID))
				{
					g_pMainDlg->AddLog(i, OPERATION_FAIL_T, "NET_DVR_Logout FAILED");
				}
				else
				{
					g_pMainDlg->AddLog(i, OPERATION_SUCC_T, "NET_DVR_Logout SUCC");
					g_struDeviceInfo[i].struPassiveDecode[iPassiveChanIndex].lUserID = -1;
				}
			}  
			if (g_struDeviceInfo[i].struPassiveDecode[iPassiveChanIndex].hFileThread != NULL)
			{
				SetEvent(g_struDeviceInfo[i].struPassiveDecode[iPassiveChanIndex].hExitThread);
				WaitForSingleObject(g_struDeviceInfo[i].struPassiveDecode[iPassiveChanIndex].hThreadExit, INFINITE);
				TRACE("### Wait for Thread Exit...\n");
			}
			if (g_struDeviceInfo[i].struPassiveDecode[iPassiveChanIndex].lPassiveHandle >= 0)
			{
				if (!NET_DVR_MatrixStopPassiveDecode(g_struDeviceInfo[i].struPassiveDecode[iPassiveChanIndex].lPassiveHandle))
				{
					g_pMainDlg->AddLog(i, OPERATION_FAIL_T, "NET_DVR_MatrixStopPassiveDecode FAILED");
					//return;
				}
				else
				{
					g_pMainDlg->AddLog(i, OPERATION_SUCC_T, "NET_DVR_MatrixStopPassiveDecode SUCC");
					g_struDeviceInfo[i].struPassiveDecode[iPassiveChanIndex].lPassiveHandle = -1; 
				}   
			} 
		}
		
	}

	g_SaveDeviceConfig();
	i = 0;
	
	CDialog::OnCancel();
}

/*********************************************************
Function:	OnCancel
Desc:		shutdown button response deal
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::OnCancel()
{
	OnBnClickedBtnExit();
}

/*********************************************************
Function:	DoGetDeviceResoureCfg
Desc:		get input device IP parameters
Input:	iDeviceIndex, the device index
Output:	none
Return:	TRUE/FALSE
**********************************************************/
BOOL CClientDemoDlg::DoGetDeviceResoureCfg(int iDeviceIndex, int iGroupNO)
{
	int i = 0,j = 0;
	CString csTemp;
	LPNET_DVR_IPPARACFG_V40 lpIPAccessCfgV40 = &g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[iGroupNO];
 	DWORD dwReturned = 0;
	DWORD dwLoopTime = 0;

    
    if (g_struDeviceInfo[iDeviceIndex].iIPChanNum >0)
    {
        if (g_struDeviceInfo[iDeviceIndex].iIPChanNum% MAX_CHANNUM_V30 == 0)
        {
            dwLoopTime = g_struDeviceInfo[iDeviceIndex].iIPChanNum/MAX_CHANNUM_V30 ;
        }
        else
        {
            dwLoopTime = g_struDeviceInfo[iDeviceIndex].iIPChanNum/MAX_CHANNUM_V30 + 1;
        }
    }
	
    for (j = 0; j <  dwLoopTime; j++)
    {
        iGroupNO = j;
        lpIPAccessCfgV40 = &g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[iGroupNO];
        //memset(lpIPAccessCfgV40, 0, sizeof(NET_DVR_IPPARACFG_V40));
        
        //2008-9-15 13:44 ip input configuration
        g_struDeviceInfo[iDeviceIndex].bIPRet = NET_DVR_GetDVRConfig(g_struDeviceInfo[iDeviceIndex].lLoginID, NET_DVR_GET_IPPARACFG_V40, iGroupNO, &g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[iGroupNO], sizeof(NET_DVR_IPPARACFG_V40), &dwReturned);
        if (!g_struDeviceInfo[iDeviceIndex].bIPRet)
        {	///device no support ip access
            g_struDeviceInfo[iDeviceIndex].lFirstEnableChanIndex = 0;
            g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GET_IPPARACFG_V40");
            i = j + iGroupNO* MAX_CHANNUM_V30;
            if (i < g_struDeviceInfo[iDeviceIndex].iAnalogChanNum)
            {
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iDeviceIndex = iDeviceIndex;
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChanIndex = i;
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChannelNO = i + g_struDeviceInfo[iDeviceIndex].iStartChan;
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bEnable = TRUE;
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChanType = DEMO_CHANNEL_TYPE_ANALOG;
                sprintf(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName, ANALOG_C_FORMAT, i+g_struDeviceInfo[iDeviceIndex].iStartChan);	
            }
            else//clear the state of other channel
            {
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iDeviceIndex = -1;
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChanIndex = -1;
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bEnable = FALSE;
                sprintf(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName, "");	
            }
            
            
            g_struDeviceInfo[iDeviceIndex].iGroupNO = -1;
        } 
        else
        {
            g_struDeviceInfo[iDeviceIndex].iGroupNO = iGroupNO;
            g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GET_IPPARACFG_V40");
            RefreshIPDevLocalCfg(iDeviceIndex);
        }
    }
	
    if (g_struDeviceInfo[iDeviceIndex].byZeroChanNum > 0)
    {
        for (i=0; i<g_struDeviceInfo[iDeviceIndex].byZeroChanNum; i++)
        {
            if (1)
            {
                g_struDeviceInfo[iDeviceIndex].struZeroChan[i].iDeviceIndex = iDeviceIndex;
                g_struDeviceInfo[iDeviceIndex].struZeroChan[i].iChanIndex = i+ZERO_CHAN_INDEX;
                sprintf(g_struDeviceInfo[iDeviceIndex].struZeroChan[i].chChanName, "ZeroChan%d", i);	
                //analog devices
                if (1)
                {
                    g_struDeviceInfo[iDeviceIndex].struZeroChan[i].bEnable = TRUE;
                    g_struDeviceInfo[iDeviceIndex].struZeroChan[i].dwImageType = CHAN_ORIGINAL;
                    //				g_struDeviceInfo[iDeviceIndex].iEnableChanNum ++;
                }
                else
                {
                    g_struDeviceInfo[iDeviceIndex].struZeroChan[i].dwImageType = CHAN_OFF_LINE;
                    g_struDeviceInfo[iDeviceIndex].struZeroChan[i].bEnable = FALSE;		
                }
            }
        }
    }
	return g_struDeviceInfo[iDeviceIndex].bIPRet;
}

/*********************************************************
Function:	DoGetIPAlarmInCfg
Desc:		get device alarm input configuration
Input:	iDeviceIndex, the device index
		bSupportMultiGroup, support multi group
		GroupNo, group number
Output:	none
Return:	TRUE/FALSE
**********************************************************/
BOOL CClientDemoDlg::DoGetIPAlarmInCfg(int iDeviceIndex, BOOL bSupportMultiGroup, int GroupNO)
{
	int i = 0;
	char szTmp[1024*5] = {0};
	char szTmp1[32] = {0};
	DWORD dwReturned = 0;
//	memset(&g_struDeviceInfo[iDeviceIndex].struAlarmInCfg, 0, sizeof(NET_DVR_IPALARMINCFG));
	if (g_struDeviceInfo[iDeviceIndex].pStruIPAlarmInCfgV40 == NULL)
    {
        return FALSE;
    }
    memset(g_struDeviceInfo[iDeviceIndex].pStruIPAlarmInCfgV40, 0, sizeof(NET_DVR_IPALARMINCFG_V40));

	if (bSupportMultiGroup)
	{
		g_struDeviceInfo[iDeviceIndex].bIPRet = NET_DVR_GetDVRConfig(g_struDeviceInfo[iDeviceIndex].lLoginID, NET_DVR_GET_IP_ALARM_IN, GroupNO, &g_struDeviceInfo[iDeviceIndex].struAlarmInCfg, sizeof(NET_DVR_IPALARMINCFG), &dwReturned);
	} 
	else
	{
		g_struDeviceInfo[iDeviceIndex].bIPRet = NET_DVR_GetDVRConfig(g_struDeviceInfo[iDeviceIndex].lLoginID, NET_DVR_GET_IPALARMINCFG_V40, 0,  g_struDeviceInfo[iDeviceIndex].pStruIPAlarmInCfgV40 , sizeof(NET_DVR_IPALARMINCFG_V40), &dwReturned);
	}
	
	if (g_struDeviceInfo[iDeviceIndex].bIPRet)
	{	//8000 device
// 		for (i=0; i< MAX_IP_ALARMIN; i++)
// 		{
// 			if (g_struDeviceInfo[iDeviceIndex].struAlarmInCfg.struIPAlarmInInfo[i].byIPID != 0)
// 			{
// 				sprintf(szTmp1, "%d[%d-%d]", i, g_struDeviceInfo[iDeviceIndex].struAlarmInCfg.struIPAlarmInInfo[i].byIPID, \
// 					g_struDeviceInfo[iDeviceIndex].struAlarmInCfg.struIPAlarmInInfo[i].byAlarmIn);
// 				strcat(szTmp, szTmp1);
// 			}
// 		}
		for (i= 0; i < MAX_IP_ALARMIN_V40; i++)
        {
            if (g_struDeviceInfo[iDeviceIndex].pStruIPAlarmInCfgV40->struIPAlarmInInfo[i].dwIPID != 0)
            {
                sprintf(szTmp1, "%d[%d-%d]", i, g_struDeviceInfo[iDeviceIndex].pStruIPAlarmInCfgV40->struIPAlarmInInfo[i].dwIPID, \
					g_struDeviceInfo[iDeviceIndex].pStruIPAlarmInCfgV40->struIPAlarmInInfo[i].dwAlarmIn);
				strcat(szTmp, szTmp1);
            }
        }

		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GET_IPALARMINCFG:%s", szTmp);
	} 
	else
	{
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GET_IPALARMINCFG");
	}
	
	return g_struDeviceInfo[iDeviceIndex].bIPRet;
}

/*********************************************************
Function:	DoGetIPAlarmOutCfg
Desc:		get device output configuration
Input:	iDeviceIndex, the device index
		bSupportMultiGroup, support multi group
		GroupNo, group number
Output:	none
Return:	TRUE/FALSE
**********************************************************/
BOOL CClientDemoDlg::DoGetIPAlarmOutCfg(int iDeviceIndex, BOOL bSupportMultiGroup, int GroupNO)
{
	int i = 0;
	char szTmp[1024] = {0};
	char szTmp1[32] = {0};
	DWORD dwReturned = 0;
//	memset(&g_struDeviceInfo[iDeviceIndex].struAlarmOutCfg, 0, sizeof(NET_DVR_IPALARMOUTCFG));
	
	if (g_struDeviceInfo[iDeviceIndex].pStruIPAlarmOutCfgV40 == NULL)
    {
        return FALSE;
    }
    memset(g_struDeviceInfo[iDeviceIndex].pStruIPAlarmOutCfgV40, 0, sizeof(NET_DVR_IPALARMOUTCFG_V40));


	if (bSupportMultiGroup)
	{
		g_struDeviceInfo[iDeviceIndex].bIPRet = NET_DVR_GetDVRConfig(g_struDeviceInfo[iDeviceIndex].lLoginID, NET_DVR_GET_IP_ALARM_OUT, GroupNO, &g_struDeviceInfo[iDeviceIndex].struAlarmOutCfg, sizeof(NET_DVR_IPALARMOUTCFG), &dwReturned);
	} 
	else
	{
		g_struDeviceInfo[iDeviceIndex].bIPRet = NET_DVR_GetDVRConfig(g_struDeviceInfo[iDeviceIndex].lLoginID, NET_DVR_GET_IPALARMOUTCFG_V40, 0, g_struDeviceInfo[iDeviceIndex].pStruIPAlarmOutCfgV40, sizeof(NET_DVR_IPALARMOUTCFG_V40), &dwReturned);
	}
	
	if (g_struDeviceInfo[iDeviceIndex].bIPRet)
	{	//8000 device
		for (i=0; i< MAX_IP_ALARMOUT_V40; i++)
		{
			if (g_struDeviceInfo[iDeviceIndex].pStruIPAlarmOutCfgV40->struIPAlarmOutInfo[i].dwIPID != 0)
			{
				sprintf(szTmp1, "%d[%d-%d]", i, g_struDeviceInfo[iDeviceIndex].pStruIPAlarmOutCfgV40->struIPAlarmOutInfo[i].dwIPID, \
                    g_struDeviceInfo[iDeviceIndex].pStruIPAlarmOutCfgV40->struIPAlarmOutInfo[i].dwAlarmOut);
				strcat(szTmp, szTmp1);
			}
		}
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_GET_IPALARMOUTCFG_V40-%s", szTmp);
	} 
	else
	{
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_GET_IPALARMOUTCFG_V40");
	}
	
	return g_struDeviceInfo[iDeviceIndex].bIPRet;
}

/*********************************************************
Function:	DoSetIPAccessCfg
Desc:		set the ip configure of device/�豸ip�������
Input:	iDeviceIndex, the device index
		dwGroupNo  Group No. 
Output:	none
Return:	TRUE/FALSE
**********************************************************/
BOOL CClientDemoDlg::DoSetIPAccessCfg(int iDeviceIndex, DWORD dwGroupNo)
{
	BOOL bIPRet = FALSE;
	//2008-9-15 13:44 ip input configuration
	bIPRet = NET_DVR_SetDVRConfig(g_struDeviceInfo[iDeviceIndex].lLoginID, NET_DVR_SET_IPPARACFG_V40, dwGroupNo, &g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[dwGroupNo], sizeof(g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0]));
	if (bIPRet)
	{		
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "NET_DVR_SET_IPPARACFG_V40");
		RefreshIPDevLocalCfg(iDeviceIndex);
	}
	else
	{
		g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "NET_DVR_SET_IPPARACFG_V40");
	}
	
	return bIPRet;
}

/*********************************************************
Function:	RefreshIPDevLocalCfg
Desc:		refresh the ip device configure of local device configure
Input:	iDeviceIndex, the device index in the local device group
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::RefreshIPDevLocalCfg(int iDeviceIndex)
{
	EnterCriticalSection(&m_struLock);
	LPNET_DVR_IPPARACFG_V40 lpIPAccessCfgV40 = &g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[g_struDeviceInfo[iDeviceIndex].iGroupNO];
	DWORD dwChanShow = 0;
	int iIPChanIndex = 0;
	int i = 0;
	// 	for (i=0; i<MAX_IP_DEVICE; i++)
	// 	{
	// 		AddLog(-1, OPERATION_SUCC_T, "IP dev chan[%d] ip=%s name=%s", i,\
	// 		lpIPAccessCfg->struIPDevInfo[i].struIP.sIpV4, lpIPAccessCfg->struIPDevInfo[i].sUserName);
	// 	}

	//Update MAX IP channel number.
	g_struDeviceInfo[iDeviceIndex].iIPChanNum = lpIPAccessCfgV40->dwDChanNum;

	int iAnalogChanCount = 0;
	int iIPChanCount = 0;
	int iGroupNO   = g_struDeviceInfo[iDeviceIndex].iGroupNO;    //Group NO.
	int iGroupNum  = lpIPAccessCfgV40->dwGroupNum;
	int iIPChanNum = g_struDeviceInfo[iDeviceIndex].iIPChanNum;
	
    for (i = 0; i < MAX_CHANNUM_V30; i++)
    {

        //analog channel
        if (iAnalogChanCount < g_struDeviceInfo[iDeviceIndex].iAnalogChanNum)
        {
            dwChanShow = iAnalogChanCount + g_struDeviceInfo[iDeviceIndex].iStartChan + g_struDeviceInfo[iDeviceIndex].iGroupNO*64;
            
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].iDeviceIndex = iDeviceIndex;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].iChanIndex = i;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].iChanType  = DEMO_CHANNEL_TYPE_ANALOG;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].iChannelNO = dwChanShow;
            
            sprintf(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName, ANALOG_C_FORMAT, dwChanShow);	
            //analog devices
            if (lpIPAccessCfgV40->byAnalogChanEnable[i])
            {
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].bEnable = TRUE;
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].dwImageType = CHAN_ORIGINAL;
                //g_struDeviceInfo[iDeviceIndex].iEnableChanNum ++;
            }
            else
            {
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].bEnable = FALSE;	
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].dwImageType = CHAN_OFF_LINE;
            }
            
            iAnalogChanCount++;
        }
        //IP channel  //(i>=MAX_ANALOG_CHANNUM && i<MAX_ANALOG_CHANNUM+g_struDeviceInfo[iDeviceIndex].iIPChanNum) )
//         else if (iGroupNO >= 0 && ((iIPChanCount + iGroupNO * MAX_CHANNUM_V30)  <  iIPChanNum) && (iIPChanCount < iIPChanNum ))   
//         {
//             dwChanShow = iIPChanCount + iGroupNO * MAX_CHANNUM_V30 + lpIPAccessCfgV40->dwStartDChan - 1;
//             
//             g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwChanShow].iChanType  = DEMO_CHANNEL_TYPE_IP;
//             g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwChanShow].iChannelNO = dwChanShow + 1;
//             
//             iIPChanIndex = iIPChanCount;
//             g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwChanShow].iDeviceIndex = iDeviceIndex;
//             g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwChanShow].iChanIndex =i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64;	
//             sprintf(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].chChanName, DIGITAL_C_FORMAT, iIPChanCount + iGroupNO * MAX_CHANNUM_V30 + 1);				
//             // 			AddLog(-1, OPERATION_SUCC_T, "IPc%d id%d chan[%d] ", iIPChanIndex,\
//             // 				lpIPAccessCfg->struIPChanInfo[iIPChanIndex].byIPID, lpIPAccessCfg->struIPChanInfo[iIPChanIndex].byChannel);
//             if (lpIPAccessCfgV40->struStreamMode[iIPChanIndex].uGetStream.struChanInfo.byIPID != 0 && lpIPAccessCfgV40->struStreamMode[iIPChanIndex].uGetStream.struChanInfo.byEnable == TRUE)
//             {
//                 g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwChanShow].bEnable = TRUE;//
//                 if (lpIPAccessCfgV40->struStreamMode[iIPChanIndex].uGetStream.struChanInfo.byEnable)
//                 {
//                     g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwChanShow].dwImageType = CHAN_ORIGINAL;
//                 }
//                 else
//                 {
//                     g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwChanShow].dwImageType = CHAN_OFF_LINE;
//                 }
//                 
//                 //				g_struDeviceInfo[iDeviceIndex].iEnableChanNum ++;
//             }
//             else
//             {
//                 g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwChanShow].dwImageType = CHAN_OFF_LINE;
//                 g_struDeviceInfo[iDeviceIndex].pStruChanInfo[dwChanShow].bEnable = FALSE;	
//                 //g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = FALSE;
//             }
//             
//             iIPChanCount++;
//         }
        else if (iGroupNO >= 0 && ((iIPChanCount + iGroupNO * MAX_CHANNUM_V30)  <  iIPChanNum) && (iIPChanCount < iIPChanNum))   
        {
            dwChanShow = iIPChanCount + iGroupNO * MAX_CHANNUM_V30 + lpIPAccessCfgV40->dwStartDChan;
            
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].iChanType  = DEMO_CHANNEL_TYPE_IP;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].iChannelNO = dwChanShow;
            
            iIPChanIndex = iIPChanCount;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].iDeviceIndex = iDeviceIndex;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].iChanIndex = i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64;	
            sprintf(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].chChanName, DIGITAL_C_FORMAT, iIPChanCount + iGroupNO * MAX_CHANNUM_V30 + 1);				
           
            if (lpIPAccessCfgV40->struStreamMode[iIPChanIndex].uGetStream.struChanInfo.byIPID != 0 && lpIPAccessCfgV40->struStreamMode[iIPChanIndex].uGetStream.struChanInfo.byEnable == TRUE)
            {
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].bEnable = TRUE;//
                if (lpIPAccessCfgV40->struStreamMode[iIPChanIndex].uGetStream.struChanInfo.byEnable)
                {
                    g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].dwImageType = CHAN_ORIGINAL;
                }
                else
                {
                    g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].dwImageType = CHAN_OFF_LINE;
                }
                
                //				g_struDeviceInfo[iDeviceIndex].iEnableChanNum ++;
            }
            else
            {
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].dwImageType = CHAN_OFF_LINE;
                g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].bEnable = FALSE;	
                //g_struDeviceInfo[iDeviceIndex].struChanInfo[i].bAlarm = FALSE;
            }
            
            iIPChanCount++;
        }
        else
        {
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].iDeviceIndex = -1;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].iChanIndex = -1;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].iChanType  = DEMO_CHANNEL_TYPE_INVALID;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].iChannelNO = -1;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].bEnable = FALSE;	
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].bAlarm = FALSE;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].bLocalManualRec = FALSE;
            g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].lRealHandle = -1;
            sprintf(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].chChanName, "");	
        }
    }
 
	for (i = 0; i < MAX_CHANNUM_V40; i++)
	{
		if (g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i + g_struDeviceInfo[iDeviceIndex].iGroupNO*64].bEnable)
		{
			g_struDeviceInfo[iDeviceIndex].lFirstEnableChanIndex = i;
			break;
		}
	}

    if (g_struDeviceInfo[iDeviceIndex].byZeroChanNum > 0)
    {
        //get����
    }
	
	LeaveCriticalSection(&m_struLock);
}

/*********************************************************
Function:	RedrawDeviceNode
Desc:		redraw the device node on device tree, delete all channels node first, then add new channels
Input:	iDeviceIndex, the device index in the local device group
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::RedrawDeviceNode(int iDeviceIndex)
{
	HTREEITEM hChanItem = NULL;
	HTREEITEM hPreItem = NULL;
	CString csTmp;
	int i = 0;
	int iData = 0;
	int iLoginStat = DEVICE_LOGIN;
	if (iDeviceIndex >= 1000)//�豸��Ŵ���1000ʱ��ʾֻ��Ӳ���½
	{
		iDeviceIndex-=1000;
		iLoginStat = DEVICE_LOGOUT;
	}
	else
	{
		iLoginStat = DEVICE_LOGIN;
	}
	if (g_bPlayAllDevice || g_struDeviceInfo[iDeviceIndex].bPlayDevice)
	{
		return;
	}
	
	
	
	HTREEITEM hRoot = m_treeDeviceList.GetRootItem();
	if (hRoot == NULL)
	{
		return;
	}
	
	HTREEITEM hDevItem = m_treeDeviceList.GetChildItem(hRoot);
	
	//find the select device
	while (hDevItem != NULL)
	{
		iData = m_treeDeviceList.GetItemData(hDevItem);
		if (DEVICETYPE == iData/1000)
		{
			if (iDeviceIndex ==  iData%1000)
			{
				HTREEITEM hChanItem = m_treeDeviceList.GetChildItem(hDevItem);
				
                while (i < MAX_CHANNUM_V40 + g_struDeviceInfo[iDeviceIndex].byZeroChanNum && hChanItem != NULL)
				{
					hPreItem = hChanItem;
					hChanItem = m_treeDeviceList.GetNextSiblingItem(hChanItem);
					if (CHANNELTYPE != (int)m_treeDeviceList.GetItemData(hPreItem)/1000)
					{
						AddLog(iDeviceIndex, OPERATION_FAIL_T, "to next channel node error");
						break;
					}
					i++;
				}
				break;
			}
		}//end else if
		
		hDevItem = m_treeDeviceList.GetNextVisibleItem(hDevItem);
	}//end while

	if (hDevItem != NULL)
	{
		while(i>0 && hPreItem != NULL)
		{
			hChanItem = hPreItem;
			hPreItem = m_treeDeviceList.GetPrevSiblingItem(hChanItem);
			if (hPreItem == NULL)
			{
				
				//				AddLog(iDeviceIndex, OPERATION_SUCC_T, "del last chan[%d]", m_treeDeviceList.GetItemData(hChanItem)%1000);
				hPreItem = m_treeDeviceList.GetParentItem(hChanItem);
				m_treeDeviceList.DeleteItem(hChanItem);
				
				if (hPreItem == NULL)
				{
					AddLog(-1, OPERATION_FAIL_T, "get the device node err");
					break;
				}	
				if (CHANNELTYPE != (int)m_treeDeviceList.GetItemData(hPreItem)/1000)
				{
					//		AddLog(iDeviceIndex, OPERATION_SUCC_T, "dev[%d] node all delete", m_treeDeviceList.GetItemData(hPreItem)%1000);
					break;
				}				
			}
			//AddLog(iDeviceIndex, OPERATION_SUCC_T, "del chan[%d]", m_treeDeviceList.GetItemData(hChanItem)%1000);
			m_treeDeviceList.DeleteItem(hChanItem);
			
			i--;
		}
	}
	
	if (hDevItem == NULL)
	{//added 9000 devices
		hDevItem = m_treeDeviceList.InsertItem(g_struDeviceInfo[iDeviceIndex].chLocalNodeName, iLoginStat, iLoginStat, hRoot, GetAfterItem(hRoot));	
	}
	
	//add channel node//g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bEnable || 
	for (i=0; i< g_struDeviceInfo[iDeviceIndex].iIPChanNum + g_struDeviceInfo[iDeviceIndex].iAnalogChanNum; i++)
	{
		if (g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChanIndex != -1 )
		{
			csTmp.Format("%s", g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].chChanName);
			hChanItem =  m_treeDeviceList.InsertItem(csTmp, g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].dwImageType, \
				g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].dwImageType, hDevItem);
			m_treeDeviceList.SetItemData(hChanItem, CHANNELTYPE * 1000 + g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].iChanIndex);
		}
	}
    for (i=0; i< g_struDeviceInfo[iDeviceIndex].byZeroChanNum; i++)
    {
        if (g_struDeviceInfo[iDeviceIndex].struZeroChan[i].iChanIndex != -1 )
        {
            csTmp.Format("%s", g_struDeviceInfo[iDeviceIndex].struZeroChan[i].chChanName);
            hChanItem =  m_treeDeviceList.InsertItem(csTmp, g_struDeviceInfo[iDeviceIndex].struZeroChan[i].dwImageType, \
                g_struDeviceInfo[iDeviceIndex].struZeroChan[i].dwImageType, hDevItem);
            m_treeDeviceList.SetItemData(hChanItem, CHANNELTYPE * 1000 + g_struDeviceInfo[iDeviceIndex].struZeroChan[i].iChanIndex);
        }
    }
    
}

/*********************************************************
Function:	DoTestElseFunction
Desc:		test 
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::DoTestElseFunction()
{
	int iDevIndex = GetCurDeviceIndex();
	CDlgDoTest dlg;
	dlg.m_iDeviceIndex = iDevIndex;
	dlg.DoModal();
}

/*********************************************************
Function:	OnDbclkListAllLog
Desc:		get current device fault info
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::OnDblclkListAllLog(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int iItemSel = 0;
	CString csTxt;
	
	POSITION  iPos = m_listAllLog.GetFirstSelectedItemPosition();
	if (iPos == NULL)
	{
		return;
	}
	iItemSel = m_listAllLog.GetNextSelectedItem(iPos);//%s\t%s\tm_listAllLog.GetItemText(iItemSel,1),\m_listAllLog.GetItemText(iItemSel,2)
	csTxt.Format("%s",m_listAllLog.GetItemText(iItemSel,5));
	if (csTxt.GetLength() > 3)
	{
		AfxMessageBox(csTxt);	
	}
	
	*pResult = 0;
}

/*********************************************************
Function:	OnSelchangeComboListType
Desc:		select the corresponding list, alarm or log
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::OnSelchangeComboListType() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	ListRestore();
	
	if (m_iMainType == CONFIG_ALL_T)
	{
		m_dlgConfigAll->ConfigWndUpdate();			
	}
	else if (m_iMainType == PLAY_BACK_T)
	{
		m_dlgPlayBack->ShowWindow(SW_SHOW);
		m_dlgPlayBack->PlayBackWinUpdate();
	}
	else if (m_iMainType == PREVIEW_T)
	{
		PreviewReferShow(TRUE);
	}
	switch (m_iSelListType)
	{
	case 0:
		m_listAlarmInfo.ShowWindow(SW_HIDE);
		m_listAllLog.ShowWindow(SW_SHOW);	
		break;
	case 1:		
		m_listAllLog.ShowWindow(SW_HIDE);
		m_listAlarmInfo.ShowWindow(SW_SHOW);
		break;
	default:
		break;
	}
}

/*********************************************************
Function:	OnColumnclickListAllLog
Desc:		response the Column-click of all log list  
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::OnColumnclickListAllLog(NMHDR* pNMHDR, LRESULT* pResult) 
{
	EnlargeList(m_listAllLog, m_bListLogEnlarge);
	*pResult = 0;
}

/*********************************************************
Function:	OnColumnclickListAlarmInfo
Desc:		response the Column-click of all log list
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::OnColumnclickListAlarmInfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	EnlargeList(m_listAlarmInfo, m_bListAlarmEnlarge);
	*pResult = 0;	
}

/*********************************************************
Function:	EnlargeList
Desc:		enlarge list when click the column
Input:	list, log or alarm information tree; bEnargeList, TRUE/FALSE;
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::EnlargeList(CListCtrl &list, BOOL &bEnargeList)
{
	CRect rc(0,0,0,0);
	list.GetWindowRect(&rc);
	ScreenToClient(&rc);
	
	if (!bEnargeList)
	{
		rc.top-=LIST_ENLARGE_HIGH;//move top upper, stretch
		list.MoveWindow(&rc);
		bEnargeList = TRUE;
		if (m_iMainType == CONFIG_ALL_T)
		{
			m_dlgRemoteCfg->ShowWindow(SW_HIDE);
			// m_dlgRemoteCfg->ChangeCurConfigWnd(REMOTE_MAT_DEC_CFG_T+1);     
            m_dlgRemoteCfg->ChangeCurConfigWnd(REMOTE_DEVICE_CFG_T);
			m_dlgLocalCfg->ShowWindow(SW_HIDE);
			m_dlgIPAccess->ShowWindow(SW_HIDE);	
			m_dlgHDConfig->ShowWindow(SW_HIDE);
			m_dlgBatchCfg->ShowWindow(SW_HIDE);
		}
		else if (m_iMainType == PLAY_BACK_T)
		{
			m_dlgPlayBack->ShowWindow(SW_HIDE);
			// 			m_dlgPlayBack->m_dlgPlayRemoteFile.ShowWindow(SW_HIDE);
			// 			m_dlgPlayBack->m_dlgPlayLocalFile.ShowWindow(SW_HIDE);
			// 			m_dlgPlayBack->m_dlgPlayRemoteTime.ShowWindow(SW_HIDE);			
		}
		else if(m_iMainType == STREAM_ID_T)
		{
		//	m_pStreamIDDlg->ShowWindow(SW_HIDE);
		}
		else if (m_iMainType == PRODUCT_RELATE_T)
		{
			m_dlgProductCfg->ShowWindow(SW_HIDE);
		}
		else
		{
			PreviewReferShow(FALSE);
			m_dlgOutputCtrl->ShowWindow(SW_SHOW);
		}
		
	}
	else
	{
		rc.top+=LIST_ENLARGE_HIGH;//move top down, compress
		list.MoveWindow(&rc);
		bEnargeList = FALSE;
		if (m_iMainType == CONFIG_ALL_T)
		{
			m_dlgConfigAll->ConfigWndUpdate();			
		}
		else if (m_iMainType == PLAY_BACK_T)
		{
			m_dlgPlayBack->ShowWindow(SW_SHOW);
			//			m_dlgPlayBack->PlayBackWinUpdate();
		}
		else if (m_iMainType == STREAM_ID_T)
		{
			//m_pStreamIDDlg->ShowWindow(SW_SHOW);
		}
		else if (m_iMainType ==  PRODUCT_RELATE_T)
		{
			m_dlgProductCfg->ShowWindow(SW_SHOW);
		}
		else
		{
			PreviewReferShow(TRUE);
		}
		
	}
	if (!g_struLocalParam.bFullScreen)
	{
		list.ShowWindow(SW_SHOW);
	}
	
}
void CClientDemoDlg::ProcAlarmHostAlarm(WPARAM wParam, LPARAM lParam)
{
    char szInfoBuf[1024] = {0};
    char szLan[128] ={0};
    LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iWinIndex = -1;
    int i = 0;
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_ALARMHOST_ALARMINFO  struAlarmInfo;
    memset(&struAlarmInfo, 0, sizeof(struAlarmInfo));
    CTime time = CTime::GetCurrentTime();
    memcpy(&struAlarmInfo, pAlarmInfo, sizeof(struAlarmInfo));
    
    g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
    ::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));	
    
    g_StringLanType(szLan, "�յ����籨����������", "receive alarmhost alarm");
    memcpy(szInfoBuf, szLan, strlen(szLan));
    for (i = 0; i < ARRAY_SIZE(struAlarmInfo.byAlarmInputNumber); i++)
    {
        sprintf(&szInfoBuf[i + strlen(szLan)], "%d", struAlarmInfo.byAlarmInputNumber[i]);                
    }

    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);

}
/*********************************************************
Function:	OnBtnTest
Desc:		for testing the interfaces those are not used usually
Input:	none
Output:	none
Return:	none
**********************************************************/
void CClientDemoDlg::OnBtnTest() 
{
	// TODO: Add your control notification handler code here
	DoTestElseFunction();
	NET_DVR_SDKSTATE SDKState;
	NET_DVR_GetSDKState( &SDKState );
	g_pMainDlg->AddLog(-1, OPERATION_SUCC_T, "SDKState.dwTotalRealPlayNum %d", SDKState.dwTotalRealPlayNum);
	return;
}

void CClientDemoDlg::ListRestore()
{
	CRect rc(0,0,0,0);
	if (m_bListLogEnlarge)
	{
		m_listAllLog.GetWindowRect(&rc);
		ScreenToClient(&rc);
		rc.top+=LIST_ENLARGE_HIGH;//move top down, compress
		m_listAllLog.MoveWindow(&rc);
		m_bListLogEnlarge = FALSE;
	}
	
	if (m_bListAlarmEnlarge)
	{
		m_listAlarmInfo.GetWindowRect(&rc);
		ScreenToClient(&rc);
		rc.top+=LIST_ENLARGE_HIGH;//move top down, compress
		m_listAlarmInfo.MoveWindow(&rc);
		m_bListAlarmEnlarge = FALSE;
	}
}

void CClientDemoDlg::OnRadioLocalLog() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_iSelListType = 0;
	OnSelchangeComboListType();
	UpdateData(FALSE);
}

void CClientDemoDlg::OnRadioAlarmInfo() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_iSelListType = 1;
	OnSelchangeComboListType();
	UpdateData(FALSE);	
}

void CClientDemoDlg::OnMenuAtmFrameFormat() 
{
	// TODO: Add your command handler code here
    int iDeviceIndex = GetCurDeviceIndex();
    if (iDeviceIndex < 0)
    {
        return;
    }
    
    int iChanIndex = GetCurChanIndex();
    if (iChanIndex < 0)
    {
        iChanIndex = 0;//default to config channel one
    }
    
    CDlgAtmFrameFormatV30 dlg;
    dlg.m_lServerID = g_struDeviceInfo[iDeviceIndex].lLoginID;
    dlg.m_iDevIndex = iDeviceIndex;
    dlg.m_iChannelNum = g_struDeviceInfo[iDeviceIndex].iDeviceChanNum;
    dlg.DoModal();
}

void CClientDemoDlg::OnMenuSensorAddPreview() 
{
   int iDeviceIndex = GetCurDeviceIndex();
   if (iDeviceIndex < 0)
   {
       return;
   }
   CDlgSensorAddPreview dlg;
   dlg.m_lServerID = g_struDeviceInfo[iDeviceIndex].lLoginID;
   dlg.m_iDevIndex = iDeviceIndex;
   dlg.DoModal();
}

void CClientDemoDlg::OnMenuHolidayCfg() 
{
   
    int iDeviceIndex = GetCurDeviceIndex();
    if (iDeviceIndex < 0)
    {
        return;
    }

    CDlgHolidayCfg dlg;
    dlg.m_lServerID = g_struDeviceInfo[iDeviceIndex].lLoginID;
    dlg.m_iDevIndex = iDeviceIndex;
    dlg.DoModal();


    DWORD dwGetCommand = NET_DVR_GET_ALARMOUT_HOLIDAY_HANDLE;
    DWORD dwSetCommand = NET_DVR_SET_ALARMOUT_HOLIDAY_HANDLE;
    NET_DVR_HOLIDAY_HANDLE struHolidayHandle = {0};
    DWORD dwReturn = 0;
    if (NET_DVR_GetDVRConfig(g_struDeviceInfo[iDeviceIndex].lLoginID, dwGetCommand, 1, &struHolidayHandle, sizeof(struHolidayHandle), &dwReturn))
    {
        g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "Get Holiday handle");
    }
    else
    {
        g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, "Get Holiday handle");
    }
    
    memset(&struHolidayHandle, 0, sizeof(struHolidayHandle));
    if (NET_DVR_SetDVRConfig(g_struDeviceInfo[iDeviceIndex].lLoginID, dwSetCommand, 1, &struHolidayHandle, sizeof(struHolidayHandle)))
    {
        g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "Set Holiday handle");
    }
    else
    {
        g_pMainDlg->AddLog(iDeviceIndex, OPERATION_SUCC_T, "Set Holiday handle");
    }
}

void CClientDemoDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	NET_DVR_Cleanup();
	NET_DVR_Init();
	NET_DVR_SDKSTATE SDKState;
	NET_DVR_GetSDKState(&SDKState);
}

void CClientDemoDlg::ProcAlarmVQD(WPARAM wParam, LPARAM lParam)
{
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
    char *pAlarmInfo = (char *)(lParam);		
    int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_VQD_DIAGNOSE_INFO struVQDAlarm = {0};
    memcpy(&struVQDAlarm, pAlarmInfo, sizeof(struVQDAlarm));
	
	char szLan[1024] = {0};
	char szLanCn[1024] = {0};
	char szLanEn[1024] = {0};
	if (struVQDAlarm.struVQDDiagnoseExceptionInfo.dwChannelNO < g_struDeviceInfo[iDeviceIndex].iAnalogChanNum)
	{
		sprintf(szLanCn,"Camera%d",struVQDAlarm.struVQDDiagnoseExceptionInfo.dwChannelNO);
		sprintf(szLanEn,"Camera%d",struVQDAlarm.struVQDDiagnoseExceptionInfo.dwChannelNO);
	}
	else
	{
		sprintf(szLanCn,"IPCamera%02d",struVQDAlarm.struVQDDiagnoseExceptionInfo.dwChannelNO);
		sprintf(szLanEn,"IPCamera%02d",struVQDAlarm.struVQDDiagnoseExceptionInfo.dwChannelNO);
	}
	
	switch (struVQDAlarm.struVQDDiagnoseExceptionInfo.dwVQDType)
	{
	case VQD_BLUR_FLAG:
		sprintf(szLanCn,"%s ͼ��ģ��",szLanCn);
		sprintf(szLanEn,"%s Blur",szLanEn);
		
		break;
	case VQD_LUMA_FLAG:
		sprintf(szLanCn,"%s �����쳣",szLanCn);
		sprintf(szLanEn,"%s Luma",szLanEn);
		break;
	case VQD_CHROMA_FLAG:
		sprintf(szLanCn,"%s ͼ��ƫɫ",szLanCn);
		sprintf(szLanEn,"%s Chroma",szLanEn);
		break;
	case VQD_SNOW_FLAG:
		sprintf(szLanCn,"%s ѩ������",szLanCn);
		sprintf(szLanEn,"%s Snow",szLanEn);
		break;
	case VQD_STREAK_FLAG:
		sprintf(szLanCn,"%s ���Ƹ���",szLanCn);
		sprintf(szLanEn,"%s Streak",szLanEn);
		break;
	case VQD_FREEZE_FLAG:
		sprintf(szLanCn,"%s ���涳��",szLanCn);
		sprintf(szLanEn,"%s Freeze",szLanEn);
		break;
	case VQD_SIGLOSE_FLAG:
		sprintf(szLanCn,"%s �źŶ�ʧ",szLanCn);
		sprintf(szLanEn,"%s Signal Lose",szLanEn);
		break;
	case VQD_PTZ_FLAG:
		sprintf(szLanCn,"%s ��̨ʧ��",szLanCn);
		sprintf(szLanEn,"%s PTZ",szLanEn);
		break;	
	default:
		g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, "Unknown Alarm");
		return;
	}
	
	sprintf(szLanCn,"%s %4d-%2d-%2d %2d:%2d:%2d",szLanCn, struVQDAlarm.struVQDDiagnoseExceptionInfo.struDiagnoseTime.dwYear,\
		struVQDAlarm.struVQDDiagnoseExceptionInfo.struDiagnoseTime.dwMonth, struVQDAlarm.struVQDDiagnoseExceptionInfo.struDiagnoseTime.dwDay,\
		struVQDAlarm.struVQDDiagnoseExceptionInfo.struDiagnoseTime.dwHour, struVQDAlarm.struVQDDiagnoseExceptionInfo.struDiagnoseTime.dwMinute,\
		struVQDAlarm.struVQDDiagnoseExceptionInfo.struDiagnoseTime.dwSecond);
	
	sprintf(szLanEn,"%s %4d-%2d-%2d %2d:%2d:%2d",szLanEn, struVQDAlarm.struVQDDiagnoseExceptionInfo.struDiagnoseTime.dwYear,\
		struVQDAlarm.struVQDDiagnoseExceptionInfo.struDiagnoseTime.dwMonth, struVQDAlarm.struVQDDiagnoseExceptionInfo.struDiagnoseTime.dwDay,\
		struVQDAlarm.struVQDDiagnoseExceptionInfo.struDiagnoseTime.dwHour, struVQDAlarm.struVQDDiagnoseExceptionInfo.struDiagnoseTime.dwMinute,\
		struVQDAlarm.struVQDDiagnoseExceptionInfo.struDiagnoseTime.dwSecond);
	
	g_StringLanType(szLan,szLanCn,szLanEn);
	g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szLan);
}

void CClientDemoDlg::ProcAlarmVQDEx(WPARAM wParam,LPARAM lParam)
{
	char szInfoBuf[1024] = {0};
	LPLOCAL_ALARM_INFO pAlarmDev = (LPLOCAL_ALARM_INFO)(wParam);
	char *pAlarmInfo = (char *)(lParam);		
	int iWinIndex = -1;         
	int i = 0;
	int iDeviceIndex = pAlarmDev->iDeviceIndex;
    
    NET_DVR_VQD_ALARM  struVQDAlarm = {0};
	CTime time = CTime::GetCurrentTime();
	memcpy(&struVQDAlarm, pAlarmInfo, sizeof(struVQDAlarm));
	i = struVQDAlarm .struDevInfo.byIvmsChannel-g_struDeviceInfo[iDeviceIndex].iStartChan;
	g_struDeviceInfo[iDeviceIndex].pStruChanInfo[i].bAlarm = TRUE;
	::PostMessage(g_pMainDlg->m_hWnd, WM_CHANGE_CHANNEL_ITEM_IMAGE, WPARAM(iDeviceIndex), LPARAM(i));
	
    NET_DVR_TIME struAbsTime = {0};     
    struAbsTime.dwYear   = GET_YEAR(struVQDAlarm.dwAbsTime);
    struAbsTime.dwMonth  = GET_MONTH(struVQDAlarm.dwAbsTime);
    struAbsTime.dwDay    = GET_DAY(struVQDAlarm.dwAbsTime);
    struAbsTime.dwHour   = GET_HOUR(struVQDAlarm.dwAbsTime);
    struAbsTime.dwMinute = GET_MINUTE(struVQDAlarm.dwAbsTime);
    struAbsTime.dwSecond = GET_SECOND(struVQDAlarm.dwAbsTime);
	
    CString str = "";
    sprintf(szInfoBuf, "VQD extended Alarm Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d]Dev IP[%s] Port[%d] Chan[%d]  \
        EventType[%d],Threshold[%.3f]", 
        struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay,
        struAbsTime.dwHour, struAbsTime.dwMinute, struAbsTime.dwSecond, struVQDAlarm.struDevInfo.struDevIP.sIpV4,
        struVQDAlarm.struDevInfo.wPort, struVQDAlarm.struDevInfo.byChannel,struVQDAlarm.dwEventType,struVQDAlarm.fThreshold);
	
    g_pMainDlg->AddLog(iDeviceIndex, ALARM_INFO_T, szInfoBuf);
	
    if (struVQDAlarm.dwPicDataLen > 0 && struVQDAlarm.pImage != NULL)
	{
		int num = ulAlarmPic/1000+1;
		ulAlarmPic++;
		
		char cFilename[256] = {0};
		HANDLE hFile;
		DWORD dwReturn;
        
        SYSTEMTIME t;
        GetLocalTime(&t);
        char chTime[128];
        sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d%3.3d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
		
		sprintf(cFilename, "%s\\%s[%d]", g_struLocalParam.chPictureSavePath, g_struDeviceInfo[iDeviceIndex].chDeviceIPInFileName, num);
		if (GetFileAttributes(cFilename) != FILE_ATTRIBUTE_DIRECTORY)
		{
			CreateDirectory(cFilename, NULL);
		}
        
		sprintf(cFilename, "%s\\LocalTime[%s].jpg",cFilename, chTime);
		hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}
		WriteFile(hFile, struVQDAlarm.pImage, struVQDAlarm.dwPicDataLen, &dwReturn, NULL);
		CloseHandle(hFile);
		hFile = NULL;
	}
	if (struVQDAlarm.pImage != NULL)
	{
		delete [] (struVQDAlarm.pImage);
        struVQDAlarm.pImage = NULL;
	}		
}

// void CClientDemoDlg::OnMenuTs()
// {
// 	int iDeviceIndex = GetCurDeviceIndex();
//     if (iDeviceIndex < 0)
//     {
//         return;
//     }
// 	
//     int iChanIndex = GetCurChanIndex();
//     if (iChanIndex < 0)
//     {
//         iChanIndex = 0;//default to config channel one
//     }
// 	
// 	STRU_CHANNEL_INFO *pChannelInfo = &(g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex]);
// 	int iChanShow = pChannelInfo->iChannelNO;
// 	
// 	CDlgTsSeries dlg;
// 	dlg.m_lLoginID = g_struDeviceInfo[iDeviceIndex].lLoginID;	
// 	dlg.m_iDeviceIndex = iDeviceIndex;			
// 	dlg.m_lChannel = iChanShow;
// 	dlg.DoModal();
// }

// void CClientDemoDlg::OnMenuIPIntell()
// {
// 	// TODO: Add your command handler code here
// 	int iDeviceIndex = GetCurDeviceIndex();
//     if (iDeviceIndex < 0)
//     {
//         return;
//     }
//     
//     
//     int iChanIndex = GetCurChanIndex();
//     if (iChanIndex < 0)
//     {
//         iChanIndex = 0;//default to config channel one
//     }
// 
// 	CDlgIPCSimpIntellCfg dlg;
// 	dlg.m_lLoginID = g_struDeviceInfo[iDeviceIndex].lLoginID;	
// 	dlg.m_iDeviceIndex = iDeviceIndex;			
// 	dlg.m_lChannel = iChanIndex + g_struDeviceInfo[iDeviceIndex].iStartChan;
// 	dlg.m_iChanCount = g_struDeviceInfo[iDeviceIndex].iDeviceChanNum;
// 	dlg.m_iAnaChanCount = g_struDeviceInfo[iDeviceIndex].iAnalogChanNum;
// 	dlg.m_iDStartChannel = g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].dwStartDChan;
// 
// 	dlg.DoModal();
// }

// void CClientDemoDlg::OnMenuIpcSpecialex() 
// {
// 	//2013-06-17
// 	int iDeviceIndex = GetCurDeviceIndex();
//     if (iDeviceIndex < 0)
//     {
//         return;
//     }
//     
//     
//     int iChanIndex = GetCurChanIndex();
//     if (iChanIndex < 0)
//     {
//         iChanIndex = 0;
//     }
// 	
// 	CDlgIPCSpeclalEx dlg;
// 	dlg.m_lLoginID = g_struDeviceInfo[iDeviceIndex].lLoginID;	
// 	dlg.m_iDeviceIndex = iDeviceIndex;			
// 	dlg.m_lChannel = iChanIndex + g_struDeviceInfo[iDeviceIndex].iStartChan;
// 	dlg.m_iChanCount = g_struDeviceInfo[iDeviceIndex].iDeviceChanNum;
// 	dlg.m_iAnaChanCount = g_struDeviceInfo[iDeviceIndex].iAnalogChanNum;
// 	dlg.m_iDStartChannel = g_struDeviceInfo[iDeviceIndex].pStruIPParaCfgV40[0].dwStartDChan;
// 	
// 	dlg.DoModal();
// }

void CClientDemoDlg::OnMENUT1Test() 
{
	// TODO: Add your command handler code here
	CDlgT1test dlg;
	int iDevIndex = GetCurDeviceIndex();
	if (iDevIndex < 0)
	{
		return;
	}
	dlg.m_iDeviceIndex = iDevIndex;
	dlg.DoModal();
}
// void CClientDemoDlg::OnMenuGBT28181Protocol()
// {
// 	int iDeviceIndex = GetCurDeviceIndex();
// 	if (iDeviceIndex < 0)
// 	{
// 		return;
// 	}
// 	CDlgGBT28181ProtocolAccess dlg;
// 	dlg.m_lServerID = g_struDeviceInfo[iDeviceIndex].lLoginID;
// 	dlg.m_dwAlarmOutNum = g_struDeviceInfo[iDeviceIndex].iAlarmOutNum;
// 	dlg.m_dwDevIndex = iDeviceIndex;
// 	dlg.m_nChannel = g_struDeviceInfo[iDeviceIndex].iDeviceChanNum;
// 	dlg.m_lStartChan = g_struDeviceInfo[iDeviceIndex].iStartChan;
// 	dlg.m_dwAlarmInNum = g_struDeviceInfo[iDeviceIndex].iAlarmInNum;
// 	dlg.DoModal();
// }

// void CClientDemoDlg::OnMenuDevServer()
// {
// 	int iDeviceIndex = GetCurDeviceIndex();
// 	if (iDeviceIndex < 0)
// 	{
// 		return;
// 	}
// 	CDlgDevServer dlg;
// 	dlg.m_lServerID = g_struDeviceInfo[iDeviceIndex].lLoginID;
// 	dlg.m_dwAlarmOutNum = g_struDeviceInfo[iDeviceIndex].iAlarmOutNum;
// 	dlg.m_dwDevIndex = iDeviceIndex;
// 	dlg.m_nChannel = g_struDeviceInfo[iDeviceIndex].iDeviceChanNum;
// 	dlg.m_lStartChan = g_struDeviceInfo[iDeviceIndex].iStartChan;
// 	dlg.m_dwAlarmInNum = g_struDeviceInfo[iDeviceIndex].iAlarmInNum;
// 	dlg.DoModal();
// }

void CClientDemoDlg::OnMenuProductionLineTest() 
{
	// TODO: Add your command handler code here
    int iDeviceIndex = GetCurDeviceIndex();
    if (iDeviceIndex < 0)
    {
        return;
    }  
    int iChanIndex = GetCurChanIndex();
    if (iChanIndex < 0)
    {
        iChanIndex = 0;
    }
    CDlgProductionTest dlg; 
    dlg.m_lUserID = g_struDeviceInfo[iDeviceIndex].lLoginID;
    dlg.m_dwChannel = iChanIndex + g_struDeviceInfo[iDeviceIndex].iStartChan;
    dlg.m_iDevIndex = iDeviceIndex;
    dlg.DoModal();
}



void CClientDemoDlg::OnMenuVca() 
{
	// TODO: Add your command handler code here
	DlgVCADetionCfg dlg;
	dlg.DoModal();
}


// void CClientDemoDlg::OnMenuItsIoinCfg() 
// {
// 	// TODO: Add your command handler code here
// 	int iDeviceIndex = GetCurDeviceIndex();
//     if (iDeviceIndex < 0)
//     {
//         return;
//     }  
//     int iChanIndex = GetCurChanIndex();
//     if (iChanIndex < 0)
//     {
//         iChanIndex = 0;
//     }
//     CDlgITSIOinCfg dlg; 
//     dlg.m_lUserID = g_struDeviceInfo[iDeviceIndex].lLoginID;
//     dlg.m_dwChannel = iChanIndex + g_struDeviceInfo[iDeviceIndex].iStartChan;
//     dlg.m_iDevIndex = iDeviceIndex;
//     dlg.DoModal();
// }


void CClientDemoDlg::OnBtntProductCfg()
{   
	CRect rectConfig(0,0,0,0);	
	
	rectConfig.top    = m_iFunBtnHeight-2;
	rectConfig.left   = m_iTreeWidth-2;//-2 cover preview box
	rectConfig.right  = DEMO_FULL_WIDTH - m_iRightWidth ;
	rectConfig.bottom = DEMO_FULL_HEIGHT - m_iListLogHeight;//+4
	
	if (m_iMainType != PRODUCT_RELATE_T)
	{
		m_iMainType = PRODUCT_RELATE_T;
		PreviewReferShow(FALSE);
		ConfigAllHide();

	//	m_dlgPlayBack->HideAll();
		//InitConfigAllRefer();
	//	m_dlgProductCfg->MoveWindow(rectConfig);
	//	m_dlgProductCfg->ShowWindow(SW_SHOW);
		InitProdouctCfgRefer();
		m_dlgProductCfg->UpdateWindowParam(GetCurDeviceIndex(),GetCurChanIndex());
	}
	/*
	int iDeviceIndex = GetCurDeviceIndex();
    if (iDeviceIndex < 0)
    {
        return;
    }  

	if (g_struDeviceInfo[iDeviceIndex].lLoginID < 0)
	{
		AddLog(-1, OPERATION_SUCC_T, "device is offline!");
		return;
	}

	LONG lChannel = -1;
    int iChanIndex = GetCurChanIndex();
    if (iChanIndex >= 0)
    {
        lChannel = g_struDeviceInfo[iDeviceIndex].pStruChanInfo[iChanIndex].iChannelNO;
    }

	CDlgProductCfg dlg;
	dlg.m_lUserID   = g_struDeviceInfo[iDeviceIndex].lLoginID;
    dlg.m_lChannel  = lChannel;
    dlg.m_iDevIndex = iDeviceIndex;
	dlg.DoModal();
	*/
}

void CClientDemoDlg::InitProdouctCfgRefer()
{
	if (!m_dlgProductCfg->IsWindowVisible())
	{
		CRect rectProductCfg(0,0,0,0);	
		
		rectProductCfg.top    = m_iFunBtnHeight-2;
		rectProductCfg.left   = m_iTreeWidth-2;//-2 cover preview box
		rectProductCfg.right  = DEMO_FULL_WIDTH;
		rectProductCfg.bottom = DEMO_FULL_HEIGHT - m_iListLogHeight;//+4
		
		m_dlgProductCfg->MoveWindow(&rectProductCfg,TRUE);
		m_dlgProductCfg->ShowWindow(SW_SHOW);
	}
}
