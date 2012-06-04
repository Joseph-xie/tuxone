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
#ifndef TX_VIEW32_MANAGER_H__
#define TX_VIEW32_MANAGER_H__

#include "View32Util.h"
#include <lwpr.h>

class View32Manager
{
public:

	View32Manager();
	~View32Manager();
	static View32Manager* GetInstance();

	/**
	 * ����.V�ļ�
	 */
	void LoadStruct();

	/**
	 * �ṹ���Ƿ����
	 */
	bool IsStructExist(const char* name, int& size);

	/**
	 * ��ȡ�ṹ����Ϣ
	 */
	bool GetStructInfo(const char* name, STRUCT_INFO_T& info);

	/**
	 * �����ṹ��
	 */
	bool AdjustStructData(const char* name,
	                      const void* indata,
	                      const STRUCT_INFO_T& ininfo,
	                      const bool inbigendian,
	                      LWPR::Buffer& outdata);

private:

	StructInfoMap m_Structs;
};

#endif // end of TX_VIEW32_MANAGER_H__
