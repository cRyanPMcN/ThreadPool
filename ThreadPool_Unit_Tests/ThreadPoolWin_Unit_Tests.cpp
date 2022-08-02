#include "pch.h"
#include "UnitTestImplementations.hpp"
#include "CppUnitTest.h"
#include "ThreadPoolWin32.hpp"
#include <random>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ThreadPoolUnitTests {
	TEST_CLASS(ThreadPoolWin32UnitTests) {
public:
	TEST_METHOD(ThreadPoolWin32_Constructor) {
		Threading::ThreadPoolWin32 threadpool;
		Logger::WriteMessage("ThreadPoolWin32->Constructor Passed.\n");
	}

#define ASSERT_EXPECTED_VALUE(expected, test) Assert::AreEqual(expected, test)

	TEST_METHOD(ThreadPoolWin32_Execution_Single) {
		Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Start\n");
		Threading::ThreadPoolWin32 threadpool;
		long expectedValue = 0;
		long testValue = 0;
		long incrementValue = 5;
		// Sanity check
		ASSERT_EXPECTED_VALUE(expectedValue, testValue);

		{
			threadpool.Push(ExecutionTest::Function, std::ref(testValue));
			ExecutionTest::Function(expectedValue);
			threadpool.Wait();
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);
		}
		Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Static Function Passed.\n");

		{
			threadpool.Push((void(*)(long&))ExecutionTest::OverloadFunction, std::ref(testValue));
			threadpool.Push((void(*)(long&, long))ExecutionTest::OverloadFunction, std::ref(testValue), incrementValue);
			ExecutionTest::OverloadFunction(expectedValue);
			ExecutionTest::OverloadFunction(expectedValue, incrementValue);
			threadpool.Wait();
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);
		}

		Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Static Overload Function Passed.\n");

		{
			threadpool.Push(ExecutionTest::Object::Static, std::ref(testValue));
			ExecutionTest::Object::Static(expectedValue);
			threadpool.Wait();
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);
		}

		Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Class Static Member Function Passed.\n");

		{
			ExecutionTest::Object testObject;
			ExecutionTest::Object expectedObject;
			ASSERT_EXPECTED_VALUE(expectedObject.store, testObject.store);
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);

			{
				threadpool.Push((void(ExecutionTest::Object::*)(long)) & ExecutionTest::Object::Member, &testObject, testValue);
				expectedObject.Member(expectedValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE(expectedObject.store, testObject.store);
				ASSERT_EXPECTED_VALUE(expectedValue, testValue);
			}

			Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Member Function One-Arg Passed.\n");

			{
				threadpool.Push((void(ExecutionTest::Object::*)()) & ExecutionTest::Object::Member, &testObject);
				expectedObject.Member();
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE(expectedObject.store, testObject.store);
				ASSERT_EXPECTED_VALUE(expectedValue, testValue);
			}

			Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Member Function Zero-Arg Passed.\n");

			{
				threadpool.Push(&ExecutionTest::Object::ConstMember, testObject, std::ref(testValue));
				expectedObject.ConstMember(expectedValue);
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE(expectedObject.store, testObject.store);
				ASSERT_EXPECTED_VALUE(expectedValue, testValue);
			}

			Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Const-Member Function Passed.\n");
		}


		{
			threadpool.Push((void(*)(long&))ExecutionTest::Object::OverLoadStatic, std::ref(testValue));
			ExecutionTest::Object::OverLoadStatic(expectedValue);
			threadpool.Wait();
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);
		}

		Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Class Static OverLoad Function Stage-One Passed.\n");

		{
			threadpool.Push((void(*)(long&, long))ExecutionTest::Object::OverLoadStatic, std::ref(testValue), incrementValue);
			ExecutionTest::Object::OverLoadStatic(expectedValue, incrementValue);
			threadpool.Wait();
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);
		}

		Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Class Static OverLoad Function Stage-Two Passed\n");

		{
			ExecutionTest::Callable expectedCallable;
			ExecutionTest::Callable testCallable;
			ASSERT_EXPECTED_VALUE(expectedCallable.store, testCallable.store);
			Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Callable Object Stage-One Passed.\n");

			threadpool.Push(std::ref(testCallable), testValue);
			threadpool.Wait();
			expectedCallable(expectedValue);
			ASSERT_EXPECTED_VALUE(expectedCallable.store, testCallable.store);
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);
			Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Callable Object Stage-Two Passed.\n");

			threadpool.Push(std::ref(testCallable));
			threadpool.Wait();
			expectedCallable();
			ASSERT_EXPECTED_VALUE(expectedCallable.store, testCallable.store);
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);
			Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Callable Object Stage-Three Passed.\n");

			threadpool.Push(std::ref(testCallable), &testValue);
			threadpool.Wait();
			expectedCallable(&expectedValue);
			ASSERT_EXPECTED_VALUE(expectedCallable.store, testCallable.store);
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);
			Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Callable Object Stage-Four Passed.\n");
		}

		Logger::WriteMessage("ThreadPoolWin32->Execution_Single: Callable Object Passed.\n");

		Logger::WriteMessage("ThreadPoolWin32->Execution_Single: End\n");
	}

	TEST_METHOD(ThreadPoolWin32_Execution_Multiple) {
		Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: Start\n");
		long expectedValue = 0;
		long testValue = 0;
		std::uniform_int_distribution<long> uid(25, 250);
		std::default_random_engine randomEngine;
		long incrementValue = uid(randomEngine);
		incrementValue = uid(randomEngine);
		const long REPETITION_NUMBER = uid(randomEngine);
		// Sanity check
		ASSERT_EXPECTED_VALUE(expectedValue, testValue);
		Threading::ThreadPoolWin32 threadpool;

		{
			for (long i = 0; i < REPETITION_NUMBER; ++i) {
				threadpool.Push(ExecutionTest::Function, std::ref(testValue));
				ExecutionTest::Function(expectedValue);
			}
			threadpool.Wait();
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);
		}

		Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: Static Function Passed\n");

		{
			for (long i = 0; i < REPETITION_NUMBER; ++i) {
				threadpool.Push((void(*)(long&))ExecutionTest::OverloadFunction, std::ref(testValue));
				threadpool.Push<void(*)(long&, long), long&, long>(ExecutionTest::OverloadFunction, std::ref(testValue), incrementValue);
				ExecutionTest::OverloadFunction(expectedValue);
				ExecutionTest::OverloadFunction(expectedValue, incrementValue);
			}
			threadpool.Wait();
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);
		}

		Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: Static Overload Function Passed\n");

		{
			for (long i = 0; i < REPETITION_NUMBER; ++i) {
				threadpool.Push(ExecutionTest::Object::Static, std::ref(testValue));
				ExecutionTest::Object::Static(expectedValue);
			}
			threadpool.Wait();
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);
		}

		Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: Static Member Function Passed\n");

		{
			ExecutionTest::Object testObject;
			ExecutionTest::Object expectedObject;
			ASSERT_EXPECTED_VALUE(expectedObject.store, testObject.store);
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);

			{
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push<void(ExecutionTest::Object::*)(long)>(&ExecutionTest::Object::Member, &testObject, testValue);
					expectedObject.Member(expectedValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE(expectedObject.store, testObject.store);
				ASSERT_EXPECTED_VALUE(expectedValue, testValue);
			}

			Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: Member Function One-Arg Passed\n");

			{
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push<void(ExecutionTest::Object::*)()>(&ExecutionTest::Object::Member, &testObject);
					expectedObject.Member();
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE(expectedObject.store, testObject.store);
				ASSERT_EXPECTED_VALUE(expectedValue, testValue);
			}

			Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: Member Function Zero-Arg Passed\n");

			{
				for (long i = 0; i < REPETITION_NUMBER; ++i) {
					threadpool.Push(&ExecutionTest::Object::ConstMember, &testObject, std::ref(testValue));
					expectedObject.ConstMember(expectedValue);
				}
				threadpool.Wait();
				ASSERT_EXPECTED_VALUE(expectedObject.store, testObject.store);
				ASSERT_EXPECTED_VALUE(expectedValue, testValue);
			}

			Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: Const Member Function Passed\n");
		}

		{
			for (long i = 0; i < REPETITION_NUMBER; ++i) {
				threadpool.Push((void(*)(long&))ExecutionTest::Object::OverLoadStatic, testValue);
				ExecutionTest::Object::OverLoadStatic(expectedValue);
			}
			threadpool.Wait();
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);
		}

		Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: OverLoad Static Function Stage-One Passed\n");

		{
			for (long i = 0; i < REPETITION_NUMBER; ++i) {
				threadpool.Push((void(*)(long&, long))ExecutionTest::Object::OverLoadStatic, testValue, incrementValue);
				ExecutionTest::Object::OverLoadStatic(expectedValue, incrementValue);
			}
			threadpool.Wait();
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);
		}

		Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: OverLoad Static Function Stage-Two Passed\n");

		{
			ExecutionTest::Callable expectedCallable;
			ExecutionTest::Callable testCallable;
			ASSERT_EXPECTED_VALUE(expectedCallable.store, testCallable.store);
			Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: Callable Object Stage-One Passed\n");

			for (long i = 0; i < REPETITION_NUMBER; ++i) {
				threadpool.Push(testCallable, testValue);
				expectedCallable(expectedValue);
				threadpool.Wait();
			}
			ASSERT_EXPECTED_VALUE(expectedCallable.store, testCallable.store);
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);

			Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: Callable Object Stage-Two Passed\n");

			for (long i = 0; i < REPETITION_NUMBER; ++i) {
				threadpool.Push(testCallable);
				expectedCallable();
			}
			threadpool.Wait();
			ASSERT_EXPECTED_VALUE(expectedCallable.store, testCallable.store);
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);

			Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: Callable Object Stage-Three Passed\n");

			for (long i = 0; i < REPETITION_NUMBER; ++i) {
				threadpool.Push(testCallable, std::ref(testValue));
				expectedCallable(&expectedValue);
			}
			threadpool.Wait();
			ASSERT_EXPECTED_VALUE(expectedCallable.store, testCallable.store);
			ASSERT_EXPECTED_VALUE(expectedValue, testValue);

			Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: Callable Object Stage-Four Passed\n");
		}

		Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: Callable Object Passed\n");

		Logger::WriteMessage("ThreadPoolWin32->Execution_Multiple: End\n");
	}
#undef ASSERT_EXPECTED_VALUE
#undef ASSERT_EXPECTED_STORE
	};
}
