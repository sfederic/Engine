module;
#include <Windows.h>
#include <comdef.h>
#include "..\EditorMainWindow.h"
import <string>;
export module Debug;

export void DXTrace(HRESULT hr, const char* filename, const char* func, int line)
{
	_com_error err(hr);
	char errmsg[1024];
	snprintf(errmsg, sizeof(errmsg), "HR: %s\nFile: %s\nFunction: %s\nLine: %d", err.ErrorMessage(), filename, func, line);
	MessageBox(0, errmsg, "Error", 0);
}

export int Print(const char* format, va_list args)
{
	char buff[1024];
	int charCount = vsnprintf(buff, sizeof(buff), format, args);

	OutputDebugString(buff);
	gEditorSystem->Print(buff);

	return charCount;
}

export int DebugPrint(const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	int charsWritten = Print(format, argList);
	return charsWritten;
}
