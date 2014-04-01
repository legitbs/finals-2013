#ifndef __SMTP_CLIENT_INSTANCE_H__
#define __SMTP_CLIENT_INSTANCE_H__

class CSMTPClientInstance
{
public:
	CSMTPClientInstance() : m_pszDomain( NULL ), m_pIOConnection( NULL ) { m_state = STATE_DOHELO; };
	~CSMTPClientInstance() 
	{
		if ( m_pszDomain )
			delete m_pszDomain;
	}

	void AddIOConnection( CIOConnection *pConnection )
	{
		m_pIOConnection = pConnection;
	}

	void SetDomain( const char *pszDomain )
	{
		if ( m_pszDomain )
			delete m_pszDomain;

		m_pszDomain = new char[strlen(pszDomain)+1];

		strcpy( m_pszDomain, pszDomain );
	}

	bool SendRelayMessageFromFile( const char *pszFileName );
	bool Connect( void );

	bool SendHelo( void );
	bool SendReset( void );
	bool SendTurn( void );

private:
	typedef enum
	{
		STATE_DOHELO,
		STATE_DOMAIL,
		STATE_DORCPT,
		STATE_DODATA,
		STATE_ERROR
	} eSMTPClientState;

	eSMTPClientState m_state;
	CIOConnection *m_pIOConnection;
	char *m_pszDomain;
};

#endif // __SMTP_CLIENT_INSTANCE_H__