#ifndef __MAILDATA_H__
#define __MAILDATA_H__

class CMailDataChunk
{
public:
	CMailDataChunk( uint8_t *pData, uint32_t dataLen )
	{
		m_pData = new uint8_t[dataLen];

		memcpy( m_pData, pData, dataLen );

		m_chunkLen = dataLen;
	}

	~CMailDataChunk( )
	{
		if ( m_pData )
			delete m_pData;
	}

	uint32_t GetChunkSize( void ) const { return m_chunkLen; };
	uint8_t* GetData( void ) const { return m_pData; };

	NUtil::LIST_LINK( CMailDataChunk )	m_mailDataChunkLink;

private:
	uint8_t		*m_pData;
	uint32_t	m_chunkLen;
};

class CMailData
{
public:
	CMailData();
	CMailData( CMailData& oRHS );
	~CMailData();

	void AddDataStart( uint8_t *pData, uint32_t dataLen );
	void AddDataEnd( uint8_t *pData, uint32_t dataLen );
	void InsertData( uint8_t *pData, uint32_t dataLen, uint32_t pos );
	void ClearData( void );

	uint32_t GetDataAt( uint8_t *pData, uint32_t maxLen, uint32_t pos );

	uint32_t GetDataSize( void ) const { return m_dataSize; };

private:
	uint32_t	m_dataSize;
	NUtil::LIST_DECLARE( CMailDataChunk, m_mailDataChunkLink )	m_mailDataChunkList;
};

#endif // _MAILDATA_H__