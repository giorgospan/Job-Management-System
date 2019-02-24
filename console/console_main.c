#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "console_header.h"

int main(int argc,char* argv[])
{

	int i;
	int in;
	int out;
	int retval;

	char* jms_in=NULL;
	char* jms_out=NULL;
	char* opfile=NULL;

	/*scanning command line arguments*/
	for(i=1;i<argc;++i)
	{
		if(!strcmp("-w",argv[i]))
		{
			jms_in = malloc((strlen(argv[i+1])+1)* sizeof(char)   );
			strcpy(jms_in,argv[i+1]);
		}
		else if(!strcmp("-r",argv[i]))
		{
			jms_out = malloc((strlen(argv[i+1])+1)* sizeof(char)   );
			strcpy(jms_out,argv[i+1]);
		}
		else if(!strcmp("-o",argv[i]))
		{
			opfile = malloc((strlen(argv[i+1])+1)* sizeof(char)   );
			strcpy(opfile,argv[i+1]);
		}
	}
	/*Sanity check*/
	if(!jms_in || !jms_out)
	{
		fprintf(stderr,"Usage: ./jms_console -w <jms-in> -r <jms-out> -o <operations-file>\n\n");
		exit(EXIT_FAILURE);
	}

/***********************************************************************************/

	/*Creating jms_in fifo in current directory*/
	if ( mkfifo(jms_out, 0666) == -1 ){
		if ( errno!=EEXIST ) { perror("receiver: mkfifo"); exit(EXIT_FAILURE); };
		}

	/*Opening both fifos*/
	if ( (out=open(jms_out, O_RDONLY  )) < 0)
	{
		perror("jms_out open problem[console]"); exit(EXIT_FAILURE);
	}
	if ( (in=open(jms_in, O_WRONLY )) < 0)
	{
		perror("jms_in open problem[console]"); exit(EXIT_FAILURE);
	}

/***********************************************************************************/


	/*Use operations file*/
	if(opfile!=NULL)
	{
		FILE* fp;
		if(!(fp=fopen(opfile,"r")))
		{
			perror("[Error] Opening Operation File");
			exit(EXIT_FAILURE);
		}
		else
		{
			retval = console_communication(fp,in,out);
			fclose(fp);
			free(opfile);
		}
	}

	/*Use stdin[prompt]*/
	if(retval==0)
		console_communication(stdin,in,out);

/***********************************************************************************/

	/*Closing FIFOs*/
	if( close(in)==-1)
	{
		perror("Closing jms_in[console]");
		exit(EXIT_FAILURE);
	}

	if( close(out)==-1)
	{
		perror("Closing jms_out[console]");
		exit(EXIT_FAILURE);
	}
	free(jms_in);
	free(jms_out);
	// printf("Console is exiting...\n");
	exit(0);
}