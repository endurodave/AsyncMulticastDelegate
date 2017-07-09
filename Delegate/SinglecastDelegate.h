#ifndef _SINGLECAST_DELEGATE_H
#define _SINGLECAST_DELEGATE_H

#include "Delegate.h"

namespace DelegateLib {

/// @brief SinglecastDelegate holds a single Delegate<> base class. When the 
/// SinglecastDelegate is invoked the delegate instance is called.
template<class RetType=void>
class SinglecastDelegate0 
{
public:
	SinglecastDelegate0() : m_delegate(0) {}
	~SinglecastDelegate0() { Clear(); }

	void operator=(const Delegate0<RetType>& delegate) {
		Clear();
		m_delegate = delegate.Clone();	// Create a duplicate delegate
	}

	void operator=(const Delegate0<RetType>* delegate) {
		Clear();
		if (delegate)
			m_delegate = delegate->Clone();  // Create a duplicate delegate
	}

	RetType operator()() {
		return (*m_delegate)();	} // Invoke delegate callback

	bool Empty() const { return !m_delegate; }
	void Clear() { 
		if (m_delegate)	{
			delete m_delegate;
			m_delegate = 0; } 
	}

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (SinglecastDelegate0::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &SinglecastDelegate0::this_type_does_not_support_comparisons; }
	bool operator !() const { return !m_delegate; }
#endif

private:
	// Prevent copying objects
	SinglecastDelegate0(const SinglecastDelegate0&);
	SinglecastDelegate0& operator=(const SinglecastDelegate0&);

	Delegate0<RetType>* m_delegate;
};

template<typename Param1, class RetType=void>
class SinglecastDelegate1 
{
public:
	SinglecastDelegate1() : m_delegate(0) {}
	~SinglecastDelegate1() { Clear(); }

	void operator=(const Delegate1<Param1, RetType>& delegate) {
		Clear();
		m_delegate = delegate.Clone();	// Create a duplicate delegate
	}

	void operator=(const Delegate1<Param1, RetType>* delegate) {
		Clear();
		if (delegate)
			m_delegate = delegate->Clone();  // Create a duplicate delegate
	}

	RetType operator()(Param1 p1) {
		return (*m_delegate)(p1); }	// Invoke delegate callback

	bool Empty() const { return !m_delegate; }
	void Clear() { 
		if (m_delegate)	{
			delete m_delegate;
			m_delegate = 0; } 
	}

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (SinglecastDelegate1::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &SinglecastDelegate1::this_type_does_not_support_comparisons; }
	bool operator !() const { return !m_delegate; }
#endif

private:
	// Prevent copying objects
	SinglecastDelegate1(const SinglecastDelegate1&);
	SinglecastDelegate1& operator=(const SinglecastDelegate1&);

	Delegate1<Param1, RetType>* m_delegate;
};

template<typename Param1, class Param2, class RetType=void>
class SinglecastDelegate2 
{
public:
	SinglecastDelegate2() : m_delegate(0) {}
	~SinglecastDelegate2() { Clear(); }

	void operator=(const Delegate2<Param1, Param2, RetType>& delegate) {
		Clear();
		m_delegate = delegate.Clone();	// Create a duplicate delegate
	}

	void operator=(const Delegate2<Param1, Param2, RetType>* delegate) {
		Clear();
		if (delegate)
			m_delegate = delegate->Clone();  // Create a duplicate delegate
	}

	RetType operator()(Param1 p1, Param2 p2) {
		return (*m_delegate)(p1, p2); }	// Invoke delegate callback

	bool Empty() const { return !m_delegate; }
	void Clear() { 
		if (m_delegate)	{
			delete m_delegate;
			m_delegate = 0; } 
	}

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (SinglecastDelegate2::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &SinglecastDelegate2::this_type_does_not_support_comparisons; }
	bool operator !() const { return !m_delegate; }
#endif

private:
	// Prevent copying objects
	SinglecastDelegate2(const SinglecastDelegate2&);
	SinglecastDelegate2& operator=(const SinglecastDelegate2&);

	Delegate2<Param1, Param2, RetType>* m_delegate;
};

template<typename Param1, class Param2, class Param3, class RetType=void>
class SinglecastDelegate3 
{
public:
	SinglecastDelegate3() : m_delegate(0) {}
	~SinglecastDelegate3() { Clear(); }

	void operator=(const Delegate3<Param1, Param2, Param3, RetType>& delegate) {
		Clear();
		m_delegate = delegate.Clone();	// Create a duplicate delegate
	}

	void operator=(const Delegate3<Param1, Param2, Param3, RetType>* delegate) {
		Clear();
		if (delegate)
			m_delegate = delegate->Clone();  // Create a duplicate delegate
	}

	RetType operator()(Param1 p1, Param2 p2, Param3 p3) {
		return (*m_delegate)(p1, p2, p3); }	// Invoke delegate callback

	bool Empty() const { return !m_delegate; }
	void Clear() { 
		if (m_delegate)	{
			delete m_delegate;
			m_delegate = 0; } 
	}

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (SinglecastDelegate3::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &SinglecastDelegate3::this_type_does_not_support_comparisons; }
	bool operator !() const { return !m_delegate; }
#endif

private:
	// Prevent copying objects
	SinglecastDelegate3(const SinglecastDelegate3&);
	SinglecastDelegate3& operator=(const SinglecastDelegate3&);

	Delegate3<Param1, Param2, Param3, RetType>* m_delegate;
};

template<typename Param1, class Param2, class Param3, class Param4, class RetType=void>
class SinglecastDelegate4
{
public:
	SinglecastDelegate4() : m_delegate(0) {}
	~SinglecastDelegate4() { Clear(); }

	void operator=(const Delegate4<Param1, Param2, Param3, Param4, RetType>& delegate) {
		Clear();
		m_delegate = delegate.Clone();	// Create a duplicate delegate
	}

	void operator=(const Delegate4<Param1, Param2, Param3, Param4, RetType>* delegate) {
		Clear();
		if (delegate)
			m_delegate = delegate->Clone();  // Create a duplicate delegate
	}

	RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
		return (*m_delegate)(p1, p2, p3, p4); }	// Invoke delegate callback

	bool Empty() const { return !m_delegate; }
	void Clear() { 
		if (m_delegate)	{
			delete m_delegate;
			m_delegate = 0; } 
	}

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (SinglecastDelegate4::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &SinglecastDelegate4::this_type_does_not_support_comparisons; }
	bool operator !() const { return !m_delegate; }
#endif

private:
	// Prevent copying objects
	SinglecastDelegate4(const SinglecastDelegate4&);
	SinglecastDelegate4& operator=(const SinglecastDelegate4&);

	Delegate4<Param1, Param2, Param3, Param4, RetType>* m_delegate;
};

template<typename Param1, class Param2, class Param3, class Param4, class Param5, class RetType=void>
class SinglecastDelegate5
{
public:
	SinglecastDelegate5() : m_delegate(0) {}
	~SinglecastDelegate5() { Clear(); }

	void operator=(const Delegate5<Param1, Param2, Param3, Param4, Param5, RetType>& delegate) {
		Clear();
		m_delegate = delegate.Clone();	// Create a duplicate delegate
	}

	void operator=(const Delegate5<Param1, Param2, Param3, Param4, Param5, RetType>* delegate) {
		Clear();
		if (delegate)
			m_delegate = delegate->Clone();  // Create a duplicate delegate
	}

	RetType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
		return (*m_delegate)(p1, p2, p3, p4, p5); }	// Invoke delegate callback

	bool Empty() const { return !m_delegate; }
	void Clear() { 
		if (m_delegate)	{
			delete m_delegate;
			m_delegate = 0; } 
	}

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
private:
	// Old-school safe bool idiom
    typedef void (SinglecastDelegate5::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
public:
	operator bool_type() const {
		return Empty()? 0 : &SinglecastDelegate5::this_type_does_not_support_comparisons; }
	bool operator !() const { return !m_delegate; }
#endif

private:
	// Prevent copying objects
	SinglecastDelegate5(const SinglecastDelegate5&);
	SinglecastDelegate5& operator=(const SinglecastDelegate5&);

	Delegate5<Param1, Param2, Param3, Param4, Param5, RetType>* m_delegate;
};

}

#endif
