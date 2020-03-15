#include "DelegateAsync.h"

namespace DelegateLib {

DelegateFreeAsync0 MakeDelegate(void (*func)(), DelegateThread* thread) { 
	return DelegateFreeAsync0(func, thread);
}

}