#ifndef __FUNC_LIST_H__
#define __FUNC_LIST_H__

#include <Windows.h>
#include <vector>
typedef int (*FuncCall_t)(PVOID pParam,PVOID pInput);

#define  MAX_FUNCLIST_PRIOR      0xffff


#ifdef INJECTBASE_EXPORTS
#ifndef INJECTBASE_CPP_API
#define   INJECTBASE_CPP_API    __declspec(dllexport)
#endif
#else   /*INJECTBASE_EXPORTS*/
#ifndef INJECTBASE_CPP_API
#define   INJECTBASE_CPP_API   __declspec(dllimport)
#endif
#endif   /*INJECTBASE_EXPORTS*/

class INJECTBASE_CPP_API CFuncList
{
public:
	CFuncList();
	~CFuncList();
	int AddFuncList(FuncCall_t pFunc,LPVOID pParam,int prior=MAX_FUNCLIST_PRIOR);
	int RemoveFuncList(FuncCall_t pFunc);
	int CallList(LPVOID pParam);

private:
	int __RemoveFunc(FuncCall_t pFunc);
	FuncCall_t __GetFuncCall(int idx,LPVOID& pParam);
	int __PutFuncCall(FuncCall_t pFunc);
private:
	CRITICAL_SECTION m_FuncListCS;
	std::vector<FuncCall_t>* m_pFuncVecs;
	std::vector<LPVOID>* m_pParamVecs;
	std::vector<int>* m_pFuncUseVecs;	
	std::vector<int>* m_pFuncPriorVecs;
} ;

#endif /*__FUNC_LIST_H__*/
