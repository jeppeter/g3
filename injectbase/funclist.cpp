
#include "funclist.h"


#define  FUNC_LIST_VEC_ASSERT() \
do\
{\
	assert(this->m_FuncVecs.size() == this->m_ParamVecs.size());\
	assert(this->m_ParamVecs.size() == this->m_FuncUseVecs.size());\
}while(0)


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
    FUNC_LIST_VEC_ASSERT();
    for(i=0; i<this->m_FuncVecs.size() ; i++)
    {
        if(pFunc == NULL || pFunc == this->m_FuncVecs[i])
        {
            findidx = i;
            break;
        }
    }

    if(findidx >= 0)
    {
        if(this->m_FuncUseVecs[findidx] > 0)
        {
            ret = 0;
        }
        else
        {
            ret = 1;
            this->m_FuncUseVecs.erase(this->m_FuncUseVecs.begin() + findidx);
            this->m_FuncVecs.erase(this->m_FuncVecs.begin() + findidx);
            this->m_ParamVecs.erase(this->m_ParamVecs.begin() + findidx);
        }
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
    int ret;
    do
    {
        ret = this->__RemoveFunc(NULL);
        if(ret == 0)
        {
            /*if we can not remove one ,just sched out*/
            SchedOut();
        }
    }
    while(ret >= 0);

    assert(this->m_FuncUseVecs.size() == 0);
    FUNC_LIST_VEC_ASSERT();
    DeleteCriticalSection(&(this->m_FuncListCS));
}

int CFuncList::AddFuncList(FuncCall_t pFunc,PVOID pParam)
{
    int ret = 1;
    int findidx = -1;
    UINT i;

    EnterCriticalSection(&(this->m_FuncListCS));
    FUNC_LIST_VEC_ASSERT();
    for(i=0; i<this->m_FuncVecs.size(); i++)
    {
        if(this->m_FuncVecs[i] == pFunc)
        {
            findidx = i;
            break;
        }
    }

    if(findidx >=0)
    {
        ret = 0;
    }
    else
    {
        ret = 1;
        this->m_FuncVecs.push_back(pFunc);
        this->m_ParamVecs.push_back(pParam);
        this->m_FuncUseVecs.push_back(0);
    }
    LeaveCriticalSection(&(this->m_FuncListCS));
    return ret;
}


int CFuncList::RemoveFuncList(FuncCall_t pFunc)
{
    int ret=0;

    do
    {
        ret = this->__RemoveFunc(pFunc);
        if(ret == 0)
        {
            SchedOut();
        }
    }
    while(ret == 0);

    return ret > 0 ? 1 : 0;
}

FuncCall_t CFuncList::__GetFuncCall(int idx,LPVOID& pParam)
{
    FuncCall_t pFunc = NULL;

    EnterCriticalSection(&(this->m_FuncListCS));
    if(this->m_FuncVecs.size() > idx)
    {
        pFunc = this->m_FuncVecs[idx];
        this->m_FuncUseVecs[idx] ++;
    }
    LeaveCriticalSection(&(this->m_FuncListCS));
    return pFunc;
}

int CFuncList::__PutFuncCall(FuncCall_t pFunc)
{
    int idx=-1;
    UINT i;
    EnterCriticalSection(&(this->m_FuncListCS));
    for(i=0; i<this->m_FuncVecs.size(); i++)
    {
        if(this->m_FuncVecs[i] == pFunc)
        {
            idx = i+1;
            break;
        }
    }
    LeaveCriticalSection(&(this->m_FuncListCS));
    return idx;
}

int CFuncList::CallList(LPVOID pParam)
{
    FuncCall_t pFunc=NULL;
    LPVOID pCallParam=NULL;
    int totalret=0;
    int ret;
    int idx;

    idx = 0;
    while(1)
    {
        pFunc = this->__GetFuncCall(idx,pCallParam);
        if(pFunc == NULL)
        {
            break;
        }

        ret = pFunc(pCallParam,pParam);
        if(ret < 0)
        {
            ret = LAST_ERROR_CODE();
            ERROR_INFO("Call<0x%p> Function Error(%d)\n",pFunc,ret);
            totalret = ret;
        }

        idx = this->__PutFuncCall(pFunc);
    }

    return totalret;
}

