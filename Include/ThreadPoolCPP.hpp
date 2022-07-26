#pragma once

#include "ThreadPoolBase.hpp"
#include <vector>
#include <thread>
#include <tuple>
#include <mutex>
#include <functional>

namespace Threading {
	class ThreadPoolCPP : public ThreadPoolBase {
	public:
		using base_type = ThreadPoolBase;
		using Config = typename base_type::Config;
		using thread_type = std::thread;
		using thread_container = std::vector<thread_type>;

		using lock_type = std::unique_lock<std::mutex>;

	protected:
		std::mutex _workMutex;
		std::mutex _sleepMutex;
		std::condition_variable _conditionVariable;
		thread_container _threads;
		std::atomic_uint64_t _waitingThreads;
	public:
		ThreadPoolCPP(Config config = Config()) : _waitingThreads(0), base_type(config) {
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				_threads.push_back(thread_type(&ThreadPoolCPP::FunctionWrapper, this));
			}
			Wait();
		}

		~ThreadPoolCPP() {
			Resume();
			Wait();
			Stop();
			for (thread_type& t : _threads) {
				if (t.joinable()) {
					t.join();
				}
			}
		}

		virtual void Push(work_base* work) override {
			lock_type lock(_workMutex);
			_works.push(work);
			WakeOne();
		}

		using base_type::Push;

		virtual void WakeOne() override {
			_conditionVariable.notify_one();
		}

		virtual void WakeAll() override {
			_conditionVariable.notify_all();
		}

		virtual void Wait() override {
			// Wait until all threads are waiting
			while (_waitingThreads < _threads.size()) {
				std::this_thread::yield();
			}
			// This lock is required so that Wake cannot be called before all threads are asleep
			lock_type lock(_sleepMutex);
		}

		virtual std::size_t Size() override {
			return _threads.size();
		}

		void FunctionWrapper() {
			while (base_type::_run) {
				// Sleep thread
				{
					lock_type lock(_sleepMutex);
					++_waitingThreads;
					_conditionVariable.wait(lock);
					--_waitingThreads;
				}

				// Loop work execution
				while (!_works.empty() && !_pause) {
					// Acquire lock and ensure there is work to be done
					lock_type lock(_workMutex);
					if (_works.empty()) {
						break;
					}
					work_base* work = _works.front();
					_works.pop();
					lock.unlock();
					
					work->Execute();
				}
			}
		}
	};
}