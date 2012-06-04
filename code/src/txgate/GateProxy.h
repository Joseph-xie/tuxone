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
#ifndef TX_GATE_PROXY_H__
#define TX_GATE_PROXY_H__
#include <lwpr.h>
#include <string>

class GateManager;

class GateProxy
{
	friend class GateManager;

public:

	GateProxy(const char* addr, LWPR::INT32 index);
	~GateProxy();

	bool SendHeartBeat();

	bool SyncService();

	void ClearBB();

private:
	bool Connect();

	void Disconnect();

private:
	std::string			m_strGateAddr;			// gate��ַ
	LWPR::INT32			m_nGateIndex;			// Gate���
	LWPR::SOCKET_FD_T	m_nFDGate;				// gate socket
	LWPR::UINT32		m_nBBUpdateSeqSync;		// �ϴ�ͬ��ʱ����¼��gate�����ͬ�����
	LWPR::UINT32		m_nBBUpdateSeqHeart;	// �ϴ�����ʱ����¼��gate������������
};

#endif // end of TX_GATE_PROXY_H__
