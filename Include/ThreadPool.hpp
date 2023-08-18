#pragma once
#include <cstddef>
#include "ThreadPoolBase.hpp"
#include "ThreadPoolCPP.hpp"
#include "ThreadPoolWin32.hpp"
#include "ThreadPoolWin32TpApi.hpp"

namespace Threading {
	template <typename..._ArgsTy>
	class ThreadPool {
#if defined(WIN32)
//#if (_WIN32_WINNT > 0x0600)
//		using threadpool_type = ThreadPoolWin32TpApi<_ArgsTy...>;
//#else
		using threadpool_type = ThreadPoolWin32;
//#endif
#else
		using threadpool_type = ThreadPoolCPP;
#endif

		using Config = typename threadpool_type::Config;
		using work_type = typename threadpool_type::work_type;
		using work_container = typename threadpool_type::work_container;
	private:
		threadpool_type _threadpool;
	public:
		ThreadPool(std::size_t numberThreads) : _threadpool(numberThreads) {

		}

		template <class _FuncTy, class..._ArgsTy>
		void Push(_FuncTy functor, _ArgsTy... work) {
			_threadpool.Push(functor, work...);
			std::function<void> wrapper([functor, work...]() { ThreadPoolBase::_Execute(functor, work...); });
			wrapper();
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

		std::size_t Size(){
			return _threadpool.Size();
		}
	};
}