#pragma once
#include<memory>
#include<WinSock2.h>
enum ETYPE
{
	ETypeTCP=1,
	ETypeUDP
};

class ESockaddrIn
{
public:
	ESockaddrIn() {
		memset(&m_addr, 0, sizeof(sockaddr_in));
		m_nPort = -1;
	}
	ESockaddrIn(sockaddr_in addr) {
		memcpy(&m_addr, &addr, sizeof(addr));
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_nPort = ntohs(m_addr.sin_port);
	}
	ESockaddrIn(UINT nIP, short nPort) {
		m_nPort = nPort;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(nPort);
		m_addr.sin_addr.s_addr = htonl(nIP);
		m_ip = inet_ntoa(m_addr.sin_addr);
	}
	ESockaddrIn(const std::string& strIP, short nPort) {
		m_ip = strIP;
		m_nPort = nPort;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(nPort);
		m_addr.sin_addr.s_addr = inet_addr(strIP.c_str());
	}
	ESockaddrIn(const ESockaddrIn& addr) {
		memcpy(&m_addr, &addr.m_addr, sizeof(addr.m_addr));
		m_ip = addr.m_ip;
		m_nPort = addr.m_nPort;
	}
	ESockaddrIn& operator=(const ESockaddrIn& addr) {
		if (this != &addr) {
			memcpy(&m_addr, &addr.m_addr, sizeof(addr.m_addr));
			m_ip = addr.m_ip;
			m_nPort = addr.m_nPort;
		}
		return *this;
	}
	operator sockaddr* ()const {
		return (sockaddr*)&m_addr;
	}
	operator void* () {
		return (void*)&m_addr;
	}
	void update() {
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_nPort = ntohs(m_addr.sin_port);
	}

	std::string GetIP()const {
		return m_ip;
	}
	short GetPort()const {
		return m_nPort;
	}
	inline int size()const { return sizeof(sockaddr_in); }
	const char* Data() {
		strOut.resize(m_ip.size() + sizeof(sockaddr_in) + sizeof(short));
		char* pData = (char*)strOut.c_str();
		memcpy(pData, m_ip.c_str(), m_ip.size()); pData += m_ip.size();
		memcpy(pData, (void*)&m_addr, sizeof(sockaddr_in)); pData += sizeof(sockaddr_in);
		*(short*)pData = m_nPort;
		return strOut.c_str();
	}
private:
	std::string m_ip;
	sockaddr_in m_addr;
	short m_nPort;
	std::string strOut;
};

class EBuffer :public std::string {
public:
	EBuffer(size_t size = 0) :std::string() {
		if (size > 0) {
			resize(size);
			memset((void*)c_str(), 0, size);
		}
	}
	EBuffer(void* buffer, size_t size)  {
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
	EBuffer(const char* str) :std::string(str) {

	}
	~EBuffer() {
		std::string::~basic_string();
	}
	operator char* ()const { return (char*)c_str(); }
	operator const char* ()const { return c_str(); }
	operator BYTE* ()const { return (BYTE*)c_str(); }
	operator void* ()const { return (void*)c_str(); }
	void Update(void* buffer, size_t size) {
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
};

class ESocket
{
public:
	ESocket(ETYPE nType = ETypeTCP, int nProtocol = 0) {
		m_socket = socket(PF_INET, (int)nType, nProtocol);
		m_type = nType;
		m_protocol = nProtocol;
	}

	ESocket(const ESocket& sock) {
		m_socket = socket(PF_INET, (int)sock.m_type, sock.m_protocol);
		m_type = sock.m_type;
		m_protocol = sock.m_protocol;
		m_addr = sock.m_addr;
	}

	~ESocket() {
		closesocket(m_socket);
	}
	ESocket& operator=(const ESocket& sock) {
		if (this != &sock) {
			m_socket = socket(PF_INET, (int)sock.m_type, sock.m_protocol);
			m_type = sock.m_type;
			m_protocol = sock.m_protocol;
			m_addr = sock.m_addr;
		}
		return *this;
	}
	operator SOCKET()const { return m_socket; }
	operator SOCKET() { return m_socket; }
	bool operator==(SOCKET sock)const {
		return m_socket == sock;
	}
	int listen(int backlog = 5) {
		if (m_type != ETypeTCP)return -1;
		return ::listen(m_socket, backlog);
	}
	int bind(const std::string& ip, short port) {
		m_addr = ESockaddrIn(ip, port);
		return ::bind(m_socket, m_addr, m_addr.size());
	}
	int accept(){}
	int connect(const std::string&ip,short port){}
	int send(const EBuffer&buf){
		return ::send(m_socket, buf, buf.size(), 0);
	}
	int recv(const EBuffer& buffer){
		return ::recv(m_socket, buffer, buffer.size(), 0);
	}
	int sendto(const EBuffer& buffer,const ESockaddrIn& to){
		return ::sendto(m_socket, buffer, buffer.size(), 0, to, to.size());
	}
	int recvfrom(EBuffer& buffer, ESockaddrIn& from){
		int len = from.size();
		int ret = ::recvfrom(m_socket, buffer, buffer.size(), 0, from, &len);
		if (ret > 0) { from.update(); }
		return ret;
	}
	void close() {
		if (m_socket == INVALID_SOCKET)return;
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
private:
	SOCKET m_socket;
	ETYPE m_type;
	int m_protocol;
	ESockaddrIn m_addr;
};

typedef std::shared_ptr<ESocket> ESOCKET;

