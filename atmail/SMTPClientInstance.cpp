#include "Common.h"

bool CSMTPClientInstance::SendRelayMessageFromFile( const char *pszFileName )
{
	// Check mode
	if ( m_state != STATE_DOMAIL )
	{
		if ( m_state == STATE_DOHELO )
		{
			SendHelo();
		}
		else
		{
			// It is in MAIL somewhere... send reset
			SendReset();
		}
	}

	// Now check state
	if ( m_state != STATE_DOMAIL )
	{
		m_state = STATE_ERROR;
		return false;
	}

	// Open file for relay...
	FILE *pFile = fopen( pszFileName, "r" );

	if ( !pFile )
	{
		m_state = STATE_ERROR;
		return false;
	}

	CIOFileConnection oFileConnection;

	oFileConnection.SetFile( pFile );

	for (;;)
	{

		// Now read in the file and send it
		char szLineBuffer[1024];
		char szResponseMessage[1024];
		uint32_t readLen = 0;
		uint32_t responseCode = 0;

		// Read in a line and send it
		readLen = oFileConnection.ReadLine( (uint8_t*)szLineBuffer, 1024 );

		if ( readLen == 0 )
			break;

		m_pIOConnection->WriteLine( (uint8_t*)szLineBuffer, readLen );

		// Now read response
		m_pIOConnection->ReadLine( (uint8_t*)szLineBuffer, 1024 );

		// Parse response
		sscanf( szLineBuffer, "%d %s\r\n", &responseCode, szResponseMessage );

		if ( responseCode == 354 )
			break;

		if ( responseCode != 250 )
		{
			m_state = STATE_ERROR;
			SendReset();
			return false;
		}
	}

	// DATA MODE
	for (;;)
	{
		// Now read in the file and send it
		char szLineBuffer[1024];
		uint32_t readLen = 0;

		// Read in a line and send it
		readLen = oFileConnection.ReadLine( (uint8_t*)szLineBuffer, 1024 );

		if ( readLen == 0 )
			break;

		m_pIOConnection->WriteLine( (uint8_t*)szLineBuffer, readLen );
	}

	// Close the file
	fclose( pFile );

	// Now read the response
	{
		char szLineBuffer[1024];
		char szResponseMessage[1024];
		uint32_t readLen = 0;
		uint32_t responseCode = 0;

		// Now read response
		m_pIOConnection->ReadLine( (uint8_t*)szLineBuffer, 1024 );

		// Parse response
		sscanf( szLineBuffer, "%d %s\r\n", &responseCode, szResponseMessage );

		if ( responseCode != 250 )
		{
			m_state = STATE_ERROR;
			SendReset();
			return false;
		}

	}

	return true;
}

bool CSMTPClientInstance::Connect( void )
{
	char szLineBuffer[1024];
	char szDomain[1024];
	char szResponseMessage[1024];
	uint32_t readLen = 0;
	uint32_t responseCode = 0;

	// Wait for server response
	m_pIOConnection->ReadLine( (uint8_t*)szLineBuffer, 1024 );

	// Parse response
	sscanf( szLineBuffer, "%d %s %s\r\n", &responseCode, szDomain, szResponseMessage );

	if ( responseCode != 220 )
	{
		m_state = STATE_ERROR;
		return false;
	}

	SetDomain( szDomain );

	// Advance state
	m_state = STATE_DOHELO;

	return true;
}

bool CSMTPClientInstance::SendHelo( void )
{
	// Check I/O
	if ( m_pIOConnection == NULL )
	{
		exit(1);
	}

	// Check state
	if ( m_state != STATE_DOHELO || m_pszDomain == NULL )
	{
		m_state = STATE_ERROR;
		return false;
	}


	// Send Helo
	char szDomainString[1024];

	sprintf( szDomainString, "HELO %s\r\n", m_pszDomain );

	// Write HELO
	m_pIOConnection->WriteLine( (uint8_t*)szDomainString, strlen( szDomainString ) );

	// Now await response...
	char szResponseString[1025];
	uint32_t readLen;
	uint32_t responseCode;
	char szResponseMessage[1024];

	readLen = m_pIOConnection->ReadLine( (uint8_t*)szResponseString, 1024 );
	szResponseString[readLen] = '\0';

	sscanf( szResponseString, "%d %s\r\n", &responseCode, szResponseMessage );

	if ( responseCode != 250 )
	{
		m_state = STATE_ERROR;
		return false;
	}

	// Ready to send MAIL
	m_state = STATE_DOMAIL;

	return true;
}

bool CSMTPClientInstance::SendReset( void )
{
	// Check I/O
	if ( m_pIOConnection == NULL )
	{
		exit(1);
	}

	// Check state
	if ( m_state == STATE_DOHELO )
	{
		return SendHelo();
	}

	// Write RESET
	m_pIOConnection->WriteLine( (uint8_t*)"RSET\r\n", strlen( "RSET\r\n" ) );

	// Now await response...
	char szResponseString[1025];
	uint32_t readLen;
	uint32_t responseCode;
	char szResponseMessage[1024];

	readLen = m_pIOConnection->ReadLine( (uint8_t*)szResponseString, 1024 );
	szResponseString[readLen] = '\0';

	sscanf( szResponseString, "%d %s\r\n", &responseCode, szResponseMessage );

	if ( responseCode != 250 )
	{
		m_state = STATE_ERROR;
		return false;
	}

	// Ready to send MAIL
	m_state = STATE_DOMAIL;

	// Success
	return true;
}

bool CSMTPClientInstance::SendTurn( void )
{
	// Check I/O
	if ( m_pIOConnection == NULL )
	{
		exit(1);
	}

	// Write TURN
	m_pIOConnection->WriteLine( (uint8_t*)"TURN\r\n", strlen( "TURN\r\n" ) );

	// Now await response...
	char szResponseString[1025];
	uint32_t readLen;
	uint32_t responseCode;
	char szResponseMessage[1024];

	readLen = m_pIOConnection->ReadLine( (uint8_t*)szResponseString, 1024 );
	szResponseString[readLen] = '\0';

	sscanf( szResponseString, "%d %s\r\n", &responseCode, szResponseMessage );

	if ( responseCode != 250 )
	{
		m_state = STATE_ERROR;
		return false;
	}

	return true;
}