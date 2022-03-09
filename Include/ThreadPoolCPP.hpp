#pragma once

#include "ThreadPool.hpp"
#include <vector>
#include <queue>
#include <thread>
#include <tuple>
#include <mutex>
#include <functional>

namespace Threading {

	template <class... _ArgsTy>
	class ThreadPoolCPP : public ThreadPool {
	public:
		using thread_container = std::vector<std::thread>;
		using work_type = std::tuple<_ArgsTy...>;
	protected:
		std::queue<work_type> _works;
		std::mutex _workMutex;
		std::condition_variable _conditionVariable;
		thread_container _threads;
		std::thread _watcherThread;
		std::atomic_uint64_t _waitingThreads;
	public:
		template <typename _FuncTy>
		ThreadPoolCPP(_FuncTy functor, Config config = Config()) : _waitingThreads(0), ThreadPool(config) {
			// _watcherThread(&ThreadPoolCPP::ThreadWrapper<_FuncTy>, this, functor),
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper<_FuncTy>, this, functor));
			}
		}

		template <typename _RetTy>
		ThreadPoolCPP(_RetTy(*functor)(_ArgsTy...), Config config = Config()) : _waitingThreads(0), ThreadPool(config) {
			// _watcherThread(&ThreadPoolCPP::ThreadWrapper<decltype(functor)>, this, functor), 
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper<decltype(functor)>, this, functor));
			}
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolCPP(_RetTy(_ObjTy::* functor)(_ArgsTy...), _ObjTy* obj, Config config = Config()) : _waitingThreads(0), ThreadPool(config) {
			// _watcherThread(&ThreadPoolCPP::ThreadWrapper<decltype(functor), _ObjTy>, this, functor, obj), 
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper<decltype(functor), _ObjTy>, this, functor, obj));
			}
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolCPP(_RetTy(_ObjTy::* functor)(_ArgsTy...) const, _ObjTy const* obj, Config config = Config()) : _waitingThreads(0), ThreadPool(config) {
			// _watcherThread(&ThreadPoolCPP::ThreadWrapper<decltype(functor), _ObjTy const>, this, functor, obj), 
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper<decltype(functor), _ObjTy const>, this, functor, obj));
			}
		}

		~ThreadPoolCPP() {
			Stop();
			_conditionVariable.notify_all();
			for (std::thread& t : _threads) {
				if (t.joinable()) {
					t.join();
				}
			}
			//if (_watcherThread.joinable()) {
			//	_watcherThread.join();
			//}
		}

		virtual void Push(work_type const& work) {
			std::unique_lock<std::mutex> lock(_workMutex);
			_works.push(work);
			_conditionVariable.notify_one();
		}

		virtual void Push(work_type const&& work) {
			std::unique_lock<std::mutex> lock(_workMutex);
			_works.push(work);
			_conditionVariable.notify_one();
		}

		virtual void Push(_ArgsTy...args) {
			Push(std::forward_as_tuple(args...));
		}

		template <class _Iter>
		void Push(_Iter begin, _Iter end) {
			std::size_t count = 0;
			while (begin != end) {
				_works.push(*begin);
				++begin;
				++count;
			}

			if (count > _waitingThreads) {
				_conditionVariable.notify_all();
			}
			else {
				while (count != 0) {
					_conditionVariable.notify_one();
				}
			}
		}

		virtual void Wait() override {
			while (_waitingThreads < _threads.size()) {
				std::this_thread::yield();
			}
		}

		template <typename _FuncTy>
		void FunctionWrapper(_FuncTy functor) {
			while (_run) {
				{
					std::unique_lock<std::mutex> lock(_workMutex);
					++_waitingThreads;
					_conditionVariable.wait(lock);
					--_waitingThreads;
				}

				while (!_works.empty()) {
					std::unique_lock<std::mutex> lock(_workMutex);
					if (_works.empty()) {
						break;
					}
					work_type work = _works.front();
					_works.pop();
					lock.unlock();

					_Execute(functor, work, std::make_index_sequence<sizeof...(_ArgsTy)>());
				}
			}
		}

		template <typename _FuncTy, class _ObjTy>
		void FunctionWrapper(_FuncTy functor, _ObjTy* obj) {
			while (_run) {
				{
					std::unique_lock<std::mutex> lock(_workMutex);
					++_waitingThreads;
					_conditionVariable.wait(lock);
					--_waitingThreads;
				}

				while (!_works.empty()) {
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

	protected:
		template <class _FuncTy, size_t..._indexes>
		inline void _Execute(_FuncTy functor, work_type& work, std::index_sequence<_indexes...> indexSequence) {
			std::invoke(functor, std::move(std::get<_indexes>(work))...);
		}

		template <typename _FuncTy, class _ObjTy, size_t..._indexes>
		inline void _Execute(_FuncTy functor, _ObjTy* obj, work_type& work, std::index_sequence<_indexes...> indexSequence) {
			std::invoke(functor, obj, std::move(std::get<_indexes>(work))...);
		}
	};
}