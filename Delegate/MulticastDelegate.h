#ifndef _MULTICAST_DELEGATE_H
#define _MULTICAST_DELEGATE_H

#include "MulticastDelegateBase.h"
#include "Delegate.h"

namespace DelegateLib {

/// @brief Multicast delegate container class. The class has a linked list of 
/// Delegate<> instances. When invoked, each Delegate instance within the invocation 
/// list is called. MulticastDelegate<> does support return values. A void return  
/// must always be used.
class MulticastDelegate0 : public MulticastDelegateBase
{
public:
	MulticastDelegate0() { }
	void operator()() {
		InvocationNode* node = GetInvocationHead();
		while (node != 0) {
			Delegate0<>* delegate = 
				static_cast<Delegate0<>*>(node->Delegate);
			(*delegate)();	// Invoke delegate callback
			node = node->Next;
		}
	}
	void operator+=(const Delegate0<>& delegate) { MulticastDelegateBase::operator+=(delegate); }
	void operator-=(const Delegate0<>& delegate) { MulticastDelegateBase::operator-=(delegate); }

private:
	// Prevent copying objects
	MulticastDelegate0(const MulticastDelegate0&);
	MulticastDelegate0& operator=(const MulticastDelegate0&);
};

template<typename Param1>
class MulticastDelegate1 : public MulticastDelegateBase
{
public:
	MulticastDelegate1() { }
	void operator()(Param1 p1) {
		InvocationNode* node = GetInvocationHead();
		while (node != 0) {
			Delegate1<Param1>* delegate = 
				static_cast<Delegate1<Param1>*>(node->Delegate);
			(*delegate)(p1);	// Invoke delegate callback
			node = node->Next;
		}
	}
	void operator+=(const Delegate1<Param1>& delegate) { MulticastDelegateBase::operator+=(delegate); }
	void operator-=(const Delegate1<Param1>& delegate) { MulticastDelegateBase::operator-=(delegate); }

private:
	// Prevent copying objects
	MulticastDelegate1(const MulticastDelegate1&);
	MulticastDelegate1& operator=(const MulticastDelegate1&);
};

template<typename Param1, class Param2>
class MulticastDelegate2 : public MulticastDelegateBase
{
public:
	MulticastDelegate2() { }
	void operator()(Param1 p1, Param2 p2) {
		InvocationNode* node = GetInvocationHead();
		while (node != 0) {
			Delegate2<Param1, Param2>* delegate = 
				static_cast<Delegate2<Param1, Param2>*>(node->Delegate);
			(*delegate)(p1, p2);	// Invoke delegate callback
			node = node->Next;
		}
	}
	void operator+=(const Delegate2<Param1, Param2>& delegate) { MulticastDelegateBase::operator+=(delegate); }
	void operator-=(const Delegate2<Param1, Param2>& delegate) { MulticastDelegateBase::operator-=(delegate); }

private:
	// Prevent copying objects
	MulticastDelegate2(const MulticastDelegate2&);
	MulticastDelegate2& operator=(const MulticastDelegate2&);
};

template<typename Param1, class Param2, class Param3>
class MulticastDelegate3 : public MulticastDelegateBase
{
public:
	MulticastDelegate3() { }
	void operator()(Param1 p1, Param2 p2, Param3 p3) {
		InvocationNode* node = GetInvocationHead();
		while (node != 0) {
			Delegate3<Param1, Param2, Param3>* delegate = 
				static_cast<Delegate3<Param1, Param2, Param3>*>(node->Delegate);
			(*delegate)(p1, p2, p3);	// Invoke delegate callback
			node = node->Next;
		}
	}
	void operator+=(const Delegate3<Param1, Param2, Param3>& delegate) { MulticastDelegateBase::operator+=(delegate); }
	void operator-=(const Delegate3<Param1, Param2, Param3>& delegate) { MulticastDelegateBase::operator-=(delegate); }

private:
	// Prevent copying objects
	MulticastDelegate3(const MulticastDelegate3&);
	MulticastDelegate3& operator=(const MulticastDelegate3&);
};

template<typename Param1, class Param2, class Param3, class Param4>
class MulticastDelegate4 : public MulticastDelegateBase
{
public:
	MulticastDelegate4() { }
	void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
		InvocationNode* node = GetInvocationHead();
		while (node != 0) {
			Delegate4<Param1, Param2, Param3, Param4>* delegate = 
				static_cast<Delegate4<Param1, Param2, Param3, Param4>*>(node->Delegate);
			(*delegate)(p1, p2, p3, p4);	// Invoke delegate callback
			node = node->Next;
		}
	}
	void operator+=(const Delegate4<Param1, Param2, Param3, Param4>& delegate) { MulticastDelegateBase::operator+=(delegate); }
	void operator-=(const Delegate4<Param1, Param2, Param3, Param4>& delegate) { MulticastDelegateBase::operator-=(delegate); }

private:
	// Prevent copying objects
	MulticastDelegate4(const MulticastDelegate4&);
	MulticastDelegate4& operator=(const MulticastDelegate4&);
};

template<typename Param1, class Param2, class Param3, class Param4, class Param5>
class MulticastDelegate5 : public MulticastDelegateBase
{
public:
	MulticastDelegate5() { }
	void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
		InvocationNode* node = GetInvocationHead();
		while (node != 0) {
			Delegate5<Param1, Param2, Param3, Param4, Param5>* delegate = 
				static_cast<Delegate5<Param1, Param2, Param3, Param4, Param5>*>(node->Delegate);
			(*delegate)(p1, p2, p3, p4, p5);	// Invoke delegate callback
			node = node->Next;
		}
	}
	void operator+=(const Delegate5<Param1, Param2, Param3, Param4, Param5>& delegate) { MulticastDelegateBase::operator+=(delegate); }
	void operator-=(const Delegate5<Param1, Param2, Param3, Param4, Param5>& delegate) { MulticastDelegateBase::operator-=(delegate); }

private:
	// Prevent copying objects
	MulticastDelegate5(const MulticastDelegate5&);
	MulticastDelegate5& operator=(const MulticastDelegate5&);
};

}

#endif