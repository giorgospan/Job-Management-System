#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h> /*kill()*/
#include <sys/wait.h> /*waitpid()*/
#include <signal.h>

#include "misc_header.h" /*Misc defines*/
#include "pool_header.h" /*Declarations of vars associated with pool*/
#include "pool_operations.h" /* submit() status()... */

void pool_coord_comm(int in ,int out)
{

	int nwrite,nread;
	int status;
	pid_t wpid;
	int i;
	char* operation = malloc(MSGSIZE*sizeof(char));
	char* response  = malloc(RESPONSESIZE*sizeof(char));


	while(1)
	{
		/* Check if there is something new from coordinator */
		if((nread=read(in,operation, MSGSIZE))>0)
		{
			parse_operation(operation,response,out);

			/* Coord has allowed me to exit */
			if(!strcmp(response,"I AM EXITING")){break;}

			/* Inform coordinator that I'm about to exit */
			/* by appending a dollar sign to the response */			
			if(finished == maxjobs)
			{
				strcat(response,"$");
			}
			/* Otherwise just send back the response */
			if ((nwrite=write(out,response, RESPONSESIZE)) == -1)
			{ perror("Pool Writing"); exit(EXIT_FAILURE); }
		}


		/* Check if a job has exited */
		if( (wpid=waitpid (-1, &status , WNOHANG ))>0)
				update_table(wpid);

	}
	free(operation);
	free(response);
}

void catch_term_signal(int signo)
{
	int i;
	int wpid;
	int status;
	int still_in_progress = 0;

	for(i=0;i<maxjobs;++i)
	{
		/* Send SIGTERM[15] to all processes that has not yet finished */
		if(job_table[i].jobID)
		if(job_table[i].status ==1 || job_table[i].status ==3)
		{

			/* Send the signal */
			kill(job_table[i].pid,signo);

			/* Wait for it to exit */
			waitpid(job_table[i].pid,&status,WNOHANG);

			/* Increase counter */
			++still_in_progress;
		}
	}

	/* Pool's exit code will be used from coordinator */
	exit(still_in_progress);
}

void parse_operation(char* operation,char* response,int out)
{
	char* token = malloc(MSGSIZE*sizeof(char));
	sscanf(operation,"%s",token);
	int nwrite;
	if(!strcmp(token,"submit"))
	{
		char* job = malloc(MSGSIZE*sizeof(char));
		sscanf(operation,"%*s %[^\t\n]s",job);
		submit(job,response);
		free(job);
	}
	else if(!strcmp(token,"status"))
	{
		int jobID;
		sscanf(operation,"%*s %d",&jobID);
		status(jobID,response);
	}
	else if(!strcmp(token,"status-all"))
	{
		int limit = -1;
		sscanf(operation,"%*s %d",&limit);
		status_all(limit,response);
	}
	else if(!strcmp(token,"show-active"))
	{
		show_active(response);
	}
	else if(!strcmp(token,"show-pools"))
	{
		show_pools(response);
	}
	else if(!strcmp(token,"show-finished"))
	{
		show_finished(response);
	}
	else if(!strcmp(token,"suspend"))
	{
		int jobID;
		sscanf(operation,"%*s %d",&jobID);
		suspend(jobID,response);
	}
	else if(!strcmp(token,"resume"))
	{
		int jobID;
		sscanf(operation,"%*s %d",&jobID);
		resume(jobID,response);
	}
	else if(!strcmp(operation,"EXIT YES"))
	{
		strcpy(response,"I AM EXITING");
	}
	free(token);
}

void update_table(pid_t p)
{
	int i;
	for(i=0;i<maxjobs;++i)
	{
		if(job_table[i].pid == p)
		{
			++finished;
			job_table[i].status = 2;
		}
	}
}


void find_status(int jobID,int i,char* response)
{
	switch(job_table[i].status)
	{
		/* Currently Active */
		case 1:
		{
			int init    = job_table[i].init_time;
			int active  = job_table[i]. active_time;
			int plus    = job_table[i].last_suspended;
			int current = time(NULL);

			int running ;

			/* It has never been suspended */
			if(!plus)
			{
				running = current - init;
			}
			/* It has been suspended at least one time */
			else
			{
				running = active + current - plus;
			}
			sprintf(response,"JobID %d Status: Active (running for %d seconds)",jobID,running);
			break;
		}
		/* Finished */
		case 2:
		{
			sprintf(response,"JobID %d Status: Finished",jobID);
			break;
		}
		/* Currently Suspended */
		default:
		{
			sprintf(response,"JobID %d Status: Suspended",jobID);
		}
	}
}