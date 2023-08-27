#pragma once

#include <Windows.h>
#include <vector>
#include <set>
#include <queue>
#include <tuple>
#include <functional>

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
				InterlockedIncrement(&_spinCount);
			}

			void Unlock() {
				_section.Leave();
				InterlockedDecrement(&_spinCount);
			}
		};
	}

	class ThreadPoolWin32 {
	public:
		using thread_type = struct _thrd {
			HANDLE handle;
			DWORD id;
		};
		using thread_container = std::vector<thread_type>;

		using lock_type = DetailWin::SpinLock;
		using work_type = std::function<void()>;
		using work_container = std::queue<std::function<void()>>;
	protected:
		bool _run;
		bool _pause;
		DetailWin::CriticalSection _workSection;
		work_container _works;
		DetailWin::CriticalSection _sleepSection;
		CONDITION_VARIABLE _conditionVariable CONDITION_VARIABLE_INIT;
		thread_container _threads;
		unsigned long long _waitingThreads;
	public:
		ThreadPoolWin32(std::size_t numberThreads) : _waitingThreads(0), _run(true), _pause(false) {
			InitializeConditionVariable(&_conditionVariable);
			for (std::size_t i = 0; i < numberThreads; ++i) {
				thread_type newThread;
				newThread.handle = CreateThread(NULL, 0, &ThreadPoolWin32::FunctionWrapper, this, NULL, &newThread.id);
				_threads.push_back(newThread);
			}
			Wait();
		}

		~ThreadPoolWin32() {
			Stop();
			Resume();
			for (thread_type& t : _threads) {
				WaitForSingleObject(t.handle, INFINITE);
			}
		}

		template <class _FuncTy, class..._ArgsTy>
		void Push(_FuncTy functor, _ArgsTy...args) {
			lock_type lock(_workSection);
			_works.emplace([functor, args...](){ Execute(functor, args...); });
			WakeOne();
		}

		void WakeOne() {
			WakeConditionVariable(&_conditionVariable);
		}

		void WakeAll() {
			WakeAllConditionVariable(&_conditionVariable);
		}

		void Stop() {
			_run = false;
		}

		void Resume() {
			_pause = false;
			WakeAll();
		}

		void Pause() {
			_pause = true;
		}

		void Wait() {
			// Wait until all threads are waiting
			while (_waitingThreads < _threads.size() || !(_works.empty() || _pause)) {
				std::this_thread::yield();
			}
			lock_type lock(_sleepSection);
		}

	private:
		static DWORD WINAPI FunctionWrapper(LPVOID threadData) {
			ThreadPoolWin32* threadpool = (ThreadPoolWin32*)threadData;
			work_container& works = threadpool->_works;

			while (threadpool->_run) {
				// Sleep thread
				{
					lock_type lock(threadpool->_sleepSection);
					InterlockedIncrement(&threadpool->_waitingThreads);
					SleepConditionVariableCS(&threadpool->_conditionVariable, &lock._section._section, INFINITE);
					InterlockedIncrement(&threadpool->_waitingThreads);
				}

				// Loop work execution
				while (!works.empty() && !threadpool->_pause) {
					// Acquire lock and ensure there is work to be done
					lock_type lock(threadpool->_workSection);
					if (works.empty()) {
						break;
					}
					work_type work(works.front());
					works.pop();
					lock.Unlock();

					work();
				}
			}
			
			return EXIT_SUCCESS;
		}

		template <class _FuncTy, class..._ArgsTy>
		static inline void Execute(_FuncTy functor, _ArgsTy...args) {
			std::invoke(functor, args...);
		}
	};
}