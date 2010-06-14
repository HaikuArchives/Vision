/* 
 * The contents of this file are subject to the Mozilla Public 
 * License Version 1.1 (the "License"); you may not use this file 
 * except in compliance with the License. You may obtain a copy of 
 * the License at http://www.mozilla.org/MPL/ 
 * 
 * Software distributed under the License is distributed on an "AS 
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or 
 * implied. See the License for the specific language governing 
 * rights and limitations under the License. 
 * 
 * The Original Code is Vision. 
 * 
 * The Initial Developer of the Original Code is The Vision Team.
 * Portions created by The Vision Team are
 * Copyright (C) 1999-2010 The Vision Team.	All Rights
 * Reserved.
 * 
 * Contributor(s): Wade Majors <wade@ezri.org>
 *								 Rene Gollent
 *								 Todd Lair
 *								 Joshua Jensen
 */

#ifndef _LUASCRIPT_H_
#define _LUASCRIPT_H_

extern "C"
{
#include "lua/include/lua.h"
#include "lua/include/luadebug.h"
#include "lua/include/lauxlib.h"
}

/**
	A lightweight Lua wrapper.

	\include ../TestScript/TestScript.cpp
**/
class Script
{
public:
	typedef lua_CFunction CFunction;

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	/**
		Representation of a Lua script object residing on the Lua stack.
	**/
	class Object
	{
	public:
		/**
			Copy constructor.
		**/
		Object(const Object& src) : m_parent(src.m_parent)
		{
			m_stackIndex = src.m_stackIndex;
		}

		/**
			Assignment operator.
		**/
		const Object& operator=(const Object& src)
		{
			m_stackIndex = src.m_stackIndex;
			return *this;
		}

		Script& GetParent() const			{	return m_parent;	}
		lua_State* GetState() const			{	return m_parent.m_state;	}

		int GetType() const					{	return lua_type(GetState(), m_stackIndex);	}

		bool IsNil() const					{	return m_parent.IsNil(m_stackIndex);	}
		bool IsTable() const				{	return m_parent.IsTable(m_stackIndex);	}
		bool IsUserData() const				{	return m_parent.IsUserData(m_stackIndex);	}
		bool IsCFunction() const			{	return lua_iscfunction(GetState(), m_stackIndex) != 0;	}
		bool IsNumber() const				{	return lua_isnumber(GetState(), m_stackIndex) != 0;	}
		bool IsString() const				{	return m_parent.IsString(m_stackIndex);	}
		bool IsFunction() const				{	return m_parent.IsFunction(m_stackIndex);	}
		bool IsNull() const					{	return m_parent.IsNull(m_stackIndex);	}

		int GetStackIndex() const			{	return m_stackIndex;	}

		int GetInteger() const				{	return (int)lua_tonumber(GetState(), m_stackIndex);	}
		float GetNumber() const				{	return (float)lua_tonumber(GetState(), m_stackIndex);	}
		const char* GetString() const		{	return lua_tostring(GetState(), m_stackIndex);	}
		int StrLen() const					{	return lua_strlen(GetState(), m_stackIndex);	}
		CFunction GetCFunction() const		{	return lua_tocfunction(GetState(), m_stackIndex);	}
		void* GetUserData() const			{	return lua_touserdata(GetState(), m_stackIndex);	}
		const void* GetPointer() const		{	return lua_topointer(GetState(), m_stackIndex);	}

		/**
			Creates a table called [name] within the current Object.

			@param name The name of the table to create.
			@return Returns the object representing the newly created table.
		**/
		Object CreateTable(const char* name)
		{
			int val;
			val = m_parent.GetTop();
			lua_newtable(GetState());							// T
			val = m_parent.GetTop();
			lua_pushstring(GetState(), name);					// T name
			val = m_parent.GetTop();
			lua_pushvalue(GetState(), lua_gettop(GetState()) - 1);	// T name T
			val = m_parent.GetTop();
			lua_settable(GetState(), m_stackIndex);
			val = m_parent.GetTop();

			return Object(m_parent, m_parent.GetTop());
		}
		
		/**
			Creates (or reassigns) the object called [name] to [value].

			@param name The name of the object to assign the value to.
			@param value The value to assign to [name].
		**/
		void SetNumber(const char* name, double value)
		{
			lua_pushstring(GetState(), name);
			lua_pushnumber(GetState(), value);
			lua_settable(GetState(), m_stackIndex);
		}
		
		/**
			Creates (or reassigns) the object called [name] to [value].

			@param name The name of the object to assign the value to.
			@param value The value to assign to [name].
		**/
		void SetString(const char* name, const char* value)
		{
			lua_pushstring(GetState(), name);
			lua_pushstring(GetState(), value);
			lua_settable(GetState(), m_stackIndex);
		}
		
		/**
			Creates (or reassigns) the object called [name] to [value].

			@param name The name of the object to assign the value to.
			@param value The value to assign to [name].
		**/
		void SetUserData(const char* name, void* data)
		{
			lua_pushstring(GetState(), name);
			lua_pushuserdata(GetState(), data);
			lua_settable(GetState(), m_stackIndex);
		}
		
//		int CallFunction();
		int Tag()								{	return lua_tag(GetState(), m_stackIndex);	}

		/**
			Assuming the current object is a table, retrieves the table entry
			called [name].

			@param name The name of the entry from the current table to
				retrieve.
			@return Returns an Object representing the retrieved entry.
		**/
		Object GetByName(const char* name)
		{
			lua_pushstring(GetState(), name);
			lua_rawget(GetState(), m_stackIndex);
			return Object(m_parent, m_parent.GetTop());
		}

		/**
			Assuming the current object is a table, retrieves the table entry
			at [index].

			@param index The numeric name of a table entry.
			@return Returns an Object representing the retrieved entry.
		**/
		Object GetByIndex(int index)
		{
			lua_rawgeti(GetState(), m_stackIndex, index);
			return Object(m_parent, m_parent.GetTop());
		}

	protected:
		friend class Script;

		Object(Script& parent, int index) :
				m_parent(parent), m_stackIndex(index) { }
		Script& m_parent;		//!< The parent script of this object.
		int m_stackIndex;		//!< The stack index representing this object.
	};

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	/**
	**/
	class AutoBlock
	{
	public:
		AutoBlock(Script& script) :
			m_script(script)
		{
			m_stackTop = m_script.GetTop();
		}

		AutoBlock(Object& object) :
			m_script(object.GetParent())
		{
			m_stackTop = m_script.GetTop();
		}

		~AutoBlock()
		{
			m_script.SetTop(m_stackTop);
		}

	private:
		AutoBlock(const AutoBlock& src);					// Not implemented
		const AutoBlock& operator=(const AutoBlock& src); // Not implemented

		Script& m_script;
		int m_stackTop;
	};


	///////////////////////////////////////////////////////////////////////////
	enum { NOREF = LUA_NOREF };
	enum { REFNIL = LUA_REFNIL };
	enum { ANYTAG = LUA_ANYTAG };

	Script(bool initStandardLibrary = true);
	Script(lua_State* state);
	~Script();

	// Basic stack manipulation.
	int GetTop()								{	return lua_gettop(m_state);	}
	void SetTop(int index)						{	lua_settop(m_state, index);	}
	void PushValue(int index)					{	lua_pushvalue(m_state, index);	}
	void Remove(int index)						{	lua_remove(m_state, index);	}
	void Insert(int index)						{	lua_insert(m_state, index);	}
	int StackSpace()							{	return lua_stackspace(m_state);	}

	Object GetObject(int index)					{	return Object(*this, index);	}
	void PushObject(Object object)				{	lua_pushvalue(m_state, object.GetStackIndex());	}

	// access functions (stack -> C)
	int Equal(int index1, int index2)			{	return lua_equal(m_state, index1, index2);	}
	int LessThan(int index1, int index2)		{	return lua_lessthan(m_state, index1, index2);	}

	// push functions (C -> stack)
	void PushBool(bool value)					{	if (value)	lua_pushnumber(m_state, 1);	else	lua_pushnil(m_state);	}
	void PushNil()								{	lua_pushnil(m_state);	}
	void PushNumber(double n)					{	lua_pushnumber(m_state, n);	}
	void PushLString(const char *s, size_t len)	{	lua_pushlstring(m_state, s, len);	}
	void PushString(const char *s)				{	lua_pushstring(m_state, s);	}
	void PushCClosure(lua_CFunction fn, int n)	{	lua_pushcclosure(m_state, fn, n);	}
	void PushUserTag(void *u, int tag)			{	lua_pushusertag(m_state, u, tag);	}

	// get functions (Lua -> stack)
	Object GetGlobal(const char *name)			{	lua_getglobal(m_state, name);	return Object(*this, GetTop());	}
	void GetTable(int index)					{	lua_gettable(m_state, index);	}
	void RawGet(int index)						{	lua_rawget(m_state, index);	}
	void RawGetI(int index, int n)				{	lua_rawgeti(m_state, index, n);	}
	Object GetGlobals()							{	lua_getglobals(m_state);	return Object(*this, GetTop());	}
	void GetTagMethod(int tag, const char *event)	{	lua_gettagmethod(m_state, tag, event);	}

	int GetRef(int ref)							{	return lua_getref(m_state, ref);	}

	Object NewTable()							{	lua_newtable(m_state);	return Object(*this, GetTop());	}

	// set functions(stack -> Lua)
	void SetGlobal(const char *name)			{	lua_setglobal(m_state, name);	}
	void SetTable(int index)					{	lua_settable(m_state, index);	}
	void RawSet(int index)						{	lua_rawset(m_state, index);	}
	void RawSetI(int index, int n)				{	lua_rawseti(m_state, index, n);	}
	void SetGlobals()							{	lua_setglobals(m_state);	}
	void SetTagMethod(int tag, const char *event)	{	lua_settagmethod(m_state, tag, event);	}
	int Ref(int lock_)							{	return lua_ref(m_state, lock_);	}


	// "do" functions(run Lua code)
	int Call(int nargs, int nresults)			{	return lua_call(m_state, nargs, nresults);	}
	void RawCall(int nargs, int nresults)		{	lua_rawcall(m_state, nargs, nresults);	}
	int DoFile(const char *filename)			{	return lua_dofile(m_state, filename);	}
	int DoString(const char *str)				{	return lua_dostring(m_state, str);	}
	int DoBuffer(const char *buff, size_t size, const char *name)	{	return lua_dobuffer(m_state, buff, size, name);	}


	// miscellaneous functions
	int NewTag()								{	return lua_newtag(m_state);	}
	int CopyTagMethods(int tagto, int tagfrom)	{	return lua_copytagmethods(m_state, tagto, tagfrom);	}
	void SetTag(int tag)						{	lua_settag(m_state, tag);	}

	void Error(const char *s)					{	lua_error(m_state, s);	}

	void Unref(int ref)							{	lua_unref(m_state, ref);	}

	int Next(int index)							{	return lua_next(m_state, index);	}
	int GetN(int index)							{	return lua_getn(m_state, index);	}

	void Concat(int n)							{	lua_concat(m_state, n);	}

	// Helper function
	void Pop()									{	lua_pop(m_state, 1);	}
	void Pop(int amount)						{	lua_pop(m_state, amount);	}

	void Register(const char* funcName, lua_CFunction function)	{	lua_register(m_state, funcName, function);	}
	void PushUserData(void* u)					{	lua_pushuserdata(m_state, u);	}
	void PushCFunction(lua_CFunction f)			{	lua_pushcclosure(m_state, f, 0);	}
	int CloneTag(int t)							{	return lua_copytagmethods(m_state, lua_newtag(m_state), t);	}

	bool IsFunction(int index)					{	return lua_isfunction(m_state, index);	}
	bool IsString(int index)					{	return lua_isstring(m_state, index) != 0;	}
	bool IsTable(int index)						{	return lua_istable(m_state, index);	}
	bool IsUserData(int index)					{	return lua_isuserdata(m_state, index);	}
	bool IsNil(int index)						{	return lua_isnil(m_state, index);	}
	bool IsNull(int index)						{	return lua_isnull(m_state, index);	}

	
	
	
	
	int ConfigGetInteger(const char* section, const char* entry, int defaultValue = 0);
	float ConfigGetReal(const char* section, const char* entry, double defaultValue = 0.0);
	const char* ConfigGetString(const char* section, const char* entry, const char* defaultValue = "");
	void ConfigSetInteger(const char* section, const char* entry, int value);
	void ConfigSetReal(const char* section, const char* entry, double value);
	void ConfigSetString(const char* section, const char* entry, const char* value);

	void SaveText(const char* filename);

	operator lua_State*()						{	return m_state;	}
	lua_State* GetState() const					{	return m_state;	}

public:
	friend class Object;

	lua_State* m_state;
	bool m_ownState;
};


/**
**/
inline
Script::Script(lua_State* state)
{
	m_state = state;
	m_ownState = false;
}


/**
**/
inline
Script::~Script()
{
	// Only close the Lua state if we own it.
	if (m_ownState)
		lua_close(m_state);
}

#endif
