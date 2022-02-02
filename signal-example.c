/* Written by Moe */
#include <signal.h> /* For Ctrl-C handling */
#include <unistd.h> /* For usleep */
#include <stdio.h>
#include <stdlib.h> /* For exit() */
#include <errno.h>
#include "tdcbase.h"

#define COINC_WINDOW 2000 /*Coincidence window in ps */
#define CHAN_MASK 1 /* Channel mask for enableChannels */
#define INTEGRATION_TIME 100 /* Integration time for coincidence counters in ms */

/* For SIGINT handling. When Ctrl-C is pressed, call TDC_deInit before quitting */
static volatile int keeprunning = 1;
static void handler(int sig);

/* For checking TDC errors. Copied from example0.c */
static void checkRc( const char * fctname, int rc );

/* Main function prints out coincidence between channels (including singles) */
int main(int agrc, char **argv)
{
	/* Setup signal handler for SIGINT */
	struct sigaction sa;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
	/* Device setup */
	int rc = TDC_init( -1 ); /* Connect to any device */
	checkRc( "TDC_init", rc );
	rc = TDC_enableChannels( 1, CHAN_MASK ); /* Enable some channels */
	checkRc( "TDC_enableChannels", rc );
	/* Default signal conditioning seems sufficient. If not, set here. */
	//rc = TDC_configureSignalConditioning( 0, SCOND_MISC, 1, 1 );	
	//checkRc( "TDC_configureSignalConditioning", rc );
	rc = TDC_setCoincidenceWindow( COINC_WINDOW ); /* Set some coincidence window */
	checkRc( "TDC_setCoincidenceWindow", rc );
	rc = TDC_setExposureTime( INTEGRATION_TIME ); /* Set integration time for internal coincidence counter */
	checkRc( "TDC_setExposureTime", rc );
	
	/* Setup array for the coincidence counter results */
	int coincCnt[TDC_COINC_CHANNELS];
	int i; /* for printing the coincidence data */

	while (keeprunning)
	{
		/* Give the device some time to collect data. Ideally, this should be some select call. Something to improve here. */
		usleep( INTEGRATION_TIME * 1000 );
		TDC_getCoincCounters( coincCnt, NULL );
		for ( i = 0; i < TDC_COINC_CHANNELS; i++ )
		{
			printf( "%d ", coincCnt[i] );
		}
		printf("\n");
	}
	/* Stop the device and exit */
	TDC_deInit();
	exit(EXIT_SUCCESS);
}
static void handler(int sig)
{
	keeprunning = 0;
}
static void checkRc( const char * fctname, int rc )
{
	/* Quit on any error */
	if ( rc )
	{
		if ( rc != TDC_NotConnected ){
		printf(">>> %s: %s\n", fctname, TDC_perror( rc ));
		TDC_deInit();
		exit(EXIT_FAILURE);
		}
	}
}
