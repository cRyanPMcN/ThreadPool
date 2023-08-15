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

		template <class _FuncTy, typename..._ArgsTy, size_t..._indexes>
		static inline void _Execute(_FuncTy functor, std::tuple<_ArgsTy...>& work, std::index_sequence<_indexes...> indexSequence) {
			std::invoke(std::move(functor), std::get<_indexes>(work)...);
		}

		template <typename _FuncTy, class _ObjTy, typename..._ArgsTy, size_t..._indexes>
		static inline void _Execute(_FuncTy functor, _ObjTy obj, std::tuple<_ArgsTy...>& work, std::index_sequence<_indexes...> indexSequence) {
			std::invoke(std::move(functor), std::move(obj), std::get<_indexes>(work)...);
		}

		template <class _FuncTy, typename..._ArgsTy>
		static inline void _Execute(_FuncTy functor, std::tuple<_ArgsTy...>& work) {
			_Execute(functor, work, std::make_index_sequence<sizeof...(_ArgsTy)>());
		}

		template <typename _FuncTy, class _ObjTy, typename..._ArgsTy>
		static inline void _Execute(_FuncTy functor, _ObjTy obj, std::tuple<_ArgsTy...>& work) {
			_Execute(functor, obj, work, std::make_index_sequence<sizeof...(_ArgsTy)>());
		}
	};

}