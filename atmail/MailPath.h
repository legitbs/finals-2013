#ifndef __MAILPATH_H__
#define __MAILPATH_H__

class CDomain;

#define MAX_LOCAL_NAME_LENGTH	128	

class CDomainContainer
{
public:
	CDomain m_domain;

	NUtil::LIST_LINK( CDomainContainer )	m_atListLink;
};

class CMailPath
{
public:
	CMailPath();
	~CMailPath();

	bool ParseMailPath( const char *pszString, uint32_t length );

	char *GetString( char *pszString, uint32_t maxLen );

	char *GetMailBoxDomain( char *pszString, uint32_t len )
	{
		return m_mailBox.domain.GetString( pszString, len );
	}

	char *GetMailBoxName( char *pszString, uint32_t len )
	{
		return strcpy( pszString, m_mailBox.szLocalName );
	}

	uint32_t GetAtDomainCount( void ) 
	{
		uint32_t count = 0;

		CDomainContainer *pCur;
		for ( pCur = m_oAtDomainList.Head(); pCur; pCur = m_oAtDomainList.Next( pCur ) )
			count++;

		return (count);
	}

	char *GetAtDomainString( char *pszString, uint32_t len )
	{
		uint32_t curLen = 0;
		uint32_t pos = 0;
		CDomainContainer *pCur;
		
		pszString[0] = '\0';

		for ( pCur = m_oAtDomainList.Head(); pCur;  )
		{
			char szTemp[255];

			pCur->m_domain.GetString( szTemp, 255 );

			curLen += strlen( szTemp )+1;

			if ( curLen > len )
				return (NULL);
			
			pos += sprintf( pszString+pos, "@%s", szTemp );

			pCur = m_oAtDomainList.Next( pCur );
			
			if ( pCur )
			{
				pszString[pos] = ',';
				pos++;
				curLen++;
			}
		}

		return pszString;
	}

private:
	

	typedef struct
	{
		char szLocalName[MAX_LOCAL_NAME_LENGTH];
		CDomain domain;
	} tMailbox;


	NUtil::LIST_DECLARE( CDomainContainer, m_atListLink ) m_oAtDomainList;
	tMailbox	m_mailBox;
};

#endif // __MAILPATH_H__
