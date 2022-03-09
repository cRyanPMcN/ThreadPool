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

	namespace ExecutionTest {
		void Function(int& x) {
			++x;
		}

		void OverloadFunction(int& x) {
			++x;
		}

		void OverloadFunction(int& x, int changeValue) {
			x += changeValue;
		}

		struct Object {
			int store;

			static void Static(int& x) {
				++x;
			}

			void Member(int x) {
				store = x;
			}

			void ConstMember(int& x) const {
				x = store;
			}

			static void OverLoadStatic(int& x) {
				++x;
			}

			static void OverLoadStatic(int& x, int changeValue) {
				x += changeValue;
			}

			void OverLoadMember(int x) {
				store = x;
			}

			void OverLoadMember(int& x) {
				x = store;
			}
		};
		struct Callable {
			int store = 0;
			
			void operator()() {
				++store;
			}

			void operator()(int x) {
				store = x;
			}

			void operator()(int& x) {
				x = store;
			}

		};
	}

	TEST_CLASS(ThreadPoolCPPUnitTests) {
	public:
		// This test checks if each ThreadPool with construct when scoped alone. This allows easier debugging for which test causes a compiler error
		TEST_METHOD(ThreadPoolCPP_Constructor_Incongruous) {
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
	
		// This test checks if all threadpools with successfully construct when scoped congruously
		TEST_METHOD(ThreadPoolCPP_Constructor_Congruous) {
				Threading::ThreadPoolCPP nonMemberStaticThreadPool(ConstructorTest::Function);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Non-member static function threadpool construction completed.\n");

				Threading::ThreadPoolCPP memberStaticThreadPool(&ConstructorTest::Object::Static);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Member static function threadpool construction completed.\n");

				ConstructorTest::Object object;
				Threading::ThreadPoolCPP memberThreadPool(&ConstructorTest::Object::Member, &object);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Member function threadpool construction completed.\n");

				Threading::ThreadPoolCPP constMemberThreadPool(&ConstructorTest::Object::ConstMember, &object);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Const member function threadpool construction completed.\n");

				Threading::ThreadPoolCPP staticOverloadVoidThreadPool((void(*)())(ConstructorTest::OverloadFunction));
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Overloaded static function threadpool construction completed.\n");

				Threading::ThreadPoolCPP staticOverloadIntThreadPool((void(*)(int))(ConstructorTest::OverloadFunction));
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Overloaded static function threadpool construction completed.\n");

				Threading::ThreadPoolCPP staticMemberOverloadVoidThreadPool((void(*)())(ConstructorTest::Object::OverLoadStatic));
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Overloaded member static function threadpool construction completed.\n");

				Threading::ThreadPoolCPP staticMemberOverloadIntThreadPool((void(*)(int))(ConstructorTest::Object::OverLoadStatic));
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Overloaded member static function threadpool construction completed.\n");

				Threading::ThreadPoolCPP memberOverloadVoidThreadPool((void(ConstructorTest::Object::*)())(&ConstructorTest::Object::OverLoadMember), &object);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Overloaded member static function threadpool construction completed.\n");

				Threading::ThreadPoolCPP memberOverloadIntThreadPool((void(ConstructorTest::Object::*)(int))(&ConstructorTest::Object::OverLoadMember), &object);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Overloaded member static function threadpool construction completed.\n");

				auto func = std::bind(&ConstructorTest::Object::Member, &object);
				Threading::ThreadPoolCPP bindThreadPool(func);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: std::bind function object threadpool construction completed.\n");

				ConstructorTest::Callable callable;
				Threading::ThreadPoolCPP callableThreadPool(callable);
				Logger::WriteMessage("ThreadPoolCPP->Constructor: Callable object threadpool construction completed.\n");

			Assert::AreEqual(0, 0);
		}

		TEST_METHOD(ThreadPoolCPP_Execution_Single) {

		}
	};
}
