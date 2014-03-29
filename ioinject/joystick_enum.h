
#ifndef __JOYSTICK_ENUM_H__
#define __JOYSTICK_ENUM_H__

class JoyStickBase
{
public:
    JoyStickBase();
    virtual ~JoyStickBase();
    virtual int SetJoyStickConfig(const char* name,void* pConfig,int configlen);
    virtual int
};

class JoyStickConfigBase
{
public:
    JoyStickConfigBase();
    virtual ~JoyStickConfigBase();
    virtual int SetJoyStickConfig(const char* name,void* pConfig,int configlen);
    virtual HRESULT CallEnumTypes(LPDIJOYTYPECALLBACK lpCallback,LPVOID pvRef);
    virtual HRESULT GetTypeInfo(LPCWSTR pwszTypeName,LPDIJOYTYPEINFO pjti,DWORD dwFlags);
    virtual HRESULT SetTypeInfo(LPCWSTR pwszTypeName,LPDIJOYTYPEINFO pjti, DWORD dwFlags);
    virtual HRESULT DeleteType(LPCWSTR pwszTypeName);
};

class JoyStickEnum
{
}

#endif /*__JOYSTICK_ENUM_H__*/

