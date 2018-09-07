#include "TimerThread.h"
#include "Defines.h"
#include "GameObject.h"

TimerThread::TimerThread()
{
}

TimerThread::~TimerThread()
{
}

void TimerThread::Run()
{
	while (true) {
		std::this_thread::sleep_for(std::chrono::duration<float>(0.01f));
		while (false == _timerQueue.empty()) {
			if (_timerQueue.top().time >= GetTickCount()) break;
			timerLock.lock();
			Event ev = _timerQueue.top();
			_timerQueue.pop();
			timerLock.unlock();
			EXOver *ex = new EXOver;
			ex->eventType = ev.type;
			ex->eventTarget = ev.target;
			PostQueuedCompletionStatus(Thread::GetIocp(), 1, reinterpret_cast<ULONG_PTR>(ev.object), &ex->wsaOver);

		}
	}
}

void TimerThread::AddTimer(GameObject* id, int type, unsigned int time, GameObject* target)
{
	timerLock.lock();
	_timerQueue.push(Event{ time, type, id, target });
	timerLock.unlock();
}