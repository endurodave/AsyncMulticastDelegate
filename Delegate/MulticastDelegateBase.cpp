#include "MulticastDelegateBase.h"

namespace DelegateLib {

//------------------------------------------------------------------------------
// operator+=
//------------------------------------------------------------------------------
void MulticastDelegateBase::operator+=(const DelegateBase& delegate)
{
	InvocationNode* node = new InvocationNode();
	node->Delegate = delegate.Clone();
	
	// First element in the list?
	if (m_invocationHead == 0)
	{
		// Set the head element
		m_invocationHead = node;
	}
	else
	{
		// Iterate over list until the end of list is found
		InvocationNode* curr = m_invocationHead;
		while (curr->Next != 0)
			curr = curr->Next;
		
		// Set the info pointer at the end of the list
		curr->Next = node;
	}
}

//------------------------------------------------------------------------------
// operator-=
//------------------------------------------------------------------------------
void MulticastDelegateBase::operator-=(const DelegateBase& delegate)
{
	// Iterate over list to find delegate to remove
	InvocationNode* curr = m_invocationHead;
	InvocationNode* prev = 0;
	while (curr != 0)
	{
		// Is this the delegate to remove?
		if (*curr->Delegate == delegate)
		{
			if (curr == m_invocationHead)
				m_invocationHead = curr->Next;
			else
				prev->Next = curr->Next;
			
			delete curr->Delegate;
			delete curr;
			break;
		}
		prev = curr;
		curr = curr->Next;
	}	
}

//------------------------------------------------------------------------------
// Clear
//------------------------------------------------------------------------------
void MulticastDelegateBase::Clear()
{
	while (m_invocationHead)
	{
		InvocationNode* curr = m_invocationHead;
		m_invocationHead = curr->Next;
		delete curr->Delegate;
		delete curr;
	}
}

}

