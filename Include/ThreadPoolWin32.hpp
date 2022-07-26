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
				_InterlockedDecrement(&_spinCount);
			}
		};
	}

	class ThreadPoolWin32 : public ThreadPoolBase {
	public:
		using base_type = ThreadPoolBase;
		using Config = typename base_type::Config;
		using thread_type = struct _thrd {
			HANDLE handle;
			DWORD id;
		};
		using thread_container = std::vector<thread_type>;

		using lock_type = DetailWin::SpinLock;
	protected:
		CONDITION_VARIABLE _conditionVariable CONDITION_VARIABLE_INIT;
		DetailWin::CriticalSection _workSection;
		DetailWin::CriticalSection _sleepSection;
		thread_container _threads;
		unsigned long long _waitingThreads;
		// Required, gets cast to the correct type inside thread function
	public:
		ThreadPoolWin32(Config config = Config()) : base_type(config) {
			InitializeConditionVariable(&_conditionVariable);
			for (decltype(base_type::_config.startingThreads) i = 0; i < base_type::_config.startingThreads; ++i) {
				thread_type newThread;
				newThread.handle = CreateThread(NULL, 0, &ThreadPoolWin32::FunctionWrapper, this, NULL, &newThread.id);
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
		}

		using base_type::Push;

		virtual void Push(work_base* work) override {
			lock_type lock(_workSection);
			_works.push(work);
			WakeOne();
		}

		virtual void WakeOne() override {
			WakeConditionVariable(&_conditionVariable);
		}

		virtual void WakeAll() override {
			WakeAllConditionVariable(&_conditionVariable);
		}

		virtual void Wait() override {
			// Wait until all threads are waiting
			while (_waitingThreads < _threads.size() || !(_works.empty() || _pause)) {
				std::this_thread::yield();
			}
			lock_type lock(_sleepSection);
		}

		virtual void Resume() override {
			base_type::Resume();
			Wake(_works.size());
		}

		virtual std::size_t Size() override {
			return _threads.size();
		}

		static DWORD WINAPI FunctionWrapper(LPVOID threadData) {
			ThreadPoolWin32* threadpool = (ThreadPoolWin32*)threadData;
			base_type::container_type& works = threadpool->_works;

			while (threadpool->_run) {
				// Sleep thread
				{
					lock_type lock(threadpool->_sleepSection);
					_InterlockedIncrement(&threadpool->_waitingThreads);
					SleepConditionVariableCS(&threadpool->_conditionVariable, &lock._section._section, INFINITE);
					_InterlockedDecrement(&threadpool->_waitingThreads);
				}

				// Loop work execution
				while (!works.empty() && !threadpool->_pause) {
					// Acquire lock and ensure there is work to be done
					lock_type lock(threadpool->_workSection);
					if (works.empty()) {
						break;
					}
					work_base* work = works.front();
					works.pop();
					lock.Unlock();

					work->Execute();
				}
			}
			
			return EXIT_SUCCESS;
		}
	};
}