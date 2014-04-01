#ifndef __SMTPSERVERINSTANCE_H__
#define __SMTPSERVERINSTANCE_H__

class CSMTPCommand;
class CMailEngine;

class CForwardPath
{
public:
	CForwardPath( CMailPath *pMailPath )
		: m_pMailPath( pMailPath )
	{

	}

	~CForwardPath( )
	{
		if ( m_pMailPath )
			delete m_pMailPath;
	}

	CMailPath *GetMailPath( void ) { return m_pMailPath; }

	void ClearMailPath( void ) { m_pMailPath = NULL; };

	NUtil::LIST_LINK( CForwardPath )	m_forwardPathLink;

private:
	CMailPath	*m_pMailPath;

	
};

class CSMTPServerInstance
{
public:
	friend class CSMTPCommand;

	typedef enum
	{
		STATE_CONNECT,
		STATE_IDENTIFIED,
		STATE_DATA,
		STATE_UNKNOWN
	} eSMTPInstanceState;

public:
	CSMTPServerInstance( CMailEngine *pMailEngine );
	~CSMTPServerInstance( );

	void AddError( uint32_t errorCode, char *pszExtra );

	void AddResponse( uint32_t responseCode, char *pszResponseMsg );

	void AddIOConnection( CIOConnection *pIOConnection )
	{
		m_pIOConnection = pIOConnection;
	}

	void Start( const char *pszDomain );
	void Run( void );

protected:
	uint32_t RecvLine();
	void SendData( uint8_t *pData, uint32_t dataLen );

	void DoHelo( CDomain *pNewDomain );
	void DoMail( CMailPath *pNewMailPath );
	void DoRcpt( CMailPath *pNewMailPath );
	void DoData( void );
	void DoReset( void );
	void DoQuit( void );
	void DoTurn( void );

	void ClearError( void ) 
	{ 
		m_lastErrorCode = 0;

		if ( m_pszErrorExtra )
			delete m_pszErrorExtra;

		m_pszErrorExtra = NULL;
	}

	bool HasError( void ) { return (m_lastErrorCode != 0); }
	const char *GetErrorString( uint32_t errorCode );

	void ClearResponse( void )
	{
		m_responseCode = 0;
		
		if ( m_pszResponseMsg )
			delete m_pszResponseMsg;

		m_pszResponseMsg = NULL;
	}

	bool HasResponse( void ) { return (m_responseCode != 0); }

	void SendErrorReply( void );
	void SendResponseReply( void );

	uint32_t GetForwardPathListSize( void );

	void ResetMailState( void );

private:
	CSMTPCommand		*m_pCommandProcessor;
	eSMTPInstanceState	m_currentState;
	uint32_t			m_lastErrorCode;
	char				*m_pszErrorExtra;

	uint32_t			m_responseCode;
	char			    *m_pszResponseMsg;


	// MAIL STATE DEPENDENT DATA:
	CDomain		*m_pDomain;
	CMailPath	*m_pReversePath;
	CMailData	*m_pMailData;


	// IO Connection
	CIOConnection *m_pIOConnection;

	// Mail Engine
	CMailEngine *m_pMailEngine;

	NUtil::LIST_DECLARE( CForwardPath, m_forwardPathLink )	m_forwardPathList;
};

#endif // __SMTPSERVERINSTANCE_H__