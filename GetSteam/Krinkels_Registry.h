/*
Krinkels_Registry.h

���� ���� �������� ������ GetSteam

Copyright (c) 2010-2014 Krinkels Inc (http://krinkels.org).
��� ����� �������� �� ������.

��� ��������� ���������������� ��� ����, � ������� ��� ��� ����� �������, ��� ����� ���� �������� � ������������. 
*/

/*******************************************
*** ���������� �� ���� � �������? **********
********************************************/
int RegKeyExists( const HKEY RootKey, const char *Key )
{
	HKEY Handle;
	if( RegOpenKeyEx( RootKey, Key, 0, KEY_READ, &Handle ) == ERROR_SUCCESS )
	{
		return 1;
		RegCloseKey( Handle );
	}
	else
	{
		return 0;
	}
}

/*******************************************
*** ���������� �� �������� � �������? ******
********************************************/
BOOL RegValueExists ( const HKEY RootKey, const char *Key, const char *Name )
{
	HKEY Handle;
	if ( RegOpenKeyEx( RootKey, Key, 0, KEY_READ, &Handle ) == ERROR_SUCCESS )
	{		
		if ( RegQueryValueEx( Handle, Name, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS )
			return TRUE;
	}
	return FALSE;
}

/*******************************************
*** ������ ���������� �������� *************
********************************************/
char *GetRegistryString( HKEY RootKey, const char *Key, const char *Name )
{
	HKEY Handle;
	DWORD DataType, DataSize;
	LPBYTE buffer;
	if ( RegOpenKeyEx( RootKey, Key, 0, KEY_READ , &Handle ) != ERROR_SUCCESS )
		return NULL;
	if ( ( RegQueryValueEx( Handle, Name, NULL, &DataType, NULL, &DataSize ) != ERROR_SUCCESS ) || (DataType != REG_SZ) )
	{
		RegCloseKey( Handle );
		return NULL;
	}
	buffer = ( LPBYTE )malloc( DataSize );
	RegQueryValueEx( Handle, Name, NULL, &DataType, buffer, &DataSize );
	RegCloseKey(Handle);
	return ( char * )buffer;
}
