#ifndef __MAILENGINE_H__
#define __MAILENGINE_H__

class CMailPacket
{
public:
	CMailPacket();
	~CMailPacket();

	static CMailPacket *GenerateMailPacket( CMailData *pMailData, CDomain *pDomain, CMailPath *pReversePath, NUtil::LIST_DECLARE( CForwardPath, m_forwardPathLink ) *pForwardPathList );

	uint32_t GetRecipientListSize( void ) const { return m_forwardPathListCount; };
	CMailPath *GetRecipientAt( uint32_t idx )
	{
		if ( idx < m_forwardPathListCount )
			return m_pForwardPathList[idx];
		else
			return (NULL);
	}

	char *GetFromDomain( char *buffer, uint32_t len )
	{
		return m_pFromDomain->GetString( buffer, len );
	}

	CDomain *GetDomain( void ) { return m_pFromDomain; }

	char *GetMailTimeStamp( char *buffer, uint32_t len );

	CMailData *GetMailData( void )
	{
		return (m_pMailData);
	}

	CMailPath *GetReversePath( void )
	{
		return (m_pReversePath);
	}

private:
	CMailData	*m_pMailData;

	CDomain		*m_pFromDomain;
	CMailPath	*m_pReversePath;

	CMailPath	**m_pForwardPathList;
	uint32_t	m_forwardPathListCount;
};

class CRelayMailContainer
{
public:
	NUtil::LIST_LINK( CRelayMailContainer )		m_relayMailContainer;

	
public:
	CRelayMailContainer( char *pszFileName, char *pszNextDomain )
	{
		m_pszFileName = new char[strlen(pszFileName)+1];
		m_pszNextDomain = new char[strlen(pszNextDomain)+1];

		strcpy( m_pszFileName, pszFileName );
		strcpy( m_pszNextDomain, pszNextDomain );
	}

	~CRelayMailContainer()
	{
		if ( m_pszFileName )
			delete m_pszFileName;

		if ( m_pszNextDomain )
			delete m_pszNextDomain;
	}

	const char *GetFileName( void ) const { return m_pszFileName; };
	const char *GetDomain( void ) const { return m_pszNextDomain; };

private:
	char		*m_pszFileName;		// File name path for mail
	char		*m_pszNextDomain;		// Next domain for this message
};

class CUserMailBox
{
public:
	NUtil::LIST_LINK( CUserMailBox )	m_userMailBoxLink;

public:
	CUserMailBox( char *pszUserName, char *pszFolderPath )
	{
		strcpy( m_szUserName, pszUserName );
		strcpy( m_szFolderPath, pszFolderPath );
	}

	~CUserMailBox()
	{

	}

	const char *GetRecipientName( void ) const { return m_szUserName; };
	const char *GetFolderPath( void ) const { return m_szFolderPath; };

private:
	char	m_szFolderPath[255];		// Path to the users mailbox
	char	m_szUserName[255];			// Username for this mailbox

};

class CMailEngine
{
public:
	CMailEngine();
	~CMailEngine();

	void SetIOConnection( CIOConnection *pIOConnection )
	{
		m_pIOConnection = pIOConnection;
	}

	void AttachSpamFilter( CSpamFilter *pSpamFilter )
	{
		m_pSpamFilter = pSpamFilter;
	}

	bool RecvMail( CSMTPServerInstance *pServer, CMailPacket *pMailPacket );

	CUserMailBox *GetMailBoxForRecipient( const char *pszRecipientName );

	char *GetMailTimeStamp( char *pszBuffer, uint32_t len );

	void GenerateReceivedFromLine( CMailData *pData, CDomain *pFromDomain );
	void GenerateReturnPathLine( CMailData *pMailData, CMailPath *pRecipient );
	void GenerateSpamGuardHeader( CMailData *pMailData );

	void DoRelayMessage( CMailData *pMailData, CMailPath *pReturnPath, CMailPath *pRecipient, CDomain *pFromDomain );
	void DoRecvMail( CUserMailBox *pMailBox, CMailData *pMailData, CMailPath *pReturnPath, CMailPath *pRecipient, CDomain *pFromDomain );
	void DoUndelivered( CMailData *pMailData, CMailPath *pReturnPath, CMailPath *pRecipient, CDomain *pFromDomain );

	void LoadRelayMessages( void );

	void SanitizeMailDataForMBOX( CMailData *pData );
	char *SanitizeFileName( char *pszInFile );

	void LoadUsers( void );

	void SetRelayState( void )
	{
		m_state = STATE_CLIENT;
	}

	void DoRun( void );
	void DoServer( void );
	void DoRelay( void );

private:
	typedef enum
	{
		STATE_SERVER,
		STATE_CLIENT,
	} eSMTPEngineState;

	NUtil::LIST_DECLARE( CUserMailBox, m_userMailBoxLink )				m_userMailBoxList;
	NUtil::LIST_DECLARE( CRelayMailContainer, m_relayMailContainer )	m_relayMailList;

	CIOConnection *m_pIOConnection;

	MersenneRNG		m_oRNG;

	uint32_t	m_relayCount;
	eSMTPEngineState	m_state;

	CSpamFilter		*m_pSpamFilter;

	char	m_szDomain[255];		// The domain for this engine
};

#endif // __MAILENGINE_H__
