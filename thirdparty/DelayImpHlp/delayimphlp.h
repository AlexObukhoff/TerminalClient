//	delayimphlp.h: Delay import helper library
//	Developer:	Andrew Solodovnikov
//	E-mail:		none
//	Date:		17.11.2006
//  Version     1.4.2

#ifndef	__DELAYIMPHLP_H__
#define __DELAYIMPHLP_H__


#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#ifndef DL_NO_MT
	#ifdef _MT
		#define DL_MT
	#endif
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4786)
#endif

#ifndef DL_USE_BOOST_PP

#define DL_CAT_(x,y) x##y
#define DL_CAT(x,y) DL_CAT_(x,y)

#define DL_STRINGIZE_(x) #x
#define DL_STRINGIZE(x) DL_STRINGIZE_(x)


#define DL_MAX_REPEAT 16
#define DL_COMMA() ,
#define DL_EMPTY() 


#define DL_BOOL(x)      DL_BOOL_IMPL(x)
#define DL_BOOL_IMPL(x) DL_CAT(DL_BOOL_N, x)

#define DL_BOOL_N0  0
#define DL_BOOL_N1  1
#define DL_BOOL_N2  1
#define DL_BOOL_N3  1
#define DL_BOOL_N4  1
#define DL_BOOL_N5  1
#define DL_BOOL_N6  1
#define DL_BOOL_N7  1
#define DL_BOOL_N8  1
#define DL_BOOL_N9  1
#define DL_BOOL_N10 1
#define DL_BOOL_N11 1
#define DL_BOOL_N12 1
#define DL_BOOL_N13 1
#define DL_BOOL_N14 1
#define DL_BOOL_N15 1
#define DL_BOOL_N16 1


#define DL_IF(c, t, f)       DL_IF_IMPL1(c, t, f)
#define DL_IF_IMPL1(c, t, f) DL_IF_IMPL2(DL_BOOL(c), t ,f)
#define DL_IF_IMPL2(c, t, f) DL_CAT(DL_IF_IMPL_N, c)(t, f)
#define DL_IF_IMPL_N0(_, f) f
#define DL_IF_IMPL_N1(t, _) t


#define DL_COMMA_IF(c) DL_COMMA_IF_IMPL(c)
#define DL_COMMA_IF_IMPL(c) DL_IF(c, DL_COMMA, DL_EMPTY)()

// DL_REPEAT_PARAM_N
#define DL_REPEAT_IMPL_N0(m, d)
#define DL_REPEAT_IMPL_N1(m, d)	m(0, d)
#define DL_REPEAT_IMPL_N2(m, d)	DL_REPEAT_IMPL_N1(m, d) m(1, d)
#define DL_REPEAT_IMPL_N3(m, d)	DL_REPEAT_IMPL_N2(m, d) m(2, d)
#define DL_REPEAT_IMPL_N4(m, d)	DL_REPEAT_IMPL_N3(m, d) m(3, d)
#define DL_REPEAT_IMPL_N5(m, d)	DL_REPEAT_IMPL_N4(m, d) m(4, d)
#define DL_REPEAT_IMPL_N6(m, d)	DL_REPEAT_IMPL_N5(m, d) m(5, d)
#define DL_REPEAT_IMPL_N7(m, d)	DL_REPEAT_IMPL_N6(m, d) m(6, d)
#define DL_REPEAT_IMPL_N8(m, d)	DL_REPEAT_IMPL_N7(m, d) m(7, d)
#define DL_REPEAT_IMPL_N9(m, d)	DL_REPEAT_IMPL_N8(m, d) m(8, d)
#define DL_REPEAT_IMPL_N10(m, d) DL_REPEAT_IMPL_N9(m, d) m(9, d)
#define DL_REPEAT_IMPL_N11(m, d) DL_REPEAT_IMPL_N10(m, d) m(10, d)
#define DL_REPEAT_IMPL_N12(m, d) DL_REPEAT_IMPL_N11(m, d) m(11, d)
#define DL_REPEAT_IMPL_N13(m, d) DL_REPEAT_IMPL_N12(m, d) m(12, d)
#define DL_REPEAT_IMPL_N14(m, d) DL_REPEAT_IMPL_N13(m, d) m(13, d)
#define DL_REPEAT_IMPL_N15(m, d) DL_REPEAT_IMPL_N14(m, d) m(14, d)
#define DL_REPEAT_IMPL_N16(m, d) DL_REPEAT_IMPL_N15(m, d) m(15, d)

#define DL_REPEAT_IMPL_N(n, m, d) DL_CAT(DL_REPEAT_IMPL_N,n)(m, d)
#define DL_REPEAT_N(n, m, d)	DL_REPEAT_IMPL_N(n, m, d)

#define DL_REPEAT_PARAM_IMPL(n, d)  DL_COMMA_IF(n) DL_CAT(d, n)
#define DL_REPEAT_PARAM_N(c, n)	DL_REPEAT_IMPL_N(c, DL_REPEAT_PARAM_IMPL, n)
#define DL_REPEAT_TRAILING_PARAM_N(c, n)	DL_COMMA_IF(c) DL_REPEAT_PARAM_N(c, n)




#define DL_REPEAT_BINARY_PARAM_IMPL0(a, b) a
#define DL_REPEAT_BINARY_PARAM_IMPL1(a, b) b

#define DL_REPEAT_BINARY_PARAM_IMPL2(n, p1, p2) DL_COMMA_IF(n) DL_CAT(p1,n) DL_CAT(p2,n)
#define DL_REPEAT_BINARY_PARAM_IMPL(n, d) DL_REPEAT_BINARY_PARAM_IMPL2(n, DL_REPEAT_BINARY_PARAM_IMPL0 d, DL_REPEAT_BINARY_PARAM_IMPL1 d)

#define DL_REPEAT_BINARY_PARAM_N(c, p1, p2) DL_REPEAT_IMPL_N(c, DL_REPEAT_BINARY_PARAM_IMPL, (p1,p2))
#define DL_REPEAT_TRAILING_BINARY_PARAM_N(c, p1, p2)	DL_COMMA_IF(c) DL_REPEAT_BINARY_PARAM_N(c, p1, p2)



// DL_SEQ_SIZE

#define DL_SEQ_SIZE(seq)	DL_SEQ_SIZE_IMPL(seq)
#define DL_SEQ_SIZE_IMPL(seq)	DL_CAT(DL_N_, DL_SEQ_SIZE_0 seq)

#define DL_SEQ_SIZE_0(_)	DL_SEQ_SIZE_1
#define DL_SEQ_SIZE_1(_)	DL_SEQ_SIZE_2
#define DL_SEQ_SIZE_2(_)	DL_SEQ_SIZE_3
#define DL_SEQ_SIZE_3(_)	DL_SEQ_SIZE_4
#define DL_SEQ_SIZE_4(_)	DL_SEQ_SIZE_5
#define DL_SEQ_SIZE_5(_)	DL_SEQ_SIZE_6
#define DL_SEQ_SIZE_6(_)	DL_SEQ_SIZE_7
#define DL_SEQ_SIZE_7(_)	DL_SEQ_SIZE_8
#define DL_SEQ_SIZE_8(_)	DL_SEQ_SIZE_9
#define DL_SEQ_SIZE_9(_)	DL_SEQ_SIZE_10
#define DL_SEQ_SIZE_10(_)	DL_SEQ_SIZE_11
#define DL_SEQ_SIZE_11(_)	DL_SEQ_SIZE_12
#define DL_SEQ_SIZE_12(_)	DL_SEQ_SIZE_13
#define DL_SEQ_SIZE_13(_)	DL_SEQ_SIZE_14
#define DL_SEQ_SIZE_14(_)	DL_SEQ_SIZE_15
#define DL_SEQ_SIZE_15(_)	DL_SEQ_SIZE_16

#define DL_N_DL_SEQ_SIZE_0	0
#define DL_N_DL_SEQ_SIZE_1	1
#define DL_N_DL_SEQ_SIZE_2	2
#define DL_N_DL_SEQ_SIZE_3	3
#define DL_N_DL_SEQ_SIZE_4	4
#define DL_N_DL_SEQ_SIZE_5	5
#define DL_N_DL_SEQ_SIZE_6	6
#define DL_N_DL_SEQ_SIZE_7	7
#define DL_N_DL_SEQ_SIZE_8	8
#define DL_N_DL_SEQ_SIZE_9	9
#define DL_N_DL_SEQ_SIZE_10	10
#define DL_N_DL_SEQ_SIZE_11	11
#define DL_N_DL_SEQ_SIZE_12	12
#define DL_N_DL_SEQ_SIZE_13	13
#define DL_N_DL_SEQ_SIZE_14	14
#define DL_N_DL_SEQ_SIZE_15	15
#define DL_N_DL_SEQ_SIZE_16	16

// DL_SEQ_ENUM

#define DL_SEQ_ENUM(seq)	DL_SEQ_ENUM_IMPL(seq)
#define DL_SEQ_ENUM_IMPL(seq)	DL_CAT(DL_SEQ_ENUM_, DL_SEQ_SIZE(seq)) seq

#define	DL_SEQ_ENUM_1(x)	x
#define	DL_SEQ_ENUM_2(x)	x, DL_SEQ_ENUM_1
#define	DL_SEQ_ENUM_3(x)	x, DL_SEQ_ENUM_2
#define	DL_SEQ_ENUM_4(x)	x, DL_SEQ_ENUM_3
#define	DL_SEQ_ENUM_5(x)	x, DL_SEQ_ENUM_4
#define	DL_SEQ_ENUM_6(x)	x, DL_SEQ_ENUM_5
#define	DL_SEQ_ENUM_7(x)	x, DL_SEQ_ENUM_6
#define	DL_SEQ_ENUM_8(x)	x, DL_SEQ_ENUM_7
#define	DL_SEQ_ENUM_9(x)	x, DL_SEQ_ENUM_8
#define	DL_SEQ_ENUM_10(x)	x, DL_SEQ_ENUM_9
#define	DL_SEQ_ENUM_11(x)	x, DL_SEQ_ENUM_10
#define	DL_SEQ_ENUM_12(x)	x, DL_SEQ_ENUM_11
#define	DL_SEQ_ENUM_13(x)	x, DL_SEQ_ENUM_12
#define	DL_SEQ_ENUM_14(x)	x, DL_SEQ_ENUM_13
#define	DL_SEQ_ENUM_15(x)	x, DL_SEQ_ENUM_14
#define	DL_SEQ_ENUM_16(x)	x, DL_SEQ_ENUM_15

#else

#define DL_MAX_REPEAT 32

#include <boost/preprocessor/cat.hpp> 
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>

#define DL_COMMA BOOST_PP_COMMA
#define DL_EMPTY BOOST_PP_EMPTY

#define DL_CAT BOOST_PP_CAT
#define DL_STRINGIZE BOOST_PP_STRINGIZE

#define DL_BOOL     BOOST_PP_BOOL
#define DL_IF       BOOST_PP_IF
#define DL_COMMA_IF BOOST_PP_COMMA_IF

#define DL_SEQ_ENUM BOOST_PP_SEQ_ENUM
#define DL_SEQ_SIZE BOOST_PP_SEQ_SIZE

#define DL_REPEAT_N BOOST_PP_REPEAT

#define DL_REPEAT_PARAM_N BOOST_PP_ENUM_PARAMS
#define DL_REPEAT_TRAILING_PARAM_N BOOST_PP_ENUM_TRAILING_PARAMS

#define DL_REPEAT_BINARY_PARAM_N BOOST_PP_ENUM_BINARY_PARAMS
#define DL_REPEAT_TRAILING_BINARY_PARAM_N BOOST_PP_ENUM_TRAILING_BINARY_PARAMS

#endif

//  delay load names


namespace delayload
{


template <bool>
struct SelectTypeTraits
{
	template <class T, class U>
	struct Type
	{
		typedef T type;
	};
};

template <>
struct SelectTypeTraits<false>
{
	template <class T, class U>
	struct Type
	{
		typedef U type;
	};
};

template<bool flag, class T, class U>
struct SelectType
{
	typedef typename SelectTypeTraits<flag>::template Type<T, U>::type value;
};


template <class T>
struct Type2Type
{
	typedef T type;
};

template <class T, class U>
struct IsEqualType
{
private:
	typedef char yes;
	typedef char no[2];
	static yes &check(Type2Type<T>);
	static no &check(...);
public:
	enum {value = sizeof(check(Type2Type<U>())) == sizeof(yes)};
};



#define DL_NAME_ID(id)	DL_CAT(CNameId, id)

#define DL_DECLARE_NAME_ID_IMPL(id, name, ret, text)\
struct DL_NAME_ID(id)\
{\
	enum {length = sizeof(name)};\
	static ret GetStr(){return (ret)text(name);}\
};


#define DL_DECLARE_NAME_ID_A(id, name)	DL_DECLARE_NAME_ID_IMPL(id, name, LPCSTR, DL_EMPTY())
#define DL_DECLARE_NAME_ID(id, name)	DL_DECLARE_NAME_ID_IMPL(id, name, LPCTSTR,_T)


#ifdef DL_MT
//  MT only
struct CLWMutex
{
    CLWMutex(volatile LONG &pFlag):m_pFlag(pFlag)
    {
    }
    CLWMutex(const CLWMutex &lwSource): m_pFlag(lwSource.m_pFlag)
	{
	}
    CLWMutex & operator=(const CLWMutex &lwSource)
	{
		m_pFlag = lwSource.m_pFlag;
		return *this;
	}
	void Lock()
	{
        while(::InterlockedExchange(&m_pFlag, TRUE))
            ::Sleep(1);
	}
	void Unlock()
	{
        ::InterlockedExchange(&m_pFlag, FALSE);
	}
private:
    volatile LONG &m_pFlag;
};

template<class T>
struct CAutoLock
{
	CAutoLock(T& obj):m_objLock(obj)
	{
		m_objLock.Lock();
	}
	~CAutoLock()
	{
		m_objLock.Unlock();
	}
private:
    CAutoLock(const CAutoLock &);
    CAutoLock & operator=(const CAutoLock &);
private:
	T &m_objLock;
};

#endif //DL_MT


struct CModuleLoadLibraryPolicy
{
	static HMODULE Load(LPCTSTR szFileName)
	{
		return ::LoadLibrary(szFileName);
	}
	static BOOL Free(HMODULE hModule)
	{
		return ::FreeLibrary(hModule);
	}
};

struct CModuleGetModuleHandlePolicy
{
	static HMODULE Load(LPCTSTR szFileName)
	{
		return ::GetModuleHandle(szFileName);
	}
	static BOOL Free(HMODULE /*hModule*/)
	{
		return TRUE;
	}
};

template <class Name, class LoadPolicy = CModuleLoadLibraryPolicy>
class CModule
{
public:
	typedef CModule<Name, LoadPolicy>	type;
	typedef Name						name_type;
	static type &GetModule()
	{
#ifdef DL_MT
		static volatile LONG lMutex = FALSE;
		CLWMutex theMutex(lMutex);
		CAutoLock<CLWMutex> autoLock(theMutex);
#endif //DL_MT
		static type Module;
		return Module;
	}
	HMODULE GetModuleHandle() const
	{
		return m_hModule;
	}
	HMODULE LoadModule()
	{
		return (m_hModule = LoadPolicy::Load(name_type::GetStr()));
	}
	BOOL IsLoaded() const
	{
		return m_hModule != NULL;
	}
//  Caution - use with care. Not thread-safe
	BOOL UnloadModule()
	{
		HMODULE hModule = m_hModule;
		m_hModule = NULL;
		return LoadPolicy::Free(hModule);
	}
	~CModule()
	{
		if (m_hModule)
			UnloadModule();
	}
private:
	CModule()
	{
		LoadModule();
	}
	HMODULE m_hModule;
};


// try to minimize proxy function size

static BOOL inline DL_GetProcAddressImpl(
#ifdef DL_MT
							volatile LONG &pMutex,
							const FARPROC pProxyDef,
#endif // DL_MT
							volatile FARPROC &pProxy,
							HMODULE hModule, 
							LPCSTR lpProcName
						   )
{
#ifdef DL_MT
		CLWMutex theMutex(pMutex);
		CAutoLock<CLWMutex> autoLock(theMutex);
	//  test for first entry
		if (pProxy != pProxyDef)
			return TRUE;
#endif // DL_MT
		FARPROC pFunction = ::GetProcAddress(hModule, lpProcName);
		if (pFunction)
		{
			pProxy = pFunction;
			return TRUE;
		}
		return FALSE;
}

template <class Module, class Name, class Proxy>
class CDynFunction
{
public:
	typedef CDynFunction<Module, Name, Proxy> type;
	typedef Proxy							  proxy_type;
	typedef Module							  module_type;
	typedef Name							  name_type;
	typedef typename proxy_type::fun_type	  fun_type; 
	
	static fun_type &GetProxy()
	{
		static fun_type proxy = proxy_type::template Proxy<type>::ProxyFun;
		return proxy;
	}
	static void Reset()
	{
		GetProxy() = proxy_type::template Proxy<type>::ProxyFun;
	}
	static BOOL IsProxy()
	{
		return (GetProxy() == proxy_type::template Proxy<type>::ProxyFun);
	}
	static BOOL InitFunction()
	{
#ifdef DL_MT
		static volatile LONG lMutex = FALSE;
#endif // DL_MT
		const module_type &theModule = module_type::GetModule();
		if (theModule.IsLoaded())
			return DL_GetProcAddressImpl(
#ifdef DL_MT
											lMutex,
											(const FARPROC)proxy_type::template Proxy<type>::ProxyFun,
#endif //DL_MT
											(volatile FARPROC &)GetProxy(),
											theModule.GetModuleHandle(),
											name_type::GetStr()
										);
		return FALSE;
	}
};

struct CDynFunException
{
	CDynFunException(): m_sMessage(NULL)
	{
	};
	~CDynFunException()
	{
		free(m_sMessage);
	};
	CDynFunException(LPCTSTR sMessage):m_sMessage(NULL)
	{
		SetMessage(sMessage);
	}
	CDynFunException(const CDynFunException &other):m_sMessage(NULL)
	{
		SetMessage(other.m_sMessage);
	}
	CDynFunException &operator = (const CDynFunException &other)
	{
		SetMessage(other.m_sMessage);
		return *this;
	}
	void SetMessage(LPCTSTR sMessage)
	{
		free(m_sMessage);
		m_sMessage = (LPTSTR)malloc((_tcslen(sMessage) + 1) * sizeof(TCHAR));
		if (m_sMessage)
			_tcscpy_s(m_sMessage, _tcslen(m_sMessage), sMessage);
	}
	LPCTSTR GetMessage() const
	{
		return m_sMessage;
	}
private:
	LPTSTR m_sMessage;
};

template<class E = CDynFunException>
struct CFunProxyThrowPolicy
{
	template <class DynFunction> 
	struct FunctionTrait
	{
	//  we don't care about return value - anywhere it could not be used
		typedef typename DynFunction::proxy_type::ret_type raw_ret_type;
		typedef typename SelectType<IsEqualType<raw_ret_type, void>::value, int, raw_ret_type>::value ret_type;
		static  ret_type MakeReturn()
		{
			TCHAR szMessage[DynFunction::name_type::length + 64];
			_stprintf_s(szMessage, _T("Can't resolve procedure <%hs>: %d"), DynFunction::name_type::GetStr(), GetLastError());
			throw E(szMessage);
		//	return ret_type();
		}
	};
};


//  we need not to implement void return type value policy, 
//  coz void function can only throw on error

template<class R, R value = R()>
struct CFunProxyValuePolicy
{
	template <class DynFunction> 
	struct FunctionTrait
	{
		static typename DynFunction::proxy_type::ret_type MakeReturn()
		{
			return value;
		}
	};
};

struct CFunProxyDefPolicy
{
    template <class DynFunction> 
    struct FunctionTrait
    {
        static typename DynFunction::proxy_type::ret_type MakeReturn()
        {
            return DynFunction::proxy_type::ret_type();
        }
    };
};


#define DL_FUN_PROXY_CC(cc) DL_CAT(CFunProxy, cc)

#define DL_FUN_PROXY(cc, n) DL_CAT(DL_FUN_PROXY_CC(cc),n)

#define DL_FUN_PROXY_IMPL(cc, n) DL_CAT(DL_FUN_PROXY(cc, n),Impl)
#define DL_FUN_PROXY_IMPL1(cc, n) DL_CAT(DL_FUN_PROXY(cc, n),Impl1)

#define DL_DECLARE_FUN_PROXY_IMPL(call_conv, param_count) \
template <typename R>\
struct DL_FUN_PROXY_IMPL(call_conv, param_count)\
{\
	template <class DynFunction DL_REPEAT_TRAILING_PARAM_N(param_count, typename P), class Policy> struct RetProxy\
	{\
		static R call_conv ProxyFun(DL_REPEAT_BINARY_PARAM_N(param_count, P, v))\
		{\
			if (DynFunction::InitFunction())\
				return DynFunction::GetProxy()(DL_REPEAT_PARAM_N(param_count, v));\
			return Policy::template FunctionTrait<DynFunction>::MakeReturn();\
		}\
	};\
};\
\
template <>\
struct DL_FUN_PROXY_IMPL(call_conv, param_count) <void>\
{\
	template <class DynFunction DL_REPEAT_TRAILING_PARAM_N(param_count, typename P), class Policy> struct RetProxy\
	{\
		static void call_conv ProxyFun(DL_REPEAT_BINARY_PARAM_N(param_count, P, v))\
		{\
			if (DynFunction::InitFunction())\
				DynFunction::GetProxy()(DL_REPEAT_PARAM_N(param_count, v));\
			else\
				Policy::template FunctionTrait<DynFunction>::MakeReturn();\
		}\
	};\
};

//  instantiate ProxyFunImpl for zero parameters count


// VC6 bug - nested class inheritance from class instantiated with selecttype causes compiler error

#define DL_DECLARE_FUN_PROXY_IMPL1(call_conv, param_count) \
DL_DECLARE_FUN_PROXY_IMPL(call_conv, param_count) \
template <class R DL_REPEAT_TRAILING_PARAM_N(param_count, typename P), class DynFunction, class Policy>\
struct DL_FUN_PROXY_IMPL1(call_conv, param_count):public \
	SelectType<\
				IsEqualType<P0, void>::value,\
				typename DL_FUN_PROXY_IMPL(call_conv, 0)<R>::template RetProxy<DynFunction, Policy>,\
				typename DL_FUN_PROXY_IMPL(call_conv, param_count)<R>::template RetProxy<DynFunction DL_REPEAT_TRAILING_PARAM_N(param_count, P), Policy> \
			  >::value\
{\
};

#define DL_DECLARE_FUN_PROXY(call_conv, param_count) \
DL_DECLARE_FUN_PROXY_IMPL1(call_conv, param_count) \
template <typename R DL_REPEAT_TRAILING_PARAM_N(param_count, typename P), class Policy = CFunProxyValuePolicy<R> >\
struct DL_FUN_PROXY(call_conv, param_count)\
{\
	typedef R (call_conv *fun_type)(DL_REPEAT_PARAM_N(param_count, P));\
	typedef R ret_type;\
	template <class DynFunction> struct Proxy:public DL_FUN_PROXY_IMPL1(call_conv, param_count)<R DL_REPEAT_TRAILING_PARAM_N(param_count, P), DynFunction, Policy>\
	{\
	};\
};



#define DL_REPEAT_DECLARE_FUN_PROXY_IMPL0(n, cc)
#define DL_REPEAT_DECLARE_FUN_PROXY_IMPL1(n, cc) DL_DECLARE_FUN_PROXY(cc, n)
#define DL_REPEAT_DECLARE_FUN_PROXY_IMPL2(n, cc) DL_IF(DL_BOOL(n), DL_REPEAT_DECLARE_FUN_PROXY_IMPL1, DL_REPEAT_DECLARE_FUN_PROXY_IMPL0)(n, cc)

#ifndef DL_USE_BOOST_PP
#define DL_REPEAT_DECLARE_FUN_PROXY_IMPL(n, cc) DL_REPEAT_DECLARE_FUN_PROXY_IMPL2(n, cc)
#else
#define DL_REPEAT_DECLARE_FUN_PROXY_IMPL(a, n, cc) DL_REPEAT_DECLARE_FUN_PROXY_IMPL2(n, cc)
#endif

#define DL_REPEAT_DECLARE_FUN_PROXY(cc) DL_REPEAT_N(DL_MAX_REPEAT, DL_REPEAT_DECLARE_FUN_PROXY_IMPL, cc)

}


#define DL_DECLARE_FUN_PROXY_CC(call_conv) \
namespace delayload\
{\
DL_DECLARE_FUN_PROXY_IMPL(call_conv, 0)\
DL_REPEAT_DECLARE_FUN_PROXY(call_conv/*1...DL_MAX_REPEAT*/)\
}

//  instantiate winapi proxy
#ifndef DL_NO_STDCALL
	DL_DECLARE_FUN_PROXY_CC(WINAPI)
#endif /*DL_NO_WINAPI*/

//  instantiate __cdecl proxy

#ifdef DL_USE_CDECL
	DL_DECLARE_FUN_PROXY_CC(__cdecl)
#endif /*DL_USE_CDECL*/

// usefull macro's


//  module definitions

#define DL_USE_MODULE_LOAD_POLICY_BEGIN(nmspace, name, load_policy) \
namespace nmspace \
{\
	namespace internal_types \
	{\
		DL_DECLARE_NAME_ID(nmspace, name)\
		typedef delayload::CModule<DL_NAME_ID(nmspace), load_policy> module_type;\
	}

#define DL_USE_MODULE_BEGIN(nmspace, name) DL_USE_MODULE_LOAD_POLICY_BEGIN(nmspace, name, delayload::CModuleLoadLibraryPolicy)
#define DL_USE_MODULE_NON_LOAD_BEGIN(nmspace, name) DL_USE_MODULE_LOAD_POLICY_BEGIN(nmspace, name, delayload::CModuleGetModuleHandlePolicy)

#define DL_USE_MODULE_END() \
}


//  function definitions

#define DL_FUN_TYPE(name_id) DL_CAT(name_id, _fun_type)

#define DL_DECLARE_FUN_ERR_POLICY_CC_EX(cc, name, name_id, r, p, pl) \
namespace internal_types \
{\
	DL_DECLARE_NAME_ID_A(name, name_id)\
	typedef delayload::CDynFunction<module_type, DL_NAME_ID(name), delayload::DL_FUN_PROXY(cc, DL_SEQ_SIZE(p))<r, DL_SEQ_ENUM(p), pl > > DL_FUN_TYPE(name);\
}\
static internal_types::DL_FUN_TYPE(name)::fun_type &name = internal_types::DL_FUN_TYPE(name)::GetProxy();

//  extended set of macroses to support import by ord && rename imported functions

#define DL_DECLARE_FUN_ERR_POLICY_EX(name, name_id, r, p, pl) DL_DECLARE_FUN_ERR_POLICY_CC_EX(WINAPI, name, name_id, r, p, pl)
#define DL_DECLARE_FUN_CC_EX(cc, name, name_id, r, p)         DL_DECLARE_FUN_ERR_POLICY_CC_EX(cc, name, name_id, r, p, delayload::CFunProxyDefPolicy)
#define DL_DECLARE_FUN_EX(name, name_id, r, p)                DL_DECLARE_FUN_CC_EX(WINAPI, name, name_id, r, p)
#define DL_DECLARE_FUN_THROW_CC_EX(cc, name, name_id, r, p)   DL_DECLARE_FUN_ERR_POLICY_CC_EX(cc, name, name_id, r, p, delayload::CFunProxyThrowPolicy<>)
#define DL_DECLARE_FUN_THROW_EX(name, name_id, r, p)          DL_DECLARE_FUN_THROW_CC_EX(WINAPI, name, name_id, r, p)
#define DL_DECLARE_FUN_ERR_CC_EX(cc, name, name_id, r, p, e)  DL_DECLARE_FUN_ERR_POLICY_CC_EX(cc, name, name_id, r, p, delayload::CFunProxyValuePolicy<r DL_COMMA() e>)
#define DL_DECLARE_FUN_ERR_EX(name, name_id, r, p, e)         DL_DECLARE_FUN_ERR_CC_EX(WINAPI, name, name_id, r, p, e)

//  simplified set of macroses, compatible with previous versions

#define DL_DECLARE_FUN_ERR_POLICY_CC(cc, name, r, p, pl) DL_DECLARE_FUN_ERR_POLICY_CC_EX(cc, name, DL_STRINGIZE(name), r, p, pl)
#define DL_DECLARE_FUN_ERR_POLICY(name, r, p, pl)        DL_DECLARE_FUN_ERR_POLICY_EX(name, DL_STRINGIZE(name), r, p, pl)
#define DL_DECLARE_FUN_CC(cc, name, r, p)                DL_DECLARE_FUN_CC_EX(cc, name, DL_STRINGIZE(name), r, p)
#define DL_DECLARE_FUN(name, r, p)                       DL_DECLARE_FUN_EX(name, DL_STRINGIZE(name), r, p)
#define DL_DECLARE_FUN_THROW_CC(cc, name, r, p)          DL_DECLARE_FUN_THROW_CC_EX(cc, name, DL_STRINGIZE(name), r, p)
#define DL_DECLARE_FUN_THROW(name, r, p)                 DL_DECLARE_FUN_THROW_EX(name, DL_STRINGIZE(name), r, p)
#define DL_DECLARE_FUN_ERR_CC(cc, name, r, p, e)         DL_DECLARE_FUN_ERR_CC_EX(cc, name, DL_STRINGIZE(name), r, p, e)
#define DL_DECLARE_FUN_ERR(name, r, p, e)                DL_DECLARE_FUN_ERR_EX(name, DL_STRINGIZE(name), r, p, e)

#define DL_GET_MODULE(nmspace) nmspace::internal_types::module_type::GetModule()
#define DL_LOAD_MODULE(nmspace) DL_GET_MODULE(nmspace).LoadModule()
#define DL_UNLOAD_MODULE(nmspace) DL_GET_MODULE(nmspace).UnloadModule()
#define DL_IS_MODULE_LOADED(nmspace) DL_GET_MODULE(nmspace).IsLoaded()

#define DL_GET_FUN_TYPE(nmspace, name) nmspace::internal_types::DL_FUN_TYPE(name)
#define DL_RESET_FUN(nmspace, name)  DL_GET_FUN_TYPE(nmspace, name)::Reset()
#define DL_IS_FUN_PROXY(nmspace, name) DL_GET_FUN_TYPE(nmspace, name)::IsProxy()

#endif
