#ifndef __SPAM_FILTER_H__
#define __SPAM_FILTER_H__

class CSpamFilter
{
public:
	CSpamFilter();
	~CSpamFilter();


private:
	bool IsHomoglyphCharacter( const char c );
	char GetBaseCharacter( const char c );
};

#endif // __SPAM_FILTER_H__