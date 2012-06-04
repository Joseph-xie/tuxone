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
#ifndef TUX_GATE_CHECK_THREAD_H__
#define TUX_GATE_CHECK_THREAD_H__
#include <lwpr.h>

class GateCheckThread : public LWPR::Thread,
	public LWPR::Resource
{
public:
	GateCheckThread();
	~GateCheckThread();
	static GateCheckThread* GetInstance();

	/**
	 * ��ʼ��
	 */
	bool Init();

	/**
	 * ����NetServer��ͬʱ���AppΪ�����
	 */
	void SetNetServer(LWPR::NetServer* pNetServer);

	/**
	 * ���AppΪ�ͻ������
	 */
	void MarkClient();

	/**
	 * �����̣߳��߳�������Select���������У�
	 */
	bool Wakeup();

	/**
	 * �Ƿ���Gate�´��֪ͨ
	 */
	bool HasGateNotice();

	/**
	 * ��Gate����Service
	 */
	bool DownloadService();

	void Term();

	void Go();

private:
	/**
	 * ��ȡApp����
	 */
	LWPR::UINT32 GetAppType();

	/**
	 * ��ȡServer�ļ�����ַ
	 */
	void GetServerListeningAddr(LWPR::SOCKET_ADDR_T& addr);

	/**
	 * ����Gate
	 */
	bool ConnectGate();

	/**
	 * ͬGate�Ͽ�
	 */
	void DisconnectGate();

	/**
	 * ��Gate��������
	 */
	bool SendHeartBeat();

	/**
	 * �ȴ�֪ͨ�������߳�֪ͨ��Gate�·�֪ͨ����ʱ�Զ����ѣ�
	 */
	void WaitForNotify();

	/**
	 * �̺߳���
	 */
	virtual void Run();

	/**
	 * ע��
	 */
	bool RegisterToGate();

	/**
	 * ����
	 */
	bool SubscibeFromGate();

private:
	LWPR::SOCKET_FD_T	m_nFDPIPERead;
	LWPR::SOCKET_FD_T	m_nFDPIPEWrite;

	typedef std::vector<std::string> GateAddrs;
	GateAddrs			m_GateAddrs;
	LWPR::UINT32		m_nGateIndex;
	//
	// �� Gate ͨ�����
	//
	LWPR::Mutex			m_MutexGate;
	LWPR::SOCKET_FD_T	m_nFDGate;
	LWPR::UINT32		m_nBBUpdateSeqRegister;
	LWPR::UINT32		m_nBBUpdateSeqSubscibe;

	// ֻ�е�ǰ�̷߳��ʣ�����Ҫ����
	LWPR::UINT32		m_nHeartInterval;

	volatile bool		m_bGateNotified;

	volatile bool		m_bTermed;

	LWPR::Mutex			m_MutexOther;
	LWPR::UINT32		m_nAppType;
	LWPR::NetServer*	m_pNetServer;

	// �����߳�����ͬ��ǰ�߳̾�����Դ
	LWPR::Mutex			m_MutexResource;
};

#endif // end of TUX_GATE_CHECK_THREAD_H__
