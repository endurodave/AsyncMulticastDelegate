#ifndef _DELEGATE_REMOTE_INVOKER_H
#define _DELEGATE_REMOTE_INVOKER_H

#include "LockGuard.h"
#include <istream>
#include <map>

namespace DelegateLib {

typedef int DelegateIdType;

/// @brief An abstract base class used to invoke a delegate on a remote system. 
class DelegateRemoteInvoker
{
public:
    /// Constructor
    /// @param[in] id - an id shared by both remote systems.
    DelegateRemoteInvoker(DelegateIdType id);

	/// Called to invoke the callback by the remote system. 
    /// @param[in] s - the incoming remote message stream. 
	virtual void DelegateInvoke(std::istream& s) = 0;

    /// Get an invoker by id
    /// @return A delegate invoker or null if not matching id. 
    static DelegateRemoteInvoker* GetInvoker(DelegateIdType id);

private:
    static std::map<DelegateIdType, DelegateRemoteInvoker*> m_idMap;
    static LOCK m_lock; 
};

}

#endif