#pragma once
#include "pch.h"
#include "framework.h"

#pragma pack(push)
#pragma pack(1)	//设置结构体、联合体和类成员的对齐方式为1字节对齐。

class CPacket {
public:
	CPacket():sHead(0),nLength(0),sCmd(0),sSum(0){}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}
		
		sSum = 0;
		for (size_t j = 0;j < strData.size();j++) {
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket(const BYTE* pData, size_t& nSize) {
		size_t i = 0;
		for (;i < nSize;i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);i += 4;
		if (nLength + i > nSize) {	//包未完全接受到;
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 4);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i);i += 2;
		WORD sum = 0;
		for (size_t j = 0;j < strData.size();j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;//实际使用的size；
			return;
		}
		nSize = 0;
	}
	~CPacket() {}
	CPacket& operator=(const CPacket& pack) {	//const
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}
	int Size() {
		return nLength + 6;
	}
	const char* Data() {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;pData += 2;
		*(DWORD*)pData = nLength;pData += 4;
		*(WORD*)pData = sCmd;pData += 2;
		memcpy(pData, strData.c_str(), strData.size());pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

	WORD sHead;//固定位FE FF
	DWORD nLength;//包长度(包括下面3个!但是不包括自身);
	WORD sCmd;//控制命令
	std::string strData;//包数据，一个对象,传&仅仅会传一个地址，不是数据；
	WORD sSum;	//和校验,只对strData求和
	std::string strOut;
};

#pragma pack(pop)
typedef struct MouseEvent{
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//点击、移动、双击；
	WORD nButton;//左键、右键、中键；
	POINT ptXY;//坐标；
}MOUSEEV,*PMOUSEEV;

typedef struct file_info {
	file_info() {
		IsInvalid = FALSE;  //是否无效的文件
		IsDirectory = -1;
		hasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;
	BOOL IsDirectory;
	BOOL hasNext;
	char szFileName[256];//文件名
}FILEINFO, * PFILEINFO;

class CServerSocket
{
public:
	static CServerSocket* getInstance() {
		if (m_instance == NULL) {
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	bool InitSocket() {
		if (m_sock == -1)return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);
		//绑定
		if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)return false;
		//TODO:
		if (listen(m_sock, 1) == -1)return false;

		return true;
	}

	bool AcceptClient() {
		//char buffer[1024];
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		TRACE("m_client= %d\r\n", m_client);
		if (m_client == -1)return false;
		return true;
		/*recv(serv_sock, buffer, sizeof(buffer), 0);
		send(serv_sock, buffer, sizeof(buffer), 0);*/
	}
#define BUFFER_SIZE 4096
	int DealCommand() {
		if (m_client == -1)return -1;
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == NULL) {
			TRACE(_T("内存不足！！\r\n"));
		}
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true) {
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0) {
				delete[]buffer;
				return -1;
			}
			TRACE(_T("server recv %d\r\n"), len);
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				delete[]buffer;
				return m_packet.sCmd;
			}
		}
		return -1;
	}
	bool Send(const char* pData, int nSize) {
		if (m_client == -1)return false;
		return send(m_client, pData, nSize, 0)>0;
	}
	bool Send(CPacket& pack) {
		if (m_client == -1)return false;
		return send(m_client, pack.Data(), pack.Size(), 0)>0;
	}
	bool GetFilePath(std::string& strPath) {
		if ((m_packet.sCmd >= 2)&& (m_packet.sCmd <= 4)) {
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}
	bool GetMouseEvent(MOUSEEV& mouse) {
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), m_packet.strData.size());
			return true;
		}
		return false;
	}
	CPacket& GetPacket() {
		return m_packet;
	}

	void CloseClient() {
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}

private:
	SOCKET m_sock;
	SOCKET m_client;
	CPacket m_packet;
	CServerSocket& operator=(const CServerSocket& ss) {

	}
	CServerSocket(const CServerSocket&ss) {
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}
	CServerSocket() {
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
		m_client = INVALID_SOCKET;
	}
	~CServerSocket(){
		closesocket(m_sock);
		WSACleanup();
	}
	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}

	static void releaseInstance() {
		if (m_instance != NULL) {
			CServerSocket* tem = m_instance;
			m_instance = NULL;
			delete tem;
		}
	}


	static CServerSocket* m_instance;
	class CHelper {
	public:
		CHelper() {
			getInstance();
		}
		~CHelper() {
			releaseInstance();
		}
	};
	static CHelper m_helper;  
};

extern CServerSocket server;

