#ifndef _____CPYTHON_____
#define _____CPYTHON_____

#ifdef _DEBUG
//Hack to work around not having the python debug lib
  #undef _DEBUG
  #include <python.h>
  #define _DEBUG
#else
  #include <python.h>
#endif

#include <extcomp.he>

typedef struct omnisvariant
   {
   ffttype	ffttype;
   union 
     {
		void* vNone; 
		char* vChar;
		bool vBool;
		//date 
		float vFloat;
		long vLong;
		char* vBinary;
		EXTqlist* vList;
		EXTqlist* vRow;
		void* vObject;
     } variant;
   } OMNISVARIANT;

class CPython
{
public:
	CPython();
	CPython(char* pPythonPath);
	~CPython();

	char* GetPythonPath();
	short CallFunction(char* pModuleName, char* pFunctionName, EXTqlist* pArgList=NULL, OMNISVARIANT* pReturn=NULL, char** pError=NULL);
private:
	void init();
	void pythonReturnValuesToOmnisVariant(PyObject *pValues, OMNISVARIANT *pReturn, char** pError);
	PyObject* omnisListToPyObject(EXTqlist* pList);
	PyObject* omnisValToPythonVal(EXTfldval* pFldVal);
	unsigned char* pythonValToString(PyObject* pValue);
	void pythonValToEXTfldval(PyObject *pValue, EXTfldval);
	void pyTupleToOmnisList(PyObject *pValue, OMNISVARIANT *pReturn);
	void pyListToOmnisList(PyObject *pValue, OMNISVARIANT *pReturn);
	char* getPyError();
	char* getTypeMatchError(const char* pPythonType, ffttype pOmnisType);
	const char* getfftypeString(ffttype pType);

	void setPythonPath(char* pPythonPath);
};
#endif /* ______CPYTHON_____ */
