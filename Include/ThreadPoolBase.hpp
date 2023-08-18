#pragma once
#include <cstddef>
#include <tuple>
#include <queue>

namespace Threading {
	struct ThreadPoolBase {
	
		template <class _FuncTy, class..._ArgsTy, size_t..._indexes>
		static inline void _Execute(_FuncTy functor, std::tuple<_ArgsTy...>& work, std::index_sequence<_indexes...> indexSequence) {
			std::invoke(std::move(functor), std::get<_indexes>(work)...);
		}
	
		template <typename _FuncTy, class _ObjTy, class..._ArgsTy, size_t..._indexes>
		static inline void _Execute(_FuncTy functor, _ObjTy obj, std::tuple<_ArgsTy...>& work, std::index_sequence<_indexes...> indexSequence) {
			std::invoke(std::move(functor), std::move(obj), std::get<_indexes>(work)...);
		}
	};
}