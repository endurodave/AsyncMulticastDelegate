#ifndef _DELEGATE_SP_ASYNC_H
#define _DELEGATE_SP_ASYNC_H

// Delegate.h
// @see https://www.codeproject.com/Articles/1160934/Asynchronous-Multicast-Delegates-in-Cplusplus
// David Lafreniere, Dec 2016.
//
// The DelegateMemberSpX delegate implemenations asynchronously bind and invoke class instance member functions. 
// The std::shared_ptr<TClass> is used in lieu of a raw TClass* pointer. 

#include "DelegateSp.h"
#include "DelegateThread.h"
#include "DelegateInvoker.h"

namespace DelegateLib {

/// @brief Asynchronous memeber delegate that invokes the target function on the specified thread of control.
template <class TClass> 
class DelegateMemberSpAsync0 : public DelegateMemberSp0<TClass>, public IDelegateInvoker {
public:
	typedef std::shared_ptr<TClass> ObjectPtr;
	typedef void (TClass::*MemberFunc)();
	typedef void (TClass::*ConstMemberFunc)() const;

	// Contructors take a class instance, member function, and delegate thread
	DelegateMemberSpAsync0(ObjectPtr object, MemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberSpAsync0(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberSpAsync0() : m_thread(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateMemberSp0<TClass>::Bind(object, func); }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)	{
		m_thread = thread;
		DelegateMemberSp0<TClass>::Bind(object, func); }

	virtual DelegateMemberSpAsync0<TClass>* Clone() const {
		return new DelegateMemberSpAsync0<TClass>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberSpAsync0<TClass>* derivedRhs = dynamic_cast<const DelegateMemberSpAsync0<TClass>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread && 
			DelegateMemberSp0<TClass>::operator == (rhs); }

	/// Invoke delegate function asynchronously
	virtual void operator()() {
		if (m_thread == 0)
			DelegateMemberSp0<TClass>::operator()();
		else
		{
			// Create a clone instance of this delegate 
			DelegateMemberSpAsync0<TClass>* delegate = Clone();

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
		DelegateMemberSp0<TClass>::operator()();

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
class DelegateMemberSpAsync1 : public DelegateMemberSp1<TClass, Param1>, public IDelegateInvoker {
public:
	typedef std::shared_ptr<TClass> ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1);
	typedef void (TClass::*ConstMemberFunc)(Param1) const;

	// Contructors take a class instance, member function, and callback thread
	DelegateMemberSpAsync1(ObjectPtr object, MemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberSpAsync1(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberSpAsync1() : m_thread(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateMemberSp1<TClass, Param1>::Bind(object, func); }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)	{
		m_thread = thread;
		DelegateMemberSp1<TClass, Param1>::Bind(object, func); }

	virtual DelegateMemberSpAsync1<TClass, Param1>* Clone() const {
		return new DelegateMemberSpAsync1<TClass, Param1>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberSpAsync1<TClass, Param1>* derivedRhs = dynamic_cast<const DelegateMemberSpAsync1<TClass, Param1>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread && 
			DelegateMemberSp1<TClass, Param1>::operator == (rhs); }

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1) {
		if (m_thread == 0)
			DelegateMemberSp1<TClass, Param1>::operator()(p1);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);

			// Create a clone instance of this delegate 
			DelegateMemberSpAsync1<TClass, Param1>* delegate = Clone();

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
		DelegateMemberSp1<TClass, Param1>::operator()(param1);

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
class DelegateMemberSpAsync2 : public DelegateMemberSp2<TClass, Param1, Param2>, public IDelegateInvoker {
public:
	typedef std::shared_ptr<TClass> ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1, Param2);
	typedef void (TClass::*ConstMemberFunc)(Param1, Param2) const;

	// Contructors take a class instance, member function, and callback thread
	DelegateMemberSpAsync2(ObjectPtr object, MemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberSpAsync2(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberSpAsync2() : m_thread(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateMemberSp2<TClass, Param1, Param2>::Bind(object, func); }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)	{
		m_thread = thread;
		DelegateMemberSp2<TClass, Param1, Param2>::Bind(object, func); }

	virtual DelegateMemberSpAsync2<TClass, Param1, Param2>* Clone() const {
		return new DelegateMemberSpAsync2<TClass, Param1, Param2>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberSpAsync2<TClass, Param1, Param2>* derivedRhs = dynamic_cast<const DelegateMemberSpAsync2<TClass, Param1, Param2>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread && 
			DelegateMemberSp2<TClass, Param1, Param2>::operator == (rhs); }

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2) {
		if (m_thread == 0)
			DelegateMemberSp2<TClass, Param1, Param2>::operator()(p1, p2);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);
			Param2 heapParam2 = DelegateParam<Param2>::New(p2);

			// Create a clone instance of this delegate 
			DelegateMemberSpAsync2<TClass, Param1, Param2>* delegate = Clone();

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
		DelegateMemberSp2<TClass, Param1, Param2>::operator()(param1, param2);

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
class DelegateMemberSpAsync3 : public DelegateMemberSp3<TClass, Param1, Param2, Param3>, public IDelegateInvoker {
public:
	typedef std::shared_ptr<TClass> ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1, Param2, Param3);
	typedef void (TClass::*ConstMemberFunc)(Param1, Param2, Param3) const;

	// Contructors take a class instance, member function, and callback thread
	DelegateMemberSpAsync3(ObjectPtr object, MemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberSpAsync3(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberSpAsync3() : m_thread(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateMemberSp3<TClass, Param1, Param2, Param3>::Bind(object, func); }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)	{
		m_thread = thread;
		DelegateMemberSp3<TClass, Param1, Param2, Param3>::Bind(object, func); }

	virtual DelegateMemberSpAsync3<TClass, Param1, Param2, Param3>* Clone() const {
		return new DelegateMemberSpAsync3<TClass, Param1, Param2, Param3>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberSpAsync3<TClass, Param1, Param2, Param3>* derivedRhs = dynamic_cast<const DelegateMemberSpAsync3<TClass, Param1, Param2, Param3>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread && 
			DelegateMemberSp3<TClass, Param1, Param2, Param3>::operator == (rhs); }

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3) {
		if (m_thread == 0)
			DelegateMemberSp3<TClass, Param1, Param2, Param3>::operator()(p1, p2, p3);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);
			Param2 heapParam2 = DelegateParam<Param2>::New(p2);
			Param3 heapParam3 = DelegateParam<Param3>::New(p3);

			// Create a clone instance of this delegate 
			DelegateMemberSpAsync3<TClass, Param1, Param2, Param3>* delegate = Clone();

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
		DelegateMemberSp3<TClass, Param1, Param2, Param3>::operator()(param1, param2, param3);

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
class DelegateMemberSpAsync4 : public DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4>, public IDelegateInvoker {
public:
	typedef std::shared_ptr<TClass> ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1, Param2, Param3, Param4);
	typedef void (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4) const;

	// Contructors take a class instance, member function, and callback thread
	DelegateMemberSpAsync4(ObjectPtr object, MemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberSpAsync4(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberSpAsync4() : m_thread(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4>::Bind(object, func); }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)	{
		m_thread = thread;
		DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4>::Bind(object, func); }

	virtual DelegateMemberSpAsync4<TClass, Param1, Param2, Param3, Param4>* Clone() const {
		return new DelegateMemberSpAsync4<TClass, Param1, Param2, Param3, Param4>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberSpAsync4<TClass, Param1, Param2, Param3, Param4>* derivedRhs = dynamic_cast<const DelegateMemberSpAsync4<TClass, Param1, Param2, Param3, Param4>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread && 
			DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4>::operator == (rhs); }

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
		if (m_thread == 0)
			DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4>::operator()(p1, p2, p3, p4);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);
			Param2 heapParam2 = DelegateParam<Param2>::New(p2);
			Param3 heapParam3 = DelegateParam<Param3>::New(p3);
			Param4 heapParam4 = DelegateParam<Param4>::New(p4);

			// Create a clone instance of this delegate 
			DelegateMemberSpAsync4<TClass, Param1, Param2, Param3, Param4>* delegate = Clone();

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
		DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4>::operator()(param1, param2, param3, param4);

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
class DelegateMemberSpAsync5 : public DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5>, public IDelegateInvoker {
public:
	typedef std::shared_ptr<TClass> ObjectPtr;
	typedef void (TClass::*MemberFunc)(Param1, Param2, Param3, Param4, Param5);
	typedef void (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4, Param5) const;

	// Contructors take a class instance, member function, and callback thread
	DelegateMemberSpAsync5(ObjectPtr object, MemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberSpAsync5(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread) { Bind(object, func, thread); }
	DelegateMemberSpAsync5() : m_thread(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func, DelegateThread* thread) {
		m_thread = thread; 
		DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5>::Bind(object, func); }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func, DelegateThread* thread)	{
		m_thread = thread;
		DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5>::Bind(object, func); }

	virtual DelegateMemberSpAsync5<TClass, Param1, Param2, Param3, Param4, Param5>* Clone() const {
		return new DelegateMemberSpAsync5<TClass, Param1, Param2, Param3, Param4, Param5>(*this); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberSpAsync5<TClass, Param1, Param2, Param3, Param4, Param5>* derivedRhs = dynamic_cast<const DelegateMemberSpAsync5<TClass, Param1, Param2, Param3, Param4, Param5>*>(&rhs);
		return derivedRhs &&
			m_thread == derivedRhs->m_thread && 
			DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5>::operator == (rhs); }

	/// Invoke delegate function asynchronously
	virtual void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
		if (m_thread == 0)
			DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5>::operator()(p1, p2, p3, p4, p5);
		else
		{
			// Create a new instance of the function argument data and copy
			Param1 heapParam1 = DelegateParam<Param1>::New(p1);
			Param2 heapParam2 = DelegateParam<Param2>::New(p2);
			Param3 heapParam3 = DelegateParam<Param3>::New(p3);
			Param4 heapParam4 = DelegateParam<Param4>::New(p4);
			Param5 heapParam5 = DelegateParam<Param5>::New(p5);

			// Create a clone instance of this delegate 
			DelegateMemberSpAsync5<TClass, Param1, Param2, Param3, Param4, Param5>* delegate = Clone();

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
		DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5>::operator()(param1, param2, param3, param4, param5);

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

//N=0
template <class TClass>
DelegateMemberSpAsync0<TClass> MakeDelegate(std::shared_ptr<TClass> object, void (TClass::*func)(), DelegateThread* thread) { 
	return DelegateMemberSpAsync0<TClass>(object, func, thread);
}

template <class TClass>
DelegateMemberSpAsync0<TClass> MakeDelegate(std::shared_ptr<TClass> object, void (TClass::*func)() const, DelegateThread* thread) { 
	return DelegateMemberSpAsync0<TClass>(object, func, thread);
}

//N=1
template <class TClass, class Param1>
DelegateMemberSpAsync1<TClass, Param1> MakeDelegate(std::shared_ptr<TClass> object, void (TClass::*func)(Param1 p1), DelegateThread* thread) { 
	return DelegateMemberSpAsync1<TClass, Param1>(object, func, thread);
}

template <class TClass, class Param1>
DelegateMemberSpAsync1<TClass, Param1> MakeDelegate(std::shared_ptr<TClass> object, void (TClass::*func)(Param1 p1) const, DelegateThread* thread) { 
	return DelegateMemberSpAsync1<TClass, Param1>(object, func, thread);
}

//N=2
template <class TClass, class Param1, class Param2>
DelegateMemberSpAsync2<TClass, Param1, Param2> MakeDelegate(std::shared_ptr<TClass> object, void (TClass::*func)(Param1 p1, Param2 p2), DelegateThread* thread) { 
	return DelegateMemberSpAsync2<TClass, Param1, Param2>(object, func, thread);
}

template <class TClass, class Param1, class Param2>
DelegateMemberSpAsync2<TClass, Param1, Param2> MakeDelegate(std::shared_ptr<TClass> object, void (TClass::*func)(Param1 p1, Param2 p2) const, DelegateThread* thread) { 
	return DelegateMemberSpAsync2<TClass, Param1, Param2>(object, func, thread);
}

//N=3
template <class TClass, class Param1, class Param2, class Param3>
DelegateMemberSpAsync3<TClass, Param1, Param2, Param3> MakeDelegate(std::shared_ptr<TClass> object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3), DelegateThread* thread) { 
	return DelegateMemberSpAsync3<TClass, Param1, Param2, Param3>(object, func, thread);
}

template <class TClass, class Param1, class Param2, class Param3>
DelegateMemberSpAsync3<TClass, Param1, Param2, Param3> MakeDelegate(std::shared_ptr<TClass> object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3) const, DelegateThread* thread) { 
	return DelegateMemberSpAsync3<TClass, Param1, Param2, Param3>(object, func, thread);
}

//N=4
template <class TClass, class Param1, class Param2, class Param3, class Param4>
DelegateMemberSpAsync4<TClass, Param1, Param2, Param3, Param4> MakeDelegate(std::shared_ptr<TClass> object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4), DelegateThread* thread) { 
	return DelegateMemberSpAsync4<TClass, Param1, Param2, Param3, Param4>(object, func, thread);
}

template <class TClass, class Param1, class Param2, class Param3, class Param4>
DelegateMemberSpAsync4<TClass, Param1, Param2, Param3, Param4> MakeDelegate(std::shared_ptr<TClass> object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const, DelegateThread* thread) { 
	return DelegateMemberSpAsync4<TClass, Param1, Param2, Param3, Param4>(object, func, thread);
}

//N=5
template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5>
DelegateMemberSpAsync5<TClass, Param1, Param2, Param3, Param4, Param5> MakeDelegate(std::shared_ptr<TClass> object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5), DelegateThread* thread) { 
	return DelegateMemberSpAsync5<TClass, Param1, Param2, Param3, Param4, Param5>(object, func, thread);
}

template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5>
DelegateMemberSpAsync5<TClass, Param1, Param2, Param3, Param4, Param5> MakeDelegate(std::shared_ptr<TClass> object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const, DelegateThread* thread) { 
	return DelegateMemberSpAsync5<TClass, Param1, Param2, Param3, Param4, Param5>(object, func, thread);
}

}

#endif
