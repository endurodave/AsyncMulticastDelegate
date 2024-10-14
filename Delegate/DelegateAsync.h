#ifndef _DELEGATE_ASYNC_H
#define _DELEGATE_ASYNC_H

// Delegate.h
// @see https://github.com/endurodave/AsyncMulticastDelegate
// David Lafreniere, Dec 2016.

#include "Delegate.h"
#include "DelegateThread.h"
#include "DelegateInvoker.h"
#if USE_XALLOCATOR
	#include <new>
#endif

namespace DelegateLib {

/// @brief Implements a new/delete for pass by value parameter values. Doesn't 
/// actually create memory as pass by value already has a full copy.
template <typename Param>
class DelegateParam
{
public:
	static Param New(Param param) {	return param; }
	static void Delete(Param param) { }
};

/// @brief Implement new/delete for pointer parameter values. If USE_ALLOCATOR is
/// defined, get memory from the fixed block allocator and not global heap.
template <typename Param>
class DelegateParam<Param *>
{
public:
	static Param* New(Param* param)	{
#if USE_XALLOCATOR
		void* mem = xmalloc(sizeof(*param));
		Param* newParam = new (mem) Param(*param);
#else
		Param* newParam = new Param(*param);
#endif
		return newParam;
	}

	static void Delete(Param* param) {
#if USE_XALLOCATOR
		param->~Param();
		xfree((void*)param);
#else
		delete param;
#endif
	}
};

/// @brief Implement new/delete for pointer to pointer parameter values. 
template <typename Param>
class DelegateParam<Param **>
{
public:
	static Param** New(Param** param) {
#if USE_XALLOCATOR
		void* mem = xmalloc(sizeof(*param));
		Param** newParam = new (mem) Param*();

		void* mem2 = xmalloc(sizeof(**param));
		*newParam = new (mem2) Param(**param);
#else
		Param** newParam = new Param*();
		*newParam = new Param(**param);
#endif
		return newParam;
	}

	static void Delete(Param** param) {
#if USE_XALLOCATOR
		(*param)->~Param();
		xfree((void*)(*param));

		xfree((void*)(param));
#else
		delete *param;
		delete param;
#endif
	}
};

/// @brief Implement new/delete for reference parameter values. 
template <typename Param>
class DelegateParam<Param &>
{
public:
	static Param& New(Param& param)	{
#if USE_XALLOCATOR
		void* mem = xmalloc(sizeof(param));
		Param* newParam = new (mem) Param(param);
#else
		Param* newParam = new Param(param);
#endif
		return *newParam;
	}

	static void Delete(Param& param) {
#if USE_XALLOCATOR
		(&param)->~Param();
		xfree((void*)(&param));
#else
		delete &param;
#endif
	}
};

/// @brief Asynchronous member delegate that invokes the target function on the specified thread of control.
template <class TClass> 
class DelegateMemberAsync0 : public DelegateMember0<TClass>, public IDelegateInvoker {
public:
	typedef TClass* ObjectPtr;
	typedef void (TClass::*MemberFunc)();
	typedef void (TClass::*ConstMemberFunc)() const;

	// Contructors take a class instance, member function, and delegate thread
	DelegateMemberAsync0(ObjectPtr object, MemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberAsync0(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberAsync0() : m_thread(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateMember0<TClass>::Bind(object, func); }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)	{
		m_thread = thread;
		DelegateMember0<TClass>::Bind(object, func); }

	virtual DelegateMemberAsync0<TClass>* Clone() const {
		return new DelegateMemberAsync0<TClass>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberAsync0<TClass>* derivedRhs = dynamic_cast<const DelegateMemberAsync0<TClass>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread && 
			DelegateMember0<TClass>::operator == (rhs); }

	/// Invoke delegate function asynchronously
	virtual void operator()() {
		if (m_thread == 0)
			DelegateMember0<TClass>::operator()();
		else
		{
			// Create a clone instance of this delegate 
			DelegateMemberAsync0<TClass>* delegate = Clone();

			// Create a new message instance 
			DelegateMsgBase* msg = new DelegateMsgBase(delegate);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			m_thread->DispatchDelegate(msg);
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		// Invoke the delegate function
		DelegateMember0<TClass>::operator()();

		// Delete heap data created inside operator()
		delete *msg;
		*msg = 0;

		// Do this last before returning!
		delete this;
	}

private:
	/// Target thread to invoke the delegate function
	DelegateThread* m_thread;
};

template <class TClass, class Param1> 
class DelegateMemberAsync1 : public DelegateMember1<TClass, Param1>, public IDelegateInvoker {
public:
	typedef TClass* ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1);
	typedef void (TClass::*ConstMemberFunc)(Param1) const;

	// Contructors take a class instance, member function, and callback thread
	DelegateMemberAsync1(ObjectPtr object, MemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberAsync1(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberAsync1() : m_thread(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateMember1<TClass, Param1>::Bind(object, func); }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)	{
		m_thread = thread;
		DelegateMember1<TClass, Param1>::Bind(object, func); }

	virtual DelegateMemberAsync1<TClass, Param1>* Clone() const {
		return new DelegateMemberAsync1<TClass, Param1>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberAsync1<TClass, Param1>* derivedRhs = dynamic_cast<const DelegateMemberAsync1<TClass, Param1>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread && 
			DelegateMember1<TClass, Param1>::operator == (rhs); }

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1) {
		if (m_thread == 0)
			DelegateMember1<TClass, Param1>::operator()(p1);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);

			// Create a clone instance of this delegate 
			DelegateMemberAsync1<TClass, Param1>* delegate = Clone();

			// Create a new message instance 
			DelegateMsg1<Param1>* msg = new DelegateMsg1<Param1>(delegate, heapParam1);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			m_thread->DispatchDelegate(msg);
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		// Typecast the base pointer to back to the templatized instance
		DelegateMsg1<Param1>* delegateMsg = static_cast<DelegateMsg1<Param1>*>(*msg);

		// Get the function parameter data
		Param1 param1 = delegateMsg->GetParam1();

		// Invoke the delegate function
		DelegateMember1<TClass, Param1>::operator()(param1);

		// Delete heap data created inside operator()
		DelegateParam<Param1>::Delete(param1);
		delete *msg;
		*msg = 0;

		// Do this last before returning!
		delete this;
	}

private:
	/// Target thread to invoke the delegate function
	DelegateThread* m_thread;
};

template <class TClass, class Param1, class Param2> 
class DelegateMemberAsync2 : public DelegateMember2<TClass, Param1, Param2>, public IDelegateInvoker {
public:
	typedef TClass* ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1, Param2);
	typedef void (TClass::*ConstMemberFunc)(Param1, Param2) const;

	// Contructors take a class instance, member function, and callback thread
	DelegateMemberAsync2(ObjectPtr object, MemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberAsync2(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberAsync2() : m_thread(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateMember2<TClass, Param1, Param2>::Bind(object, func); }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)	{
		m_thread = thread;
		DelegateMember2<TClass, Param1, Param2>::Bind(object, func); }

	virtual DelegateMemberAsync2<TClass, Param1, Param2>* Clone() const {
		return new DelegateMemberAsync2<TClass, Param1, Param2>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberAsync2<TClass, Param1, Param2>* derivedRhs = dynamic_cast<const DelegateMemberAsync2<TClass, Param1, Param2>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread && 
			DelegateMember2<TClass, Param1, Param2>::operator == (rhs); }

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2) {
		if (m_thread == 0)
			DelegateMember2<TClass, Param1, Param2>::operator()(p1, p2);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);
			Param2 heapParam2 = DelegateParam<Param2>::New(p2);

			// Create a clone instance of this delegate 
			DelegateMemberAsync2<TClass, Param1, Param2>* delegate = Clone();

			// Create a new message instance 
			DelegateMsg2<Param1, Param2>* msg = new DelegateMsg2<Param1, Param2>(delegate, heapParam1, heapParam2);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			m_thread->DispatchDelegate(msg);
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		// Typecast the base pointer to back to the templatized instance
		DelegateMsg2<Param1, Param2>* delegateMsg = static_cast<DelegateMsg2<Param1, Param2>*>(*msg);

		// Get the function parameter data
		Param1 param1 = delegateMsg->GetParam1();
		Param2 param2 = delegateMsg->GetParam2();

		// Invoke the delegate function
		DelegateMember2<TClass, Param1, Param2>::operator()(param1, param2);

		// Delete heap data created inside operator()
		DelegateParam<Param1>::Delete(param1);
		DelegateParam<Param2>::Delete(param2);
		delete *msg;
		*msg = 0;

		// Do this last before returning!
		delete this;
	}

private:
	/// Target thread to invoke the delegate function
	DelegateThread* m_thread;
};

template <class TClass, class Param1, class Param2, class Param3> 
class DelegateMemberAsync3 : public DelegateMember3<TClass, Param1, Param2, Param3>, public IDelegateInvoker {
public:
	typedef TClass* ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1, Param2, Param3);
	typedef void (TClass::*ConstMemberFunc)(Param1, Param2, Param3) const;

	// Contructors take a class instance, member function, and callback thread
	DelegateMemberAsync3(ObjectPtr object, MemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberAsync3(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberAsync3() : m_thread(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateMember3<TClass, Param1, Param2, Param3>::Bind(object, func); }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)	{
		m_thread = thread;
		DelegateMember3<TClass, Param1, Param2, Param3>::Bind(object, func); }

	virtual DelegateMemberAsync3<TClass, Param1, Param2, Param3>* Clone() const {
		return new DelegateMemberAsync3<TClass, Param1, Param2, Param3>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberAsync3<TClass, Param1, Param2, Param3>* derivedRhs = dynamic_cast<const DelegateMemberAsync3<TClass, Param1, Param2, Param3>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread && 
			DelegateMember3<TClass, Param1, Param2, Param3>::operator == (rhs); }

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3) {
		if (m_thread == 0)
			DelegateMember3<TClass, Param1, Param2, Param3>::operator()(p1, p2, p3);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);
			Param2 heapParam2 = DelegateParam<Param2>::New(p2);
			Param3 heapParam3 = DelegateParam<Param3>::New(p3);

			// Create a clone instance of this delegate 
			DelegateMemberAsync3<TClass, Param1, Param2, Param3>* delegate = Clone();

			// Create a new message instance 
			DelegateMsg3<Param1, Param2, Param3>* msg = new DelegateMsg3<Param1, Param2, Param3>(delegate, heapParam1, heapParam2, heapParam3);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			m_thread->DispatchDelegate(msg);
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		// Typecast the base pointer to back to the templatized instance
		DelegateMsg3<Param1, Param2, Param3>* delegateMsg = static_cast<DelegateMsg3<Param1, Param2, Param3>*>(*msg);

		// Get the function parameter data
		Param1 param1 = delegateMsg->GetParam1();
		Param2 param2 = delegateMsg->GetParam2();
		Param3 param3 = delegateMsg->GetParam3();

		// Invoke the delegate function
		DelegateMember3<TClass, Param1, Param2, Param3>::operator()(param1, param2, param3);

		// Delete heap data created inside operator()
		DelegateParam<Param1>::Delete(param1);
		DelegateParam<Param2>::Delete(param2);
		DelegateParam<Param3>::Delete(param3);
		delete *msg;
		*msg = 0;

		// Do this last before returning!
		delete this;
	}

private:
	/// Target thread to invoke the delegate function
	DelegateThread* m_thread;
};

template <class TClass, class Param1, class Param2, class Param3, class Param4> 
class DelegateMemberAsync4 : public DelegateMember4<TClass, Param1, Param2, Param3, Param4>, public IDelegateInvoker {
public:
	typedef TClass* ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1, Param2, Param3, Param4);
	typedef void (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4) const;

	// Contructors take a class instance, member function, and callback thread
	DelegateMemberAsync4(ObjectPtr object, MemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberAsync4(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberAsync4() : m_thread(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateMember4<TClass, Param1, Param2, Param3, Param4>::Bind(object, func); }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)	{
		m_thread = thread;
		DelegateMember4<TClass, Param1, Param2, Param3, Param4>::Bind(object, func); }

	virtual DelegateMemberAsync4<TClass, Param1, Param2, Param3, Param4>* Clone() const {
		return new DelegateMemberAsync4<TClass, Param1, Param2, Param3, Param4>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberAsync4<TClass, Param1, Param2, Param3, Param4>* derivedRhs = dynamic_cast<const DelegateMemberAsync4<TClass, Param1, Param2, Param3, Param4>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread && 
			DelegateMember4<TClass, Param1, Param2, Param3, Param4>::operator == (rhs); }

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
		if (m_thread == 0)
			DelegateMember4<TClass, Param1, Param2, Param3, Param4>::operator()(p1, p2, p3, p4);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);
			Param2 heapParam2 = DelegateParam<Param2>::New(p2);
			Param3 heapParam3 = DelegateParam<Param3>::New(p3);
			Param4 heapParam4 = DelegateParam<Param4>::New(p4);

			// Create a clone instance of this delegate 
			DelegateMemberAsync4<TClass, Param1, Param2, Param3, Param4>* delegate = Clone();

			// Create a new message instance 
			DelegateMsg4<Param1, Param2, Param3, Param4>* msg = new DelegateMsg4<Param1, Param2, Param3, Param4>(delegate, heapParam1, heapParam2, heapParam3, heapParam4);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			m_thread->DispatchDelegate(msg);
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		// Typecast the base pointer to back to the templatized instance
		DelegateMsg4<Param1, Param2, Param3, Param4>* delegateMsg = static_cast<DelegateMsg4<Param1, Param2, Param3, Param4>*>(*msg);

		// Get the function parameter data
		Param1 param1 = delegateMsg->GetParam1();
		Param2 param2 = delegateMsg->GetParam2();
		Param3 param3 = delegateMsg->GetParam3();
		Param4 param4 = delegateMsg->GetParam4();

		// Invoke the delegate function
		DelegateMember4<TClass, Param1, Param2, Param3, Param4>::operator()(param1, param2, param3, param4);

		// Delete heap data created inside operator()
		DelegateParam<Param1>::Delete(param1);
		DelegateParam<Param2>::Delete(param2);
		DelegateParam<Param3>::Delete(param3);
		DelegateParam<Param4>::Delete(param4);
		delete *msg;
		*msg = 0;

		// Do this last before returning!
		delete this;
	}

private:
	/// Target thread to invoke the delegate function
	DelegateThread* m_thread;
};

template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5> 
class DelegateMemberAsync5 : public DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5>, public IDelegateInvoker {
public:
	typedef TClass* ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1, Param2, Param3, Param4, Param5);
	typedef void (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4, Param5) const;

	// Contructors take a class instance, member function, and callback thread
	DelegateMemberAsync5(ObjectPtr object, MemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberAsync5(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberAsync5() : m_thread(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5>::Bind(object, func); }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)	{
		m_thread = thread;
		DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5>::Bind(object, func); }

	virtual DelegateMemberAsync5<TClass, Param1, Param2, Param3, Param4, Param5>* Clone() const {
		return new DelegateMemberAsync5<TClass, Param1, Param2, Param3, Param4, Param5>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberAsync5<TClass, Param1, Param2, Param3, Param4, Param5>* derivedRhs = dynamic_cast<const DelegateMemberAsync5<TClass, Param1, Param2, Param3, Param4, Param5>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread && 
			DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5>::operator == (rhs); }

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
		if (m_thread == 0)
			DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5>::operator()(p1, p2, p3, p4, p5);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);
			Param2 heapParam2 = DelegateParam<Param2>::New(p2);
			Param3 heapParam3 = DelegateParam<Param3>::New(p3);
			Param4 heapParam4 = DelegateParam<Param4>::New(p4);
			Param5 heapParam5 = DelegateParam<Param5>::New(p5);

			// Create a clone instance of this delegate 
			DelegateMemberAsync5<TClass, Param1, Param2, Param3, Param4, Param5>* delegate = Clone();

			// Create a new message instance 
			DelegateMsg5<Param1, Param2, Param3, Param4, Param5>* msg = new DelegateMsg5<Param1, Param2, Param3, Param4, Param5>(delegate, heapParam1, heapParam2, heapParam3, heapParam4, heapParam5);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			m_thread->DispatchDelegate(msg);
		}
	}

	/// Called by the target thread to invoke the delegate function 
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		// Typecast the base pointer to back to the templatized instance
		DelegateMsg5<Param1, Param2, Param3, Param4, Param5>* delegateMsg = static_cast<DelegateMsg5<Param1, Param2, Param3, Param4, Param5>*>(*msg);

		// Get the function parameter data
		Param1 param1 = delegateMsg->GetParam1();
		Param2 param2 = delegateMsg->GetParam2();
		Param3 param3 = delegateMsg->GetParam3();
		Param4 param4 = delegateMsg->GetParam4();
		Param5 param5 = delegateMsg->GetParam5();

		// Invoke the delegate function
		DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5>::operator()(param1, param2, param3, param4, param5);

		// Delete heap data created inside operator()
		DelegateParam<Param1>::Delete(param1);
		DelegateParam<Param2>::Delete(param2);
		DelegateParam<Param3>::Delete(param3);
		DelegateParam<Param4>::Delete(param4);
		DelegateParam<Param5>::Delete(param5);
		delete *msg;
		*msg = 0;

		// Do this last before returning!
		delete this;
	}

private:
	/// Target thread to invoke the delegate function
	DelegateThread* m_thread;
};

/// @brief Asynchronous free delegate that invokes the target function on the specified thread of control.
class DelegateFreeAsync0 : public DelegateFree0<void>, public IDelegateInvoker {
public:
	typedef void (*FreeFunc)();

	DelegateFreeAsync0(FreeFunc func, DelegateThread* thread) { Bind(func, thread); }
	DelegateFreeAsync0() : m_thread(0) { }

	/// Bind a free function to the delegate.
	void Bind(FreeFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateFree0<void>::Bind(func);	}

	virtual DelegateFreeAsync0* Clone() const {
		return new DelegateFreeAsync0(*this); }

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFreeAsync0* derivedRhs = dynamic_cast<const DelegateFreeAsync0*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateFree0<void>::operator == (rhs); }

	// Invoke delegate function asynchronously
	virtual void operator()() {
		if (m_thread == 0)
			DelegateFree0<void>::operator()();
		else
		{
			// Create a clone instance of this delegate 
			DelegateFreeAsync0* delegate = Clone();

			// Create a new message instance 
			DelegateMsgBase* msg = new DelegateMsgBase(delegate);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			m_thread->DispatchDelegate(msg);
		}
	}

	// Called to invoke the delegate function on the target thread of control
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		// Invoke the delegate function
		DelegateFree0<void>::operator()();

		delete *msg;
		*msg = 0;

		// Do this last before returning!
		delete this;
	}

private:
	DelegateThread* m_thread;
};

template <class Param1> 
class DelegateFreeAsync1 : public DelegateFree1<Param1>, public IDelegateInvoker {
public:
	typedef void (*FreeFunc)(Param1);

	DelegateFreeAsync1(FreeFunc func, DelegateThread* thread) { Bind(func, thread); }
	DelegateFreeAsync1() : m_thread(0) { }

	/// Bind a free function to the delegate.
	void Bind(FreeFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateFree1<Param1>::Bind(func);	}

	virtual DelegateFreeAsync1<Param1>* Clone() const {
		return new DelegateFreeAsync1<Param1>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFreeAsync1<Param1>* derivedRhs = dynamic_cast<const DelegateFreeAsync1<Param1>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateFree1<Param1>::operator == (rhs); }

	// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1) {
		if (m_thread == 0)
			DelegateFree1<Param1>::operator()(p1);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);

			// Create a clone instance of this delegate 
			DelegateFreeAsync1<Param1>* delegate = Clone();

			// Create a new message instance 
			DelegateMsg1<Param1>* msg = new DelegateMsg1<Param1>(delegate, heapParam1);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			m_thread->DispatchDelegate(msg);
		}
	}

	// Called to invoke the delegate function on the target thread of control
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		// Typecast the base pointer to back to the templatized instance
		DelegateMsg1<Param1>* delegateMsg = static_cast<DelegateMsg1<Param1>*>(*msg);

		// Get the function parameter data
		Param1 param1 = delegateMsg->GetParam1();

		// Invoke the delegate function
		DelegateFree1<Param1>::operator()(param1);

		// Delete heap data created inside operator()
		DelegateParam<Param1>::Delete(param1);
		delete *msg;
		*msg = 0;

		// Do this last before returning!
		delete this;
	}

private:
	DelegateThread* m_thread;
};

template <class Param1, class Param2> 
class DelegateFreeAsync2 : public DelegateFree2<Param1, Param2>, public IDelegateInvoker {
public:
	typedef void (*FreeFunc)(Param1, Param2);

	DelegateFreeAsync2(FreeFunc func, DelegateThread* thread) { Bind(func, thread); }
	DelegateFreeAsync2() : m_thread(0) { }

	/// Bind a free function to the delegate.
	void Bind(FreeFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateFree2<Param1, Param2>::Bind(func);	}

	virtual DelegateFreeAsync2<Param1, Param2>* Clone() const {
		return new DelegateFreeAsync2<Param1, Param2>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFreeAsync2<Param1, Param2>* derivedRhs = dynamic_cast<const DelegateFreeAsync2<Param1, Param2>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateFree2<Param1, Param2>::operator == (rhs); }

	// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2) {
		if (m_thread == 0)
			DelegateFree2<Param1, Param2>::operator()(p1, p2);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);
			Param2 heapParam2 = DelegateParam<Param2>::New(p2);

			// Create a clone instance of this delegate 
			DelegateFreeAsync2<Param1, Param2>* delegate = Clone();

			// Create a new message instance 
			DelegateMsg2<Param1, Param2>* msg = new DelegateMsg2<Param1, Param2>(delegate, heapParam1, heapParam2);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			m_thread->DispatchDelegate(msg);
		}
	}

	// Called to invoke the delegate function on the target thread of control
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		// Typecast the base pointer to back to the templatized instance
		DelegateMsg2<Param1, Param2>* delegateMsg = static_cast<DelegateMsg2<Param1, Param2>*>(*msg);

		// Get the function parameter data
		Param1 param1 = delegateMsg->GetParam1();
		Param2 param2 = delegateMsg->GetParam2();

		// Invoke the delegate function
		DelegateFree2<Param1, Param2>::operator()(param1, param2);

		// Delete heap data created inside operator()
		DelegateParam<Param1>::Delete(param1);
		DelegateParam<Param2>::Delete(param2);
		delete *msg;
		*msg = 0;

		// Do this last before returning!
		delete this;
	}

private:
	DelegateThread* m_thread;
};

template <class Param1, class Param2, class Param3> 
class DelegateFreeAsync3 : public DelegateFree3<Param1, Param2, Param3>, public IDelegateInvoker {
public:
	typedef void (*FreeFunc)(Param1, Param2, Param3);

	DelegateFreeAsync3(FreeFunc func, DelegateThread* thread) { Bind(func, thread); }
	DelegateFreeAsync3() : m_thread(0) { }

	/// Bind a free function to the delegate.
	void Bind(FreeFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateFree3<Param1, Param2, Param3>::Bind(func);	}

	virtual DelegateFreeAsync3<Param1, Param2, Param3>* Clone() const {
		return new DelegateFreeAsync3<Param1, Param2, Param3>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFreeAsync3<Param1, Param2, Param3>* derivedRhs = dynamic_cast<const DelegateFreeAsync3<Param1, Param2, Param3>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateFree3<Param1, Param2, Param3>::operator == (rhs); }

	// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3) {
		if (m_thread == 0)
			DelegateFree3<Param1, Param2, Param3>::operator()(p1, p2, p3);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);
			Param2 heapParam2 = DelegateParam<Param2>::New(p2);
			Param3 heapParam3 = DelegateParam<Param3>::New(p3);

			// Create a clone instance of this delegate 
			DelegateFreeAsync3<Param1, Param2, Param3>* delegate = Clone();

			// Create a new message instance 
			DelegateMsg3<Param1, Param2, Param3>* msg = new DelegateMsg3<Param1, Param2, Param3>(delegate, heapParam1, heapParam2, heapParam3);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			m_thread->DispatchDelegate(msg);
		}
	}

	// Called to invoke the delegate function on the target thread of control
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		// Typecast the base pointer to back to the templatized instance
		DelegateMsg3<Param1, Param2, Param3>* delegateMsg = static_cast<DelegateMsg3<Param1, Param2, Param3>*>(*msg);

		// Get the function parameter data
		Param1 param1 = delegateMsg->GetParam1();
		Param2 param2 = delegateMsg->GetParam2();
		Param3 param3 = delegateMsg->GetParam3();

		// Invoke the delegate function
		DelegateFree3<Param1, Param2, Param3>::operator()(param1, param2, param3);

		// Delete heap data created inside operator()
		DelegateParam<Param1>::Delete(param1);
		DelegateParam<Param2>::Delete(param2);
		DelegateParam<Param3>::Delete(param3);
		delete *msg;
		*msg = 0;

		// Do this last before returning!
		delete this;
	}

private:
	DelegateThread* m_thread;
};

template <class Param1, class Param2, class Param3, class Param4> 
class DelegateFreeAsync4 : public DelegateFree4<Param1, Param2, Param3, Param4>, public IDelegateInvoker {
public:
	typedef void (*FreeFunc)(Param1, Param2, Param3, Param4);

	DelegateFreeAsync4(FreeFunc func, DelegateThread* thread) { Bind(func, thread); }
	DelegateFreeAsync4() : m_thread(0) { }

	/// Bind a free function to the delegate.
	void Bind(FreeFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateFree4<Param1, Param2, Param3, Param4>::Bind(func);	}

	virtual DelegateFreeAsync4<Param1, Param2, Param3, Param4>* Clone() const {
		return new DelegateFreeAsync4<Param1, Param2, Param3, Param4>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFreeAsync4<Param1, Param2, Param3, Param4>* derivedRhs = dynamic_cast<const DelegateFreeAsync4<Param1, Param2, Param3, Param4>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateFree4<Param1, Param2, Param3, Param4>::operator == (rhs); }

	// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
		if (m_thread == 0)
			DelegateFree4<Param1, Param2, Param3, Param4>::operator()(p1, p2, p3, p4);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);
			Param2 heapParam2 = DelegateParam<Param2>::New(p2);
			Param3 heapParam3 = DelegateParam<Param3>::New(p3);
			Param4 heapParam4 = DelegateParam<Param4>::New(p4);

			// Create a clone instance of this delegate 
			DelegateFreeAsync4<Param1, Param2, Param3, Param4>* delegate = Clone();

			// Create a new message instance 
			DelegateMsg4<Param1, Param2, Param3, Param4>* msg = new DelegateMsg4<Param1, Param2, Param3, Param4>(delegate, heapParam1, heapParam2, heapParam3, heapParam4);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			m_thread->DispatchDelegate(msg);
		}
	}

	// Called to invoke the delegate function on the target thread of control
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		// Typecast the base pointer to back to the templatized instance
		DelegateMsg4<Param1, Param2, Param3, Param4>* delegateMsg = static_cast<DelegateMsg4<Param1, Param2, Param3, Param4>*>(*msg);

		// Get the function parameter data
		Param1 param1 = delegateMsg->GetParam1();
		Param2 param2 = delegateMsg->GetParam2();
		Param3 param3 = delegateMsg->GetParam3();
		Param4 param4 = delegateMsg->GetParam4();

		// Invoke the delegate function
		DelegateFree4<Param1, Param2, Param3, Param4>::operator()(param1, param2, param3, param4);

		// Delete heap data created inside operator()
		DelegateParam<Param1>::Delete(param1);
		DelegateParam<Param2>::Delete(param2);
		DelegateParam<Param3>::Delete(param3);
		DelegateParam<Param4>::Delete(param4);
		delete *msg;
		*msg = 0;

		// Do this last before returning!
		delete this;
	}

private:
	DelegateThread* m_thread;
};

template <class Param1, class Param2, class Param3, class Param4, class Param5> 
class DelegateFreeAsync5 : public DelegateFree5<Param1, Param2, Param3, Param4, Param5>, public IDelegateInvoker {
public:
	typedef void (*FreeFunc)(Param1, Param2, Param3, Param4, Param5);

	DelegateFreeAsync5(FreeFunc func, DelegateThread* thread) { Bind(func, thread); }
	DelegateFreeAsync5() : m_thread(0) { }

	/// Bind a free function to the delegate.
	void Bind(FreeFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateFree5<Param1, Param2, Param3, Param4, Param5>::Bind(func);	}

	virtual DelegateFreeAsync5<Param1, Param2, Param3, Param4, Param5>* Clone() const {
		return new DelegateFreeAsync5<Param1, Param2, Param3, Param4, Param5>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFreeAsync5<Param1, Param2, Param3, Param4, Param5>* derivedRhs = dynamic_cast<const DelegateFreeAsync5<Param1, Param2, Param3, Param4, Param5>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread &&
			DelegateFree5<Param1, Param2, Param3, Param4, Param5>::operator == (rhs); }

	// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
		if (m_thread == 0)
			DelegateFree5<Param1, Param2, Param3, Param4, Param5>::operator()(p1, p2, p3, p4, p5);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);
			Param2 heapParam2 = DelegateParam<Param2>::New(p2);
			Param3 heapParam3 = DelegateParam<Param3>::New(p3);
			Param4 heapParam4 = DelegateParam<Param4>::New(p4);
			Param5 heapParam5 = DelegateParam<Param5>::New(p5);

			// Create a clone instance of this delegate 
			DelegateFreeAsync5<Param1, Param2, Param3, Param4, Param5>* delegate = Clone();

			// Create a new message instance 
			DelegateMsg5<Param1, Param2, Param3, Param4, Param5>* msg = new DelegateMsg5<Param1, Param2, Param3, Param4, Param5>(delegate, heapParam1, heapParam2, heapParam3, heapParam4, heapParam5);

			// Dispatch message onto the callback destination thread. DelegateInvoke()
			// will be called by the target thread. 
			m_thread->DispatchDelegate(msg);
		}
	}

	// Called to invoke the delegate function on the target thread of control
	virtual void DelegateInvoke(DelegateMsgBase** msg) {
		// Typecast the base pointer to back to the templatized instance
		DelegateMsg5<Param1, Param2, Param3, Param4, Param5>* delegateMsg = static_cast<DelegateMsg5<Param1, Param2, Param3, Param4, Param5>*>(*msg);

		// Get the function parameter data
		Param1 param1 = delegateMsg->GetParam1();
		Param2 param2 = delegateMsg->GetParam2();
		Param3 param3 = delegateMsg->GetParam3();
		Param4 param4 = delegateMsg->GetParam4();
		Param5 param5 = delegateMsg->GetParam5();

		// Invoke the delegate function
		DelegateFree5<Param1, Param2, Param3, Param4, Param5>::operator()(param1, param2, param3, param4, param5);

		// Delete heap data created inside operator()
		DelegateParam<Param1>::Delete(param1);
		DelegateParam<Param2>::Delete(param2);
		DelegateParam<Param3>::Delete(param3);
		DelegateParam<Param4>::Delete(param4);
		DelegateParam<Param5>::Delete(param5);
		delete *msg;
		*msg = 0;

		// Do this last before returning!
		delete this;
	}

private:
	DelegateThread* m_thread;
};

//N=0
template <class TClass>
DelegateMemberAsync0<TClass> MakeDelegate(TClass* object, void (TClass::*func)(), DelegateThread* thread) { 
	return DelegateMemberAsync0<TClass>(object, func, thread);
}

template <class TClass>
DelegateMemberAsync0<TClass> MakeDelegate(TClass* object, void (TClass::*func)() const, DelegateThread* thread) { 
	return DelegateMemberAsync0<TClass>(object, func, thread);
}

DelegateFreeAsync0 MakeDelegate(void(*func)(), DelegateThread* thread);

//N=1
template <class TClass, class Param1>
DelegateMemberAsync1<TClass, Param1> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1), DelegateThread* thread) { 
	return DelegateMemberAsync1<TClass, Param1>(object, func, thread);
}

template <class TClass, class Param1>
DelegateMemberAsync1<TClass, Param1> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1) const, DelegateThread* thread) { 
	return DelegateMemberAsync1<TClass, Param1>(object, func, thread);
}

template <class Param1>
DelegateFreeAsync1<Param1> MakeDelegate(void (*func)(Param1 p1), DelegateThread* thread) { 
	return DelegateFreeAsync1<Param1>(func, thread);
}

//N=2
template <class TClass, class Param1, class Param2>
DelegateMemberAsync2<TClass, Param1, Param2> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2), DelegateThread* thread) { 
	return DelegateMemberAsync2<TClass, Param1, Param2>(object, func, thread);
}

template <class TClass, class Param1, class Param2>
DelegateMemberAsync2<TClass, Param1, Param2> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2) const, DelegateThread* thread) { 
	return DelegateMemberAsync2<TClass, Param1, Param2>(object, func, thread);
}

template <class Param1, class Param2>
DelegateFreeAsync2<Param1, Param2> MakeDelegate(void (*func)(Param1 p1, Param2 p2), DelegateThread* thread) { 
	return DelegateFreeAsync2<Param1, Param2>(func, thread);
}

//N=3
template <class TClass, class Param1, class Param2, class Param3>
DelegateMemberAsync3<TClass, Param1, Param2, Param3> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3), DelegateThread* thread) { 
	return DelegateMemberAsync3<TClass, Param1, Param2, Param3>(object, func, thread);
}

template <class TClass, class Param1, class Param2, class Param3>
DelegateMemberAsync3<TClass, Param1, Param2, Param3> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3) const, DelegateThread* thread) { 
	return DelegateMemberAsync3<TClass, Param1, Param2, Param3>(object, func, thread);
}

template <class Param1, class Param2, class Param3>
DelegateFreeAsync3<Param1, Param2, Param3> MakeDelegate(void (*func)(Param1 p1, Param2 p2, Param3 p3), DelegateThread* thread) { 
	return DelegateFreeAsync3<Param1, Param2, Param3>(func, thread);
}

//N=4
template <class TClass, class Param1, class Param2, class Param3, class Param4>
DelegateMemberAsync4<TClass, Param1, Param2, Param3, Param4> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4), DelegateThread* thread) { 
	return DelegateMemberAsync4<TClass, Param1, Param2, Param3, Param4>(object, func, thread);
}

template <class TClass, class Param1, class Param2, class Param3, class Param4>
DelegateMemberAsync4<TClass, Param1, Param2, Param3, Param4> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const, DelegateThread* thread) { 
	return DelegateMemberAsync4<TClass, Param1, Param2, Param3, Param4>(object, func, thread);
}

template <class Param1, class Param2, class Param3, class Param4>
DelegateFreeAsync4<Param1, Param2, Param3, Param4> MakeDelegate(void (*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4), DelegateThread* thread) { 
	return DelegateFreeAsync4<Param1, Param2, Param3, Param4>(func, thread);
}

//N=5
template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5>
DelegateMemberAsync5<TClass, Param1, Param2, Param3, Param4, Param5> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5), DelegateThread* thread) { 
	return DelegateMemberAsync5<TClass, Param1, Param2, Param3, Param4, Param5>(object, func, thread);
}

template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5>
DelegateMemberAsync5<TClass, Param1, Param2, Param3, Param4, Param5> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const, DelegateThread* thread) { 
	return DelegateMemberAsync5<TClass, Param1, Param2, Param3, Param4, Param5>(object, func, thread);
}

template <class Param1, class Param2, class Param3, class Param4, class Param5>
DelegateFreeAsync5<Param1, Param2, Param3, Param4, Param5> MakeDelegate(void (*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5), DelegateThread* thread) { 
	return DelegateFreeAsync5<Param1, Param2, Param3, Param4, Param5>(func, thread);
}

}

#endif
