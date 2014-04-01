#include "Common.h"

bool CSMTPHelperFunctions::IsSpace( const char c )
{
	if ( c == ' ' )
		return true;
	else
		return false;
}

bool CSMTPHelperFunctions::IsLineTerminator( const char *pszCur )
{
	if ( pszCur[0] != '\0' &&
		 pszCur[1] != '\0' )
	{
		if ( pszCur[0] == '\r' && pszCur[1] == '\n' )
			return true;
	}

	return false;
}

bool CSMTPHelperFunctions::IsSpecialChar( const char c )
{
	/*
	<special> ::= "<" | ">" | "(" | ")" | "[" | "]" | "\" | "."
                      | "," | ";" | ":" | "@"  """ | the control
                      characters (ASCII codes 0 through 31 inclusive and
                      127)
					  */

	if ( c == '<' || c == '>' || c == '(' || c == ')' || c == '[' || c == ']'
	     || c == '\\' || c == '.' || c == ',' || c == ';' || c == ':' || c == '@'
		 || c == '"' || c <= 31 || c >= 127 )
		return true;
	else
		return false;
}

bool CSMTPHelperFunctions::IsDigit( const char c )
{
	if ( c >= '0' && c <= '9' )
		return true;
	else
		return false;
}

bool CSMTPHelperFunctions::IsAlpha( const char c )
{
	if ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') )
		return true;
	else
		return false;
}

bool CSMTPHelperFunctions::IsAlphaOrDigit( const char c )
{
	if ( IsAlpha( c ) || IsDigit( c ) )
		return true;
	else
		return false;
}

bool CSMTPHelperFunctions::IsQChar( const char c )
{
	if ( c >= 128 || c == '\r' || c == '\n' || c == '"' || c == '\\' )
		return false;
	else
		return true;
}