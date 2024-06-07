#pragma once
#include "pch.h"
#include "framework.h"
#include<string>
#include <vector>
#include<list>
#include<map>
#include<mutex>
#define WM_SEND_PACK (WM_USER+1)//发送包数据
#define WM_SEND_PACK_ACK (WM_USER+2)
#pragma pack(push)
#pragma pack(1)	//设置结构体、联合体和类成员的对齐方式为1字节对齐。

class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
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
		for (size_t j = 0; j < strData.size(); j++) {
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
		for (; i < nSize; i++) {
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
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {	//包未完全接受到;
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 4);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
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
	const char* Data(std::string &strOut)const {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}

	WORD sHead;//固定位FE FF
	DWORD nLength;//包长度(包括下面3个!但是不包括自身);
	WORD sCmd;//控制命令
	std::string strData;//包数据，一个对象,传&仅仅会传一个地址，不是数据；
	WORD sSum;	//和校验,只对strData求和
	//std::string strOut;
};

#pragma pack(pop)
typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//点击、移动、双击；
	WORD nButton;//左键、右键、中键；
	POINT ptXY;//坐标；
}MOUSEEV, * PMOUSEEV;

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

enum {
	CSM_AUTOCLOSE=1,//自动关闭模式
};

typedef struct PacketData{
	std::string strData;
	UINT nMode;
	WPARAM wParam;
	PacketData(const char* pData, size_t nLen, UINT mode, WPARAM nParam=0) {
		strData.resize(nLen);
		memcpy((char*)strData.c_str(), pData, nLen);
		nMode = mode;
		wParam = nParam;
	}
	PacketData(const PacketData& data) {
		strData = data.strData;
		nMode = data.nMode;
		wParam = data.wParam;
	}
	PacketData& operator=(const PacketData& data) {
		if (this != &data) {
			strData = data.strData;
			nMode = data.nMode;
			wParam = data.wParam;
		}
		return *this;
	}
}PACKET_DATA;

class CClientSocket
{
public:
	static CClientSocket* getInstance() {
		if (m_instance == NULL) {
			m_instance = new CClientSocket();
		}
		return m_instance;
	}
	bool InitSocket();

#define BUFFER_SIZE 4096000
	int DealCommand() {
		if (m_sock == -1)return -1;
		char* buffer = m_buffer.data();//TODO:多线程发送命令时可能会出现冲突
		//memset(buffer, 0, BUFFER_SIZE);不能清0
		static size_t index = 0;	//index的值要保留；
		while (true) {
			size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);
			if (((int)len <= 0) && ((int)index == 0))return -1;
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, index - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}
	
	bool SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed = true, WPARAM wParam=0);
	bool GetFilePath(std::string& strPath) {
		if ((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4)) {
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
	void CloseSocket() {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
	void UpdateAddress(int nIP, int nPort) {
		if ((m_nIP != nIP) || (m_nPort != nPort)) {
			m_nIP = nIP;
			m_nPort = nPort;
		}
	}
private:
	HANDLE m_eventInvoke;//
	UINT m_nThreadID;
	typedef void(CClientSocket::* MSGFUNC)(UINT
		nMsg, WPARAM wParam, LPARAM lParam);
	std::map<UINT, MSGFUNC>m_mapFunc;
	HANDLE m_hThread;
	bool m_bAutoClose;
	std::mutex m_lock;
	std::list<CPacket>m_listSend;
	std::map<HANDLE, std::list<CPacket>&>m_mapAck;
	std::map<HANDLE, bool>m_mapAutoClosed;
	int m_nIP;
	int m_nPort;
	std::vector<char>m_buffer;
	SOCKET m_sock;
	CPacket m_packet;
	CClientSocket& operator=(const CClientSocket& ss) {

	}
	CClientSocket(const CClientSocket& ss);
	
	CClientSocket();

	~CClientSocket() {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		WSACleanup();
	}
	static unsigned __stdcall threadEntry(void* arg);
	//void threadFunc();
	void threadFunc2();
	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}

	static void releaseInstance() {
		if (m_instance != NULL) {
			CClientSocket* tem = m_instance;
			m_instance = NULL;
			delete tem;
		}
	}
	bool Send(const char* pData, int nSize) {
		if (m_sock == -1)return false;
		return send(m_sock, pData, nSize, 0) > 0;
	}
	bool Send(const CPacket& pack);
	void SendPack(UINT nMsg, WPARAM wParam/*缓冲区的值*/,
		LPARAM lParam/*缓冲区的长度*/);
	static CClientSocket* m_instance;
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

extern CClientSocket server;
