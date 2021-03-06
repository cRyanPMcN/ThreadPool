#pragma once
#include "ThreadPoolBase.hpp"
#include <Windows.h>

namespace Threading {
	template <typename..._ArgsTy>
	class ThreadPoolWin32TpApi : public ThreadPoolBase<_ArgsTy...> {
	public:
		using base_type = ThreadPoolBase<_ArgsTy...>;
		struct Config : base_type::Config {

		};
	protected:
		PTP_POOL _threadpool;
		PTP_CALLBACK_ENVIRON _callback;
		PTP_CLEANUP_GROUP _cleanup;

		ThreadPoolWin32TpApi(Config config) : _threadpool(CreateThreadpool(nullptr)), _cleanup(CreateThreadpoolCleanupGroup()), base_type(config) {
			InitializeThreadpoolEnvironment(_callback);
			SetThreadpoolThreadMinimum(_threadpool, _config.minimumThreads);
			SetThreadpoolThreadMaximum(_threadpool, _config.maximumThreads);
			SetThreadpoolCallbackPool(&_callback, pool);
			SetThreadpoolCallbackCleanupGroup(_callbac, _cleanup, NULL);
		}

	public:

		template <typename _FuncTy>
		ThreadPoolWin32TpApi(_FuncTy functor, Config config = Config()) : ThreadPoolWin32TpApi(config) {
			_threadData = new ThreadData<_FuncTy>(*this, functor);
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				thread_type newThread;
				newThread.handle = CreateThread(NULL, 0, &ThreadPoolWin32TpApi::FunctionWrapper<_FuncTy>, _threadData, NULL, &newThread.id);
				_threads.push_back(newThread);
			}
			Wait();
		}

		template <typename _RetTy>
		ThreadPoolWin32TpApi(_RetTy(*functor)(_ArgsTy...), Config config = Config()) : ThreadPoolWin32TpApi(config) {
			_threadData = new ThreadData<decltype(functor)>(*this, functor);
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				thread_type newThread;
				newThread.handle = CreateThread(NULL, 0, &ThreadPoolWin32TpApi::FunctionWrapper<decltype(functor)>, _threadData, NULL, &newThread.id);
				_threads.push_back(newThread);
			}
			Wait();
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolWin32TpApi(_RetTy(_ObjTy::* functor)(_ArgsTy...), _ObjTy* obj, Config config = Config()) : ThreadPoolWin32TpApi(config) {
			_threadData = new MemberThreadData<decltype(functor), _ObjTy>(*this, functor, obj);
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				thread_type newThread;
				newThread.handle = CreateThread(NULL, 0, &ThreadPoolWinVista::FunctionWrapper<decltype(functor), _ObjTy>, _threadData, NULL, &newThread.id);
				_threads.push_back(newThread);
			}
			Wait();
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolWinVista(_RetTy(_ObjTy::* functor)(_ArgsTy...) const, _ObjTy const* obj, Config config = Config()) : ThreadPoolWinVista(config) {
			_threadData = new MemberThreadData<decltype(functor), _ObjTy const>(*this, functor, obj);
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				thread_type newThread;
				newThread.handle = CreateThread(NULL, 0, &ThreadPoolWinVista::FunctionWrapper<decltype(functor), _ObjTy const>, _threadData, NULL, &newThread.id);
				_threads.push_back(newThread);
			}
			Wait();
		}

		~ThreadPoolWin32TpApi() {
			DestroyThreadpoolEnvironment(_callback);
		}
	};
}