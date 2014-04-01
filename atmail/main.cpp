#include "Common.h"

void sig_alarm_handler( int signum )
{
	// Send timeout response
	printf( "421 Connection timeout\r\n" );

	// Shutdown
	exit(1);
}

int main( int argc, void **argv )
{
	chdir( "/home/atmail/" );

	// Disable buffer on stdout
        setvbuf(stdout, NULL, _IONBF, 0);

	// Turn on signal alarm
	signal( SIGALRM, sig_alarm_handler );
	alarm( MAX_IDLE_SECS );


	CMailEngine oEngine;
	CIOStdioConnection oConnection;

	oEngine.SetIOConnection( &oConnection );

	oEngine.DoRun();
}
