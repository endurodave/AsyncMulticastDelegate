#ifndef _DELEGATE_ASYNC_WAIT_H
#define _DELEGATE_ASYNC_WAIT_H

// DelegateAsyncWait.h
// @see https://www.codeproject.com/Articles/1160934/Asynchronous-Multicast-Delegates-in-Cplusplus
// David Lafreniere, Dec 2016.

#include "Delegate.h"
#include "DelegateThread.h"
#include "DelegateInvoker.h"
#include "Semaphore.h"

/// @brief Asynchronous member delegate that invokes the target function on the specified thread of control
/// and waits for the function to be executed or a timeout occurs. Use IsSuccess() to determine if asynchronous 
/// call succeeded.

namespace DelegateLib {

const int WAIT_INFINITE = -1;

// N=0 abstract base
template <class TClass, class RetType=void> 
class DelegateMemberAsyncWaitBase0 : public DelegateMember0<TClass, RetType>, public IDelegateInvoker {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)();
	typedef RetType (TClass::*ConstMemberFunc)() const;
	
	// Contructors take a class instance, member function, and delegate thread
	DelegateMemberAsyncWaitBase0(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(object, func, thread); 
		LockGuard::Create(&m_lock);
	}
	DelegateMemberAsyncWaitBase0(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(object, func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateMemberAsyncWaitBase0(const DelegateMemberAsyncWaitBase0& rhs) : DelegateMember0<TClass, RetType>(rhs), m_refCnt(0) {
		LockGuard::Create(&m_lock);
		Swap(rhs);
	}
	DelegateMemberAsyncWaitBase0() : m_thread(0), m_success(false), m_timeout(0), m_refCnt(0) { LockGuard::Create(&m_lock); }
	virtual ~DelegateMemberAsyncWaitBase0() { LockGuard::Destroy(&m_lock); }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateMember0<TClass, RetType>::Bind(object, func); }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)	{
		m_thread = thread;
		DelegateMember0<TClass, RetType>::Bind(object, func); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberAsyncWaitBase0<TClass, RetType>* derivedRhs = dynamic_cast<const DelegateMemberAsyncWaitBase0<TClass, RetType>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread && 
			DelegateMember0<TClass, RetType>::operator==(rhs); }

	DelegateMemberAsyncWaitBase0& operator=(const DelegateMemberAsyncWaitBase0& rhs) {
		if (&rhs != this) {
			DelegateMember0<TClass, RetType>::operator=(rhs);
			Swap(rhs);
		}
		return *this;
	}

	/// Returns true if asynchronous function successfully invoked on target thread
	bool IsSuccess() { return m_success; }	

protected:
	DelegateThread* m_thread;		// Target thread to invoke the delegate function
	bool m_success;					// Set to true if async function succeeds
	int m_timeout;					// Time in mS to wait for async function to invoke
	Semaphore m_sema;				// Semaphore to signal waiting thread
	LOCK m_lock;					// Lock to synchronize threads
	int m_refCnt;					// Ref count to determine when to delete objects

private:
	void Swap(const DelegateMemberAsyncWaitBase0& s) {
		m_thread = s.m_thread;
		m_timeout = s.m_timeout;
		m_success = s.m_success;
	}
};

// N=0 with return type
template <class TClass, class RetType = void>
class DelegateMemberAsyncWait0 : public DelegateMemberAsyncWaitBase0<TClass, RetType> {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)();
	typedef RetType (TClass::*ConstMemberFunc)() const;
	
	DelegateMemberAsyncWait0(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) : 
		DelegateMemberAsyncWaitBase0<TClass, RetType>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait0(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) : 
		DelegateMemberAsyncWaitBase0<TClass, RetType>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait0() { }
	virtual DelegateMemberAsyncWait0<TClass, RetType>* Clone() const {	
		return new DelegateMemberAsyncWait0<TClass, RetType>(*this); }
	
	/// Invoke delegate function asynchronously
	virtual RetType operator()() {
		if (this->m_thread == 0)
			return DelegateMemberAsyncWaitBase0<TClass, RetType>::operator()();
		else {
			// Create a clone instance of this delegate 
			DelegateMemberAsyncWait0<TClass, RetType>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsgBase* msg = new DelegateMsgBase(delegate);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout)))
				m_retVal = delegate->m_retVal;

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
			return m_retVal;
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				m_retVal = DelegateMemberAsyncWaitBase0<TClass, RetType>::operator()();
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateMemberAsyncWait0& operator=(const DelegateMemberAsyncWait0& rhs) {
		DelegateMemberAsyncWaitBase0<TClass, RetType>::operator=(rhs);
		return *this;
	}

	RetType GetRetVal() { return m_retVal; }
private:
	RetType m_retVal;				// The delegate return value
};

// N=0 void return type
template <class TClass>
class DelegateMemberAsyncWait0<TClass, void> : public DelegateMemberAsyncWaitBase0<TClass, void> {
public:
	typedef TClass* ObjectPtr;
	typedef void (TClass::*MemberFunc)();
	typedef void (TClass::*ConstMemberFunc)() const;
	
	DelegateMemberAsyncWait0(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase0<TClass>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait0(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase0<TClass>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait0() { }
	virtual DelegateMemberAsyncWait0<TClass>* Clone() const {
		return new DelegateMemberAsyncWait0<TClass>(*this);	}

	/// Invoke delegate function asynchronously
	virtual void operator()() {
		if (this->m_thread == 0)
			DelegateMemberAsyncWaitBase0<TClass>::operator()();
		else {
			// Create a clone instance of this delegate 
			DelegateMemberAsyncWait0<TClass>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsgBase* msg = new DelegateMsgBase(delegate);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout))) {
				// No return or param arguments
			}

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				DelegateMemberAsyncWaitBase0<TClass>::operator()();
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateMemberAsyncWait0& operator=(const DelegateMemberAsyncWait0& rhs) {
		DelegateMemberAsyncWaitBase0<TClass>::operator=(rhs);
		return *this;
	}
};

// N=1 abstract base
template <class TClass, class Param1, class RetType = void>
class DelegateMemberAsyncWaitBase1 : public DelegateMember1<TClass, Param1, RetType>, public IDelegateInvoker {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1);
	typedef RetType (TClass::*ConstMemberFunc)(Param1) const;
	
	// Contructors take a class instance, member function, and delegate thread
	DelegateMemberAsyncWaitBase1(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(object, func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateMemberAsyncWaitBase1(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(object, func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateMemberAsyncWaitBase1(const DelegateMemberAsyncWaitBase1& rhs) : DelegateMember1<TClass, Param1, RetType>(rhs), m_refCnt(0) {
		LockGuard::Create(&m_lock);
		Swap(rhs);
	}
	DelegateMemberAsyncWaitBase1() : m_thread(0), m_success(false), m_timeout(0), m_refCnt(0) { LockGuard::Create(&m_lock); }
	virtual ~DelegateMemberAsyncWaitBase1() { LockGuard::Destroy(&m_lock); }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateMember1<TClass, Param1, RetType>::Bind(object, func);
	}

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateMember1<TClass, Param1, RetType>::Bind(object, func);
	}

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateMemberAsyncWaitBase1<TClass, Param1, RetType>* derivedRhs = dynamic_cast<const DelegateMemberAsyncWaitBase1<TClass, Param1, RetType>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateMember1<TClass, Param1, RetType>::operator==(rhs);
	}

	DelegateMemberAsyncWaitBase1& operator=(const DelegateMemberAsyncWaitBase1& rhs) {
		if (&rhs != this) {
			DelegateMember1<TClass, Param1, RetType>::operator=(rhs);
			Swap(rhs);
		}
		return *this;
	}

	/// Returns true if asynchronous function successfully invoked on target thread
	bool IsSuccess() { return m_success; }

protected:
	DelegateThread* m_thread;		// Target thread to invoke the delegate function
	bool m_success;					// Set to true if async function succeeds
	int m_timeout;					// Time in mS to wait for async function to invoke
	Semaphore m_sema;				// Semaphore to signal waiting thread
	LOCK m_lock;					// Lock to synchronize threads
	int m_refCnt;					// Ref count to determine when to delete objects

private:
	void Swap(const DelegateMemberAsyncWaitBase1& s) {
		m_thread = s.m_thread;
		m_timeout = s.m_timeout;
		m_success = s.m_success;
	}
};

// N=1 with return type 
template <class TClass, class Param1, class RetType = void>
class DelegateMemberAsyncWait1 : public DelegateMemberAsyncWaitBase1<TClass, Param1, RetType> {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1);
	typedef RetType (TClass::*ConstMemberFunc)(Param1) const;
	
	DelegateMemberAsyncWait1(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase1<TClass, Param1, RetType>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait1<TClass, Param1, RetType>(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase1<TClass, Param1, RetType>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait1() { }
	virtual DelegateMemberAsyncWait1<TClass, Param1, RetType>* Clone() const {
		return new DelegateMemberAsyncWait1<TClass, Param1, RetType>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual RetType operator()(Param1 p1) {
		if (this->m_thread == 0)
			return DelegateMemberAsyncWaitBase1<TClass, Param1, RetType>::operator()(p1);
		else {
			// Create a clone instance of this delegate 
			DelegateMemberAsyncWait1<TClass, Param1, RetType>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg1<Param1>* msg = new DelegateMsg1<Param1>(delegate, p1);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout)))
				m_retVal = delegate->m_retVal;

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
			return m_retVal;
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg1<Param1>* delegateMsg = static_cast<DelegateMsg1<Param1>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				m_retVal = DelegateMemberAsyncWaitBase1<TClass, Param1, RetType>::operator()(param1);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateMemberAsyncWait1& operator=(const DelegateMemberAsyncWait1& rhs) {
		DelegateMemberAsyncWaitBase1<TClass, Param1, RetType>::operator=(rhs);
		return *this;
	}

	RetType GetRetVal() { return m_retVal; }
private:
	RetType m_retVal;				// The delegate return value
};

// N=1 void return type
template <class TClass, class Param1>
class DelegateMemberAsyncWait1<TClass, Param1, void> : public DelegateMemberAsyncWaitBase1<TClass, Param1, void> {
public:
	typedef TClass* ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1);
	typedef void (TClass::*ConstMemberFunc)(Param1) const;
	
	DelegateMemberAsyncWait1(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase1<TClass, Param1>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait1(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase1<TClass, Param1>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait1() { }
	virtual DelegateMemberAsyncWait1<TClass, Param1>* Clone() const {
		return new DelegateMemberAsyncWait1<TClass, Param1>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1) {
		if (this->m_thread == 0)
			DelegateMemberAsyncWaitBase1<TClass, Param1>::operator()(p1);
		else {
			// Create a clone instance of this delegate 
			DelegateMemberAsyncWait1<TClass, Param1>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg1<Param1>* msg = new DelegateMsg1<Param1>(delegate, p1);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			this->m_success = delegate->m_sema.Wait(this->m_timeout);

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg1<Param1>* delegateMsg = static_cast<DelegateMsg1<Param1>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				DelegateMemberAsyncWaitBase1<TClass, Param1>::operator()(param1);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateMemberAsyncWait1& operator=(const DelegateMemberAsyncWait1& rhs) {
		DelegateMemberAsyncWaitBase1<TClass, Param1>::operator=(rhs);
		return *this;
	}
};

// N=2 abstract base
template <class TClass, class Param1, class Param2, class RetType = void>
class DelegateMemberAsyncWaitBase2 : public DelegateMember2<TClass, Param1, Param2, RetType>, public IDelegateInvoker {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2);
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2) const;
	
	// Contructors take a class instance, member function, and delegate thread
	DelegateMemberAsyncWaitBase2(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(object, func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateMemberAsyncWaitBase2(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(object, func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateMemberAsyncWaitBase2(const DelegateMemberAsyncWaitBase2& rhs) : DelegateMember2<TClass, Param1, Param2, RetType>(rhs), m_refCnt(0) {
		LockGuard::Create(&m_lock);
		Swap(rhs);
	}
	DelegateMemberAsyncWaitBase2() : m_thread(0), m_success(false), m_timeout(0), m_refCnt(0) { LockGuard::Create(&m_lock); }
	virtual ~DelegateMemberAsyncWaitBase2() { LockGuard::Destroy(&m_lock); }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateMember2<TClass, Param1, Param2, RetType>::Bind(object, func);
	}

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateMember2<TClass, Param1, Param2, RetType>::Bind(object, func);
	}

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateMemberAsyncWaitBase2<TClass, Param1, Param2, RetType>* derivedRhs = dynamic_cast<const DelegateMemberAsyncWaitBase2<TClass, Param1, Param2, RetType>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateMember2<TClass, Param1, Param2, RetType>::operator==(rhs);
	}

	DelegateMemberAsyncWaitBase2& operator=(const DelegateMemberAsyncWaitBase2& rhs) {
		if (&rhs != this) {
			DelegateMember2<TClass, Param1, Param2, RetType>::operator=(rhs);
			Swap(rhs);
		}
		return *this;
	}

	/// Returns true if asynchronous function successfully invoked on target thread
	bool IsSuccess() { return m_success; }

protected:
	DelegateThread* m_thread;		// Target thread to invoke the delegate function
	bool m_success;					// Set to true if async function succeeds
	int m_timeout;					// Time in mS to wait for async function to invoke
	Semaphore m_sema;				// Semaphore to signal waiting thread
	LOCK m_lock;					// Lock to synchronize threads
	int m_refCnt;					// Ref count to determine when to delete objects

private:
	void Swap(const DelegateMemberAsyncWaitBase2& s) {
		m_thread = s.m_thread;
		m_timeout = s.m_timeout;
		m_success = s.m_success;
	}
};

// N=2 with return type 
template <class TClass, class Param1, class Param2, class RetType = void>
class DelegateMemberAsyncWait2 : public DelegateMemberAsyncWaitBase2<TClass, Param1, Param2, RetType> {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2);
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2) const;
	
	DelegateMemberAsyncWait2(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase2<TClass, Param1, Param2, RetType>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait2(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase2<TClass, Param1, Param2, RetType>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait2() { }
	virtual DelegateMemberAsyncWait2<TClass, Param1, Param2, RetType>* Clone() const {
		return new DelegateMemberAsyncWait2<TClass, Param1, Param2, RetType>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual RetType operator()(Param1 p1, Param2 p2) {
		if (this->m_thread == 0)
			return DelegateMemberAsyncWaitBase2<TClass, Param1, Param2, RetType>::operator()(p1, p2);
		else {
			// Create a clone instance of this delegate 
			DelegateMemberAsyncWait2<TClass, Param1, Param2, RetType>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg2<Param1, Param2>* msg = new DelegateMsg2<Param1, Param2>(delegate, p1, p2);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout)))
				m_retVal = delegate->m_retVal;

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
			return m_retVal;
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg2<Param1, Param2>* delegateMsg = static_cast<DelegateMsg2<Param1, Param2>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				m_retVal = DelegateMemberAsyncWaitBase2<TClass, Param1, Param2, RetType>::operator()(param1, param2);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateMemberAsyncWait2& operator=(const DelegateMemberAsyncWait2& rhs) {
		DelegateMemberAsyncWaitBase2<TClass, Param1, Param2, RetType>::operator=(rhs);
		return *this;
	}

	RetType GetRetVal() { return m_retVal; }
private:
	RetType m_retVal;				// The delegate return value
};

// N=2 void return type
template <class TClass, class Param1, class Param2>
class DelegateMemberAsyncWait2<TClass, Param1, Param2, void> : public DelegateMemberAsyncWaitBase2<TClass, Param1, Param2, void> {
public:
	typedef TClass* ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1, Param2);
	typedef void (TClass::*ConstMemberFunc)(Param1, Param2) const;
	
	DelegateMemberAsyncWait2(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase2<TClass, Param1, Param2>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait2(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase2<TClass, Param1, Param2>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait2() { }
	virtual DelegateMemberAsyncWait2<TClass, Param1, Param2>* Clone() const {
		return new DelegateMemberAsyncWait2<TClass, Param1, Param2>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2) {
		if (this->m_thread == 0)
			DelegateMemberAsyncWaitBase2<TClass, Param1, Param2>::operator()(p1, p2);
		else {
			// Create a clone instance of this delegate 
			DelegateMemberAsyncWait2<TClass, Param1, Param2>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg2<Param1, Param2>* msg = new DelegateMsg2<Param1, Param2>(delegate, p1, p2);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			this->m_success = delegate->m_sema.Wait(this->m_timeout);

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg2<Param1, Param2>* delegateMsg = static_cast<DelegateMsg2<Param1, Param2>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				DelegateMemberAsyncWaitBase2<TClass, Param1, Param2>::operator()(param1, param2);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateMemberAsyncWait2& operator=(const DelegateMemberAsyncWait2& rhs) {
		DelegateMemberAsyncWaitBase2<TClass, Param1, Param2>::operator=(rhs);
		return *this;
	}
};

// N=3 abstract base
template <class TClass, class Param1, class Param2, class Param3, class RetType = void>
class DelegateMemberAsyncWaitBase3 : public DelegateMember3<TClass, Param1, Param2, Param3, RetType>, public IDelegateInvoker {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2, Param3);
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2, Param3) const;
	
	// Contructors take a class instance, member function, and delegate thread
	DelegateMemberAsyncWaitBase3(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(object, func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateMemberAsyncWaitBase3(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(object, func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateMemberAsyncWaitBase3(const DelegateMemberAsyncWaitBase3& rhs) : DelegateMember3<TClass, Param1, Param2, Param3, RetType>(rhs), m_refCnt(0) {
		LockGuard::Create(&m_lock);
		Swap(rhs);
	}
	DelegateMemberAsyncWaitBase3() : m_thread(0), m_success(false), m_timeout(0), m_refCnt(0) { LockGuard::Create(&m_lock); }
	virtual ~DelegateMemberAsyncWaitBase3() { LockGuard::Destroy(&m_lock); }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateMember3<TClass, Param1, Param2, Param3, RetType>::Bind(object, func);
	}

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateMember3<TClass, Param1, Param2, Param3, RetType>::Bind(object, func);
	}

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3, RetType>* derivedRhs = dynamic_cast<const DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3, RetType>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateMember3<TClass, Param1, Param2, Param3, RetType>::operator==(rhs);
	}

	DelegateMemberAsyncWaitBase3& operator=(const DelegateMemberAsyncWaitBase3& rhs) {
		if (&rhs != this) {
			DelegateMember3<TClass, Param1, Param2, Param3, RetType>::operator=(rhs);
			Swap(rhs);
		}
		return *this;
	}

	/// Returns true if asynchronous function successfully invoked on target thread
	bool IsSuccess() { return m_success; }

protected:
	DelegateThread* m_thread;		// Target thread to invoke the delegate function
	bool m_success;					// Set to true if async function succeeds
	int m_timeout;					// Time in mS to wait for async function to invoke
	Semaphore m_sema;				// Semaphore to signal waiting thread
	LOCK m_lock;					// Lock to synchronize threads
	int m_refCnt;					// Ref count to determine when to delete objects

private:
	void Swap(const DelegateMemberAsyncWaitBase3& s) {
		m_thread = s.m_thread;
		m_timeout = s.m_timeout;
		m_success = s.m_success;
	}
};

// N=3 with return type 
template <class TClass, class Param1, class Param2, class Param3, class RetType = void>
class DelegateMemberAsyncWait3 : public DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3, RetType> {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2, Param3);
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2, Param3) const;
	
	DelegateMemberAsyncWait3(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3, RetType>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait3(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3, RetType>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait3() { }
	virtual DelegateMemberAsyncWait3<TClass, Param1, Param2, Param3, RetType>* Clone() const {
		return new DelegateMemberAsyncWait3<TClass, Param1, Param2, Param3, RetType>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3) {
		if (this->m_thread == 0)
			return DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3, RetType>::operator()(p1, p2, p3);
		else {
			// Create a clone instance of this delegate 
			DelegateMemberAsyncWait3<TClass, Param1, Param2, Param3, RetType>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg3<Param1, Param2, Param3>* msg = new DelegateMsg3<Param1, Param2, Param3>(delegate, p1, p2, p3);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout)))
				m_retVal = delegate->m_retVal;

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
			return m_retVal;
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg3<Param1, Param2, Param3>* delegateMsg = static_cast<DelegateMsg3<Param1, Param2, Param3>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();
			Param3 param3 = delegateMsg->GetParam3();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				m_retVal = DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3, RetType>::operator()(param1, param2, param3);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateMemberAsyncWait3& operator=(const DelegateMemberAsyncWait3& rhs) {
		DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3, RetType>::operator=(rhs);
		return *this;
	}

	RetType GetRetVal() { return m_retVal; }
private:
	RetType m_retVal;				// The delegate return value
};

// N=3 void return type
template <class TClass, class Param1, class Param2, class Param3>
class DelegateMemberAsyncWait3<TClass, Param1, Param2, Param3, void> : public DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3, void> {
public:
	typedef TClass* ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1, Param2, Param3);
	typedef void (TClass::*ConstMemberFunc)(Param1, Param2, Param3) const;
	
	DelegateMemberAsyncWait3(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait3(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait3() { }
	virtual DelegateMemberAsyncWait3<TClass, Param1, Param2, Param3>* Clone() const {
		return new DelegateMemberAsyncWait3<TClass, Param1, Param2, Param3>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3) {
		if (this->m_thread == 0)
			DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3>::operator()(p1, p2, p3);
		else {
			// Create a clone instance of this delegate 
			DelegateMemberAsyncWait3<TClass, Param1, Param2, Param3>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg3<Param1, Param2, Param3>* msg = new DelegateMsg3<Param1, Param2, Param3>(delegate, p1, p2, p3);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			this->m_success = delegate->m_sema.Wait(this->m_timeout);

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg3<Param1, Param2, Param3>* delegateMsg = static_cast<DelegateMsg3<Param1, Param2, Param3>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();
			Param3 param3 = delegateMsg->GetParam3();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3>::operator()(param1, param2, param3);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateMemberAsyncWait3& operator=(const DelegateMemberAsyncWait3& rhs) {
		DelegateMemberAsyncWaitBase3<TClass, Param1, Param2, Param3>::operator=(rhs);
		return *this;
	}
};

// N=4 abstract base
template <class TClass, class Param1, class Param2, class Param3, class Param4, class RetType = void>
class DelegateMemberAsyncWaitBase4 : public DelegateMember4<TClass, Param1, Param2, Param3, Param4, RetType>, public IDelegateInvoker {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2, Param3, Param4);
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4) const;

	// Contructors take a class instance, member function, and delegate thread
	DelegateMemberAsyncWaitBase4(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(object, func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateMemberAsyncWaitBase4(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(object, func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateMemberAsyncWaitBase4(const DelegateMemberAsyncWaitBase4& rhs) : DelegateMember4<TClass, Param1, Param2, Param3, Param4, RetType>(rhs), m_refCnt(0) {
		LockGuard::Create(&m_lock);
		Swap(rhs);
	}
	DelegateMemberAsyncWaitBase4() : m_thread(0), m_success(false), m_timeout(0), m_refCnt(0) { LockGuard::Create(&m_lock); }
	virtual ~DelegateMemberAsyncWaitBase4() { LockGuard::Destroy(&m_lock); }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateMember4<TClass, Param1, Param2, Param3, Param4, RetType>::Bind(object, func);
	}

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateMember4<TClass, Param1, Param2, Param3, Param4, RetType>::Bind(object, func);
	}

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4, RetType>* derivedRhs = dynamic_cast<const DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4, RetType>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateMember4<TClass, Param1, Param2, Param3, Param4, RetType>::operator==(rhs);
	}

	DelegateMemberAsyncWaitBase4& operator=(const DelegateMemberAsyncWaitBase4& rhs) {
		if (&rhs != this) {
			DelegateMember4<TClass, Param1, Param2, Param3, Param4, RetType>::operator=(rhs);
			Swap(rhs);
		}
		return *this;
	}

	/// Returns true if asynchronous function successfully invoked on target thread
	bool IsSuccess() { return m_success; }

protected:
	DelegateThread* m_thread;		// Target thread to invoke the delegate function
	bool m_success;					// Set to true if async function succeeds
	int m_timeout;					// Time in mS to wait for async function to invoke
	Semaphore m_sema;				// Semaphore to signal waiting thread
	LOCK m_lock;					// Lock to synchronize threads
	int m_refCnt;					// Ref count to determine when to delete objects

private:
	void Swap(const DelegateMemberAsyncWaitBase4& s) {
		m_thread = s.m_thread;
		m_timeout = s.m_timeout;
		m_success = s.m_success;
	}
};

// N=4 with return type 
template <class TClass, class Param1, class Param2, class Param3, class Param4, class RetType = void>
class DelegateMemberAsyncWait4 : public DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4, RetType> {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2, Param3, Param4);
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4) const;
	
	DelegateMemberAsyncWait4(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4, RetType>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait4(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4, RetType>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait4() { }
	virtual DelegateMemberAsyncWait4<TClass, Param1, Param2, Param3, Param4, RetType>* Clone() const {
		return new DelegateMemberAsyncWait4<TClass, Param1, Param2, Param3, Param4, RetType>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
		if (this->m_thread == 0)
			return DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4, RetType>::operator()(p1, p2, p3, p4);
		else {
			// Create a clone instance of this delegate 
			DelegateMemberAsyncWait4<TClass, Param1, Param2, Param3, Param4, RetType>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg4<Param1, Param2, Param3, Param4>* msg = new DelegateMsg4<Param1, Param2, Param3, Param4>(delegate, p1, p2, p3, p4);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout)))
				m_retVal = delegate->m_retVal;

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
			return m_retVal;
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg4<Param1, Param2, Param3, Param4>* delegateMsg = static_cast<DelegateMsg4<Param1, Param2, Param3, Param4>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();
			Param3 param3 = delegateMsg->GetParam3();
			Param4 param4 = delegateMsg->GetParam4();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				m_retVal = DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4, RetType>::operator()(param1, param2, param3, param4);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateMemberAsyncWait4& operator=(const DelegateMemberAsyncWait4& rhs) {
		DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4, RetType>::operator=(rhs);
		return *this;
	}

	RetType GetRetVal() { return m_retVal; }
private:
	RetType m_retVal;				// The delegate return value
};

// N=4 void return type
template <class TClass, class Param1, class Param2, class Param3, class Param4>
class DelegateMemberAsyncWait4<TClass, Param1, Param2, Param3, Param4, void> : public DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4, void> {
public:
	typedef TClass* ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1, Param2, Param3, Param4);
	typedef void (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4) const;
	
	DelegateMemberAsyncWait4(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait4(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait4() { }
	virtual DelegateMemberAsyncWait4<TClass, Param1, Param2, Param3, Param4>* Clone() const {
		return new DelegateMemberAsyncWait4<TClass, Param1, Param2, Param3, Param4>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
		if (this->m_thread == 0)
			DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4>::operator()(p1, p2, p3, p4);
		else {
			// Create a clone instance of this delegate 
			DelegateMemberAsyncWait4<TClass, Param1, Param2, Param3, Param4>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg4<Param1, Param2, Param3, Param4>* msg = new DelegateMsg4<Param1, Param2, Param3, Param4>(delegate, p1, p2, p3, p4);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			this->m_success = delegate->m_sema.Wait(this->m_timeout);

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg4<Param1, Param2, Param3, Param4>* delegateMsg = static_cast<DelegateMsg4<Param1, Param2, Param3, Param4>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();
			Param3 param3 = delegateMsg->GetParam3();
			Param4 param4 = delegateMsg->GetParam4();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4>::operator()(param1, param2, param3, param4);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateMemberAsyncWait4& operator=(const DelegateMemberAsyncWait4& rhs) {
		DelegateMemberAsyncWaitBase4<TClass, Param1, Param2, Param3, Param4>::operator=(rhs);
		return *this;
	}
};

// N=5 abstract base
template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5, class RetType = void>
class DelegateMemberAsyncWaitBase5 : public DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>, public IDelegateInvoker {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2, Param3, Param4, Param5);
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4, Param5) const;
	
	// Contructors take a class instance, member function, and delegate thread
	DelegateMemberAsyncWaitBase5(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(object, func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateMemberAsyncWaitBase5(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(object, func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateMemberAsyncWaitBase5(const DelegateMemberAsyncWaitBase5& rhs) : DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>(rhs), m_refCnt(0) {
		LockGuard::Create(&m_lock);
		Swap(rhs);
	}
	DelegateMemberAsyncWaitBase5() : m_thread(0), m_success(false), m_timeout(0), m_refCnt(0) { LockGuard::Create(&m_lock); }
	virtual ~DelegateMemberAsyncWaitBase5() { LockGuard::Destroy(&m_lock); }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>::Bind(object, func);
	}

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>::Bind(object, func);
	}

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>* derivedRhs = dynamic_cast<const DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>::operator==(rhs);
	}

	DelegateMemberAsyncWaitBase5& operator=(const DelegateMemberAsyncWaitBase5& rhs) {
		if (&rhs != this) {
			DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>::operator=(rhs);
			Swap(rhs);
		}
		return *this;
	}

	/// Returns true if asynchronous function successfully invoked on target thread
	bool IsSuccess() { return m_success; }

protected:
	DelegateThread* m_thread;		// Target thread to invoke the delegate function
	bool m_success;					// Set to true if async function succeeds
	int m_timeout;					// Time in mS to wait for async function to invoke
	Semaphore m_sema;				// Semaphore to signal waiting thread
	LOCK m_lock;					// Lock to synchronize threads
	int m_refCnt;					// Ref count to determine when to delete objects

private:
	void Swap(const DelegateMemberAsyncWaitBase5& s) {
		m_thread = s.m_thread;
		m_timeout = s.m_timeout;
		m_success = s.m_success;
	}
};

// N=5 with return type 
template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5, class RetType = void>
class DelegateMemberAsyncWait5 : public DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5, RetType> {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2, Param3, Param4, Param5);
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4, Param5) const;
	
	DelegateMemberAsyncWait5(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait5(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait5() { }
	virtual DelegateMemberAsyncWait5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>* Clone() const {
		return new DelegateMemberAsyncWait5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
		if (this->m_thread == 0)
			return DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>::operator()(p1, p2, p3, p4, p5);
		else {
			// Create a clone instance of this delegate 
			DelegateMemberAsyncWait5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg5<Param1, Param2, Param3, Param4, Param5>* msg = new DelegateMsg5<Param1, Param2, Param3, Param4, Param5>(delegate, p1, p2, p3, p4, p5);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout)))
				m_retVal = delegate->m_retVal;

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
			return m_retVal;
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg5<Param1, Param2, Param3, Param4, Param5>* delegateMsg = static_cast<DelegateMsg5<Param1, Param2, Param3, Param4, Param5>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();
			Param3 param3 = delegateMsg->GetParam3();
			Param4 param4 = delegateMsg->GetParam4();
			Param4 param5 = delegateMsg->GetParam5();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				m_retVal = DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>::operator()(param1, param2, param3, param4, param5);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateMemberAsyncWait5& operator=(const DelegateMemberAsyncWait5& rhs) {
		DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>::operator=(rhs);
		return *this;
	}

	RetType GetRetVal() { return m_retVal; }
private:
	RetType m_retVal;				// The delegate return value
};

// N=5 void return type
template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5>
class DelegateMemberAsyncWait5<TClass, Param1, Param2, Param3, Param4, Param5, void> : public DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5, void> {
public:
	typedef TClass* ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1, Param2, Param3, Param4, Param5);
	typedef void (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4, Param5) const;
	
	DelegateMemberAsyncWait5(ObjectPtr object, MemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait5(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread, int timeout) :
		DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5>(object, func, thread, timeout) { }
	DelegateMemberAsyncWait5() { }
	virtual DelegateMemberAsyncWait5<TClass, Param1, Param2, Param3, Param4, Param5>* Clone() const {
		return new DelegateMemberAsyncWait5<TClass, Param1, Param2, Param3, Param4, Param5>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
		if (this->m_thread == 0)
			DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5>::operator()(p1, p2, p3, p4, p5);
		else {
			// Create a clone instance of this delegate 
			DelegateMemberAsyncWait5<TClass, Param1, Param2, Param3, Param4, Param5>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg5<Param1, Param2, Param3, Param4, Param5>* msg = new DelegateMsg5<Param1, Param2, Param3, Param4, Param5>(delegate, p1, p2, p3, p4, p5);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			this->m_success = delegate->m_sema.Wait(this->m_timeout);

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg5<Param1, Param2, Param3, Param4, Param5>* delegateMsg = static_cast<DelegateMsg5<Param1, Param2, Param3, Param4, Param5>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();
			Param3 param3 = delegateMsg->GetParam3();
			Param4 param4 = delegateMsg->GetParam4();
			Param5 param5 = delegateMsg->GetParam5();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5>::operator()(param1, param2, param3, param4, param5);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateMemberAsyncWait5& operator=(const DelegateMemberAsyncWait5& rhs) {
		DelegateMemberAsyncWaitBase5<TClass, Param1, Param2, Param3, Param4, Param5>::operator=(rhs);
		return *this;
	}
};

// *** Free Classes Start ***

// N=0 abstract base
template <class RetType = void>
class DelegateFreeAsyncWaitBase0 : public DelegateFree0<RetType>, public IDelegateInvoker {
public:
	typedef RetType (*FreeFunc)();

	// Contructors take a class instance, member function, and delegate thread
	DelegateFreeAsyncWaitBase0(FreeFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(func, thread); 
		LockGuard::Create(&m_lock);
	}
	DelegateFreeAsyncWaitBase0(const DelegateFreeAsyncWaitBase0& rhs) : DelegateFree0<RetType>(rhs), m_refCnt(0) {
		LockGuard::Create(&m_lock);
		Swap(rhs);
	}
	DelegateFreeAsyncWaitBase0() : m_thread(0), m_success(false), m_timeout(0), m_refCnt(0) { LockGuard::Create(&m_lock); }
	virtual ~DelegateFreeAsyncWaitBase0() { LockGuard::Destroy(&m_lock); }

	/// Bind a member function to a delegate. 
	void Bind(FreeFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateFree0<RetType>::Bind(func);
	}

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFreeAsyncWaitBase0<RetType>* derivedRhs = dynamic_cast<const DelegateFreeAsyncWaitBase0<RetType>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateFree0<RetType>::operator==(rhs);
	}

	DelegateFreeAsyncWaitBase0& operator=(const DelegateFreeAsyncWaitBase0& rhs) {
		if (&rhs != this) {
			DelegateFree0<RetType>::operator=(rhs);
			Swap(rhs);
		}
		return *this;
	}

	/// Returns true if asynchronous function successfully invoked on target thread
	bool IsSuccess() { return m_success; }

protected:
	DelegateThread* m_thread;		// Target thread to invoke the delegate function
	bool m_success;					// Set to true if async function succeeds
	int m_timeout;					// Time in mS to wait for async function to invoke
	Semaphore m_sema;				// Semaphore to signal waiting thread
	LOCK m_lock;					// Lock to synchronize threads
	int m_refCnt;					// Ref count to determine when to delete objects

private:
	void Swap(const DelegateFreeAsyncWaitBase0& s) {
		m_thread = s.m_thread;
		m_timeout = s.m_timeout;
		m_success = s.m_success;
	}
};

// N=0 with return type
template <class RetType = void>
class DelegateFreeAsyncWait0 : public DelegateFreeAsyncWaitBase0<RetType> {
public:
	typedef RetType (*FreeFunc)();

	DelegateFreeAsyncWait0(FreeFunc func, DelegateThread* thread, int timeout) :
		DelegateFreeAsyncWaitBase0<RetType>(func, thread, timeout) { }
	DelegateFreeAsyncWait0() { }
	virtual DelegateFreeAsyncWait0<RetType>* Clone() {
		return new DelegateFreeAsyncWait0<RetType>(*this); }		

	/// Invoke delegate function asynchronously
	virtual RetType operator()() {
		if (this->m_thread == 0)
			return DelegateFreeAsyncWaitBase0<RetType>::operator()();
		else {
			// Create a clone instance of this delegate 
			DelegateFreeAsyncWait0<RetType>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsgBase* msg = new DelegateMsgBase(delegate);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout)))
				m_retVal = delegate->m_retVal;

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
			return m_retVal;
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				m_retVal = DelegateFreeAsyncWaitBase0<RetType>::operator()();
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateFreeAsyncWait0& operator=(const DelegateFreeAsyncWait0& rhs) {
		DelegateFreeAsyncWaitBase0<RetType>::operator=(rhs);
		return *this;
	}

	RetType GetRetVal() { return m_retVal; }
private:
	RetType m_retVal;				// The delegate return value
};

// N=0 void return type
template <>
class DelegateFreeAsyncWait0<void> : public DelegateFreeAsyncWaitBase0<void> {
public:
	typedef void (*FreeFunc)();

	DelegateFreeAsyncWait0(FreeFunc func, DelegateThread* thread, int timeout) :
		DelegateFreeAsyncWaitBase0(func, thread, timeout) { }
	DelegateFreeAsyncWait0() { }
	virtual DelegateFreeAsyncWait0<>* Clone() const {
		return new DelegateFreeAsyncWait0<>(*this);	}

	/// Invoke delegate function asynchronously
	virtual void operator()() {
		if (this->m_thread == 0)
			return DelegateFreeAsyncWaitBase0::operator()();
		else {
			// Create a clone instance of this delegate 
			DelegateFreeAsyncWait0<>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsgBase* msg = new DelegateMsgBase(delegate);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout))) {
				// No return or param arguments
			}

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				DelegateFreeAsyncWaitBase0<>::operator()();
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateFreeAsyncWait0& operator=(const DelegateFreeAsyncWait0& rhs) {
		DelegateFreeAsyncWaitBase0<>::operator=(rhs);
		return *this;
	}
};

// N=1 abstract base
template <class Param1, class RetType = void>
class DelegateFreeAsyncWaitBase1 : public DelegateFree1<Param1, RetType>, public IDelegateInvoker {
public:
	typedef RetType (*FreeFunc)(Param1);

	// Contructors take a class instance, member function, and delegate thread
	DelegateFreeAsyncWaitBase1(FreeFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateFreeAsyncWaitBase1(const DelegateFreeAsyncWaitBase1& rhs) : DelegateFree1<Param1, RetType>(rhs), m_refCnt(0) {
		LockGuard::Create(&m_lock);
		Swap(rhs);
	}
	DelegateFreeAsyncWaitBase1() : m_thread(0), m_success(false), m_timeout(0), m_refCnt(0) { LockGuard::Create(&m_lock); }
	virtual ~DelegateFreeAsyncWaitBase1() { LockGuard::Destroy(&m_lock); }

	/// Bind a member function to a delegate. 
	void Bind(FreeFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateFree1<Param1, RetType>::Bind(func);
	}

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFreeAsyncWaitBase1<Param1, RetType>* derivedRhs = dynamic_cast<const DelegateFreeAsyncWaitBase1<Param1, RetType>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateFree1<Param1, RetType>::operator==(rhs);
	}

	DelegateFreeAsyncWaitBase1& operator=(const DelegateFreeAsyncWaitBase1& rhs) {
		if (&rhs != this) {
			DelegateFree1<Param1, RetType>::operator=(rhs);
			Swap(rhs);
		}
		return *this;
	}

	/// Returns true if asynchronous function successfully invoked on target thread
	bool IsSuccess() { return m_success; }

protected:
	DelegateThread* m_thread;		// Target thread to invoke the delegate function
	bool m_success;					// Set to true if async function succeeds
	int m_timeout;					// Time in mS to wait for async function to invoke
	Semaphore m_sema;				// Semaphore to signal waiting thread
	LOCK m_lock;					// Lock to synchronize threads
	int m_refCnt;					// Ref count to determine when to delete objects

private:
	void Swap(const DelegateFreeAsyncWaitBase1& s) {
		m_thread = s.m_thread;
		m_timeout = s.m_timeout;
		m_success = s.m_success;
	}
};

// N=1 with return type
template <class Param1, class RetType = void>
class DelegateFreeAsyncWait1 : public DelegateFreeAsyncWaitBase1<Param1, RetType> {
public:
	typedef RetType (*FreeFunc)(Param1);

	DelegateFreeAsyncWait1(FreeFunc func, DelegateThread* thread, int timeout) :
		DelegateFreeAsyncWaitBase1<Param1, RetType>(func, thread, timeout) { }
	DelegateFreeAsyncWait1() { }
	virtual DelegateFreeAsyncWait1<Param1, RetType>* Clone() const {
		return new DelegateFreeAsyncWait1<Param1, RetType>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual RetType operator()(Param1 p1) {
		if (this->m_thread == 0)
			return DelegateFreeAsyncWaitBase1<Param1, RetType>::operator()(p1);
		else {
			// Create a clone instance of this delegate 
			DelegateFreeAsyncWait1<Param1, RetType>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg1<Param1>* msg = new DelegateMsg1<Param1>(delegate, p1);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout)))
				m_retVal = delegate->m_retVal;

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
			return m_retVal;
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			LockGuard lockGuard(&this->m_lock);
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg1<Param1>* delegateMsg = static_cast<DelegateMsg1<Param1>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();

			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				m_retVal = DelegateFreeAsyncWaitBase1<Param1, RetType>::operator()(param1);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateFreeAsyncWait1& operator=(const DelegateFreeAsyncWait1& rhs) {
		DelegateFreeAsyncWaitBase1<Param1, RetType>::operator=(rhs);
		return *this;
	}

	RetType GetRetVal() { return m_retVal; }
private:
	RetType m_retVal;				// The delegate return value
};

// N=1 void return type
template <class Param1>
class DelegateFreeAsyncWait1<Param1, void> : public DelegateFreeAsyncWaitBase1<Param1, void> {
public:
	typedef void (*FreeFunc)(Param1);

	DelegateFreeAsyncWait1(FreeFunc func, DelegateThread* thread, int timeout) :
		DelegateFreeAsyncWaitBase1<Param1>(func, thread, timeout) { }
	DelegateFreeAsyncWait1() { }
	virtual DelegateFreeAsyncWait1<Param1>* Clone() const {
		return new DelegateFreeAsyncWait1<Param1>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1) {
		if (this->m_thread == 0)
			DelegateFreeAsyncWaitBase1<Param1>::operator()(p1);
		else {
			// Create a clone instance of this delegate 
			DelegateFreeAsyncWait1<Param1>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg1<Param1>* msg = new DelegateMsg1<Param1>(delegate, p1);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			this->m_success = delegate->m_sema.Wait(this->m_timeout);

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			LockGuard lockGuard(&this->m_lock);
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg1<Param1>* delegateMsg = static_cast<DelegateMsg1<Param1>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();

			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				DelegateFreeAsyncWaitBase1<Param1>::operator()(param1);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateFreeAsyncWait1& operator=(const DelegateFreeAsyncWait1& rhs) {
		DelegateFreeAsyncWaitBase1<Param1>::operator=(rhs);
		return *this;
	}
};

// N=2 abstract base
template <class Param1, class Param2, class RetType = void>
class DelegateFreeAsyncWaitBase2 : public DelegateFree2<Param1, Param2, RetType>, public IDelegateInvoker {
public:
	typedef RetType (*FreeFunc)(Param1, Param2);

	// Contructors take a class instance, member function, and delegate thread
	DelegateFreeAsyncWaitBase2(FreeFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateFreeAsyncWaitBase2(const DelegateFreeAsyncWaitBase2& rhs) : DelegateFree2<Param1, Param2, RetType>(rhs), m_refCnt(0) {
		LockGuard::Create(&m_lock);
		Swap(rhs);
	}
	DelegateFreeAsyncWaitBase2() : m_thread(0), m_success(false), m_timeout(0), m_refCnt(0) { LockGuard::Create(&m_lock); }
	virtual ~DelegateFreeAsyncWaitBase2() { LockGuard::Destroy(&m_lock); }

	/// Bind a member function to a delegate. 
	void Bind(FreeFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateFree2<Param1, Param2, RetType>::Bind(func);
	}

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFreeAsyncWaitBase2<Param1, Param2, RetType>* derivedRhs = dynamic_cast<const DelegateFreeAsyncWaitBase2<Param1, Param2, RetType>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateFree2<Param1, Param2, RetType>::operator==(rhs);
	}

	DelegateFreeAsyncWaitBase2& operator=(const DelegateFreeAsyncWaitBase2& rhs) {
		if (&rhs != this) {
			DelegateFree2<Param1, Param2, RetType>::operator=(rhs);
			Swap(rhs);
		}
		return *this;
	}

	/// Returns true if asynchronous function successfully invoked on target thread
	bool IsSuccess() { return m_success; }

protected:
	DelegateThread* m_thread;		// Target thread to invoke the delegate function
	bool m_success;					// Set to true if async function succeeds
	int m_timeout;					// Time in mS to wait for async function to invoke
	Semaphore m_sema;				// Semaphore to signal waiting thread
	LOCK m_lock;					// Lock to synchronize threads
	int m_refCnt;					// Ref count to determine when to delete objects

private:
	void Swap(const DelegateFreeAsyncWaitBase2& s) {
		m_thread = s.m_thread;
		m_timeout = s.m_timeout;
		m_success = s.m_success;
	}
};

// N=2 with return type
template <class Param1, class Param2, class RetType = void>
class DelegateFreeAsyncWait2 : public DelegateFreeAsyncWaitBase2<Param1, Param2, RetType> {
public:
	typedef RetType (*FreeFunc)(Param1, Param2);

	DelegateFreeAsyncWait2(FreeFunc func, DelegateThread* thread, int timeout) :
		DelegateFreeAsyncWaitBase2<Param1, Param2, RetType>(func, thread, timeout) { }
	DelegateFreeAsyncWait2() { }
	virtual DelegateFreeAsyncWait2<Param1, Param2, RetType>* Clone() const {
		return new DelegateFreeAsyncWait2<Param1, Param2, RetType>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual RetType operator()(Param1 p1, Param2 p2) {
		if (this->m_thread == 0)
			return DelegateFreeAsyncWaitBase2<Param1, Param2, RetType>::operator()(p1, p2);
		else {
			// Create a clone instance of this delegate 
			DelegateFreeAsyncWait2<Param1, Param2, RetType>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg2<Param1, Param2>* msg = new DelegateMsg2<Param1, Param2>(delegate, p1, p2);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout)))
				m_retVal = delegate->m_retVal;

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
			return m_retVal;
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			LockGuard lockGuard(&this->m_lock);
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg2<Param1, Param2>* delegateMsg = static_cast<DelegateMsg2<Param1, Param2>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();

			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				m_retVal = DelegateFreeAsyncWaitBase2<Param1, Param2, RetType>::operator()(param1, param2);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateFreeAsyncWait2& operator=(const DelegateFreeAsyncWait2& rhs) {
		DelegateFreeAsyncWaitBase2<Param1, Param2, RetType>::operator=(rhs);
		return *this;
	}

	RetType GetRetVal() { return m_retVal; }
private:
	RetType m_retVal;				// The delegate return value
};

// N=2 void return type
template <class Param1, class Param2>
class DelegateFreeAsyncWait2<Param1, Param2, void> : public DelegateFreeAsyncWaitBase2<Param1, Param2, void> {
public:
	typedef void (*FreeFunc)(Param1, Param2);

	DelegateFreeAsyncWait2(FreeFunc func, DelegateThread* thread, int timeout) :
		DelegateFreeAsyncWaitBase2<Param1, Param2>(func, thread, timeout) { }
	DelegateFreeAsyncWait2() { }
	virtual DelegateFreeAsyncWait2<Param1, Param2>* Clone() const {
		return new DelegateFreeAsyncWait2<Param1, Param2>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2) {
		if (this->m_thread == 0)
			DelegateFreeAsyncWaitBase2<Param1, Param2>::operator()(p1, p2);
		else {
			// Create a clone instance of this delegate 
			DelegateFreeAsyncWait2<Param1, Param2>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg2<Param1, Param2>* msg = new DelegateMsg2<Param1, Param2>(delegate, p1, p2);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			this->m_success = delegate->m_sema.Wait(this->m_timeout);

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg2<Param1, Param2>* delegateMsg = static_cast<DelegateMsg2<Param1, Param2>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				DelegateFreeAsyncWaitBase2<Param1, Param2>::operator()(param1, param2);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateFreeAsyncWait2& operator=(const DelegateFreeAsyncWait2& rhs) {
		DelegateFreeAsyncWaitBase2<Param1, Param2>::operator=(rhs);
		return *this;
	}
};

// N=3 abstract base
template <class Param1, class Param2, class Param3, class RetType = void>
class DelegateFreeAsyncWaitBase3 : public DelegateFree3<Param1, Param2, Param3, RetType>, public IDelegateInvoker {
public:
	typedef RetType (*FreeFunc)(Param1, Param2, Param3);

	// Contructors take a class instance, member function, and delegate thread
	DelegateFreeAsyncWaitBase3(FreeFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateFreeAsyncWaitBase3(const DelegateFreeAsyncWaitBase3& rhs) : DelegateFree3<Param1, Param2, Param3, RetType>(rhs), m_refCnt(0) {
		LockGuard::Create(&m_lock);
		Swap(rhs);
	}
	DelegateFreeAsyncWaitBase3() : m_thread(0), m_success(false), m_timeout(0), m_refCnt(0) { LockGuard::Create(&m_lock); }
	virtual ~DelegateFreeAsyncWaitBase3() { LockGuard::Destroy(&m_lock); }

	/// Bind a member function to a delegate. 
	void Bind(FreeFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateFree3<Param1, Param2, Param3, RetType>::Bind(func);
	}

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFreeAsyncWaitBase3<Param1, Param2, Param3, RetType>* derivedRhs = dynamic_cast<const DelegateFreeAsyncWaitBase3<Param1, Param2, Param3, RetType>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateFree3<Param1, Param2, Param3, RetType>::operator==(rhs);
	}

	DelegateFreeAsyncWaitBase3& operator=(const DelegateFreeAsyncWaitBase3& rhs) {
		if (&rhs != this) {
			DelegateFree3<Param1, Param2, Param3, RetType>::operator=(rhs);
			Swap(rhs);
		}
		return *this;
	}

	/// Returns true if asynchronous function successfully invoked on target thread
	bool IsSuccess() { return m_success; }

protected:
	DelegateThread* m_thread;		// Target thread to invoke the delegate function
	bool m_success;					// Set to true if async function succeeds
	int m_timeout;					// Time in mS to wait for async function to invoke
	Semaphore m_sema;				// Semaphore to signal waiting thread
	LOCK m_lock;					// Lock to synchronize threads
	int m_refCnt;					// Ref count to determine when to delete objects

private:
	void Swap(const DelegateFreeAsyncWaitBase3& s) {
		m_thread = s.m_thread;
		m_timeout = s.m_timeout;
		m_success = s.m_success;
	}
};

// N=3 with return type
template <class Param1, class Param2, class Param3, class RetType = void>
class DelegateFreeAsyncWait3 : public DelegateFreeAsyncWaitBase3<Param1, Param2, Param3, RetType> {
public:
	typedef RetType (*FreeFunc)(Param1, Param2, Param3);

	DelegateFreeAsyncWait3(FreeFunc func, DelegateThread* thread, int timeout) :
		DelegateFreeAsyncWaitBase3<Param1, Param2, Param3, RetType>(func, thread, timeout) { }
	DelegateFreeAsyncWait3() { }
	virtual DelegateFreeAsyncWait3<Param1, Param2, Param3, RetType>* Clone() const {
		return new DelegateFreeAsyncWait3<Param1, Param2, Param3, RetType>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3) {
		if (this->m_thread == 0)
			return DelegateFreeAsyncWaitBase3<Param1, Param2, Param3, RetType>::operator()(p1, p2, p3);
		else {
			// Create a clone instance of this delegate 
			DelegateFreeAsyncWait3<Param1, Param2, Param3, RetType>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg3<Param1, Param2, Param3>* msg = new DelegateMsg3<Param1, Param2, Param3>(delegate, p1, p2, p3);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout)))
				m_retVal = delegate->m_retVal;

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
			return m_retVal;
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg3<Param1, Param2, Param3>* delegateMsg = static_cast<DelegateMsg3<Param1, Param2, Param3>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();
			Param3 param3 = delegateMsg->GetParam3();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				m_retVal = DelegateFreeAsyncWaitBase3<Param1, Param2, Param3, RetType>::operator()(param1, param2, param3);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateFreeAsyncWait3& operator=(const DelegateFreeAsyncWait3& rhs) {
		DelegateFreeAsyncWaitBase3<Param1, Param2, Param3, RetType>::operator=(rhs);
		return *this;
	}

	RetType GetRetVal() { return m_retVal; }
private:
	RetType m_retVal;				// The delegate return value
};

// N=3 void return type
template <class Param1, class Param2, class Param3>
class DelegateFreeAsyncWait3<Param1, Param2, Param3, void> : public DelegateFreeAsyncWaitBase3<Param1, Param2, Param3, void> {
public:
	typedef void (*FreeFunc)(Param1, Param2, Param3);

	DelegateFreeAsyncWait3(FreeFunc func, DelegateThread* thread, int timeout) :
		DelegateFreeAsyncWaitBase3<Param1, Param2, Param3>(func, thread, timeout) { }
	DelegateFreeAsyncWait3() { }
	virtual DelegateFreeAsyncWait3<Param1, Param2, Param3>* Clone() const {
		return new DelegateFreeAsyncWait3<Param1, Param2, Param3>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3) {
		if (this->m_thread == 0)
			DelegateFreeAsyncWaitBase3<Param1, Param2, Param3>::operator()(p1, p2, p3);
		else {
			// Create a clone instance of this delegate 
			DelegateFreeAsyncWait3<Param1, Param2, Param3>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg3<Param1, Param2, Param3>* msg = new DelegateMsg3<Param1, Param2, Param3>(delegate, p1, p2, p3);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			this->m_success = delegate->m_sema.Wait(this->m_timeout);

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg3<Param1, Param2, Param3>* delegateMsg = static_cast<DelegateMsg3<Param1, Param2, Param3>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();
			Param3 param3 = delegateMsg->GetParam3();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				DelegateFreeAsyncWaitBase3<Param1, Param2, Param3>::operator()(param1, param2, param3);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateFreeAsyncWait3& operator=(const DelegateFreeAsyncWait3& rhs) {
		DelegateFreeAsyncWaitBase3<Param1, Param2, Param3>::operator=(rhs);
		return *this;
	}
};

// N=4 abstract base
template <class Param1, class Param2, class Param3, class Param4, class RetType = void>
class DelegateFreeAsyncWaitBase4 : public DelegateFree4<Param1, Param2, Param3, Param4, RetType>, public IDelegateInvoker {
public:
	typedef RetType (*FreeFunc)(Param1, Param2, Param3, Param4);

	// Contructors take a class instance, member function, and delegate thread
	DelegateFreeAsyncWaitBase4(FreeFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateFreeAsyncWaitBase4(const DelegateFreeAsyncWaitBase4& rhs) : DelegateFree4<Param1, Param2, Param3, Param4, RetType>(rhs), m_refCnt(0) {
		LockGuard::Create(&m_lock);
		Swap(rhs);
	}
	DelegateFreeAsyncWaitBase4() : m_thread(0), m_success(false), m_timeout(0), m_refCnt(0) { LockGuard::Create(&m_lock); }
	virtual ~DelegateFreeAsyncWaitBase4() { LockGuard::Destroy(&m_lock); }

	/// Bind a member function to a delegate. 
	void Bind(FreeFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateFree4<Param1, Param2, Param3, Param4, RetType>::Bind(func);
	}

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFreeAsyncWaitBase4<Param1, Param2, Param3, Param4, RetType>* derivedRhs = dynamic_cast<const DelegateFreeAsyncWaitBase4<Param1, Param2, Param3, Param4, RetType>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateFree4<Param1, Param2, Param3, Param4, RetType>::operator==(rhs);
	}

	DelegateFreeAsyncWaitBase4& operator=(const DelegateFreeAsyncWaitBase4& rhs) {
		if (&rhs != this) {
			DelegateFree4<Param1, Param2, Param3, Param4, RetType>::operator=(rhs);
			Swap(rhs);
		}
		return *this;
	}

	/// Returns true if asynchronous function successfully invoked on target thread
	bool IsSuccess() { return m_success; }

protected:
	DelegateThread* m_thread;		// Target thread to invoke the delegate function
	bool m_success;					// Set to true if async function succeeds
	int m_timeout;					// Time in mS to wait for async function to invoke
	Semaphore m_sema;				// Semaphore to signal waiting thread
	LOCK m_lock;					// Lock to synchronize threads
	int m_refCnt;					// Ref count to determine when to delete objects

private:
	void Swap(const DelegateFreeAsyncWaitBase4& s) {
		m_thread = s.m_thread;
		m_timeout = s.m_timeout;
		m_success = s.m_success;
	}
};

// N=4 with return type
template <class Param1, class Param2, class Param3, class Param4, class RetType = void>
class DelegateFreeAsyncWait4 : public DelegateFreeAsyncWaitBase4<Param1, Param2, Param3, Param4, RetType> {
public:
	typedef RetType (*FreeFunc)(Param1, Param2, Param3, Param4);

	DelegateFreeAsyncWait4(FreeFunc func, DelegateThread* thread, int timeout) :
		DelegateFreeAsyncWaitBase4<Param1, Param2, Param3, Param4, RetType>(func, thread, timeout) { }
	DelegateFreeAsyncWait4() { }
	virtual DelegateFreeAsyncWait4<Param1, Param2, Param3, Param4, RetType>* Clone() const {
		return new DelegateFreeAsyncWait4<Param1, Param2, Param3, Param4, RetType>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
		if (this->m_thread == 0)
			return DelegateFreeAsyncWaitBase4<Param1, Param2, Param3, Param4, RetType>::operator()(p1, p2, p3, p4);
		else {
			// Create a clone instance of this delegate 
			DelegateFreeAsyncWait4<Param1, Param2, Param3, Param4, RetType>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg4<Param1, Param2, Param3, Param4>* msg = new DelegateMsg4<Param1, Param2, Param3, Param4>(delegate, p1, p2, p3, p4);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout)))
				m_retVal = delegate->m_retVal;

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
			return m_retVal;
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg4<Param1, Param2, Param3, Param4>* delegateMsg = static_cast<DelegateMsg4<Param1, Param2, Param3, Param4>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();
			Param3 param3 = delegateMsg->GetParam3();
			Param4 param4 = delegateMsg->GetParam4();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				m_retVal = DelegateFreeAsyncWaitBase4<Param1, Param2, Param3, Param4, RetType>::operator()(param1, param2, param3, param4);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateFreeAsyncWait4& operator=(const DelegateFreeAsyncWait4& rhs) {
		DelegateFreeAsyncWaitBase4<Param1, Param2, Param3, Param4, RetType>::operator=(rhs);
		return *this;
	}

	RetType GetRetVal() { return m_retVal; }
private:
	RetType m_retVal;				// The delegate return value
};

// N=4 void return type
template <class Param1, class Param2, class Param3, class Param4>
class DelegateFreeAsyncWait4<Param1, Param2, Param3, Param4, void> : public DelegateFreeAsyncWaitBase4<Param1, Param2, Param3, Param4, void> {
public:
	typedef void (*FreeFunc)(Param1, Param2, Param3, Param4);

	DelegateFreeAsyncWait4(FreeFunc func, DelegateThread* thread, int timeout) :
		DelegateFreeAsyncWaitBase4<Param1, Param2, Param3, Param4>(func, thread, timeout) { }
	DelegateFreeAsyncWait4() { }
	virtual DelegateFreeAsyncWait4<Param1, Param2, Param3, Param4>* Clone() const {
		return new DelegateFreeAsyncWait4<Param1, Param2, Param3, Param4>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
		if (this->m_thread == 0)
			DelegateFreeAsyncWaitBase4<Param1, Param2, Param3, Param4>::operator()(p1, p2, p3, p4);
		else {
			// Create a clone instance of this delegate 
			DelegateFreeAsyncWait4<Param1, Param2, Param3, Param4>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg4<Param1, Param2, Param3, Param4>* msg = new DelegateMsg4<Param1, Param2, Param3, Param4>(delegate, p1, p2, p3, p4);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			this->m_success = delegate->m_sema.Wait(this->m_timeout);

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg4<Param1, Param2, Param3, Param4>* delegateMsg = static_cast<DelegateMsg4<Param1, Param2, Param3, Param4>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();
			Param3 param3 = delegateMsg->GetParam3();
			Param4 param4 = delegateMsg->GetParam4();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				DelegateFreeAsyncWaitBase4<Param1, Param2, Param3, Param4>::operator()(param1, param2, param3, param4);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateFreeAsyncWait4& operator=(const DelegateFreeAsyncWait4& rhs) {
		DelegateFreeAsyncWaitBase4<Param1, Param2, Param3, Param4>::operator=(rhs);
		return *this;
	}
};

// N=5 abstract base
template <class Param1, class Param2, class Param3, class Param4, class Param5, class RetType = void>
class DelegateFreeAsyncWaitBase5 : public DelegateFree5<Param1, Param2, Param3, Param4, Param5, RetType>, public IDelegateInvoker {
public:
	typedef RetType (*FreeFunc)(Param1, Param2, Param3, Param4, Param5);

	// Contructors take a class instance, member function, and delegate thread
	DelegateFreeAsyncWaitBase5(FreeFunc func, DelegateThread* thread, int timeout) : m_success(false), m_timeout(timeout), m_refCnt(0) {
		Bind(func, thread);
		LockGuard::Create(&m_lock);
	}
	DelegateFreeAsyncWaitBase5(const DelegateFreeAsyncWaitBase5& rhs) : DelegateFree5<Param1, Param2, Param3, Param4, Param5, RetType>(rhs), m_refCnt(0) {
		LockGuard::Create(&m_lock);
		Swap(rhs);
	}
	DelegateFreeAsyncWaitBase5() : m_thread(0), m_success(false), m_timeout(0), m_refCnt(0) { LockGuard::Create(&m_lock); }
	virtual ~DelegateFreeAsyncWaitBase5() { LockGuard::Destroy(&m_lock); }

	/// Bind a member function to a delegate. 
	void Bind(FreeFunc func, DelegateThread* thread) {
		m_thread = thread;
		DelegateFree5<Param1, Param2, Param3, Param4, Param5, RetType>::Bind(func);
	}

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFreeAsyncWaitBase5<Param1, Param2, Param3, Param4, Param5, RetType>* derivedRhs = dynamic_cast<const DelegateFreeAsyncWaitBase5<Param1, Param2, Param3, Param4, Param5, RetType>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateFree5<Param1, Param2, Param3, Param4, Param5, RetType>::operator==(rhs);
	}

	DelegateFreeAsyncWaitBase5& operator=(const DelegateFreeAsyncWaitBase5& rhs) {
		if (&rhs != this) {
			DelegateFree5<Param1, Param2, Param3, Param4, Param5, RetType>::operator=(rhs);
			Swap(rhs);
		}
		return *this;
	}

	/// Returns true if asynchronous function successfully invoked on target thread
	bool IsSuccess() { return m_success; }

protected:
	DelegateThread* m_thread;		// Target thread to invoke the delegate function
	bool m_success;					// Set to true if async function succeeds
	int m_timeout;					// Time in mS to wait for async function to invoke
	Semaphore m_sema;				// Semaphore to signal waiting thread
	LOCK m_lock;					// Lock to synchronize threads
	int m_refCnt;					// Ref count to determine when to delete objects

private:
	void Swap(const DelegateFreeAsyncWaitBase5& s) {
		m_thread = s.m_thread;
		m_timeout = s.m_timeout;
		m_success = s.m_success;
	}
};

// N=5 with return type
template <class Param1, class Param2, class Param3, class Param4, class Param5, class RetType = void>
class DelegateFreeAsyncWait5 : public DelegateFreeAsyncWaitBase5<Param1, Param2, Param3, Param4, Param5, RetType> {
public:
	typedef RetType (*FreeFunc)(Param1, Param2, Param3, Param4, Param5);

	DelegateFreeAsyncWait5(FreeFunc func, DelegateThread* thread, int timeout) :
		DelegateFreeAsyncWaitBase5<Param1, Param2, Param3, Param4, Param5, RetType>(func, thread, timeout) { }
	DelegateFreeAsyncWait5() { }
	virtual DelegateFreeAsyncWait5<Param1, Param2, Param3, Param4, Param5, RetType>* Clone() const {
		return new DelegateFreeAsyncWait5<Param1, Param2, Param3, Param4, Param5, RetType>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
		if (this->m_thread == 0)
			return DelegateFreeAsyncWaitBase5<Param1, Param2, Param3, Param4, Param5, RetType>::operator()(p1, p2, p3, p4, p5);
		else {
			// Create a clone instance of this delegate 
			DelegateFreeAsyncWait5<Param1, Param2, Param3, Param4, Param5, RetType>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg5<Param1, Param2, Param3, Param4, Param5>* msg = new DelegateMsg5<Param1, Param2, Param3, Param4, Param5>(delegate, p1, p2, p3, p4, p5);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			if ((this->m_success = delegate->m_sema.Wait(this->m_timeout)))
				m_retVal = delegate->m_retVal;

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
			return m_retVal;
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg5<Param1, Param2, Param3, Param4, Param5>* delegateMsg = static_cast<DelegateMsg5<Param1, Param2, Param3, Param4, Param5>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();
			Param3 param3 = delegateMsg->GetParam3();
			Param4 param4 = delegateMsg->GetParam4();
			Param5 param5 = delegateMsg->GetParam5();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				m_retVal = DelegateFreeAsyncWaitBase5<Param1, Param2, Param3, Param4, Param5, RetType>::operator()(param1, param2, param3, param4, param5);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateFreeAsyncWait5& operator=(const DelegateFreeAsyncWait5& rhs) {
		DelegateFreeAsyncWaitBase5<Param1, Param2, Param3, Param4, Param5, RetType>::operator=(rhs);
		return *this;
	}

	RetType GetRetVal() { return m_retVal; }
private:
	RetType m_retVal;				// The delegate return value
};

// N=5 void return type
template <class Param1, class Param2, class Param3, class Param4, class Param5>
class DelegateFreeAsyncWait5<Param1, Param2, Param3, Param4, Param5, void> : public DelegateFreeAsyncWaitBase5<Param1, Param2, Param3, Param4, Param5, void> {
public:
	typedef void (*FreeFunc)(Param1, Param2, Param3, Param4, Param5);

	DelegateFreeAsyncWait5(FreeFunc func, DelegateThread* thread, int timeout) :
		DelegateFreeAsyncWaitBase5<Param1, Param2, Param3, Param4, Param5>(func, thread, timeout) { }
	DelegateFreeAsyncWait5() { }
	virtual DelegateFreeAsyncWait5<Param1, Param2, Param3, Param4, Param5>* Clone() const {
		return new DelegateFreeAsyncWait5<Param1, Param2, Param3, Param4, Param5>(*this);
	}

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
		if (this->m_thread == 0)
			DelegateFreeAsyncWaitBase5<Param1, Param2, Param3, Param4, Param5>::operator()(p1, p2, p3, p4, p5);
		else {
			// Create a clone instance of this delegate 
			DelegateFreeAsyncWait5<Param1, Param2, Param3, Param4, Param5>* delegate = Clone();
			delegate->m_refCnt = 2;
			delegate->m_sema.Create();
			delegate->m_sema.Reset();

			// Create a new message instance 
			DelegateMsg5<Param1, Param2, Param3, Param4, Param5>* msg = new DelegateMsg5<Param1, Param2, Param3, Param4, Param5>(delegate, p1, p2, p3, p4, p5);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			this->m_thread->DispatchDelegate(msg);

			// Wait for target thread to execute the delegate function
			this->m_success = delegate->m_sema.Wait(this->m_timeout);

			bool deleteData = false;
			{
				LockGuard lockGuard(&delegate->m_lock);
				if (--delegate->m_refCnt == 0)
					deleteData = true;
			}
			if (deleteData) {
				delete msg;
				delete delegate;
			}
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		bool deleteData = false;
		{
			// Typecast the base pointer to back to the templatized instance
			DelegateMsg5<Param1, Param2, Param3, Param4, Param5>* delegateMsg = static_cast<DelegateMsg5<Param1, Param2, Param3, Param4, Param5>*>(*msg);

			// Get the function parameter data
			Param1 param1 = delegateMsg->GetParam1();
			Param2 param2 = delegateMsg->GetParam2();
			Param3 param3 = delegateMsg->GetParam3();
			Param4 param4 = delegateMsg->GetParam4();
			Param5 param5 = delegateMsg->GetParam5();

			LockGuard lockGuard(&this->m_lock);
			if (this->m_refCnt == 2) {
				// Invoke the delegate function then signal the waiting thread
				DelegateFreeAsyncWaitBase5<Param1, Param2, Param3, Param4, Param5>::operator()(param1, param2, param3, param4, param5);
				this->m_sema.Signal();
			}

			// If waiting thread is no longer waiting then delete heap data
			if (--this->m_refCnt == 0)
				deleteData = true;
		}
		if (deleteData) {
			delete *msg;
			*msg = 0;
			delete this;
		}
	}

	DelegateFreeAsyncWait5& operator=(const DelegateFreeAsyncWait5& rhs) {
		DelegateFreeAsyncWaitBase5<Param1, Param2, Param3, Param4, Param5>::operator=(rhs);
		return *this;
	}
};

//N=0
template <class TClass, class RetType>
DelegateMemberAsyncWait0<TClass, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(), DelegateThread* thread, int timeout) { 
	return DelegateMemberAsyncWait0<TClass, RetType>(object, func, thread, timeout);
}

template <class TClass, class RetType>
DelegateMemberAsyncWait0<TClass, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)() const, DelegateThread* thread, int timeout) { 
	return DelegateMemberAsyncWait0<TClass, RetType>(object, func, thread, timeout);
}

template <class RetType>
DelegateFreeAsyncWait0<RetType> MakeDelegate(RetType (*func)(), DelegateThread* thread, int timeout) { 
	return DelegateFreeAsyncWait0<RetType>(func, thread, timeout);
}

//N=1
template <class TClass, class Param1, class RetType>
DelegateMemberAsyncWait1<TClass, Param1, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1), DelegateThread* thread, int timeout) { 
	return DelegateMemberAsyncWait1<TClass, Param1, RetType>(object, func, thread, timeout);
}

template <class TClass, class Param1, class RetType>
DelegateMemberAsyncWait1<TClass, Param1, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1) const, DelegateThread* thread, int timeout) { 
	return DelegateMemberAsyncWait1<TClass, Param1, RetType>(object, func, thread, timeout);
}

template <class Param1, class RetType>
DelegateFreeAsyncWait1<Param1, RetType> MakeDelegate(RetType (*func)(Param1 p1), DelegateThread* thread, int timeout) { 
	return DelegateFreeAsyncWait1<Param1, RetType>(func, thread, timeout);
}

//N=2
template <class TClass, class Param1, class Param2, class RetType>
DelegateMemberAsyncWait2<TClass, Param1, Param2, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2), DelegateThread* thread, int timeout) { 
	return DelegateMemberAsyncWait2<TClass, Param1, Param2, RetType>(object, func, thread, timeout);
}

template <class TClass, class Param1, class Param2, class RetType>
DelegateMemberAsyncWait2<TClass, Param1, Param2, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2) const, DelegateThread* thread, int timeout) { 
	return DelegateMemberAsyncWait2<TClass, Param1, Param2, RetType>(object, func, thread, timeout);
}

template <class Param1, class Param2, class RetType>
DelegateFreeAsyncWait2<Param1, Param2, RetType> MakeDelegate(RetType (*func)(Param1 p1, Param2 p2), DelegateThread* thread, int timeout) { 
	return DelegateFreeAsyncWait2<Param1, Param2, RetType>(func, thread, timeout);
}

//N=3
template <class TClass, class Param1, class Param2, class Param3, class RetType>
DelegateMemberAsyncWait3<TClass, Param1, Param2, Param3, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3), DelegateThread* thread, int timeout) { 
	return DelegateMemberAsyncWait3<TClass, Param1, Param2, Param3, RetType>(object, func, thread, timeout);
}

template <class TClass, class Param1, class Param2, class Param3, class RetType>
DelegateMemberAsyncWait3<TClass, Param1, Param2, Param3, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3) const, DelegateThread* thread, int timeout) { 
	return DelegateMemberAsyncWait3<TClass, Param1, Param2, Param3, RetType>(object, func, thread, timeout);
}

template <class Param1, class Param2, class Param3, class RetType>
DelegateFreeAsyncWait3<Param1, Param2, Param3, RetType> MakeDelegate(RetType (*func)(Param1 p1, Param2 p2, Param3 p3), DelegateThread* thread, int timeout) { 
	return DelegateFreeAsyncWait3<Param1, Param2, Param3, RetType>(func, thread, timeout);
}

//N=4
template <class TClass, class Param1, class Param2, class Param3, class Param4, class RetType>
DelegateMemberAsyncWait4<TClass, Param1, Param2, Param3, Param4, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4), DelegateThread* thread, int timeout) { 
	return DelegateMemberAsyncWait4<TClass, Param1, Param2, Param3, Param4, RetType>(object, func, thread, timeout);
}

template <class TClass, class Param1, class Param2, class Param3, class Param4, class RetType>
DelegateMemberAsyncWait4<TClass, Param1, Param2, Param3, Param4, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const, DelegateThread* thread, int timeout) { 
	return DelegateMemberAsyncWait4<TClass, Param1, Param2, Param3, Param4, RetType>(object, func, thread, timeout);
}

template <class Param1, class Param2, class Param3, class Param4, class RetType>
DelegateFreeAsyncWait4<Param1, Param2, Param3, Param4, RetType> MakeDelegate(RetType (*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4), DelegateThread* thread, int timeout) { 
	return DelegateFreeAsyncWait4<Param1, Param2, Param3, Param4, RetType>(func, thread, timeout);
}

//N=5
template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5, class RetType>
DelegateMemberAsyncWait5<TClass, Param1, Param2, Param3, Param4, Param5, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5), DelegateThread* thread, int timeout) { 
	return DelegateMemberAsyncWait5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>(object, func, thread, timeout);
}

template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5, class RetType>
DelegateMemberAsyncWait5<TClass, Param1, Param2, Param3, Param4, Param5, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const, DelegateThread* thread, int timeout) { 
	return DelegateMemberAsyncWait5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>(object, func, thread, timeout);
}

template <class Param1, class Param2, class Param3, class Param4, class Param5, class RetType>
DelegateFreeAsyncWait5<Param1, Param2, Param3, Param4, Param5, RetType> MakeDelegate(RetType (*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5), DelegateThread* thread, int timeout) { 
	return DelegateFreeAsyncWait5<Param1, Param2, Param3, Param4, Param5, RetType>(func, thread, timeout);
}

} 

#endif
