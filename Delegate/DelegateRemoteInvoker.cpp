#include "DelegateRemoteInvoker.h"
#include "Fault.h"

namespace DelegateLib {
    std::map<int, DelegateRemoteInvoker*> DelegateRemoteInvoker::m_idMap;
    LOCK DelegateRemoteInvoker::m_lock;

    DelegateRemoteInvoker::DelegateRemoteInvoker(DelegateIdType id)
    {
        LockGuard lockGuard(&m_lock);

        // Don't allow duplicate entries
        std::map<DelegateIdType, DelegateRemoteInvoker*>::iterator it = m_idMap.find(id);
        ASSERT_TRUE(it != m_idMap.end());

        m_idMap[id] = this;
    }
    
    DelegateRemoteInvoker* DelegateRemoteInvoker::GetInvoker(DelegateIdType id)
    {
        LockGuard lockGuard(&m_lock);
        std::map<DelegateIdType, DelegateRemoteInvoker*>::iterator it = m_idMap.find(id);
        if (it != m_idMap.end())
            return (*it).second;
        return NULL; 
    }
}