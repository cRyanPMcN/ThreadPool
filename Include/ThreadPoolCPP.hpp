#pragma once

#include "ThreadPoolImpl.hpp"
#include <vector>
#include <thread>
#include <tuple>
#include <mutex>
#include <functional>

namespace Threading {

	template <class... _ArgsTy>
	class ThreadPoolCPP : public ThreadPoolImpl<_ArgsTy...> {
	public:
		using base_type = ThreadPoolImpl<_ArgsTy...>;
		using Config = typename base_type::Config;

		using lock_type = std::unique_lock<std::mutex>;

		using work_type = typename base_type::work_type;
		using work_container = typename base_type::work_container;

		using thread_type = struct _thrd {
			bool live;

			long long averageExecutionTime;
			long long minimumExecutionTime;
			long long maximumExecutionTime;

			unsigned long long executionCount;

			std::thread thread;
			
			work_container threadWorks;

			std::mutex _workMutex;

			std::condition_variable _conditionVariable;

			_thrd() : live(true), averageExecutionTime(0), minimumExecutionTime(std::numeric_limits<long long>::max()), maximumExecutionTime(0), executionCount(0) {

			}

			~_thrd() {
				if (thread.joinable()) {
					thread.join();
				}
			}

			void Sleep() {
				_conditionVariable.wait(std::unique_lock<std::mutex>(_workMutex));
			}
		};

		using thread_container = std::vector<thread_type*>;
	protected:
		work_container _works;
		std::mutex _workMutex;
		std::mutex _sleepMutex;
		std::condition_variable _conditionVariable;
		thread_container _threads;
		std::thread _watcherThread;
		std::atomic_ullong _waitingThreads;
	public:
		template <typename _FuncTy>
		ThreadPoolCPP(_FuncTy functor, Config config = Config()) : _waitingThreads(0), base_type(config) {
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				thread_type* thread = new thread_type();
				thread->thread = std::thread(&ThreadPoolCPP::FunctionWrapper<_FuncTy>, this, thread, functor);
				_threads.push_back(thread);
			}
			Wait();
		}

		template <typename _RetTy>
		ThreadPoolCPP(_RetTy(*functor)(_ArgsTy...), Config config = Config()) : _waitingThreads(0), base_type(config) {
			// _watcherThread(&ThreadPoolCPP::ThreadWrapper<decltype(functor)>, this, functor), 
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				thread_type* thread = new thread_type();
				thread->thread = std::thread(&ThreadPoolCPP::FunctionWrapper<decltype(functor)>, this, thread, functor);
				_threads.push_back(thread);
			}
			Wait();
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolCPP(_RetTy(_ObjTy::* functor)(_ArgsTy...), _ObjTy* obj, Config config = Config()) : _waitingThreads(0), base_type(config) {
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				thread_type* thread = new thread_type();
				thread->thread = std::thread(&ThreadPoolCPP::FunctionWrapper<decltype(functor), _ObjTy>, this, thread, functor, obj);
				_threads.push_back(thread);
			}
			Wait();
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolCPP(_RetTy(_ObjTy::* functor)(_ArgsTy...) const, _ObjTy const* obj, Config config = Config()) : _waitingThreads(0), base_type(config) {
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				thread_type* thread = new thread_type();
				thread->thread = std::thread(&ThreadPoolCPP::FunctionWrapper<decltype(functor), _ObjTy const>, this, thread, functor, obj);
				_threads.push_back(thread);
			}
			Wait();
		}

		~ThreadPoolCPP() {
			Resume();
			Wait();
			Stop();
			//if (_watcherThread.joinable()) {
			//	_watcherThread.join();
			//}
		}

		using base_type::Push;

		virtual void Push(work_type const& work) override {
			lock_type lock(_workMutex);
			_works.push(work);
			WakeOne();
		}

		virtual void Push(work_type const&& work) override {
			lock_type lock(_workMutex);
			_works.push(work);
			WakeOne();
		}

		virtual void WakeOne() override {
			_conditionVariable.notify_one();
		}

		virtual void WakeAll() override {
			_conditionVariable.notify_all();
		}

		virtual void Wait() override {
			// Wait until all threads are waiting
			while (_waitingThreads < _threads.size() || !(_works.empty() || _pause)) {
				std::this_thread::yield();
			}
			// This lock is required so that Wake cannot be called before all threads are asleep
			lock_type lock(_sleepMutex);
		}

		virtual void Resume() override {
			Resume();
			Wake(_works.size());
		}

		virtual std::size_t Size() override {
			return _threads.size();
		}

		template <typename _FuncTy>
		void FunctionWrapper(thread_type* threadData, _FuncTy functor) {
			while (_run && threadData->live) {
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
					work_type work = _works.front();
					_works.pop();
					lock.unlock();
				   	
					std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
					_Execute(functor, work, std::make_index_sequence<sizeof...(_ArgsTy)>());
					std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
					long long executionTime = (end - start).count();
					++(threadData->executionCount);
					
					if (threadData->minimumExecutionTime < executionTime) {
						threadData->minimumExecutionTime = executionTime;
					}
					
					if (threadData->maximumExecutionTime > executionTime) {
						threadData->maximumExecutionTime = executionTime;
					}

					threadData->averageExecutionTime += ((threadData->averageExecutionTime - executionTime) / _config.averageCalculationCount);
				}
			}
		}

		template <typename _FuncTy, class _ObjTy>
		void FunctionWrapper(thread_type* threadData, _FuncTy functor, _ObjTy* obj) {
			while (_run && threadData->live) {
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
					work_type work = _works.front();
					_works.pop();
					lock.unlock();
					
					std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
					_Execute(functor, obj, work, std::make_index_sequence<sizeof...(_ArgsTy)>());
					std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
					long long executionTime = (end - start).count();
					++threadData->executionCount;
					
					if (threadData->minimumExecutionTime < executionTime) {
						threadData->minimumExecutionTime = executionTime;
					}
					
					if (threadData->maximumExecutionTime > executionTime) {
						threadData->maximumExecutionTime = executionTime;
					}

					threadData->averageExecutionTime += ((threadData->averageExecutionTime - executionTime) / _config.averageCalculationCount);
				}
			}
		}

		template <typename _FuncTy>
		void WatcherFunction(_FuncTy functor) {
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				thread_type thread;
				thread.thread = std::thread(&ThreadPoolCPP::FunctionWrapper<_FuncTy>, this, std::ref(thread), functor);
				_threads.push_back(thread);
			}
		
			float averageWorks = 0;
			float averageWaitingThreads = 0;
		
			while (_run) {
				averageWorks = averageWorks + ((static_cast<float>(_works.size()) - averageWorks) / _config.averageCalculationCount);
				averageWaitingThreads = averageWaitingThreads + ((static_cast<float>(_waitingThreads.load()) - averageWaitingThreads) / _config.averageCalculationCount);
		
				if (averageWaitingThreads < 1 && averageWorks > 1 && _threads.size() < _config.maximumThreads) {
					thread_type thread;
					thread.thread = std::thread(&ThreadPoolCPP::FunctionWrapper<_FuncTy>, this, std::ref(thread), functor);
					_threads.push_back(thread);
				}
				
				if (averageWaitingThreads > 1 && averageWorks < _threads.size()) {
					while (averageWaitingThreads > 1 && _threads.size() > _config.minimumThreads) {
						thread_type* data = _threads.back();
						data.live = false;
						--averageWaitingThreads;
						WakeAll();
						data.thread.join();
						_threads.pop_back();
						delete data;
					}
				}
		
				std::this_thread::sleep_for(std::chrono::microseconds(100));
			}
		}
		
		template <typename _FuncTy, class _ObjTy>
		void WatcherFunction(_FuncTy functor, _ObjTy* obj) {
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				thread_type thread;
				thread.thread = std::thread(&ThreadPoolCPP::FunctionWrapper<_FuncTy, _ObjTy>, this, std::ref(thread), functor, obj);
				_threads.push_back(thread);
			}
		
			float averageWorks = 0;
			float averageWaitingThreads = 0;
		
			while (_run) {
				averageWorks = averageWorks + ((static_cast<float>(_works.size()) - averageWorks) / _config.averageCalculationCount);
				averageWaitingThreads = averageWaitingThreads + ((static_cast<float>(_waitingThreads.load()) - averageWaitingThreads) / _config.averageCalculationCount);
		
				if (averageWaitingThreads < 1 && averageWorks > 1 && _threads.size() < _config.maximumThreads) {
					thread_type thread;
					thread.thread = std::thread(&ThreadPoolCPP::FunctionWrapper<_FuncTy, _ObjTy>, this, std::ref(thread), functor, obj);
					_threads.push_back(thread);
				}
				
				if (averageWaitingThreads > 1 && averageWorks < _threads.size()) {
					while (averageWaitingThreads > 1 && _threads.size() > _config.minimumThreads) {
						thread_type* data = _threads.back();
						data.live = false;
						--averageWaitingThreads;
						WakeAll();
						data.thread.join();
						_threads.pop_back();
						delete data;
					}
				}

				std::this_thread::sleep_for(std::chrono::microseconds(100));
			}
		}
	};
}