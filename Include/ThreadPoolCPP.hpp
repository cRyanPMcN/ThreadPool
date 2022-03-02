#pragma once

#include "ThreadPool.hpp"
#include <vector>
#include <queue>
#include <thread>
#include <tuple>
#include <mutex>
#include <functional>

namespace Threading {

	template <class _Ty>
	struct FunctionTraits;

	template <class _RetTy, typename..._ArgsTy>
	struct FunctionTraits<std::function<_RetTy(_ArgsTy...)>> {
		static const size_t nargs = sizeof...(_ArgsTy);

		using arg_types = _ArgsTy...;
		using result_type = _RetTy;

		template <size_t i>
		struct arg {
			using type = typename std::tuple_element<i, std::tuple<_ArgsTy...>>::type;
		};
	};

	template <class _RetTy, typename..._ArgsTy>
	struct FunctionTraits<_RetTy(*)(_ArgsTy...)> {
		static const size_t nargs = sizeof...(_ArgsTy);

		using arg_types = _ArgsTy...;
		using result_type = _RetTy;

		template <size_t i>
		struct arg {
			using type = typename std::tuple_element<i, std::tuple<_ArgsTy...>>::type;
		};
	};

	template<typename T>
	struct function_traits;

	template<typename _RetTy, typename ..._ArgsTy>
	struct function_traits<std::function<_RetTy(_ArgsTy...)>> {
		static const size_t nargs = sizeof...(_ArgsTy);

		using arg_types = _ArgsTy...;
		using result_type = _RetTy;

		template <size_t i>
		struct arg {
			using type = typename std::tuple_element<i, std::tuple<_ArgsTy...>>::type;
		};
	};

	template <class _FuncTy, class... _ArgsTy>
	class ThreadPoolCPP : public ThreadPool {
	public:
		using thread_container = std::vector<std::thread>;
		using function_type = _FuncTy;
		using work_type = std::tuple<_ArgsTy...>;
	protected:
		function_type _functor;
		std::queue<work_type> _works;
		std::mutex _workMutex;
		std::condition_variable _conditionVariable;
		thread_container _threads;
		std::thread _watcherThread;
		std::atomic_uint64_t _waitingThreads;
	public:
		ThreadPoolCPP(function_type functor) : ThreadPoolCPP(Config(), functor) {
		}
		
		ThreadPoolCPP(Config config, function_type functor) : _functor(functor), _waitingThreads(0), _watcherThread(&ThreadPoolCPP::ThreadWrapper, this), ThreadPool(config) {

		}

		~ThreadPoolCPP() {
			Stop();
			Wait();
		}

		//virtual void Push(work_type const& work) {
		//	std::unique_lock<std::mutex> lock(_workMutex);
		//	_works.push(work);
		//	_conditionVariable.notify_one();
		//}

		//virtual void Push(work_type const&& work) {
		//	std::unique_lock<std::mutex> lock(_workMutex);
		//	_works.push(work);
		//	_conditionVariable.notify_one();
		//	if (_waitingThreads == 0 && _threads.size() < _config.maximumThreads) {
		//		_threads.push_back(std::thread(&ThreadPoolCPP::FunctionWrapper, this));
		//	}
		//}

		//virtual void Push(_Args...args) {
		//	Push(std::forward_as_tuple(args...));
		//}

		//template <class _Iter>
		//void Push(_Iter begin, _Iter end) {
		//	std::size_t count = 0;
		//	while (begin != end) {
		//		_works.push(*begin);
		//		++begin;
		//		++count;
		//	}

		//	if (count > _waitingThreads) {
		//		_conditionVariable.notify_all();
		//	}
		//	else {
		//		while (count != 0) {
		//			_conditionVariable.notify_one();
		//		}
		//	}
		//}

		virtual void Wait() override {
			//while (_waitingThreads < _threads.size()) {
			//	std::this_thread::yield();
			//}
		}

		virtual void FunctionWrapper() {
			//while (_run) {
			//	{
			//		std::unique_lock<std::mutex> lock(_workMutex);
			//		++_waitingThreads;
			//		_conditionVariable.wait(lock);
			//		--_waitingThreads;
			//	}

			//	while (!_works.empty()) {
			//		std::unique_lock<std::mutex> lock(_workMutex);
			//		if (_works.empty()) {
			//			break;
			//		}
			//		work_type work = _works.front();
			//		_works.pop();
			//		lock.unlock();

			//		_Execute(work, std::make_index_sequence<sizeof...(_Args)>());
			//	}
			//}
		}

		void ThreadWrapper() {
			// TODO: Write code for a main thread that manages creating and destroying threads
			// while _run == true
			//		check number of waiting threads
			//			waiting threads too low
			//				create new thread
			//			waiting threads too high
			//				destroy threads
			// end loop
		}

	protected:
		template <size_t..._indexes>
		inline void _Execute(work_type& work, std::index_sequence<_indexes...> indexSequence) {
			std::invoke(std::move(_functor), std::get<_indexes>(work)...);
		}
	};
}