#ifndef _DELEGATE_TRANSPORT_H
#define _DELEGATE_TRANSPORT_H

#include <ostream>

namespace DelegateLib {

/// @brief Each platform specific implementation must inherit from DelegateTransport
/// and provide an implementation for DispatchDelegate().
class DelegateTransport
{
public:
	/// Destructor
	virtual ~DelegateTransport() {}

	/// Dispatch a stream of bytes to a remote system. The implementer is responsible
	/// for sending the bytes over a communication link. Once the receiver obtains the 
	/// bytes, the DelegateRemoteInvoker::DelegateInvoke() function must be called to 
    /// execute the callback on the remote system. 
	/// @param[in] s - an outgoing stream to send to the remote CPU.
    virtual void DispatchDelegate(std::ostream& s) = 0;
};

}

#endif
