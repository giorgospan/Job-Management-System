#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> /* read() , write()*/
#include <sys/types.h> /*kill()*/
#include <signal.h>
#include <errno.h>

#include "CoordHeader.h" /*Declarations of vars associated with coordinator*/
#include "CoordOperations.h"
#include "MiscHeader.h"


/************************ NOTE ********************************************/

/* In those functions there is no need to check if console has sent me something
 because it has not yet received "OK" for current operation as it is currently 
 under processesing by the pools.*/

/*************************************************************************/


void submit(char* operation,char* response)
{
	pid_t pid;
	int available;
	int nwrite;
	
	/*Find first available pool*/
	available = first_available();
	/*Send operation to pool */
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
			/* Increase number of jobs sent in general */
			++pool_table[available].CurrentNumberOfJobs ;
			++jobs_sent;
			/*Check if a pool wants to exit*/
			check_exit(available,response);
			
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
				check_exit(pool,response);
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
	int send_counter = 0;
	int receive_counter = 0;
	int at_least_one_job = 0;
	char* new_info = malloc(RESPONSESIZE*sizeof(char));
	
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
	// printf("=====================================\n");
	while(1)
	{
		/*Loop through currently running pools until I've received all info*/
		for(i=0;i<pools;++i)
		{
			if(pool_table[i].running)
			{
				// printf("Pool[%d] is running |  i:%d   | Address:%p \n",pool_table[i].pool_pid,i,&pool_table[i].pool_pid);
				if(read(pool_table[i].fd_out,new_info,RESPONSESIZE)>0)
				{
					/*Check if a pool wants to exit*/
					check_exit(i,new_info);
					if(strcmp(new_info,"ZERO JOBS FOUND"))
					{
						at_least_one_job = 1;
						/*Attach info sent by the pool to our response*/
						if(!receive_counter)strcpy(response,new_info);
						else strcat(response,new_info);
					}
					++receive_counter;
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
	int send_counter = 0;
	int receive_counter = 0;
	int at_least_one_active = 0;
	char* new_info = malloc(RESPONSESIZE*sizeof(char));
	
	/* Send this request to all pools currently running*/
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
		/*Loop through currently running pools until I've received all info*/
		for(i=0;i<pools;++i)
		{
			if(pool_table[i].running)
			{
				if(read(pool_table[i].fd_out,new_info,RESPONSESIZE)>0)
				{
					/*Check if a pool wants to exit*/
					check_exit(i,new_info);
					if(strcmp(new_info,"ZERO ACTIVE"))
					{
						at_least_one_active = 1;
						/*Attach info sent by the pool to our response*/
						strcat(new_info,"\n");
						if(!receive_counter)strcpy(response,new_info);
						else strcat(response,new_info);
					}
					++receive_counter;
				}
			}
		}
		
		/*Break as soon as all pools have sent their info*/
		if(receive_counter == send_counter)break;
		

	}
	
	if(!at_least_one_active)strcpy(response,"No jobs currently active");
	free(new_info);
	/*Return back in order to send response to console*/
	return;
}



void show_pools(char* operation,char* response)
{
	pid_t pid;
	int nwrite;
	int nread;
	int i;
	int send_counter = 0;
	int receive_counter = 0;
	char* info = malloc(RESPONSESIZE*sizeof(char));
	char* temp = malloc(RESPONSESIZE*sizeof(char)); 
	
	/* Send this request to all pools currently running*/
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
		/*Loop through currently running pools until I've received all info*/
		for(i=0;i<pools;++i)
		{
			if(pool_table[i].running)
			{
				if(read(pool_table[i].fd_out,info,RESPONSESIZE)>0)
				{
					/*Check if a pool wants to exit*/
					check_exit(i,info);
					/*Attach info to our response*/
					sprintf(temp,"%s\n",info);
					if(!receive_counter)strcpy(response,temp);
					else strcat(response,temp);
					++receive_counter;
				}
			}
		}
		/*Break as soon as all pools have sent their info*/
		if(receive_counter == send_counter)break;
	}

	free(info);
	free(temp);
	/*Return back in order to send response to console*/
	return;
}


void show_finished(char* operation,char* response)
{
	pid_t pid;
	int nwrite;
	int nread;
	int i;
	int send_counter = 0;
	int receive_counter = 0;
	int at_least_one_finished = 0;
	char* info = malloc(RESPONSESIZE*sizeof(char));
	
	/* Send this request to all pools currently running*/
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
		/*Loop through currently running pools until I've received all info*/
		for(i=0;i<pools;++i)
		{
			if(pool_table[i].running)
			{
				if(read(pool_table[i].fd_out,info,RESPONSESIZE)>0)
				{
					/*Check if a pool wants to exit*/
					check_exit(i,info);
					
					/*Attach info sent by the pool to our response*/
					if(strcmp(info,"ZERO FINISHED"))
					{
						at_least_one_finished = 1;
						strcat(info,"\n");
						if(!receive_counter)strcpy(response,info);
						else strcat(response,info);
					}
					++receive_counter;
				}
			}
		}
		
		
		/*Break as soon as all pools have sent their info*/
		if(receive_counter == send_counter)break;
	}

	if(!at_least_one_finished)strcpy(response,"No jobs finished yet.");
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
				check_exit(pool,response);
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
				check_exit(pool,response);
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
	
	/*Send SIGTERM[15] to all currently running pools*/
	for(i=0;i<pools;++i)
	{
		if(pool_table[i].running)
		{
			/*Send SIGTERM*/
			kill(pool_table[i].pool_pid,15);
			
			/*Wait until it has exited*/
			if(waitpid(pool_table[i].pool_pid, &status)>0)
			{
				/*Each pool will exit with status equivalent to the number of jobs still in progress*/
				still_in_progress+=WEXITSTATUS(status);
			}
		}
	}
	if(!jobs_served)sum = jobs_sent - still_in_progress;
	else sum = jobs_served;
	sprintf(response,"Served %d jobs,%d were still in progress",sum,still_in_progress);
	return ;
}


