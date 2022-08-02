#pragma once
#include "ThreadPoolBase.hpp"
#include <Windows.h>

namespace Threading {
	class ThreadPoolWin32TpApi : public ThreadPoolBase {
	public:
		using base_type = ThreadPoolBase;
		base_type::Config;
		struct WorkContext {
			ThreadPoolWin32TpApi& threadpool;
			work_base* work;

			WorkContext(ThreadPoolWin32TpApi& tp, work_base* w) : threadpool(tp), work(w) {

			}
		};
	protected:
		PTP_POOL _threadpool;
		PTP_CALLBACK_ENVIRON _callback;
		PTP_CLEANUP_GROUP _cleanup;
		unsigned long long _workingThreads;
	public:
		ThreadPoolWin32TpApi(Config config = Config()) : _threadpool(CreateThreadpool(nullptr)), _cleanup(CreateThreadpoolCleanupGroup()), _workingThreads(0), base_type(config) {
			InitializeThreadpoolEnvironment(_callback);
			SetThreadpoolThreadMinimum(_threadpool, _config.minimumThreads);
			SetThreadpoolThreadMaximum(_threadpool, _config.maximumThreads);
			SetThreadpoolCallbackPool(_callback, _threadpool);
			SetThreadpoolCallbackCleanupGroup(_callback, _cleanup, NULL);
		}

		~ThreadPoolWin32TpApi() {
			DestroyThreadpoolEnvironment(_callback);
		}

		void Push(work_base* work) {
			PTP_WORK pwork = CreateThreadpoolWork(&ThreadPoolWin32TpApi::Function_Wrapper, new WorkContext(*this, work), _callback);
			SubmitThreadpoolWork(pwork);
			CloseThreadpoolWork(pwork);
		}

		virtual void WakeOne() override {
			// This cannot be done
		}

		virtual void WakeAll() override {
			// This cannot be done
		}

		virtual void Wait() override {
			while (_workingThreads > 0) {
				_Thrd_yield();
				//Sleep(0);
			}
		}

		static void NTAPI Function_Wrapper(PTP_CALLBACK_INSTANCE instance, PVOID data, PTP_WORK pwork) {
			WorkContext* context = (WorkContext*)data;
			_InterlockedIncrement(&context->threadpool._workingThreads);
			context->work->Execute();
			_InterlockedDecrement(&context->threadpool._workingThreads);
		}
	};
}