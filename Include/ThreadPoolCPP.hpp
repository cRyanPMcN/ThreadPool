#pragma once

#include "ThreadPoolBase.hpp"
#include <vector>
#include <queue>
#include <thread>
#include <tuple>
#include <mutex>

namespace Threading {

	template <class... _ArgsTy>
	class ThreadPoolCPP : public ThreadPoolBase<_ArgsTy...> {
	public:
		using base_type = ThreadPoolBase<_ArgsTy...>;
		using Config = typename base_type::Config;
		using thread_type = std::thread;
		using thread_container = std::vector<thread_type>;

		using lock_type = std::unique_lock<std::mutex>;

		using work_type = std::tuple<_ArgsTy...>;
		using work_container = std::queue<work_type>;
	protected:
		Config _config;
		work_container _works;
		bool _run;
		bool _pause;
		std::mutex _workMutex;
		std::mutex _sleepMutex;
		std::condition_variable _conditionVariable;
		thread_container _threads;
		std::atomic_uint64_t _waitingThreads;
	public:
		template <typename _FuncTy>
		ThreadPoolCPP(_FuncTy functor, Config config = Config()) : _waitingThreads(0), base_type(config) {
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				_threads.push_back(thread_type(&ThreadPoolCPP::FunctionWrapper<_FuncTy>, this, functor));
			}
			Wait();
		}

		template <typename _RetTy>
		ThreadPoolCPP(_RetTy(*functor)(_ArgsTy...), Config config = Config()) : _waitingThreads(0), base_type(config) {
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				_threads.push_back(thread_type(&ThreadPoolCPP::FunctionWrapper<decltype(functor)>, this, functor));
			}
			Wait();
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolCPP(_RetTy(_ObjTy::* functor)(_ArgsTy...), _ObjTy* obj, Config config = Config()) : _waitingThreads(0), base_type(config) {
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				_threads.push_back(thread_type(&ThreadPoolCPP::FunctionWrapper<decltype(functor), _ObjTy>, this, functor, obj));
			}
			Wait();
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolCPP(_RetTy(_ObjTy::* functor)(_ArgsTy...) const, _ObjTy const* obj, Config config = Config()) : _waitingThreads(0), base_type(config) {
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				_threads.push_back(thread_type(&ThreadPoolCPP::FunctionWrapper<decltype(functor), _ObjTy const>, this, functor, obj));
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

		virtual void Push(_ArgsTy...args) {
			Push(std::forward_as_tuple(args...));
		}

		template <class _Iter>
		void Push(_Iter begin, _Iter end) {
			while (begin != end) {
				lock_type lock(_workMutex);
				Push(*begin);
				++begin;
				WakeAll();
			}
		}

		void Push(work_type const& work) {
			lock_type lock(_workMutex);
			_works.push(work);
			WakeOne();
		}

		void Push(work_type const&& work) {
			lock_type lock(_workMutex);
			_works.push(work);
			WakeOne();
		}

		void WakeOne() {
			_conditionVariable.notify_one();
		}

		void WakeAll() {
			_conditionVariable.notify_all();
		}

		void Wait() {
			// Wait until all threads are waiting
			while (_waitingThreads < _threads.size() || !(_works.empty() || _pause)) {
				std::this_thread::yield();
			}
			// This lock is required so that Wake cannot be called before all threads are asleep
			lock_type lock(_sleepMutex);
		}

		std::size_t Size() {
			return _threads.size();
		}

		template <typename _FuncTy>
		void FunctionWrapper(_FuncTy functor) {
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
					work_type work = _works.front();
					_works.pop();
					lock.unlock();

					base_type::_Execute(functor, work);
				}
			}
		}

		template <typename _FuncTy, class _ObjTy>
		void FunctionWrapper(_FuncTy functor, _ObjTy* obj) {
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
					work_type work = _works.front();
					_works.pop();
					lock.unlock();

					_Execute(functor, obj, work);
				}
			}
		}
	};
}