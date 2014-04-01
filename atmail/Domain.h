#ifndef __DOMAIN_H__
#define __DOMAIN_H__

#define MAX_DOMAIN_NAME_SIZE	128

class CDomain
{
public:
	CDomain();
	~CDomain();

public:
	typedef enum
	{
		DOMAIN_TYPE_IPV4,
		DOMAIN_TYPE_NAME,
		DOMAIN_TYPE_NUMBER,
		DOMAIN_TYPE_UNKNOWN
	} eDomainType;

	bool ParseDomain( const char *pszString, uint32_t length );

	char *GetString( char *buf, uint32_t len );

private:
	typedef struct
	{
		uint8_t ip[4];
	} tIPV4;

	typedef struct
	{
		char pszName[MAX_DOMAIN_NAME_SIZE];
	} tDomainName;

	typedef struct
	{
		union
		{
			tIPV4			ipV4;
			tDomainName		domainName;
		} data;
	} tDomainInfo;

	tDomainInfo m_domainInfo;
	eDomainType m_domainType;
};

#endif __DOMAIN_H__
