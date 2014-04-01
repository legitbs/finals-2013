#include "Common.h"

CMailPath::CMailPath()
{

}
	
CMailPath::~CMailPath()
{
	m_oAtDomainList.DeleteAll();
}


bool CMailPath::ParseMailPath( const char *pszString, uint32_t length )
{
	typedef enum
	{
		READ_ATDOMAINLIST,
		READ_MAILBOX_QUOTE,
		READ_MAILBOX_NORMAL,
		READ_MAILBOX_DOMAIN,
		READ_COMPLETE,
		READ_BEGIN
	} eMode;

	eMode mode = READ_BEGIN;
	uint32_t copyPos = 0;

	// Start processing
	for ( uint32_t pos = 0; pos < length;  )
	{
		switch ( mode )
		{
		case READ_ATDOMAINLIST:
			{
				// Scan for comma or :
				uint32_t endPos = pos;

				bool bFound = false;
				for ( ; endPos < length; endPos++ )
				{
					if ( pszString[endPos] == ',' )
					{
						// READ DOMAIN inside this length
						CDomainContainer *pNewDomainContainer = new CDomainContainer();

						if ( !pNewDomainContainer->m_domain.ParseDomain( (pszString+pos), (endPos-pos) ) )
						{
							delete pNewDomainContainer;
							return (false);
						}

						m_oAtDomainList.InsertTail( pNewDomainContainer );

						// Advance past the :
						pos = endPos+1;

						if ( pos >= length )
							return false;

						// Consume next @
						if ( pszString[pos] != '@' )
							return false;

						// Advance
						pos++;
					}
					else if ( pszString[endPos] == ':' )
					{
						// READ DOMAIN inside this length
						CDomainContainer *pNewDomainContainer = new CDomainContainer();

						if ( !pNewDomainContainer->m_domain.ParseDomain( (pszString+pos), (endPos-pos) ) )
						{
							delete pNewDomainContainer;
							return (false);
						}

						m_oAtDomainList.InsertTail( pNewDomainContainer );

						// Advance past the :
						pos = endPos+1;
						
						// Now set mode to mailbox
						if ( pos < length && pszString[pos] == '"' )
						{
							mode = READ_MAILBOX_QUOTE;
							pos++;
						}
						else if ( pos < length )
						{
							mode = READ_MAILBOX_NORMAL;
						}
						else
							return false;
						
						bFound = true;
						break;
					}
				}

				if ( !bFound )
					return false;
			}
			break;

		case READ_MAILBOX_NORMAL:
			if ( pszString[pos] == '@' )
			{
				// END
				m_mailBox.szLocalName[copyPos] = '\0';

				pos++;

				mode = READ_MAILBOX_DOMAIN;
			}
			else
			{
				if ( CSMTPHelperFunctions::IsSpecialChar( pszString[pos] ) )
					return false;

				m_mailBox.szLocalName[copyPos++] = pszString[pos];

				// CHECK LENGTH HERE:
				if ( copyPos >= MAX_LOCAL_NAME_LENGTH )
					return false;

				pos++;
			}
			break;

		case READ_MAILBOX_QUOTE:
			if ( pszString[pos] == '"' )
			{
				// END
				m_mailBox.szLocalName[copyPos] = '\0';

				pos++;

				if ( pos < length && pszString[pos] == '@' )
				{
					pos++;

					mode = READ_MAILBOX_DOMAIN;
				}
				else
					return false;

			}
			else if ( pszString[pos] == '\\' )
			{
				pos++;
				if ( pos >= length )
					return false;

				if ( pszString[pos] >= 128 )
					return false;

				// Store value
				m_mailBox.szLocalName[copyPos++] = pszString[pos];

				// CHECK LENGTH HERE:
				if ( copyPos >= MAX_LOCAL_NAME_LENGTH )
					return false;
				
				// Advance
				pos++;
			}
			else
			{
				if ( CSMTPHelperFunctions::IsSpecialChar( pszString[pos] ) )
					return false;

				m_mailBox.szLocalName[copyPos++] = pszString[pos];

				// CHECK LENGTH HERE:
				if ( copyPos >= MAX_LOCAL_NAME_LENGTH )
					return false;

				// Advance
				pos++;
			}

			break;

		case READ_MAILBOX_DOMAIN:
			{
				// READ DOMAIN inside this length
				if ( !m_mailBox.domain.ParseDomain( (pszString+pos), (length-pos) ) )
				{
					return (false);
				}

				pos = length;
				mode = READ_COMPLETE;
			}
			break;

		case READ_BEGIN:
			if ( pszString[pos] == '@' )
			{
				pos++;

				mode = READ_ATDOMAINLIST;
			}
			else if ( pszString[pos] == '"' )
			{
				pos++;

				mode = READ_MAILBOX_QUOTE;
			}
			else if ( !CSMTPHelperFunctions::IsSpecialChar( pszString[pos] ) )
			{
				mode = READ_MAILBOX_NORMAL;
			}
			else
				return false;

			break;

		default:
			// SHOULD NOT GET HERE IF PROPERLY FORMATTED
			return false;
		}
	}

	if ( mode != READ_COMPLETE )
		return false;

	return true;
}

char *CMailPath::GetString( char *pszString, uint32_t maxLen )
{
	if ( GetAtDomainCount() > 0 )
	{
		char szAtDomainList[256];
		char szLocalName[256];
		char szLocalDomain[256];

		sprintf( pszString, "%s:%s@%s", GetAtDomainString( szAtDomainList, 256 ), GetMailBoxName( szLocalName, maxLen ), GetMailBoxDomain( szLocalDomain, maxLen ) );
	}
	else
	{
		char szLocalName[256];
		char szLocalDomain[256];

		sprintf( pszString, "%s@%s", GetMailBoxName( szLocalName, maxLen ), GetMailBoxDomain( szLocalDomain, maxLen ) ); 
	}

	return pszString;
}
