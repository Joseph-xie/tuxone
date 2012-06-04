/**
 * Copyright 2012 Wangxr, vintage.wang@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "AppContext.h"
#include "AppConfig.h"
#include "GateCheckThread.h"
#include "ServiceRepository.h"
#include "AppNSHandlerImpl.h"
#include "LocalServiceStub.h"
#include "ATMIBufferManager.h"
#include "TLSLibtux.h"
#include "View32Manager.h"
#include "FML32API.h"
#include <stdlib.h>
#include <lwpr.h>

AppContext::AppContext():
	m_bFirstInit(true),
	m_bInited(false),
	m_bFirstRegister(true),
	m_bFirstCall(true),
	m_pNetServer(NULL)
{
}

AppContext::~AppContext()
{
}

int AppContext::tpinit(TPINIT *tpinfo)
{
	DEBUG_FUNCTION();

	if(!m_bInited)
	{
		// ����ǵ�һ�γ�ʼ��
		if(m_bFirstInit)
		{
			// �´β���ʼ��
			m_bFirstInit = false;

			// �����ض��ź�
			LWPR::Utility::RegisterSignal(SIGCHLD, SIG_IGN);
			LWPR::Utility::RegisterSignal(SIGPIPE, SIG_IGN);

			// ��ʼ�������ļ�ģ��
			AppConfig::GetInstance()->ConfigInit();
			AppConfig::GetInstance()->PrintConfig();

			// ��ʼ����־�ļ�ģ��
			LWPR::LOGGER_OPTION_T optLogger;
			optLogger.strLogFileName = GetLogFileName();
			optLogger.nLogLevel = AppConfig::GetInstance()->TUXONE_APP_LOG_LEVEL();
			optLogger.bPrintWhere = AppConfig::GetInstance()->TUXONE_APP_LOG_PRINTWHERE();;
			optLogger.bPrintFile = AppConfig::GetInstance()->TUXONE_APP_LOG_ENABLE() && AppConfig::GetInstance()->TUXONE_APP_LOG_PRINTFILE();
			optLogger.bPrintConsole = AppConfig::GetInstance()->TUXONE_APP_LOG_ENABLE() && AppConfig::GetInstance()->TUXONE_APP_LOG_PRINTCONSOLE();
			logger->Initialize(optLogger);

			// ��ʼ��Gate�߳�
			if(!GateCheckThread::GetInstance()->Init())
			{
				errno = TPEOS;
				return -1;
			}

			// ����View32�ļ�
			View32Manager::GetInstance()->LoadStruct();

			// �����߳�
			GateCheckThread::GetInstance()->Start();
		}
		// ��N ����1�γ�ʼ��
		else
		{
			GateCheckThread::GetInstance()->Go();
		}

		m_bInited = true;
	}
	else
	{
		errno = TPESYSTEM;
		return -1;
	}

	return 0;
}

int AppContext::tpterm()
{
	DEBUG_FUNCTION();

	m_bInited = false;

	// �Ͽ���Gate������
	GateCheckThread::GetInstance()->Term();

	// �Ͽ���Server������
	LocalServiceStub::GetInstance()->DisconnectAll();

	return 0;
}

int AppContext::tpcall(char *svc, char *idata, long ilen, char **odata, long *olen, long flags)
{
	DEBUG_FUNCTION();

	// �������
	if(svc == NULL
	   || strlen(svc) == 0
	   || odata == NULL
	   || *odata == NULL
	   || olen == NULL)
	{
		tperrno = TPEINVAL;
		return -1;
	}

	// ��ʼ��֮��ִ��
	if(!m_bInited)
	{
		tperrno = TPEPROTO;
		return -1;
	}

	// �������ΪClient
	if(m_bFirstCall)
	{
		m_bFirstCall = false;
		GateCheckThread::GetInstance()->MarkClient();
	}

	// ����Service
	bool bDownload = GateCheckThread::GetInstance()->HasGateNotice() ?
	                 GateCheckThread::GetInstance()->DownloadService() : false;

	// ��ȡStub
	ServerProxyVector servers;
	ServerProxyVectorHolder holder(servers);
	bool bFound = LocalServiceStub::GetInstance()->PickServers(svc, holder)
	              || !(bDownload || (bDownload = GateCheckThread::GetInstance()->DownloadService()))
	              || LocalServiceStub::GetInstance()->PickServers(svc, holder);
	// û�п��õ�Server
	if(!bFound)
	{
		// ����Gate����
		if(!bDownload)
		{
			logger->warn(LTRACE, "No gate found");
			tperrno = TPENOENT;
		}
		else
		{
			logger->warn(LTRACE, "No server found");
			tperrno = TPENOENT;
		}

		return -1;
	}

	// ��֯����
	TUX_APP_TPCALL_REQ_T request;
	TUX_APP_TPCALL_REP_T response;
	request.strSvcName = svc;
	request.nFlags = flags;
	request.pReqBuf = idata;
	request.nReqLen = ilen;
	request.bHasReq = ATMIBufferManager::GetInstance()->GetBufferTypes((char *)request.pReqBuf, request.nReqBufType, request.strReqBufTypeSub);

	if(request.bHasReq)
	{
		if(TYPED_BUFFER_STRING == request.nReqBufType)
		{
			request.nReqLen = strlen(request.pReqBuf);
		}
		else if(TYPED_BUFFER_VIEW32 == request.nReqBufType)
		{
			if(!View32Manager::GetInstance()->GetStructInfo(request.strReqBufTypeSub.c_str(), request.tStructInfo))
			{
				tperrno = TPEITYPE;
				return -1;
			}

			request.nReqLen = request.tStructInfo.structSize;
		}
		else if(TYPED_BUFFER_FML32 == request.nReqBufType)
		{
			request.nReqLen = FMLValidSize((FBFR32 *)request.pReqBuf);
			FMLHeaderToNet(request.pReqBuf);
		}
	}

	// ִ�и��ؾ���
	bool bResult = false, bTimeout = false;
	size_t len = servers.size();
	for(size_t i = 0; i < len && !bTimeout && !bResult; i++)
	{
		LWPR::UINT32 index = TLSLibtux::GetInstance()->GetNextBalanceFactor();

		TUX_INVOKE_RESULT_E nInvokeResult = servers[index % len]->CallServer(request, response);
		switch(nInvokeResult)
		{
		case TUX_INVOKE_CONNECT_FAILED:
		case TUX_INVOKE_SEND_HEADER_FAILED:
		case TUX_INVOKE_SEND_HEADER_TIMEOUT:
		case TUX_INVOKE_SEND_BODY_FAILED:
			break;
		case TUX_INVOKE_SEND_BODY_TIMEOUT:
		case TUX_INVOKE_RECEIVE_FAILED:
		case TUX_INVOKE_RECEIVE_TIMEOUT:
		case TUX_INVOKE_RECEIVE_PROTOCOL_ERROR:
		case TUX_INVOKE_RECEIVE_OS_ERROR:
			bTimeout = true;
			break;
		case TUX_INVOKE_OK:
			bResult = true;
			break;
		default:
			assert(0);
		}

		logger->debug(LTRACE, "call server result %d %d", (int) nInvokeResult, (int) response.MsgHeader().nMsgRetCode);
	}

	// �����ؽ��
	if(bResult)
	{
		if(TUX_MSG_RET_CODE_OK == response.MsgHeader().nMsgRetCode)
		{
			if(response.bHasRep)
			{
				// У�鷵��Buffer������Ҫ����Ƿ�һ��
				TYPED_BUFFER_E repBufType;
				std::string repBufTypeSub;
				if(!ATMIBufferManager::GetInstance()->GetBufferTypes((char *)*odata, repBufType, repBufTypeSub))
				{
					tperrno = TPEOTYPE;
					return -1;
				}

				if(response.nRepBufType != repBufType
				   || response.strRepBufTypeSub != repBufTypeSub)
				{
					tperrno = TPEOTYPE;
					return -1;
				}

				if(TYPED_BUFFER_VIEW32 == response.nRepBufType)
				{
					LWPR::Buffer_var vBufResult = new LWPR::Buffer(1024);
					bool bRetResult = View32Manager::GetInstance()->AdjustStructData(response.strRepBufTypeSub.c_str(),
					                  response.vRepBuf->Inout(),
					                  response.tStructInfo,
					                  response.MsgHeader().IsBigEndian(),
					                  *vBufResult);

					if(!bRetResult)
					{
						logger->error(LTRACE, "AdjustStructData error [%s]", response.strRepBufTypeSub.c_str());
						tperrno = TPEOTYPE;
						return -1;
					}

					response.vRepBuf = vBufResult;
				}
				else if(TYPED_BUFFER_FML32 == response.nRepBufType)
				{
					FMLHeaderToLocal(response.vRepBuf->Inout());
				}

				*odata = ATMIBufferManager::GetInstance()->ReallocateBuffer(*odata, response.vRepBuf->Size() + 1);
				if(*odata != NULL)
				{
					memcpy(*odata, response.vRepBuf->Inout(), response.vRepBuf->Size());

					// Ϊ�ַ���ĩβ��\0
					if(TYPED_BUFFER_STRING == response.nRepBufType)
					{
						*(*odata + response.vRepBuf->Size()) = 0;
					}

					*olen = response.vRepBuf->Size();
				}
				else
				{
					tperrno = TPESYSTEM;
					return -1;
				}
			}

			tperrno = 0;
			return 0;
		}
		else if(TUX_MSG_RET_CODE_SVC_NOT_FOUND == response.MsgHeader().nMsgRetCode)
		{
			tperrno = TPENOENT;
			return -1;
		}
		else if(TUX_MSG_RET_CODE_SVC_FAIL == response.MsgHeader().nMsgRetCode)
		{
			tperrno = TPESVCFAIL;
			return -1;
		}
		else if(TUX_MSG_RET_CODE_SVC_ERR == response.MsgHeader().nMsgRetCode)
		{
			tperrno = TPESVCERR;
			return -1;
		}
		else
		{
			tperrno = TPESVCERR;
			return -1;
		}
	}
	else if(bTimeout)
	{
		tperrno = TPETIME;
		return -1;
	}
	else
	{
		logger->warn(LTRACE, "All server break down");
		tperrno = TPENOENT;
		return -1;
	}

	return 0;
}

int AppContext::tpregister(const TMSVCFUNC* svcs, long len, long flags)
{
	DEBUG_FUNCTION();

	// ������Ч�Լ��
	if(len <= 0)
	{
		tperrno = TPEINVAL;
		return -1;
	}

	// ������Ч�Լ��
	ServerServiceList svcsTmp;
	for(int i = 0; i < len; i++)
	{
		if(svcs[i].svcname == NULL
		   || strlen(svcs[i].svcname) == 0
		   || svcs[i].svcfunc == NULL)
		{
			tperrno = TPEINVAL;
			return -1;
		}
		else
		{
			SERVER_SERVICE_ITEM_T item;
			item.strSVCName = svcs[i].svcname;
			item.fpSVCFunc = svcs[i].svcfunc;
			svcsTmp.push_back(item);
		}
	}

	// ע��Service������
	ServiceRepository::GetInstance()->Register(svcsTmp);

	// ����Server���񣨴˲��ַ��̰߳�ȫ��
	if(m_bFirstRegister)
	{
		m_bFirstRegister = false;

		// ����NetServer����
		LWPR::NET_SERVER_OPTION_T opt;
		opt.strAddr = AppConfig::GetInstance()->TUXONE_APP_SERVER_LISTENER_ADDR();
		opt.nThreadPoolMin = AppConfig::GetInstance()->TUXONE_APP_SERVER_THREADPOOL_THREADMIN();
		opt.nThreadPoolMax = AppConfig::GetInstance()->TUXONE_APP_SERVER_THREADPOOL_THREADMAX();
		opt.nThreadPoolMaxIdle = AppConfig::GetInstance()->TUXONE_APP_SERVER_THREADPOOL_THREADMAXIDLE();
		opt.nConnectionMax = AppConfig::GetInstance()->TUXONE_APP_SERVER_CONCURRENTMAX();
		opt.nSelectTimeout = 60;				// ����ģ�͸��������رչ���Socket
		opt.nSocketExpiredTime = 120;			// ���opt.nSelectTimeout == 0����ѡ�������
		opt.pHandler = new AppNSHandlerImpl();
		m_pNetServer = LWPR::NetServer::CreateNetServer(AppConfig::GetInstance()->TUXONE_APP_SERVER_THREADMODE() == AutoConfig::MT
		               ? LWPR::NET_SERVER_MT : LWPR::NET_SERVER_ST, opt);

		// ������񣬿�ʼ�����˿�
		m_pNetServer->ActivateService();

		GateCheckThread::GetInstance()->SetNetServer(m_pNetServer);
		GateCheckThread::GetInstance()->Wakeup();
	}

	return 0;
}

int AppContext::tprun(long flags)
{
	DEBUG_FUNCTION();

	if(m_pNetServer != NULL)
	{
		m_pNetServer->Go();
	}
	else
	{
		tperrno = TPESYSTEM;
		return -1;
	}

	return 0;
}

std::string AppContext::GetLogFileName()
{
	std::string strName;

	const char* pEnv = getenv("TUXLOGDIR");
	if(pEnv && strlen(pEnv) > 0)
	{
		strName = pEnv;
		strName += "/";
		LWPR::FileUtil::BuildPath(strName.c_str());
	}

	strName += "ULOG." + (LWPR::ProcessUtil::GetPName() + "." + LWPR::StringUtil::IntToStr(LWPR::ProcessUtil::GetPID()) + ".log");

	return strName;
}

int AppContext::tpadvertise(char *svcname, void (*func)(TPSVCINFO *))
{
	if(NULL == svcname
	   || strlen(svcname) == 0
	   || svcname[0] == '.'
	   || NULL == func)
	{
		tperrno = TPEINVAL;
		return -1;
	}

	TMSVCFUNC item = {0};
	item.svcname = svcname;
	item.svcfunc = func;

	return tpregister(&item, 1, 0);
}

int AppContext::tpunadvertise(char *svcname)
{
	if(NULL == svcname
	   || strlen(svcname) == 0
	   || svcname[0] == '.')
	{
		tperrno = TPEINVAL;
		return -1;
	}

	ServiceRepository::GetInstance()->Unregister(svcname);

	return 0;
}
