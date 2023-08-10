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
	public:
	protected:
		template <class _FuncTy, size_t..._indexes>
		static inline void _Execute(_FuncTy functor, work_type& work, std::index_sequence<_indexes...> indexSequence) {
			std::invoke(std::move(functor), std::get<_indexes>(work)...);
		}

		template <typename _FuncTy, class _ObjTy, size_t..._indexes>
		static inline void _Execute(_FuncTy functor, _ObjTy obj, work_type& work, std::index_sequence<_indexes...> indexSequence) {
			std::invoke(std::move(functor), std::move(obj), std::get<_indexes>(work)...);
		}

		template <class _FuncTy>
		static inline void _Execute(_FuncTy functor, work_type& work) {
			_Execute(functor, work, std::make_index_sequence<sizeof...(_ArgsTy)>());
		}

		template <typename _FuncTy, class _ObjTy>
		static inline void _Execute(_FuncTy functor, _ObjTy obj, work_type& work) {
			_Execute(functor, obj, work, std::make_index_sequence<sizeof...(_ArgsTy)>());
		}
	};

}