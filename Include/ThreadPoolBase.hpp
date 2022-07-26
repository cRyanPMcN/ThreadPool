#pragma once
#include <cstddef>
#include <tuple>
#include <queue>

namespace Threading {
	class ThreadPoolBase {
	public:
		struct Config {
			std::size_t minimumThreads;
			std::size_t maximumThreads;
			std::size_t startingThreads;

			Config() : minimumThreads(1), maximumThreads(16), startingThreads(1) {

			}
		};

		struct work_base {
			virtual void Execute() = 0;
		};

		template <class _Func, class..._Args>
		struct work_type : work_base {
			_Func functor;
			std::tuple<_Args...> args;

			work_type(_Func func, std::tuple<_Args...>& arg) : functor(func), args(arg) {

			}

			work_type(_Func func, std::tuple<_Args...>&& arg) : functor(func), args(arg) {

			}

			work_type(_Func func, _Args... args) : work_type(func, std::forward_as_tuple(args...)) {

			}

			template <size_t..._indexes>
			inline void Execute(std::index_sequence<_indexes...> indexSequence) {
				std::invoke(std::move(functor), std::get<_indexes>(args)...);
			}

			void Execute() override {
				Execute(std::make_index_sequence<sizeof...(_Args)>());
			}
		};

		using container_type = std::queue<work_base*>;
	protected:
		Config _config;
		container_type _works;
		bool _run;
		bool _pause;
	public:
		ThreadPoolBase(Config config) : _config(config), _run(true), _pause(false) {
			
		}

		virtual void Push(work_base* work) = 0;
		
		template <class _FuncTy, class..._ArgsTy>
		void Push(work_type<_FuncTy, _ArgsTy...>* work) {
			Push((work_base*)work);
		}

		template <class _FuncTy, class..._ArgsTy>
		void Push(_FuncTy functor, _ArgsTy...args) {
			Push(new work_type<_FuncTy, _ArgsTy...>(functor, args...));
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
		}
		
		virtual std::size_t Size() = 0;
	//protected:
	//	template <class _FuncTy, size_t..._indexes>
	//	static inline void _Execute(_FuncTy functor, work_type& work, std::index_sequence<_indexes...> indexSequence) {
	//		std::invoke(std::move(functor), std::get<_indexes>(work)...);
	//	}
	//
	//	template <typename _FuncTy, class _ObjTy, size_t..._indexes>
	//	static inline void _Execute(_FuncTy functor, _ObjTy obj, work_type& work, std::index_sequence<_indexes...> indexSequence) {
	//		std::invoke(std::move(functor), std::move(obj), std::get<_indexes>(work)...);
	//	}
	};
}