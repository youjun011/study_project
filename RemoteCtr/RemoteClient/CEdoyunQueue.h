#pragma once

#include "pch.h"
#include <atomic>
#include<list>
template<class T>
class CEdoyunQueue
{//线程安全的队列（利用IOCP实现）
public:
	enum
	{
		EQNone,
		EQPush,
		EQPop,
		EQsize,
		EQClear
	};
	typedef struct IocpParam {
		size_t nOperator;
		T strData;
		HANDLE hEvent;//pop操作用的
		IocpParam(int op, const T& sData,HANDLE hEve=NULL) {
			nOperator = op;
			strData = sData;
			hEvent = hEve;
		}
		IocpParam() {
			nOperator = EQNone;
		}
	}PPARAM; //Post Parameter 用于投递信息的结构体
public:
	CEdoyunQueue() {
		m_lock = false;
		m_hCompeletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 
			NULL, NULL, 1);
		m_hThread = INVALID_HANDLE_VALUE;
		if (m_hCompeletionPort != NULL) {
			m_hThread= (HANDLE)_beginthread(
				&CEdoyunQueue<T>::threadEntry, 0, m_hCompeletionPort);
		}
	}
	~CEdoyunQueue() {
		if (m_lock)return;
		m_lock = true;
		HANDLE hTemp = m_hCompeletionPort;
		PostQueuedCompletionStatus(hIOCP, 0, NULL, NULL);
		WaitForSingleObject(hThread, INFINITE);
		m_hCompeletionPort = NULL;
		CloseHandle(hTemp);
	}
	bool PushBack(const T& data) {
		if (m_lock)return false;
		IocpParam* pParam = new IocpParam(EQPush, data);
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM),
			(ULONG_PTR)pParam, NULL);
		if (ret == false)delete pParam;
		return ret;
	}
	bool PopFront(T& data) {
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		IocpParam Param(EQPop, data, hEvent);
		if (m_lock) {
			if (hEvent) {
				CloseHandle(hEvent);
			}
			return false;
		}
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM),
			(ULONG_PTR)&Param, NULL);
		if (ret == false) {
			CloseHandle(hEvent);
			return false;
		}
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;
		if (ret) {
			data = Param.strData;
		}
		return ret;
	}
	size_t Size() {
		HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		IocpParam Param(EQPop, T(), hEvent);
		if (m_lock) {
			if (hEvent) {
				CloseHandle(hEvent);
			}
			return -1;
		}
		bool ret = PostQueuedCompletionStatus(EQsize, sizeof(PPARAM),
			(ULONG_PTR)&Param, NULL);
		if (ret == false) {
			CloseHandle(hEvent);
			return -1;
		}
		ret = WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0;
		if (ret) {
			return Param.nOperator;
		}
		return -1;
	}
	void Clear() {
		if (m_lock)return false;
		IocpParam* pParam = new IocpParam(EQClear, T());
		bool ret = PostQueuedCompletionStatus(m_hCompeletionPort, sizeof(PPARAM),
			(ULONG_PTR)pParam, NULL);
		if (ret == false)delete pParam;
		return ret;
	}
private:
	static void threadEntry(void* arg) {
		CEdoyunQueue<T>* thiz = (CEdoyunQueue<T>*)arg;
		thiz->threadMain();
		_endthread();
	}
	void threadMain() {
		PPARAM* pParam = NULL;
		DWORD dwTransferred = 0;
		ULONG_PTR CompletionKet = 0;
		OVERLAPPED* pOverlapped = NULL;
		while (GetQueuedCompletionStatus(m_hCompeletionPort,
			&dwTransferred, &CompletionKet, &pOverlapped, INFINITE)) {
			if ((dwTransferred == 0) || (CompletionKet == NULL)) {
				printf("thread is prepare to exit!\r\n");
				break;
			}
			pParam = (PPARAM*)CompletionKet;
			switch (pParam->nOperator)
			{
			case EQPush:
				m_lstData.push_back(pParam->strData);
				delete pParam;
			break;
			case EQPop:
				std::string str;
				if (m_lstData.size() > 0) {
					pParam->strData = m_lstData.front();
					m_lstData.pop_front();
				}
				if (pParam->hEvent != NULL) {
					SetEvent(pParam->hEvent);
				}
			break;
			case EQsize:
				pParam->nOperator = m_lstData.size();
				if (pParam->hEvent != NULL) {
					SetEvent(pParam->hEvent);
				}
			break;
			case EQClear:
				m_lstData.clear();
				delete pParam;
				break;
			default:
				OutputDebugString("unknown operator!\r\n");
				break;
			}
		}
		CloseHandle(m_hCompeletionPort);
	}
private:
	std::list<T>m_lstData;
	HANDLE m_hCompeletionPort;
	HANDLE m_hThread;
	std::atomic<bool> m_lock;//队列正在析构
};

