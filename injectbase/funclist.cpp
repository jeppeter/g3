
#include "funclist.h"

CFuncList::CFuncList()
{
    InitializeCriticalSection(&m_FuncListCS);
    assert(m_FuncVecs.size() == 0);
    assert(m_ParamVecs.size() == 0);
    assert(m_FuncVecs.size() == 0);
}

int CFuncList::__RemoveFunc(FuncCall_t pFunc)
{
    int ret = -ERROR_NOT_FOUND;
    int findidx=-1;
    UINT i;
    EnterCriticalSection(&(this->m_FuncListCS));
    for(i=0; i<this->m_FuncVecs.size() ; i++)
    {
    }
    LeaveCriticalSection(&(this->m_FuncListCS));
    if(ret < 0)
    {
        SetLastError(-ret);
    }
    return ret;
}


CFuncList::~CFuncList()
{
}

