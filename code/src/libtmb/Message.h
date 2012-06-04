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
#ifndef TUX_MESSAGE_H__
#define TUX_MESSAGE_H__
#include <lwpr.h>
#include "View32Util.h"

/**
 * ���ķ�����
 */
const LWPR::INT32 TUX_MSG_RET_CODE_OK = 0;
const LWPR::INT32 TUX_MSG_RET_CODE_SYSERROR = 1;
const LWPR::INT32 TUX_MSG_RET_CODE_BBFULL = 2;
const LWPR::INT32 TUX_MSG_RET_CODE_SVC_NOT_FOUND = 3;		// TPENOENT
const LWPR::INT32 TUX_MSG_RET_CODE_SVC_ERR = 4;				// TPESVCERR
const LWPR::INT32 TUX_MSG_RET_CODE_SVC_FAIL = 5;			// TPESVCFAIL

/**
 * �������ͣ�Gate��App�����´�֪ͨ
 */
const LWPR::INT32 TUX_MSG_TYPE_GATE_NOTICE = 0;

/**
 * �������ͣ���ȡ���������
 */
const LWPR::INT32 TUX_MSG_TYPE_ADMIN_GET_BB_REQ = 1;
const LWPR::INT32 TUX_MSG_TYPE_ADMIN_GET_BB_REP = 2;

/**
 * �������ͣ�APP��Gateͨ�ţ�ע��APP������SERVICE
 */
const LWPR::INT32 TUX_MSG_TYPE_APP_REGISTER_REQ = 3;
const LWPR::INT32 TUX_MSG_TYPE_APP_REGISTER_REP = 4;

/**
 * �������ͣ�Client��Gateͨ�ţ�����SERVICE
 */
const LWPR::INT32 TUX_MSG_TYPE_APP_SUBSCIBE_REQ = 5;
const LWPR::INT32 TUX_MSG_TYPE_APP_SUBSCIBE_REP = 6;

/**
 * �������ͣ�Client��Serverͨ�ţ�����к�
 */
const LWPR::INT32 TUX_MSG_TYPE_PING_PANG_REQ = 7;
const LWPR::INT32 TUX_MSG_TYPE_PING_PANG_REP = 8;

/**
 * �������ͣ�Client��Serverͨ�ţ�tpcall
 */
const LWPR::INT32 TUX_MSG_TYPE_TPCALL_REQ = 9;
const LWPR::INT32 TUX_MSG_TYPE_TPCALL_REP = 10;

/**
 * �������ͣ�Client��Serverͨ�ţ���������server���ã�����server�����Ϣ
 */
const LWPR::INT32 TUX_MSG_TYPE_SERVER_INFO_REQ = 11;
const LWPR::INT32 TUX_MSG_TYPE_SERVER_INFO_REP = 12;

/**
 * ��Ϣͷ�ı�־λ
 */
// nMsgFlags ��1λ
const LWPR::INT32 TUX_MSG_BIG_ENDIAN = 1;

// nMsgFlags ��2λ
const LWPR::INT32 TUX_MSG_DISCONNECT_SERVER = (1 << 1);

/**
 * ��Ϣͷ
 */
typedef struct tux_header_message_t
{
	LWPR::INT32		nMsgLength;		// ���ĳ��ȣ�������ͷ
	LWPR::INT32		nMsgType;		// ��������
	LWPR::INT32		nMsgFlags;		// ���ı�־
	LWPR::INT32		nMsgRetCode;	// ����Ӧ������ֻ��Ӧ�����������ã�

	tux_header_message_t();
	void Marshal(LWPR::OutputStream& out);
	void Unmarshal(LWPR::InputStream& in);

	/**
	 * �Ƿ��Ǵ��
	 */
	bool IsBigEndian();

	/**
	 * �ͻ����Ƿ���Ҫ������������Ͽ�
	 */
	bool DisconnectFromServer();
	void DisconnectFromServer(bool discon);
} TUX_HEADER_MESSAGE_T;

#define TUX_HEADER_MESSAGE_SIZE		(sizeof(LWPR::INT32) * 4)

/**
 * ��Ϣ����
 */
class Message
{
public:
	Message();
	virtual ~Message();

	virtual void Marshal(LWPR::OutputStream& out);
	virtual void Unmarshal(LWPR::InputStream& in);
	TUX_HEADER_MESSAGE_T& MsgHeader();
	void MsgHeader(const TUX_HEADER_MESSAGE_T& header);

private:
	TUX_HEADER_MESSAGE_T m_tHeader;
};

/**
 * �������
 */
typedef enum tux_invoke_result_e
{
	TUX_INVOKE_OK						= 0,
	TUX_INVOKE_CONNECT_FAILED			= 1,
	TUX_INVOKE_SEND_HEADER_FAILED		= 2,
	TUX_INVOKE_SEND_HEADER_TIMEOUT		= 3,
	TUX_INVOKE_SEND_BODY_FAILED			= 4,
	TUX_INVOKE_SEND_BODY_TIMEOUT		= 5,
	TUX_INVOKE_RECEIVE_FAILED			= 6,
	TUX_INVOKE_RECEIVE_TIMEOUT			= 7,
	TUX_INVOKE_RECEIVE_PROTOCOL_ERROR	= 8,
	TUX_INVOKE_RECEIVE_OS_ERROR			= 9
} TUX_INVOKE_RESULT_E;

TUX_INVOKE_RESULT_E Invoke(LWPR::SOCKET_FD_T fd, Message& msg_in, Message& msg_out, LWPR::INT32 timeout);

/**
 * �������ࣺ��ȡ�����������
 */
typedef struct tux_fetch_bb_req_t : public Message
{
	LWPR::BOOL			bFetchAll;
	tux_fetch_bb_req_t();
	virtual void Marshal(LWPR::OutputStream& out);
	virtual void Unmarshal(LWPR::InputStream& in);
} TUX_FETCH_BB_REQ_T;

typedef struct tux_fetch_bb_rep_t : public Message
{
	LWPR::Buffer_var		vBufBody;

	tux_fetch_bb_rep_t();
	virtual void Marshal(LWPR::OutputStream& out);
	virtual void Unmarshal(LWPR::InputStream& in);
} TUX_FETCH_BB_REP_T;

/**
 * APP��Gateͨ�ţ�ע��APP������SERVICE
 */
const LWPR::UINT32 TUX_APP_TYPE_UNKNOW = 0;
const LWPR::UINT32 TUX_APP_TYPE_CLIENT = 1;
const LWPR::UINT32 TUX_APP_TYPE_SERVER = 1 << 1;

typedef struct tux_app_register_req_t : public Message
{
	LWPR::UINT32				nAppType;		// App����
	LWPR::SOCKET_ADDR_T			tServerAddr;	// Server ������ַ
	std::vector<std::string>	vtServices;		// Server ��Ҫע���Service

	tux_app_register_req_t();
	virtual void Marshal(LWPR::OutputStream& out);
	virtual void Unmarshal(LWPR::InputStream& in);
} TUX_APP_REGISTER_REQ_T;

typedef struct tux_app_register_rep_t : public Message
{
	LWPR::UINT32				nBBUpdateSeq;		// ����������
	LWPR::UINT32				nAppHeartInterval;	// Ӧ�������������
	std::vector<std::string>	vtServices;			// Server ע��ɹ���Service

	tux_app_register_rep_t();

	virtual void Marshal(LWPR::OutputStream& out);
	virtual void Unmarshal(LWPR::InputStream& in);
} TUX_APP_REGISTER_REP_T;

/**
 * APP��Gateͨ�ţ�Client��Gate���ķ���
 */
typedef struct tux_app_subscibe_req_t : public Message
{
	std::vector<std::string>	vtServices;		// Client ����Ȥ��Service

	tux_app_subscibe_req_t();
	virtual void Marshal(LWPR::OutputStream& out);
	virtual void Unmarshal(LWPR::InputStream& in);
} TUX_APP_SUBSCIBE_REQ_T;

typedef struct service_info_t
{
	std::string			strSvcName;		// Service ����
	LWPR::SOCKET_ADDR_T	tServerAddr;	// Server ������ַ

	service_info_t();
} SERVICE_INFO_T;

typedef struct tux_app_subscibe_rep_t : public Message
{
	LWPR::UINT32				nBBUpdateSeq;	// ���������ţ�Gate��ʵ�ʱ��ֵ��
	std::vector<SERVICE_INFO_T>	vtServiceInfo;	// ��Gate��ȡ������Service��Ϣ

	tux_app_subscibe_rep_t();

	virtual void Marshal(LWPR::OutputStream& out);
	virtual void Unmarshal(LWPR::InputStream& in);
} TUX_APP_SUBSCIBE_REP_T;

/**
 * Client��Serverͨ�ţ�����к�
 */
typedef struct tux_app_pingpang_req_t : public Message
{
	tux_app_pingpang_req_t();
} TUX_APP_PINGPANG_REQ_T;

typedef struct tux_app_pingpang_rep_t : public Message
{
	tux_app_pingpang_rep_t();
} TUX_APP_PINGPANG_REP_T;

/**
 * Client��Serverͨ�ţ�tpcall
 */
typedef enum typed_buffer_e
{
	TYPED_BUFFER_CARRAY,
	TYPED_BUFFER_STRING,
	TYPED_BUFFER_MBSTRING,
	TYPED_BUFFER_VIEW32,
	TYPED_BUFFER_FML32,
	TYPED_BUFFER_XML,
} TYPED_BUFFER_E;

typedef struct tux_app_tpcall_req_t : public Message
{
	std::string			strSvcName;
	LWPR::BOOL			bHasReq;
	char*				pReqBuf;
	LWPR::UINT32		nReqLen;
	TYPED_BUFFER_E		nReqBufType;
	std::string			strReqBufTypeSub;
	STRUCT_INFO_T		tStructInfo;

	LWPR::UINT32		nFlags;

	LWPR::Buffer_var	vReqBuf;			// ͬpReqBufһ�£�ΪUnmarshalʹ��

	tux_app_tpcall_req_t();
	virtual void Marshal(LWPR::OutputStream& out);
	virtual void Unmarshal(LWPR::InputStream& in);
} TUX_APP_TPCALL_REQ_T;

typedef struct tux_app_tpcall_rep_t : public Message
{
	LWPR::BOOL			bHasRep;
	char*				pRepBuf;
	LWPR::UINT32		nRepLen;

	TYPED_BUFFER_E		nRepBufType;
	std::string			strRepBufTypeSub;
	STRUCT_INFO_T		tStructInfo;

	LWPR::UINT32		nFlags;
	LWPR::INT32			nRetCode;

	LWPR::Buffer_var	vRepBuf;			// ͬpRepBufһ�£�ΪUnmarshalʹ��

	tux_app_tpcall_rep_t();
	virtual void Marshal(LWPR::OutputStream& out);
	virtual void Unmarshal(LWPR::InputStream& in);
} TUX_APP_TPCALL_REP_T;

/**
 * Client��Serverͨ�ţ���������server���ã�����server�����Ϣ
 */
typedef struct tux_app_server_info_req_t : public Message
{
	tux_app_server_info_req_t();
	virtual void Marshal(LWPR::OutputStream& out);
	virtual void Unmarshal(LWPR::InputStream& in);
} TUX_APP_SERVER_INFO_REQ_T;

typedef struct tux_app_server_info_rep_t : public Message
{
	std::string strServerName; // server����
	LWPR::INT32 nClientTotal;  // ��ǰ���߿ͻ�������

	tux_app_server_info_rep_t();

	virtual void Marshal(LWPR::OutputStream& out);
	virtual void Unmarshal(LWPR::InputStream& in);
} TUX_APP_SERVER_INFO_REP_T;
#endif // end of TUX_MESSAGE_H__
