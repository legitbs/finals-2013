#include "Common.h"

CSMTPCommand::CSMTPCommand( CSMTPServerInstance *pServer )
	: m_pServer( pServer )
{

}

CSMTPCommand::~CSMTPCommand()
{

}

/*
HELO <SP> <domain> <CRLF>

            MAIL <SP> FROM:<reverse-path> <CRLF>

            RCPT <SP> TO:<forward-path> <CRLF>

            DATA <CRLF>

            RSET <CRLF>

            SEND <SP> FROM:<reverse-path> <CRLF>

            SOML <SP> FROM:<reverse-path> <CRLF>

            SAML <SP> FROM:<reverse-path> <CRLF>

            VRFY <SP> <string> <CRLF>

            EXPN <SP> <string> <CRLF>

            HELP [<SP> <string>] <CRLF>

            NOOP <CRLF>

            QUIT <CRLF>

            TURN <CRLF>

		*/


uint32_t CSMTPCommand::ParseInput( const char *pszLine, uint32_t length )
{
	static tCommandEntry oCommandEntries[] = {
	{ "HELO",	&CSMTPCommand::HeloCommandHandler	},
	{ "MAIL",	&CSMTPCommand::MailCommandHandler	},
	{ "RCPT",	&CSMTPCommand::RcptCommandHandler	},
	{ "DATA",	&CSMTPCommand::DataCommandHandler	},
	{ "RSET",	&CSMTPCommand::RsetCommandHandler	},
	{ "SEND",	&CSMTPCommand::SendCommandHandler	},
	{ "SOML",	&CSMTPCommand::SomlCommandHandler	},
	{ "SAML",	&CSMTPCommand::SamlCommandHandler	},
	{ "VRFY",	&CSMTPCommand::VrfyCommandHandler	},
	{ "EXPN",	&CSMTPCommand::ExpnCommandHandler	},
	{ "HELP",	&CSMTPCommand::HelpCommandHandler	},
	{ "NOOP",	&CSMTPCommand::NoopCommandHandler	},
	{ "QUIT",	&CSMTPCommand::QuitCommandHandler	},
	{ "TURN",	&CSMTPCommand::TurnCommandHandler	},
	{ "",		NULL }
	};

	// READ IN THE COMMAND
	if ( length >= 6 )
	{
		bool bFound = false;
		unsigned char pszCommand[5];

		// Consume to CRLF
		// Consume whitespace
		const char *startPos = pszLine;
		uint32_t scanPos = 0;

		// Read until CRLF
		while ( scanPos < (length-1) )
		{
			if ( CSMTPHelperFunctions::IsLineTerminator( pszLine+scanPos ) )
			{
				bFound = true;
				break;
			}

			scanPos++;
		}

		if ( !bFound )
		{
			// NO CRLF -- need more data.
			printf( "Not found %d.\n", length );
			return 0;
		}

		pszCommand[0] = toupper( pszLine[0] );
		pszCommand[1] = toupper( pszLine[1] );
		pszCommand[2] = toupper( pszLine[2] );
		pszCommand[3] = toupper( pszLine[3] );
		pszCommand[4] = '\0';

		bFound = false;
		bool bError = false;
		for ( uint32_t idx = 0; ; idx++ )
		{
			if ( oCommandEntries[idx].cmdHandler == NULL )
				break;

			if ( strncmp( oCommandEntries[idx].pszCommand, (const char *)pszCommand, 4 ) == 0 )
			{
				(this->*oCommandEntries[idx].cmdHandler)( pszLine+4, length-6 );

				bFound = true;
				break;
			}
		}

		if ( !bFound )
		{
			m_pServer->AddError( ERROR_502, "" );		// Command not implemented.
		}

		// Consume the CRLF line.
		return scanPos;
	}
	else
	{
		// Means we need more data to process command
		return 0;
	}
}

uint32_t CSMTPCommand::HeloCommandHandler( const char *pszLine, uint32_t length )
{
	if ( length == 0 )
	{
		m_pServer->AddError( ERROR_501, "" );
		return 0;
	}

	if ( !CSMTPHelperFunctions::IsSpace( pszLine[0] ) )
	{
		m_pServer->AddError( ERROR_501, "" );
		return 0;
	}

	// Construct domain...
	CDomain *pNewDomain = new CDomain();

	if ( !pNewDomain->ParseDomain( pszLine+1, length-1 ) )
	{
		delete pNewDomain;

		m_pServer->AddError( ERROR_501, "" );
		return (0);
	}

	m_pServer->DoHelo( pNewDomain );

	return (length);
}

uint32_t CSMTPCommand::MailCommandHandler( const char *pszLine, uint32_t length )
{
	if ( length < 8 )
	{
		m_pServer->AddError( ERROR_501, "" );
		return 0;
	}

	if ( !CSMTPHelperFunctions::IsSpace( pszLine[0] ) || pszLine[1] != 'F' || pszLine[2] != 'R' || pszLine[3] != 'O' || pszLine[4] != 'M' || pszLine[5] != ':' || pszLine[6] != '<' )
	{
		m_pServer->AddError( ERROR_501, "" );
		return 0;
	}

	uint32_t startPos = 7;
	uint32_t endPos = 7;

	bool bFoundEnd = false;
	while ( endPos < length )
	{
		if ( pszLine[endPos] == '>' )
		{
			bFoundEnd = true;
			break;
		}

		endPos++;
	}

	if ( !bFoundEnd )
	{
		m_pServer->AddError( ERROR_501, "" );
		return 0;
	}

	// Now process it...
	CMailPath *pNewMailPath = new CMailPath();

	if ( !pNewMailPath->ParseMailPath( pszLine+startPos, endPos-startPos ) )
	{
		delete pNewMailPath;

		m_pServer->AddError( ERROR_501, "" );
		return (0);
	}

	m_pServer->DoMail( pNewMailPath );

	return (length);
}

uint32_t CSMTPCommand::RcptCommandHandler( const char *pszLine, uint32_t length )
{
	if ( length < 6 )
	{
		m_pServer->AddError( ERROR_501, "" );
		return 0;
	}

	if ( !CSMTPHelperFunctions::IsSpace( pszLine[0] ) || pszLine[1] != 'T' || pszLine[2] != 'O' || pszLine[3] != ':' || pszLine[4] != '<' )
	{
		m_pServer->AddError( ERROR_501, "" );
		return 0;
	}

	uint32_t startPos = 5;
	uint32_t endPos = 5;

	bool bFoundEnd = false;
	while ( endPos < length )
	{
		if ( pszLine[endPos] == '>' )
		{
			bFoundEnd = true;
			break;
		}

		endPos++;
	}

	if ( !bFoundEnd )
	{
		m_pServer->AddError( ERROR_501, "" );
		return 0;
	}

	// Now process it...
	CMailPath *pNewMailPath = new CMailPath();

	if ( !pNewMailPath->ParseMailPath( pszLine+startPos, endPos-startPos ) )
	{
		delete pNewMailPath;

		m_pServer->AddError( ERROR_501, "" );
		return (0);
	}

	m_pServer->DoRcpt( pNewMailPath );

	return (length);
}

uint32_t CSMTPCommand::DataCommandHandler( const char *pszLine, uint32_t length )
{
	if ( length != 0 )
	{
		m_pServer->AddError( ERROR_501, "" );
		return 0;
	}

	m_pServer->DoData( );
	return length;
}

uint32_t CSMTPCommand::RsetCommandHandler( const char *pszLine, uint32_t length )
{
	if ( length != 0 )
	{
		m_pServer->AddError( ERROR_501, "" );
		return 0;
	}

	m_pServer->DoReset( );
	return length;
}

uint32_t CSMTPCommand::SendCommandHandler( const char *pszLine, uint32_t length )
{
	m_pServer->AddError( 502, "" );
	return 0;
}

uint32_t CSMTPCommand::SomlCommandHandler( const char *pszLine, uint32_t length )
{
	m_pServer->AddError( 502, "" );
	return 0;
}

uint32_t CSMTPCommand::SamlCommandHandler( const char *pszLine, uint32_t length )
{
	m_pServer->AddError( 502, "" );
	return 0;
}

uint32_t CSMTPCommand::VrfyCommandHandler( const char *pszLine, uint32_t length )
{
	m_pServer->AddError( 502, "" );
	return 0;
}

uint32_t CSMTPCommand::ExpnCommandHandler( const char *pszLine, uint32_t length )
{
	m_pServer->AddError( 502, "" );
	return 0;
}

uint32_t CSMTPCommand::HelpCommandHandler( const char *pszLine, uint32_t length )
{
	m_pServer->AddResponse( 200, "Valid commands are: HELO, MAIL, RCPT, DATA, RSET, HELP, NOOP, QUIT, TURN" ); 
	return 0;
}

uint32_t CSMTPCommand::NoopCommandHandler( const char *pszLine, uint32_t length )
{
	if ( length != 0 )
	{
		m_pServer->AddError( ERROR_501, "" );
		return 0;
	}

	m_pServer->AddResponse( 200, "Ok" );
	return 0;
}

uint32_t CSMTPCommand::QuitCommandHandler( const char *pszLine, uint32_t length )
{
	if ( length != 0 )
	{
		m_pServer->AddError( ERROR_501, "" );
		return 0;
	}

	m_pServer->DoQuit( );
	return length;
}

uint32_t CSMTPCommand::TurnCommandHandler( const char *pszLine, uint32_t length )
{
	if ( length != 0 )
	{
		m_pServer->AddError( ERROR_501, "" );
		return 0;
	}

	m_pServer->DoTurn();

	return 0;
}
