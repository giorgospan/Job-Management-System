#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> /* read() , write()*/
#include <sys/types.h> /*kill()*/
#include <sys/wait.h> /*waitpid()*/
#include <signal.h>
#include <errno.h>

#include "coord_header.h" /*Declarations of vars associated with coordinator*/
#include "coord_operations.h"
#include "misc_header.h"


/**
 * In these functions there is no need to check if console has sent me something
 * because it has not yet received "OK" for current operation as it is currently
 * under processesing by the pools.
 *
 */

void submit(char* operation,char* response)
{
	pid_t pid;
	int available;
	int nwrite;

	/*Find first available pool*/
	available = first_available();

	/*Send operation to the pool */
	if ((nwrite=write(pool_table[available].fd_in,operation, MSGSIZE)) == -1)
	{
		perror("Coord writing to pool ");exit(-6);
	}
	while(1)
	{
		/*Break if this pool has sent me the response for submit*/
		if(read(pool_table[available].fd_out,response,RESPONSESIZE)>0)
		{

			/* Increase number of jobs in this pool */
			/* Increase total number of jobs sent to all pools */
			++pool_table[available].CurrentNumberOfJobs ;
			++jobs_sent;
			/*Check if the pool wants to exit*/
			exit_pool(available,response);

			break;
		}
	}
	/*Return back to communication with console */
	return ;
}

void status(char* operation,char* response)
{
	pid_t pid;
	int pool;
	int nwrite;
	int nread;
	int index=0;
	int jobID;
	sscanf(operation,"%*s %d",&jobID);



	/* jobID could not be found */
	if( (pool = find_pool_index(jobID)) == -1 )
	{
		/*No need to send anything to any of the pools*/
		sprintf(response,"Job with ID \'%d\' could not be found\n",jobID);
	}

	/* jobID was found */
	else
	{
		/*Send request to this specific pool */
		if ((nwrite=write(pool_table[pool].fd_in,operation, MSGSIZE)) == -1)
		{
			perror("Coord writing to pool ");exit(-6);
		}
		while(1)
		{
			/*Break if this pool has sent me the response for "status jobID"*/
			if(read(pool_table[pool].fd_out,response,RESPONSESIZE)>0)
			{
				/*Check if a pool wants to exit*/
				exit_pool(pool,response);
				break;
			}
		}
	}
	/*Return back in order to send response to console*/
	return;
}

void status_all(char* operation,char* response)
{
	pid_t pid;
	int nwrite;
	int nread;
	int i;
	int send_counter     = 0;
	int receive_counter  = 0;
	int at_least_one_job = 0;
	char* new_info       = malloc(RESPONSESIZE*sizeof(char));

	/* Send this request to all pools currently running*/
	for(i=0;i<pools;++i)
	{
		if(pool_table[i].running)
		{
			// printf("\nPool[%d] is running |  i:%d  | Address:%p\n",pool_table[i].pool_pid,i,&pool_table[i].pool_pid);
			if ((nwrite=write(pool_table[i].fd_in,operation, MSGSIZE)) == -1)
			{
				perror("Coord writing to pool ");exit(-6);
			}
			++send_counter;
		}
	}

	while(1)
	{
		/*Loop through currently running pools until I've received all info*/
		for(i=0;i<pools;++i)
		{
			if(pool_table[i].running)
			{
				// printf("About to read from pool[%d]\n",i);
				if(read(pool_table[i].fd_out,new_info,RESPONSESIZE)>0)
				{
					/*Check if a pool wants to exit*/
					// exit_pool(i,new_info);
					if(strcmp(new_info,"ZERO JOBS FOUND"))
					{
						at_least_one_job = 1;
						/*Attach info sent by the pool to our response*/
						if(!receive_counter)strcpy(response,new_info);
						else strcat(response,new_info);
					}
					++receive_counter;
					// printf("Finished reading from pool[%d]:\n%s\n",i,new_info);
				}
			}
		}

		/*Break as soon as all pools have sent their info*/
		if(receive_counter == send_counter)break;
	}

	if(!at_least_one_job)strcpy(response,"[Status-all]:No jobs found");
	free(new_info);
	/*Return back in order to send response to console*/
	return;

}

void show_active(char* operation,char* response)
{
	pid_t pid;
	int nwrite;
	int nread;
	int i;
	int at_least_one_active = 0;
	char* new_info          = malloc(RESPONSESIZE*sizeof(char));
	response[0]             = '\0';

	for(i=0;i<pools;++i)
	{
		/* Pool is running */
		/* Send the operation */
		if(pool_table[i].running)
			if ((nwrite=write(pool_table[i].fd_in,operation, MSGSIZE)) == -1)
			{
				perror("Coord writing to pool ");exit(-6);
			}
		/* If pool has terminated, then all of its jobs have finished */
	}

	/* Loop through currently running pools until I've received all info */
	for(i=0;i<pools;++i)
	{
		if(pool_table[i].running)
		{
			if(read(pool_table[i].fd_out,new_info,RESPONSESIZE)>0)
			{
				/* Check if this pool wants to exit */
				exit_pool(i,new_info);
				if(strcmp(new_info,"ZERO ACTIVE"))
				{
					at_least_one_active = 1;
					
					/* Attach info sent by the pool to our response */
					strcat(new_info,"\n");
					strcat(response,new_info);
				}
			}
		}
	}

	if(!at_least_one_active)strcpy(response,"No jobs currently active");
	free(new_info);
	/* Return back in order to send response to console */
	return;
}

void show_pools(char* operation,char* response)
{
	pid_t pid;
	int nwrite;
	int nread;
	int i;
	int send_counter    = 0;
	int receive_counter = 0;
	char* info          = malloc(RESPONSESIZE*sizeof(char));
	char* temp          = malloc(RESPONSESIZE*sizeof(char));
	response[0]         = '\0';
	
	/* Send this request to all pools currently running */
	for(i=0;i<pools;++i)
	{
		if(pool_table[i].running)
		{
			if ((nwrite=write(pool_table[i].fd_in,operation, MSGSIZE)) == -1)
			{
				perror("Coord writing to pool ");exit(-6);
			}
			++send_counter;
		}
	}

	while(1)
	{
		/* Loop through currently running pools until I've received all info */
		for(i=0;i<pools;++i)
		{
			if(pool_table[i].running)
			{
				if(read(pool_table[i].fd_out,info,RESPONSESIZE)>0)
				{
					/* Check if a pool wants to exit */
					exit_pool(i,info);
					/* Attach info sent by the pool to our response */
					strcat(info,"\n");
					strcat(response,info);
					++receive_counter;
				}
			}
		}
		/* Break as soon as all pools have sent their info */
		if(receive_counter == send_counter)break;
	}

	free(info);
	free(temp);
	/* Return back in order to send response to console */
	return;
}

void show_finished(char* operation,char* response)
{
	pid_t pid;
	int nwrite;
	int nread;
	int i;
	int send_counter          = 0;
	int receive_counter       = 0;
	int at_least_one_finished = 0;
	char* info                = malloc(RESPONSESIZE*sizeof(char));
	response[0]               = '\0';

	for(i=0;i<pools;++i)
	{
		/* Pool is running */
		/* Send the operation */
		if(pool_table[i].running)
		{
			if ((nwrite=write(pool_table[i].fd_in,operation, MSGSIZE)) == -1)
			{
				perror("Coord writing to pool ");exit(-6);
			}
			++send_counter;
		}

		/* Pool has terminated --> all jobs assigned to it have finished */
		/* Attach all its jobs to the response */
		else
		{
			// printf("Pool with jobs [%d,%d] has exited\n",pool_table[i].jobIDLowerBound,pool_table[i].jobIDUpperBound);
			for (int jobid = pool_table[i].jobIDLowerBound; jobid <= pool_table[i].jobIDUpperBound; ++jobid)
			{
				sprintf(info,"JobID %d\n",jobid);
				strcat(response,info);
			}
		}
	}

	// while(1)
	// {
		/*Loop through currently running pools until I've received all info*/
		for(i=0;i<pools;++i)
		{
			if(pool_table[i].running)
			{
				if(read(pool_table[i].fd_out,info,RESPONSESIZE)>0)
				{
					/*Check if a pool wants to exit*/
					exit_pool(i,info);

					/*Attach info sent by the pool to our response*/
					if(strcmp(info,"ZERO FINISHED"))
					{
						at_least_one_finished = 1;
						strcat(info,"\n");
						strcat(response,info);
					}
					++receive_counter;
				}
			}
		}

		/*Break as soon as all pools have sent their info*/
	// 	if(receive_counter == send_counter)break;
	// }

	if(!at_least_one_finished)strcpy(response,"No jobs finished yet");
	free(info);
	/*Return back in order to send response to console*/
	return;
}

void suspend(char* operation,char* response)
{
	pid_t pid;
	int pool;
	int nwrite;
	int nread;
	int jobID;
	sscanf(operation,"%*s %d",&jobID);

	/* jobID could not be found */
	if( (pool = find_pool_index(jobID)) == -1 )
	{
		/*No need to send anything to any of the pools*/
		sprintf(response,"Job with ID \'%d\' could not be found\n",jobID);
	}

	/* jobID was found */
	else
	{
		/*Send request to this specific pool */
		if ((nwrite=write(pool_table[pool].fd_in,operation, MSGSIZE)) == -1)
		{
			perror("Coord writing to pool ");exit(-6);
		}
		while(1)
		{
			/*Break if this pool has sent me the response for "suspend jobID"*/
			if(read(pool_table[pool].fd_out,response,RESPONSESIZE)>0)
			{
				/*Check if a pool wants to exit*/
				exit_pool(pool,response);
				break;
			}

		}
	}
	/*Return back in order to send response to console*/
	return;
}

void resume(char* operation,char* response)
{
	pid_t pid;
	int pool;
	int nwrite;
	int nread;
	int jobID;
	sscanf(operation,"%*s %d",&jobID);

	/* jobID could not be found */
	if( (pool = find_pool_index(jobID)) == -1 )
	{
		/*No need to send anything to any of the pools*/
		sprintf(response,"Job with ID \'%d\' could not be found\n",jobID);
	}

	/* jobID was found */
	else
	{
		/*Send request to this specific pool */
		if ((nwrite=write(pool_table[pool].fd_in,operation, MSGSIZE)) == -1)
		{
			perror("Coord writing to pool ");exit(-6);
		}
		while(1)
		{
			/*Break if this pool has sent me the response for "resume jobID"*/
			if(read(pool_table[pool].fd_out,response,RESPONSESIZE)>0)
			{
				/*Check if a pool wants to exit*/
				exit_pool(pool,response);
				break;
			}
		}
	}
	/*Return back in order to send response to console*/
	return;

}

void shutdown(char* operation,char* response)
{
	int i;
	int status;
	pid_t wpid;
	int sum;

	/*Number of jobs still in progress*/
	int still_in_progress = 0;

	/*Send SIGTERM[15] to every pool currently running */
	for(i=0;i<pools;++i)
	{
		if(pool_table[i].running)
		{
			/*Send SIGTERM*/
			kill(pool_table[i].pool_pid,15);

			/*Wait until it has exited*/
			if(waitpid(pool_table[i].pool_pid, &status,0)>0)
			{
				/*Each pool will exit with status equal to the number of jobs still in progress*/
				still_in_progress+=WEXITSTATUS(status);
			}
		}
	}
	// printf("jobs_sent        :%d\n",jobs_sent);
	// printf("jobs_served      :%d\n",jobs_served);
	// printf("still_in_progress:%d\n\n",still_in_progress);

	sum = jobs_sent - still_in_progress;

	sprintf(response,"Served %d jobs,%d were still in progress",sum,still_in_progress);
	return ;
}


