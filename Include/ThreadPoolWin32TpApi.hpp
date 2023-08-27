#pragma once
#include <Windows.h>
#include <functional>

namespace Threading {
	class ThreadPoolWin32TpApi {
	public:
		struct WorkContext {
			ThreadPoolWin32TpApi& threadpool;
			 std::function<void()> work;

			WorkContext(ThreadPoolWin32TpApi& tp, std::function<void()> w) : threadpool(tp), work(w) {

			}
		};
		struct HelperWorkingCounter {
			unsigned long long& counter;
			HelperWorkingCounter(unsigned long long& c) : counter(c) {
				InterlockedIncrement(&counter);
			}

			~HelperWorkingCounter() {
				InterlockedDecrement(&counter);
			}
		};
	protected:
		bool _pause;
		bool _stop;
		std::queue<std::function<void()>> _bufferWork;
		PTP_POOL _threadpool;
		unsigned long long _workingThreads;
	public:
		ThreadPoolWin32TpApi(std::size_t numberThreads) : _threadpool(CreateThreadpool(nullptr)), _workingThreads(0) {
			SetThreadpoolThreadMinimum(_threadpool, 1);
			SetThreadpoolThreadMaximum(_threadpool, numberThreads);
		}

		~ThreadPoolWin32TpApi() {
			CloseThreadpool(_threadpool);
		}

		template <class _FuncTy, class..._ArgsTy>
		void Push(_FuncTy functor, _ArgsTy...args) {
			if (stop) {
				return;
			}
			if (!_pause) {
				std::function<void()> work([functor, args...]() { Execute(functor, args...); });
				PTP_WORK pwork = CreateThreadpoolWork(&ThreadPoolWin32TpApi::Function_Wrapper, new WorkContext(*this, work), NULL);
				SubmitThreadpoolWork(pwork);
				CloseThreadpoolWork(pwork);
			} else {
				_bufferWork.emplace([functor, args...]() { Execute(functor, args...); });
			}
		}

		void Wait() {
			while (_workingThreads > 0) {
				Sleep(0);
			}
		}

		void Stop() {
			_stop = true;
		}

		void Pause() {
			_pause = true;
		}

		void Resume() {
			_pause = false;
			while (!_bufferWork.empty()) {
				std::function<void()> work = _bufferWork.front();
				_bufferWork.pop();
				PTP_WORK pwork = CreateThreadpoolWork(&ThreadPoolWin32TpApi::Function_Wrapper, new WorkContext(*this, work), NULL);
				SubmitThreadpoolWork(pwork);
				CloseThreadpoolWork(pwork);
			}
		}
	private:
		static void NTAPI Function_Wrapper(PTP_CALLBACK_INSTANCE instance, PVOID data, PTP_WORK pwork) {
			WorkContext* context = (WorkContext*)data;
			HelperWorkingCounter counter(context->threadpool._workingThreads);
			context->work();
			delete context;
		}

		template <class _FuncTy, class..._ArgsTy>
		static inline void Execute(_FuncTy functor, _ArgsTy...args) {
			std::invoke(functor, args...);
		}
	};
}