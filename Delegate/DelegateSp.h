#ifndef _DELEGATE_SP_H
#define _DELEGATE_SP_H

// Delegate.h
// @see https://www.codeproject.com/Articles/1160934/Asynchronous-Multicast-Delegates-in-Cplusplus
// David Lafreniere, Dec 2016.
//
// The DelegateMemberSpX delegate implemenations synchronously bind and invoke class instance member functions. 
// The std::shared_ptr<TClass> is used in lieu of a raw TClass* pointer. 

#include "Delegate.h"
#include <memory>

namespace DelegateLib {

/// @brief DelegateMemberSp is used to store and invoke an instance member function.
template <class TClass, class RetType=void> 
class DelegateMemberSp0 : public Delegate0<RetType> {
public:
	typedef std::shared_ptr<TClass> ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(); 
	typedef RetType (TClass::*ConstMemberFunc)() const; 

	DelegateMemberSp0(ObjectPtr object, MemberFunc func) { Bind(object, func); }
	DelegateMemberSp0(ObjectPtr object, ConstMemberFunc func) { Bind(object, func); }
	DelegateMemberSp0() :	m_object(0), m_func(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func) {
		m_object = object;
		m_func = func; }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func)	{
		m_object = object;
		m_func = reinterpret_cast<MemberFunc>(func); }

	virtual DelegateMemberSp0<TClass, RetType>* Clone() const {
		return new DelegateMemberSp0<TClass, RetType>(*this); }

	// Invoke the bound delegate function
	virtual RetType operator()() {
		return (*m_object.*m_func)(); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberSp0<TClass, RetType>* derivedRhs = dynamic_cast<const DelegateMemberSp0<TClass, RetType>*>(&rhs);
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
    typedef void (DelegateMemberSp0::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateMemberSp0::this_type_does_not_support_comparisons; }
	bool operator !() const { return !(m_object && m_func); }
#endif

private:
	ObjectPtr m_object;		// Pointer to a class object
	MemberFunc m_func;   	// Pointer to an instance member function
};

template <class TClass, class Param1, class RetType=void> 
class DelegateMemberSp1 : public Delegate1<Param1, RetType> {
public:
	typedef std::shared_ptr<TClass> ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1); 
	typedef RetType (TClass::*ConstMemberFunc)(Param1) const; 

	DelegateMemberSp1(ObjectPtr object, MemberFunc func) { Bind(object, func); }
	DelegateMemberSp1(ObjectPtr object, ConstMemberFunc func) { Bind(object, func);	}
	DelegateMemberSp1() :	m_object(0), m_func(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func) {
		m_object = object;
		m_func = func; }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func)	{
		m_object = object;
		m_func = reinterpret_cast<MemberFunc>(func); }

	virtual DelegateMemberSp1<TClass, Param1, RetType>* Clone() const {
		return new DelegateMemberSp1<TClass, Param1, RetType>(*this); }

	// Invoke the bound delegate function
	virtual RetType operator()(Param1 p1) {
		return (*m_object.*m_func)(p1); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberSp1<TClass, Param1, RetType>* derivedRhs = dynamic_cast<const DelegateMemberSp1<TClass, Param1, RetType>*>(&rhs);
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
    typedef void (DelegateMemberSp1::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateMemberSp1::this_type_does_not_support_comparisons; }
	bool operator !() const { return !(m_object && m_func); }
#endif

private:
	ObjectPtr m_object;		// Pointer to a class object
	MemberFunc m_func;   	// Pointer to an instance member function
};

template <class TClass, class Param1, class Param2, class RetType=void> 
class DelegateMemberSp2 : public Delegate2<Param1, Param2, RetType> {
public:
	typedef std::shared_ptr<TClass> ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2); 
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2) const; 

	DelegateMemberSp2(ObjectPtr object, MemberFunc func) { Bind(object, func); }
	DelegateMemberSp2(ObjectPtr object, ConstMemberFunc func) { Bind(object, func);	}
	DelegateMemberSp2() :	m_object(0), m_func(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func) {
		m_object = object;
		m_func = func; }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func)	{
		m_object = object;
		m_func = reinterpret_cast<MemberFunc>(func); }

	virtual DelegateMemberSp2<TClass, Param1, Param2, RetType>* Clone() const {
		return new DelegateMemberSp2<TClass, Param1, Param2, RetType>(*this); }

	// Invoke the bound delegate function
	virtual RetType operator()(Param1 p1, Param2 p2) {
		return (*m_object.*m_func)(p1, p2); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberSp2<TClass, Param1, Param2, RetType>* derivedRhs = dynamic_cast<const DelegateMemberSp2<TClass, Param1, Param2, RetType>*>(&rhs);
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
    typedef void (DelegateMemberSp2::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateMemberSp2::this_type_does_not_support_comparisons; }
	bool operator !() const { return !(m_object && m_func); }
#endif

private:
	ObjectPtr m_object;		// Pointer to a class object
	MemberFunc m_func;   	// Pointer to an instance member function
};

template <class TClass, class Param1, class Param2, class Param3, class RetType=void> 
class DelegateMemberSp3 : public Delegate3<Param1, Param2, Param3, RetType> {
public:
	typedef std::shared_ptr<TClass> ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2, Param3); 
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2, Param3) const; 

	DelegateMemberSp3(ObjectPtr object, MemberFunc func) { Bind(object, func); }
	DelegateMemberSp3(ObjectPtr object, ConstMemberFunc func) { Bind(object, func);	}
	DelegateMemberSp3() :	m_object(0), m_func(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func) {
		m_object = object;
		m_func = func; }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func)	{
		m_object = object;
		m_func = reinterpret_cast<MemberFunc>(func); }

	virtual DelegateMemberSp3<TClass, Param1, Param2, Param3, RetType>* Clone() const {
		return new DelegateMemberSp3<TClass, Param1, Param2, Param3, RetType>(*this); }

	// Invoke the bound delegate function
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3) {
		return (*m_object.*m_func)(p1, p2, p3); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberSp3<TClass, Param1, Param2, Param3, RetType>* derivedRhs = dynamic_cast<const DelegateMemberSp3<TClass, Param1, Param2, Param3, RetType>*>(&rhs);
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
    typedef void (DelegateMemberSp3::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateMemberSp3::this_type_does_not_support_comparisons; }
	bool operator !() const { return !(m_object && m_func); }
#endif

private:
	ObjectPtr m_object;		// Pointer to a class object
	MemberFunc m_func;   	// Pointer to an instance member function
};

template <class TClass, class Param1, class Param2, class Param3, class Param4, class RetType=void> 
class DelegateMemberSp4 : public Delegate4<Param1, Param2, Param3, Param4, RetType> {
public:
	typedef std::shared_ptr<TClass> ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2, Param3, Param4); 
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4) const; 

	DelegateMemberSp4(ObjectPtr object, MemberFunc func) { Bind(object, func); }
	DelegateMemberSp4(ObjectPtr object, ConstMemberFunc func) { Bind(object, func);	}
	DelegateMemberSp4() :	m_object(0), m_func(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func) {
		m_object = object;
		m_func = func; }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func)	{
		m_object = object;
		m_func = reinterpret_cast<MemberFunc>(func); }

	virtual DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4, RetType>* Clone() const {
		return new DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4, RetType>(*this); }

	// Invoke the bound delegate function
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
		return (*m_object.*m_func)(p1, p2, p3, p4); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4, RetType>* derivedRhs = dynamic_cast<const DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4, RetType>*>(&rhs);
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
    typedef void (DelegateMemberSp4::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateMemberSp4::this_type_does_not_support_comparisons; }
	bool operator !() const { return !(m_object && m_func); }
#endif

private:
	ObjectPtr m_object;		// Pointer to a class object
	MemberFunc m_func;   	// Pointer to an instance member function
};

template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5, class RetType=void> 
class DelegateMemberSp5 : public Delegate5<Param1, Param2, Param3, Param4, Param5, RetType> {
public:
	typedef std::shared_ptr<TClass> ObjectPtr;
	typedef RetType (TClass::*MemberFunc)(Param1, Param2, Param3, Param4, Param5); 
	typedef RetType (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4, Param5) const; 

	DelegateMemberSp5(ObjectPtr object, MemberFunc func) { Bind(object, func); }
	DelegateMemberSp5(ObjectPtr object, ConstMemberFunc func) { Bind(object, func);	}
	DelegateMemberSp5() :	m_object(0), m_func(0) { }

	/// Bind a member function to a delegate. 
	void Bind(ObjectPtr object, MemberFunc func) {
		m_object = object;
		m_func = func; }

	/// Bind a const member function to a delegate. 
	void Bind(ObjectPtr object, ConstMemberFunc func)	{
		m_object = object;
		m_func = reinterpret_cast<MemberFunc>(func); }

	virtual DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>* Clone() const {
		return new DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>(*this); }

	// Invoke the bound delegate function
	virtual RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
		return (*m_object.*m_func)(p1, p2, p3, p4, p5); }

	virtual bool operator==(const DelegateBase& rhs) const 	{
		const DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>* derivedRhs = dynamic_cast<const DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>*>(&rhs);
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
    typedef void (DelegateMemberSp5::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &DelegateMemberSp5::this_type_does_not_support_comparisons; }
	bool operator !() const { return !(m_object && m_func); }
#endif

private:
	ObjectPtr m_object;		// Pointer to a class object
	MemberFunc m_func;   	// Pointer to an instance member function
};

// MakeDelegate function creates a delegate object. C++ template argument deduction
// means you can call MakeDelegate without manually specifying the template parameters. 

//N=0
template <class TClass, class RetType>
DelegateMemberSp0<TClass, RetType> MakeDelegate(std::shared_ptr<TClass> object, RetType (TClass::*func)()) { 
	return DelegateMemberSp0<TClass, RetType>(object, func);
}

template <class TClass, class RetType>
DelegateMemberSp0<TClass, RetType> MakeDelegate(std::shared_ptr<TClass> object, RetType (TClass::*func)() const) { 
	return DelegateMemberSp0<TClass, RetType>(object, func);
}

//N=1
template <class TClass, class Param1, class RetType>
DelegateMemberSp1<TClass, Param1, RetType> MakeDelegate(std::shared_ptr<TClass> object, RetType (TClass::*func)(Param1 p1)) { 
	return DelegateMemberSp1<TClass, Param1, RetType>(object, func);
}

template <class TClass, class Param1, class RetType>
DelegateMemberSp1<TClass, Param1, RetType> MakeDelegate(std::shared_ptr<TClass> object, RetType (TClass::*func)(Param1 p1) const) { 
	return DelegateMemberSp1<TClass, Param1, RetType>(object, func);
}

//N=2
template <class TClass, class Param1, class Param2, class RetType>
DelegateMemberSp2<TClass, Param1, Param2, RetType> MakeDelegate(std::shared_ptr<TClass> object, RetType (TClass::*func)(Param1 p1, Param2 p2)) { 
	return DelegateMemberSp2<TClass, Param1, Param2, RetType>(object, func);
}

template <class TClass, class Param1, class Param2, class RetType>
DelegateMemberSp2<TClass, Param1, Param2, RetType> MakeDelegate(std::shared_ptr<TClass> object, RetType (TClass::*func)(Param1 p1, Param2 p2) const) { 
	return DelegateMemberSp2<TClass, Param1, Param2, RetType>(object, func);
}

//N=3
template <class TClass, class Param1, class Param2, class Param3, class RetType>
DelegateMemberSp3<TClass, Param1, Param2, Param3, RetType> MakeDelegate(std::shared_ptr<TClass> object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3)) { 
	return DelegateMemberSp3<TClass, Param1, Param2, Param3, RetType>(object, func);
}

template <class TClass, class Param1, class Param2, class Param3, class RetType>
DelegateMemberSp3<TClass, Param1, Param2, Param3, RetType> MakeDelegate(std::shared_ptr<TClass> object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3) const) { 
	return DelegateMemberSp3<TClass, Param1, Param2, Param3, RetType>(object, func);
}

//N=4
template <class TClass, class Param1, class Param2, class Param3, class Param4, class RetType>
DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4, RetType> MakeDelegate(std::shared_ptr<TClass> object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4)) { 
	return DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4, RetType>(object, func);
}

template <class TClass, class Param1, class Param2, class Param3, class Param4, class RetType>
DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4, RetType> MakeDelegate(std::shared_ptr<TClass> object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const) { 
	return DelegateMemberSp4<TClass, Param1, Param2, Param3, Param4, RetType>(object, func);
}

//N=5
template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5, class RetType>
DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5, RetType> MakeDelegate(std::shared_ptr<TClass> object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5)) { 
	return DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>(object, func);
}

template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5, class RetType>
DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5, RetType> MakeDelegate(std::shared_ptr<TClass> object, RetType (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const) { 
	return DelegateMemberSp5<TClass, Param1, Param2, Param3, Param4, Param5, RetType>(object, func);
}

}

#endif