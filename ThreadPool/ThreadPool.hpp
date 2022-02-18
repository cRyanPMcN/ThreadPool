#pragma once

namespace Threading {
	class ThreadPool {
	public:

	protected:
		bool _run;
	public:
		ThreadPool() : _run(true) {

		}

		~ThreadPool() {
			_run = false;
		}

		void Stop() {
			_run = false;
		}

		virtual void Wait() = 0;
	};

	class ThreadPoolWin {

	};

	class ThreadPoolWinVista {

	};
}