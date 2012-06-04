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
#ifndef TUX_SERVICE_REPOSITORY_H__
#define TUX_SERVICE_REPOSITORY_H__
#include <lwpr.h>
#include <atmi.h>
#include <list>
#include <vector>
#include <string>

typedef struct server_service_item_t
{
	std::string		strSVCName;
	SVCFUNC_FP		fpSVCFunc;
} SERVER_SERVICE_ITEM_T;

typedef std::list<SERVER_SERVICE_ITEM_T> ServerServiceList;

typedef std::map<std::string, SERVER_SERVICE_ITEM_T> ServiceMap;

typedef std::vector<std::string> ServiceNameVector;

class ServiceRepository
{
public:

	ServiceRepository();
	~ServiceRepository();
	static ServiceRepository* GetInstance();

	void Register(const ServerServiceList& svcs);

	void Unregister(const char* svcname);

	/**
	 * ����Service ���ƻ�ȡ��Ӧ�ĺ���ָ�루���������쳣������
	 */
	SVCFUNC_FP GetSvcFunction(const char* name);

	/**
	 * ��ȡ��Ҫע���Service ����
	 */
	void GetSvcListWill(ServiceNameVector& svcs);

	/**
	 * �Ѿ�ע����ɵ�Service
	 */
	void RegisterSvcDone(const ServiceNameVector& svcs);

	/**
	 * ���Ѿ�ע���SVC���δע��״̬
	 */
	void MakeSvcRegister();
private:
	LWPR::Mutex			m_Mutex;
	ServiceMap			m_SVCWill;		// ��Ҫ��Gateע���Service
	ServiceMap			m_SVCAlready;	// �Ѿ���Gateע���˵�Service
};

#endif // end of TUX_SERVICE_REPOSITORY_H__
