#ifndef __FUNC_LIST_H__
#define __FUNC_LIST_H__

typedef int (*FuncCall_t)(PVOID pParam,PVOID pInput);

class CFuncList
{
public:
	CFuncList();
	~CFuncList();
	int AddFuncList(FuncCall_t pFunc,PVOID pParam);
	int RemoveFuncList(FuncCall_t pFunc);
	int CallList(LPVOID pParam);

private:
	int __RemoveFunc(FuncCall_t pFunc);
	FuncCall_t __GetFuncCall(int idx,LPVOID& pParam);
	int __PutFuncCall(FuncCall_t pFunc);
private:
	CRITICAL_SECTION m_FuncListCS;
	std::vector<FuncCall_t> m_FuncVecs;
	std::vector<PVOID> m_ParamVecs;
	std::vector<int> m_FuncUseVecs;	
} ;

#endif /*__FUNC_LIST_H__*/