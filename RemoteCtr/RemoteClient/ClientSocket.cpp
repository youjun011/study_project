#include "pch.h"
#include "ClientSocket.h"

//CServerSocket server; 
CClientSocket* CClientSocket::m_instance = NULL;//ֻ����ʾ�ĳ�ʼ����

CClientSocket::CHelper CClientSocket::m_helper;//��ʾ��ʼ����̬������
CClientSocket* pclient = CClientSocket::getInstance();