#include "pch.h"
#include "ClientSocket.h"

//CServerSocket server; 
int test = 0;
CClientSocket* CClientSocket::m_instance = NULL;//只能显示的初始化；

//CClientSocket::CHelper CClientSocket::m_helper;//显示初始化静态变量；
CClientSocket* pclient = CClientSocket::getInstance();