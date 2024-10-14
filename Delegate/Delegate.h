#ifndef _DELEGATE_H
#define _DELEGATE_H

// Delegate.h
// @see https://github.com/endurodave/AsyncMulticastDelegate
// David Lafreniere, Dec 2016.

#if USE_XALLOCATOR
	#include "xallocator.h"
#endif

namespace DelegateLib {

/// @brief Non-template common base class for all delegates.
class DelegateBase {
#if USE_XALLOCATOR
	XALLOCATOR
#endif
public:
	virtual ~DelegateBase() {}

	/// Derived class must implement operator== to compare objects.
	virtual bool operator==(const DelegateBase& rhs) const = 0;
	virtual bool operator!=(const DelegateBase& rhs) { return !(*this == rhs); }

	/// Use Clone to provide a deep copy using a base pointer. Covariant 
	/// overloading is used so that a Clone() method return type is a
	/// more specific type in the derived class implementations.
	/// @return A dynamic copy of this instance created with operator new. 
	/// @post The caller is responsible for deleting the clone instance. 
	virtual DelegateBase* Clone() const = 0;
};

/// @brief Abstract delegate template base class.
template <class RetType=void>
class Delegate0 : public DelegateBase {
public:
	virtual RetType operator()() = 0;
	virtual Delegate0* Clone() const = 0;
};

template <class Param1, class RetType=void>
class Delegate1 : public DelegateBase {
public:
	virtual RetType operator()(Param1 p1) = 0;
	virtual Delegate1* Clone() const = 0;
};

template <class Param1, class Param2, class RetType=void>
class Delegate2 : public DelegateBase {
public:
	virtual RetType operator()(Param1 p1, Param2 p2) = 0;
	virtual Delegate2* Clone() const = 0;
};

template <class Param1, class Param2, class Param3, class RetType=void>
class Delegate3 : public DelegateBase {
public:
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3) = 0;
	virtual Delegate3* Clone() const = 0;
};

template <class Param1, class Param2, class Param3, class Param4, class RetType=void>
class Delegate4 : public DelegateBase {
public:
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) = 0;
	virtual Delegate4* Clone() const = 0;
};

template <class Param1, class Param2, class Param3, class Param4, class Param5, class RetType=void>
class Delegate5 : public DelegateBase {
public:
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) = 0;
	virtual Delegate5* Clone() const = 0;
};

/// @brief DelegateMember is used to store and invoke an instance member function.
template <class TClass, class RetType=void> 
class DelegateMember0 : public Delegate0<RetType> {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(); 
	typedef RetType (TClass::*ConstMemberFunc)() const; 

	DelegateMember0(ObjectPtr object, MemberFunc func) { Bind(object, func); }
	DelegateMember0(ObjectPtr object, ConstMemberFunc func) { Bind(object, func); }
	DelegateMember0() :	m_object(0), m_func(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func) {
		m_object = object;
		m_func = func; }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func)	{
		m_object = object;
		m_func = reinterpret_cast<MemberFunc>(func); }

	virtual DelegateMember0* Clone() const { return new DelegateMember0<TClass, RetType>(*this); }

	// Invoke the bound delegate function
	virtual RetType operator()() {
		return (*m_object.*m_func)(); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMember0<TClass, RetType>* derivedRhs = dynamic_cast<const DelegateMember0<TClass, RetType>*>(&rhs);
		return derivedRhs &&
			m_func == derivedRhs->m_func && 
			m_object == derivedRhs->m_object; }

	bool Empty() const { return !(m_object && m_func); }
	void Clear() { m_object = 0; m_func = 0; }

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (DelegateMember0::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateMember0::this_type_does_not_support_comparisons; }
	bool operator !() const { return !(m_object && m_func); }
#endif

private:
	ObjectPtr m_object;		// Pointer to a class object
	MemberFunc m_func;   	// Pointer to an instance member function
};

template <class TClass, class Param1, class RetType=void> 
class DelegateMember1 : public Delegate1<Param1, RetType> {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1); 
	typedef RetType (TClass::*ConstMemberFunc)(Param1) const; 

	DelegateMember1(ObjectPtr object, MemberFunc func) { Bind(object, func); }
	DelegateMember1(ObjectPtr object, ConstMemberFunc func) { Bind(object, func);	}
	DelegateMember1() :	m_object(0), m_func(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func) {
		m_object = object;
		m_func = func; }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func)	{
		m_object = object;
		m_func = reinterpret_cast<MemberFunc>(func); }

	virtual DelegateMember1* Clone() const { return new DelegateMember1(*this); }

	// Invoke the bound delegate function
	virtual RetType operator()(Param1 p1) {
		return (*m_object.*m_func)(p1); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMember1<TClass, Param1, RetType>* derivedRhs = dynamic_cast<const DelegateMember1<TClass, Param1, RetType>*>(&rhs);
		return derivedRhs &&
			m_func == derivedRhs->m_func && 
			m_object == derivedRhs->m_object; }

	bool Empty() const { return !(m_object && m_func); }
	void Clear() { m_object = 0; m_func = 0; }

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (DelegateMember1::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateMember1::this_type_does_not_support_comparisons; }
	bool operator !() const { return !(m_object && m_func); }
#endif

private:
	ObjectPtr m_object;		// Pointer to a class object
	MemberFunc m_func;   	// Pointer to an instance member function
};

template <class TClass, class Param1, class Param2, class RetType=void> 
class DelegateMember2 : public Delegate2<Param1, Param2, RetType> {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2); 
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2) const; 

	DelegateMember2(ObjectPtr object, MemberFunc func) { Bind(object, func); }
	DelegateMember2(ObjectPtr object, ConstMemberFunc func) { Bind(object, func);	}
	DelegateMember2() :	m_object(0), m_func(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func) {
		m_object = object;
		m_func = func; }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func)	{
		m_object = object;
		m_func = reinterpret_cast<MemberFunc>(func); }

	virtual DelegateMember2* Clone() const { return new DelegateMember2(*this); }

	// Invoke the bound delegate function
	virtual RetType operator()(Param1 p1, Param2 p2) {
		return (*m_object.*m_func)(p1, p2); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMember2<TClass, Param1, Param2, RetType>* derivedRhs = dynamic_cast<const DelegateMember2<TClass, Param1, Param2, RetType>*>(&rhs);
		return derivedRhs &&
			m_func == derivedRhs->m_func && 
			m_object == derivedRhs->m_object; }

	bool Empty() const { return !(m_object && m_func); }
	void Clear() { m_object = 0; m_func = 0; }

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (DelegateMember2::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateMember2::this_type_does_not_support_comparisons; }
	bool operator !() const { return !(m_object && m_func); }
#endif

private:
	ObjectPtr m_object;		// Pointer to a class object
	MemberFunc m_func;   	// Pointer to an instance member function
};

template <class TClass, class Param1, class Param2, class Param3, class RetType=void> 
class DelegateMember3 : public Delegate3<Param1, Param2, Param3, RetType> {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2, Param3); 
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2, Param3) const; 

	DelegateMember3(ObjectPtr object, MemberFunc func) { Bind(object, func); }
	DelegateMember3(ObjectPtr object, ConstMemberFunc func) { Bind(object, func);	}
	DelegateMember3() :	m_object(0), m_func(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func) {
		m_object = object;
		m_func = func; }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func)	{
		m_object = object;
		m_func = reinterpret_cast<MemberFunc>(func); }

	virtual DelegateMember3* Clone() const { return new DelegateMember3(*this); }

	// Invoke the bound delegate function
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3) {
		return (*m_object.*m_func)(p1, p2, p3); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMember3<TClass, Param1, Param2, Param3, RetType>* derivedRhs = dynamic_cast<const DelegateMember3<TClass, Param1, Param2, Param3, RetType>*>(&rhs);
		return derivedRhs &&
			m_func == derivedRhs->m_func && 
			m_object == derivedRhs->m_object; }

	bool Empty() const { return !(m_object && m_func); }
	void Clear() { m_object = 0; m_func = 0; }

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (DelegateMember3::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateMember3::this_type_does_not_support_comparisons; }
	bool operator !() const { return !(m_object && m_func); }
#endif

private:
	ObjectPtr m_object;		// Pointer to a class object
	MemberFunc m_func;   	// Pointer to an instance member function
};

template <class TClass, class Param1, class Param2, class Param3, class Param4, class RetType=void> 
class DelegateMember4 : public Delegate4<Param1, Param2, Param3, Param4, RetType> {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2, Param3, Param4); 
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4) const; 

	DelegateMember4(ObjectPtr object, MemberFunc func) { Bind(object, func); }
	DelegateMember4(ObjectPtr object, ConstMemberFunc func) { Bind(object, func);	}
	DelegateMember4() :	m_object(0), m_func(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func) {
		m_object = object;
		m_func = func; }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func)	{
		m_object = object;
		m_func = reinterpret_cast<MemberFunc>(func); }

	virtual DelegateMember4* Clone() const { return new DelegateMember4(*this); }

	// Invoke the bound delegate function
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
		return (*m_object.*m_func)(p1, p2, p3, p4); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMember4<TClass, Param1, Param2, Param3, Param4, RetType>* derivedRhs = dynamic_cast<const DelegateMember4<TClass, Param1, Param2, Param3, Param4, RetType>*>(&rhs);
		return derivedRhs &&
			m_func == derivedRhs->m_func && 
			m_object == derivedRhs->m_object; }

	bool Empty() const { return !(m_object && m_func); }
	void Clear() { m_object = 0; m_func = 0; }

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (DelegateMember4::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateMember4::this_type_does_not_support_comparisons; }
	bool operator !() const { return !(m_object && m_func); }
#endif

private:
	ObjectPtr m_object;		// Pointer to a class object
	MemberFunc m_func;   	// Pointer to an instance member function
};

template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5, class RetType=void> 
class DelegateMember5 : public Delegate5<Param1, Param2, Param3, Param4, Param5, RetType> {
public:
	typedef TClass* ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2, Param3, Param4, Param5); 
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4, Param5) const; 

	DelegateMember5(ObjectPtr object, MemberFunc func) { Bind(object, func); }
	DelegateMember5(ObjectPtr object, ConstMemberFunc func) { Bind(object, func);	}
	DelegateMember5() :	m_object(0), m_func(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func) {
		m_object = object;
		m_func = func; }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func)	{
		m_object = object;
		m_func = reinterpret_cast<MemberFunc>(func); }

	virtual DelegateMember5* Clone() const { return new DelegateMember5(*this); }

	// Invoke the bound delegate function
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
		return (*m_object.*m_func)(p1, p2, p3, p4, p5); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>* derivedRhs = dynamic_cast<const DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>*>(&rhs);
		return derivedRhs &&
			m_func == derivedRhs->m_func && 
			m_object == derivedRhs->m_object; }

	bool Empty() const { return !(m_object && m_func); }
	void Clear() { m_object = 0; m_func = 0; }

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (DelegateMember5::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateMember5::this_type_does_not_support_comparisons; }
	bool operator !() const { return !(m_object && m_func); }
#endif

private:
	ObjectPtr m_object;		// Pointer to a class object
	MemberFunc m_func;   	// Pointer to an instance member function
};

/// @brief DelegateFree is used to store and invoke any non-member function 
/// (i.e. a static member function or a global function). 
template <class RetType=void> 
class DelegateFree0 : public Delegate0<RetType> {
public:
	typedef RetType (*FreeFunc)(); 

	DelegateFree0(FreeFunc func) { Bind(func); }
	DelegateFree0() : m_func(0) { }

	/// Bind a free function to the delegate.
	void Bind(FreeFunc func) { m_func = func; }

	virtual DelegateFree0* Clone() const { return new DelegateFree0(*this); }

	/// Invoke the bound delegate function. 
	virtual RetType operator()() {
		return (*m_func)(); }

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFree0<RetType>* derivedRhs = dynamic_cast<const DelegateFree0<RetType>*>(&rhs);
		return derivedRhs &&
			m_func == derivedRhs->m_func; }

	bool Empty() const { return !m_func; }
	void Clear() { m_func = 0; }

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (DelegateFree0::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateFree0::this_type_does_not_support_comparisons; }
	bool operator !() const { return !m_func; }
#endif

private:
	FreeFunc m_func;		// Pointer to a free function
};

template <class Param1, class RetType=void> 
class DelegateFree1 : public Delegate1<Param1, RetType> {
public:
	typedef RetType (*FreeFunc)(Param1); 

	DelegateFree1(FreeFunc func) { Bind(func); }
	DelegateFree1() : m_func(0) { }

	/// Bind a free function to the delegate.
	void Bind(FreeFunc func) { m_func = func; }

	virtual DelegateFree1* Clone() const { return new DelegateFree1(*this); }

	/// Invoke the bound delegate function. 
	virtual RetType operator()(Param1 p1) {
		return (*m_func)(p1); }

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFree1<Param1, RetType>* derivedRhs = dynamic_cast<const DelegateFree1<Param1, RetType>*>(&rhs);
		return derivedRhs &&
			m_func == derivedRhs->m_func; }

	bool Empty() const { return !m_func; }
	void Clear() { m_func = 0; }

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (DelegateFree1::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateFree1::this_type_does_not_support_comparisons; }
	bool operator !() const { return !m_func; }
#endif

private:
	FreeFunc m_func;		// Pointer to a free function
};

template <class Param1, class Param2, class RetType=void> 
class DelegateFree2 : public Delegate2<Param1, Param2, RetType> {
public:
	typedef RetType (*FreeFunc)(Param1, Param2); 

	DelegateFree2(FreeFunc func) { Bind(func); }
	DelegateFree2() : m_func(0) { }

	/// Bind a free function to the delegate.
	void Bind(FreeFunc func) {
		m_func = func; }

	virtual DelegateFree2* Clone() const { return new DelegateFree2(*this); }

	/// Invoke the bound delegate function. 
	virtual RetType operator()(Param1 p1, Param2 p2) {
		return (*m_func)(p1, p2); }

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFree2<Param1, Param2, RetType>* derivedRhs = dynamic_cast<const DelegateFree2<Param1, Param2, RetType>*>(&rhs);
		return derivedRhs &&
			m_func == derivedRhs->m_func; }

	bool Empty() const { return !m_func; }
	void Clear() { m_func = 0; }

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (DelegateFree2::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateFree2::this_type_does_not_support_comparisons; }
	bool operator !() const { return !m_func; }
#endif

private:
	FreeFunc m_func;		// Pointer to a free function
};

template <class Param1, class Param2, class Param3, class RetType=void> 
class DelegateFree3 : public Delegate3<Param1, Param2, Param3, RetType> {
public:
	typedef RetType (*FreeFunc)(Param1, Param2, Param3); 

	DelegateFree3(FreeFunc func) { Bind(func); }
	DelegateFree3() : m_func(0) { }

	/// Bind a free function to the delegate.
	void Bind(FreeFunc func) {
		m_func = func; }

	virtual DelegateFree3* Clone() const { return new DelegateFree3(*this); }

	/// Invoke the bound delegate function. 
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3) {
		return (*m_func)(p1, p2, p3); }

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFree3<Param1, Param2, Param3, RetType>* derivedRhs = dynamic_cast<const DelegateFree3<Param1, Param2, Param3, RetType>*>(&rhs);
		return derivedRhs &&
			m_func == derivedRhs->m_func; }

	bool Empty() const { return !m_func; }
	void Clear() { m_func = 0; }

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (DelegateFree3::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateFree3::this_type_does_not_support_comparisons; }
	bool operator !() const { return !m_func; }
#endif

private:
	FreeFunc m_func;		// Pointer to a free function
};

template <class Param1, class Param2, class Param3, class Param4, class RetType=void> 
class DelegateFree4 : public Delegate4<Param1, Param2, Param3, Param4, RetType> {
public:
	typedef RetType (*FreeFunc)(Param1, Param2, Param3, Param4); 

	DelegateFree4(FreeFunc func) { Bind(func); }
	DelegateFree4() : m_func(0) { }

	/// Bind a free function to the delegate.
	void Bind(FreeFunc func) {
		m_func = func; }

	virtual DelegateFree4* Clone() const { return new DelegateFree4(*this); }

	/// Invoke the bound delegate function. 
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
		return (*m_func)(p1, p2, p3, p4); }

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFree4<Param1, Param2, Param3, Param4, RetType>* derivedRhs = dynamic_cast<const DelegateFree4<Param1, Param2, Param3, Param4, RetType>*>(&rhs);
		return derivedRhs &&
			m_func == derivedRhs->m_func; }

	bool Empty() const { return !m_func; }
	void Clear() { m_func = 0; }

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (DelegateFree4::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateFree4::this_type_does_not_support_comparisons; }
	bool operator !() const { return !m_func; }
#endif

private:
	FreeFunc m_func;		// Pointer to a free function
};

template <class Param1, class Param2, class Param3, class Param4, class Param5, class RetType=void> 
class DelegateFree5 : public Delegate5<Param1, Param2, Param3, Param4, Param5, RetType> {
public:
	typedef RetType (*FreeFunc)(Param1, Param2, Param3, Param4, Param5); 

	DelegateFree5(FreeFunc func) { Bind(func); }
	DelegateFree5() : m_func(0) { }

	/// Bind a free function to the delegate.
	void Bind(FreeFunc func) {
		m_func = func; }

	virtual DelegateFree5* Clone() const {
		return new DelegateFree5(*this); }

	/// Invoke the bound delegate function. 
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
		return (*m_func)(p1, p2, p3, p4, p5); }

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateFree5<Param1, Param2, Param3, Param4, Param5, RetType>* derivedRhs = dynamic_cast<const DelegateFree5<Param1, Param2, Param3, Param4, Param5, RetType>*>(&rhs);
		return derivedRhs &&
			m_func == derivedRhs->m_func; }

	bool Empty() const { return !m_func; }
	void Clear() { m_func = 0; }

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (DelegateFree5::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateFree5::this_type_does_not_support_comparisons; }
	bool operator !() const { return !m_func; }
#endif

private:
	FreeFunc m_func;		// Pointer to a free function
};

// MakeDelegate function creates a delegate object. C++ template argument deduction
// means you can call MakeDelegate without manually specifying the template parameters. 

//N=0
template <class TClass, class RetType>
DelegateMember0<TClass, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)()) { 
	return DelegateMember0<TClass, RetType>(object, func);
}

template <class TClass, class RetType>
DelegateMember0<TClass, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)() const) { 
	return DelegateMember0<TClass, RetType>(object, func);
}

template <class RetType>
DelegateFree0<RetType> MakeDelegate(RetType (*func)()) { 
	return DelegateFree0<RetType>(func);
}

//N=1
template <class TClass, class Param1, class RetType>
DelegateMember1<TClass, Param1, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1)) { 
	return DelegateMember1<TClass, Param1, RetType>(object, func);
}

template <class TClass, class Param1, class RetType>
DelegateMember1<TClass, Param1, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1) const) { 
	return DelegateMember1<TClass, Param1, RetType>(object, func);
}

template <class Param1, class RetType>
DelegateFree1<Param1, RetType> MakeDelegate(RetType (*func)(Param1 p1)) { 
	return DelegateFree1<Param1, RetType>(func);
}

//N=2
template <class TClass, class Param1, class Param2, class RetType>
DelegateMember2<TClass, Param1, Param2, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2)) { 
	return DelegateMember2<TClass, Param1, Param2, RetType>(object, func);
}

template <class TClass, class Param1, class Param2, class RetType>
DelegateMember2<TClass, Param1, Param2, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2) const) { 
	return DelegateMember2<TClass, Param1, Param2, RetType>(object, func);
}

template <class Param1, class Param2, class RetType>
DelegateFree2<Param1, Param2, RetType> MakeDelegate(RetType (*func)(Param1 p1, Param2 p2)) { 
	return DelegateFree2<Param1, Param2, RetType>(func);
}

//N=3
template <class TClass, class Param1, class Param2, class Param3, class RetType>
DelegateMember3<TClass, Param1, Param2, Param3, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3)) { 
	return DelegateMember3<TClass, Param1, Param2, Param3, RetType>(object, func);
}

template <class TClass, class Param1, class Param2, class Param3, class RetType>
DelegateMember3<TClass, Param1, Param2, Param3, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3) const) { 
	return DelegateMember3<TClass, Param1, Param2, Param3, RetType>(object, func);
}

template <class Param1, class Param2, class Param3, class RetType>
DelegateFree3<Param1, Param2, Param3, RetType> MakeDelegate(RetType (*func)(Param1 p1, Param2 p2, Param3 p3)) { 
	return DelegateFree3<Param1, Param2, Param3, RetType>(func);
}

//N=4
template <class TClass, class Param1, class Param2, class Param3, class Param4, class RetType>
DelegateMember4<TClass, Param1, Param2, Param3, Param4, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4)) { 
	return DelegateMember4<TClass, Param1, Param2, Param3, Param4, RetType>(object, func);
}

template <class TClass, class Param1, class Param2, class Param3, class Param4, class RetType>
DelegateMember4<TClass, Param1, Param2, Param3, Param4, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const) { 
	return DelegateMember4<TClass, Param1, Param2, Param3, Param4, RetType>(object, func);
}

template <class Param1, class Param2, class Param3, class Param4, class RetType>
DelegateFree4<Param1, Param2, Param3, Param4, RetType> MakeDelegate(RetType (*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4)) { 
	return DelegateFree4<Param1, Param2, Param3, Param4, RetType>(func);
}

//N=5
template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5, class RetType>
DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5)) { 
	return DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>(object, func);
}

template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5, class RetType>
DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5, RetType> MakeDelegate(TClass* object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const) { 
	return DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>(object, func);
}

template <class Param1, class Param2, class Param3, class Param4, class Param5, class RetType>
DelegateFree5<Param1, Param2, Param3, Param4, Param5, RetType> MakeDelegate(RetType (*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5)) { 
	return DelegateFree5<Param1, Param2, Param3, Param4, Param5, RetType>(func);
}

}

#endif