#pragma once
#include <cstddef>
#include <tuple>
#include <queue>

namespace Threading {
	template <typename..._ArgsTy>
	class ThreadPoolBase {
	public:
		struct Config {
			std::size_t minimumThreads;
			std::size_t maximumThreads;
			std::size_t startingThreads;

			Config() : minimumThreads(1), maximumThreads(16), startingThreads(1) {

			}
		};
		using work_type = std::tuple<_ArgsTy...>;
		using work_container = std::queue<work_type>;
	protected:
		Config _config;
		work_container _works;
		bool _run;
		bool _pause;
	public:
		ThreadPoolBase(Config config) : _config(config), _run(true), _pause(false) {
			
		}

		virtual void Push(work_type const& work) = 0;

		virtual void Push(work_type const&& work) = 0;

		virtual void Push(_ArgsTy...args) {
			Push(std::forward_as_tuple(args...));
		}

		template <class _Iter>
		void Push(_Iter begin, _Iter end) {
			while (begin != end) {
				Push(*begin);
				++begin;
			}
		}

		virtual void WakeOne() = 0;

		virtual void Wake(std::size_t number) {
			if (number > Size()) {
				WakeAll();
			}
			else {
				while (number) {
					WakeOne();
					--number;
				}
			}
		}
		
		virtual void WakeAll() = 0;

		virtual void Wait() = 0;

		virtual void Stop() {
			_run = false;
			WakeAll();
		}

		virtual void Pause() {
			_pause = true;
		}

		virtual void Resume() {
			Wait();
			_pause = false;
			Wake(Size());
		}
		
		virtual std::size_t Size() = 0;
	protected:
		template <class _FuncTy, size_t..._indexes>
		static inline void _Execute(_FuncTy functor, work_type& work, std::index_sequence<_indexes...> indexSequence) {
			std::invoke(std::move(functor), std::get<_indexes>(work)...);
		}

		template <typename _FuncTy, class _ObjTy, size_t..._indexes>
		static inline void _Execute(_FuncTy functor, _ObjTy obj, work_type& work, std::index_sequence<_indexes...> indexSequence) {
			std::invoke(std::move(functor), std::move(obj), std::get<_indexes>(work)...);
		}
	};

}