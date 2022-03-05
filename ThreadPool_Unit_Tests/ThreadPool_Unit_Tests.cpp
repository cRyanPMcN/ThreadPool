#include "pch.h"
#include "CppUnitTest.h"
#include "ThreadPoolCPP.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ThreadPoolUnitTests {
	template <class _FuncTy, class... _Args>
	using ThreadPool = Threading::ThreadPoolCPP<_FuncTy, _Args...>;

	// ConstructorTest functions have no body
	namespace ConstructorTest {
		void Function() {

		}
		struct Object {
			static void Static() {

			}

			void Member() {

			}

			void Overload() {

			}

			void Overload(int i) {

			}
		};
		struct Callable {
			void operator()() {

			}
		};
	}

	namespace SingleExecutionTest {
		void Function(int& value) {
			++value;
		}
		struct Object {
			int store;
			
			static void Static(int& value) {
				++value;
			}

			void Member(int& value) {
				++value;
			}

			void Member(int set) {
				store = set;
				++set;
			}
		};
		struct Callable {
			int store;

			void operator()(int& value) {
				++value;
			}
			
			void operator()(int set) {
				store = set;
				++set;
			}
		};
	}

	template <class _FnTy, class..._ArgTy>
	static void FunctionTest(void(_FnTy::*)(_ArgTy...)) {

	}

	TEST_CLASS(ThreadPoolCPPUnitTests) {
	public:
		// This is a functional test, if it compiles and completes without error then the test succeeds
		TEST_METHOD(ThreadPoolCPP_Constructor_Test) {
			ThreadPool<void(*)()> nonMemberStaticThreadPool(ConstructorTest::Function);

			ThreadPool<void(*)()> memberStaticThreadPool(ConstructorTest::Object::Static);

			ThreadPool<void(ConstructorTest::Object::*)(), ConstructorTest::Object*> memberThreadPool(&ConstructorTest::Object::Member);

			ThreadPool<void(ConstructorTest::Object::*)()> overloadVoidThreadPool(&ConstructorTest::Object::Overload);

			ThreadPool<void(ConstructorTest::Object::*)(int), int> overloadIntThreadPool(&ConstructorTest::Object::Overload);

			ConstructorTest::Object object;
			auto func = std::bind(&ConstructorTest::Object::Member, &object);
			ThreadPool<decltype(func)> bindThreadPool(func);

			ConstructorTest::Callable callable;
			ThreadPool<ConstructorTest::Callable> callableThreadPool(callable);
		}
	};
}
