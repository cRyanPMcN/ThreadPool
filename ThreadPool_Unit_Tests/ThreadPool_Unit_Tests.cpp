#include "pch.h"
#include "CppUnitTest.h"
#include "ThreadPoolCPP.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ThreadPoolUnitTests {
	template <class... _Args>
	using ThreadPool = Threading::ThreadPoolCPP<_Args...>;

	// ConstructorTest functions have no body
	namespace ConstructorTest {
		void Function(int x) {

		}
		struct Object {
			static void Static() {

			}

			void Member() {

			}

			void ConstMember() const {

			}

			static void Overload() {

			}

			static void Overload(int i) {

			}
		};
		struct Callable {
			void operator()() {

			}
		};
	}

	TEST_CLASS(ThreadPoolCPPUnitTests) {
	public:
		// This is a functional test, if it compiles and completes without error then the test succeeds
		TEST_METHOD(ThreadPoolCPP_Constructor_Test) {
			std::pair x(int(1), int(2));
			Threading::ThreadPoolCPP nonMemberStaticThreadPool(ConstructorTest::Function);

			Threading::ThreadPoolCPP memberStaticThreadPool(ConstructorTest::Object::Static);

			ConstructorTest::Object object;
			Threading::ThreadPoolCPP memberThreadPool(&ConstructorTest::Object::Member, &object);

			Threading::ThreadPoolCPP constMemberThreaDPool(&ConstructorTest::Object::ConstMember, &object);

			Threading::ThreadPoolCPP overloadVoidThreadPool((void(*)())(&ConstructorTest::Object::Overload));

			Threading::ThreadPoolCPP overloadIntThreadPool((void(*)(int))(&ConstructorTest::Object::Overload));

			auto func = std::bind(&ConstructorTest::Object::Member, &object);
			Threading::ThreadPoolCPP bindThreadPool(func);

			ConstructorTest::Callable callable;
			Threading::ThreadPoolCPP callableThreadPool(&ConstructorTest::Callable::operator(), &callable);
		}
	};
}
