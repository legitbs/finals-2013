#ifndef __PARSER_H__
#define __PARSER_H__

class CSMTPCommand
{
public:
	CSMTPCommand( CSMTPServerInstance *pServer );
	~CSMTPCommand();

	uint32_t ParseInput( const char *pszLine, uint32_t length );

private:
	typedef uint32_t (CSMTPCommand::*fpCommandParser)( const char *pszLineRemaining, uint32_t length );

	typedef struct
	{
		char *pszCommand;
		fpCommandParser	cmdHandler;
	} tCommandEntry;

	CSMTPServerInstance *m_pServer;

private:

	// Command Handlers...
	uint32_t HeloCommandHandler( const char *pszLine, uint32_t length );
	uint32_t MailCommandHandler( const char *pszLine, uint32_t length );
	uint32_t RcptCommandHandler( const char *pszLine, uint32_t length );
	uint32_t DataCommandHandler( const char *pszLine, uint32_t length );
	uint32_t RsetCommandHandler( const char *pszLine, uint32_t length );
	uint32_t SendCommandHandler( const char *pszLine, uint32_t length );
	uint32_t SomlCommandHandler( const char *pszLine, uint32_t length );
	uint32_t SamlCommandHandler( const char *pszLine, uint32_t length );
	uint32_t VrfyCommandHandler( const char *pszLine, uint32_t length );
	uint32_t ExpnCommandHandler( const char *pszLine, uint32_t length );
	uint32_t HelpCommandHandler( const char *pszLine, uint32_t length );
	uint32_t NoopCommandHandler( const char *pszLine, uint32_t length );
	uint32_t QuitCommandHandler( const char *pszLine, uint32_t length );
	uint32_t TurnCommandHandler( const char *pszLine, uint32_t length );

};

#endif // __PARSER_H__