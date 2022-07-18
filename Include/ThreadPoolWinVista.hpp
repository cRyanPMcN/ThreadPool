#pragma once
#include "ThreadPoolImpl.hpp"
#include <Windows.h>

namespace Threading {
	template <typename..._ArgsTy>
	class ThreadPoolWinVistaVista : public ThreadPoolImpl<_ArgsTy...> {
	public:
		using base_type = ThreadPoolBase<_ArgsTy...>;

		using work_type = typename base_type::work_type;
		using Config = typename base_type::Config;


		template <typename _FuncTy>
		struct ThreadData {
			ThreadPoolWin32& threadpool;
			_FuncTy functor;

			ThreadData(ThreadPoolWin32& pool, _FuncTy func) : threadpool(pool), functor(func) {

			}
		};

		template <typename _FuncTy, class _ObjTy>
		struct MemberThreadData : ThreadData<_FuncTy> {
			_ObjTy* object;

			MemberThreadData(ThreadPoolWin32& pool, _FuncTy func, _ObjTy* obj) : object(obj), ThreadData<_FuncTy>(pool, func) {

			}
		};
	protected:
		PTP_POOL _threadpool;
		PTP_CALLBACK_ENVIRON _callback;
		PTP_CLEANUP_GROUP _cleanup;

		ThreadPoolWinVistaVista(Config config) : _threadpool(CreateThreadpool(nullptr)), _cleanup(CreateThreadpoolCleanupGroup()), base_type(config) {
			InitializeThreadpoolEnvironment(_callback);
			SetThreadpoolThreadMinimum(_threadpool, _config.minimumThreads);
			SetThreadpoolThreadMaximum(_threadpool, _config.maximumThreads);
			SetThreadpoolCallbackPool(&_callback, pool);
			SetThreadpoolCallbackCleanupGroup(_callbac, _cleanup, NULL);
		}

		PTP_WORK_CALLBACK _workCallback;
		void* _threadData;
	public:
		template <typename _FuncTy>
		ThreadPoolWin32TpApi(_FuncTy functor, Config config = Config()) : ThreadPoolWin32TpApi(config) {
			_workCallback = WorkCallback<_FuncTy>;
		}

		template <typename _RetTy>
		ThreadPoolWin32TpApi(_RetTy(*functor)(_ArgsTy...), Config config = Config()) : ThreadPoolWin32TpApi(config) {

		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolWin32TpApi(_RetTy(_ObjTy::* functor)(_ArgsTy...), _ObjTy* obj, Config config = Config()) : ThreadPoolWin32TpApi(config) {

		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolWin32TpApi(_RetTy(_ObjTy::* functor)(_ArgsTy...) const, _ObjTy const* obj, Config config = Config()) : ThreadPoolWin32TpApi(config) {

		}

		~ThreadPoolWin32TpApi() {
			PTP_WORK wait = CreateThreadpoolWait(NULL, NULL, NULL);
			WaitForThreadpoolWorkCallbacks(wait, false);

			//CloseThreadpoolCleanupGroupMembers();

			DestroyThreadpoolEnvironment(_callback);
			delete _threadData;
		}

		using base_type::Push;
		

		virtual void Push(work_type const& work) override {
			std::pair<work_type, void*>* pWork = new std::pair<work_type, void*>(work, _threadData);
			PTP_WORK tpWork = CreateThreadpoolWork(_workCallback, pwork, _callback);
			SubmitThreadpoolWork(tpWork);
		}

		virtual void Push(work_type const&& work) override {

		}

		template <typename _FuncTy>
		static void WorkCallback(_Inout_ PTP_CALLBACK_INSTANCE instance, _Inout_opt_ PVOID context, _Inout_ PTP_WORK work) {
			_FuncTy functor = (_FuncTy)(context);
			
			base_type::_Execute(functor, work, std::make_index_sequence<sizeof...(_ArgsTy)>());
			
			CloseThreadpoolWork(work);
		}

		template <typename _FuncTy, class _ObjTy>
		static void FunctionWrapper(_FuncTy functor, _ObjTy* obj) {
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