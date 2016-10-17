#include "cPython.h"

const char* PYERR_TYPE_MESSAGE = ("<unknown_exception_type>");
const char* PYERR_INFO_MESSAGE = ("<unknown_exception_info>");
const char* TYPE_CHARACTER = ("Character");
const char* TYPE_LIST = ("List");
const char* TYPE_ROW = ("Row");
const char* TYPE_DICTIONARY = ("Dictionary");
const char* TYPE_INTEGER = ("Integer");
const char* TYPE_NUMBER = ("Number");
const char* TYPE_TUPLE = ("Tuple");
const char* TYPE_BOOLEAN = ("Boolean");
const char* TYPE_UNKNOWN = ("Unknown");
const char* TYPE_NOT_SUPPORTED = ("Not Supported");


extern char* getStringFromOmnis(EXTfldval str);

CPython::CPython()
{
	init();	
}

CPython::CPython(char* pPythonPath)
{		
    init();
	char* path = Py_GetPath();
	if (pPythonPath != NULL)
	{
		setPythonPath(pPythonPath);
	}
}

void CPython::init()
{
	char* py_home;
	TCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
    for (int x = strlen(szPath); x > 0; x--)
	{
		if (szPath[x] == '\\')
		{
			szPath[x+1] = NULL;
			py_home = new char[strlen(szPath+1024)];
			if (py_home != NULL)
			{	
				strcat(szPath, "python");
			}
			break;
		}
	 }

	Py_SetPythonHome(szPath);
	Py_Initialize();
}

CPython::~CPython()
{
	Py_Finalize();
}


char* CPython::GetPythonPath()
{
	return Py_GetPath();
}

//Derived from http://brianray.chipy.org/pyincppchipy/
//Use python to set its path
void CPython::setPythonPath(char* pPythonPath)
{
	PyRun_SimpleString("import sys, os, string\n"); 
	PyObject* pyString = PyString_FromFormat("mypath = r\"%s\"\n", pPythonPath);
	PyRun_SimpleString(PyString_AsString(pyString));
	PyRun_SimpleString("mypath = string.replace(mypath, \"\\\\\", \"/\")\n");
	PyRun_SimpleString("mydir = os.path.dirname(mypath)\n");
	PyRun_SimpleString("sys.path.append(mydir)\n");
}

PyObject* CPython::omnisValToPythonVal(EXTfldval* pFldVal)
{
	char* str = NULL;
	char* str1 = NULL;

	PyObject* pValue = NULL;
	ffttype colType;
	pFldVal->getType(colType);

	qlong iLen, len;

	switch(colType)
	{
		case fftBinary:
			break;
		case fftBoolean:
			//pValue = PyInt_FromLong((int)pFldVal.getBool());
			break;
		case fftCharacter:
			//Something in the scope of this block goes out of scope and omnis tries to delete it causing a crash.
		    iLen = pFldVal->getBinLen();
		
			if (iLen > 0)
			{
				if ((str = new char[iLen+1]) != NULL)
				{
					pFldVal->getChar(iLen+1, (qchar*) str, len);
					str[len] = 0x0;

				}
			}
			if (str == NULL)
			{
			    str = new char[2];
				str[0] = '1';
				str[1] = 0x0;
			}
			if (str != NULL)
			{
				pValue = PyString_FromString(str);
			}
			break;
		case fftNumber:
			//TODO:
			//pValue =  PyFloat_FromDouble();
			pValue = PyLong_FromLong((long)pFldVal->getLong());
			//pValue = PyLong_FromLong(5);
			break;
		case fftList:
			//TODO:
			break;
		case fftInteger:
			pValue = PyLong_FromLong((long)pFldVal->getLong());
			//pValue = PyLong_FromLong(10); 
			break;
		case fftNone:
			break;
		case fftObject:
			break;
		case fftRow:
			//TODO:
			//ov.variant.vRow = pFldVal->getList(false);
			break;	
	}
	return pValue;
}

PyObject* CPython::omnisListToPyObject(EXTqlist* pList)
{
	PyObject* pArgs = NULL, *pValue = NULL;

	if (pList != NULL)
	{
		qshort cols = pList->colCnt();
		if (cols > 0)
		{
			if (pArgs = PyTuple_New(cols))
			{
				for (int i = 1; i <= cols; i++)
				{
					EXTfldval rowVal;
					pList->getColVal(1, i, rowVal);
					pValue = omnisValToPythonVal(&rowVal);
					if (!pValue)
					{
						
						Py_DECREF(pArgs);
						throw 1;
						return NULL;
					}
					
					PyTuple_SetItem(pArgs, i-1, pValue); 
					
					
				}
			}
		}
	}
	return pArgs;
}

unsigned char* CPython::pythonValToString(PyObject *pValue)
{
	unsigned char* val = NULL;
	qbool have_lVal = qfalse;
	long lVal;

	if (PyString_Check(pValue))
	{
		if (val = new unsigned char[PyString_Size(pValue)])
		{
			strcpy((char*)val, PyString_AsString(pValue));
		}
	}
	else if(PyInt_Check(pValue))
	{
		lVal = PyInt_AsLong(pValue);
		have_lVal = qtrue;
	}
	else if(PyLong_Check(pValue))
	{
		lVal = PyLong_AsLong(pValue);
		have_lVal = qtrue;
	}
	/*else if(PyTuple_Check(pValue))
	{
		
	}*/
	else
	{
		throw TYPE_DICTIONARY;
	}

	if (have_lVal == qtrue)
	{
		char buffer[100];
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer, "%d", lVal);
		if (val = new unsigned char[strlen(buffer)])
		{
			strcpy((char*)val, buffer);
		}
	}

	return val;
}
void CPython::pythonValToEXTfldval(PyObject *pValue, EXTfldval fval)
{	
	fval.setLong(PyInt_AsLong(pValue));
	int ii = fval.getLong();
}

void CPython::pyListToOmnisList(PyObject *pValue, OMNISVARIANT *pReturn)
{
	int len = PyList_Size(pValue);
	int row;
	
	if ((pReturn->variant.vList = new EXTqlist(listVlen)) != NULL)
	{
		pReturn->variant.vList->addCol(fftCharacter, dpFcharacter, 10000);
		
		for (int x = 0; x < len; x++)
		{
			EXTfldval fldVal;
			unsigned char* pyString = NULL;
			if (pyString = pythonValToString(PyList_GetItem(pValue, x)))
			{
				fldVal.setChar(pyString, strlen((const char*)pyString));
				row = pReturn->variant.vList->insertRow();
				pReturn->variant.vList->putColVal(row, 1, fldVal);
			}
		}
	}
}

void CPython::pyTupleToOmnisList(PyObject *pValue, OMNISVARIANT *pReturn)
{
	int len = PyTuple_Size(pValue);
	int row;
	
	
	if ((pReturn->variant.vList = new EXTqlist(listVlen)) != NULL)
	{
		for (int x = 0; x < len; x++)
		{
			pReturn->variant.vList->addCol(fftCharacter, dpFcharacter, 10000);
			
		}
		row = pReturn->variant.vList->insertRow();
		for (x = 0; x < len; x++)
		{
			EXTfldval fldVal;
			unsigned char* pyString = NULL;
			if (pyString = pythonValToString(PyTuple_GetItem(pValue, x)))
			{
				fldVal.setChar(pyString, strlen((const char*)pyString));
				pReturn->variant.vList->putColVal(row, x+1, fldVal);
			}
		}
	}
}

void CPython::pythonReturnValuesToOmnisVariant(PyObject *pValue, OMNISVARIANT *pReturn, char** pError)
{
	//

	try
	{
		if (PyString_Check(pValue))
		{
			if(pReturn->ffttype == fftCharacter)
			{
				int len = PyString_Size(pValue);
				char* pystring = PyString_AsString(pValue);
				pReturn->variant.vChar = new char[strlen(pystring)];
				strcpy(pReturn->variant.vChar, pystring);
			}
			else
			{
				*pError = getTypeMatchError(TYPE_CHARACTER, pReturn->ffttype);
			}
		}
		else if(PyInt_Check(pValue))
		{
			if (pReturn->ffttype == fftInteger)
			{
				pReturn->variant.vLong = PyInt_AsLong(pValue);
			}
			else
			{
				*pError = getTypeMatchError(TYPE_INTEGER, pReturn->ffttype);
			}
		}
		else if(PyLong_Check(pValue))
		{
			if (pReturn->ffttype == fftInteger)
			{
				pReturn->variant.vLong = PyLong_AsLong(pValue);
			}
			else
			{
				*pError = getTypeMatchError(TYPE_INTEGER, pReturn->ffttype);
			}
		}
		else if(PyTuple_Check(pValue))
		{
			if (pReturn->ffttype == fftList || pReturn->ffttype == fftRow)
			{
				pyTupleToOmnisList(pValue, pReturn);
			}
			else
			{
				*pError = getTypeMatchError(TYPE_TUPLE, pReturn->ffttype);
			}
		}
		else if(PyDict_Check(pValue))
		{
			//*pError = getTypeMatchError(TYPE_DICTIONARY, pReturn->ffttype);
			//pReturn->ffttype = fftNone;
		}
		else if(PyList_Check(pValue))
		{
			if (pReturn->ffttype == fftList)
			{
				pyListToOmnisList(pValue, pReturn);
			}
			else
			{
				*pError = getTypeMatchError(TYPE_LIST, pReturn->ffttype);
			}
		}
		else
		{
			*pError = getTypeMatchError(TYPE_UNKNOWN, pReturn->ffttype);
			pReturn->ffttype = fftNone;
			//set some error condition - type mismatch or type not supported
		}
		//Dictionary API: http://docs.python.org/c-api/dict.html
		//List API: http://docs.python.org/c-api/list.html
	}
	catch (const char* e)
	{
		*pError = getTypeMatchError(e, pReturn->ffttype);
	}

}

short CPython::CallFunction(char* pModuleName, char* pFunctionName, EXTqlist* pArgList, OMNISVARIANT* pReturn, char** pError)
{
    PyObject *pName, *pModule, *pDict, *pFunc, *pValue, *pArgs = NULL;
	char* szRet = NULL;
	int have_error = 0;

	try
	{
		// Build the name object
		if (!(pName = PyString_FromString(pModuleName)))
		{
			throw have_error;
		}

		// Load the module object
		if(!(pModule = PyImport_Import(pName)))
		{
			throw have_error;
		}
		
		// pDict is a borrowed reference 
		if (!(pDict = PyModule_GetDict(pModule)))
		{
			throw have_error;
		}
		 
		// pFunc is also a borrowed reference 
		if (!(pFunc = PyDict_GetItemString(pDict, pFunctionName)))
		{
			throw have_error;
		}
		
		if (PyCallable_Check(pFunc)) 
		{
			// Prepare the argument list for the call
			if (pArgList != NULL)
			{
				pArgs = omnisListToPyObject(pArgList);
			}

			pValue = PyObject_CallObject(pFunc, pArgs);
			if (pArgs != NULL)
			{
				Py_DECREF(pArgs);
			}

			if (PyErr_Occurred())
			{
				throw have_error;
			}
			if (pValue != NULL)
			{
				pythonReturnValuesToOmnisVariant(pValue, pReturn, pError);
			}

		
			if (pValue != NULL)
			{
				Py_DECREF(pValue);
			}
			else //we didn't get a return value - set OMNISVARIANT to none
			{
				pReturn->ffttype = fftNone;
			}
		}
		else
		{
			throw have_error;
		}
	}catch(int e)
	{
		*pError = getPyError();
		pReturn->ffttype = fftNone;
	}

    // Clean up
    if (pModule != NULL) Py_DECREF(pModule);
    if (pName != NULL) Py_DECREF(pName);

	return 1;
}

char* CPython::getTypeMatchError(const char* pPythonType, ffttype pOmnisType)
{
	char* err = NULL;
	const char* omnisType = getfftypeString(pOmnisType);
	int len = strlen(pPythonType) + strlen(omnisType) + 70;
	if (err = new char[len+1])
	{
		sprintf(err, "Type Mismatch: Python returned %s; Omnis expected %s", pPythonType, omnisType);
	}
	return err;
}

//Returns Python error
//Code derived from http://rmi.net/~lutz/errata-supplements.html#G33
char* CPython::getPyError()
{
	PyObject *errobj, *errdata, *errtraceback, *pystring = NULL;
	//char save_error_type[1024], save_error_info[1024];
	char *error_type=NULL, *error_info=NULL, *error=NULL;	
	PyErr_Fetch(&errobj, &errdata, &errtraceback);
	int len_type, len_info; 

	if (errobj != NULL && (pystring = PyObject_Str(errobj)) != NULL && (PyString_Check(pystring)))
	{
		len_type = PyString_Size(pystring);
		if (error_type = new char[len_type+1])
		{
			error_type = PyString_AsString(pystring);
		}
		Py_XDECREF(pystring);
	}
	else
	{
		len_type = strlen(PYERR_TYPE_MESSAGE);
		if(error_type = new char[len_type+1])
		{
			strcpy(error_type, PYERR_TYPE_MESSAGE);
		}
	}

	if (errdata != NULL && (pystring = PyObject_Str(errdata)) != NULL && (PyString_Check(pystring)))
	{
		len_info = PyString_Size(pystring);
		if (error_info = new char[len_info+1])
		{
			error_info = PyString_AsString(pystring);
			Py_XDECREF(pystring);
		}
	}
	else
	{
		len_info = strlen(PYERR_INFO_MESSAGE);
		if(error_info = new char[len_info+1])
		{
			strcpy(error_info, PYERR_INFO_MESSAGE);
		}
	}
	
	if (error_type != NULL && error_info != NULL)
	{
		if (error = new char[len_type+len_info+3]) //add some space for formatting
		{
			sprintf(error, "%s: %s", error_type, error_info);
		}
	}

	if (errobj != NULL) Py_XDECREF(errobj);
	if (errdata != NULL) Py_XDECREF(errdata);         
	if (errtraceback != NULL) Py_XDECREF(errtraceback);
	
	return error;
}

const char* CPython::getfftypeString(ffttype pType)
{
	switch(pType)
	{
	case fftBinary:
			break;
		case fftBoolean:
			return TYPE_BOOLEAN;
			break;
		case fftCharacter:
			return TYPE_CHARACTER;
			break;
		case fftNumber:
			return TYPE_NUMBER;
			break;
		case fftList:
			return TYPE_LIST;
			break;
		case fftInteger:
			return TYPE_INTEGER;
			break;
		case fftNone:
			return TYPE_NOT_SUPPORTED;
			break;
		case fftObject:
			return TYPE_NOT_SUPPORTED;
			break;
		case fftRow:
			return TYPE_ROW;
			break;
		default:
			return TYPE_UNKNOWN;
			break;
	}
	return TYPE_UNKNOWN;
}