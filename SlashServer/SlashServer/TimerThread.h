#pragma once
#include <queue>
#include <vector>
#include <mutex>
#include "Thread.h"
#include "GameObject.h"

struct Event {
	unsigned int time;
	int type;
	GameObject* object;
	GameObject* target;
};

class MyComparison
{
	bool reverse;
public:
	MyComparison() {}
	bool operator() (const Event lhs, const Event rhs) const
	{
		return (lhs.time > rhs.time);
	}
};

class TimerThread : public Thread
{
public:
	explicit TimerThread();
	virtual ~TimerThread();

public:
	virtual void Init() {
		thread_ = std::thread([&]() { TimerThread::Run(); });
	}
	virtual void Run();
	void AddTimer(GameObject * id, int type, unsigned int time, GameObject * target);

private:
	std::mutex timerLock;
	std::priority_queue <Event, std::vector<Event>, MyComparison> _timerQueue;
};