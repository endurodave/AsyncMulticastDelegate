#ifndef _DELEGATE_REMOTE_RECV_H
#define _DELEGATE_REMOTE_RECV_H

// DelegateRemoteRecv.h
// @see https://www.codeproject.com/Articles/5262271/Remote-Procedure-Calls-using-Cplusplus-Delegates
// David Lafreniere, Mar 2020.

#include "Delegate.h"
#include "DelegateTransport.h"
#include "DelegateRemoteInvoker.h"

namespace DelegateLib {

template <class Param>
class RemoteParam
{
public:
    Param& Get() { return m_param; }
private:
    Param m_param;
};

template <class Param>
class RemoteParam<Param*>
{
public:
    Param* Get() { return &m_param; }
private:
    Param m_param;
};

template <class Param>
class RemoteParam<Param**>
{
public:
    RemoteParam() { m_pParam = &m_param; }
    Param ** Get() { return &m_pParam; }
private:
    Param m_param;
    Param* m_pParam;
};

template <class Param>
class RemoteParam<Param&>
{
public:
    Param & Get() { return m_param; }
private:
    Param m_param;
};

/// @brief Receive a delegate from a remote system and invoke the bound function. 
template <class TClass, class Param1>
class DelegateMemberRemoteRecv1 : public DelegateMember1<TClass, Param1>, public DelegateRemoteInvoker {
public:
    typedef TClass* ObjectPtr;
    typedef void (TClass::*MemberFunc)(Param1);
    typedef void (TClass::*ConstMemberFunc)(Param1) const;

    // Contructors take a class instance, member function, and delegete id 
    DelegateMemberRemoteRecv1(ObjectPtr object, MemberFunc func, DelegateIdType id) : 
        DelegateRemoteInvoker(id) { Bind(object, func, id); }
    DelegateMemberRemoteRecv1(ObjectPtr object, ConstMemberFunc func, DelegateIdType id) : 
        DelegateRemoteInvoker(id) { Bind(object, func, id); }

    /// Bind a member function to a delegate. 
    void Bind(ObjectPtr object, MemberFunc func, DelegateIdType id) {
        m_id = id;
        DelegateMember1<TClass, Param1>::Bind(object, func);
    }

    /// Bind a const member function to a delegate. 
    void Bind(ObjectPtr object, ConstMemberFunc func, DelegateIdType id) {
        m_id = id;
        DelegateMember1<TClass, Param1>::Bind(object, func);
    }

    virtual DelegateMemberRemoteRecv1<TClass, Param1>* Clone() const {
        return new DelegateMemberRemoteRecv1<TClass, Param1>(*this);
    }

    /// Called by the remote system to invoke the delegate function
    virtual void DelegateInvoke(std::istream& stream) {
        RemoteParam<Param1> param1;

        Param1 p1 = param1.Get();

        stream >> m_id;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p1;
        stream.seekg(stream.tellg() + std::streampos(1));

        DelegateMember1<TClass, Param1>::operator()(p1);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateMemberRemoteRecv1<TClass, Param1>* derivedRhs = dynamic_cast<const DelegateMemberRemoteRecv1<TClass, Param1>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            DelegateMember1<TClass, Param1>::operator == (rhs);
    }

    private:
        DelegateIdType m_id;               // Remote delegate identifier
};

template <class TClass, class Param1, class Param2>
class DelegateMemberRemoteRecv2 : public DelegateMember2<TClass, Param1, Param2>, public DelegateRemoteInvoker {
public:
    typedef TClass* ObjectPtr;
    typedef void (TClass::*MemberFunc)(Param1, Param2);
    typedef void (TClass::*ConstMemberFunc)(Param1, Param2) const;

    // Contructors take a class instance, member function, and delegete id 
    DelegateMemberRemoteRecv2(ObjectPtr object, MemberFunc func, DelegateIdType id) :
        DelegateRemoteInvoker(id) {
        Bind(object, func, id);
    }
    DelegateMemberRemoteRecv2(ObjectPtr object, ConstMemberFunc func, DelegateIdType id) :
        DelegateRemoteInvoker(id) {
        Bind(object, func, id);
    }

    /// Bind a member function to a delegate. 
    void Bind(ObjectPtr object, MemberFunc func, DelegateIdType id) {
        m_id = id;
        DelegateMember2<TClass, Param1, Param2>::Bind(object, func);
    }

    /// Bind a const member function to a delegate. 
    void Bind(ObjectPtr object, ConstMemberFunc func, DelegateIdType id) {
        m_id = id;
        DelegateMember2<TClass, Param1, Param2>::Bind(object, func);
    }

    virtual DelegateMemberRemoteRecv2<TClass, Param1, Param2>* Clone() const {
        return new DelegateMemberRemoteRecv2<TClass, Param1, Param2>(*this);
    }

    /// Called by the remote system to invoke the delegate function
    virtual void DelegateInvoke(std::istream& stream) {
        RemoteParam<Param1> param1;
        RemoteParam<Param2> param2;

        Param1 p1 = param1.Get();
        Param2 p2 = param2.Get();

        stream >> m_id;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p1;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p2;
        stream.seekg(stream.tellg() + std::streampos(1));

        DelegateMember2<TClass, Param1, Param2>::operator()(p1, p2);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateMemberRemoteRecv2<TClass, Param1, Param2>* derivedRhs = dynamic_cast<const DelegateMemberRemoteRecv2<TClass, Param1, Param2>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            DelegateMember2<TClass, Param1, Param2>::operator == (rhs);
    }

private:
    DelegateIdType m_id;               // Remote delegate identifier
};

template <class TClass, class Param1, class Param2, class Param3>
class DelegateMemberRemoteRecv3 : public DelegateMember3<TClass, Param1, Param2, Param3>, public DelegateRemoteInvoker {
public:
    typedef TClass* ObjectPtr;
    typedef void (TClass::*MemberFunc)(Param1, Param2, Param3);
    typedef void (TClass::*ConstMemberFunc)(Param1, Param2, Param3) const;

    // Contructors take a class instance, member function, and delegete id 
    DelegateMemberRemoteRecv3(ObjectPtr object, MemberFunc func, DelegateIdType id) :
        DelegateRemoteInvoker(id) {
        Bind(object, func, id);
    }
    DelegateMemberRemoteRecv3(ObjectPtr object, ConstMemberFunc func, DelegateIdType id) :
        DelegateRemoteInvoker(id) {
        Bind(object, func, id);
    }

    /// Bind a member function to a delegate. 
    void Bind(ObjectPtr object, MemberFunc func, DelegateIdType id) {
        m_id = id;
        DelegateMember3<TClass, Param1, Param2, Param3>::Bind(object, func);
    }

    /// Bind a const member function to a delegate. 
    void Bind(ObjectPtr object, ConstMemberFunc func, DelegateIdType id) {
        m_id = id;
        DelegateMember3<TClass, Param1, Param2, Param3>::Bind(object, func);
    }

    virtual DelegateMemberRemoteRecv3<TClass, Param1, Param2, Param3>* Clone() const {
        return new DelegateMemberRemoteRecv3<TClass, Param1, Param2, Param3>(*this);
    }

    /// Called by the remote system to invoke the delegate function
    virtual void DelegateInvoke(std::istream& stream) {
        RemoteParam<Param1> param1;
        RemoteParam<Param2> param2;
        RemoteParam<Param3> param3;

        Param1 p1 = param1.Get();
        Param2 p2 = param2.Get();
        Param3 p3 = param3.Get();

        stream >> m_id;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p1;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p2;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p3;
        stream.seekg(stream.tellg() + std::streampos(1));

        DelegateMember3<TClass, Param1, Param2, Param3>::operator()(p1, p2, p3);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateMemberRemoteRecv3<TClass, Param1, Param2, Param3>* derivedRhs = dynamic_cast<const DelegateMemberRemoteRecv3<TClass, Param1, Param2, Param3>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            DelegateMember3<TClass, Param1, Param2, Param3>::operator == (rhs);
    }

private:
    DelegateIdType m_id;               // Remote delegate identifier
};

template <class TClass, class Param1, class Param2, class Param3, class Param4>
class DelegateMemberRemoteRecv4 : public DelegateMember4<TClass, Param1, Param2, Param3, Param4>, public DelegateRemoteInvoker {
public:
    typedef TClass* ObjectPtr;
    typedef void (TClass::*MemberFunc)(Param1, Param2, Param3, Param4);
    typedef void (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4) const;

    // Contructors take a class instance, member function, and delegete id 
    DelegateMemberRemoteRecv4(ObjectPtr object, MemberFunc func, DelegateIdType id) :
        DelegateRemoteInvoker(id) {
        Bind(object, func, id);
    }
    DelegateMemberRemoteRecv4(ObjectPtr object, ConstMemberFunc func, DelegateIdType id) :
        DelegateRemoteInvoker(id) {
        Bind(object, func, id);
    }

    /// Bind a member function to a delegate. 
    void Bind(ObjectPtr object, MemberFunc func, DelegateIdType id) {
        m_id = id;
        DelegateMember4<TClass, Param1, Param2, Param3, Param4>::Bind(object, func);
    }

    /// Bind a const member function to a delegate. 
    void Bind(ObjectPtr object, ConstMemberFunc func, DelegateIdType id) {
        m_id = id;
        DelegateMember4<TClass, Param1, Param2, Param3, Param4>::Bind(object, func);
    }

    virtual DelegateMemberRemoteRecv4<TClass, Param1, Param2, Param3, Param4>* Clone() const {
        return new DelegateMemberRemoteRecv4<TClass, Param1, Param2, Param3, Param4>(*this);
    }

    /// Called by the remote system to invoke the delegate function
    virtual void DelegateInvoke(std::istream& stream) {
        RemoteParam<Param1> param1;
        RemoteParam<Param2> param2;
        RemoteParam<Param3> param3;
        RemoteParam<Param4> param4;

        Param1 p1 = param1.Get();
        Param2 p2 = param2.Get();
        Param3 p3 = param3.Get();
        Param4 p4 = param4.Get();

        stream >> m_id;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p1;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p2;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p3;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p4;
        stream.seekg(stream.tellg() + std::streampos(1));

        DelegateMember4<TClass, Param1, Param2, Param3, Param4>::operator()(p1, p2, p3, p4);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateMemberRemoteRecv4<TClass, Param1, Param2, Param3, Param4>* derivedRhs = dynamic_cast<const DelegateMemberRemoteRecv4<TClass, Param1, Param2, Param3, Param4>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            DelegateMember4<TClass, Param1, Param2, Param3, Param4>::operator == (rhs);
    }

private:
    DelegateIdType m_id;               // Remote delegate identifier
};

template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5>
class DelegateMemberRemoteRecv5 : public DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5>, public DelegateRemoteInvoker {
public:
    typedef TClass* ObjectPtr;
    typedef void (TClass::*MemberFunc)(Param1, Param2, Param3, Param4, Param5);
    typedef void (TClass::*ConstMemberFunc)(Param1, Param2, Param3, Param4, Param5) const;

    // Contructors take a class instance, member function, and delegete id 
    DelegateMemberRemoteRecv5(ObjectPtr object, MemberFunc func, DelegateIdType id) :
        DelegateRemoteInvoker(id) {
        Bind(object, func, id);
    }
    DelegateMemberRemoteRecv5(ObjectPtr object, ConstMemberFunc func, DelegateIdType id) :
        DelegateRemoteInvoker(id) {
        Bind(object, func, id);
    }

    /// Bind a member function to a delegate. 
    void Bind(ObjectPtr object, MemberFunc func, DelegateIdType id) {
        m_id = id;
        DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5>::Bind(object, func);
    }

    /// Bind a const member function to a delegate. 
    void Bind(ObjectPtr object, ConstMemberFunc func, DelegateIdType id) {
        m_id = id;
        DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5>::Bind(object, func);
    }

    virtual DelegateMemberRemoteRecv5<TClass, Param1, Param2, Param3, Param4, Param5>* Clone() const {
        return new DelegateMemberRemoteRecv5<TClass, Param1, Param2, Param3, Param4, Param5>(*this);
    }

    /// Called by the remote system to invoke the delegate function
    virtual void DelegateInvoke(std::istream& stream) {
        RemoteParam<Param1> param1;
        RemoteParam<Param2> param2;
        RemoteParam<Param3> param3;
        RemoteParam<Param4> param4;
        RemoteParam<Param5> param5;

        Param1 p1 = param1.Get();
        Param2 p2 = param2.Get();
        Param3 p3 = param3.Get();
        Param4 p4 = param4.Get();
        Param4 p5 = param5.Get();

        stream >> m_id;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p1;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p2;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p3;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p4;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p5;
        stream.seekg(stream.tellg() + std::streampos(1));

        DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5>::operator()(p1, p2, p3, p4, p5);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateMemberRemoteRecv5<TClass, Param1, Param2, Param3, Param4, Param5>* derivedRhs = dynamic_cast<const DelegateMemberRemoteRecv5<TClass, Param1, Param2, Param3, Param4, Param5>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            DelegateMember5<TClass, Param1, Param2, Param3, Param4, Param5>::operator == (rhs);
    }

private:
    DelegateIdType m_id;               // Remote delegate identifier
};

template <class Param1>
class DelegateFreeRemoteRecv1 : public DelegateFree1<Param1>, public DelegateRemoteInvoker {
public:
    typedef void(*FreeFunc)(Param1);

    // Contructors take a free function and delegete id 
    DelegateFreeRemoteRecv1(FreeFunc func, DelegateIdType id) : DelegateRemoteInvoker(id) { Bind(func, id); }

    /// Bind a free function to the delegate.
    void Bind(FreeFunc func, DelegateIdType id) {
        m_id = id;
        DelegateFree1<Param1>::Bind(func);
    }

    virtual DelegateFreeRemoteRecv1* Clone() const { return new DelegateFreeRemoteRecv1(*this); }

    /// Called by the remote system to invoke the delegate function
    virtual void DelegateInvoke(std::istream& stream) {
        RemoteParam<Param1> param1;

        Param1 p1 = param1.Get();

        stream >> m_id;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p1;
        stream.seekg(stream.tellg() + std::streampos(1));

        DelegateFree1<Param1>::operator()(p1);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateFreeRemoteRecv1<Param1>* derivedRhs = dynamic_cast<const DelegateFreeRemoteRecv1<Param1>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            DelegateFree1<Param1>::operator == (rhs);
    }

private:
    DelegateIdType m_id;               // Remote delegate identifier
};

template <class Param1, class Param2>
class DelegateFreeRemoteRecv2 : public DelegateFree2<Param1, Param2>, public DelegateRemoteInvoker {
public:
    typedef void(*FreeFunc)(Param1, Param2);

    // Contructors take a free function and delegete id 
    DelegateFreeRemoteRecv2(FreeFunc func, DelegateIdType id) : DelegateRemoteInvoker(id) { Bind(func, id); }

    /// Bind a free function to the delegate.
    void Bind(FreeFunc func, DelegateIdType id) {
        m_id = id;
        DelegateFree2<Param1, Param2>::Bind(func);
    }

    virtual DelegateFreeRemoteRecv2* Clone() const { return new DelegateFreeRemoteRecv2(*this); }

    /// Called by the remote system to invoke the delegate function
    virtual void DelegateInvoke(std::istream& stream) {
        RemoteParam<Param1> param1;
        RemoteParam<Param2> param2;

        Param1 p1 = param1.Get();
        Param2 p2 = param2.Get();

        stream >> m_id;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p1;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p2;
        stream.seekg(stream.tellg() + std::streampos(1));

        DelegateFree2<Param1, Param2>::operator()(p1, p2);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateFreeRemoteRecv2<Param1, Param2>* derivedRhs = dynamic_cast<const DelegateFreeRemoteRecv2<Param1, Param2>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            DelegateFree2<Param1, Param2>::operator == (rhs);
    }

private:
    DelegateIdType m_id;               // Remote delegate identifier
};

template <class Param1, class Param2, class Param3>
class DelegateFreeRemoteRecv3 : public DelegateFree3<Param1, Param2, Param3>, public DelegateRemoteInvoker {
public:
    typedef void(*FreeFunc)(Param1, Param2, Param3);

    // Contructors take a free function and delegete id 
    DelegateFreeRemoteRecv3(FreeFunc func, DelegateIdType id) : DelegateRemoteInvoker(id) { Bind(func, id); }

    /// Bind a free function to the delegate.
    void Bind(FreeFunc func, DelegateIdType id) {
        m_id = id;
        DelegateFree3<Param1, Param2, Param3>::Bind(func);
    }

    virtual DelegateFreeRemoteRecv3* Clone() const { return new DelegateFreeRemoteRecv3(*this); }

    /// Called by the remote system to invoke the delegate function
    virtual void DelegateInvoke(std::istream& stream) {
        RemoteParam<Param1> param1;
        RemoteParam<Param2> param2;
        RemoteParam<Param3> param3;

        Param1 p1 = param1.Get();
        Param2 p2 = param2.Get();
        Param3 p3 = param3.Get();

        stream >> m_id;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p1;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p2;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p3;
        stream.seekg(stream.tellg() + std::streampos(1));

        DelegateFree3<Param1, Param2, Param3>::operator()(p1, p2, p3);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateFreeRemoteRecv3<Param1, Param2, Param3>* derivedRhs = dynamic_cast<const DelegateFreeRemoteRecv3<Param1, Param2, Param3>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            DelegateFree3<Param1, Param2, Param3>::operator == (rhs);
    }

private:
    DelegateIdType m_id;               // Remote delegate identifier
};

template <class Param1, class Param2, class Param3, class Param4>
class DelegateFreeRemoteRecv4 : public DelegateFree4<Param1, Param2, Param3, Param4>, public DelegateRemoteInvoker {
public:
    typedef void(*FreeFunc)(Param1, Param2, Param3, Param4);

    // Contructors take a free function and delegete id 
    DelegateFreeRemoteRecv4(FreeFunc func, DelegateIdType id) : DelegateRemoteInvoker(id) { Bind(func, id); }

    /// Bind a free function to the delegate.
    void Bind(FreeFunc func, DelegateIdType id) {
        m_id = id;
        DelegateFree4<Param1, Param2, Param3, Param4>::Bind(func);
    }

    virtual DelegateFreeRemoteRecv4* Clone() const { return new DelegateFreeRemoteRecv4(*this); }

    /// Called by the remote system to invoke the delegate function
    virtual void DelegateInvoke(std::istream& stream) {
        RemoteParam<Param1> param1;
        RemoteParam<Param2> param2;
        RemoteParam<Param3> param3;
        RemoteParam<Param4> param4;

        Param1 p1 = param1.Get();
        Param2 p2 = param2.Get();
        Param3 p3 = param3.Get();
        Param4 p4 = param4.Get();

        stream >> m_id;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p1;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p2;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p3;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p4;
        stream.seekg(stream.tellg() + std::streampos(1));

        DelegateFree4<Param1, Param2, Param3, Param4>::operator()(p1, p2, p3, p4);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateFreeRemoteRecv4<Param1, Param2, Param3, Param4>* derivedRhs = dynamic_cast<const DelegateFreeRemoteRecv4<Param1, Param2, Param3, Param4>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            DelegateFree4<Param1, Param2, Param3, Param4>::operator == (rhs);
    }

private:
    DelegateIdType m_id;               // Remote delegate identifier
};

template <class Param1, class Param2, class Param3, class Param4, class Param5>
class DelegateFreeRemoteRecv5 : public DelegateFree5<Param1, Param2, Param3, Param4, Param5>, public DelegateRemoteInvoker {
public:
    typedef void(*FreeFunc)(Param1, Param2, Param3, Param4, Param5);

    // Contructors take a free function and delegete id 
    DelegateFreeRemoteRecv5(FreeFunc func, DelegateIdType id) : DelegateRemoteInvoker(id) { Bind(func, id); }

    /// Bind a free function to the delegate.
    void Bind(FreeFunc func, DelegateIdType id) {
        m_id = id;
        DelegateFree5<Param1, Param2, Param3, Param4, Param5>::Bind(func);
    }

    virtual DelegateFreeRemoteRecv5* Clone() const { return new DelegateFreeRemoteRecv5(*this); }

    /// Called by the remote system to invoke the delegate function
    virtual void DelegateInvoke(std::istream& stream) {
        RemoteParam<Param1> param1;
        RemoteParam<Param2> param2;
        RemoteParam<Param3> param3;
        RemoteParam<Param4> param4;
        RemoteParam<Param5> param5;

        Param1 p1 = param1.Get();
        Param2 p2 = param2.Get();
        Param3 p3 = param3.Get();
        Param4 p4 = param4.Get();
        Param5 p5 = param5.Get();

        stream >> m_id;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p1;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p2;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p3;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p4;
        stream.seekg(stream.tellg() + std::streampos(1));
        stream >> p5;
        stream.seekg(stream.tellg() + std::streampos(1));

        DelegateFree5<Param1, Param2, Param3, Param4, Param5>::operator()(p1, p2, p3, p4, p5);
    }

    virtual bool operator==(const DelegateBase& rhs) const {
        const DelegateFreeRemoteRecv5<Param1, Param2, Param3, Param4, Param5>* derivedRhs = dynamic_cast<const DelegateFreeRemoteRecv5<Param1, Param2, Param3, Param4, Param5>*>(&rhs);
        return derivedRhs &&
            m_id == derivedRhs->m_id &&
            DelegateFree5<Param1, Param2, Param3, Param4, Param5>::operator == (rhs);
    }

private:
    DelegateIdType m_id;               // Remote delegate identifier
};

//N=1
template <class TClass, class Param1>
DelegateMemberRemoteRecv1<TClass, Param1> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1), DelegateIdType id) {
    return DelegateMemberRemoteRecv1<TClass, Param1>(object, func, id);
}

template <class TClass, class Param1>
DelegateMemberRemoteRecv1<TClass, Param1> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1) const, DelegateIdType id) {
    return DelegateMemberRemoteRecv1<TClass, Param1>(object, func, id);
}

template <class Param1>
DelegateFreeRemoteRecv1<Param1> MakeDelegate(void(*func)(Param1 p1), DelegateIdType id) {
    return DelegateFreeRemoteRecv1<Param1>(func, id);
}

//N=2
template <class TClass, class Param1, class Param2>
DelegateMemberRemoteRecv2<TClass, Param1, Param2> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2), DelegateIdType id) {
    return DelegateMemberRemoteRecv2<TClass, Param1, Param2>(object, func, id);
}

template <class TClass, class Param1, class Param2>
DelegateMemberRemoteRecv2<TClass, Param1, Param2> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2) const, DelegateIdType id) {
    return DelegateMemberRemoteRecv2<TClass, Param1, Param2>(object, func, id);
}

template <class Param1, class Param2>
DelegateFreeRemoteRecv2<Param1, Param2> MakeDelegate(void(*func)(Param1 p1, Param2 p2), DelegateIdType id) {
    return DelegateFreeRemoteRecv2<Param1, Param2>(func, id);
}

//N=3
template <class TClass, class Param1, class Param2, class Param3>
DelegateMemberRemoteRecv3<TClass, Param1, Param2, Param3> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3), DelegateIdType id) {
    return DelegateMemberRemoteRecv3<TClass, Param1, Param2, Param3>(object, func, id);
}

template <class TClass, class Param1, class Param2, class Param3>
DelegateMemberRemoteRecv3<TClass, Param1, Param2, Param3> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3) const, DelegateIdType id) {
    return DelegateMemberRemoteRecv3<TClass, Param1, Param2, Param3>(object, func, id);
}

template <class Param1, class Param2, class Param3>
DelegateFreeRemoteRecv3<Param1, Param2, Param3> MakeDelegate(void(*func)(Param1 p1, Param2 p2, Param3 p3), DelegateIdType id) {
    return DelegateFreeRemoteRecv3<Param1, Param2, Param3>(func, id);
}

//N=4
template <class TClass, class Param1, class Param2, class Param3, class Param4>
DelegateMemberRemoteRecv4<TClass, Param1, Param2, Param3, Param4> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4), DelegateIdType id) {
    return DelegateMemberRemoteRecv4<TClass, Param1, Param2, Param3, Param4>(object, func, id);
}

template <class TClass, class Param1, class Param2, class Param3, class Param4>
DelegateMemberRemoteRecv4<TClass, Param1, Param2, Param3, Param4> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const, DelegateIdType id) {
    return DelegateMemberRemoteRecv4<TClass, Param1, Param2, Param3, Param4>(object, func, id);
}

template <class Param1, class Param2, class Param3, class Param4>
DelegateFreeRemoteRecv4<Param1, Param2, Param3, Param4> MakeDelegate(void(*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4), DelegateIdType id) {
    return DelegateFreeRemoteRecv4<Param1, Param2, Param3, Param4>(func, id);
}

//N=5
template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5>
DelegateMemberRemoteRecv5<TClass, Param1, Param2, Param3, Param4, Param5> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5), DelegateIdType id) {
    return DelegateMemberRemoteRecv5<TClass, Param1, Param2, Param3, Param4, Param5>(object, func, id);
}

template <class TClass, class Param1, class Param2, class Param3, class Param4, class Param5>
DelegateMemberRemoteRecv5<TClass, Param1, Param2, Param3, Param4, Param5> MakeDelegate(TClass* object, void (TClass::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const, DelegateIdType id) {
    return DelegateMemberRemoteRecv5<TClass, Param1, Param2, Param3, Param4, Param5>(object, func, id);
}

template <class Param1, class Param2, class Param3, class Param4, class Param5>
DelegateFreeRemoteRecv5<Param1, Param2, Param3, Param4, Param5> MakeDelegate(void(*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5), DelegateIdType id) {
    return DelegateFreeRemoteRecv5<Param1, Param2, Param3, Param4, Param5>(func, id);
}

}

#endif