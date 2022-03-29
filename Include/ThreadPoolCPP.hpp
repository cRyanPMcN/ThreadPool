#pragma once

#include "ThreadPoolImpl.hpp"
#include <vector>
#include <queue>
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
		using thread_type = std::thread;
		using thread_container = std::vector<thread_type>;
		using work_type = typename base_type::work_type;
		using work_container = typename base_type::work_container;
	protected:
		std::queue<work_type> _works;
		std::mutex _workMutex;
		std::condition_variable _conditionVariable;
		thread_container _threads;
		//thread_type _watcherThread;
		std::atomic_uint64_t _waitingThreads;
	public:
		template <typename _FuncTy>
		ThreadPoolCPP(_FuncTy functor, Config config = Config()) : _waitingThreads(0), base_type(config) {
			// _watcherThread(&ThreadPoolCPP::ThreadWrapper<_FuncTy>, this, functor),
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper<_FuncTy>, this, functor));
			}
			Wait();
		}

		template <typename _RetTy>
		ThreadPoolCPP(_RetTy(*functor)(_ArgsTy...), Config config = Config()) : _waitingThreads(0), base_type(config) {
			// _watcherThread(&ThreadPoolCPP::ThreadWrapper<decltype(functor)>, this, functor), 
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper<decltype(functor)>, this, functor));
			}
			Wait();
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolCPP(_RetTy(_ObjTy::* functor)(_ArgsTy...), _ObjTy* obj, Config config = Config()) : _waitingThreads(0), base_type(config) {
			// _watcherThread(&ThreadPoolCPP::ThreadWrapper<decltype(functor), _ObjTy>, this, functor, obj), 
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper<decltype(functor), _ObjTy*>, this, functor, obj));
			}
			Wait();
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolCPP(_RetTy(_ObjTy::* functor)(_ArgsTy...) const, _ObjTy const* obj, Config config = Config()) : _waitingThreads(0), base_type(config) {
			// _watcherThread(&ThreadPoolCPP::ThreadWrapper<decltype(functor), _ObjTy const>, this, functor, obj), 
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper<decltype(functor), _ObjTy const*>, this, functor, obj));
			}
			Wait();
		}

		~ThreadPoolCPP() {
			Stop();
			for (std::thread& t : _threads) {
				if (t.joinable()) {
					t.join();
				}
			}
			//if (_watcherThread.joinable()) {
			//	_watcherThread.join();
			//}
		}

		virtual void Push(work_type const& work) override {
			std::unique_lock<std::mutex> lock(_workMutex);
			_works.push(work);
			WakeOne();
		}

		virtual void Push(work_type const&& work) override {
			std::unique_lock<std::mutex> lock(_workMutex);
			_works.push(work);
			WakeOne();
		}

		virtual void Push(_ArgsTy...args) {
			Push(std::forward_as_tuple(args...));
		}

		virtual void WakeOne() override {
			_conditionVariable.notify_one();
		}

		virtual void Wake(std::size_t number) override {
			while (number) {
				WakeOne();
				--number;
			}
		}

		virtual void WakeAll() override {
			_conditionVariable.notify_all();
		}

		virtual void Wait() override {
			// Wait until all threads are waiting
			while (_waitingThreads < _threads.size()) {
				std::this_thread::yield();
			}
		}

		virtual void Stop() override {
			base_type::_run = false;
			WakeAll();
		}

		template <typename _FuncTy>
		void FunctionWrapper(_FuncTy functor) {
			while (base_type::_run) {
				// Sleep thread
				{
					std::unique_lock<std::mutex> lock(_workMutex);
					++_waitingThreads;
					_conditionVariable.wait(lock);
					--_waitingThreads;
				}

				// Loop work execution
				while (!_works.empty()) {
					// Acquire lock and ensure there is work to be done
					std::unique_lock<std::mutex> lock(_workMutex);
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
		void FunctionWrapper(_FuncTy functor, _ObjTy obj) {
			while (base_type::_run) {
				// Sleep thread
				{
					std::unique_lock<std::mutex> lock(_workMutex);
					++_waitingThreads;
					_conditionVariable.wait(lock);
					--_waitingThreads;
				}

				// Loop work execution
				while (!_works.empty()) {
					// Acquire lock and ensure there is work to be done
					std::unique_lock<std::mutex> lock(_workMutex);
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

		// Code has been removed as it does not function correctly, and I cannot fix it without the rest of the threadpool being functional
		//template <typename _FuncTy>
		//void ThreadWrapper(_FuncTy functor) {
		//	while (_threads.size() < _config.startingThreads) {
		//		_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper<_FuncTy>, this, functor));
		//	}
		//	while (_run) {
		//		if (_waitingThreads == 0 && _threads.size() < _config.maximumThreads) {
		//			_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper<_FuncTy>, this, functor));
		//		}

		//		// TODO: Remove threads, this will probably require a Thread wrapper object
		//	}
		//}

		//template <typename _FuncTy, class _ObjTy>
		//void ThreadWrapper(_FuncTy functor, _ObjTy* obj) {
		//	while (_threads.size() < _config.startingThreads) {
		//		_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper<_FuncTy, _ObjTy>, this, functor, obj));
		//	}
		//	while (_run) {
		//		if (_waitingThreads == 0 && _threads.size() < _config.maximumThreads) {
		//			_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper<_FuncTy, _ObjTy>, this, functor, obj));
		//		}

		//		// TODO: Remove threads, this will probably require a Thread wrapper object
		//	}
		//}
	};
}