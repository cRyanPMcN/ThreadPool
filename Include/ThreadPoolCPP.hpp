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
		using thread_type = std::thread;
		using thread_container = std::vector<thread_type>;

		using lock_type = std::unique_lock<std::mutex>;
		using work_type = std::function<void()>;
		using work_container = std::queue<std::function<void()>>;
	protected:
		bool _run;
		bool _pause;
		std::mutex _workMutex;
		work_container _works;
		std::mutex _sleepMutex;
		std::condition_variable _conditionVariable;
		thread_container _threads;
		std::atomic_uint64_t _waitingThreads;
	public:
		ThreadPoolCPP(std::size_t numberThreads) : _waitingThreads(0) {
			for (std::size_t i = 0; i < numberThreads; ++i) {
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

		template <class _FuncTy, class..._ArgsTy>
		void Push(_FuncTy functor, _ArgsTy...args) {
			Push(new work_type<_FuncTy, _ArgsTy...>(functor, args...));
			WakeOne();
		}

		void Stop() {
			_run = false;
		}

		void Resume() {
			_pause = false;
			WakeAll();
		}

		void Pause() {
			_pause = true;
		}

		void WakeOne() {
			_conditionVariable.notify_one();
		}

		void WakeAll() {
			_conditionVariable.notify_all();
		}

		void Wait() {
			// Wait until all threads are waiting
			while ((_waitingThreads < _threads.size()) || !(_works.empty() && !_pause)) {
				std::this_thread::yield();
			}
			// This lock is required so that Wake cannot be called before all threads are asleep
			lock_type lock(_sleepMutex);
		}

		std::size_t Size() {
			return _threads.size();
		}

		void FunctionWrapper() {
			while (_run) {
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
					work_type work(_works.front());
					_works.pop();
					lock.unlock();
					
					work();
				}
			}
		}
	};
}