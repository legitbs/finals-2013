#include "Common.h"

CDomain::CDomain()
	: m_domainType( DOMAIN_TYPE_UNKNOWN )
{

}

CDomain::~CDomain()
{

}

bool CDomain::ParseDomain( const char *pszString, uint32_t length )
{
	typedef enum
	{
		READ_NAME,
		READ_NUMBER,
		READ_IP,
		READ_BEGIN
	} eMode;

	eMode mode = READ_BEGIN;
	uint32_t copyPos = 0;
	uint32_t tempIP = 0;
	uint8_t ipCharCount = 0;

	// Start processing
	for ( uint32_t pos = 0; pos < length;  )
	{
		switch ( mode )
		{
		case READ_NAME:
			if ( CSMTPHelperFunctions::IsAlphaOrDigit( pszString[pos] )  || (pszString[pos] == '-' && (pos+1) != length) || (pszString[pos] == '.' && (pos+1) != length) )
			{
				// BUG:: KEEP ALLOWING A DOMAIN NAME to exceed the name buffer!
				m_domainInfo.data.domainName.pszName[copyPos++] = pszString[pos];

				pos++;
			}
			else
				return false;

			break;

		case READ_NUMBER:
			if ( CSMTPHelperFunctions::IsDigit( pszString[pos] ) || (pszString[pos] == '.' && (pos+1) != length) )
			{
				// BUG:: KEEP ALLOWING DOMAIN NUMBER to exceed the name buffer!
				m_domainInfo.data.domainName.pszName[copyPos++] = pszString[pos];

				pos++;
			}
			else
				return false;

			break;

		case READ_IP:
			if ( CSMTPHelperFunctions::IsDigit( pszString[pos] ) )
			{
				tempIP = tempIP * 10;
				tempIP += (pszString[pos] - '0');
			}
			else if ( pszString[pos] == '.' && (pos+1) != length )
			{
				if ( ipCharCount > 3 )
					return false;	// Not a valid ip address

				if ( tempIP > 255 )
					return false; // Invalid IP address

				if ( copyPos > 3 )
					return false;

				if ( (pos+1) == length )
					return false;
				
				m_domainInfo.data.ipV4.ip[copyPos++] = (uint8_t)tempIP;

				tempIP = 0;
			}
			else
				return false;

			pos++;

			break;

		case READ_BEGIN:
			if ( pszString[pos] == '#' )
				mode = READ_NUMBER;
			else if ( CSMTPHelperFunctions::IsAlpha( pszString[pos] ) )
				mode = READ_NAME;
			else if ( CSMTPHelperFunctions::IsDigit( pszString[pos] ) )
				mode = READ_IP;
			else
				return false;

			break;
		}
	}

	if ( mode == READ_NAME )
	{
		m_domainType = DOMAIN_TYPE_NAME;
		m_domainInfo.data.domainName.pszName[copyPos] = '\0';
	}
	else if ( mode == READ_NUMBER )
	{
		m_domainType = DOMAIN_TYPE_NUMBER;
		m_domainInfo.data.domainName.pszName[copyPos] = '\0';
	}
	else if ( mode == READ_IP )
	{
		if ( copyPos != 3 )
			return false;

		m_domainInfo.data.ipV4.ip[copyPos] = (uint8_t)tempIP;

		m_domainType = DOMAIN_TYPE_IPV4;
	}
	else
		return false;

	return true;
}

char *CDomain::GetString( char *pszString, uint32_t len )
{
	switch( m_domainType )
	{
	case DOMAIN_TYPE_IPV4:
		sprintf( pszString, "%d.%d.%d.%d", m_domainInfo.data.ipV4.ip[0], m_domainInfo.data.ipV4.ip[1], m_domainInfo.data.ipV4.ip[2], m_domainInfo.data.ipV4.ip[3] ); 
		break;

	case DOMAIN_TYPE_NUMBER:
	case DOMAIN_TYPE_NAME:
		strcpy( pszString, m_domainInfo.data.domainName.pszName );
		break;

	default:
		pszString[0] = '\0';
	}

	return (pszString);
}