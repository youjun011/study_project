#pragma once
#include<MSWSock.h>
#include<Windows.h>
#include "EdoyunThread.h"
#include "CEdoyunQueue.h"
#include<map>

enum EdoyunOperator
{
    ENone,
    EAccept,
    ERecv,
    ESend,
    EError
};
class EdoyunClient;
class EdoyunServer;
typedef std::shared_ptr<EdoyunClient> PCLIENT;

class EdoyunOverlapped {
public:
    OVERLAPPED m_overlapped;
    DWORD m_operator;
    std::vector<char> m_buffer;
    ThreadWorker m_worker;
    EdoyunServer* m_server;
    PCLIENT m_client;
    WSABUF m_wsabuffer;
};

template<EdoyunOperator>class AcceptOverlapped;
typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;
template<EdoyunOperator>class RecvOverlapped;
typedef RecvOverlapped<ERecv> RECVOVERLAPPED;
template<EdoyunOperator>class SendOverlapped;
typedef AcceptOverlapped<ESend> SENDOVERLAPPED;
template<EdoyunOperator>class ErrorOverlapped;
typedef AcceptOverlapped<EError> ERROROVERLAPPED;

class EdoyunClient {
public:
    EdoyunClient();
    ~EdoyunClient() {
        closesocket(m_sock);
    }
    operator SOCKET() {
        return m_sock;
    }
    void SetOverlapped(PCLIENT& ptr);

    operator PVOID() {
        return &m_buffer[0];
    }
    operator LPOVERLAPPED();
    operator LPDWORD() {
        return &m_received;
    }
    LPWSABUF RecvWSABuffer();
    LPWSABUF SendWSABuffer();
    DWORD& flags() {
        return m_flags;
    }
    sockaddr_in* GetLocalAddr() { return &m_laddr; }
    sockaddr_in* GetRemoteAddr() { return &m_raddr; }
    size_t GetBufferSize()const { return m_buffer.size(); }
    int Recv() {
       int ret = recv(m_sock, m_buffer.data()+m_used, m_buffer.size()-m_used, 0);
       if (ret <= 0)return -1;
       m_used += (size_t)ret;

       return 0;
    }
private:
    SOCKET m_sock;
    DWORD m_received;
    DWORD m_flags;
    std::shared_ptr<ACCEPTOVERLAPPED> m_overlapped;
    std::shared_ptr<RECVOVERLAPPED> m_recv;
    std::shared_ptr<SENDOVERLAPPED> m_send;
    std::vector<char>m_buffer;
    size_t m_used;
    sockaddr_in m_laddr;
    sockaddr_in m_raddr;
    bool m_isbusy;
};

template<EdoyunOperator>
class AcceptOverlapped :public EdoyunOverlapped, ThreadFuncBase
{
public:
    AcceptOverlapped();
    int AcceptWorker();
};


template<EdoyunOperator>
class RecvOverlapped :public EdoyunOverlapped, ThreadFuncBase
{
public:
    RecvOverlapped();
    int RecvWorker() {
        int ret = m_client->Recv();
        return ret;
    }
};

template<EdoyunOperator>
class SendOverlapped :public EdoyunOverlapped, ThreadFuncBase
{
public:
    SendOverlapped();
    int SendWorker() {
        return -1;
    }
};

template<EdoyunOperator>
class ErrorOverlapped :public EdoyunOverlapped, ThreadFuncBase
{
public:
    ErrorOverlapped();
    int ErrorWorker() {
        return -1;
    }
};
class EdoyunServer :
    public ThreadFuncBase
{
public:
    EdoyunServer(const std::string& ip = "0.0.0.0", short port = 9527) :m_pool(10) {
        m_hIOCP = INVALID_HANDLE_VALUE;
        m_sock = INVALID_SOCKET;
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(port);
        m_addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());

    }
    ~EdoyunServer() {}
    bool StartService();

    bool NewAccept() {
        PCLIENT pClient(new EdoyunClient());
        pClient->SetOverlapped(pClient);
        m_client.insert({ *pClient,pClient });
        if (AcceptEx(m_sock, *pClient, *pClient, 0,
            sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
            *pClient, *pClient) == FALSE)
        {
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            m_hIOCP = INVALID_HANDLE_VALUE;
            return false;
        }
        return true;
    }

private:
    void CreateSocket() {
        m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        int opt = 1;
        setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    }
    int threadIocp();
private:
    EdoyunThreadPool m_pool;
    HANDLE m_hIOCP;
    SOCKET m_sock;
    std::map<SOCKET, std::shared_ptr<EdoyunClient>>m_client;
    sockaddr_in m_addr;
};