#include "DelegateLib.h"
#include "SysData.h"
#include "SysDataNoLock.h"
#include "Timer.h"
#include <iostream>
#if USE_CPLUSPLUS_11
#include <thread>
#endif
#if USE_STD_THREADS
#include "WorkerThreadStd.h"
#elif USE_WIN32_THREADS
#include "WorkerThreadWin.h"
#endif

// main.cpp
// @see https://www.codeproject.com/Articles/1160934/Asynchronous-Multicast-Delegates-in-Cplusplus
// David Lafreniere, Dec 2016.
//
// @see https://www.codeproject.com/Articles/5262271/Remote-Procedure-Calls-using-Cplusplus-Delegates
// David Lafreniere, Mar 2020.

using namespace std;
using namespace DelegateLib;

WorkerThread workerThread1("WorkerThread1");

#if 0 // Not reliable
struct DumpLeaks
{
	~DumpLeaks() {
#ifndef USE_XALLOCATOR
		BOOL leaks = _CrtDumpMemoryLeaks();
		ASSERT_TRUE(leaks == FALSE); 
#endif
	}
};
static DumpLeaks dumpLeaks;
#endif

/// @brief Test client to get callbacks from SysData::SystemModeChangedDelgate and 
/// SysDataNoLock::SystemModeChangedDelegate
class SysDataClient
{
public:
	// Constructor
	SysDataClient() :
		m_numberOfCallbacks(0)
	{
		// Register for async delegate callbacks
		SysData::GetInstance().SystemModeChangedDelegate += MakeDelegate(this, &SysDataClient::CallbackFunction, &workerThread1);
		SysDataNoLock::GetInstance().SystemModeChangedDelegate += MakeDelegate(this, &SysDataClient::CallbackFunction, &workerThread1);
	}

	~SysDataClient()
	{
		// Unregister the all registered delegates at once
		SysData::GetInstance().SystemModeChangedDelegate.Clear(); 

		// Alternatively unregister a single delegate
		SysDataNoLock::GetInstance().SystemModeChangedDelegate -= MakeDelegate(this, &SysDataClient::CallbackFunction, &workerThread1);
	}

private:
	void CallbackFunction(const SystemModeChanged& data)
	{
		m_numberOfCallbacks++;
		cout << "CallbackFunction " << data.CurrentSystemMode << endl;
	}

	int m_numberOfCallbacks;
};

struct TestStruct
{
	int x;
};

struct TestStructNoCopy
{
	TestStructNoCopy(int _x) { x = _x;  }
	int x;

private:
	// Prevent copying objects
	TestStructNoCopy(const TestStructNoCopy&);
	TestStructNoCopy& operator=(const TestStructNoCopy&);
};

void FreeFunc()
{
	cout << "FreeFunc" << endl;
}

void FreeFuncInt(int value)
{
	cout << "FreeFuncInt " << value << endl;
}

int FreeFuncRetInt()
{
	cout << "FreeFuncRetInt " << endl;
	return 567;
}

int FreeFuncIntRetInt(int value)
{
	cout << "FreeFuncIntRetInt " << value << endl;
	return value;
}

void FreeFuncPtrTestStruct(TestStruct* value)
{
	cout << "FreeFuncTestStruct" << value->x << endl;
}

void FreeFuncPtrPtrTestStruct(TestStruct** value)
{
	cout << "FreeFuncPtrPtrTestStruct " << (*value)->x << endl;
}

void FreeFuncRefTestStruct(const TestStruct& value)
{
	cout << "FreeFuncRefTestStruct " << value.x << endl;
}

class TestClass
{
public:
	~TestClass()
	{
	}

	void MemberFunc(TestStruct* value)
	{
		cout << "MemberFunc " << value->x << endl;
	}

	void MemberFuncThreeArgs(const TestStruct& value, float f, int** i)
	{
		cout << "MemberFuncThreeArgs " << value.x << " " << f << " " << (**i) << endl;
	}

	void MemberFuncNoCopy(TestStructNoCopy* value)
	{
		cout << "MemberFuncNoCopy " << value->x << endl;
	}

	void MemberFuncStdString(const std::string& s, int year)
	{
		cout << "MemberFuncStdString " << s.c_str() << " " << year << endl;
	}

	int MemberFuncStdStringRetInt(std::string& s)
	{
		s = "Hello world";
		return 2016;
	}

	static void StaticFunc(TestStruct* value)
	{
		cout << "StaticFunc " << value->x << endl;
	}


	int TestFunc()
	{
		cout << "TestFunc " << endl;
		return 987;
	}
	void TestFuncNoRet()
	{
		cout << "TestFuncNoRet " << endl;
	}
};

// Class template specialization "tells" the delegate library that TestStructNoCopy* function 
// arguments not to create a heap copy of TestStructNoCopy for traveling through the
// message queue. Instead the pointer argument is passed to the asynchronously invoked callback 
// function as is. 
namespace DelegateLib
{
	template <>
	class DelegateParam<TestStructNoCopy *>
	{
	public:
		static TestStructNoCopy* New(TestStructNoCopy* param) { return param; }
		static void Delete(TestStructNoCopy* param) {}
	};
}

// An instance of TestStructNoCopy guaranteed to exist when the asynchronous callback occurs.  
static TestStructNoCopy testStructNoCopy(999);

void TimerExpiredCb(void)
{
    static int count = 0;
    cout << "TimerExpiredCb " << count++ << endl;
}

extern void DelegateUnitTests();

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main(void)
{
	TestStruct testStruct;
	testStruct.x = 123;
	TestStruct* pTestStruct = &testStruct;

	TestClass testClass;

	// Create the worker threads
	workerThread1.CreateThread();
	SysDataNoLock::GetInstance();

#if USE_WIN32_THREADS
	// Start the worker threads
	ThreadWin::StartAllThreads();
#endif

    // Create a timer that expires every 250mS and calls 
    // TimerExpiredCb on workerThread1 upon expiration
    Timer timer;
    timer.Expired = MakeDelegate(&TimerExpiredCb, &workerThread1);
    timer.Start(250);

	// Run all unit tests (uncomment to run unit tests)
	DelegateUnitTests();

	// Create a delegate bound to a free function then invoke
	DelegateFree1<int> delegateFree = MakeDelegate(&FreeFuncInt);
	delegateFree(123);

	// Create a delegate bound to a member function then invoke
	DelegateMember1<TestClass, TestStruct*> delegateMember = MakeDelegate(&testClass, &TestClass::MemberFunc);
	delegateMember(&testStruct);

	// Create a delegate bound to a member function. Assign and invoke from
	// a base reference. 
	DelegateMember1<TestClass, TestStruct*> delegateMember2 = MakeDelegate(&testClass, &TestClass::MemberFunc);
	delegateMember2(&testStruct);

	// Create a multicast delegate container that accepts Delegate1<int> delegates.
	// Any function with the signature "void Func(int)".
	MulticastDelegate1<int> delegateA;

	// Add a DelegateFree1<int> delegate to the container 
	delegateA += MakeDelegate(&FreeFuncInt);

	// Invoke the delegate target free function FreeFuncInt()
	if (delegateA)
		delegateA(123);

	// Remove the delegate from the container
	delegateA -= MakeDelegate(&FreeFuncInt);

	// Create a multicast delegate container that accepts Delegate<TestStruct*> delegates
	// Any function with the signature "void Func(TestStruct*)".
	MulticastDelegate1<TestStruct*> delegateB;

	// Add a DelegateMember1<TestStruct*> delegate to the container
	delegateB += MakeDelegate(&testClass, &TestClass::MemberFunc);

	// Invoke the delegate target member function TestClass::MemberFunc()
	if (delegateB)
		delegateB(&testStruct);

	// Remove the delegate from the container
	delegateB -= MakeDelegate(&testClass, &TestClass::MemberFunc);

	// Create a thread-safe multicast delegate container that accepts Delegate<TestStruct*> delegates
	// Any function with the signature "void Func(TestStruct*)".
	MulticastDelegateSafe1<TestStruct*> delegateC;

	// Add a DelegateMember1<TestStruct*> delegate to the container that will invoke on workerThread1
	delegateC += MakeDelegate(&testClass, &TestClass::MemberFunc, &workerThread1);

	// Asynchronously invoke the delegate target member function TestClass::MemberFunc()
	if (delegateC)
		delegateC(&testStruct);

	// Remove the delegate from the container
	delegateC -= MakeDelegate(&testClass, &TestClass::MemberFunc, &workerThread1);

	// Create a thread-safe multicast delegate container that accepts Delegate<TestStruct&, float, int**> delegates
	// Any function with the signature "void Func(const TestStruct&, float, int**)".
	MulticastDelegateSafe3<const TestStruct&, float, int**> delegateD;

	// Add a DelegateMember1<TestStruct*> delegate to the container that will invoke on workerThread1
	delegateD += MakeDelegate(&testClass, &TestClass::MemberFuncThreeArgs, &workerThread1);

	// Asynchronously invoke the delegate target member function TestClass::MemberFuncThreeArgs()
	if (delegateD)
	{
		int i = 555;
		int* pI = &i;
		delegateD(testStruct, 1.23f, &pI);
	}

	// Remove the delegate from the container
	delegateD -= MakeDelegate(&testClass, &TestClass::MemberFuncThreeArgs, &workerThread1);

	// Create a thread-safe multicast delegate container that accepts Delegate<TestStructNoCopy*> delegates
	// Any function with the signature "void Func(TestStructNoCopy*)".
	MulticastDelegateSafe1<TestStructNoCopy*> delegateE;

	// Add a DelegateMember1<TestStructNoCopy*> delegate to the container that will invoke on workerThread1
	delegateE += MakeDelegate(&testClass, &TestClass::MemberFuncNoCopy, &workerThread1);

	// Asynchronously invoke the delegate target member function TestClass::MemberFuncNoCopy().
	// TestStructNoCopy will not be copied and created on the heap because of DelegateParam<TestStructNoCopy *>. 
	// Developer must ensure the testStructNoCopy instance exists when the asynchronous callback occurs. 
	if (delegateE)
		delegateE(&testStructNoCopy);

	// Remove the delegate from the container
	delegateE -= MakeDelegate(&testClass, &TestClass::MemberFuncNoCopy, &workerThread1);

	// Create a singlecast delegate container that accepts Delegate1<int, int> delegates.
	// Any function with the signature "int Func(int)".
	SinglecastDelegate1<int, int> delegateF;

	// Add a DelegateFree1<int, int> delegate to the container 
	delegateF = MakeDelegate(&FreeFuncIntRetInt);

	// Invoke the delegate target free function FreeFuncInt()
	int retVal = 0;
	if (delegateE)
		retVal = delegateF(123);

	// Remove the delegate from the container
	delegateF.Clear();

	// Create a singlecast delegate container that accepts delegates with 
	// the singature "void Func(TestStruct**)"
	SinglecastDelegate1<TestStruct**> delegateG;

	// Make a delegate that points to a free function 
	delegateG = MakeDelegate(&FreeFuncPtrPtrTestStruct);

	// Invoke the delegate target function FreeFuncPtrPtrTestStruct()
	delegateG(&pTestStruct);

	// Remove the delegate from the container
	delegateG = 0;

	// Create delegate with std::string and int arguments then asychronously 
	// invoke on a member function
	MulticastDelegateSafe2<const std::string&, int> delegateH;
	delegateH += MakeDelegate(&testClass, &TestClass::MemberFuncStdString, &workerThread1);
	delegateH("Hello world", 2016);
	delegateH.Clear();

	// Create a asynchronous blocking delegate and invoke. This thread will block until the 
	// msg and year stack values are set by MemberFuncStdStringRetInt on workerThread1.
#if USE_CPLUSPLUS_11
	auto delegateI = MakeDelegate(&testClass, &TestClass::MemberFuncStdStringRetInt, &workerThread1, WAIT_INFINITE);
#else
	DelegateMemberAsyncWait1<TestClass, std::string&, int> delegateI =
		MakeDelegate(&testClass, &TestClass::MemberFuncStdStringRetInt, &workerThread1, WAIT_INFINITE);
#endif
	std::string msg;
	int year = delegateI(msg);
	if (delegateI.IsSuccess())
		cout << msg.c_str() << " " << year << endl;

#if USE_CPLUSPLUS_11
	// Create a shared_ptr, create a delegate, then synchronously invoke delegate function
	std::shared_ptr<TestClass> spObject(new TestClass());
	auto delegateMemberSp = MakeDelegate(spObject, &TestClass::MemberFuncStdString);
	delegateMemberSp("Hello world using shared_ptr", 2016);

	// Example of a bug where the testClassHeap is deleted before the asychronous delegate 
	// is invoked on the workerThread1. In other words, by the time workerThread1 calls
	// the bound delegate function the testClassHeap instance is deleted and no longer valid.
	TestClass* testClassHeap = new TestClass();
	auto delegateMemberAsync = MakeDelegate(testClassHeap, &TestClass::MemberFuncStdString, &workerThread1);
	delegateMemberAsync("Function async invoked on deleted object. Bug!", 2016);
	delegateMemberAsync.Clear();
	delete testClassHeap;

	// Example of the smart pointer function version of the delegate. The testClassSp instance 
	// is only deleted after workerThread1 invokes the callback function thus solving the bug.
	std::shared_ptr<TestClass> testClassSp(new TestClass());
	auto delegateMemberSpAsync = MakeDelegate(testClassSp, &TestClass::MemberFuncStdString, &workerThread1);
	delegateMemberSpAsync("Function async invoked using smart pointer. Bug solved!", 2016);
	delegateMemberSpAsync.Clear();
	testClassSp.reset();
#endif

	// Create a SysDataClient instance on the stack
	SysDataClient sysDataClient;

	// Set new SystemMode values. Each call will invoke callbacks to all 
	// registered client subscribers.
	SysData::GetInstance().SetSystemMode(SystemMode::STARTING);
	SysData::GetInstance().SetSystemMode(SystemMode::NORMAL);

	// Set new SystemMode values for SysDataNoLock.
	SysDataNoLock::GetInstance().SetSystemMode(SystemMode::SERVICE);
	SysDataNoLock::GetInstance().SetSystemMode(SystemMode::SYS_INOP);

	// Set new SystemMode values for SysDataNoLock using async API
	SysDataNoLock::GetInstance().SetSystemModeAsyncAPI(SystemMode::SERVICE);
	SysDataNoLock::GetInstance().SetSystemModeAsyncAPI(SystemMode::SYS_INOP);

	// Set new SystemMode values for SysDataNoLock using async wait API
	SystemMode::Type previousMode;
	previousMode = SysDataNoLock::GetInstance().SetSystemModeAsyncWaitAPI(SystemMode::STARTING);
	previousMode = SysDataNoLock::GetInstance().SetSystemModeAsyncWaitAPI(SystemMode::NORMAL);

    timer.Stop();
    timer.Expired.Clear();

#ifdef USE_CPLUSPLUS_11
    std::this_thread::sleep_for(std::chrono::seconds(1));
#endif

	workerThread1.ExitThread();

#ifdef USE_CPLUSPLUS_11
    std::this_thread::sleep_for(std::chrono::seconds(1));
#endif

	return 0;
}

