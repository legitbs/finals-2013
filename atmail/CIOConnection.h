#ifndef __CIO_CONNECTION_H__
#define __CIO_CONNECTION_H__

class CIOConnection
{
public:

	virtual int32_t ReadLine( uint8_t *pBuffer, uint32_t maxLen ) = 0;
	virtual int32_t WriteLine( uint8_t *pBuffer, uint32_t maxLen ) = 0;

private:
};

class CIOStdioConnection : public CIOConnection
{
public:

	int32_t ReadLine( uint8_t *pBuffer, uint32_t maxLen );
	int32_t WriteLine( uint8_t *pBuffer, uint32_t maxLen );
};

class CIOFileConnection : public CIOConnection
{
public:
	CIOFileConnection() : m_pFile(NULL) { }
	~CIOFileConnection() { }

	void SetFile( FILE *pFile )
	{
		m_pFile = pFile;
	}

	int32_t ReadLine( uint8_t *pBuffer, uint32_t maxLen );
	int32_t WriteLine( uint8_t *pBuffer, uint32_t maxLen );

private:
	FILE *m_pFile;
};

#endif // __CIO_CONNECTION_H__
