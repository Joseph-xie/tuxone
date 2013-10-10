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
/*
 * ���ܣ� �����̳��򣬼�������gate�����䶯�����
 *       �����ȡ����gate��������ݣ���ͬ����gate����壬�ǰ�������gate�������������ͬ��
 *       ���ֱ�����
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lwpr.h>
#include "GateManager.h"
#include "ShareCookie.h"
#include "BulletinBoard.h"
#include "GateConfig.h"

using std::cout;
using std::cerr;
using std::endl;

static void DoSignalHandle(int sig)
{
	logger->fatal(LTRACE, "txsync catched signal %d", sig);
	GateManager::GetInstance()->Stop();
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

	strName += ("ULOG.txsync." + LWPR::StringUtil::IntToStr(LWPR::ProcessUtil::GetPID()) + ".log");

	return strName;
}

static int run(int argc, char** argv)
{
	// �źŴ���
	LWPR::Utility::RegisterSignal(SIGINT, DoSignalHandle);
	LWPR::Utility::RegisterSignal(SIGTERM, DoSignalHandle);
	LWPR::Utility::RegisterSignal(SIGHUP, DoSignalHandle);
	LWPR::Utility::RegisterSignal(SIGQUIT, DoSignalHandle);
	LWPR::Utility::RegisterSignal(SIGCHLD, SIG_IGN);
	LWPR::Utility::RegisterSignal(SIGPIPE, SIG_IGN);

	// ��ʼ����־�ļ�ģ��
	LWPR::LOGGER_OPTION_T optLogger;
	optLogger.strLogFileName = GetLogFileName();
	optLogger.nLogLevel = GateConfig::GetInstance()->TUXONE_GATE_LOG_LEVEL();
	optLogger.bPrintWhere = GateConfig::GetInstance()->TUXONE_GATE_LOG_PRINTWHERE();
	optLogger.bPrintFile = GateConfig::GetInstance()->TUXONE_GATE_LOG_ENABLE() && GateConfig::GetInstance()->TUXONE_GATE_LOG_PRINTFILE();
	optLogger.bPrintConsole = GateConfig::GetInstance()->TUXONE_GATE_LOG_ENABLE() && GateConfig::GetInstance()->TUXONE_GATE_LOG_PRINTCONSOLE();
	logger->Initialize(optLogger);

	// ��ʼͬ������
	GateManager::GetInstance()->Run();

	return 0;
}

extern int sync_main(int argc, char** argv)
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
