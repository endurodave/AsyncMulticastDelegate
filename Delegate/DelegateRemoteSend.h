#ifndef _DELEGATE_REMOTE_SEND_H
#define _DELEGATE_REMOTE_SEND_H

// DelegateRemoteSend.h
// @see https://www.codeproject.com/Articles/5262271/Remote-Procedure-Calls-using-Cplusplus-Delegates
// David Lafreniere, Mar 2020.

#include "Delegate.h"
#include "DelegateTransport.h"
#include "DelegateRemoteInvoker.h"

namespace DelegateLib {

/// @brief Send a delegate to invoke a function on a remote system. 
template <class Param1>
class DelegateRemoteSend1 : public Delegate1<Param1> {
public:
    DelegateRemoteSend1(IDelegateTransport& transport, std::iostream& stream, DelegateIdType id) : 
        m_transport(transport), m_stream(stream), m_id(id) { }

	virtual DelegateRemoteSend1* Clone() const { return new DelegateRemoteSend1(*this); }

	/// Invoke the bound delegate function. 
	virtual void operator()(Param1 p1) {
        m_stream << m_id << std::ends;
        m_stream << p1 << std::ends;
        m_transport.DispatchDelegate(m_stream);
    }

	virtual bool operator==(const DelegateBase& rhs) const {
		const DelegateRemoteSend1<Param1>* derivedRhs = dynamic_cast<const DelegateRemoteSend1<Param1>*>(&rhs);
		return derivedRhs &&
			m_id == derivedRhs->m_id &&
            &m_transport == &derivedRhs->m_transport; }

private:
	IDelegateTransport& m_transport;    // Object sends data to remote
    std::iostream& m_stream;            // Storage for remote message 
    DelegateIdType m_id;                // Remote delegate identifier
};

template <class Param1, class Param2>
class DelegateRemoteSend2 : public Delegate2<Param1, Param2> {
public:
    DelegateRemoteSend2(IDelegateTransport& transport, std::iostream& stream, DelegateIdType id) :
        m_transport(transport), m_stream(stream), m_id(id) { }

    virtual DelegateRemoteSend2* Clone() const { return new DelegateRemoteSend2(*this); }

    /// Invoke the bound delegate function. 
    virtual void operator()(Param1 p1, Param2 p2) {
        m_stream << m_id << std::ends;
        m_stream << p1 << std::ends;
        m_stream << p2 << std::ends;
        m_transport.DispatchDelegate(m_stream);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateRemoteSend2<Param1, Param2>* derivedRhs = dynamic_cast<const DelegateRemoteSend2<Param1, Param2>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            &m_transport == &derivedRhs->m_transport;
    }

private:
    IDelegateTransport & m_transport;   // Object sends data to remote
    std::iostream& m_stream;            // Storage for remote message 
    DelegateIdType m_id;                // Remote delegate identifier
};

template <class Param1, class Param2, class Param3>
class DelegateRemoteSend3 : public Delegate3<Param1, Param2, Param3> {
public:
    DelegateRemoteSend3(IDelegateTransport& transport, std::iostream& stream, DelegateIdType id) :
        m_transport(transport), m_stream(stream), m_id(id) { }

    virtual DelegateRemoteSend3* Clone() const { return new DelegateRemoteSend3(*this); }

    /// Invoke the bound delegate function. 
    virtual void operator()(Param1 p1, Param2 p2, Param3 p3) {
        m_stream << m_id << std::ends;
        m_stream << p1 << std::ends;
        m_stream << p2 << std::ends;
        m_stream << p3 << std::ends;
        m_transport.DispatchDelegate(m_stream);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateRemoteSend3<Param1, Param2, Param3>* derivedRhs = dynamic_cast<const DelegateRemoteSend3<Param1, Param2, Param3>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            &m_transport == &derivedRhs->m_transport;
    }

private:
    IDelegateTransport & m_transport;   // Object sends data to remote
    std::iostream& m_stream;            // Storage for remote message 
    DelegateIdType m_id;                // Remote delegate identifier
};

template <class Param1, class Param2, class Param3, class Param4>
class DelegateRemoteSend4 : public Delegate4<Param1, Param2, Param3, Param4> {
public:
    DelegateRemoteSend4(IDelegateTransport& transport, std::iostream& stream, DelegateIdType id) :
        m_transport(transport), m_stream(stream), m_id(id) { }

    virtual DelegateRemoteSend4* Clone() const { return new DelegateRemoteSend4(*this); }

    /// Invoke the bound delegate function. 
    virtual void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) {
        m_stream << m_id << std::ends;
        m_stream << p1 << std::ends;
        m_stream << p2 << std::ends;
        m_stream << p3 << std::ends;
        m_stream << p4 << std::ends;
        m_transport.DispatchDelegate(m_stream);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateRemoteSend4<Param1, Param2, Param3, Param4>* derivedRhs = dynamic_cast<const DelegateRemoteSend4<Param1, Param2, Param3, Param4>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            &m_transport == &derivedRhs->m_transport;
    }

private:
    IDelegateTransport & m_transport;   // Object sends data to remote
    std::iostream& m_stream;            // Storage for remote message 
    DelegateIdType m_id;                // Remote delegate identifier
};

template <class Param1, class Param2, class Param3, class Param4, class Param5>
class DelegateRemoteSend5 : public Delegate5<Param1, Param2, Param3, Param4, Param5> {
public:
    DelegateRemoteSend5(IDelegateTransport& transport, std::iostream& stream, DelegateIdType id) :
        m_transport(transport), m_stream(stream), m_id(id) { }

    virtual DelegateRemoteSend5* Clone() const { return new DelegateRemoteSend5(*this); }

    /// Invoke the bound delegate function. 
    virtual void operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) {
        m_stream << m_id << std::ends;
        m_stream << p1 << std::ends;
        m_stream << p2 << std::ends;
        m_stream << p3 << std::ends;
        m_stream << p4 << std::ends;
        m_stream << p5 << std::ends;
        m_transport.DispatchDelegate(m_stream);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateRemoteSend5<Param1, Param2, Param3, Param4, Param5>* derivedRhs = dynamic_cast<const DelegateRemoteSend5<Param1, Param2, Param3, Param4, Param5>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            &m_transport == &derivedRhs->m_transport;
    }

private:
    IDelegateTransport & m_transport;   // Object sends data to remote
    std::iostream& m_stream;            // Storage for remote message 
    DelegateIdType m_id;                // Remote delegate identifier
};

//N=1
template <class Param1>
DelegateRemoteSend1<Param1> MakeDelegate(IDelegateTransport& transport, std::iostream& stream, DelegateIdType id) {
    return DelegateRemoteSend1<Param1>(transport, stream, id);
}

//N=2
template <class Param1, class Param2>
DelegateRemoteSend2<Param1, Param2> MakeDelegate(IDelegateTransport& transport, std::iostream& stream, DelegateIdType id) {
    return DelegateRemoteSend2<Param1, Param2>(transport, stream, id);
}

//N=3
template <class Param1, class Param2, class Param3>
DelegateRemoteSend3<Param1, Param2, Param3> MakeDelegate(IDelegateTransport& transport, std::iostream& stream, DelegateIdType id) {
    return DelegateRemoteSend3<Param1, Param2, Param3>(transport, stream, id);
}

//N=4
template <class Param1, class Param2, class Param3, class Param4>
DelegateRemoteSend4<Param1, Param2, Param3, Param4> MakeDelegate(IDelegateTransport& transport, std::iostream& stream, DelegateIdType id) {
    return DelegateRemoteSend4<Param1, Param2, Param3, Param4>(transport, stream, id);
}

//N=5
template <class Param1, class Param2, class Param3, class Param4, class Param5>
DelegateRemoteSend5<Param1, Param2, Param3, Param4, Param5> MakeDelegate(IDelegateTransport& transport, std::iostream& stream, DelegateIdType id) {
    return DelegateRemoteSend5<Param1, Param2, Param3, Param4, Param5>(transport, stream, id);
}

}

#endif