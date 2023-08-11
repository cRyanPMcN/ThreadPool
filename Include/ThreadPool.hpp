#pragma once
#include "ThreadPoolBase.hpp"
#include "ThreadPoolCPP.hpp"
#include "ThreadPoolWin32.hpp"
#include "ThreadPoolWin32TpApi.hpp"
//#include <cstddef>

namespace Threading {
	//template <template<typename...> typename _poolTy>
	template <typename..._ArgsTy>
	class ThreadPool {
	public:
#if defined(WIN32)
//#if (_WIN32_WINNT > 0x0600)
//		using threadpool_type = ThreadPoolWin32TpApi<_ArgsTy...>;
//#else
		using threadpool_type = ThreadPoolWin32<_ArgsTy...>;
//	#endif
#else
		using threadpool_type = ThreadPoolCPP<_ArgsTy...>;
#endif

		struct Config {
			std::size_t minimumThreads;
			std::size_t maximumThreads;
			std::size_t startingThreads;

			Config() : minimumThreads(1), maximumThreads(16), startingThreads(1) {

			}
		};
		using work_type = std::tuple<_ArgsTy...>;
		using work_container = std::vector<work_type>;
	private:
		threadpool_type _threadpool;
	public:
		template <typename _FuncTy>
		ThreadPool(_FuncTy functor, Config config = Config()) : _threadpool(functor, config) {

		}

		template <typename _RetTy>
		ThreadPool(_RetTy(*functor)(_ArgsTy...), Config config = Config()) : _threadpool(functor, config) {

		}

		template <typename _RetTy, class _ObjTy>
		ThreadPool(_RetTy(_ObjTy::*functor)(_ArgsTy...), _ObjTy* obj, Config config = Config()) : _threadpool(functor, obj, config) {

		}

		template <typename _RetTy, class _ObjTy>
		ThreadPool(_RetTy(_ObjTy::*functor)(_ArgsTy...) const, _ObjTy const* obj, Config config = Config()) : _threadpool(functor, obj, config) {

		}

		void Push(work_type const& work) {
			_threadpool.Push(work);
		}

		void Push(work_type const&& work) {
			_threadpool.Push(work);
		}

		void Push(_ArgsTy...args) {
			Push(std::forward_as_tuple(args...));
		}

		template <class _Iter>
		void Push(_Iter begin, _Iter end) {
			std::unique_lock<std::mutex> lock(_watcherMutex);
			while (begin != end) {
				_buffer.push(*begin)
			}
		}

		void WakeOne() {
			_threadpool.WakeOne();
		}

		void Wake(std::size_t number) {
			_threadpool.Wake(number); 
		}

		void WakeAll() {
			_threadpool.WakeAll();
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

		std::size_t Size() {
			return _threadpool.Size();
		}
	};
}