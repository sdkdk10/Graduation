#pragma once
#include <thread>
#include "Protocol.h"

class Thread
{

public:
	explicit Thread();
	virtual ~Thread();

public:
	virtual void Init() {
		_thread = std::thread([&]() { Run(); });
	}
	virtual void Run() = 0;
	virtual void Join() {
		_thread.join();
	}

public:
	static void* GetIocp() {
		return iocp_;
	}
	static void SetIocp(void* iocp) {
		iocp_ = iocp;
	}

private:
	std::thread _thread;

private:
	static void* iocp_;
};