#include "pch.h"
#include "UnitTestImplementations.hpp"
#include "CppUnitTest.h"
#include "ThreadPoolWin32.hpp"
#include <functional>
#include <random>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ThreadPoolUnitTests {
	TEST_CLASS(ThreadPoolWinUnitTests) {
	public:
		// This test checks if each ThreadPool will construct when scoped alone. This allows easier debugging for which test causes a compiler error
		TEST_METHOD(ThreadPoolWin_Constructor_Incongruous) {
			{
				Threading::ThreadPoolWin32 nonMemberStaticThreadPool(ConstructorTest::Function);
				Logger::WriteMessage("ThreadPoolWin32->Constructor: Non-member static function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolWin32 memberStaticThreadPool(&ConstructorTest::Object::Static);
				Logger::WriteMessage("ThreadPoolWin32->Constructor: Member static function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolWin32 memberThreadPool(&ConstructorTest::Object::Member, &object);
				Logger::WriteMessage("ThreadPoolWin32->Constructor: Member function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolWin32 constMemberThreadPool(&ConstructorTest::Object::ConstMember, &object);
				Logger::WriteMessage("ThreadPoolWin32->Constructor: Const member function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolWin32 overloadVoidThreadPool((void(*)())(ConstructorTest::OverloadFunction));
				Logger::WriteMessage("ThreadPoolWin32->Constructor: Overloaded static function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolWin32 overloadIntThreadPool((void(*)(long))(ConstructorTest::OverloadFunction));
				Logger::WriteMessage("ThreadPoolWin32->Constructor: Overloaded static function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolWin32 overloadVoidThreadPool((void(*)())(ConstructorTest::Object::OverLoadStatic));
				Logger::WriteMessage("ThreadPoolWin32->Constructor: Overloaded member static function threadpool construction completed.\n");
			}

			{
				Threading::ThreadPoolWin32 overloadIntThreadPool((void(*)(long))(ConstructorTest::Object::OverLoadStatic));
				Logger::WriteMessage("ThreadPoolWin32->Constructor: Overloaded member static function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolWin32 overloadVoidThreadPool((void(ConstructorTest::Object::*)())(&ConstructorTest::Object::OverLoadMember), &object);
				Logger::WriteMessage("ThreadPoolWin32->Constructor: Overloaded member static function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolWin32 overloadIntThreadPool((void(ConstructorTest::Object::*)(long))(&ConstructorTest::Object::OverLoadMember), &object);
				Logger::WriteMessage("ThreadPoolWin32->Constructor: Overloaded member static function threadpool construction completed.\n");
			}

			{
				ConstructorTest::Object object;
				auto func = std::bind(&ConstructorTest::Object::Member, &object);
				Threading::ThreadPoolWin32 bindThreadPool(func);
				Logger::WriteMessage("ThreadPoolWin32->Constructor: std::bind function object threadpool construction completed.\n");
			}

			{
				ConstructorTest::Callable callable;
				Threading::ThreadPoolWin32 callableThreadPool(callable);
				Logger::WriteMessage("ThreadPoolWin32->Constructor: Callable object threadpool construction completed.\n");
			}

			Assert::AreEqual(0, 0);
		}

		// This test checks if all threadpools with successfully construct when scoped congruously
		TEST_METHOD(ThreadPoolWin_Constructor_Congruous) {
			Threading::ThreadPoolWin32 nonMemberStaticThreadPool(ConstructorTest::Function);
			Logger::WriteMessage("ThreadPoolWin32->Constructor: Non-member static function threadpool construction completed.\n");

			Threading::ThreadPoolWin32 memberStaticThreadPool(&ConstructorTest::Object::Static);
			Logger::WriteMessage("ThreadPoolWin32->Constructor: Member static function threadpool construction completed.\n");

			ConstructorTest::Object object;
			Threading::ThreadPoolWin32 memberThreadPool(&ConstructorTest::Object::Member, &object);
			Logger::WriteMessage("ThreadPoolWin32->Constructor: Member function threadpool construction completed.\n");

			Threading::ThreadPoolWin32 constMemberThreadPool(&ConstructorTest::Object::ConstMember, &object);
			Logger::WriteMessage("ThreadPoolWin32->Constructor: Const member function threadpool construction completed.\n");

			Threading::ThreadPoolWin32 staticOverloadVoidThreadPool((void(*)())(ConstructorTest::OverloadFunction));
			Logger::WriteMessage("ThreadPoolWin32->Constructor: Overloaded static function threadpool construction completed.\n");

			Threading::ThreadPoolWin32 staticOverloadIntThreadPool((void(*)(long))(ConstructorTest::OverloadFunction));
			Logger::WriteMessage("ThreadPoolWin32->Constructor: Overloaded static function threadpool construction completed.\n");

			Threading::ThreadPoolWin32 staticMemberOverloadVoidThreadPool((void(*)())(ConstructorTest::Object::OverLoadStatic));
			Logger::WriteMessage("ThreadPoolWin32->Constructor: Overloaded member static function threadpool construction completed.\n");

			Threading::ThreadPoolWin32 staticMemberOverloadIntThreadPool((void(*)(long))(ConstructorTest::Object::OverLoadStatic));
			Logger::WriteMessage("ThreadPoolWin32->Constructor: Overloaded member static function threadpool construction completed.\n");

			Threading::ThreadPoolWin32 memberOverloadVoidThreadPool((void(ConstructorTest::Object::*)())(&ConstructorTest::Object::OverLoadMember), &object);
			Logger::WriteMessage("ThreadPoolWin32->Constructor: Overloaded member static function threadpool construction completed.\n");

			Threading::ThreadPoolWin32 memberOverloadIntThreadPool((void(ConstructorTest::Object::*)(long))(&ConstructorTest::Object::OverLoadMember), &object);
			Logger::WriteMessage("ThreadPoolWin32->Constructor: Overloaded member static function threadpool construction completed.\n");

			auto func = std::bind(&ConstructorTest::Object::Member, &object);
			Threading::ThreadPoolWin32 bindThreadPool(func);
			Logger::WriteMessage("ThreadPoolWin32->Constructor: std::bind function object threadpool construction completed.\n");

			ConstructorTest::Callable callable;
			Threading::ThreadPoolWin32 callableThreadPool(callable);
			Logger::WriteMessage("ThreadPoolWin32->Constructor: Callable object threadpool construction completed.\n");

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
				Threading::ThreadPoolWin32 threadpool(ExecutionTest::Function);
				threadpool.Push(std::ref(testValue));
				ExecutionTest::Function(expectedValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolWin32<long&> threadpool(ExecutionTest::OverloadFunction);
				Threading::ThreadPoolWin32<long&, long> overloadThreadPool(ExecutionTest::OverloadFunction);
				threadpool.Push(std::ref(testValue));
				overloadThreadPool.Push(std::ref(testValue), incrementValue);
				ExecutionTest::OverloadFunction(expectedValue);
				ExecutionTest::OverloadFunction(expectedValue, incrementValue);
				threadpool.Wait();
				overloadThreadPool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolWin32 threadpool(ExecutionTest::Object::Static);
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
					Threading::ThreadPoolWin32<long> threadpool(&ExecutionTest::Object::Member, &testObject);
					threadpool.Push(testValue);
					expectedObject.Member(expectedValue);
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}

				{
					Threading::ThreadPoolWin32<> threadpool(&ExecutionTest::Object::Member, &testObject);
					threadpool.Push();
					expectedObject.Member();
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}

				{
					Threading::ThreadPoolWin32<ExecutionTest::Object&, long&> threadpool(&ExecutionTest::Object::ConstMember);
					threadpool.Push(std::ref(testObject), std::ref(testValue));
					expectedObject.ConstMember(expectedValue);
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}
			}

			{
				Threading::ThreadPoolWin32<long&> threadpool(&ExecutionTest::Object::OverLoadStatic);
				threadpool.Push(testValue);
				ExecutionTest::Object::OverLoadStatic(expectedValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolWin32<long&, long> threadpool(&ExecutionTest::Object::OverLoadStatic);
				threadpool.Push(testValue, incrementValue);
				ExecutionTest::Object::OverLoadStatic(expectedValue, incrementValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				ExecutionTest::Callable expectedCallable;
				ExecutionTest::Callable testCallable;
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);

				Threading::ThreadPoolWin32<long> setter(std::ref(testCallable));
				setter.Push(testValue);
				setter.Wait();
				expectedCallable(expectedValue);
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;

				Threading::ThreadPoolWin32<> increment(std::ref(testCallable));
				increment.Push();
				increment.Wait();
				expectedCallable();
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;

				Threading::ThreadPoolWin32<long*> getter(std::ref(testCallable));
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
				Threading::ThreadPoolWin32 threadpool(ExecutionTest::Function);
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(std::ref(testValue));
					ExecutionTest::Function(expectedValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolWin32<long&> threadpool(ExecutionTest::OverloadFunction);
				Threading::ThreadPoolWin32<long&, long> overloadThreadPool(ExecutionTest::OverloadFunction);
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
				Threading::ThreadPoolWin32 threadpool(ExecutionTest::Object::Static);
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
					Threading::ThreadPoolWin32<long> threadpool(&ExecutionTest::Object::Member, &testObject);
					for (long i = 0; i < REPETITION_NUMBER; ++i) {
						threadpool.Push(testValue);
						expectedObject.Member(expectedValue);
					}
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}

				{
					Threading::ThreadPoolWin32<> threadpool(&ExecutionTest::Object::Member, &testObject);
					for (long i = 0; i < REPETITION_NUMBER; ++i) {
						threadpool.Push();
						expectedObject.Member();
					}
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}

				{
					Threading::ThreadPoolWin32<ExecutionTest::Object&, long&> threadpool(&ExecutionTest::Object::ConstMember);
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
				Threading::ThreadPoolWin32<long&> threadpool(&ExecutionTest::Object::OverLoadStatic);
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(testValue);
					ExecutionTest::Object::OverLoadStatic(expectedValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolWin32<long&, long> threadpool(&ExecutionTest::Object::OverLoadStatic);
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

				Threading::ThreadPoolWin32<long> setter(std::ref(testCallable));
				Threading::ThreadPoolWin32<> increment(std::ref(testCallable));
				Threading::ThreadPoolWin32<long*> getter(std::ref(testCallable));

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
