/*
Original code by Yang G (pinner@mail.ustc.edu.cn)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
//
#include "Public.h"

//
#include "ScriptHandle.h"

//
#include "ScriptExporter.h"

//

namespace bin
{
	int CScriptHandle::Get(const char* pszName, CScriptTable& tbl)
	{
		tbl.UnRef();

		return PostFmLua(Get<CScriptTable>(pszName, tbl), tbl);
	}

	int CScriptHandle::Get(const char* pszName, CScriptUserData& ud)
	{
		ud.UnRef();

		return PostFmLua(Get<CScriptUserData>(pszName, ud), ud);
	}

	int CScriptHandle::PostFmLua(int nFmLuaRet, CScriptTable& tbl)
	{
		if(nFmLuaRet)
		{
			CScriptTable::RefNode& ref = tbl.m_ref;
			ref.LinkAfter(&m_head);
		}
		else
		{
			tbl.UnRef();
		}

		return nFmLuaRet;
	}

	int CScriptHandle::PostFmLua(int nFmLuaRet, CScriptUserData& ud)
	{
		if(nFmLuaRet)
		{
			CScriptUserData::RefNode& ref = ud.m_ref;
			ref.LinkAfter(&m_head);
		}
		else
		{
			ud.UnRef();
		}

		return nFmLuaRet;
	}

	void CScriptHandle::InvlRetVal(CScriptTable& tbl)
	{
		tbl.UnRef();
	}

	void CScriptHandle::InvlRetVal(CScriptUserData& ud)
	{
		ud.UnRef();
	}

	int CScriptHandle::NewTable(CScriptTable& tbl)
	{
		tbl.UnRef();

		if(IsNull())
		{
			return 0;
		}

		CHECK_LUA_STACK(m_pLua);

		lua_newtable(m_pLua);

		return PostFmLua(TFmLua<CScriptTable>::Make(m_pLua, -1, tbl), tbl);
	}

	int CScriptHandle::NewTable(const char* pszName, CScriptTable& tbl)
	{
		if(!NewTable(tbl))
		{
			return 0;
		}

		if(!Set(pszName, tbl))
		{
			tbl.UnRef();
		}

		return tbl.IsReferd() ? 1 : 0;
	}

	int IScriptADBase::PrepareStack()
	{
		ScriptExporterManager().CheckRefsTable(m_ref.pLua);		// __bin_refs

		lua_rawgeti(m_ref.pLua, -1, m_ref.nRef);				// __bin_refs this_table

		return 1;
	}

	int IScriptADBase::PostFmLua(int nFmLuaRet, CScriptTable& tbl)
	{
		if(nFmLuaRet)
		{
			CScriptTable::RefNode& ref = tbl.m_ref;
			ref.LinkAfter(&m_ref);
		}
		else
		{
			tbl.UnRef();
		}

		return nFmLuaRet;
	}

	int IScriptADBase::PostFmLua(int nFmLuaRet, CScriptUserData& ud)
	{
		if(nFmLuaRet)
		{
			CScriptUserData::RefNode& ref = ud.m_ref;
			ref.LinkAfter(&m_ref);
		}
		else
		{
			ud.UnRef();
		}

		return nFmLuaRet;
	}

	void IScriptADBase::InvlRetVal(CScriptTable& tbl)
	{
		tbl.UnRef();
	}

	void IScriptADBase::InvlRetVal(CScriptUserData& ud)
	{
		ud.UnRef();
	}

	int IScriptADBase::Get(const char* pszName, CScriptTable& tbl)
	{
		tbl.UnRef();

		return PostFmLua(Get<CScriptTable>(pszName, tbl), tbl);
	}

	int IScriptADBase::Get(const char* pszName, CScriptUserData& ud)
	{
		ud.UnRef();

		return PostFmLua(Get<CScriptUserData>(pszName, ud), ud);
	}

	int IScriptADBase::Get(int nIdx, CScriptTable& tbl)
	{
		tbl.UnRef();

		return PostFmLua(Get<CScriptTable>(nIdx, tbl), tbl);
	}

	int IScriptADBase::Get(int nIdx, CScriptUserData& ud)
	{
		ud.UnRef();

		return PostFmLua(Get<CScriptUserData>(nIdx, ud), ud);
	}

	// Static
	void IScriptADBase::SRefNode::UnLinker(SScriptHandleRefNode* pT)
	{
		SRefNode* pThis = static_cast<SRefNode*>(pT);

		pThis->Unlink();	// UnLink to ref list

		if(pThis->pLua && pThis->nRef!=LUA_NOREF)
		{
			CHECK_LUA_STACK(pThis->pLua);

			ScriptExporterManager().CheckRefsTable(pThis->pLua);	// Refs table is on the stack
			luaL_unref(pThis->pLua, -1, pThis->nRef);
		}

		pThis->pLua = NULL;
		pThis->nRef = LUA_NOREF;
	}

	int TFmLua<CScriptTable>::Make(lua_State* pL, int nIdx, CScriptTable& tbl)
	{
		tbl.UnRef();

		if(!lua_istable(pL, nIdx))
		{
			return 0;
		}

		CHECK_LUA_STACK(pL);

		nIdx = lua_absindex(pL, nIdx);	// To absolute index, avoid negative index

		CScriptTable::RefNode& ref = tbl.m_ref;

		ScriptExporterManager().CheckRefsTable(pL);
		lua_pushvalue(pL, nIdx);

		ref.pLua = pL;
		ref.nRef = luaL_ref(pL, -2);

		return 1;
	}

	int TFmLua<CScriptUserData>::Make(lua_State* pL, int nIdx, CScriptUserData& ud)
	{
		ud.UnRef();

		if(!lua_isuserdata(pL, nIdx))
		{
			return 0;
		}

		SScriptProxy* pProxy = (SScriptProxy*)lua_touserdata(pL, nIdx);

		if(!pProxy)
		{
			return 0;
		}

		// Check whether the proxy is associate with a valid object
		if(pProxy->ePT != SScriptProxy::EPT_OBJECT || !pProxy->objRef.pObject)
		{
			return 0;
		}

		CHECK_LUA_STACK(pL);

		nIdx = lua_absindex(pL, nIdx);	// To absolute index, avoid negative index

		CScriptUserData::RefNode& ref = ud.m_ref;

		ScriptExporterManager().CheckRefsTable(pL);
		lua_pushvalue(pL, nIdx);

		ref.pLua = pL;
		ref.nRef = luaL_ref(pL, -2);

		return 1;
	}

	int TToLua<CScriptTable>::Make(CScriptTable& tbl, lua_State* pL)
	{
		if(!tbl.IsReferd())
		{
			lua_pushnil(pL);

			return 1;
		}

		CScriptTable::RefNode& ref = tbl.m_ref;
		if(ref.pLua != pL)
		{
			return 0;
		}

		ScriptExporterManager().CheckRefsTable(pL);
		lua_rawgeti(pL, -1, ref.nRef);

		lua_replace(pL, -2);

		return 1;
	}

	int TToLua<CScriptUserData>::Make(CScriptUserData& ud, lua_State* pL)
	{
		if(!ud.IsReferd())
		{
			lua_pushnil(pL);

			return 1;
		}

		CScriptUserData::RefNode& ref = ud.m_ref;
		if(ref.pLua != pL)
		{
			return 0;
		}

		ScriptExporterManager().CheckRefsTable(pL);
		lua_rawgeti(pL, -1, ref.nRef);

		lua_replace(pL, -2);

		return 1;
	}
}