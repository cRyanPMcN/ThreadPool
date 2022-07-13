#pragma once

#include "ThreadPoolBase.hpp"
#include <vector>
#include <queue>
#include <thread>
#include <tuple>
#include <mutex>
#include <functional>

namespace Threading {

	template <class... _ArgsTy>
	class ThreadPoolCPP : public ThreadPoolBase<_ArgsTy...> {
	public:
		using base_type = ThreadPoolBase<_ArgsTy...>;
		using Config = typename base_type::Config;
		using thread_type = std::thread;
		using thread_container = std::vector<thread_type>;

		using lock_type = std::unique_lock<std::mutex>;

		using work_type = typename base_type::work_type;
		using work_container = typename base_type::work_container;
	protected:
		work_container _works;
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
			base_type::Resume();
			Wake(_works.size());
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

					base_type::_Execute(functor, work, std::make_index_sequence<sizeof...(_ArgsTy)>());
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

					_Execute(functor, obj, work, std::make_index_sequence<sizeof...(_ArgsTy)>());
				}
			}
		}
	};
}