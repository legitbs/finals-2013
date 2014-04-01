#include "Common.h"

CMailData::CMailData( )
	: m_dataSize( 0 )
{

}

CMailData::CMailData( CMailData &oRHS )
{
	CMailDataChunk *pCur;
	m_dataSize = 0;

	for ( pCur = oRHS.m_mailDataChunkList.Head(); pCur; pCur = oRHS.m_mailDataChunkList.Next( pCur ) )
	{
		m_mailDataChunkList.InsertTail( new CMailDataChunk( pCur->GetData(), pCur->GetChunkSize() ) );

		m_dataSize += pCur->GetChunkSize();
	}
}

CMailData::~CMailData( )
{
	m_mailDataChunkList.DeleteAll();
}

void CMailData::AddDataEnd( uint8_t *pData, uint32_t dataLen )
{
	CMailDataChunk *pNewChunk = new CMailDataChunk( pData, dataLen );

	m_mailDataChunkList.InsertTail( pNewChunk );

	m_dataSize += dataLen;
}

void CMailData::AddDataStart( uint8_t *pData, uint32_t dataLen )
{
	CMailDataChunk *pNewChunk = new CMailDataChunk( pData, dataLen );

	m_mailDataChunkList.InsertHead( pNewChunk );

	m_dataSize += dataLen;
}

void CMailData::InsertData( uint8_t *pData, uint32_t dataLen, uint32_t pos )
{
	// Search for position...
	CMailDataChunk *pCur = m_mailDataChunkList.Head();
	CMailDataChunk *pPrev = pCur;

	uint32_t curPos = 0;
	uint32_t lastPos = 0;

	for ( ; pPrev; )
	{
		if ( curPos >= pos )
		{
			// Insert somewhere after previous...

			if ( pos == lastPos )
			{
				// This is easy... we need a new chunk between the previous and current chunks!
				CMailDataChunk *pNewChunk = new CMailDataChunk( pData, dataLen );

				m_mailDataChunkList.InsertBefore( pCur, pNewChunk );

				m_dataSize += dataLen;
			}
			else
			{
				uint32_t lowerChunkSize = (pos - lastPos);
				uint32_t middleChunkSize = dataLen;
				uint32_t upperChunkSize = (pPrev->GetChunkSize() - lowerChunkSize);

				// Remove previous chunk
				CMailDataChunk *pLowChunk = new CMailDataChunk( pPrev->GetData(), lowerChunkSize );
				CMailDataChunk *pMidChunk = new CMailDataChunk( pData, dataLen );
				CMailDataChunk *pHighChunk = new CMailDataChunk( (pPrev->GetData()+lowerChunkSize), upperChunkSize );

				// This will unlink it
				delete pPrev;

				// Now add...
				m_mailDataChunkList.InsertBefore( pLowChunk, pCur );
				m_mailDataChunkList.InsertBefore( pMidChunk, pCur );
				m_mailDataChunkList.InsertBefore( pHighChunk, pCur );

				// Update data size
				m_dataSize += dataLen;
			}

			break;
		}

		// Remember previous chunk
		pPrev = pCur;
		lastPos = curPos;

		// Update current position.
		curPos += pCur->GetChunkSize();

		// Update
		if ( pCur )
			pCur = m_mailDataChunkList.Next( pCur );
	}
}

void CMailData::ClearData( void )
{
	m_mailDataChunkList.DeleteAll();

	m_dataSize = 0;
}

uint32_t CMailData::GetDataAt( uint8_t *pData, uint32_t maxLen, uint32_t pos )
{
	// Search for position...
	CMailDataChunk *pCur = m_mailDataChunkList.Head();
	CMailDataChunk *pPrev = pCur;

	uint32_t chunkPos = 0;
	uint32_t lastChunkPos = 0;
	uint32_t amountCopied = 0;

	for ( ; pCur; pCur = m_mailDataChunkList.Next( pCur ) )
	{
		if ( chunkPos >= pos )
			break;

		// Remember previous chunk
		pPrev = pCur;
		lastChunkPos = chunkPos;

		// Update current position.
		chunkPos += pCur->GetChunkSize();
	}

	if ( pCur == NULL || pPrev == NULL )
		return 0;	// NOT FOUND

	// Start copying
	pCur = pPrev;
	chunkPos = lastChunkPos;

	for ( ; pCur && amountCopied < maxLen; pCur = m_mailDataChunkList.Next( pCur ) )
	{
		// start copy offset
		uint32_t startChunkOffset = (pos - chunkPos);
		uint32_t copyAmount = (pCur->GetChunkSize() - startChunkOffset);

		// Constrain copy
		if ( copyAmount > maxLen )
		{
			copyAmount = maxLen;
		}

		// Remove the amount copied from the maxLen
		maxLen -= copyAmount;

		// Copy data
		memcpy( pData+amountCopied, (pCur->GetData()+startChunkOffset), copyAmount );

		// Update copy amount
		amountCopied += copyAmount;

		// Update position
		pos += copyAmount;

		// Update chunk position
		chunkPos += pCur->GetChunkSize();
	}

	return (amountCopied);
}