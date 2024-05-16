#include "pch.h"
#include "ClientSocket.h"

//CServerSocket server; 
int test = 0;
CClientSocket* CClientSocket::m_instance = NULL;//只能显示的初始化；

CClientSocket::CHelper CClientSocket::m_helper;//显示初始化静态变量；
CClientSocket* pclient = CClientSocket::getInstance();

bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& listPacks, bool isAutoClosed)
{
	if (m_sock == INVALID_SOCKET) {
		//if (InitSocket() == false)return false;
		TRACE(_T("线程开启\r\n"));
		_beginthread(&CClientSocket::threadEntry, 0, this);
	}
	auto pr = m_mapAck.insert({ pack.hEvent,{} });
	m_mapAutoClosed.insert({ pack.hEvent,isAutoClosed });
	m_listSend.push_back(pack);
	WaitForSingleObject(pack.hEvent, INFINITE);
	auto it = m_mapAck.find(pack.hEvent);
	if (it != m_mapAck.end()) {
		for (auto i = it->second.begin(); i != it->second.end(); i++) {
			listPacks.push_back(*i);
		}
		m_mapAck.erase(it);
		return true;
	}
	return false;
}

void CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc();
	_endthread();
}

void CClientSocket::threadFunc()
{
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	InitSocket();
	while (m_sock != INVALID_SOCKET) {
		if (m_listSend.size() > 0) {
			TRACE("lstSend size:%d\r\n", m_listSend.size());
			CPacket &head = m_listSend.front();
			if (Send(head) == false) {
				TRACE("发送失败！\r\n");
				continue;
			}
			auto it = m_mapAck.find(head.hEvent);
			auto it0 = m_mapAutoClosed.find(head.hEvent);
			do {
				int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
				if (length > 0 || index > 0) {
					index += length;
					size_t size = (size_t)index;
					CPacket pack((BYTE*)pBuffer, size);
					if (size > 0) {//TODO:对于文件夹信息获取，文件信息获取可能产生问题；
						pack.hEvent = head.hEvent;
						it->second.push_back(pack);
						memmove(pBuffer, pBuffer + size, index - size);
						index -= size;
						if(it0->second)
						SetEvent(head.hEvent);
					}
				}
				else if (length <= 0 && index <= 0) {
					CloseSocket();
					SetEvent(head.hEvent);//等到服务器关闭之后，再通知事情完成
				}
			} while (it0->second == false);
			m_listSend.pop_front();
			InitSocket();
		}
	}
	CloseSocket();
}

bool CClientSocket::Send(const CPacket& pack)
{
	if (m_sock == -1)return false;
	std::string strOut;
	pack.Data(strOut);
	return send(m_sock, strOut.c_str(), strOut.size(), 0) > 0;
}
