#include "pch.h"
#include "CppUnitTest.h"
#include "ThreadPoolCPP.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ThreadPoolUnitTests {
	// ConstructorTest functions have no body
	namespace ConstructorTest {
		void Function() {

		}

		void OverloadFunction() {

		}

		void OverloadFunction(int x) {

		}

		struct Object {
			static void Static() {

			}

			void Member() {

			}

			void ConstMember() const {

			}

			static void OverLoadStatic() {

			}

			static void OverLoadStatic(int i) {

			}

			void OverLoadMember() {

			}

			void OverLoadMember(int x) {

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
			{
				Threading::ThreadPoolCPP nonMemberStaticThreadPool(ConstructorTest::Function);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Non-member static function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolCPP memberStaticThreadPool(&ConstructorTest::Object::Static);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Member static function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolCPP memberThreadPool(&ConstructorTest::Object::Member, &object);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Member function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolCPP constMemberThreadPool(&ConstructorTest::Object::ConstMember, &object);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Const member function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolCPP overloadVoidThreadPool((void(*)())(ConstructorTest::OverloadFunction));
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Overloaded static function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolCPP overloadIntThreadPool((void(*)(int))(ConstructorTest::OverloadFunction));
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Overloaded static function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolCPP overloadVoidThreadPool((void(*)())(ConstructorTest::Object::OverLoadStatic));
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Overloaded member static function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolCPP overloadIntThreadPool((void(*)(int))(ConstructorTest::Object::OverLoadStatic));
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Overloaded member static function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolCPP overloadVoidThreadPool((void(ConstructorTest::Object::*)())(&ConstructorTest::Object::OverLoadMember), &object);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Overloaded member static function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolCPP overloadIntThreadPool((void(ConstructorTest::Object::*)(int))(&ConstructorTest::Object::OverLoadMember), &object);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Overloaded member static function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				auto func = std::bind(&ConstructorTest::Object::Member, &object);
				Threading::ThreadPoolCPP bindThreadPool(func);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: std::bind function object threadpool construction completed.\n");
			}

			{
				ConstructorTest::Callable callable;
				Threading::ThreadPoolCPP callableThreadPool(callable);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Callable object threadpool construction completed.\n");
			}

			Assert::AreEqual(0, 0);
		}
	};
}
