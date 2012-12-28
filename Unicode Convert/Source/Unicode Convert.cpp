///////////////////////////////////////////////////////////////////////////////
//
//  File     : Unicode Convert.cpp
//  Author   : obaby
//  Date     : 28/12/2012
//  Homepage : http://www.h4ck.ws
//  
//  License  : Copyright ?2012 火星信息安全研究院
//
//  This software is provided 'as-is', without any express or
//  implied warranty. In no event will the authors be held liable
//  for any damages arising from the use of this software.
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
#include "Unicode Convert.h"


// Global Variables:
int gSdkVersion;
char gszVersion[]      = "1.0.0.1";
// Plugin name listed in (Edit | Plugins)
char gszWantedName[]   = "English Unicode String Maker";
// plug-in hotkey
char gszWantedHotKey[] = "Shift+U";

char *gszPluginHelp;
char *gszPluginComment;



bool GetKernelVersion(char *szBuf, int bufSize)
{
	int major, minor, len;
	get_kernel_version(szBuf, bufSize);
	if ( qsscanf(szBuf, "%d.%n%d", &major, &len, &minor) != 2 )
		return false;
	if ( isdigit(szBuf[len + 1]) )
		gSdkVersion = 100*major + minor;
	else
		gSdkVersion = 10 * (10*major + minor);
	return true;
}

void PrintDefaultStrtype()
{
	switch(inf.strtype)
	{
	case(ASCSTR_PASCAL):
		msg("Current idb string type is  Pascal-style ASCII string (one byte length prefix).\n");
		break;
	case(ASCSTR_LEN2):
		msg("Current idb string type is Pascal-style, two-byte length prefix.\n");
		break;
	case(ASCSTR_UNICODE)://ASCSTR_UTF16
		msg("Current idb string type is Unicode string (UTF-16).\n");
		break;
	case(ASCSTR_LEN4):
		msg("Current idb string type is Pascal-style, four-byte length prefix.\n");
		break;
	case(ASCSTR_ULEN2):
		msg("Current idb string type is Pascal-style Unicode, two-byte length prefix\n");
		break;
	case(ASCSTR_ULEN4):
		msg("Current idb string type is Pascal-style Unicode, four-byte length prefix\n");
		break;
	case(ASCSTR_UTF32)://ASCSTR_LAST
		msg("Current idb string type is four-byte Unicode codepoints\n");
		break;
	default:
		msg("Warnning: can`t get the default string type.\n");
		break;
	}
}
//-----------------------------------------------------------------------------
// Function: init
//
// init is a plugin_t function. It is executed when the plugin is
// initially loaded by IDA.
// Three return codes are possible:
//    PLUGIN_SKIP - Plugin is unloaded and not made available
//    PLUGIN_KEEP - Plugin is kept in memory
//    PLUGIN_OK   - Plugin will be loaded upon 1st use
//
// Check are added here to ensure the plug-in is compatible with
// the current disassembly.
//-----------------------------------------------------------------------------
int initPlugin(void)
{
	char szBuffer[MAXSTR];
	char sdkVersion[32];
	int nRetCode = PLUGIN_OK;
	HINSTANCE hInstance = ::GetModuleHandle(NULL);

	// Initialize global strings
	LoadString(hInstance, IDS_PLUGIN_HELP, szBuffer, sizeof(szBuffer));
	gszPluginHelp = qstrdup(szBuffer);
	LoadString(hInstance, IDS_PLUGIN_COMMENT, szBuffer, sizeof(szBuffer));
	gszPluginComment = qstrdup(szBuffer);
	if ( !GetKernelVersion(sdkVersion, sizeof(sdkVersion)) )
	{
		msg("%s: could not determine IDA version\n", gszWantedName);
		nRetCode = PLUGIN_SKIP;
	}
	else if ( gSdkVersion < 600 )
	{
		warning("Sorry, the %s plugin required IDA v%s or higher\n", gszWantedName, sdkVersion);
		nRetCode = PLUGIN_SKIP;
	}
	else if ( ph.id != PLFM_386 || ( !inf.is_32bit() && !inf.is_64bit() ) || inf.like_binary() )
	{
		msg("%s: could not load plugin\n", gszWantedName);
		nRetCode = PLUGIN_SKIP;
	}
	else
	{
		msg("\n--------------------------------------------------------------------------------------\n" 
			"%s (v%s) plugin has been loaded\n"
			"  The hotkeys to invoke the plugin is %s.\n"
			"  Please check the Edit/Plugins menu for more informaton.\n",
			gszWantedName, gszVersion, gszWantedHotKey);
		PrintDefaultStrtype();
		msg("--------------------------------------------------------------------------------------\n");
	}
	return nRetCode;
}

//-----------------------------------------------------------------------------
// Function: term
//
// term is a plugin_t function. It is executed when the plugin is
// unloading. Typically cleanup code is executed here.
//-----------------------------------------------------------------------------
void termPlugin(void)
{
}

//-----------------------------------------------------------------------------
// Function: run
//
// run is a plugin_t function. It is executed when the plugin is run.
//
// The argument 'arg' can be passed by adding an entry in
// plugins.cfg or passed manually via IDC:
//
//   success RunPlugin(string name, long arg);
//-----------------------------------------------------------------------------
void runPlugin(int arg)
{
	ea_t ea = get_screen_ea();
	int WcharCount = 0;
	//inf.strtype = ASCSTR_UNICODE;
	
	msg("\nCurrent addr is 0x%x and the byte are :\n",ea);
	while (1)
	{
		msg("%.2x ", get_byte(ea));

		WcharCount ++;		
		if (get_byte(ea) ==0 && get_byte(ea+1)  == 0) //Detect the end of the unicode string
		{	
			msg("\nWhole length is :0x%X.",WcharCount+2);
			break;
		}
		ea ++;
	}
	
	if (!make_ascii_string(get_screen_ea(),0/*WcharCount + 2*/,ASCSTR_UNICODE)) //+2 to fix up the last two 0.
	{
		msg("\nError:Convert to string failed.\n");
	}else{
		msg("\Convert to string success.\n");
	}
	
}

///////////////////////////////////////////////////////////////////////////////
//
//                         PLUGIN DESCRIPTION BLOCK
//
///////////////////////////////////////////////////////////////////////////////
plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  0,              // plugin flags
  initPlugin,           // initialize
  termPlugin,           // terminate. this pointer may be NULL.
  runPlugin,            // invoke plugin
  gszPluginComment,     // comment about the plugin
  gszPluginHelp,        // multiline help about the plugin
  gszWantedName,        // the preferred short name of the plugin
  gszWantedHotKey       // the preferred hotkey to run the plugin
};

