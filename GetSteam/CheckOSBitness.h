/*
CheckOSBitness.h

Этот файл является частью GetSteam

Copyright (c) 2010-2014 Krinkels Inc (http://krinkels.org).
Все права защищены по закону.

Эта программа распространяется КАК ЕСТЬ, в надежде что она будет полезна, БЕЗ КАКИХ ЛИБО ГАРАНТИЙ И ОБЯЗАТЕЛЬСТВ. 
*/

#pragma region Includes
#include <stdio.h>
#include <windows.h>
#pragma endregion


#pragma region Is64BitOperatingSystem (IsWow64Process)

//
//   FUNCTION: DoesWin32MethodExist(PCWSTR, PCSTR)
//
//   PURPOSE: The function determins whether a method exists in the export 
//   table of a certain module.
//
//   PARAMETERS:
//   * pszModuleName - the name of the module.
//   * pszMethodName - the name of the method.
//
//   RETURN VALUE: The function returns TRUE if the method specified by 
//   methodName exists in the export table of the module specified by 
//   moduleName.
//
BOOL DoesWin32MethodExist(LPCSTR pszModuleName, PCSTR pszMethodName)
{
	HMODULE hModule = GetModuleHandle(pszModuleName);
	if (hModule == NULL)
	{
		return FALSE;
	}
	return (GetProcAddress(hModule, pszMethodName) != NULL);
}


//
//   FUNCTION: Is64BitOperatingSystem()
//
//   PURPOSE: The function determines whether the current operating system is 
//   a 64-bit operating system.
//
//   RETURN VALUE: The function returns TRUE if the operating system is 
//   64-bit; otherwise, it returns FALSE.
//
BOOL Is64BitOperatingSystem()
{
#if defined(_WIN64)
	return TRUE;   // 64-bit programs run only on Win64
#elif defined(_WIN32)
	// 32-bit programs run on both 32-bit and 64-bit Windows
	BOOL f64bitOS = FALSE;
	return ((DoesWin32MethodExist("kernel32.dll", "IsWow64Process") && 
		IsWow64Process(GetCurrentProcess(), &f64bitOS)) && f64bitOS);
#else
	return FALSE;  // 64-bit Windows does not support Win16
#endif
}

#pragma endregion