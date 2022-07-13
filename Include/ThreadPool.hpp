#pragma once
#include <cstddef>
#include "ThreadPoolBase.hpp"
#include "ThreadPoolCPP.hpp"
#include "ThreadPoolWin.hpp"
#include "ThreadPoolWinVista.hpp"

namespace Threading {
	template <typename..._ArgsTy>
	class ThreadPool : 
		public
#if defined(WIN32)
#if (_WIN32_WINNT > 0x0600)
		ThreadPoolWinVista<_ArgsTy...> {
	public:
		using base_type = ThreadPoolWinVista<_ArgsTy...>;
#else
		ThreadPoolWin<_ArgsTy...> {
	public:
		using base_type = ThreadPoolWin<_ArgsTy...>;
#endif
#else
		ThreadPoolCPP<_ArgsTy...> {
	public:
		using base_type = ThreadPoolCPP<_ArgsTy...>;
#endif

		using Config = typename base_type::Config;
		using work_type = typename base_type::work_type;
		using work_container = typename base_type::work_container;
	public:
		template <typename _FuncTy>
		ThreadPool(_FuncTy functor, Config config = Config()) : base_type(functor, config) {

		}

		template <typename _RetTy>
		ThreadPool(_RetTy(*functor)(_ArgsTy...), Config config = Config()) : base_type(functor, config) {

		}

		template <typename _RetTy, class _ObjTy>
		ThreadPool(_RetTy(_ObjTy::*functor)(_ArgsTy...), _ObjTy* obj, Config config = Config()) : base_type(functor, obj, config) {

		}

		template <typename _RetTy, class _ObjTy>
		ThreadPool(_RetTy(_ObjTy::*functor)(_ArgsTy...) const, _ObjTy const* obj, Config config = Config()) : base_type(functor, obj, config) {

		}

		using base_type::Pause;
	};
}