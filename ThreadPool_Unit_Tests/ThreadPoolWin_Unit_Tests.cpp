#include "pch.h"
#include "UnitTestImplementations.hpp"
#include "CppUnitTest.h"
#include "ThreadPoolWin.hpp"
#include <functional>
#include <random>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ThreadPoolUnitTests {
	TEST_CLASS(ThreadPoolWinUnitTests) {
	public:
		// This test checks if each ThreadPool will construct when scoped alone. This allows easier debugging for which test causes a compiler error
		TEST_METHOD(ThreadPoolWin_Constructor_Incongruous) {
			{
				Threading::ThreadPoolWin nonMemberStaticThreadPool(ConstructorTest::Function);
				Logger::WriteMessage("ThreadPoolWin->Constructor: Non-member static function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolWin memberStaticThreadPool(&ConstructorTest::Object::Static);
				Logger::WriteMessage("ThreadPoolWin->Constructor: Member static function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolWin memberThreadPool(&ConstructorTest::Object::Member, &object);
				Logger::WriteMessage("ThreadPoolWin->Constructor: Member function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolWin constMemberThreadPool(&ConstructorTest::Object::ConstMember, &object);
				Logger::WriteMessage("ThreadPoolWin->Constructor: Const member function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolWin overloadVoidThreadPool((void(*)())(ConstructorTest::OverloadFunction));
				Logger::WriteMessage("ThreadPoolWin->Constructor: Overloaded static function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolWin overloadIntThreadPool((void(*)(long))(ConstructorTest::OverloadFunction));
				Logger::WriteMessage("ThreadPoolWin->Constructor: Overloaded static function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolWin overloadVoidThreadPool((void(*)())(ConstructorTest::Object::OverLoadStatic));
				Logger::WriteMessage("ThreadPoolWin->Constructor: Overloaded member static function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolWin overloadIntThreadPool((void(*)(long))(ConstructorTest::Object::OverLoadStatic));
				Logger::WriteMessage("ThreadPoolWin->Constructor: Overloaded member static function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolWin overloadVoidThreadPool((void(ConstructorTest::Object::*)())(&ConstructorTest::Object::OverLoadMember), &object);
				Logger::WriteMessage("ThreadPoolWin->Constructor: Overloaded member static function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolWin overloadIntThreadPool((void(ConstructorTest::Object::*)(long))(&ConstructorTest::Object::OverLoadMember), &object);
				Logger::WriteMessage("ThreadPoolWin->Constructor: Overloaded member static function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				auto func = std::bind(&ConstructorTest::Object::Member, &object);
				Threading::ThreadPoolWin bindThreadPool(func);
				Logger::WriteMessage("ThreadPoolWin->Constructor: std::bind function object threadpool construction completed.\n");
			}

			{
				ConstructorTest::Callable callable;
				Threading::ThreadPoolWin callableThreadPool(callable);
				Logger::WriteMessage("ThreadPoolWin->Constructor: Callable object threadpool construction completed.\n");
			}

			Assert::AreEqual(0, 0);
		}

		// This test checks if all threadpools with successfully construct when scoped congruously
		TEST_METHOD(ThreadPoolWin_Constructor_Congruous) {
			Threading::ThreadPoolWin nonMemberStaticThreadPool(ConstructorTest::Function);
			Logger::WriteMessage("ThreadPoolWin->Constructor: Non-member static function threadpool construction completed.\n");

			Threading::ThreadPoolWin memberStaticThreadPool(&ConstructorTest::Object::Static);
			Logger::WriteMessage("ThreadPoolWin->Constructor: Member static function threadpool construction completed.\n");

			ConstructorTest::Object object;
			Threading::ThreadPoolWin memberThreadPool(&ConstructorTest::Object::Member, &object);
			Logger::WriteMessage("ThreadPoolWin->Constructor: Member function threadpool construction completed.\n");

			Threading::ThreadPoolWin constMemberThreadPool(&ConstructorTest::Object::ConstMember, &object);
			Logger::WriteMessage("ThreadPoolWin->Constructor: Const member function threadpool construction completed.\n");

			Threading::ThreadPoolWin staticOverloadVoidThreadPool((void(*)())(ConstructorTest::OverloadFunction));
			Logger::WriteMessage("ThreadPoolWin->Constructor: Overloaded static function threadpool construction completed.\n");

			Threading::ThreadPoolWin staticOverloadIntThreadPool((void(*)(long))(ConstructorTest::OverloadFunction));
			Logger::WriteMessage("ThreadPoolWin->Constructor: Overloaded static function threadpool construction completed.\n");

			Threading::ThreadPoolWin staticMemberOverloadVoidThreadPool((void(*)())(ConstructorTest::Object::OverLoadStatic));
			Logger::WriteMessage("ThreadPoolWin->Constructor: Overloaded member static function threadpool construction completed.\n");

			Threading::ThreadPoolWin staticMemberOverloadIntThreadPool((void(*)(long))(ConstructorTest::Object::OverLoadStatic));
			Logger::WriteMessage("ThreadPoolWin->Constructor: Overloaded member static function threadpool construction completed.\n");

			Threading::ThreadPoolWin memberOverloadVoidThreadPool((void(ConstructorTest::Object::*)())(&ConstructorTest::Object::OverLoadMember), &object);
			Logger::WriteMessage("ThreadPoolWin->Constructor: Overloaded member static function threadpool construction completed.\n");

			Threading::ThreadPoolWin memberOverloadIntThreadPool((void(ConstructorTest::Object::*)(long))(&ConstructorTest::Object::OverLoadMember), &object);
			Logger::WriteMessage("ThreadPoolWin->Constructor: Overloaded member static function threadpool construction completed.\n");

			auto func = std::bind(&ConstructorTest::Object::Member, &object);
			Threading::ThreadPoolWin bindThreadPool(func);
			Logger::WriteMessage("ThreadPoolWin->Constructor: std::bind function object threadpool construction completed.\n");

			ConstructorTest::Callable callable;
			Threading::ThreadPoolWin callableThreadPool(callable);
			Logger::WriteMessage("ThreadPoolWin->Constructor: Callable object threadpool construction completed.\n");

			Assert::AreEqual(0, 0);
		}

#define ASSERT_EXPECTED_VALUE Assert::AreEqual(expectedValue, testValue)
#define ASSERT_EXPECTED_STORE(expected, test) Assert::AreEqual(expected.store, test.store)

		TEST_METHOD(ThreadPoolWin_Execution_Single) {
			long expectedValue = 0;
			long testValue = 0;
			long incrementValue = 5;
			// Sanity check
			ASSERT_EXPECTED_VALUE;

			{
				Threading::ThreadPoolWin threadpool(ExecutionTest::Function);
				threadpool.Push(std::ref(testValue));
				ExecutionTest::Function(expectedValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolWin<long&> threadpool(ExecutionTest::OverloadFunction);
				Threading::ThreadPoolWin<long&, long> overloadThreadPool(ExecutionTest::OverloadFunction);
				threadpool.Push(std::ref(testValue));
				overloadThreadPool.Push(std::ref(testValue), incrementValue);
				ExecutionTest::OverloadFunction(expectedValue);
				ExecutionTest::OverloadFunction(expectedValue, incrementValue);
				threadpool.Wait();
				overloadThreadPool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolWin threadpool(ExecutionTest::Object::Static);
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
					Threading::ThreadPoolWin<long> threadpool(&ExecutionTest::Object::Member, &testObject);
					threadpool.Push(testValue);
					expectedObject.Member(expectedValue);
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}

				{
					Threading::ThreadPoolWin<> threadpool(&ExecutionTest::Object::Member, &testObject);
					threadpool.Push();
					expectedObject.Member();
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}

				{
					Threading::ThreadPoolWin<ExecutionTest::Object&, long&> threadpool(&ExecutionTest::Object::ConstMember);
					threadpool.Push(std::ref(testObject), std::ref(testValue));
					expectedObject.ConstMember(expectedValue);
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}
			}

			{
				Threading::ThreadPoolWin<long&> threadpool(&ExecutionTest::Object::OverLoadStatic);
				threadpool.Push(testValue);
				ExecutionTest::Object::OverLoadStatic(expectedValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolWin<long&, long> threadpool(&ExecutionTest::Object::OverLoadStatic);
				threadpool.Push(testValue, incrementValue);
				ExecutionTest::Object::OverLoadStatic(expectedValue, incrementValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				ExecutionTest::Callable expectedCallable;
				ExecutionTest::Callable testCallable;
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);

				Threading::ThreadPoolWin<long> setter(std::ref(testCallable));
				setter.Push(testValue);
				setter.Wait();
				expectedCallable(expectedValue);
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;

				Threading::ThreadPoolWin<> increment(std::ref(testCallable));
				increment.Push();
				increment.Wait();
				expectedCallable();
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;

				Threading::ThreadPoolWin<long*> getter(std::ref(testCallable));
				getter.Push(&testValue);
				getter.Wait();
				expectedCallable(&expectedValue);
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;
			}
		}

		TEST_METHOD(ThreadPoolWin_Execution_Multiple) {
			long expectedValue = 0;
			long testValue = 0;
			std::uniform_int_distribution<long> uid(25, 250);
			std::default_random_engine randomEngine;
			long incrementValue = uid(randomEngine);
			incrementValue = uid(randomEngine);
			const long REPETITION_NUMBER = uid(randomEngine);
			// Sanity check
			ASSERT_EXPECTED_VALUE;

			{
				Threading::ThreadPoolWin threadpool(ExecutionTest::Function);
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(std::ref(testValue));
					ExecutionTest::Function(expectedValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolWin<long&> threadpool(ExecutionTest::OverloadFunction);
				Threading::ThreadPoolWin<long&, long> overloadThreadPool(ExecutionTest::OverloadFunction);
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(std::ref(testValue));
					overloadThreadPool.Push(std::ref(testValue), incrementValue);
					ExecutionTest::OverloadFunction(expectedValue);
					ExecutionTest::OverloadFunction(expectedValue, incrementValue);
				}
				threadpool.Wait();
				overloadThreadPool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolWin threadpool(ExecutionTest::Object::Static);
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(std::ref(testValue));
					ExecutionTest::Object::Static(expectedValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				ExecutionTest::Object testObject;
				ExecutionTest::Object expectedObject;
				ASSERT_EXPECTED_STORE(expectedObject, testObject);
				ASSERT_EXPECTED_VALUE;

				{
					Threading::ThreadPoolWin<long> threadpool(&ExecutionTest::Object::Member, &testObject);
					for (long i = 0; i < REPETITION_NUMBER; ++i) {
						threadpool.Push(testValue);
						expectedObject.Member(expectedValue);
					}
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}

				{
					Threading::ThreadPoolWin<> threadpool(&ExecutionTest::Object::Member, &testObject);
					for (long i = 0; i < REPETITION_NUMBER; ++i) {
						threadpool.Push();
						expectedObject.Member();
					}
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}

				{
					Threading::ThreadPoolWin<ExecutionTest::Object&, long&> threadpool(&ExecutionTest::Object::ConstMember);
					for (long i = 0; i < REPETITION_NUMBER; ++i) {
						threadpool.Push(std::ref(testObject), std::ref(testValue));
						expectedObject.ConstMember(expectedValue);
					}
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}
			}

			{
				Threading::ThreadPoolWin<long&> threadpool(&ExecutionTest::Object::OverLoadStatic);
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(testValue);
					ExecutionTest::Object::OverLoadStatic(expectedValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolWin<long&, long> threadpool(&ExecutionTest::Object::OverLoadStatic);
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(testValue, incrementValue);
					ExecutionTest::Object::OverLoadStatic(expectedValue, incrementValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				ExecutionTest::Callable expectedCallable;
				ExecutionTest::Callable testCallable;
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);

				Threading::ThreadPoolWin<long> setter(std::ref(testCallable));
				Threading::ThreadPoolWin<> increment(std::ref(testCallable));
				Threading::ThreadPoolWin<long*> getter(std::ref(testCallable));

				setter.Push(testValue);
				expectedCallable(expectedValue);
				setter.Wait();
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;

				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					increment.Push();
					expectedCallable();
				}
				increment.Wait();
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;

				getter.Push(&testValue);
				expectedCallable(&expectedValue);
				getter.Wait();
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;
			}
		}
#undef ASSERT_EXPECTED_VALUE
#undef ASSERT_EXPECTED_STORE
	};
}
