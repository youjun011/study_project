#pragma once
#include "pch.h"
#include "framework.h"
#include<list>
#include "Packet.h"


typedef void(*SOCKET_CALLBACK)(void* arg, int status, 
	std::list<CPacket>&listPacket, CPacket& inPacket);

class CServerSocket
{
public:
	static CServerSocket* getInstance() {//单例模式，构造在main函数之前，析构在main函数之后；
		if (m_instance == NULL) {
			m_instance = new CServerSocket();//多线程编程问题
		}
		return m_instance;
	}
	int Run(SOCKET_CALLBACK callback, void* arg, short port = 9527) {
		m_callback = callback;
		m_arg = arg;
		bool ret = InitSocket(port);
		if (ret = false)return -1;
		std::list<CPacket>listPacket;
		int count = 0;
		while (true) {
			if (AcceptClient() == false) {
				if (count >= 3) {
					return -2;
				}
				count++;
			}
			int ret = DealCommand();
			if (ret > 0) {
				m_callback(arg, ret, listPacket,m_packet);
				while (listPacket.size() > 0) {
					Send(listPacket.front());
					listPacket.pop_front();
				}
			}
			CloseClient();
		}
		return 0;
	}
protected:
	bool InitSocket(short port) {
		if (m_sock == -1)return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(port);
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
	void CloseClient() {
		if (m_client != INVALID_SOCKET) {
			closesocket(m_client);
			m_client = INVALID_SOCKET;
		}
	}

private:
	SOCKET_CALLBACK m_callback;
	void* m_arg;
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
		if (m_instance != NULL) {//防御型编程;
			CServerSocket* tem = m_instance;	//这里为了防止delete析构时会有人调用m_instance;
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

