#include "oPython.h"
#include "cPython.h"

char* g_PythonPath = NULL;



char* getStringFromOmnis(EXTfldval str)
{
	qlong iLen = str.getBinLen();
	qlong len; 
	char* string = NULL;
	if (iLen > 0)
	{
		if ((string = new char[iLen+1]) != NULL)
		{
			str.getChar(iLen+1, (qchar*) string, len);
			string[len] = 0x0;

		}
	}
	return string;
}
//Wrapper to throw exceptions when param is not found
EXTParamInfo* findParamNum(EXTCompInfo* pEci, short pNum, bool pThrow = false)
{
	EXTParamInfo* pi = ECOfindParamNum(pEci, pNum);
	if (pi == NULL && pThrow == true)
	{
		char error[256];
		sprintf(error, "Parameter %d Not Found.", pNum);
		throw error;
	}
	return pi;
}


OMNISVARIANT getOmnisVariant(EXTfldval *pFldVal)
{
	OMNISVARIANT ov;
	pFldVal->getType(ov.ffttype);
	return ov;
}

void setOmnisValue(EXTCompInfo* eci, OMNISVARIANT pOV, short pParamNum)
{
	EXTfldval *rtnVal;
	EXTParamInfo *piParam = NULL;
			
	piParam = findParamNum(eci, pParamNum);
	if (piParam != NULL)
	{
		rtnVal = new EXTfldval((qfldval)piParam->mData);

		switch(pOV.ffttype)
		{
			case fftBinary:
				break;
			case fftBoolean:
				break;
			case fftCharacter:
				rtnVal->setChar((qchar*)pOV.variant.vChar, strlen(pOV.variant.vChar));
				break;
			case fftNumber:
				//ov.variant.vFloat
				break;
			case fftList:
				//ov.variant.vList = pFldVal->getList(false);
				rtnVal->setList(pOV.variant.vList, qtrue);
				break;
			case fftInteger: 
				rtnVal->setLong(pOV.variant.vLong);
				break;
			case fftNone:
				//ov.variant.vNone;
				break;
			case fftObject:
				//ov.variant.vObject;
				break;
			case fftRow:
				//ov.variant.vRow = pFldVal->getList(false);
				rtnVal->setList(pOV.variant.vRow, qtrue);
				break;
		}
		ECOsetParameterChanged(eci, pParamNum);
	}
}

qlong AddPythonPath(HWND hwnd, LPARAM Msg, WPARAM wParam, LPARAM lParam, EXTCompInfo* eci)
{
	EXTParamInfo *piPath;
	
	piPath = findParamNum(eci, 1, false);
	if (piPath != NULL)
	{
		EXTfldval valPath((qfldval)piPath->mData);

		if (g_PythonPath != NULL)
		{
			delete [] g_PythonPath;
		}

		g_PythonPath = getStringFromOmnis(valPath);
	}
	return qtrue;
}

qlong GetPythonPath(HWND hwnd, LPARAM Msg, WPARAM wParam, LPARAM lParam, EXTCompInfo* eci)
{
    CPython py(g_PythonPath);

	char* szRetVal = py.GetPythonPath();
    if (szRetVal != NULL)
	{
		EXTfldval rtnVal;
		rtnVal.setChar((qchar*)szRetVal, strlen(szRetVal));
		ECOaddParam(eci, &rtnVal);
		//don't delete the pointer to path. Python will clean up when finalize is called. 
		//delete[] szRetVal;
	}
	return qtrue;
}

qlong CallFunction(HWND hwnd, LPARAM Msg, WPARAM wParam, LPARAM lParam, EXTCompInfo* eci )
{
	EXTfldval* valArgList = NULL, *valReturn = NULL;
	EXTParamInfo *piModName, *piFunctName, *piArgList, *piReturn;
	EXTqlist* liArgs = NULL;
	char* szModName = NULL, *szFunctName = NULL, *szRetVal=NULL;
	CPython py(g_PythonPath);
	OMNISVARIANT ovRetVal;

	long num = 0;

	try
	{
		piModName = findParamNum(eci, 1, true);
		piFunctName = findParamNum(eci, 2, true);
		piArgList = findParamNum(eci, 3, false);
		piReturn = findParamNum(eci, 4, false);

		EXTfldval valModName((qfldval)piModName->mData);
		EXTfldval valFunctName((qfldval)piFunctName->mData);	
		
		szModName = getStringFromOmnis(valModName);
		szFunctName = getStringFromOmnis(valFunctName);
		
		//These are optional. If they weren't sent, they'll be NULL
		//Can't create non-pointer because the extfldval can only be set via the constructs
		if (piArgList != NULL)
		{
			valArgList = new EXTfldval((qfldval)piArgList->mData);
		}
		if (piReturn != NULL)
		{
			valReturn = new EXTfldval((qfldval)piReturn->mData);
			ovRetVal = getOmnisVariant(valReturn);			
		}
		
		if (valArgList != NULL)
		{
			liArgs = valArgList->getList(qtrue);
		}

		
		if (py.CallFunction(szModName, szFunctName, liArgs, &ovRetVal, &szRetVal) == 1)
		{
			setOmnisValue(eci, ovRetVal, 4);
			if (szRetVal != NULL)
			{
				EXTfldval rtnVal;
				rtnVal.setChar((qchar*)szRetVal, strlen(szRetVal));
				ECOaddParam(eci, &rtnVal);
				delete[] szRetVal;
			}
		}
	
	}catch (char* e)
	{
		//ShowMessage();
	}

	
	/*if (szModName != NULL)
		delete[] szModName;
	if (szFunctName != NULL)
		delete[] szFunctName;
	if (liArgs != NULL)
	{
		//*liArgs = qnil;
		//delete liArgs;
	}*/
    
	return qtrue;
}


qlong onConnect( HWND hwnd, LPARAM Msg, WPARAM wParam, LPARAM lParam, EXTCompInfo* eci )
{
	// Return external flags
	return EXT_FLAG_LOADED|EXT_FLAG_ALWAYS_USABLE|EXT_FLAG_REMAINLOADED|EXT_FLAG_NVOBJECTS; 
}

qlong onDisconnect( HWND hwnd, LPARAM Msg, WPARAM wParam, LPARAM lParam, EXTCompInfo* eci )
{
	if (g_PythonPath != NULL)
	{
		delete [] g_PythonPath;
	}
	return qtrue;
}

//******************************************************************************************
//
//				onGetObjects
//
//******************************************************************************************


ECOparam CallFunctionParams[] =
{
	IDS_MODULENAME,		fftCharacter,	0, 0, 
	IDS_MODULEFUNCTION,	fftCharacter,	0, 0,
};

ECOparam AddPythonPathParams[] =
{
	IDS_PATH,	fftCharacter, 0, 0
};

ECOmethodEvent StaticMethods[] = 
{
	CALLFUNCTION, IDS_CALLFUNCTION, fftCharacter, sizeof(CallFunctionParams)/sizeof(ECOparam), CallFunctionParams, 0, 0,
	ADDPYTHONPATH, IDS_ADDPYTHONPATH, fftNone, sizeof(AddPythonPathParams)/sizeof(ECOparam), AddPythonPathParams, 0, 0,
	GETDEFAULTPYTHONPATH, IDS_GETDEFAULTPYTHONPATH, fftCharacter, 0, 0, 0, 0,
};



qlong onMethodCall(HWND hwnd, LPARAM Msg, WPARAM wParam, LPARAM lParam, EXTCompInfo* eci)
{
	void* object = (void*)ECOfindNVObject(eci->mOmnisInstance, lParam); 

	switch( ECOgetId(eci))
	{
//		case -1:					return qtrue;
		case CALLFUNCTION:			return CallFunction(hwnd, Msg, wParam, lParam, eci);
		case ADDPYTHONPATH:			return AddPythonPath(hwnd, Msg, wParam, lParam, eci);
		case GETDEFAULTPYTHONPATH:	return GetPythonPath(hwnd, Msg, wParam, lParam, eci);
		default:					break;
	}
	return qfalse;
}

//******************************************************************************************
//
//	Component library entry point (name as declared in resource 31000 )
//						
//******************************************************************************************
extern "C" qlong OMNISWNDPROC Python(HWND hwnd, LPARAM Msg, WPARAM wParam, LPARAM lParam, EXTCompInfo* eci)
{
	ECOsetupCallbacks( hwnd, eci );
	switch (Msg)
	{
		case ECM_CONNECT:			return onConnect(hwnd, Msg, wParam, lParam, eci);
		case ECM_DISCONNECT:		return onDisconnect(hwnd, Msg, wParam, lParam, eci);
		case ECM_GETCOMPLIBINFO:	return ECOreturnCompInfo(gInstLib, eci, IDS_APPNAME, 0);
		case ECM_GETVERSION:		return ECOreturnVersion(1,0);
		case ECM_GETSTATICOBJECT:	return ECOreturnMethods(gInstLib, eci, StaticMethods, sizeof(StaticMethods)/sizeof(ECOmethodEvent));
		case ECM_METHODCALL:		return onMethodCall(hwnd, Msg, wParam, lParam, eci);
	 }
	 return WNDdefWindowProc((HWND)hwnd, Msg, wParam, lParam, eci);
}
