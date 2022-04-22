#include "pch.h"
#include "UnitTestImplementations.hpp"
#include "CppUnitTest.h"
#include "ThreadPoolCPP.hpp"
#include <random>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ThreadPoolUnitTests {
	TEST_CLASS(ThreadPoolCPPUnitTests) {
	public:
		// This test checks if each ThreadPool will construct when scoped alone. This allows easier debugging for which test causes a compiler error
		TEST_METHOD(ThreadPoolCPP_Constructor_Incongruous) {
			{
				Threading::ThreadPoolCPP nonMemberStaticThreadPool(ConstructorTest::Function);
			}

			{
				Threading::ThreadPoolCPP memberStaticThreadPool(&ConstructorTest::Object::Static);
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolCPP memberThreadPool(&ConstructorTest::Object::Member, &object);
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolCPP constMemberThreadPool(&ConstructorTest::Object::ConstMember, &object);
			}

			{
				Threading::ThreadPoolCPP overloadVoidThreadPool((void(*)())(ConstructorTest::OverloadFunction));
			}

			{
				Threading::ThreadPoolCPP overloadIntThreadPool((void(*)(long))(ConstructorTest::OverloadFunction));
			}

			{
				Threading::ThreadPoolCPP overloadVoidThreadPool((void(*)())(ConstructorTest::Object::OverLoadStatic));
			}

			{
				Threading::ThreadPoolCPP overloadIntThreadPool((void(*)(long))(ConstructorTest::Object::OverLoadStatic));
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolCPP overloadVoidThreadPool((void(ConstructorTest::Object::*)())(&ConstructorTest::Object::OverLoadMember), &object);
			}

			{
				ConstructorTest::Object object;
				Threading::ThreadPoolCPP overloadIntThreadPool((void(ConstructorTest::Object::*)(long))(&ConstructorTest::Object::OverLoadMember), &object);
			}

			{
				ConstructorTest::Object object;
				auto func = std::bind(&ConstructorTest::Object::Member, &object);
				Threading::ThreadPoolCPP bindThreadPool(func);
			}

			{
				ConstructorTest::Callable callable;
				Threading::ThreadPoolCPP callableThreadPool(callable);
			}
		}

		// This test checks if all threadpools will successfully construct when scoped congruously
		TEST_METHOD(ThreadPoolCPP_Constructor_Congruous) {
			Threading::ThreadPoolCPP nonMemberStaticThreadPool(ConstructorTest::Function);

			Threading::ThreadPoolCPP memberStaticThreadPool(&ConstructorTest::Object::Static);

			ConstructorTest::Object object;
			Threading::ThreadPoolCPP memberThreadPool(&ConstructorTest::Object::Member, &object);

			Threading::ThreadPoolCPP constMemberThreadPool(&ConstructorTest::Object::ConstMember, &object);

			Threading::ThreadPoolCPP staticOverloadVoidThreadPool((void(*)())(ConstructorTest::OverloadFunction));

			Threading::ThreadPoolCPP staticOverloadIntThreadPool((void(*)(long))(ConstructorTest::OverloadFunction));

			Threading::ThreadPoolCPP staticMemberOverloadVoidThreadPool((void(*)())(ConstructorTest::Object::OverLoadStatic));

			Threading::ThreadPoolCPP staticMemberOverloadIntThreadPool((void(*)(long))(ConstructorTest::Object::OverLoadStatic));

			Threading::ThreadPoolCPP memberOverloadVoidThreadPool((void(ConstructorTest::Object::*)())(&ConstructorTest::Object::OverLoadMember), &object);

			Threading::ThreadPoolCPP memberOverloadIntThreadPool((void(ConstructorTest::Object::*)(long))(&ConstructorTest::Object::OverLoadMember), &object);

			auto func = std::bind(&ConstructorTest::Object::Member, &object);
			Threading::ThreadPoolCPP bindThreadPool(func);

			ConstructorTest::Callable callable;
			Threading::ThreadPoolCPP callableThreadPool(callable);
		}

#define ASSERT_EXPECTED_VALUE Assert::AreEqual(expectedValue, testValue)
#define ASSERT_EXPECTED_STORE(expected, test) Assert::AreEqual(expected.store, test.store)

		TEST_METHOD(ThreadPoolCPP_Execution_Single) {
			long expectedValue = 0;
			long testValue = 0;
			long incrementValue = 5;
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
				Threading::ThreadPoolCPP<long&> threadpool(ExecutionTest::OverloadFunction);
				Threading::ThreadPoolCPP<long&, long> overloadThreadPool(ExecutionTest::OverloadFunction);
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
					Threading::ThreadPoolCPP<long> threadpool(&ExecutionTest::Object::Member, &testObject);
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
					Threading::ThreadPoolCPP<ExecutionTest::Object&, long&> threadpool(&ExecutionTest::Object::ConstMember);
					threadpool.Push(std::ref(testObject), std::ref(testValue));
					expectedObject.ConstMember(expectedValue);
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}
			}

			{
				Threading::ThreadPoolCPP<long&> threadpool(&ExecutionTest::Object::OverLoadStatic);
				threadpool.Push(testValue);
				ExecutionTest::Object::OverLoadStatic(expectedValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolCPP<long&, long> threadpool(&ExecutionTest::Object::OverLoadStatic);
				threadpool.Push(testValue, incrementValue);
				ExecutionTest::Object::OverLoadStatic(expectedValue, incrementValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				ExecutionTest::Callable expectedCallable;
				ExecutionTest::Callable testCallable;
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);

				Threading::ThreadPoolCPP<long> setter(std::ref(testCallable));
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
				
				Threading::ThreadPoolCPP<long*> getter(std::ref(testCallable));
				getter.Push(&testValue);
				getter.Wait();
				expectedCallable(&expectedValue);
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;
			}
		}

		TEST_METHOD(ThreadPoolCPP_Execution_Multiple) {
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
				Threading::ThreadPoolCPP threadpool(ExecutionTest::Function);
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(std::ref(testValue));
					ExecutionTest::Function(expectedValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolCPP<long&> threadpool(ExecutionTest::OverloadFunction);
				Threading::ThreadPoolCPP<long&, long> overloadThreadPool(ExecutionTest::OverloadFunction);
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
				Threading::ThreadPoolCPP threadpool(ExecutionTest::Object::Static);
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
					Threading::ThreadPoolCPP<long> threadpool(&ExecutionTest::Object::Member, &testObject);
					for (long i = 0; i < REPETITION_NUMBER; ++i) {
						threadpool.Push(testValue);
						expectedObject.Member(expectedValue);
					}
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}

				{
					Threading::ThreadPoolCPP<> threadpool(&ExecutionTest::Object::Member, &testObject);
					for (long i = 0; i < REPETITION_NUMBER; ++i) {
						threadpool.Push();
						expectedObject.Member();
					}
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}

				{
					Threading::ThreadPoolCPP<ExecutionTest::Object&, long&> threadpool(&ExecutionTest::Object::ConstMember);
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
				Threading::ThreadPoolCPP<long&> threadpool(&ExecutionTest::Object::OverLoadStatic);
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(testValue);
					ExecutionTest::Object::OverLoadStatic(expectedValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			{
				Threading::ThreadPoolCPP<long&, long> threadpool(&ExecutionTest::Object::OverLoadStatic);
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

				Threading::ThreadPoolCPP<long> setter(std::ref(testCallable));
				Threading::ThreadPoolCPP<> increment(std::ref(testCallable));
				Threading::ThreadPoolCPP<long*> getter(std::ref(testCallable));

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
