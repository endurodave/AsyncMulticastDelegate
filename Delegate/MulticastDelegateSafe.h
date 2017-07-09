#ifndef _MULTICAST_DELEGATE_SAFE_H
#define _MULTICAST_DELEGATE_SAFE_H

#include "MulticastDelegate.h"
#include "LockGuard.h"

namespace DelegateLib {

/// @brief Thread-safe multicast delegate container class. May contain any delegate,
/// but typically used to hold DelegateMemberAsync<> or DelegateFreeAsync<> instances.
class MulticastDelegateSafe0 : public MulticastDelegate0
{
public:
	MulticastDelegateSafe0() { LockGuard::Create(&m_lock); }
	~MulticastDelegateSafe0() { LockGuard::Destroy(&m_lock); }

	void operator+=(const Delegate0<>& delegate) { 
		LockGuard lockGuard(&m_lock);
		MulticastDelegate0::operator +=(delegate); 
	}
	void operator-=(const Delegate0<>& delegate)	{ 
		LockGuard lockGuard(&m_lock);
		MulticastDelegate0::operator -=(delegate); 
	}
	void operator()() {
		LockGuard lockGuard(&m_lock);
		MulticastDelegate0::operator ()(); 
	}
	bool Empty() { 
		LockGuard lockGuard(&m_lock);
		return MulticastDelegate0::Empty();
	}
	void Clear() {
		LockGuard lockGuard(&m_lock);
		return MulticastDelegate0::Clear();
	}

#if USE_CPLUSPLUS_11
	explicit operator bool() {
		LockGuard lockGuard(&m_lock);
		return MulticastDelegateBase::operator bool();
	}
#else
	operator bool_type() {
		return Empty()? 0 : &MulticastDelegateBase::this_type_does_not_support_comparisons;
    }
	bool operator !() { return Empty(); }
#endif

private:
	// Prevent copying objects
	MulticastDelegateSafe0(const MulticastDelegateSafe0&);
	MulticastDelegateSafe0& operator=(const MulticastDelegateSafe0&);

	/// Lock to make the class thread-safe
	LOCK m_lock;
};

template<typename Param1>
class MulticastDelegateSafe1 : public MulticastDelegate1<Param1>
{
public:
	MulticastDelegateSafe1() { LockGuard::Create(&m_lock); }
	~MulticastDelegateSafe1() { LockGuard::Destroy(&m_lock); }

	void operator+=(const Delegate1<Param1>& delegate) { 
		LockGuard lockGuard(&m_lock);
		MulticastDelegate1<Param1>::operator +=(delegate); 
	}
	void operator-=(const Delegate1<Param1>& delegate)	{ 
		LockGuard lockGuard(&m_lock);
		MulticastDelegate1<Param1>::operator -=(delegate); 
	}
	void operator()(Param1 p1) {
		LockGuard lockGuard(&m_lock);
		MulticastDelegate1<Param1>::operator ()(p1); 
	}
	bool Empty() { 
		LockGuard lockGuard(&m_lock);
		return MulticastDelegate1<Param1>::Empty();
	}
	void Clear() {
		LockGuard lockGuard(&m_lock);
		MulticastDelegate1<Param1>::Clear();
	}

#if USE_CPLUSPLUS_11
	explicit operator bool() {
		LockGuard lockGuard(&m_lock);
		return MulticastDelegateBase::operator bool();
	}
#else
	operator typename MulticastDelegate1<Param1>::bool_type() {
		return Empty()? 0 : &MulticastDelegateBase::this_type_does_not_support_comparisons;
    }
	bool operator !() { return Empty(); }
#endif

private:
	// Prevent copying objects
	MulticastDelegateSafe1(const MulticastDelegateSafe1&);
	MulticastDelegateSafe1& operator=(const MulticastDelegateSafe1&);

	/// Lock to make the class thread-safe
	LOCK m_lock;
};

template<typename Param1, typename Param2>
class MulticastDelegateSafe2 : public MulticastDelegate2<Param1, Param2>
{
public:
	MulticastDelegateSafe2() { LockGuard::Create(&m_lock); }
	~MulticastDelegateSafe2() { LockGuard::Destroy(&m_lock); }

	void operator+=(const Delegate2<Param1, Param2>& delegate) { 
		LockGuard lockGuard(&m_lock);
		MulticastDelegate2<Param1, Param2>::operator +=(delegate); 
	}
	void operator-=(const Delegate2<Param1, Param2>& delegate)	{ 
		LockGuard lockGuard(&m_lock);
		MulticastDelegate2<Param1, Param2>::operator -=(delegate); 
	}
	void operator()(Param1 p1, Param2 p2) {
		LockGuard lockGuard(&m_lock);
		MulticastDelegate2<Param1, Param2>::operator ()(p1, p2); 
	}
	bool Empty() { 
		LockGuard lockGuard(&m_lock);
		return MulticastDelegate2<Param1, Param2>::Empty();
	}
	void Clear() {
		LockGuard lockGuard(&m_lock);
		MulticastDelegate2<Param1, Param2>::Clear();
	}

#if USE_CPLUSPLUS_11
	explicit operator bool() {
		LockGuard lockGuard(&m_lock);
		return MulticastDelegateBase::operator bool();
	}
#else
	operator typename MulticastDelegate2<Param1, Param2>::bool_type() {
		return Empty()? 0 : &MulticastDelegateBase::this_type_does_not_support_comparisons;
    }
	bool operator !() { return Empty(); }
#endif

private:
	// Prevent copying objects
	MulticastDelegateSafe2(const MulticastDelegateSafe2&);
	MulticastDelegateSafe2& operator=(const MulticastDelegateSafe2&);

	/// Lock to make the class thread-safe
	LOCK m_lock;
};

template<typename Param1, typename Param2, typename Param3>
class MulticastDelegateSafe3 : public MulticastDelegate3<Param1, Param2, Param3>
{
public:
	MulticastDelegateSafe3() { LockGuard::Create(&m_lock); }
	~MulticastDelegateSafe3() { LockGuard::Destroy(&m_lock); }

	void operator+=(const Delegate3<Param1, Param2, Param3>& delegate) { 
		LockGuard lockGuard(&m_lock);
		MulticastDelegate3<Param1, Param2, Param3>::operator +=(delegate); 
	}
	void operator-=(const Delegate3<Param1, Param2, Param3>& delegate)	{ 
		LockGuard lockGuard(&m_lock);
		MulticastDelegate3<Param1, Param2, Param3>::operator -=(delegate); 
	}
	void operator()(Param1 p1, Param2 p2, Param3 p3) {
		LockGuard lockGuard(&m_lock);
		MulticastDelegate3<Param1, Param2, Param3>::operator ()(p1, p2, p3); 
	}
	bool Empty() { 
		LockGuard lockGuard(&m_lock);
		return MulticastDelegate3<Param1, Param2, Param3>::Empty();
	}
	void Clear() {
		LockGuard lockGuard(&m_lock);
		MulticastDelegate3<Param1, Param2, Param3>::Clear();
	}

#if USE_CPLUSPLUS_11
	explicit operator bool() {
		LockGuard lockGuard(&m_lock);
		return MulticastDelegateBase::operator bool();
	}
#else
	operator typename MulticastDelegate3<Param1, Param2, Param3>::bool_type() {
		return Empty()? 0 : &MulticastDelegateBase::this_type_does_not_support_comparisons;
    }
	bool operator !() { return Empty(); }
#endif

private:
	// Prevent copying objects
	MulticastDelegateSafe3(const MulticastDelegateSafe3&);
	MulticastDelegateSafe3& operator=(const MulticastDelegateSafe3&);

	/// Lock to make the class thread-safe
	LOCK m_lock;
};

template<typename Param1, typename Param2, typename Param3, typename Param4>
class MulticastDelegateSafe4 : public MulticastDelegate4<Param1, Param2, Param3, Param4>
{
public:
	MulticastDelegateSafe4() { LockGuard::Create(&m_lock); }
	~MulticastDelegateSafe4() { LockGuard::Destroy(&m_lock); }

	void operator+=(const Delegate4<Param1, Param2, Param3, Param4>& delegate) { 
		LockGuard lockGuard(&m_lock);
		MulticastDelegate4<Param1, Param2, Param3, Param4>::operator +=(delegate); 
	}
	void operator-=(const Delegate4<Param1, Param2, Param3, Param4>& delegate)	{ 
		LockGuard lockGuard(&m_lock);
		MulticastDelegate4<Param1, Param2, Param3, Param4>::operator -=(delegate); 
	}
	void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
		LockGuard lockGuard(&m_lock);
		MulticastDelegate4<Param1, Param2, Param3, Param4>::operator ()(p1, p2, p3, p4); 
	}
	bool Empty() { 
		LockGuard lockGuard(&m_lock);
		return MulticastDelegate4<Param1, Param2, Param3, Param4>::Empty();
	}
	void Clear() {
		LockGuard lockGuard(&m_lock);
		MulticastDelegate4<Param1, Param2, Param3, Param4>::Clear();
	}

#if USE_CPLUSPLUS_11
	explicit operator bool() {
		LockGuard lockGuard(&m_lock);
		return MulticastDelegateBase::operator bool();
	}
#else
	operator typename MulticastDelegate4<Param1, Param2, Param3, Param4>::bool_type() {
		return Empty()? 0 : &MulticastDelegateBase::this_type_does_not_support_comparisons;
    }
	bool operator !() { return Empty(); }
#endif

private:
	// Prevent copying objects
	MulticastDelegateSafe4(const MulticastDelegateSafe4&);
	MulticastDelegateSafe4& operator=(const MulticastDelegateSafe4&);

	/// Lock to make the class thread-safe
	LOCK m_lock;
};

template<typename Param1, typename Param2, typename Param3, typename Param4, typename Param5>
class MulticastDelegateSafe5 : public MulticastDelegate5<Param1, Param2, Param3, Param4, Param5>
{
public:
	MulticastDelegateSafe5() { LockGuard::Create(&m_lock); }
	~MulticastDelegateSafe5() { LockGuard::Destroy(&m_lock); }

	void operator+=(const Delegate5<Param1, Param2, Param3, Param4, Param5>& delegate) { 
		LockGuard lockGuard(&m_lock);
		MulticastDelegate5<Param1, Param2, Param3, Param4, Param5>::operator +=(delegate); 
	}
	void operator-=(const Delegate5<Param1, Param2, Param3, Param4, Param5>& delegate)	{ 
		LockGuard lockGuard(&m_lock);
		MulticastDelegate5<Param1, Param2, Param3, Param4, Param5>::operator -=(delegate); 
	}
	void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
		LockGuard lockGuard(&m_lock);
		MulticastDelegate5<Param1, Param2, Param3, Param4, Param5>::operator ()(p1, p2, p3, p4, p5); 
	}
	bool Empty() { 
		LockGuard lockGuard(&m_lock);
		return MulticastDelegate5<Param1, Param2, Param3, Param4, Param5>::Empty();
	}
	void Clear() {
		LockGuard lockGuard(&m_lock);
		MulticastDelegate5<Param1, Param2, Param3, Param4, Param5>::Clear();
	}

#if USE_CPLUSPLUS_11
	explicit operator bool() {
		LockGuard lockGuard(&m_lock);
		return MulticastDelegateBase::operator bool();
	}
#else
	operator typename MulticastDelegate5<Param1, Param2, Param3, Param4, Param5>::bool_type() {
		return Empty()? 0 : &MulticastDelegateBase::this_type_does_not_support_comparisons;
    }
	bool operator !() { return Empty(); }
#endif

private:
	// Prevent copying objects
	MulticastDelegateSafe5(const MulticastDelegateSafe5&);
	MulticastDelegateSafe5& operator=(const MulticastDelegateSafe5&);

	/// Lock to make the class thread-safe
	LOCK m_lock;
};

}

#endif
