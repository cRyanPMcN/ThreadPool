#pragma once
#include <cstddef>
#include "ThreadPoolImpl.hpp"
#include "ThreadPoolCPP.hpp"
#include "ThreadPoolWin.hpp"
#include "ThreadPoolWinVista.hpp"

namespace Threading {
//#define THREADING_THREADPOOL_CUSTOM_DEFAULT ThreadPoolCPP
#define THREADING_THREADPOOL_USE_STD
	template <typename..._ArgsTy>
	class ThreadPool : 
		public
#if defined(WIN32)
#if _WIN32_WINNT > 0x0600
		ThreadPoolWinVista<_ArgsTy...> 
#else
		ThreadPoolWin<_ArgsTy...>
#endif
#else
		ThreadPoolCPP<_ArgsTy...>
#endif
						{
	public:
		using base_type =
#if defined(WIN32)
#if WINVER > 0x0600
			ThreadPoolWinVista<_ArgsTy...>;
#else
			ThreadPoolWin<_ArgsTy...>;
#endif
#else
			ThreadPoolCPP<_ArgsTy...>;
#endif

		using Config = typename base_type::Config;
		using work_type = typename base_type::work_type;
		using work_container = typename base_type::work_container;
	public:
		using base_type::my_base;
	};

	class ThreadPoolWinVista {

	};
}