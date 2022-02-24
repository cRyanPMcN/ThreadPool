#pragma once

namespace Threading {
	class ThreadPool {
	public:
		struct Config {
			std::size_t minimumThreads;
			std::size_t maximumThreads;
			std::size_t startingThreads;

			Config() : minimumThreads(1), maximumThreads(16), startingThreads(1) {

			}
		};
	protected:
		const Config _config;
		bool _run;
	public:
		ThreadPool(Config config = Config()) : _run(true), _config(config) {

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