#ifndef _DELEGATE_MSG_H
#define _DELEGATE_MSG_H

#include "Fault.h"
#include "DelegateInvoker.h"
#if USE_XALLOCATOR
	#include "xallocator.h"
#endif

namespace DelegateLib {

class DelegateBase;

class DelegateMsgBase
{
#if USE_XALLOCATOR
	XALLOCATOR
#endif
public:
	/// Constructor
	/// @param[in] invoker - the invoker instance the delegate is registered with.
	/// @param[in] delegate - the delegate instance. 
	DelegateMsgBase(IDelegateInvoker* invoker) :
		m_invoker(invoker)
	{
		ASSERT_TRUE(m_invoker != 0);
	}

	/// Get the delegate invoker instance the delegate is registered with.
	/// @return The invoker instance. 
	IDelegateInvoker* GetDelegateInvoker() const { return m_invoker; }
	
private:
	/// The IDelegateInvoker instance 
	IDelegateInvoker* m_invoker;
};

/// @brief A class containing the delegate information passed through 
/// the message queue. 
template <typename Param1>
class DelegateMsg1 : public DelegateMsgBase
{
public:
	/// Constructor
	/// @param[in] invoker - the invoker instance the delegate is registered with.
	/// @param[in] param1 - the data sent as delegate function argument.
	DelegateMsg1(IDelegateInvoker* invoker, Param1 param1) :
		DelegateMsgBase(invoker),
		m_param1(param1)
	{
	}

	/// Get the delegate data passed into the delegate function. 
	/// @return The param1 delegate function data. 
	Param1 GetParam1() const { return m_param1;	}

private:
	/// The data argument passed into the callback function
	Param1 m_param1;
};

template <typename Param1, typename Param2>
class DelegateMsg2 : public DelegateMsgBase
{
public:
	/// Constructor
	/// @param[in] invoker - the invoker instance the delegate is registered with.
	/// @param[in] param1 - the data sent as delegate function argument.
	DelegateMsg2(IDelegateInvoker* invoker, Param1 param1, Param2 param2) :
		DelegateMsgBase(invoker),
		m_param1(param1),
		m_param2(param2)
	{
	}

	/// Get the delegate data passed into the delegate function. 
	/// @return The param1 delegate function data. 
	Param1 GetParam1() const { return m_param1;	}
	Param2 GetParam2() const { return m_param2;	}

private:
	/// The data argument passed into the invoked function
	Param1 m_param1;
	Param2 m_param2;
};

template <typename Param1, typename Param2, typename Param3>
class DelegateMsg3 : public DelegateMsgBase
{
public:
	/// Constructor
	/// @param[in] invoker - the invoker instance the delegate is registered with.
	/// @param[in] param1 - the data sent as delegate function argument.
	DelegateMsg3(IDelegateInvoker* invoker, Param1 param1, Param2 param2, Param3 param3) :
		DelegateMsgBase(invoker),
		m_param1(param1),
		m_param2(param2),
		m_param3(param3)
	{
	}

	/// Get the delegate data passed into the delegate function. 
	/// @return The param1 delegate function data. 
	Param1 GetParam1() const { return m_param1;	}
	Param2 GetParam2() const { return m_param2;	}
	Param3 GetParam3() const { return m_param3;	}

private:
	/// The data argument passed into the invoked function
	Param1 m_param1;
	Param2 m_param2;
	Param3 m_param3;
};

template <typename Param1, typename Param2, typename Param3, typename Param4>
class DelegateMsg4 : public DelegateMsgBase
{
public:
	/// Constructor
	/// @param[in] invoker - the invoker instance the delegate is registered with.
	/// @param[in] param1 - the data sent as delegate function argument.
	DelegateMsg4(IDelegateInvoker* invoker, Param1 param1, Param2 param2, Param3 param3, Param4 param4) :
		DelegateMsgBase(invoker),
		m_param1(param1),
		m_param2(param2),
		m_param3(param3),
		m_param4(param4)
	{
	}

	/// Get the delegate data passed into the delegate function. 
	/// @return The param1 delegate function data. 
	Param1 GetParam1() const { return m_param1;	}
	Param2 GetParam2() const { return m_param2;	}
	Param3 GetParam3() const { return m_param3;	}
	Param3 GetParam4() const { return m_param4;	}

private:
	/// The data argument passed into the invoked function
	Param1 m_param1;
	Param2 m_param2;
	Param3 m_param3;
	Param4 m_param4;
};

template <typename Param1, typename Param2, typename Param3, typename Param4, typename Param5>
class DelegateMsg5 : public DelegateMsgBase
{
public:
	/// Constructor
	/// @param[in] invoker - the invoker instance the delegate is registered with.
	/// @param[in] param1 - the data sent as delegate function argument.
	DelegateMsg5(IDelegateInvoker* invoker, Param1 param1, Param2 param2, Param3 param3, Param4 param4, Param5 param5) :
		DelegateMsgBase(invoker),
		m_param1(param1),
		m_param2(param2),
		m_param3(param3),
		m_param4(param4),
		m_param5(param5)
	{
	}

	/// Get the delegate data passed into the delegate function. 
	/// @return The param1 delegate function data. 
	Param1 GetParam1() const { return m_param1;	}
	Param2 GetParam2() const { return m_param2;	}
	Param3 GetParam3() const { return m_param3;	}
	Param3 GetParam4() const { return m_param4;	}
	Param5 GetParam5() const { return m_param5;	}

private:
	/// The data argument passed into the invoked function
	Param1 m_param1;
	Param2 m_param2;
	Param3 m_param3;
	Param4 m_param4;
	Param4 m_param5;
};

}

#endif