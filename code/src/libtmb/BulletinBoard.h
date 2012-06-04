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
#ifndef TUX_BULLETIN_BOARD_H__
#define TUX_BULLETIN_BOARD_H__
#include <atmi.h>
#include <lwpr.h>
#include <string>
#include <vector>
#include <list>
#include <map>

typedef enum bbslot_state_e
{
	BBSLOT_IDLE = 0,		// ����״̬
	BBSLOT_INUSE = 1,		// ʹ��״̬
	BBSLOT_ISOLATED = 2,	// ����״̬
} BBSLOT_STATE_E;

const LWPR::UINT32 BB_HEADER_MAGIC_CODE = 10504816 ^ 10225799;

/**
 * Header table entity
 */
typedef struct bb_header_entity_t
{
	LWPR::UINT32		nMagicCode;			// ����Ψһ��ʶ����TUXONEʹ�õĹ����ڴ�
	LWPR::UINT32		nSEMID;				// �ź���IPCID
	LWPR::UINT32		nSVCUpdateSeq;		// ����������
	LWPR::UINT32		nGatePID;			// txgate���̺�
	LWPR::UINT16		nUDPPort;			// UDP �˿ںţ������ֽ���
	LWPR::UINT32		nWSHMaxLength;		// WSH LEN
	LWPR::UINT32		nSVCMaxLength;		// SVC LEN
	LWPR::SOCKET_ADDR_T	tServerAddr;		// txgate�ļ�����ַ�������ֽ���

	/**
	 * ���캯��
	 */
	bb_header_entity_t();

	/**
	 * ����new[]������
	 */
	void* operator new(size_t size, void* addr);
	void* operator new(size_t size);

} BB_HEADER_ENTITY_T;

typedef std::map < LWPR::IPCID_T, BB_HEADER_ENTITY_T > BBHeaderMap;

/**
 * WSH table entity
 */
typedef struct bb_wsh_entity_t
{
	LWPR::INT32		nIndex;				// �±�
	LWPR::INT32		nPID;				// ���̺�
	LWPR::INT32		nConnectionTotal;	// SOCKET��������
	LWPR::UINT16	nPort;				// UDP �˿ںţ������ֽ���
	BBSLOT_STATE_E	nSlotState;			// ��λ��״̬���������ͣ�BBSLOT_STATE_E

	/**
	 * ���캯��
	 */
	bb_wsh_entity_t();

	/**
	 * �������캯��
	 */
	bb_wsh_entity_t(const bb_wsh_entity_t& rhs);

	/**
	 * ���ظ�ֵ�����
	 */
	bb_wsh_entity_t& operator=(const bb_wsh_entity_t& rhs);

	/**
	 * ����new[]������
	 */
	void* operator new[](size_t size, void* addr);
	void* operator new[](size_t size);
} BB_WSH_ENTITY_T;

typedef std::vector < BB_WSH_ENTITY_T > BBWSHList;

typedef std::vector < int > BBIndexList;

#define BBSVC_ENTITY_BELONG_TO_SELF		(-1)
/**
 * Service table entity
 */
typedef struct bb_service_entity_t
{
	LWPR::INT32				nIndex;				// �±�
	char					strSvcName[TUXONE_SERVICE_NAME_LENGTH + 1];	// Service Name
	LWPR::SOCKET_ADDR_T		tServerAddr;	// Server�ĵ�ַ
	LWPR::INT32				nBelongto;		// ��ǰservice�����ĸ�gate
	BBSLOT_STATE_E			nSlotState;		// ��λ��״̬���������ͣ�BBSLOT_STATE_E

	/**
	 * ���캯��
	 */
	bb_service_entity_t();

	/**
	 * �������캯��
	 */
	bb_service_entity_t(const bb_service_entity_t& rhs);

	/**
	 * ���ظ�ֵ�����
	 */
	bb_service_entity_t& operator=(const bb_service_entity_t& rhs);

	/**
	 * ����new[]������
	 */
	void* operator new[](size_t size, void* addr);
	void* operator new[](size_t size);
} BB_SERVICE_ENTITY_T;

typedef std::vector < BB_SERVICE_ENTITY_T > BBSVCList;

/**
 * ������������������紫�䣩
 */
typedef struct bb_stream_t
{
	BB_HEADER_ENTITY_T*			pHeader;			// �����ͷ���������ڹ����ڴ棩
	BB_WSH_ENTITY_T*			tblWSH;				// WSH�������ڹ����ڴ棩
	LWPR::INT32					tblWSHLength;		// WSH����

	BB_SERVICE_ENTITY_T*		tblSVC;				// SERVICE�������ڹ����ڴ棩
	LWPR::INT32					tblSVCLength;		// SERVICE����

	bb_stream_t();
	~bb_stream_t();

	void Marshal(LWPR::OutputStream& out);
	void Unmarshal(LWPR::InputStream& in);
} BB_STREAM_T;

/**
 * ������࣬������̻�����ʹ�á�
 */
class BulletinBoard
{
public:
	/**
	 * ���캯��
	 */
	BulletinBoard();

	/**
	 * ��������
	 */
	~BulletinBoard();

	/**
	 * ��������
	 */
	static BulletinBoard* GetInstance();

	/**
	 * �ڹ��ع����ڴ�֮ǰ��ʼ��
	 */
	void Init(int wsh_len, int svc_len);

	/**
	 * ��ӡ��������ݣ������ã�
	 */
	void Print();

	/**
	 * ���ù����ͷ
	 */
	void SetHeader(const BB_HEADER_ENTITY_T& header);

	/**
	 * ��ȡ�����ͷ
	 */
	void GetHeader(BB_HEADER_ENTITY_T& header);

	/**
	 * ��ȡ������к�
	 */
	LWPR::UINT32 GetSVCUpdateSeq();

	/**
	 * ��ʼ���������
	 */
	void Create(int wsh_len, int svc_len);

	/**
	 * ���ӵ������
	 */
	void Connect(LWPR::IPCID_T shmid);

	/**
	 * �Ͽ��빫��������
	 */
	void Disconnect();

	/**
	 * ���ٹ����
	 */
	void Destroy();

	/**
	 * �򹫸��ע��WSH
	 * �������false��WSH��ռ䲻��
	 * ��wsh�ṹ���з����±�
	 */
	bool RegisterWSH(BB_WSH_ENTITY_T& wsh);

	/**
	 * ����WSH�ڹ�����еļ�¼���������̵��ã�
	 */
	void CleanupWSHByIndex(const BBIndexList& wsh_indexs);

	/**
	 * ����WSH�ڹ�����еļ�¼���������̵��ã�
	 */
	void CleanupWSHByPID(const LWPR::PIDList& pids);

	/**
	 * WSH�����ӹ����ע������WSH���̵��ã�
	 */
	void UnregisterWSH();

	/**
	 * ����WSH��Socket����������WSH���̵��ã�
	 * �˺���ʹ��ԭ�Ӳ������������
	 */
	void UpdateWSHConnectionTotal(bool increase);

	/**
	 * �ӹ�����ȡWSH
	 */
	int GetAllWSH(BBWSHList& wshs);

	/**
	 * �ӹ�����ȡ���Լ�������WSH����
	 */
	int GetOtherWSH(BBWSHList& wshs);

	/**
	 * �򹫸��ע��SERVICE
	 * �������false��SERVICE��ռ䲻��
	 * ��svc�ṹ���з����±�
	 */
	bool RegisterService(BB_SERVICE_ENTITY_T& svc);

	/**
	 * �ӹ����ע��SERVICE
	 * ע�⣺ֻ��Ҫ��дBB_SERVICE_ENTITY_T�ṹ���е�strSvcName��tServerAddr����
	 */
	void UnregisterService(const BBSVCList& svcs);

	/**
	 * �ӹ����ע��һ��SERVER����������SERVICE
	 */
	void UnregisterService(const LWPR::SOCKET_ADDR_T& addr);

	/**
	 * ���ݷ������ӹ�����ȡSVC
	 * ���ع���������
	 */
	LWPR::UINT32 GetAllSVC(const std::vector<std::string>& names, BBSVCList& svcs);

	/**
	 * �жϹ�����Ƿ��б��(���ϴε������Ƚ�)
	 */
	bool IsSvcUpdated();

	/**
	 * ��ȡ�������
	 * fetchall��ʾ�Ƿ��ȡ����Service�����Ϊfalse����ֻ��ȡ���ڵ�ǰgate��service
	 */
	void GetBBStream(BB_STREAM_T& stream, bool fetchall = true);

	/**
	 * ͬ������壨������������ȡ���ݣ�
	 * ����ֵ��ʾ�Ƿ����𹫸�巢���仯
	 */
	bool SynBB(const BB_STREAM_T& stream, LWPR::INT32 index);

	/**
	 * ��������wsh����
	 */
	void WakeupOtherWSH(LWPR::SOCKET_FD_T udpfd);

	/**
	 * ����ϵͳ����ȡ�����ͷ�б�
	 */
	static void FetchGateIPCID(BBHeaderMap& headers);

private:
	/**
	 * ���㹲���ڴ��С
	 */
	int ComputeShmSize();

private:
	LWPR::IPCID_T				m_nSHMID;
	void*						m_pSHMAddr;			// �����̹��صĹ����ڴ��׵�ַ
	LWPR::IPCID_T				m_nSEMID;			// ���ƹ����ڴ��д����

	BB_HEADER_ENTITY_T*			m_pHeader;			// �����ͷ���������ڹ����ڴ棩

	BB_WSH_ENTITY_T*			m_tblWSH;			// WSH�������ڹ����ڴ棩
	int							m_tblWSHLength;		// WSH����

	BB_SERVICE_ENTITY_T*		m_tblSVC;			// SERVICE�������ڹ����ڴ棩
	int							m_tblSVCLength;		// SERVICE����

	int							m_nWSHIndex;		// WSH���̶�Ӧ���±꣨������WSH�����ڣ�
	LWPR::UINT32				m_nSVCUpdateSeq;	// ���������ţ�������WSH�����ڣ�
};

#endif // end of TUX_BULLETIN_BOARD_H__
