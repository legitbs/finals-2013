#include "Common.h"

CSpamFilter::CSpamFilter( )
{

}

CSpamFilter::~CSpamFilter( )
{

}

bool CSpamFilter::IsHomoglyphCharacter( const char c )
{
	if ( CSMTPHelperFunctions::IsAlphaOrDigit( c ) || c == '!' || c == '@' )
		return true;
	else
		return false;
}

char CSpamFilter::GetBaseCharacter( const char c )
{
	switch( c )
	{
	case 'a':
	case 'A':
	case '@':
		return 0;

	case 'b':
	case 'B':
		return 1;

	case 'c':
	case 'C':
		return 2;

	case 'd':
	case 'D':
		return 3;

	case 'e':
	case 'E':
	case '3':
		return 4;

	case 'f':
	case 'F':
		return 5;

	case 'g':
	case 'G':
		return 6;

	case 'h':
	case 'H':
		return 7;
	
	case 'i':		// lowercase i
	case 'I':		// uppercase i
	case 'l':		// lowercase l
	case 'L':		// uppercase l
	case '1':		// number 1
	case '!':		// exclamation mark
		return 8;

	case 'j':
	case 'J':
		return 9;

	case 'k':
	case 'K':
		return 10;

	// SKIP L it is covered above
	case 'm':
	case 'M':
	case 'n': // M and N
	case 'N':
		return 11;

	case 'o':
	case 'O':
	case '0':	// number zero
		return 12;

	case 'p':
	case 'P':
		return 13;

	case 'Q':
	case 'q':
		return 14;

	case 'r':
	case 'R':
		return 15;

	case 's':
	case 'S':
		return 16;

	case 't':
	case 'T':
		return 17;

	case 'u':
	case 'U':
		return 18;

	case 'v':
	case 'V':
		return 19;

	case 'w':
	case 'W':
		return 20;

	case 'X':
	case 'x':
		return 21;

	case 'y':
	case 'Y':
		return 22;

	case 'z':
	case 'Z':
		return 23;

	default:	// All other numbers etc.
		return 24;
	}
}