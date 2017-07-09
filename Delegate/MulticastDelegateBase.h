#ifndef _MULTICAST_DELEGATE_BASE_H
#define _MULTICAST_DELEGATE_BASE_H

#include "Delegate.h"

namespace DelegateLib {

/// @brief A non-template base class for the multicast delegates. 
/// @details Since the MulticastDelegate template class inherits from this class, 
/// as much code is placed into this base class as possible to minimize the
/// template code instantiation. 
class MulticastDelegateBase
{
public:
	/// Constructor
	MulticastDelegateBase() : m_invocationHead(0) {}

	/// Destructor
	virtual ~MulticastDelegateBase() { Clear(); }

	/// Any registered delegates?
	bool Empty() const { return !m_invocationHead; }

	/// Removal all registered delegates.
	void Clear();

protected:
	struct InvocationNode
	{
		InvocationNode() : Next(0), Delegate(0) { }
		InvocationNode* Next;
		DelegateBase* Delegate;
	};

	/// Insert a delegate into the invocation list. A delegate argument 
	/// pointer is not stored. Instead, the DelegateBase derived object is 
	/// copied (cloned) and saved in the invocation list.
	/// @param[in] delegate - a delegate to register. 
	void operator+=(const DelegateBase& delegate);

	/// Remove a delegate previously registered delegate from the invocation
	/// list. 
	/// @param[in] delegate - a delegate to unregister. 
	void operator-=(const DelegateBase& delegate);

	/// Get the head of the delegate invocation list. 
	/// @return Pointer to the head of the invocation list. 
	InvocationNode* GetInvocationHead() { return m_invocationHead; }

#if USE_CPLUSPLUS_11
public:
	// New-school safe bool
	explicit operator bool() const { return !Empty();  }
#else
public:
	// Old-school safe bool idiom
    typedef void (MulticastDelegateBase::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}
	operator bool_type() const {
		return Empty()? 0 : &MulticastDelegateBase::this_type_does_not_support_comparisons;
    }
	bool operator !() const { return Empty(); }
#endif

private:
	// Prevent copying objects
	MulticastDelegateBase(const MulticastDelegateBase&);
	MulticastDelegateBase& operator=(const MulticastDelegateBase&);

	/// Head pointer to the delegate invocation list
	InvocationNode* m_invocationHead;
};

}

#endif
