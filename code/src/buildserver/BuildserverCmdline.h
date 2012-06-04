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
#ifndef TUX_BUILDSERVER_CMDLINE_H__
#define TUX_BUILDSERVER_CMDLINE_H__

#include <lwpr.h>

class BuildserverCmdline : public LWPR::ProgramOption
{
    BuildserverCmdline();

public:
	static BuildserverCmdline* GetInstance();

	/**
	 * [-C]
	 * �Ƿ��Ǳ���COBOL����
	 */
	bool IsCOBOLCompilation();

	/**
	 * [-s services[:func[()]]]
	 * ��ȡ������
	 */
	bool GetServiceNames(std::vector<std::string>& services);

	/**
	 * [-k]
	 * �Ƿ������ɵĴ���main������Դ�ļ�
	 */
	bool IsKeepMainStubFile();

	/**
	 * [-t]
	 * �Ƿ�֧�ֶ��߳�
	 */
	bool IsMultithreading();

	/**
	 * [-v]
	 * �Ƿ������̴�ӡ�������Ϣ
	 */
	bool IsVerboseMode();

	/**
	 * [-r rmname]
	 * ��ȡrmname
	 */
	bool GetRMNAME(std::string& rmname);

	/**
	 * [-o outfile]
	 * ��ȡĿ���ļ�����Ĭ��ֵ��a.out��
	 */
	std::string GetOutfile();

	/**
	 * [-f firstfiles]
	 * ��ȡ������TUXEDOϵͳ��֮ǰ���ӵ��ļ����б�
	 */
	bool GetFirstFiles(std::string& firstfiles);

	/**
	 * [-l lastfiles]
	 * ��ȡҪ��TUXEDOϵͳ��֮�����ӵ��ļ���
	 */
	bool GetLastFiles(std::string& lastfiles);

private:
	bool CheckCmdline();

private:
};

#endif // end of TUX_BUILDSERVER_CMDLINE_H__
