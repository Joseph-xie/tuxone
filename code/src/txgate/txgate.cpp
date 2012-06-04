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
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <lwpr.h>
#include "GateConfig.h"
#include "BulletinBoard.h"
#include "ShareCookie.h"

using std::cout;
using std::cerr;
using std::endl;

extern int wsh_main(int argc, char** argv);
extern int sync_main(int argc, char** argv);

static volatile bool g_bTerminated = false;
static volatile LWPR::PID_T g_nSyncPID = -1;

static void DoSignalHandle(int sig)
{
	logger->fatal(LTRACE, "txgate catched signal %d", sig);
	g_bTerminated = true;
}

static void CreateWSH(int argc, char** argv)
{
	DEBUG_FUNCTION();

	LWPR::PID_T pid = fork();
	// �ӽ���
	if(pid == 0)
	{
		exit(wsh_main(argc, argv));
	}
	// ������
	else if(pid > 0)
	{
		logger->info(LTRACE, "fork a wsh process [%d]", pid);
	}
	// ����
	else if(pid == -1)
	{
		// TODO:
		logger->fatal(LTRACE, "fork error");
	}
}

static void CreateSYNC(int argc, char** argv)
{
	DEBUG_FUNCTION();

	LWPR::PID_T pid = fork();
	// �ӽ���
	if(pid == 0)
	{
		exit(sync_main(argc, argv));
	}
	// ������
	else if(pid > 0)
	{
		g_nSyncPID = pid;
		logger->info(LTRACE, "fork a sync process [%d]", pid);
	}
	// ����
	else if(pid == -1)
	{
		g_nSyncPID = pid;
		logger->fatal(LTRACE, "fork error");
	}
}

static void DoHousekeeping(int argc, char** argv)
{
	DEBUG_FUNCTION();

	try
	{
		// �����ӽ���
		LWPR::PIDList pids;
		while(1)
		{
			LWPR::PID_T pid = waitpid(-1, NULL, WNOHANG);
			if(pid > 0)
			{
				if(pid == g_nSyncPID)
				{
					CreateSYNC(argc, argv);
				}
				else
				{
					pids.push_back(pid);
				}
			}
			else
			{
				break;
			}
		}

		// ���������¼
		BulletinBoard::GetInstance()->CleanupWSHByPID(pids);

		// ��ȡ����WSH������Ϣ����У��
		BBWSHList wshs;
		int nWSHCount = BulletinBoard::GetInstance()->GetAllWSH(wshs);
		BBIndexList exitedwsh;
		for(int i = 0; i < nWSHCount; i++)
		{
			if(!LWPR::ProcessUtil::IsProcessExist(wshs[i].nPID))
			{
				exitedwsh.push_back(wshs[i].nIndex);
			}
		}

		// ���������¼
		BulletinBoard::GetInstance()->CleanupWSHByIndex(exitedwsh);

		// ���㵱ǰ�ͻ���������
		nWSHCount = BulletinBoard::GetInstance()->GetAllWSH(wshs);
		int nConnectionTotal = 0;
		for(int k = 0; k < nWSHCount; k++)
		{
			nConnectionTotal += wshs[k].nConnectionTotal;
		}

		// ���㵱ǰϵͳ֧�ֵĲ�����
		int nWSHAccessMax = GateConfig::GetInstance()->TUXONE_GATE_WSH_ACCESSMAX() >= FD_SETSIZE ?
		                    GateConfig::GetInstance()->TUXONE_GATE_WSH_ACCESSMAX() - LWPR::NET_SERVER_RESERVED_FDS :
		                    GateConfig::GetInstance()->TUXONE_GATE_WSH_ACCESSMAX();
		int nConcurrentSupported = nWSHAccessMax * nWSHCount;

		// ���ڵ�ˮλ����Ҫ����WSH����
		int nReservedConnect = nConcurrentSupported - nConnectionTotal;
		logger->debug(LTRACE, "reserved connection num %d", nReservedConnect);
		if((nReservedConnect <= GateConfig::GetInstance()->TUXONE_GATE_CONCURRENT_LOWWATER())
		   && nWSHCount < GateConfig::GetInstance()->TUXONE_GATE_WSH_MAX())
		{
			CreateWSH(argc, argv);
		}
		else if((nReservedConnect >= GateConfig::GetInstance()->TUXONE_GATE_CONCURRENT_HIGHWATER())
		        && nWSHCount >= GateConfig::GetInstance()->TUXONE_GATE_WSH_MIN())
		{
			for(int k = 0; k < nWSHCount; k++)
			{
				if(wshs[k].nConnectionTotal == 0)
				{
					// TODO: ɱ��WSH����
				}
			}
		}
	}
	catch(LWPR::Exception& e)
	{
		logger->error(LTRACE, "%s", e.what());
	}
	catch(std::exception& e)
	{
		logger->error(LTRACE, "%s", e.what());
	}
	catch(...)
	{
		logger->error(LTRACE, "Unknow exception");
	}
}

static void DoDaemon(int argc, char** argv)
{
	DEBUG_FUNCTION();

	// ����WSH����
	for(int i = 0; !g_bTerminated && i < GateConfig::GetInstance()->TUXONE_GATE_WSH_MIN(); i++)
	{
		CreateWSH(argc, argv);
	}

	// ����SYNC����
	CreateSYNC(argc, argv);

	while(!g_bTerminated)
	{
		fd_set fds;
		struct timeval tv = {0};
		FD_ZERO(&fds);

		FD_SET(ShareCookie::GetInstance()->FdUDPListening(), &fds);
		tv.tv_sec = GateConfig::GetInstance()->TUXONE_GATE_HOUSEKEEPING_INTERVAL();
		int ret = select(ShareCookie::GetInstance()->FdUDPListening() + 1, &fds, NULL, NULL, &tv);
		// �յ�UDP��Ϣ
		if(ret > 0)
		{
			logger->debug(LTRACE, "select OK, receive UDP data");
			// ������Ϣ�����;��幤��
			DoHousekeeping(argc, argv);
		}
		// ��ʱ
		else if(ret == 0)
		{
			logger->debug(LTRACE, "select timeout");
			DoHousekeeping(argc, argv);
		}
		// �������󣨿������ź��жϣ�
		else if(ret == -1)
		{
			logger->error(LTRACE, "select error");
		}
	}
}

static std::string GetLogFileName()
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

#ifndef TUXONE_TRIAL_TIME
#define TUXONE_TRIAL_TIME 365
#endif

static int run(int argc, char** argv)
{
	// У��License
	if(!LWPR::Utility::HasLicenseEffect(TUXONE_TRIAL_TIME))
	{
		fprintf(stderr, "Your License is expired, Please contact vintage.wang@gmail.com!\n");
		return -1;
	}

	// �źŴ���
	LWPR::Utility::RegisterSignal(SIGINT, DoSignalHandle);
	LWPR::Utility::RegisterSignal(SIGTERM, DoSignalHandle);
	LWPR::Utility::RegisterSignal(SIGHUP, DoSignalHandle);
	LWPR::Utility::RegisterSignal(SIGQUIT, DoSignalHandle);
	LWPR::Utility::RegisterSignal(SIGCHLD, SIG_IGN);
	LWPR::Utility::RegisterSignal(SIGPIPE, SIG_IGN);

	// ��ʼ�������ļ�ģ��
	GateConfig::GetInstance()->ConfigInit(argc, argv);

	// ��ʼ����־�ļ�ģ��
	LWPR::LOGGER_OPTION_T optLogger;
	optLogger.strLogFileName = GetLogFileName();
	optLogger.nLogLevel = GateConfig::GetInstance()->TUXONE_GATE_LOG_LEVEL();
	optLogger.bPrintWhere = GateConfig::GetInstance()->TUXONE_GATE_LOG_PRINTWHERE();
	optLogger.bPrintFile = GateConfig::GetInstance()->TUXONE_GATE_LOG_ENABLE() && GateConfig::GetInstance()->TUXONE_GATE_LOG_PRINTFILE();
	optLogger.bPrintConsole = GateConfig::GetInstance()->TUXONE_GATE_LOG_ENABLE() && GateConfig::GetInstance()->TUXONE_GATE_LOG_PRINTCONSOLE();
	logger->Initialize(optLogger);

	// ����TCP�����˿ں�
	LWPR::SOCKET_FD_T nFdTCPListening = LWPR::Socket::CreateServerTCPSocket(GateConfig::GetInstance()->TUXONE_GATE_LISTENER_ADDR().c_str());
	if(-1 == nFdTCPListening)
	{
		logger->fatal(LTRACE, "create TCP socket error");
		// TODO: �׳��쳣
		return -1;
	}

	// ����UDP�����˿ں�
	LWPR::UINT16 nPort = 0;
	LWPR::SOCKET_FD_T nFdUDPListening = LWPR::Socket::CreateServerUDPSocket(nPort);
	if(-1 == nFdUDPListening)
	{
		logger->fatal(LTRACE, "create UDP socket error");
		return -1;
	}

	// ������������ShareCookie�����ӽ���ʹ��
	ShareCookie::GetInstance()->FdTCPListening(nFdTCPListening);
	ShareCookie::GetInstance()->FdUDPListening(nFdUDPListening);
	ShareCookie::GetInstance()->UDPPort(nPort);

	// ���������
	BulletinBoard::GetInstance()->Create(GateConfig::GetInstance()->TUXONE_GATE_WSH_MAX(),
	                                     GateConfig::GetInstance()->TUXONE_GATE_BBL_BERTHMAX());

	// ��ʼ�������
	BB_HEADER_ENTITY_T header;
	header.nMagicCode = BB_HEADER_MAGIC_CODE;
	header.nSVCUpdateSeq = 0;
	header.nGatePID = LWPR::Utility::GetPid();
	header.nUDPPort = ShareCookie::GetInstance()->UDPPort();
	LWPR::Socket::AddrTransform(GateConfig::GetInstance()->TUXONE_GATE_LISTENER_ADDR().c_str(), header.tServerAddr);
	BulletinBoard::GetInstance()->SetHeader(header);

	// ��ӡ��Ȩ��Ϣ
	printf("TUXONE Gate Version 2.0 Copyright vintage.wang@gmail.com <http://tuxone.googlecode.com>\n");
	GateConfig::GetInstance()->PrintConfig();

	// �������¼�ѭ��
	DoDaemon(argc, argv);

	// �����˳���ִ����Դ������
	BBWSHList wshs;
	int nWSHCount = BulletinBoard::GetInstance()->GetAllWSH(wshs);
	for(int i = 0; i < nWSHCount; i++)
	{
		// TODO: ɱ������WSH���̣�Ҫ��ͨ��UDP����֪ͨ��Ȼ��ȴ�ָ��ʱ��������̻�û���˳�����ǿ��ɱ��
		if(wshs[i].nPID > 0)
		{
			kill(wshs[i].nPID, SIGKILL);
			logger->debug(LTRACE, "Send signal [SIGKILL] to wsh Process %d", wshs[i].nPID);
		}
	}

	// ɱ��ͬ������
	if(-1 != g_nSyncPID)
	{
		kill(g_nSyncPID, SIGKILL);
		logger->debug(LTRACE, "Send signal [SIGKILL] to sync Process %d", g_nSyncPID);
	}

	// �����ӽ���
	while(waitpid(-1, NULL, WNOHANG) > 0);

	BulletinBoard::GetInstance()->Destroy();

	return 0;
}

int main(int argc, char** argv)
{
	try
	{
		return run(argc, argv);
	}
	catch(LWPR::Exception& e)
	{
		cerr << e.what() << endl;
	}
	catch(std::exception& e)
	{
		cerr << e.what() << endl;
	}
	catch(...)
	{
		cerr << "Unknow exception" << endl;
	}

	return -1;
}
