#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ConsoleHeader.h"

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
		printf("[Console]You did not enter jms_in and jms_out pipe names.Exiting...\n");
		exit(1);
	}
	// printf("Console:\n\n");
	// printf("jms_in: %s\n",jms_in);
	// printf("jms_out: %s\n",jms_out);
	// printf("Operation file:%s\n\n",opfile);
	
/***********************************************************************************/
		
	/*Creating jms_in fifo in current directory*/
	if ( mkfifo(jms_out, 0666) == -1 ){
		if ( errno!=EEXIST ) { perror("receiver: mkfifo"); exit(1); };
		}
	
	/*Opening both fifos*/
	if ( (out=open(jms_out, O_RDONLY  )) < 0)
	{
		perror("jms_out open problem[console]"); exit(3);	
	}
	if ( (in=open(jms_in, O_WRONLY )) < 0)
	{
		perror("jms_in open problem[console]"); exit(2);	
	}
	
/***********************************************************************************/	
	
	
	/*Use operations file*/
	if(opfile!=NULL)
	{
		FILE* fp;
		if(!(fp=fopen(opfile,"r")))
		{
			perror("[Error] Opening Operation File");
			exit(2);
		}
		else
		{
			retval = console_communication(fp,in,out);
			fclose(fp);
			free(opfile);
		}
	}
	
	/*Use stdin[prompt]*/
	// if(retval==0)
	// console_communication(stdin,in,out);

/***********************************************************************************/

	/*Closing FIFOs*/
	if( close(in)==-1)
	{
		perror("Closing jms_in[console]");
		exit(6);
	}
	
	if( close(out)==-1)
	{
		perror("Closing jms_out[console]");
		exit(7);
	}
	free(jms_in);
	free(jms_out);
	// printf("Console is exiting...\n");
	exit(0);
}