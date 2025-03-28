#include <Windows.h>
#include "../valve_sdk/sdk.hpp"

class KeyValues
{
public:
    void* operator new(size_t allocatedsize)
    {
        static PVOID pKeyValuesSystem;
        if (!pKeyValuesSystem) {
            auto KeyValuesSystem = reinterpret_cast<PVOID(__cdecl*)()>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "KeyValuesSystem"));
            pKeyValuesSystem = KeyValuesSystem();
        }

        return CallVFunction<PVOID(__thiscall*)(PVOID, size_t)>(pKeyValuesSystem, 1)(pKeyValuesSystem, allocatedsize);
    }

    void operator delete(void* mem)
    {
        static PVOID pKeyValuesSystem;
        if (!pKeyValuesSystem) {
            auto KeyValuesSystem = reinterpret_cast<PVOID(__cdecl*)()>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "KeyValuesSystem"));
            pKeyValuesSystem = KeyValuesSystem();
        }

        CallVFunction<void(__thiscall*)(PVOID, PVOID)>(pKeyValuesSystem, 2)(pKeyValuesSystem, mem);
    }

    KeyValues(const char* setName)
    {
        Init();
        SetName(setName);
    }

    void SetName(const char* setName)
    {
        m_iKeyName = 2;
    }

    enum types_t
    {
        TYPE_NONE = 0,
        TYPE_STRING,
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_PTR,
        TYPE_WSTRING,
        TYPE_COLOR,
        TYPE_UINT64,
        TYPE_NUMTYPES,
    };

    void Init()
    {
        m_iKeyName = -1;
        m_iDataType = TYPE_NONE;

        m_pSub = NULL;
        m_pPeer = NULL;
        m_pChain = NULL;

        m_sValue = NULL;
        m_wsValue = NULL;
        m_pValue = NULL;

        m_bHasEscapeSequences = false;

        // for future proof
        memset(unused, 0, sizeof(unused));
    }

    int m_iKeyName; // keyname is a symbol defined in KeyValuesSystem

// These are needed out of the union because the API returns string pointers
    char* m_sValue;
    wchar_t* m_wsValue;

    union
    {
        int m_iValue;
        float m_flValue;
        void* m_pValue;
        unsigned char m_Color[4];
    };

    char       m_iDataType;
    char       m_bHasEscapeSequences; // true, if while parsing this KeyValue, Escape Sequences are used (default false)
    char       m_bEvaluateConditionals; // true, if while parsing this KeyValue, conditionals blocks are evaluated (default true)
    char       unused[1];

    KeyValues* m_pPeer; // pointer to next key in list
    KeyValues* m_pSub;  // pointer to Start of a new sub key list
    KeyValues* m_pChain;// Search here if it's not in our list

    template<class CDATA> CDATA GetValueByKey(const char* keyname)
    {
        KeyValues* pFind = FindKey(keyname.c_str(), false);
        if (pFind)
        {
            CDATA return_value;
            std::stringstream ss(GetString(this, keyname, NULL));
            ss >> return_value;
            return return_value;
        }
        else return NULL;
    }

    KeyValues* FindKey(const char* keyName, bool bCreate);

    const char* GetString(KeyValues* pThis, const char* keyName, const char* defaultValue);
    const char* GetStringxD(const char* keyName, const char* defaultValue);

    bool LoadFromBuffer(KeyValues* pThis, const char* pszFirst, const char* pszSecond, PVOID pSomething = 0, PVOID pAnother = 0, PVOID pLast = 0, PVOID pAnother2 = 0);

    void SetString(const char* name, const char* value);
    void SetInt(const char* name, int value);
    void SetFloat(const char* name, float value);

};