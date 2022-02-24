#pragma once

#include "ThreadPool.hpp"
#include <vector>
#include <queue>
#include <thread>
#include <tuple>
#include <mutex>
#include <functional>

namespace Threading {
	template <class _FuncTy, class ..._Args>
	class ThreadPoolCPP : public ThreadPool {
	public:
		using thread_container = std::vector<std::thread>;
		using function_type = _FuncTy;
		using work_type = std::tuple<_Args...>;
	protected:
		const Config _config;
		function_type _functor;
		std::queue<work_type> _works;
		std::mutex _workMutex;
		std::condition_variable _conditionVariable;
		thread_container _threads;
		std::atomic_uint64_t _waitingThreads;
	public:
		ThreadPoolCPP(function_type functor, _Args...args) : _config(config), _functor(functor), _waitingThreads(0), ThreadPool() {
			// std::thread cannot be copied, only moved
			for (std::size_t i = 0; i < config.startingThreads; ++i) {
				_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper, *this));
			}
		}

		ThreadPoolCPP(Config config, function_type functor, _Args...args) : _config(config), _functor(functor), _waitingThreads(0), ThreadPool(config) {
			// std::thread cannot be copied, only moved
			for (std::size_t i = 0; i < config.startingThreads; ++i) {
				_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper, *this));
			}
		}

		~ThreadPoolCPP() {
			Stop();
			Wait();
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
			if (_waitingThreads == 0 && _threads.size() < _config.maximumThreads) {
				_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper, *this));
			}
		}

		virtual void Push(_Args...args) {
			Push(std::forward_as_tuple(args...));
		}

		virtual void Push(_Args const&&... args) {
			Push(std::forward_as_tuple(args...));
		}

		template <class _Iter, std::enable_if_t<std::is_same_v<std::iterator_traits<_Iter>::value_type>, work_type>>
		virtual void Push(_Iter begin, _Iter end) {
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

		virtual void FunctionWrapper() {
			while (_run) {
				{
					std::unique_lock<std::mutex> lock(_workMutex);
					++_waitingThreads;
					_conditionVariable.wait(lock);
					--_waitingThreads;
				}

				while (!_works.empty()) {
					work_type work;
					{
						std::unique_lock<std::mutex> lock(_workMutex);
						work = _works.front();
						_works.pop();
					}

					_Execute(work, std::make_index_sequence<std::tuple_size_v<work_type>>());
				}
			}
		}

	protected:
		template <size_t..._indexes>
		inline void _Execute(work_type& work, std::index_sequence<_indexes...> indexSequence) {
			std::invoke(_functor, std::move(std::get<_indexes>(work)));
		}
	};
}