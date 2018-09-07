#pragma once
#include <atomic>
#include <mutex>

template <typename T>
class SingleTone {
public:
	SingleTone() {};
public:
	static T* GET_INSTANCE()
	{
		if (pinstance_ == nullptr) {
			std::lock_guard<std::mutex> lock(m_);
			if (pinstance_ == nullptr) {
				pinstance_ = new T();
			}
		}
		return pinstance_;
	};
private:
	static std::atomic<T*> pinstance_;
	static std::mutex m_;
};

template <typename T> std::atomic<T*> SingleTone<T>::pinstance_{ nullptr };
template <typename T> std::mutex SingleTone<T>::m_;
