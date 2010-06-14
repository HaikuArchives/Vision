
#ifndef _TESTSCRIPT_H_
#define _TESTSCRIPT_H_

#include <stdio.h>
#include <stddef.h>

class lua_State;

class TestScript
{
	public:
													 TestScript (void);
		static int						 Script_PrintNumber (lua_State*);
		static int						 Script_Add (lua_State*);
		void									 DoScriptIniWriteTest (void);
		void									 DoScriptIniReadTest (void);
		void									 DoScriptCallbackTest (void);
		void									 DoScriptSaveTest (void);
		void									 DoScriptArrayTest (void);
		void									 RunTestScripts (void);
};

#endif
