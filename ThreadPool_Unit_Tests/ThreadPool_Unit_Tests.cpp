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
			int store = 0;

			static void Static(int& x) {
				++x;
			}

			void Member() {
				++store;
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
		};
		struct Callable {
			using void_t = void*;
			int store = 0;
			
			void operator()() {
				++store;
			}
			
			void operator()(int x) {
				store = x;
			}

			void operator()(int* x) {
				*x = store;
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

#define ASSERT_EXPECTED_VALUE Assert::AreEqual(expectedValue, testValue)
#define ASSERT_EXPECTED_STORE(expected, test) Assert::AreEqual(expected.store, test.store)
		 
		TEST_METHOD(ThreadPoolCPP_Execution_Single) {
			int expectedValue = 0;
			int testValue = 0;
			int incrementValue = 5;
			// Sanity check
			ASSERT_EXPECTED_VALUE;

			{
				Threading::ThreadPoolCPP threadpool(ExecutionTest::Function);
				threadpool.Push(std::ref(testValue));
				ExecutionTest::Function(expectedValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolCPP<int&> threadpool(ExecutionTest::OverloadFunction);
				Threading::ThreadPoolCPP<int&, int> overloadThreadPool(ExecutionTest::OverloadFunction);
				threadpool.Push(std::ref(testValue));
				overloadThreadPool.Push(std::ref(testValue), incrementValue);
				ExecutionTest::OverloadFunction(expectedValue);
				ExecutionTest::OverloadFunction(expectedValue, incrementValue);
				threadpool.Wait();
				overloadThreadPool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolCPP threadpool(ExecutionTest::Object::Static);
				threadpool.Push(std::ref(testValue));
				ExecutionTest::Object::Static(expectedValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				ExecutionTest::Object testObject;
				ExecutionTest::Object expectedObject;
				ASSERT_EXPECTED_STORE(expectedObject, testObject);
				ASSERT_EXPECTED_VALUE;

				{
					Threading::ThreadPoolCPP<int> threadpool(&ExecutionTest::Object::Member, &testObject);
					threadpool.Push(testValue);
					expectedObject.Member(expectedValue);
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}

				{
					Threading::ThreadPoolCPP<> threadpool(&ExecutionTest::Object::Member, &testObject);
					threadpool.Push();
					expectedObject.Member();
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}

				{
					Threading::ThreadPoolCPP<ExecutionTest::Object&, int&> threadpool(&ExecutionTest::Object::ConstMember);
					threadpool.Push(std::ref(testObject), std::ref(testValue));
					expectedObject.ConstMember(expectedValue);
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}
			}

			{
				Threading::ThreadPoolCPP<int&> threadpool(&ExecutionTest::Object::OverLoadStatic);
				threadpool.Push(testValue);
				ExecutionTest::Object::OverLoadStatic(expectedValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolCPP<int&, int> threadpool(&ExecutionTest::Object::OverLoadStatic);
				threadpool.Push(testValue, incrementValue);
				ExecutionTest::Object::OverLoadStatic(expectedValue, incrementValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				ExecutionTest::Callable expectedCallable;
				ExecutionTest::Callable testCallable;
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);

				Threading::ThreadPoolCPP<int> setter(std::ref(testCallable));
				setter.Push(testValue);
				setter.Wait();
				expectedCallable(expectedValue);
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;
				
				Threading::ThreadPoolCPP<> increment(std::ref(testCallable));
				increment.Push();
				increment.Wait();
				expectedCallable();
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;
				
				Threading::ThreadPoolCPP<int*> getter(std::ref(testCallable));
				getter.Push(&testValue);
				getter.Wait();
				expectedCallable(&expectedValue);
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;
			}
		}
#undef ASSERT_EXPECTED_VALUE
#undef ASSERT_EXPECTED_STORE
	};
}
