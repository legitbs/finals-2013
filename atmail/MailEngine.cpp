#include "Common.h"

CMailEngine g_oMailEngine;

CMailPacket::CMailPacket()
	: m_pMailData( NULL ), m_pFromDomain( NULL ), m_pReversePath( NULL ), m_pForwardPathList( NULL ), m_forwardPathListCount( 0 )
{

}

CMailPacket::~CMailPacket()
{
	if ( m_pMailData )
		delete m_pMailData;

	if ( m_pReversePath )
		delete m_pReversePath;

	if ( m_pForwardPathList )
	{
		for ( uint32_t i = 0; i < m_forwardPathListCount; i++ )
			delete m_pForwardPathList[i];

		delete [] m_pForwardPathList;
	}

}

CMailPacket *CMailPacket::GenerateMailPacket( CMailData *pMailData, CDomain *pDomain, CMailPath *pReversePath, NUtil::LIST_DECLARE( CForwardPath, m_forwardPathLink ) *pForwardPathList )
{
	if ( pMailData == NULL )
		return (NULL);

	if ( pDomain == NULL )
		return (NULL);

	if ( pReversePath == NULL )
		return (NULL);

	if ( pForwardPathList == NULL )
		return NULL;

	CMailPacket *pNewMailPacket = new CMailPacket();

	pNewMailPacket->m_pMailData = pMailData;

	// Set domain
	pNewMailPacket->m_pFromDomain = pDomain;

	// Set reverse path...
	pNewMailPacket->m_pReversePath = pReversePath;

	// Create an array out of the forward path list
	CForwardPath *pCur;

	uint32_t count = 0;
	for ( pCur = pForwardPathList->Head(); pCur; pCur = pForwardPathList->Next( pCur ) )
		count++;

	pNewMailPacket->m_pForwardPathList = new CMailPath*[count];
	pNewMailPacket->m_forwardPathListCount = count;
	
	if ( pNewMailPacket->m_pForwardPathList  == NULL )
	{
		// OUT OF MEMORY
		delete pNewMailPacket;

		return (NULL);
	}

	// Now copy over
	count = 0;
	for ( pCur = pForwardPathList->Head(); pCur; pCur = pForwardPathList->Next( pCur ) )
	{
		// Store it.
		pNewMailPacket->m_pForwardPathList[count] = (pCur->GetMailPath());
		
		// Clear them as we are the new owner.
		pCur->ClearMailPath();

		// Update count
		count++;
	}

	return (pNewMailPacket);
}

CMailEngine::CMailEngine()
	: m_relayCount( 0 ), m_pSpamFilter(NULL), m_pIOConnection( NULL ), m_state( STATE_SERVER )
{
	strcpy( m_szDomain, "legitbs.net" );

	LoadUsers();
}

CMailEngine::~CMailEngine()
{
	m_userMailBoxList.DeleteAll();
	m_relayMailList.DeleteAll();
}

bool CMailEngine::RecvMail( CSMTPServerInstance *pServer, CMailPacket *pMailPacket )
{
	// Check recipients...
	bool bFoundGoodRecipient = false;

	uint32_t recipientCount = pMailPacket->GetRecipientListSize();

	for ( uint32_t idx = 0; idx < recipientCount; idx++ )
	{
		// Process the mail per recipient!

		// Check domain
		CMailPath *pRecipientPath = pMailPacket->GetRecipientAt( idx );

		// Check for message relay
		if ( pRecipientPath->GetAtDomainCount() > 0 )
		{
			// Process relay
			CMailData oTempData( *(pMailPacket->GetMailData()) );

			DoRelayMessage( &oTempData, pMailPacket->GetReversePath(), pRecipientPath, pMailPacket->GetDomain() );

			bFoundGoodRecipient = true;
			continue;
		}

		char szRecipientDomain[256];

		pRecipientPath->GetMailBoxDomain( szRecipientDomain, 256 );

		if ( strcmp( szRecipientDomain, m_szDomain ) != 0 )
		{
			// Bad DOMAIN 
			CMailData oTempData( *(pMailPacket->GetMailData()) );

			DoUndelivered( &oTempData, pMailPacket->GetReversePath(), pRecipientPath, pMailPacket->GetDomain() );
			continue;
		}

		// Check that this recipient has a mailbox
		char szRecipientName[256];
		
		pRecipientPath->GetMailBoxName( szRecipientName, 256 );

		CUserMailBox *pUserMailBox = GetMailBoxForRecipient( szRecipientName );

		// FOUND MAIL BOX:
		if ( pUserMailBox == NULL )
		{
			// Bad Mailbox
			CMailData oTempData( *(pMailPacket->GetMailData()) );

			DoUndelivered( &oTempData, pMailPacket->GetReversePath(), pRecipientPath, pMailPacket->GetDomain() );
			continue;
		}
		
		// This is the end of the mail message
		CMailData oTempData( *(pMailPacket->GetMailData()) );

		DoRecvMail( pUserMailBox, &oTempData, pMailPacket->GetReversePath(), pRecipientPath, pMailPacket->GetDomain() );

		bFoundGoodRecipient = true;	
	}

	// Done with mail packet... delete it
	delete pMailPacket;

	if ( bFoundGoodRecipient )
		return true;
	else
		return false;
}

CUserMailBox *CMailEngine::GetMailBoxForRecipient( const char *pszRecipientName )
{
	CUserMailBox *pCur;

	// Scan mailboxes for a valid recipient
	for ( pCur = m_userMailBoxList.Head(); pCur; pCur = m_userMailBoxList.Next( pCur ) )
	{
		if ( strcmp( pszRecipientName, pCur->GetRecipientName() ) == 0 )
			return pCur;
	}

	return (NULL);
}

char *CMailEngine::GetMailTimeStamp( char *pszBuffer, uint32_t len )
{
	static char *oDayTable[] = 
	{
		"Sun",
		"Mon",
		"Tue",
		"Wed",
		"Thu",
		"Fri",
		"Sat"
	};

	static char *oMonTable[] = 
	{
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec"
	};

	char szTimeBuf[512];
	time_t rawtime;
	struct tm * ptm;

	time ( &rawtime );

	ptm = localtime ( &rawtime );

#ifdef BUILD_LINUX_CODE
	char szTimeZone[20];
	
	if ( ptm->tm_gmtoff < 0 )
		sprintf( szTimeZone, "-%02d00", abs(ptm->tm_gmtoff) / (60 * 60) );
	else
		sprintf( szTimeZone, "+%02d00", ptm->tm_gmtoff / (60 * 60) );

#elif BUILD_WINDOWS_CODE
	char szTimeZone[] = "-0500";
#endif 

	//Fri, 26 Jul 2013 07:07:28 -0600
	sprintf( szTimeBuf, "%s, %d %s %d %02d:%02d:%02d %s", oDayTable[ptm->tm_wday], ptm->tm_mday, oMonTable[ptm->tm_mon], (1900+ptm->tm_year), ptm->tm_hour, ptm->tm_min, ptm->tm_sec, szTimeZone );

	if ( strlen( szTimeBuf ) >= len )
	{
		pszBuffer[0] = '\0';
	}
	else
	{
		strcpy( pszBuffer, szTimeBuf );
	}

	return pszBuffer;
}

void CMailEngine::GenerateReceivedFromLine( CMailData *pData, CDomain *pFromDomain )
{
	// We have a good mailbox... now tack on the timestamp and reverse path header to the data.
	char szRecvLine[512];
	char szFromBuffer[256];
	char szTimeStamp[128];

	// Generate Received: from line:
	sprintf( szRecvLine, "Received: from %s by %s ; %s\r\n", pFromDomain->GetString( szFromBuffer, 256 ), m_szDomain, GetMailTimeStamp( szTimeStamp, 128 ) );

	// Append received line to mail data...
	pData->AddDataStart( (uint8_t*)szRecvLine, strlen( szRecvLine ) );
}

void CMailEngine::GenerateReturnPathLine( CMailData *pMailData, CMailPath *pRecipient )
{
	// Generate Return-Path:
	char szReturnPathBuffer[1024];
	char szAtDomainBuffer[512];
	char szMailBoxName[256];
	char szMailBoxDomain[256];
	
	// Loop through the @ domain list in the MAIL FROM: portion of the message generating the the Return-Path: 
	if ( pRecipient->GetAtDomainCount() == 0 )
		sprintf( szReturnPathBuffer, "Return-Path: <%s@%s>\r\n", pRecipient->GetMailBoxName( szMailBoxName, 256 ), pRecipient->GetMailBoxDomain( szMailBoxDomain, 256 ) );  
	else
		sprintf( szReturnPathBuffer, "Return-Path: <%s:%s@%s>\r\n", pRecipient->GetAtDomainString( szAtDomainBuffer, 512 ), pRecipient->GetMailBoxName( szMailBoxName, 256 ), pRecipient->GetMailBoxDomain( szMailBoxDomain, 256 ) );  

	// Append to front
	pMailData->AddDataStart( (uint8_t*)szReturnPathBuffer, strlen( szReturnPathBuffer ) );
}

void CMailEngine::GenerateSpamGuardHeader( CMailData *pMailData )
{
	// Generate the spam guard header.
	// This actually leaks some data about the program (the stack specifically)
	// allowing the user to generate ROP code
	// BUG: TYPE: INFORMATION LEAK
	// BUG1:  LEAK IN THE RANDOM NUMBER GENERATION STACK ADDRESS
	// BUG2:  LEAK IN THE SECOND RANDOM NUMBER GENERATION THE ROP CODE ADDRESS
	
	uint32_t spamIDLow;
	char szLineBuf[512];

	if ( m_oRNG.GetU32( spamIDLow ) == CRNGCommon::RNG_NEED_SEED )
	{
		m_oRNG.Seed( (uint32_t)time(NULL) ^ (uint32_t)pMailData );
		m_oRNG.GetU32( spamIDLow );
	}
	

	sprintf( szLineBuf, "X-TrackerID: %d-%d-10\r\n", spamIDLow, time(NULL) );

	pMailData->AddDataStart( (uint8_t*)szLineBuf, strlen( szLineBuf ) );
}

void CMailEngine::DoRelayMessage( CMailData *pMailData, CMailPath *pReturnPath, CMailPath *pRecipient, CDomain *pFromDomain )
{
	// Append received from line
	GenerateReceivedFromLine( pMailData, pFromDomain );

	// Append X-Spam-Guard: header
	GenerateSpamGuardHeader( pMailData );

	// Queue message for relay.
	
	// Write out message to disk...
	char szLineBuf[1024];
	struct
	{
	char szFileName[256];
	char szMailBoxName[64];
	char szMailExtension[32];
	} oFileData;
	char szMailPath[512];
	
	uint32_t dataRead = 0;
	uint32_t pos = 0;

	sprintf( oFileData.szMailExtension, "%d.%d", m_relayCount, time(NULL) );

	sprintf( oFileData.szFileName, "%s%s%s", RELAY_MESSAGE_FOLDER, pRecipient->GetMailBoxName( oFileData.szMailBoxName, 64 ), oFileData.szMailExtension );

	
	// Sanitize filename...
	SanitizeFileName( oFileData.szFileName );

	// Update relay count
	m_relayCount++;

	// Add to relay list
	m_relayMailList.InsertTail( new CRelayMailContainer( oFileData.szFileName, pRecipient->GetMailBoxDomain( szMailPath, 512 ) ) );

	// Notice we do FILE I/O after we've inserted it into memory!
	// BUG: This will allow the user to attempt to overwrite the keyfile (tho it won't overwrite)
	// and get back the key file after using TURN and getting the relayed messages!	
	FILE *pFile = fopen( oFileData.szFileName, "w" );

	if ( !pFile )
	{
		printf( "Failed to open relay file.\n" );
		return;
	}

	// Now write out MAIL FROM and MAIL TO
	sprintf( szLineBuf, "MAIL FROM: <%s>\r\n", pReturnPath->GetString( szMailPath, 512 ) );

	fwrite( szLineBuf, 1, strlen( szLineBuf ), pFile );
	
	sprintf( szLineBuf, "RCPT TO: <%s>\r\n", pRecipient->GetString( szMailPath, 512 ) );

	fwrite( szLineBuf, 1, strlen( szLineBuf ), pFile );

	strcpy( szLineBuf, "DATA\r\n" );

	fwrite( szLineBuf, 1, strlen( szLineBuf ), pFile );

	// Write out data
	for (;;)
	{
		dataRead = pMailData->GetDataAt( (uint8_t*)szLineBuf, 1024, pos );

		if ( dataRead == 0 )
			break;

		// Write data out directly...
		fwrite( szLineBuf, 1, (dataRead), pFile );

		// Update pos
		pos += (dataRead);
	}

	strcpy( szLineBuf, "\r\n.\r\n" );

	fwrite( szLineBuf, 1, strlen( szLineBuf ), pFile );

	// Close
	fclose( pFile );

}

void CMailEngine::DoRecvMail( CUserMailBox *pMailBox, CMailData *pMailData, CMailPath *pReturnPath, CMailPath *pRecipient, CDomain *pFromDomain)
{
	// Append the received from line
	GenerateReceivedFromLine( pMailData, pFromDomain );

	// Append return path
	GenerateReturnPathLine( pMailData, pRecipient );

	// Genereate MBOX mail format ... add it to the end of the existing file
#ifdef BUILD_LINUX_CODE
	// Open file and lock goes here

	char szFileName[512];

	sprintf( szFileName, "%s%s%s", INBOX_MESSAGE_FOLDER, pMailBox->GetFolderPath(), MAILBOX_INBOX_NAME );

	FILE *pFile = fopen( szFileName, "w+" );

	if ( !pFile )
	{
		return;
	}

#elif BUILD_WINDOWS_CODE
	char szFileName[512];

	sprintf( szFileName, "%s%s%s", INBOX_MESSAGE_FOLDER, pMailBox->GetFolderPath(), MAILBOX_INBOX_NAME );

	FILE *pFile = fopen( szFileName, "w+" );

	if ( !pFile )
	{
		return;
	}

	// Now write the data to the .mbox file..
#endif 

	bool bDone = false;
	char szLineBuf[1024];
	char szFromDomain[512];
	char szTimeStamp[512];
	uint32_t pos = 0;
	uint32_t dataRead = 0;

	// Sanitize mail data
	SanitizeMailDataForMBOX( pMailData );

	// Seek to end of mbox file
	fseek( pFile, 0, SEEK_END );

	// Output from string
	sprintf( szLineBuf, "\nFrom %s %s\n", pFromDomain->GetString( szFromDomain, 512 ), GetMailTimeStamp( szTimeStamp, 512 ) );

	// Write out from line
	fwrite( szLineBuf, 1, strlen( szLineBuf ), pFile );

	// Write out data
	for (;;)
	{
		dataRead = pMailData->GetDataAt( (uint8_t*)szLineBuf, 1024, pos );

		if ( dataRead == 0 )
			break;

		// Find end of line.
		char *pEndOfLinePtr = strstr( szLineBuf, "\r\n" );

		if ( pEndOfLinePtr == NULL )
		{
			// Write data out directly...
			fwrite( szLineBuf, 1, (dataRead-1), pFile );

			// Update pos
			pos += (dataRead-1);
		}
		else
		{
			// Process the line
			dataRead = (pEndOfLinePtr-szLineBuf)+2;
			pos += dataRead;

			// Modify line to remove \r\n in message
			pEndOfLinePtr[0] = '\n';
			pEndOfLinePtr[1] = '\0';

			// Write data out directly but remove \n\r
			fwrite( szLineBuf, 1, (dataRead-1), pFile );
		}


	}

	// End.
	fwrite( "\n", 1, 1, pFile );

#ifdef BUILD_LINUX_CODE
	// TODO: Locking code goes here

	// Close mbox file
	fclose( pFile );
#elif BUILD_WINDOWS_CODE
	// Close mbox file
	fclose( pFile );
#endif 
}

void CMailEngine::DoUndelivered( CMailData *pMailData, CMailPath *pReturnPath, CMailPath *pRecipient, CDomain *pFromDomain )
{
	return;
}

void CMailEngine::SanitizeMailDataForMBOX( CMailData *pData )
{
	uint32_t dataPos = 0;
	uint32_t dataRead = 0;
	char szLineBuf[1024];

	for ( ; dataPos < pData->GetDataSize(); )
	{
		dataRead = pData->GetDataAt( (uint8_t*)szLineBuf, 1024, dataPos );

		if ( dataRead == 0 )
			break;

		char *pFoundPtr = strstr( szLineBuf, "From" );

		if ( pFoundPtr )
		{
			uint32_t foundPos = (pFoundPtr - szLineBuf);

			pData->InsertData( (uint8_t *)">", 1, foundPos );

			dataPos += (foundPos+5);

			continue;
		}
		
		pFoundPtr = strstr( szLineBuf, ">From" );

		if ( pFoundPtr )
		{
			uint32_t foundPos = (pFoundPtr - szLineBuf);

			pData->InsertData( (uint8_t *)">", 1, foundPos );

			dataPos += (foundPos+6);

			continue;
		}

		if ( dataRead < 4 )
			dataPos += dataRead;
		else
			dataPos += (dataRead-3); 
	}
}

char *CMailEngine::SanitizeFileName( char *pszInFileName )
{
	char *pszOutFileName = pszInFileName;

	for ( ; pszInFileName[0] != '\0'; pszInFileName++ )
	{
		// Skip non-printables
		if ( pszInFileName[0] < 32 || pszInFileName[0] > 128 )
			continue;

		pszOutFileName[0] = pszInFileName[0];

		pszOutFileName++;
	}

	pszOutFileName[0] = '\0';

	return pszOutFileName;
}

void CMailEngine::LoadUsers( void )
{
	m_userMailBoxList.InsertHead( new CUserMailBox( "joe", "joe/" ) );
}

void CMailEngine::LoadRelayMessages( void )
{
	// TODO: Load relay messages from disk.

}

void CMailEngine::DoRelay( void )
{
	// We have been sent a TURN command so we are now a relay...

	// Consume server response...
	CRelayMailContainer *pCur;
	CRelayMailContainer *pNext;
	CIOStdioConnection oStdio;
	CSMTPClientInstance oClient;

	oClient.AddIOConnection( &oStdio );

	if ( !oClient.Connect() )
		return;
	
	// Send relay messages
	for ( pCur = m_relayMailList.Head(); pCur; pCur = pNext )
	{
		// Save next in case of delete
		pNext = m_relayMailList.Next( pCur );

		// Now send messages
		if ( oClient.SendRelayMessageFromFile( pCur->GetFileName() ) )
		{
			delete pCur;
			m_relayCount--;
		}
	}

	// Issue turn to switch back
	oClient.SendTurn();

	// Switch back to server mode
	m_state = STATE_SERVER;
}

void CMailEngine::DoServer( void )
{
	CSMTPServerInstance oServerInstance( this );

	oServerInstance.AddIOConnection( m_pIOConnection );

	oServerInstance.Start( m_szDomain );

	for (;;)
	{
		oServerInstance.Run();

		if ( m_state != STATE_SERVER )
			break;
	}
}

void CMailEngine::DoRun( void )
{
	for (;;)
	{
		switch( m_state )
		{
		case STATE_SERVER:
			DoServer();
			break;

		case STATE_CLIENT:
			DoRelay();
			break;
		}
	}
}
