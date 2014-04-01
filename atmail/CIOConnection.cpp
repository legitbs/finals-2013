#include "Common.h"	

int32_t CIOStdioConnection::ReadLine( uint8_t *pBuffer, uint32_t maxLen )
{
	uint32_t linePos = 0;
	unsigned char curChar;
	int readCount;

	for ( ; linePos < maxLen; linePos++ )
	{
		readCount = read( 0, &curChar, 1 );
		
		if ( readCount != 1 )
		{
			exit(1);
			break;
		}
 
		pBuffer[linePos] = curChar & 0x7F;

		if ( curChar == '\r' )
		{
			linePos++;
			if ( linePos < maxLen )
			{
				readCount = read( 0, &curChar, 1 );

				if ( readCount != 1 )
				{
					exit(1);
					break;
				}

				pBuffer[linePos] = curChar & 0x7F;

				if ( curChar == '\n' )
				{
					linePos++;
					break;
				}
			}
		}

#if DEBUG_INPUT_HELPER
		if ( curChar == '\n' )
		{
			pBuffer[linePos++] = '\r';
			pBuffer[linePos++] = '\n';
			pBuffer[linePos] = '\0';
			printf( "%s", pBuffer );
			break;
		}

		if ( curChar == '\\' )
		{
			curChar = fgetc( stdin );	
			
			if ( curChar == '\\' )
			{
				pBuffer[linePos] = '\\';
			}
			else if ( curChar == '0' )
			{
				pBuffer[linePos] = '\0';
			}
			else if ( curChar == 'x' )
			{
				curChar = fgetc( stdin );
				uint8_t hexValue = 0;
	
				if ( (curChar >= 'A' && curChar <= 'F') )
					hexValue = (curChar - 'A') << 4;	
				else if ( (curChar >= 'a' && curChar <= 'f') )
					hexValue = (curChar - 'a') << 4;
				else if ( (curChar >= '0' && curChar <= '9') )
					hexValue = (curChar - '0') << 4;
				else
					pBuffer[linePos] = 'x';
				
				curChar = fgetc( stdin );
				
				if ( (curChar >= 'A' && curChar <= 'F') )
					hexValue |= (curChar - 'A');	
				else if ( (curChar >= 'a' && curChar <= 'f') )
					hexValue |= (curChar - 'a');
				else if ( (curChar >= '0' && curChar <= '9') )
					hexValue |= (curChar - '0');
				else
					pBuffer[linePos] = 'x';

				pBuffer[linePos] = hexValue;

				printf( "HEX[%d]\n", hexValue );
			}
		}
#endif
	}

	pBuffer[linePos] = '\0';

	alarm( MAX_IDLE_SECS );	

	return (linePos);
}
	
int32_t CIOStdioConnection::WriteLine( uint8_t *pBuffer, uint32_t maxLen )
{
	return write( 1, pBuffer, maxLen );
}



int32_t CIOFileConnection::ReadLine( uint8_t *pBuffer, uint32_t maxLen )
{
	uint32_t linePos = 0;

	for ( ; linePos < maxLen; linePos++ )
	{
		int curChar = fgetc( m_pFile );
		
		if ( curChar == EOF )
			break;

		pBuffer[linePos] = curChar & 0x7F;

		if ( curChar == '\r' )
		{
			linePos++;
			if ( linePos < maxLen )
			{
				curChar = fgetc( m_pFile );
		
				if ( curChar == EOF )
					break;

				pBuffer[linePos] = curChar & 0x7F;

				if ( curChar == '\n' )
					break;
			}
		}
	}

	return (linePos);
}
	
int32_t CIOFileConnection::WriteLine( uint8_t *pBuffer, uint32_t maxLen )
{
	return fwrite( pBuffer, 1, maxLen, m_pFile );
}
