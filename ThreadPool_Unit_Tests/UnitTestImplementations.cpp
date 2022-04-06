#include "pch.h"
#include "UnitTestImplementations.hpp"

#pragma region ConstructorTests
	void ConstructorTest::Function() {

	}

	void ConstructorTest::OverloadFunction() {

	}

	void ConstructorTest::OverloadFunction(long x) {

	}
#pragma endregion

#pragma region ExecutionTests
	void ExecutionTest::Function(long& x) {
		_InterlockedIncrement(&x);
	}

	void ExecutionTest::OverloadFunction(long& x) {
		_InterlockedIncrement(&x);
	}

	void ExecutionTest::OverloadFunction(long& x, long changeValue) {
		//while (changeValue) {
		//	_InterlockedIncrement(&x);
		//	--changeValue;
		//}
		_InterlockedExchangeAdd(&x, changeValue);
	}
#pragma endregion