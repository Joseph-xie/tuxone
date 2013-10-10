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
#include "TmshutdownCmdline.h"
#include <lwpr.h>
#include <map>
#include <iostream>
#include <string>
#include "BulletinBoard.h"
#include <stdlib.h>
#include <string.h>

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char *argv[])
{
	if(!TmshutdownCmdline::GetInstance()->ParseCmdlineOption(argc, argv, "y"))
	{
		printf("Usage: %s [-y]\n", argv[0]);
		return -1;
	}

	// ��ȡ��ǰϵͳ����Ȩ�޵�Gate
	BBHeaderMap headers;
	BulletinBoard::FetchGateIPCID(headers);

	// ���Թر�Gate
	for(BBHeaderMap::iterator it = headers.begin(); it != headers.end(); it++)
	{
		// û����ʾֱ�ӹر�
		bool bShutdown = TmshutdownCmdline::GetInstance()->IsPromptingSuppressed();

		// ���ڽ����Ѿ�����������IPC��Դû�������������д���
		if(!LWPR::ProcessUtil::IsProcessExist(it->second.nGatePID))
		{
			bShutdown = true;
		}

		if(!bShutdown)
		{
			while(1)
			{
				printf("Do you want to shutdown the gate <%s>? [YES/NO]", LWPR::Socket::AddrTransform(it->second.tServerAddr).c_str());
				char bufInput[128] = {0};
				const char* pInput = fgets(bufInput, sizeof(bufInput) - 1, stdin);
				if(NULL != pInput)
				{
					LWPR::StringUtil::TrimAll(bufInput);

					bShutdown = (!strcmp(pInput, "YES")
					             || !strcmp(pInput, "yes")
					             || !strcmp(pInput, "Y")
					             || !strcmp(pInput, "y"));

					bool bNotShutdown = (!strcmp(pInput, "NO")
					                     || !strcmp(pInput, "no")
					                     || !strcmp(pInput, "N")
					                     || !strcmp(pInput, "n"));

					if(bShutdown || bNotShutdown)
					{
						break;
					}
				}
			}
		}

		if(bShutdown)
		{
			kill(it->second.nGatePID, SIGTERM);

			// ��ͼ�ȴ�����ʱ��
			const int MAX_WAIT_TIMES = 5;
			bool bHasProcssAlive = true;
			for(int i = 0; i < MAX_WAIT_TIMES && bHasProcssAlive; i++)
			{
				bHasProcssAlive = LWPR::ProcessUtil::IsProcessExist(it->second.nGatePID);
				if(bHasProcssAlive)
				{
					LWPR::Thread::Sleep(1);
				}
			}

			// ǿ��KILL txgate���̣���WSH��
			if(bHasProcssAlive)
			{
				// KILL �ػ�����
				kill(it->second.nGatePID, SIGKILL);

				// TODO: KILL WSH����
			}

			// ���txgate�˳���IPCIDû�����������tmshutdown�����Զ����
			char bufCmdLine[64] = {0};
			sprintf(bufCmdLine, "ipcrm -m %d -s %d 2> /dev/null", it->first, it->second.nSEMID);
			system(bufCmdLine);
		}
	}

	return 0;
}
