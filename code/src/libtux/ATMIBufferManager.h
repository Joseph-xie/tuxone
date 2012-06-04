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
#ifndef TUX_ATMIBUFFER_MANAGER_H__
#define TUX_ATMIBUFFER_MANAGER_H__

#include <map>
#include <string>
#include <lwpr.h>
#include "Message.h"

typedef struct atmi_buffer_item_t
{
	int nCurrentSize;				// ��ǰBuffer��С
	int nActualSize;				// ��ǰ�ڴ��ʵ�ʴ�С
	TYPED_BUFFER_E eBufferType;	// ��ǰ�ڴ������
	std::string strSubType;			// ��ǰ�ڴ��������
	bool bIsInUse;					// ��ǰ�ڴ���Ƿ�����ʹ��
} ATMI_BUFFER_ITEM_T;

class ATMIBufferManager
{
public:
	/**
	 * ���캯��
	 */
	ATMIBufferManager();

	/**
	 * ��������
	 */
	~ATMIBufferManager();

	/**
	 * ��������
	 */
	static ATMIBufferManager* GetInstance();

	/**
	 * �����ڴ��
	 */
	char* AllocateBuffer(char *type, char *subtype, long size);

	/**
	 * ���·����ڴ��
	 */
	char* ReallocateBuffer(char *ptr, long size);

	/**
	 * ��ȡ�ڴ������
	 */
	long GetBufferTypes(char *ptr, char *type, char *subtype);

	bool GetBufferTypes(char *ptr, TYPED_BUFFER_E& type);

	bool GetBufferTypes(char *ptr, TYPED_BUFFER_E& type, std::string& subtype);

	/**
	 * �ͷ��ڴ��
	 */
	void FreeBuffer(const char *ptr);

private:
	typedef std::map< char*, ATMI_BUFFER_ITEM_T > ATMIBufferMap;

	LWPR::Mutex		m_Mutex;
	ATMIBufferMap	m_BufferList;
	int				m_nBufferTotalSize;
};

#endif // end of TUX_ATMIBUFFER_MANAGER_H__
