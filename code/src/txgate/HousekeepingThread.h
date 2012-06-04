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
#ifndef TUX_HOUSEKEEPING_THREAD_H__
#define TUX_HOUSEKEEPING_THREAD_H__
#include <lwpr.h>
#include <time.h>

/**
 * 1�����Server�Ƿ���ڣ����ٹ��ڵ�Server
 * 2����û�й��ڵ�Server����Service���֪ͨ
 * 3��������WSH���̷���Service���֪ͨ
 */
class HousekeepingThread : public LWPR::Thread, public LWPR::Resource
{
public:

	HousekeepingThread();
	~HousekeepingThread();
	static HousekeepingThread* GetInstance();

	void SetNetServer(LWPR::NetServer* pNetServer);
	void SetUDPFd(LWPR::SOCKET_FD_T fd);

	/**
	 * �����߳�
	 * self ��ʾ�Ƿ��ǵ�ǰ���̻���
	 */
	void Wakeup(bool self);

	/**
	 * �Ƿ��Ǳ�������
	 */
	bool IsWakeupBySelf();

	virtual void Run();

private:
	/**
	 * �����пͻ��˷��͹㲥
	 */
	void BroadcastNotice();

	/**
	 * ����ǵ�ǰWSH���̵��¹����������֪ͨ����WSH����
	 */
	void NotifyOtherWSH();

private:
	LWPR::NetServer*	m_pNetServer;
	time_t				m_nLastCheckTime;
	LWPR::SOCKET_FD_T	m_nUDPFd;

	LWPR::Mutex			m_Mutex;
	bool				m_bWakeupSelf;
};

#endif // end of TUX_HOUSEKEEPING_THREAD_H__
