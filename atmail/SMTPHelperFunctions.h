#ifndef __SMTPHELPER_FUNCTIONS_H__
#define __SMTPHELPER_FUNCTIONS_H__

class CSMTPHelperFunctions
{
public:
	static bool IsSpace( const char c );
	static bool IsLineTerminator( const char *pszCur );
	static bool IsSpecialChar( const char c );
	static bool IsDigit( const char c );
	static bool IsAlpha( const char c );
	static bool IsAlphaOrDigit( const char c );
	static bool IsQChar( const char c );
};

#endif // __SMTPHELPER_FUNCTIONS_H__