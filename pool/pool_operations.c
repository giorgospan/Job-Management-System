#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> /*fork()*/
#include <sys/types.h> /*kill()*/
#include <signal.h>

#include "misc_header.h" /*Misc defines*/
#include "pool_header.h" /*Declarations of vars associated with pool*/
#include "pool_operations.h"
#include "job_header.h" /*create_directory() , prepare_for_exec() */

void submit(char* job,char* response)
{
	int i;
	int id;
	int place;
	pid_t pid;


	for(i=0;i<maxjobs;++i)
	{
		if(!job_table[i].jobID)
		{
			place = i;
			break;
		}
	}


	/*Mark its unique jobID*/
	id = job_table[place].jobID = pool_first_job + jobs ;

	/*Mark this job as Active*/
	job_table[place].status = 1;

	/*Mark its initialization time*/
	job_table[place].init_time = time(NULL);

	/*Activity time initialized to 0*/
	job_table[place].active_time = 0;

	/*last_suspended set to 0*/
	job_table[place].last_suspended = 0;


	/*Store the job name [might be useful for debugging]*/
	job_table[place].job = malloc((strlen(job)+1)*sizeof(char));
	strcpy(job_table[place].job,job);

	/*Increase number of jobs in the pool*/
	++jobs;

	switch(pid=fork())
	{
		case -1:
		{
			perror("Fork pool --> job");
			exit(-1);
		}

		case 0:
		{
			/*Create directory-files*/
			/*Seperate arguments passed to exec*/
			char** arguments = prepare_for_exec(path,job,id);
			/*Execute job*/
			if(execvp(job,arguments)==-1)
			{
				printf("Job:\"%s\" failed\n",job);
				exit(-2);
			}
		}
		default:
		{
			job_table[place].pid = pid;
			sprintf(response,"JobID:%d  PID:%d",id,pid);
		}
	}
}


void status(int jobID,char* response)
{
	int i;
	/*Coord has already made sure that the given jobID exists*/
	/*Search for this job according to the given jobID*/
	for(i=0;i<maxjobs;++i)
	{
		if(job_table[i].jobID == jobID)
		{
			find_status(jobID,i,response);
			break;
		}
	}
}


void status_all(int limit,char* response)
{
	int i;
	int curr_time     = time(NULL);
	char* curr_status = malloc(RESPONSESIZE*sizeof(char));
	int c             = 0;
	int found         = 0;

	printf("LIMIT GIVEN:%d\n\n",limit);
	for(i=0;i<maxjobs;++i)
	{

		if(job_table[i].jobID)
			if((limit>0 && curr_time - job_table[i].init_time <= limit) || (limit==-1))
			{
				found = 1;
				find_status(job_table[i].jobID,i,curr_status);
				strcat(curr_status,"\n");
				if(!c)strcpy(response,curr_status);
				else strcat(response,curr_status);
				++c;
			}
	}
	if(!found)strcpy(response,"ZERO JOBS FOUND");
	free(curr_status);
}


void show_active(char* response)
{

	int i;
	int c = 0;
	char* jobID = malloc(10*sizeof(char));
	for(i=0;i<maxjobs;++i)
	{
		if(job_table[i].status == 1 && job_table[i].jobID)
		{
			sprintf(jobID,"jobID %d\n",job_table[i].jobID);
			if(!c)strcpy(response,jobID);
			else strcat(response,jobID);
			++c;
		}
	}
	/* Remove trailing newline, if there. */
	if ((strlen(response)>0) && (response[strlen (response) - 1] == '\n'))response[strlen (response) - 1] = '\0';
	free(jobID);
	/*In case all jobs in this pools are all finished*/
	if(!c)strcpy(response,"ZERO ACTIVE");
}


void show_pools(char* response)
{
	int counter=0;
	int i;

	for(i=0;i<maxjobs;++i)
	{
		if(job_table[i].jobID)
		if(job_table[i].status == 1 || job_table[i].status == 3)++counter;
	}
	sprintf(response,"%d %d",getpid(),counter);
}


void show_finished(char* response)
{
	int i;
	int c = 0;
	char* jobID = malloc(10*sizeof(char));
	for(i=0;i<maxjobs;++i)
	{
		if(job_table[i].status == 2 && job_table[i].jobID)
		{
			sprintf(jobID,"jobID %d\n",job_table[i].jobID);
			if(!c)strcpy(response,jobID);
			else strcat(response,jobID);
			++c;
		}
	}
	/* Remove trailing newline, if there. */
	if ((strlen(response)>0) && (response[strlen (response) - 1] == '\n'))response[strlen (response) - 1] = '\0';
	/*In case all jobs in this pools are still running*/
	if(!c)strcpy(response,"ZERO FINISHED");
	free(jobID);
}


void suspend(int jobID,char* response)
{
	int i;
	int ret;

	for(i=0;i<maxjobs;++i)
	{
		if(job_table[i].jobID == jobID)
		{
			/*Send SIGSTOP to this pid */
			ret=kill(job_table[i].pid,19);
			/*Change its status*/
			job_table[i].status = 3;
			int current = time(NULL);

			/*First time being suspended*/
			if(!job_table[i].last_suspended)
			{
				job_table[i].active_time = current - job_table[i].init_time;
				job_table[i].last_suspended = current;
			}
			/*It has already been suspended at least one time in the past*/
			else
			{
				job_table[i].active_time = current - job_table[i].last_suspended;
				job_table[i].last_suspended = current;
			}
			break;
		}
	}

	/*If kill() returned -1 ---> job had already finished*/
	if(!ret)sprintf(response,"Sent suspend signal to JobID %d",jobID);
	else sprintf(response,"Job had already finished %d",jobID);
}


void resume(int jobID,char* response)
{
	int i;
	int ret;
	for(i=0;i<maxjobs;++i)
	{
		if(job_table[i].jobID == jobID)
		{
			/*Send SIGCONT to this pid*/
			ret=kill(job_table[i].pid,18);
			/*Change its status */
			job_table[i].status = 1;
			break;
		}
	}
	/*If kill() returned -1 ---> job had already finished*/
	if(!ret)sprintf(response,"Sent resume signal to JobID %d",jobID);
	else sprintf(response,"Job had already finished %d",jobID);
}

