#pragma once

#include "ThreadPoolBase.hpp"
#include <Windows.h>
#include <vector>
#include <set>
#include <queue>
#include <tuple>

namespace Threading {
	namespace DetailWin {
		struct CriticalSection {
			CRITICAL_SECTION _section;

			CriticalSection() {
				InitializeCriticalSection(&_section);
			}

			~CriticalSection() {
				DeleteCriticalSection(&_section);
			}

			void Enter() {
				EnterCriticalSection(&_section);
			}

			void Leave() {
				LeaveCriticalSection(&_section);
			}
		};

		struct SpinLock {
			CriticalSection& _section;
			unsigned long long _spinCount;

			SpinLock(CriticalSection& section) : _section(section), _spinCount(0) {
				Lock();
			}

			~SpinLock() {
				while (_spinCount) {
					Unlock();
				}
			}

			void Lock() {
				_section.Enter();
				_InterlockedIncrement(&_spinCount);
			}

			void Unlock() {
				_section.Leave();
				if (_spinCount > 0) {
					_InterlockedDecrement(&_spinCount);
				}
			}
		};
	}


	template <class... _ArgsTy>
	class ThreadPoolWin32 : public ThreadPoolBase<_ArgsTy...> {
	public:
		using base_type = ThreadPoolBase<_ArgsTy...>;
		using Config = typename base_type::Config;
		using thread_type = struct _thrd {
			HANDLE handle;
			DWORD id;
		};
		using thread_container = std::vector<thread_type>;

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

		using lock_type = DetailWin::SpinLock;

		using work_type = typename base_type::work_type;
		using work_container = typename base_type::work_container;
	protected:
		Config _config;
		bool _run;
		bool _pause;
		work_container _works;
		CONDITION_VARIABLE _conditionVariable CONDITION_VARIABLE_INIT;
		DetailWin::CriticalSection _workSection;
		DetailWin::CriticalSection _sleepSection;
		thread_container _threads;
		unsigned long long _waitingThreads;
		// Win32 Threading api takes a void pointer
		void* _threadData;
		ThreadPoolWin32(Config config) : _waitingThreads(0), base_type(config) {
			InitializeConditionVariable(&_conditionVariable);
			InitializeConditionVariable(&_conditionVariable);
		}
	public:
		template <typename _FuncTy>
		ThreadPoolWin32(_FuncTy functor, Config config = Config()) : ThreadPoolWin32(config) {
			_threadData = new ThreadData<_FuncTy>(*this, functor);
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				thread_type newThread;
				newThread.handle = CreateThread(NULL, 0, &ThreadPoolWin32::FunctionWrapper<_FuncTy>, _threadData, NULL, &newThread.id);
				_threads.push_back(newThread);
			}
			Wait();
		}

		template <typename _RetTy>
		ThreadPoolWin32(_RetTy(*functor)(_ArgsTy...), Config config = Config()) : ThreadPoolWin32(config) {
			_threadData = new ThreadData<decltype(functor)>(*this, functor);
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				thread_type newThread;
				newThread.handle = CreateThread(NULL, 0, &ThreadPoolWin32::FunctionWrapper<decltype(functor)>, _threadData, NULL, &newThread.id);
				_threads.push_back(newThread);
			}
			Wait();
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolWin32(_RetTy(_ObjTy::* functor)(_ArgsTy...), _ObjTy* obj, Config config = Config()) : ThreadPoolWin32(config) {
			_threadData = new MemberThreadData<decltype(functor), _ObjTy>(*this, functor, obj);
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				thread_type newThread;
				newThread.handle = CreateThread(NULL, 0, &ThreadPoolWin32::FunctionWrapper<decltype(functor), _ObjTy>, _threadData, NULL, &newThread.id);
				_threads.push_back(newThread);
			}
			Wait();
		}

		template <typename _RetTy, class _ObjTy>
		ThreadPoolWin32(_RetTy(_ObjTy::* functor)(_ArgsTy...) const, _ObjTy const* obj, Config config = Config()) : ThreadPoolWin32(config) {
			_threadData = new MemberThreadData<decltype(functor), _ObjTy const>(*this, functor, obj);
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				thread_type newThread;
				newThread.handle = CreateThread(NULL, 0, &ThreadPoolWin32::FunctionWrapper<decltype(functor), _ObjTy const>, _threadData, NULL, &newThread.id);
				_threads.push_back(newThread);
			}
			Wait();
		}

		~ThreadPoolWin32() {
			Resume();
			Wait();
			Stop();
			for (thread_type& t : _threads) {
				WaitForSingleObject(t.handle, INFINITE);
			}
			delete _threadData;
		}

		using base_type::Push;

		void Push(work_type const& work) {
			lock_type lock(_workSection);
			_works.push(work);
			WakeOne();
		}

		void Push(work_type const&& work) {
			lock_type lock(_workSection);
			_works.push(work);
			WakeOne();
		}

		void WakeOne() {
			WakeConditionVariable(&_conditionVariable);
		}

		void WakeAll() {
			WakeAllConditionVariable(&_conditionVariable);
		}

		void Wait() {
			// Wait until all threads are waiting
			while (_waitingThreads < _threads.size() || !(_works.empty() || _pause)) {
				sleep(0);
			}
			lock_type lock(_sleepSection);
		}

		std::size_t Size() {
			return _threads.size();
		}

		template <typename _FuncTy>
		static DWORD WINAPI FunctionWrapper(LPVOID threadData) {
			ThreadData<_FuncTy>* data = (ThreadData<_FuncTy>*)threadData;
			ThreadPoolWin32& threadpool = data->threadpool;
			work_container& works = threadpool._works;
			_FuncTy functor = data->functor;

			while (threadpool._run) {
				// Sleep thread
				{
					lock_type lock(threadpool._sleepSection);
					_InterlockedIncrement(&threadpool._waitingThreads);
					SleepConditionVariableCS(&threadpool._conditionVariable, &lock._section._section, INFINITE);
					_InterlockedDecrement(&threadpool._waitingThreads);
				}

				// Loop work execution
				while (!works.empty() && !threadpool._pause) {
					// Acquire lock and ensure there is work to be done
					lock_type lock(threadpool._workSection);
					if (works.empty()) {
						break;
					}
					work_type work = works.front();
					works.pop();
					lock.Unlock();

					_Execute(functor, work);
				}
			}
			
			return EXIT_SUCCESS;
		}

		template <typename _FuncTy, class _ObjTy>
		static DWORD WINAPI FunctionWrapper(LPVOID threadData) {
			MemberThreadData<_FuncTy, _ObjTy>* data = (MemberThreadData<_FuncTy, _ObjTy>*)threadData;
			ThreadPoolWin32& threadpool = data->threadpool;
			work_container& works = threadpool._works;
			_ObjTy* obj = data->object;
			_FuncTy functor = data->functor;

			while (threadpool._run) {
				// Sleep thread
				{
					lock_type lock(threadpool._sleepSection);
					_InterlockedIncrement(&threadpool._waitingThreads);
					SleepConditionVariableCS(&threadpool._conditionVariable, &lock._section._section, INFINITE);
					_InterlockedDecrement(&threadpool._waitingThreads);
				}

				// Loop work execution
				while (!works.empty() && !threadpool._pause) {
					// Acquire lock and ensure there is work to be done
					lock_type lock(threadpool._workSection);
					if (works.empty()) {
						break;
					}
					work_type work = works.front();
					works.pop();
					lock.Unlock();

					_Execute(functor, obj, work);
				}
			}

			return EXIT_SUCCESS;
		}
	};
}