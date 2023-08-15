#pragma once

#include "ThreadPoolBase.hpp"
#include <vector>
#include <queue>
#include <thread>
#include <tuple>
#include <mutex>

namespace Threading {
	template <class..._ArgsTy>
	struct Thread {
		using work_type = std::tuple<_ArgsTy...>;
		using lock_type = std::unique_lock<std::mutex>;

		std::mutex workMutex;
		std::queue<work_type> workBuffer;
		std::thread thread;
		std::condition_variable conditionVariable;
		bool waiting;
		bool run;
		long long averageTimeRun;
		long long averageTimeRunRecent;
		long long countExecution;

		template <typename _FuncTy>
		Thread(_FuncTy functor) : run(true), waiting(false), thread(&Thread:WorkFunction<decltype(functor)>, this, functor), workBuffer() {

		}

		template <typename _RetTy>
		Thread(_RetTy(*functor)(_ArgsTy...)) : run(true), waiting(false), thread(&Thread:WorkFunction<decltype(functor)>, this, functor), workBuffer() {

		}

		template <typename _RetTy, class _ObjTy>
		Thread(_RetTy(_ObjTy::*functor)(_ArgsTy...), _ObjTy* obj) : run(true), waiting(false), thread(&Thread:WorkFunction<decltype(functor), _ObjTy>, this, functor, obj), workBuffer() {

		}

		template <typename _RetTy, class _ObjTy>
		Thread(_RetTy(_ObjTy::*functor)(_ArgsTy...) const, _ObjTy const* obj) : run(true), waiting(false), thread(&Thread:WorkFunction<decltype(functor), _ObjTy const>, this, functor, obj), workBuffer() {

		}

		void Push(work_type& work) {
			lock_type lock(workMutex);
			workBuffer.push(work);
		}

		void Push(work_type&& work) {
			lock_type lock(workMutex);
			workBuffer.push(work);
		}

		template <typename _FuncTy>
		void FunctionWrapper(_FuncTy functor) {
			while (run) {
				// Sleep thread
				{
					lock_type lock(workMutex);
					waiting = true;
					conditionVariable.wait(lock, [this]() { return !workBuffer.empty(); });
					waiting = false;
				}

				// Loop work execution
				while (!workBuffer.empty()) {
					lock_type lock(_workMutex);
					work_type work = _works.front();
					_works.pop();
					lock.unlock();

					std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
					ThreadPoolBase::_Execute(functor, work);
					std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
					std::chrono::steady_clock::duration diff = end - start;
					++countExecution;
					averageTimeRun += (diff.count() - averageTimeRun) / countExecution;
					averageTimeRunRecent += (diff.count() - averageTimeRunRecent) / (countExecution < 1000) ? countExecution : 1000;
				}
			}
		}

		template <typename _FuncTy, class _ObjTy>
		void FunctionWrapper(_FuncTy functor, _ObjTy* obj) {
			while (run) {
				// Sleep thread
				{
					lock_type lock(workMutex);
					waiting = true;
					conditionVariable.wait(lock, [this]() { return !workBuffer.empty(); });
					std::atomic_thread_fence
					waiting = false;
				}

				// Loop work execution
				while (!workBuffer.empty()) {
					lock_type lock(_workMutex);
					work_type work = _works.front();
					_works.pop();
					lock.unlock();

					std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
					ThreadPoolBase::_Execute(functor, obj, work);
					std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
					std::chrono::steady_clock::duration diff = end - start;
					++countExecution;
					averageTimeRun += (diff.count() - averageTimeRun) / countExecution;
					averageTimeRunRecent += (diff.count() - averageTimeRunRecent) / (countExecution < 1000) ? countExecution : 1000;
				}
			}
		}
	};

	template <class... _ArgsTy>
	class ThreadPoolCPP {
	public:
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
		thread_type _watcherThread;
		std::atomic_uint64_t _waitingThreads;
		ThreadPoolCPP(Config config) : _config(config), _run(true), _pause(false), _waitingThreads(0) {

		}
	public:
		template <typename _FuncTy>
		ThreadPoolCPP(_FuncTy functor, Config config = Config()) : ThreadPoolCPP(config) {
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				_threads.push_back(thread_type(&ThreadPoolCPP::FunctionWrapper<_FuncTy>, this, functor));
			}
			Wait();
		}

		template <typename _RetTy>
		ThreadPoolCPP(_RetTy(*functor)(_ArgsTy...), Config config = Config()) : ThreadPoolCPP(config) {
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				_threads.push_back(thread_type(&ThreadPoolCPP::FunctionWrapper<decltype(functor)>, this, functor));
			}
			Wait();
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolCPP(_RetTy(_ObjTy::* functor)(_ArgsTy...), _ObjTy* obj, Config config = Config()) : ThreadPoolCPP(config) {
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				_threads.push_back(thread_type(&ThreadPoolCPP::FunctionWrapper<decltype(functor), _ObjTy>, this, functor, obj));
			}
			Wait();
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolCPP(_RetTy(_ObjTy::* functor)(_ArgsTy...) const, _ObjTy const* obj, Config config = Config()) : ThreadPoolCPP(config) {
			for (decltype(_config.startingThreads) i = 0; i < _config.startingThreads; ++i) {
				_threads.push_back(thread_type(&ThreadPoolCPP::FunctionWrapper<decltype(functor), _ObjTy const>, this, functor, obj));
			}
			Wait();
		}

		~ThreadPoolCPP() {
			Resume();
			Stop();
			Wait();
			for (thread_type& t : _threads) {
				if (t.joinable()) {
					t.join();
				}
			}
		}

		void Push(work_type& work) {
			lock_type lock(_workMutex);
			_works.push(work);
			WakeOne();
		}

		void Push(work_type&& work) {
			lock_type lock(_workMutex);
			_works.push(work);
			WakeOne();
		}

		void Push(_ArgsTy...args) {
			Push(std::forward_as_tuple(args...));
		}

		template <class _Iter>
		void Push(_Iter begin, _Iter end) {
			while (begin != end) {
				Push(*begin);
				++begin;
			}
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

		void Stop() {
			_run = false;
		}

		void Pause() {
			_pause = true;
		}

		void Resume() {
			_pause = false;
		}

		std::size_t Size() {
			return _threads.size();
		}

		template <typename _FuncTy>
		void FunctionWrapper(_FuncTy functor) {
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
					work_type work = _works.front();
					_works.pop();
					lock.unlock();

					ThreadPoolBase::_Execute(functor, work);
				}
			}
		}

		template <typename _FuncTy, class _ObjTy>
		void FunctionWrapper(_FuncTy functor, _ObjTy* obj) {
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
					work_type work = _works.front();
					_works.pop();
					lock.unlock();

					ThreadPoolBase::_Execute(functor, obj, work);
				}
			}
		}
	};
}