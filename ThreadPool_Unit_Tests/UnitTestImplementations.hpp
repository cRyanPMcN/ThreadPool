#pragma once
#include <atomic>
#include <mutex>

// ConstructorTest functions have no body
namespace ConstructorTest {
	void Function();

	void OverloadFunction();

	void OverloadFunction(long x);

	struct Object {
		static void Static() {

		}

		void Member() {

		}

		void ConstMember() const {

		}

		static void OverLoadStatic() {

		}

		static void OverLoadStatic(long i) {

		}

		void OverLoadMember() {

		}

		void OverLoadMember(long x) {

		}
	};
	struct Callable {
		void operator()() {

		}
	};
}

namespace ExecutionTest {
	void Function(long& x);

	void OverloadFunction(long& x);

	void OverloadFunction(long& x, long changeValue);

	struct Object {
		long store = 0;

		static void Static(long& x) {
			_InterlockedIncrement(&x);
		}

		void Member() {
			_InterlockedIncrement(&store);
		}

		void Member(long x) {
			_InterlockedExchange(&store, x);
		}

		void ConstMember(long& x) const {
			_InterlockedExchange(&x, store);
		}

		static void OverLoadStatic(long& x) {
			_InterlockedIncrement(&x);
		}

		static void OverLoadStatic(long& x, long changeValue) {
			_InterlockedExchangeAdd(&x, changeValue);
		}
	};
	struct Callable {
		long store = 0;

		void operator()() {
			_InterlockedIncrement(&store);
		}

		void operator()(long x) {
			_InterlockedExchange(&store, x);
		}

		void operator()(long* x) {
			_InterlockedExchange(x, store);
		}
	};
}