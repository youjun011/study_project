#include "pch.h"
#include "ServerSocket.h"

//CServerSocket server; 
CServerSocket* CServerSocket::m_instance = NULL;//只能显示的初始化；

//CServerSocket::CHelper CServerSocket::m_helper;//显示初始化静态变量；
//CServerSocket* pserver = CServerSocket::getInstance();