#include "pch.h"
#include "ClientSocket.h"

//CServerSocket server; 
int test = 0;
CClientSocket* CClientSocket::m_instance = NULL;//ֻ����ʾ�ĳ�ʼ����

CClientSocket::CHelper CClientSocket::m_helper;//��ʾ��ʼ����̬������
CClientSocket* pclient = CClientSocket::getInstance();

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
	while (m_sock != INVALID_SOCKET) {
		if (m_listSend.size() > 0) {
			CPacket& head = m_listSend.front();
			if (Send(head) == false) {
				TRACE("����ʧ�ܣ�\r\n");
				continue;
			}
			auto pr = m_mapAck.insert({ head.hEvent,{} });
			int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
			if (length > 0 || index > 0) {
				index += length;
				size_t size = (size_t)index;
				CPacket pack((BYTE*)pBuffer, size);
				if (size > 0) {//TODO:�����ļ�����Ϣ��ȡ���ļ���Ϣ��ȡ���ܲ������⣻
					pack.hEvent = head.hEvent;
					pr.first->second.push_back(pack);
					SetEvent(head.hEvent);
				}
			}
			else if (length <= 0 && index <= 0) {
				CloseSocket();
			}
			m_listSend.pop_front();
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
