
#ifndef _TESTSCRIPT_H_
#define _TESTSCRIPT_H_

#include <stdio.h>
#include <stddef.h>

class lua_State;

class TestScript
{
public:
	TestScript();
	static int Script_PrintNumber(lua_State*);
	static int Script_Add(lua_State*);
	void DoScriptIniWriteTest();
	void DoScriptIniReadTest();
	void DoScriptCallbackTest();
	void DoScriptSaveTest();
	void DoScriptArrayTest();
	void RunTestScripts();
};

#endif
