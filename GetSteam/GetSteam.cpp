/*
GetSteam.h

���� ���� �������� ������ GetSteam

Copyright (c) 2010-2014 Krinkels Inc (http://krinkels.org).
��� ����� �������� �� ������.

��� ��������� ���������������� ��� ����, � ������� ��� ��� ����� �������, ��� ����� ���� �������� � ������������. 
*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h> // ����������� ���������� �����-������
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
	"��",
	"����������� SteamAppData.vdf",
	"���������� �������� ������ �����",
	"��� ������� � �����",
	"���������� ������� �����",
	"������ ������ ����� � �����",
	"��� ������ ������ �� �������",
	"�� ���� ����� ���� � Steam",
	"������� �����������",
	"SteamAppData.vdf ������",
	"��� ������� �������",
	"���� ������� �������",
	"������ ������ �����"
};

typedef enum
{
	ErrCode_Ok = 0,			// ��
	ErrCode_FileExst,		// ����������� SteamAppData.vdf
	ErrCode_GetSize,		// ���������� �������� ������ �����
	ErrCode_OpenErr,		// ��� ������� � �����
	ErrCode_ErrNewBuf,		// ���������� ������� �����
	ErrCode_ErrWriteF,		// ������ ������ ����� � �����
	ErrCode_ErrSearch,		// ��� ������ ������ �� �������
	ErrCode_NoPath,			// �� ���� ����� ���� � Steam
	ErrCode_NoAcc,			// ������� �����������
	ErrCode_EmptyFile,		// SteamAppData.vdf ������
	ErrCode_ErrNick,		// ��� ������� �������
	ErrCode_SizeFile,       // ���� ������� �������
	ErrCode_ReadFile        // ������ ������ �����
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
		// ���� ����� �� ������� �� ���������� ������ ��������
		::FindClose(hFind);
		return true;
	}
	return false;
}

/* Gets file size from 'name' (file must exists) */
/*******************************************
*** �������� ������ ����� �� "�����" *******
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

	//���������, ���������� �� ����
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

	//������ ������ �����
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

// �������� ��������� ����� (���������� ����� ����� �������� ���������)
static BOOL GetToken( void )
{
    char    *ptr, *ptr2;

    memset( token, 0, sizeof( token ) );

    // ���� ������� �������
    ptr = strchr( CurrentPos, '"' );
	//���� �� �����
    if ( !ptr )
    {
        EndOfBuffer = TRUE;
        return FALSE;
    }
    else
    {
		//���� ��������� �������
        ptr++;
        ptr2 = strchr( ptr, '"' );
		//���� �� �����
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
		//�������� � token ����� ������ ���������
        strncpy( token, ptr, ptr2 - ptr );
        CurrentPos = ptr2 + 1;
    }

    return TRUE;
}

// ����� ����� ������� � ������
 BOOL FindNick2( void )
{
    BOOL    Found = false;

    //���� EndOfBuffer = FALSE
    while ( !EndOfBuffer )
    {
        //���� GetToken ������ �� �����, �.�. ������ FALSE �� ��������� ����
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

// �������� ���� � ������ ����� �� ������� � ��������� ��� � ������
static char *Steam_LoadConfigFile( void )
{
	char	ConfigPath[ _MAX_PATH ] = {0};
    char 	key_value[ _MAX_PATH ] = {0};
    char	*buffer = NULL;

	EndOfBuffer = FALSE;

	//��������� ����� �������
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

	//���� ��� �� ����� ��
	if ( *key_value )
		sprintf( ConfigPath, "%s\\config\\SteamAppData.vdf", key_value ); //�������� ���� � SteamAppData.vdf
	else
	{
        strcpy( ErrCode, Steam_StrError( ErrCode_NoPath ) ); //����� ���������� ������
		return NULL;
	}

	// ������ ���� ���� � �����.
	if ( !LoadFile( ConfigPath, &buffer ) )
	{
        strcpy( ErrCode, Steam_StrError( ErrCode_ReadFile ) );
        return NULL;
    }

	FileBuffer = buffer;
	CurrentPos = FileBuffer;

    return FileBuffer;
}

// ���������� ����� ���������
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

// ���������� ��� # ��������
extern "C" __declspec( dllexport ) char *Steam_GetNickNameN( int iAccount )
{
    int		NumAccounts = 1;
    char    *buffer;
    BOOL    Found = true;

    //�������� ���� � ����� ������������
    buffer = Steam_LoadConfigFile();
    if ( !buffer )
        return NULL;

    while ( Found )
    {
        //���������� User � �����
        Found = FindNick2();
        //���� ��� �� ��������
        if ( Found )
        {
            //���� NumAccounts ����� ������ �������������� ���� �� ���������� ���
            if ( iAccount == NumAccounts )
            {
                return nick;
            }
            //����� ����������� NumAccounts �� 1
            NumAccounts++;
        }
    }

    //���� ��� ��� �� ������ �� ���������� NULL
    return NULL;
}

// ���������� ��� ��������� ��������
extern "C" __declspec( dllexport ) char *Steam_GetActiveNickName( void )
{
	char 	*buffer;

	buffer = Steam_LoadConfigFile();
	if ( !buffer )
		return NULL;

	//���� EndOfBuffer = FALSE
	while ( !EndOfBuffer )
	{
		//���� GetToken ������ �� �����, �.�. ������ FALSE �� ��������� ����
		if ( !GetToken() )
			break;
		//����� ���������� �����
		//���� stricmp == 0 �.�. token ����� User
		if ( !_stricmp( token, "AutoLoginUser" ) )
		{
			//� ���� ����� ������� FALSE
			if ( !GetToken() )
				break;
			//�������� ��� ���
			strcpy( nick, token );
		}
		if ( *nick )
		{
			//���������� �������� ���
			return nick;
		}

		*nick = '\0';
	}

	return NULL;
}

//���������� ���� � N �������
extern "C" __declspec( dllexport ) char *Steam_GetProfilePath( int iAccount )
{
	char 	*key_value = NULL;
	char	*NickName = NULL;

	//��������� ����� �������
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

	//���� ��� �� �����
	if ( *key_value )
	{
		if ( iAccount == 0 ) //���� � �������� ��������� ���� 0 �� ���������� �������� ���
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

//� ������ ���������� Steam ��������� True
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

//� ������ ���������� Steam ��������� True
extern "C" __declspec( dllexport ) char *Steam_Path( void )
{
	//��������� ����� �������
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

//���������� ����� ������
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
