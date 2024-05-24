#pragma once

template<class T>
class CEdoyunQueue
{//线程安全的队列（利用IOCP实现）
public:
	CEdoyunQueue();
	~CEdoyunQueue();
	bool PushBack(const T& data);
	bool PopFront(T& data);
	size_t Size();
	void Clear();
private:
	static void threadEntry(void* arg);
	void threadMain();
private:
	std::list<T>m_lstData;
	HANDLE m_hCompeletionPort;
	HANDLE m_hThread;

public:
	typedef struct IocpParam {
		int nOperator;
		T strData;
		_beginthread_proc_type cbFunc;//回调函数
		HANDLE hEvent;//pop操作用的
		IocpParam(int op, const char* sData, _beginthread_proc_type cb = NULL) {
			nOperator = op;
			strData = sData;
		}
		IocpParam() {
			nOperator = -1;
		}
	}PPARAM; //Post Parameter 用于投递信息的结构体

	enum
	{
		EQPush,
		EQPop,
		EQsize,
		EQClear
	};
};

