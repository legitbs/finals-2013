#include "Common.h"

CSMTPServerInstance::CSMTPServerInstance( CMailEngine *pMailEngine )
	: m_currentState( STATE_CONNECT ), m_pDomain( NULL ), m_pMailData( NULL ), m_pReversePath( NULL ), m_lastErrorCode( 0 ), m_pszErrorExtra( NULL ), m_responseCode( 0 ), m_pszResponseMsg( NULL ), m_pIOConnection( NULL )
{
	m_pMailEngine = pMailEngine;
	m_pCommandProcessor = new CSMTPCommand( this );
}

CSMTPServerInstance::~CSMTPServerInstance( )
{
	delete m_pCommandProcessor;

	if ( m_pReversePath )
		delete m_pReversePath;

	m_forwardPathList.DeleteAll();

	if ( m_pMailData )
		delete m_pMailData;
}

void CSMTPServerInstance::AddError( uint32_t errorCode, char *pszExtra )
{
	if ( HasError() )
	{
		ClearError();
	}

	m_lastErrorCode = errorCode;

	m_pszErrorExtra = new char[strlen(pszExtra)+1];

	strcpy( m_pszErrorExtra, pszExtra );
}

const char *CSMTPServerInstance::GetErrorString( uint32_t errorCode )
{
	typedef struct
	{
		uint32_t errorCode;
		const char *pszErrorCode;
	} tErrorCodeTable;

		/*
		421 Service not available, closing transmission channel (This may be a reply to any command if the service knows it must shut down)
450 Requested mail action not taken: mailbox unavailable (E.g., mailbox busy)
451 Requested action aborted: local error in processing
452 Requested action not taken: insufficient system storage
500 Syntax error, command unrecognized (This may include errors such as command line too long)
501 Syntax error in parameters or arguments
502 Command not implemented
503 Bad sequence of commands
504 Command parameter not implemented
550 Requested action not taken: mailbox unavailable (E.g., mailbox not found, no access)
551 User not local; please try
552 Requested mail action aborted: exceeded storage allocation
553 Requested action not taken: mailbox name not allowed (E.g., mailbox syntax incorrect)
554 Transaction failed

The other codes that provide you with helpful information about what's happening with your messages are:

211 System status, or system help reply
214 Help message (Information on how to use the receiver or the meaning of a particular non-standard command; this reply is useful
only to the human user)
220 Service ready
221 Service closing transmission channel
250 Requested mail action okay, completed
251 User not local; will forward to
354 Start mail input; end with . (a dot)
*/

	static tErrorCodeTable oErrorCodeTable[] = {
		{ 421,	"Service not available, closing transmission channel" },
		{ 450,	"Requested mail action not taken: mailbox unavailable" },
		{ 451,	"Requested action aborted: local error in processing" },
		{ 452,  "Requested action not taken: insufficient system storage" },
		{ 500,	"Syntax error, command unrecognized" },
		{ 501,	"Syntax error in parameters or arguments" },
		{ 502,  "Command not implemented" },
		{ 503,  "Bad sequence of commands" },
		{ 504,  "Command parameter not implemented" },
		{ 550,  "Requested action not taken: mailbox unavailable" },
		{ 551,	"User not local; please try" },
		{ 552,	"Requested mail action aborted: exceeded storage allocation" },
		{ 553,  "Requested action not taken: mailbox name not allowed" },
		{ 554,	"Transaction failed" },

		{ 0, "" }
	};

	uint32_t i = 0;
	for ( ; ; i++ )
	{
		if ( oErrorCodeTable[i].errorCode == 0 )
			break;

		if ( oErrorCodeTable[i].errorCode == errorCode )
			return (oErrorCodeTable[i].pszErrorCode);
	}

	return (oErrorCodeTable[i].pszErrorCode);
}

void CSMTPServerInstance::AddResponse( uint32_t responseCode, char *pszResponseMsg )
{
	if ( HasResponse() )
	{
		ClearResponse();
	}

	m_responseCode = responseCode;

	m_pszResponseMsg = new char[strlen(pszResponseMsg)+1];

	strcpy( m_pszResponseMsg, pszResponseMsg );
}

void CSMTPServerInstance::SendErrorReply( void )
{
	if ( !HasError() )
		return;

	if ( m_pszErrorExtra )
	{
		const char *pszErrorCodeString = GetErrorString( m_lastErrorCode );
		char *pszTemp = new char [strlen(pszErrorCodeString) + strlen(m_pszErrorExtra) + 32];

		sprintf( pszTemp, "%d %s %s\r\n", m_lastErrorCode, pszErrorCodeString, m_pszErrorExtra );

		SendData( (uint8_t*)pszTemp, strlen( pszTemp ) );

		delete pszTemp;
	}
	else
	{
		const char *pszErrorCodeString = GetErrorString( m_lastErrorCode );
		char *pszTemp = new char [strlen(pszErrorCodeString) + 32];

		sprintf( pszTemp, "%d %s\r\n", m_lastErrorCode, pszErrorCodeString );

		SendData( (uint8_t*)pszTemp, strlen( pszTemp ) );

		delete pszTemp;
	}
}

void CSMTPServerInstance::SendResponseReply( void )
{
	if ( !HasResponse() )
		return;

	char *pszTemp = new char[strlen(m_pszResponseMsg) + 32];

	sprintf( pszTemp, "%d %s\r\n", m_responseCode, m_pszResponseMsg );

	SendData( (uint8_t*)pszTemp, strlen( pszTemp ) );

	delete pszTemp;
}

uint32_t CSMTPServerInstance::RecvLine( void )
{
	uint8_t pData[1024];
	uint32_t dataLen;

	if ( m_pIOConnection == NULL )
		return 0;

	dataLen = m_pIOConnection->ReadLine( pData, 1024 );

	if ( dataLen == 0 )
		return 0;

	ClearError();
	ClearResponse();

	if ( m_currentState != STATE_DATA )
	{
		m_pCommandProcessor->ParseInput( (const char *)pData, dataLen );
	}
	else
	{
		uint32_t pos = 0;

		bool bFound = false;
		if ( dataLen >= 3 && m_pMailData->GetDataSize() > 0 )
		{
			while ( pos < (dataLen-2) )
			{
				if ( pData[pos] == '.' && pData[pos+1] == '\r' && pData[pos+2] == '\n'  )
				{
					bFound = true;
					break;
				}

				pos++; 
			}
		}

		if ( m_pMailData == NULL )
		{
			AddError( ERROR_503, "" );		// Bad sequence of commands
		}
		else if ( m_pMailData->GetDataSize() > MAX_MAIL_DATA_SIZE )
		{
			// Overran data buffer
			AddError( 552, "Too much mail data" );

			// Reset our mail state
			ResetMailState();

			// Resume mail recieving... CLEAR EVERYTHING FIRST
			m_currentState = STATE_IDENTIFIED;
		}
		else if ( bFound )
		{
			// Add chunk (excluding CRLF.CRLF)
			m_pMailData->AddDataEnd( pData, pos );

			// ____ SEND MAIL FOR PROCESSING ____
			bool bProcessResult = m_pMailEngine->RecvMail( this, CMailPacket::GenerateMailPacket( m_pMailData, m_pDomain, m_pReversePath, &m_forwardPathList ) );

			m_pReversePath = NULL;
			m_pMailData = NULL;

			// Reset our mail state
			ResetMailState();

			if ( bProcessResult )
			{
				// MIGHT WANT to make this more informative.
				AddResponse( 250, "OK. Queued." );
			}
			else
			{
				// No mailbox found
				AddError( 550, "" );
			}

			// Resume mail recieving... CLEAR EVERYTHING FIRST
			m_currentState = STATE_IDENTIFIED;
		}
		else
		{
			m_pMailData->AddDataEnd( pData, dataLen );
		}
	}

	if ( HasError() )
	{
		SendErrorReply();
	}
	else if ( HasResponse() )
	{
		SendResponseReply();
	}

	return (dataLen);
}

void CSMTPServerInstance::SendData( uint8_t *pData, uint32_t dataLen )
{
	if ( m_pIOConnection )
		m_pIOConnection->WriteLine( pData, dataLen );
}

uint32_t CSMTPServerInstance::GetForwardPathListSize( void )
{
	CForwardPath *pPath;

	uint32_t count = 0;
	for ( pPath = m_forwardPathList.Head(); pPath; pPath = m_forwardPathList.Next( pPath ) )
		count++;

	return (count);
}

void CSMTPServerInstance::DoHelo( CDomain *pNewDomain )
{
	if ( m_currentState != STATE_CONNECT )
	{
		AddError( ERROR_503, "" );		// Bad sequence of commands
		return;
	}

	if ( pNewDomain == NULL )
	{
		AddError( ERROR_500, "" );		// THIS SHOULD NOT HAPPEN: SYNTAX ERROR
		return;
	}

	m_pDomain = pNewDomain;

	char szTemp[1024];
	char szDomain[256];
	
	// Create response
	sprintf( szTemp, "Hello %s", m_pDomain->GetString( szDomain, 1024 ) );

	AddResponse( 250, szTemp );

	// Advance state
	m_currentState = STATE_IDENTIFIED;
}

void CSMTPServerInstance::DoMail( CMailPath *pNewMailPath )
{
	if ( m_currentState != STATE_IDENTIFIED )
	{
		AddError( ERROR_503, "" );
		return;
	}

	if ( pNewMailPath == NULL )
	{
		AddError( ERROR_500, "" );
		return;
	}

	// Reset mail state
	ResetMailState();

	m_pReversePath = pNewMailPath;

	AddResponse( 250, "Ok" ); 
}

void CSMTPServerInstance::DoRcpt( CMailPath *pNewMailPath )
{
	if ( m_currentState != STATE_IDENTIFIED )
	{
		AddError( ERROR_503, "" );
		return;
	}

	if ( pNewMailPath == NULL )
	{
		AddError( ERROR_500, "" );
		return;
	}

	if ( GetForwardPathListSize() > MAX_FORWARD_PATH_LIST )
	{
		AddError( ERROR_502, "Too many recipients" );
	}

	// Add recipients
	CForwardPath *pPathContainer = new CForwardPath(pNewMailPath);

	m_forwardPathList.InsertTail( pPathContainer );

	AddResponse( 250, "Ok" ); 
}

void CSMTPServerInstance::DoData( void )
{
	if ( m_currentState != STATE_IDENTIFIED )
	{
		AddError( ERROR_503, "" );
		return;
	}

	// Switch to data state
	m_currentState = STATE_DATA;

	// Generate mail data
	if ( m_pMailData )
		delete m_pMailData;

	m_pMailData = new CMailData();

	// Add response that we entered data state
	AddResponse( 354, "End data with <CR><LF>.<CR><LF>" );
}

void CSMTPServerInstance::DoReset( void )
{
	if ( m_currentState == STATE_CONNECT )
	{
		AddError( ERROR_503, "" );
		return;
	}

	// Reset mail state
	ResetMailState( );
	
	// Switch to data state
	m_currentState = STATE_IDENTIFIED;

	// Add response that received command OK
	AddResponse( 250, "Ok" );
}

void CSMTPServerInstance::DoQuit( void )
{
	// Send response
	AddResponse( 221, "LEGITBS.NET Service closing transmission channel" );

	// Send response
	SendResponseReply();

	ResetMailState();

	// Close program
	exit(1);
}

void CSMTPServerInstance::ResetMailState( void )
{
	// Clear if we have DATA, FORWARD PATH or REVERSE PATH
	if ( m_pReversePath )
		delete m_pReversePath;

	m_pReversePath = NULL;

	// Clear recipient list
	m_forwardPathList.DeleteAll();

	// Clear mail data
	if ( m_pMailData )
		m_pMailData->ClearData();
}

void CSMTPServerInstance::DoTurn( void )
{
	if ( m_currentState == STATE_CONNECT )
	{
		AddError( ERROR_503, "" );
		return;
	}

	// Reset mail state
	ResetMailState();

	// Add response that received command OK
	AddResponse( 250, "Ok" );

	// Send response
	SendResponseReply();

	// Run relay
	m_pMailEngine->SetRelayState();
}

void CSMTPServerInstance::Start( const char *pszDomain )
{
	char szConnectMessage[1024];

	sprintf( szConnectMessage, "%s SMTP ready", pszDomain );

	// Add response that received command OK
	AddResponse( 220, szConnectMessage );

	// Send response
	SendResponseReply();
}

void CSMTPServerInstance::Run( void )
{
	RecvLine();
}
