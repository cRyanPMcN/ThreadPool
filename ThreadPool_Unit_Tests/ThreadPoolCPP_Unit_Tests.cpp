#include "pch.h"
#include "UnitTestImplementations.hpp"
#include "CppUnitTest.h"
#include "ThreadPoolCPP.hpp"
#include <random>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ThreadPoolUnitTests {
	TEST_CLASS(ThreadPoolCPPUnitTests) {
	public:
		TEST_METHOD(ThreadPoolCPP_Constructor) {
			Threading::ThreadPoolCPP threadpool;
			Logger::WriteMessage("ThreadPoolCPP->Constructor Passed.\n");
		}

#define ASSERT_EXPECTED_VALUE Assert::AreEqual(expectedValue, testValue)
#define ASSERT_EXPECTED_STORE(expected, test) Assert::AreEqual(expected.store, test.store)

		TEST_METHOD(ThreadPoolCPP_Execution_Single) {
			Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Start\n");
			Threading::ThreadPoolCPP threadpool;
			long expectedValue = 0;
			long testValue = 0;
			long incrementValue = 5;
			// Sanity check
			ASSERT_EXPECTED_VALUE;

			{
				threadpool.Push(ExecutionTest::Function, std::ref(testValue));
				ExecutionTest::Function(expectedValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}
			Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Static Function Passed.\n");

			{
				threadpool.Push((void(*)(long&))ExecutionTest::OverloadFunction, std::ref(testValue));
				threadpool.Push((void(*)(long&, long))ExecutionTest::OverloadFunction, std::ref(testValue), incrementValue);
				ExecutionTest::OverloadFunction(expectedValue);
				ExecutionTest::OverloadFunction(expectedValue, incrementValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}
			
			Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Static Overload Function Passed.\n");

			{
				threadpool.Push(ExecutionTest::Object::Static, std::ref(testValue));
				ExecutionTest::Object::Static(expectedValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}
			
			Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Class Static Member Function Passed.\n");

			{
				ExecutionTest::Object testObject;
				ExecutionTest::Object expectedObject;
				ASSERT_EXPECTED_STORE(expectedObject, testObject);
				ASSERT_EXPECTED_VALUE;

				{
					threadpool.Push((void(ExecutionTest::Object::*)(long))&ExecutionTest::Object::Member, &testObject, testValue);
					expectedObject.Member(expectedValue);
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}
				
				Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Member Function One-Arg Passed.\n");

				{
					threadpool.Push((void(ExecutionTest::Object::*)())&ExecutionTest::Object::Member, &testObject);
					expectedObject.Member();
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}
			
				Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Member Function Zero-Arg Passed.\n");

				{
					threadpool.Push(&ExecutionTest::Object::ConstMember, testObject, std::ref(testValue));
					expectedObject.ConstMember(expectedValue);
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}
				
				Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Const-Member Function Passed.\n");
			}
			
			
			{
				threadpool.Push((void(*)(long&))ExecutionTest::Object::OverLoadStatic, std::ref(testValue));
				ExecutionTest::Object::OverLoadStatic(expectedValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}
			
			Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Class Static OverLoad Function Stage-One Passed.\n");
			
			{
				threadpool.Push((void(*)(long&, long))ExecutionTest::Object::OverLoadStatic, std::ref(testValue), incrementValue);
				ExecutionTest::Object::OverLoadStatic(expectedValue, incrementValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}

			Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Class Static OverLoad Function Stage-Two Passed\n");
			
			{
				ExecutionTest::Callable expectedCallable;
				ExecutionTest::Callable testCallable;
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Callable Object Stage-One Passed.\n");

				threadpool.Push(std::ref(testCallable), testValue);
				threadpool.Wait();
				expectedCallable(expectedValue);
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;
				Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Callable Object Stage-Two Passed.\n");

				threadpool.Push(std::ref(testCallable));
				threadpool.Wait();
				expectedCallable();
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;
				Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Callable Object Stage-Three Passed.\n");

				threadpool.Push(std::ref(testCallable), &testValue);
				threadpool.Wait();
				expectedCallable(&expectedValue);
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;
				Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Callable Object Stage-Four Passed.\n");
			}
			
			Logger::WriteMessage("ThreadPoolCPP->Execution_Single: Callable Object Passed.\n");

			Logger::WriteMessage("ThreadPoolCPP->Execution_Single: End\n");
		}

		TEST_METHOD(ThreadPoolCPP_Execution_Multiple) {
			Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: Start\n");
			long expectedValue = 0;
			long testValue = 0;
			std::uniform_int_distribution<long> uid(25, 250);
			std::default_random_engine randomEngine;
			long incrementValue = uid(randomEngine);
			incrementValue = uid(randomEngine);
			const long REPETITION_NUMBER = uid(randomEngine);
			// Sanity check
			ASSERT_EXPECTED_VALUE;
			Threading::ThreadPoolCPP threadpool;

			{
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(ExecutionTest::Function, std::ref(testValue));
					ExecutionTest::Function(expectedValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}
			
			Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: Static Function Passed\n");

			{
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push((void(*)(long&))ExecutionTest::OverloadFunction, std::ref(testValue));
					threadpool.Push<void(*)(long&, long), long&, long>(ExecutionTest::OverloadFunction, std::ref(testValue), incrementValue);
					ExecutionTest::OverloadFunction(expectedValue);
					ExecutionTest::OverloadFunction(expectedValue, incrementValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}
			
			Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: Static Overload Function Passed\n");

			{
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(ExecutionTest::Object::Static, std::ref(testValue));
					ExecutionTest::Object::Static(expectedValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}
			
			Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: Static Member Function Passed\n");

			{
				ExecutionTest::Object testObject;
				ExecutionTest::Object expectedObject;
				ASSERT_EXPECTED_STORE(expectedObject, testObject);
				ASSERT_EXPECTED_VALUE;
			
				{
					for (long i = 0; i < REPETITION_NUMBER; ++i) {
						threadpool.Push<void(ExecutionTest::Object::*)(long)>(&ExecutionTest::Object::Member, &testObject, testValue);
						expectedObject.Member(expectedValue);
					}
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}
			
				Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: Member Function One-Arg Passed\n");
			
				{
					for (long i = 0; i < REPETITION_NUMBER; ++i) {
						threadpool.Push<void(ExecutionTest::Object::*)()>(&ExecutionTest::Object::Member, &testObject);
						expectedObject.Member();
					}
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}
			
				Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: Member Function Zero-Arg Passed\n");

				{
					for (long i = 0; i < REPETITION_NUMBER; ++i) {
						threadpool.Push(&ExecutionTest::Object::ConstMember, &testObject, std::ref(testValue));
						expectedObject.ConstMember(expectedValue);
					}
					threadpool.Wait();
					ASSERT_EXPECTED_STORE(expectedObject, testObject);
					ASSERT_EXPECTED_VALUE;
				}

				Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: Const Member Function Passed\n");
			}
			
			{
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push((void(*)(long&))ExecutionTest::Object::OverLoadStatic, testValue);
					ExecutionTest::Object::OverLoadStatic(expectedValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}
			
			Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: OverLoad Static Function Stage-One Passed\n");
			
			{
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push((void(*)(long&, long))ExecutionTest::Object::OverLoadStatic, testValue, incrementValue);
					ExecutionTest::Object::OverLoadStatic(expectedValue, incrementValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE;
			}
			
			Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: OverLoad Static Function Stage-Two Passed\n");

			{
				ExecutionTest::Callable expectedCallable;
				ExecutionTest::Callable testCallable;
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: Callable Object Stage-One Passed\n");

				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(testCallable, testValue);
					expectedCallable(expectedValue);
					threadpool.Wait();
				}
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;

				Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: Callable Object Stage-Two Passed\n");

				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(testCallable);
					expectedCallable();
				}
				threadpool.Wait();
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;

				Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: Callable Object Stage-Three Passed\n");

				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(testCallable, std::ref(testValue));
					expectedCallable(&expectedValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_STORE(expectedCallable, testCallable);
				ASSERT_EXPECTED_VALUE;
			
				Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: Callable Object Stage-Four Passed\n");
			}

			Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: Callable Object Passed\n");

			Logger::WriteMessage("ThreadPoolCPP->Execution_Multiple: End\n");
		}
#undef ASSERT_EXPECTED_VALUE
#undef ASSERT_EXPECTED_STORE
	};
}
