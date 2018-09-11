#pragma once
#include <thread>
#include "Protocol.h"

class Thread
{

public:
	explicit Thread();
	virtual ~Thread();

public:
	virtual void Init() = 0;
	virtual void Run() = 0;
	virtual void Join() {
		thread_.join();
	}
//public:
//	virtual void SetThread(std::thread pthread) {
//		thread_ = std::move(pthread);
//	}
//	virtual void Init() {
//		thread_ = std::thread([&]() { Run(); });
//	}
public:
	static void* GetIocp() {
		return iocp_;
	}
	static void SetIocp(void* iocp) {
		iocp_ = iocp;
	}

protected:
	std::thread thread_;

private:
	static void* iocp_;
};