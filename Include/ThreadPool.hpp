#pragma once
#include <cstddef>
#include "ThreadPoolCPP.hpp"
#include "ThreadPoolWin32.hpp"
#include "ThreadPoolWin32TpApi.hpp"

namespace Threading {
	template <typename..._ArgsTy>
	class ThreadPool {
#if defined(WIN32)
#if (_WIN32_WINNT > 0x0600)
		using threadpool_type = ThreadPoolWin32TpApi;
#else
		using threadpool_type = ThreadPoolWin32;
#endif
#else
		using threadpool_type = ThreadPoolCPP;
#endif
	private:
		threadpool_type _threadpool;
	public:
		ThreadPool(std::size_t numberThreads) : _threadpool(numberThreads) {

		}

		template <class _FuncTy, class..._ArgsTy>
		void Push(_FuncTy functor, _ArgsTy... work) {
			_threadpool.Push(functor, work...);
		}

		void Wait() {
			_threadpool.Wait();
		}

		void Stop() {
			_threadpool.Stop();
		}

		void Pause() {
			_threadpool.Pause();
		}

		void Resume() {
			_threadpool.Resume();
		}
	};
}