#pragma once

#include "ThreadPool.hpp"
#include <vector>
#include <thread>
#include <tuple>

namespace Threading {

	template <class ..._Args>
	class ThreadPoolCPP : public ThreadPool {
	public:
		using thread_container = std::vector<std::thread>;
		using function_type = void(*)(_Args...);
		using work_type = std::tuple<_Args...>;
	protected:
		std::vector<work_type> _works;
		thread_container _threads;
	public:
		ThreadPoolCPP() : ThreadPool() {

		}

		~ThreadPoolCPP() {

		}

		virtual void Wait() override {
			while (!_works.empty()) {
				std::this_thread::yield();
			}
		}
	};
}