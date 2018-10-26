#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "CoordHeader.h"
#include "MiscHeader.h"


/*These variable are declared in coord_header.h*/
struct entry* pool_table;
int jobs_pool;
int finished = 0;
int created = 0;
int pools =0;
int jobs_sent = 0;
int jobs_served = 0;
char* path =NULL;


int main(int argc,char* argv[])
{
	int i;
	int in;
	int out;
	char* jms_in=NULL;
	char* jms_out=NULL;
	char* console_exit = malloc(MSGSIZE*sizeof(char));
	strcpy(console_exit,"NO");
	
	/*scanning command line arguments*/
	for(i=1;i<argc;++i)
	{
		if(!strcmp("-l",argv[i]))
		{
			path = malloc((strlen(argv[i+1])+1)* sizeof(char)   );
			strcpy(path,argv[i+1]);
		}		
		else if(!strcmp("-n",argv[i]))
		{
			jobs_pool = atoi(argv[i+1]);
		}	
		else if(!strcmp("-w",argv[i]))
		{
			jms_out = malloc((strlen(argv[i+1])+1)* sizeof(char)   );
			strcpy(jms_out,argv[i+1]);
		}
		else if(!strcmp("-r",argv[i]))
		{
			jms_in= malloc((strlen(argv[i+1])+1)* sizeof(char)   );
			strcpy(jms_in,argv[i+1]);
		}
	}
	
	if(!path || !jms_in || !jms_out || !jobs_pool)
	{
		printf("[Coordinator] You did not enter all of the arguments needed.Exiting...\n");
		exit(1);
	}
	
	// printf("Coordinator:\n");
	// printf("Path: %s\n",path);
	// printf("jms_in: %s\n",jms_in);
	// printf("jms_out: %s\n\n",jms_out);
/***********************************************************************************/	
	/*Creating jms_in fifo*/
	if ( mkfifo(jms_in, 0666) == -1 ){
		if ( errno!=EEXIST ) { perror("coordinator: mkfifo"); exit(1); };
		}
	
	/*Opening jms_in [NO BLOCK , NO ERROR]*/
	if ( (in=open(jms_in, O_RDONLY|O_NONBLOCK)) == -1)
	{
		perror("jms_in open problem[coordinator]"); exit(2);	
	}
	
	/*Opening jms_out [loop until jms_out has been running by the console]*/
	while((out=open(jms_out, O_WRONLY)) == -1);
	
	
/***********************************************************************************/
	coord_communication(in,out);
	
	/*Send terminating message to console*/
	if (write(out,"END_OF_EVERY_POOL", MSGSIZE) == -1)
	{ perror("Coord Writing to console "); exit(-5); }
	
	
	/*Wait for console to exit first*/
	while(strcmp(console_exit,"YES"))read(in,console_exit,MSGSIZE);

	
	free(jms_in);
	free(jms_out);
	free(console_exit);
	// printf("Coordinator is exiting...\n");
	exit(0);
}