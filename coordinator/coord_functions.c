#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>/*for flags in open() system call*/
#include <sys/types.h> /*for creating named pipe*/
#include <sys/wait.h> /*for wait*/
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

#include "coord_header.h"
#include "coord_operations.h"
#include "misc_header.h"


void coord_communication(int in,int out)
{
	char* operation = malloc(MSGSIZE*sizeof(char));
	char* msg       = malloc(RESPONSESIZE*sizeof(char));
	memset(operation,0,MSGSIZE);
	memset(msg,0,RESPONSESIZE);


	int nwrite;
	int nread;
	int status;
	int i;
	int no_op_left      = 0;
	int send_counter    = 0;
	int receive_counter = 0;
	pid_t wpid;

	strcpy(operation,"START");

/***********************************************************************************/

	/*Loop until operation is "shutdown"*/
	while(strcmp(operation,"shutdown"))
	{

		/*Read a new operation from console*/
		if(  (nread=read(in,operation,MSGSIZE))>0 )
		{
			/*Execute the given command and fetch response msg */
			execute_operation(operation,msg);

			/*Send response msg to console*/
			if( (nwrite=write(out,msg,RESPONSESIZE))==-1)
			{
				perror("Error writing response to console...");exit(EXIT_FAILURE);
			}

			/*Send OK for the next operation*/
			if( (nwrite=write(out,"OK",RESPONSESIZE))==-1)
			{
				perror("Error writing \"OK\" to console...");exit(EXIT_FAILURE);
			}
		}
	}

/***********************************************************************************/

	/*No need to wait for pools here*/
	/*Ive done that in shutdown() function */

	/*Going back to main() in order to exit*/
	free(operation);
	free(msg);
}

void execute_operation(char* operation,char* msg)
{

	char* token         = malloc(MSGSIZE*sizeof(char));
	char* pool_response = malloc(RESPONSESIZE*sizeof(char));
	int ret             = sscanf(operation,"%s",token);

	sprintf(msg,"\n=================================\n%s\n\n",operation);
	
	/* In case an empty line is read from input */
	if(ret <= 0)
	{
		strcpy(pool_response,"Invalid Operation !");
		strcat(msg,pool_response);
		strcat(msg,"\n=================================\n");
		free(token);
		return;
	}		


	if(!strcmp(token,"submit"))
	{
		submit(operation,pool_response);
	}
	else if(!strcmp(token,"status"))
	{
		status(operation,pool_response);
	}
	else if(!strcmp(token,"status-all"))
	{
		status_all(operation,pool_response);
	}
	else if(!strcmp(token,"show-active"))
	{
		show_active(operation,pool_response);
	}
	else if(!strcmp(token,"show-pools"))
	{
		show_pools(operation,pool_response);
	}
	else if(!strcmp(token,"show-finished"))
	{
		show_finished(operation,pool_response);
	}
	else if(!strcmp(token,"suspend"))
	{
		suspend(operation,pool_response);
	}
	else if(!strcmp(token,"resume"))
	{
		resume(operation,pool_response);
	}
	else if(!strcmp(token,"shutdown"))
	{
		shutdown(operation,pool_response);
	}
	else
	{
		strcpy(pool_response,"Invalid Operation !");
	}

	strcat(msg,pool_response);
	strcat(msg,"\n=================================\n");
	free(pool_response);
	free(token);
}

void update_table(int i)
{
	pool_table[i].running = 0;
	pool_table[i].CurrentNumberOfJobs = 0;
	jobs_served+=jobs_per_pool;
}

int first_available(void)
{
	/*Find first available pool*/
	int i;
	int retval;
	for(i=0;i<pools;++i)
	{
		if(pool_table[i].CurrentNumberOfJobs < jobs_per_pool)
		{
			retval=i;
			break;
		}
	}
	/*All pools are full --> need to realloc MORE_POOLS */
	if(i==pools)
	{
		retval=pools;

		/*Increase pools*/
		pools+=MORE_POOLS;

		/*Make sure that realloc succeeded*/
		struct entry* newptr;
		if(newptr = realloc(pool_table,pools*sizeof(struct entry))) pool_table = newptr;
		else {perror("realloc error...");exit(EXIT_FAILURE);}


		/*Initialize i=pools ---> pools+MORE_POOLS*/
		for(;i<pools;++i)
		{
			pool_table[i].CurrentNumberOfJobs=0;
			pool_table[i].running=0;
		}
		create_pool(retval);
	}

	/*We found a pool with enough space*/
	else
	{
		/*If pool his not currently running*/
		if(!pool_table[retval].running)
		{
			create_pool(retval);
		}
	}
	return retval;
}

int find_pool_index(int jobID)
{

	int i;

	if(jobID > jobs_sent)return -1;

	for(i=0;i<pools;++i)
	{
		if( pool_table[i].jobIDLowerBound <= jobID && pool_table[i].jobIDUpperBound >= jobID && pool_table[i].running )
		{
			return i;
		}
	}
	return -1;
}

void create_pool(int index)
{
	char out[MSGSIZE];
	char in[MSGSIZE];
	sprintf(out,"%s/pool_out_%d",path,index+1);
	sprintf(in,"%s/pool_in_%d",path,index+1);
	pid_t p;

	switch(p=fork())
	{
		case -1:
		{
			perror (" Failed to fork [coordinator --> pool]");
			exit (-1) ;
		}

		case 0:
		{
			/**
			 * Argumnets given to the pool program:
			 *
			 * 1. pool_first_job
			 * 2. max jobs per pool [= jobs_per_pool]
			 * 3. fifo in name  [already attached to the path]
			 * 4. fifo out name [already attached to the path]
			 * 5. path for output directories
			 */
			char pool_first_job[5];
			char maxjobs[5];
			sprintf(pool_first_job,"%d",jobs_sent+1);
			sprintf(maxjobs,"%d",jobs_per_pool);

			if(execl("./pool","pool",pool_first_job,maxjobs,in,out,path,NULL)==-1){perror("[Error] Executing pool");exit(EXIT_FAILURE);}
		}
		default:
		{
			pool_table[index].jobIDUpperBound = jobs_sent + jobs_per_pool;
			pool_table[index].jobIDLowerBound = pool_table[index].jobIDUpperBound - jobs_per_pool + 1;
			pool_table[index].running = 1;
			pool_table[index].pool_pid = p;


			/*Creating pool_out fifo*/
			if ( mkfifo(out, 0666) == -1 )if ( errno!=EEXIST ) { perror("coordinator: mkfifo"); exit(EXIT_FAILURE); }

			/*Opening pool_out fifo*/
			if ( ( pool_table[index].fd_out =open(out, O_RDONLY|O_NONBLOCK )) < 0)
			{
				perror("pool_out open problem[coordinator]"); exit(EXIT_FAILURE);
			}

			/*Opening pool_in [loop until pool_in has been created by the pool]*/
			while((pool_table[index].fd_in=open(in, O_WRONLY)) == -1);
		}
	}
}

void exit_pool(int pool,char* response)
{

	char* temp = malloc(RESPONSESIZE*sizeof(char));
	int status;
	int i;
	int nwrite;
	for(i=0;i<strlen(response);++i)
	{
		/*pool wants to exit*/
		if(response[i] == '$')
		{
			/*Send EXIT YES to this pool*/
			if((nwrite=write(pool_table[pool].fd_in,"EXIT YES",MSGSIZE)==-1))
			{
				perror("Error writing to pools");exit(EXIT_FAILURE);
			}

			/*Wait for it to exit*/
			waitpid(pool_table[pool].pool_pid,&status,WNOHANG);

			/*Update pool table*/
			update_table(pool);
			break;
		}
	}
	/*Remove dollar sign if there */
	sscanf(response,"%[^$]",temp);
	strcpy(response,temp);
	free(temp);
}