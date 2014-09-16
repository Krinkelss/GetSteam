/*
GetSteam.h

Этот файл является частью GetSteam

Copyright (c) 2010-2014 Krinkels Inc (http://krinkels.org).
Все права защищены по закону.

Эта программа распространяется КАК ЕСТЬ, в надежде что она будет полезна, БЕЗ КАКИХ ЛИБО ГАРАНТИЙ И ОБЯЗАТЕЛЬСТВ. 
*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h> // Стандартная библиотека ввода-вывода
#include "CheckOSBitness.h"

#include "Krinkels_Registry.h"
//#include "CheckOS.h"

#define SIZEOF(X) (sizeof(X)/sizeof(X[0]))

#define VALVE_REG_PATH_1	"SOFTWARE\\Valve\\Steam"
#define NICKLEN 128

char 	nick[ NICKLEN ] = {0};
char	ProfilePath[ _MAX_PATH ] = {0};

char 	token[ NICKLEN ] = {0};
char	*FileBuffer;
char	*CurrentPos;
char    SteamPath[ _MAX_PATH ] = {0};
BOOL	EndOfBuffer = false;
char	Buffer[ 65535 ] = {0};

char	ErrCode[ 50 ] = {0};

//int ErrCode;

static const char *DllError[] =
{
	"Ок",
	"Отсутствует SteamAppData.vdf",
	"Невозможно получить размер файла",
	"Нет доступа к файлу",
	"Невозможно создать буфер",
	"Ошибка записи файла в буфер",
	"При поиске ничего не найдено",
	"Не могу найти путь к Steam",
	"Аккаунт отсутствует",
	"SteamAppData.vdf пустой",
	"Ник слишком длинный",
	"Файл слишком больщой",
	"Ошибка чтения файла"
};

typedef enum
{
	ErrCode_Ok = 0,			// Ок
	ErrCode_FileExst,		// Отсутствует SteamAppData.vdf
	ErrCode_GetSize,		// Невозможно получить размер файла
	ErrCode_OpenErr,		// Нет доступа к файлу
	ErrCode_ErrNewBuf,		// Невозможно создать буфер
	ErrCode_ErrWriteF,		// Ошибка записи файла в буфер
	ErrCode_ErrSearch,		// При поиске ничего не найдено
	ErrCode_NoPath,			// Не могу найти путь к Steam
	ErrCode_NoAcc,			// Аккаунт отсутствует
	ErrCode_EmptyFile,		// SteamAppData.vdf пустой
	ErrCode_ErrNick,		// Ник слишком длинный
	ErrCode_SizeFile,       // Файл слишком больщой
	ErrCode_ReadFile        // Ошибка чтения файла
} GetDLLError;

// Returns error string
char *Steam_StrError( GetDLLError errcode )
{
	return ( char * )DllError[ errcode ];
}

/*int stricmp(const char *s1, const char *s2) {return _stricmp(s1, s2);}
int _stricmp(const char *s1, const char *s2)
{
	return lstrcmpiA(s1, s2);
}*/

#include<windows.h>
bool FileExists(LPCTSTR fname)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind = ::FindFirstFile(fname, &wfd);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		// Если этого не сделать то произойдет утечка ресурсов
		::FindClose(hFind);
		return true;
	}
	return false;
}

/* Gets file size from 'name' (file must exists) */
/*******************************************
*** Получаем размер файла по "имени" *******
********************************************/
long GetFileSizeByName( HANDLE hFile )
{
	return GetFileSize( hFile, NULL );;
}

int LoadFile( char *FileName, char **BufferPtr )
{
	long		Size = 0;	
//	size_t 		result;
	HANDLE		hFile;
	DWORD		dw = 0;

	//Проверяем, существует ли файл
	if ( !FileExists( FileName ) )
	{
		strcpy( ErrCode, Steam_StrError( ErrCode_FileExst ) );
		return 0;
	}

	hFile = CreateFile(	FileName, 
		GENERIC_READ/*GENERIC_READ | GENERIC_WRITE*/, 
		0, 
		NULL,
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL );

	if ( hFile == INVALID_HANDLE_VALUE ) 
	{
		strcpy( ErrCode, Steam_StrError( ErrCode_OpenErr ) );
		return 0;
	}

	//Узнаем размер файла
	Size = GetFileSizeByName( hFile );
	if ( Size == -1 )
	{
		strcpy( ErrCode, Steam_StrError( ErrCode_GetSize ) );
		return 0;
	}

	// No need to load empty files
	if ( !Size )
	{
		strcpy( ErrCode, Steam_StrError( ErrCode_EmptyFile ) );
		return 0;
	}

	if( Size >= 65535 )
	{
		strcpy( ErrCode, Steam_StrError( ErrCode_SizeFile ) );
		return 0;
	}

	//result = ReadFile( FSetting, Buffer, sizeof( char ), &dw, NULL );

	HANDLE hFileMapping = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL );
	char *lpText = ( char * ) MapViewOfFile( hFileMapping, FILE_MAP_READ, 0, 0, 0 );

	if( !lpText )
	{
		strcpy( ErrCode, Steam_StrError( ErrCode_ErrWriteF ) );
		return 0;
	}

	*Buffer = '\0';
	lstrcpy( Buffer, lpText );

	*BufferPtr = Buffer;

	UnmapViewOfFile( lpText );
	CloseHandle( hFileMapping );
	CloseHandle( hFile );
	return Size;
}

// Получает очередной токен (помещенный между двумя двойными кавычками)
static BOOL GetToken( void )
{
    char    *ptr, *ptr2;

    memset( token, 0, sizeof( token ) );

    // ищем двойную кавычку
    ptr = strchr( CurrentPos, '"' );
	//Если не нашли
    if ( !ptr )
    {
        EndOfBuffer = TRUE;
        return FALSE;
    }
    else
    {
		//Ищем следующую кавычку
        ptr++;
        ptr2 = strchr( ptr, '"' );
		//Если не нашли
        if ( !ptr2 )
        {
            EndOfBuffer = TRUE;
            return FALSE;
        }
		if ( ( ptr2 - ptr ) >= NICKLEN )
		{
			strcpy( ErrCode, Steam_StrError( ErrCode_ErrNick ) );
			EndOfBuffer = TRUE;
			return FALSE;
		}
		//Копируем в token слово мнежду кавычками
        strncpy( token, ptr, ptr2 - ptr );
        CurrentPos = ptr2 + 1;
    }

    return TRUE;
}

// Поиск имени профиля в буфере
 BOOL FindNick2( void )
{
    BOOL    Found = false;

    //Пока EndOfBuffer = FALSE
    while ( !EndOfBuffer )
    {
        //Если GetToken ничего не нашел, т.е. вернул FALSE то прерываем цикл
        if ( !GetToken() )
            break;

        if ( !_stricmp( token, "User" ) && GetToken() )
        {
            Found = true;
            strcpy( nick, token );
            break;
        }
    }

    return Found;
}

// Получает путь к конфиг файлу из реестра и загружает его в память
static char *Steam_LoadConfigFile( void )
{
	char	ConfigPath[ _MAX_PATH ] = {0};
    char 	key_value[ _MAX_PATH ] = {0};
    char	*buffer = NULL;

	EndOfBuffer = FALSE;

	//Проверяем ветки реестра
	if ( RegValueExists( HKEY_LOCAL_MACHINE, VALVE_REG_PATH_1, "InstallPath" ) )
		lstrcpy( key_value, GetRegistryString( HKEY_LOCAL_MACHINE, VALVE_REG_PATH_1, "InstallPath" ) );
    else
	if ( RegValueExists( HKEY_CURRENT_USER, VALVE_REG_PATH_1, "InstallPath" ) )
		lstrcpy( key_value, GetRegistryString( HKEY_CURRENT_USER, VALVE_REG_PATH_1, "InstallPath" ) );
	else
	{
		lstrcpy( ErrCode, Steam_StrError( ErrCode_NoPath ) );
		EndOfBuffer = TRUE;
		return NULL;
	}

	//Если что то нашли то
	if ( *key_value )
		sprintf( ConfigPath, "%s\\config\\SteamAppData.vdf", key_value ); //Получаем путь к SteamAppData.vdf
	else
	{
        strcpy( ErrCode, Steam_StrError( ErrCode_NoPath ) ); //Иначе возвращаем ошибку
		return NULL;
	}

	// читаем этот файл в буфер.
	if ( !LoadFile( ConfigPath, &buffer ) )
	{
        strcpy( ErrCode, Steam_StrError( ErrCode_ReadFile ) );
        return NULL;
    }

	FileBuffer = buffer;
	CurrentPos = FileBuffer;

    return FileBuffer;
}

// Возвращает число аккаунтов
extern "C" __declspec( dllexport ) int Steam_GetNumAccounts( void )
{
	int		NumAccounts = 0;
	char    *buffer;
	BOOL	Found = TRUE;

	buffer = Steam_LoadConfigFile();
	if ( !buffer )
		return 0;

	while ( Found )
	{
		Found = FindNick2();
		if ( Found )
			NumAccounts++;
	}

	buffer = NULL;

	return NumAccounts;
}

// Возвращает имя # аккаунта
extern "C" __declspec( dllexport ) char *Steam_GetNickNameN( int iAccount )
{
    int		NumAccounts = 1;
    char    *buffer;
    BOOL    Found = true;

    //Получаем путь к файлу конфигурации
    buffer = Steam_LoadConfigFile();
    if ( !buffer )
        return NULL;

    while ( Found )
    {
        //Перебираем User в файле
        Found = FindNick2();
        //Если что то найденно
        if ( Found )
        {
            //Если NumAccounts равен номеру запрашиваемого ника то возвращаем его
            if ( iAccount == NumAccounts )
            {
                return nick;
            }
            //Иначе увеличиваем NumAccounts на 1
            NumAccounts++;
        }
    }

    //Если наш ник не найден то возвращаем NULL
    return NULL;
}

// Возвращает имя активного аккаунта
extern "C" __declspec( dllexport ) char *Steam_GetActiveNickName( void )
{
	char 	*buffer;

	buffer = Steam_LoadConfigFile();
	if ( !buffer )
		return NULL;

	//Пока EndOfBuffer = FALSE
	while ( !EndOfBuffer )
	{
		//Если GetToken ничего не нашел, т.е. вернул FALSE то прерываем цикл
		if ( !GetToken() )
			break;
		//Токен перебирает слова
		//Если stricmp == 0 т.е. token равен User
		if ( !_stricmp( token, "AutoLoginUser" ) )
		{
			//И если токен неравен FALSE
			if ( !GetToken() )
				break;
			//Копируем наш ник
			strcpy( nick, token );
		}
		if ( *nick )
		{
			//Возвращаем активный ник
			return nick;
		}

		*nick = '\0';
	}

	return NULL;
}

//Возвращаем путь к N профилю
extern "C" __declspec( dllexport ) char *Steam_GetProfilePath( int iAccount )
{
	char 	*key_value = NULL;
	char	*NickName = NULL;

	//Проверяем ветки реестра
	if ( RegValueExists( HKEY_LOCAL_MACHINE, VALVE_REG_PATH_1, "InstallPath" ) )
		key_value =  GetRegistryString( HKEY_LOCAL_MACHINE, VALVE_REG_PATH_1, "InstallPath" );
	else
	if ( RegValueExists( HKEY_CURRENT_USER, VALVE_REG_PATH_1, "InstallPath" ) )
		key_value = GetRegistryString( HKEY_CURRENT_USER, VALVE_REG_PATH_1, "InstallPath" );
	else
	{
		strcpy( ErrCode, Steam_StrError( ErrCode_NoPath ) );
		return NULL;
	}

	//Если что то нашли
	if ( *key_value )
	{
		if ( iAccount == 0 ) //Если в качестве параметра идет 0 то возвращаем активный ник
            NickName = Steam_GetActiveNickName();
		else
            NickName = Steam_GetNickNameN( iAccount );
		if ( NickName )
			sprintf( ProfilePath, "%s\\steamapps\\%s", key_value, NickName );
		else
		{
			*ProfilePath = '\0';
			strcpy( ErrCode, Steam_StrError( ErrCode_NoAcc ) );
		}
	}
    return ProfilePath;
}

//В случае отсутствия Steam взвращаем True
extern "C" __declspec( dllexport ) BOOL Steam_GetError( void )
{
    char    *buffer;

    buffer = Steam_LoadConfigFile();
    if ( buffer )
    {
        if( Steam_GetActiveNickName() )
        {
            return FALSE;
        }
        else
        if ( Steam_GetNumAccounts() < 1 )
        {
            return TRUE;
        }
        return FALSE;
    }

    return TRUE;
}

//В случае отсутствия Steam взвращаем True
extern "C" __declspec( dllexport ) char *Steam_Path( void )
{
	//Проверяем ветки реестра
	if ( RegValueExists( HKEY_LOCAL_MACHINE, VALVE_REG_PATH_1, "InstallPath" ) )
		strcpy( SteamPath, GetRegistryString( HKEY_LOCAL_MACHINE, VALVE_REG_PATH_1, "InstallPath" ) );
	else
	if ( RegValueExists( HKEY_CURRENT_USER, VALVE_REG_PATH_1, "InstallPath" ) )
		strcpy( SteamPath, GetRegistryString( HKEY_CURRENT_USER, VALVE_REG_PATH_1, "InstallPath" ) );
	else
	{
		strcpy( ErrCode, Steam_StrError( ErrCode_NoPath ) );
		return NULL;
	}
    return SteamPath;
}

//Возвращаем текст ошибки
extern "C" __declspec( dllexport ) char *Steam_GetErrorText( void )
{
    return ( char * )ErrCode;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
//    BOOL f64bitOS = Is64BitOperatingSystem();

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
