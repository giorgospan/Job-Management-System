#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h> /*to have access to flags def*/
#include <sys/types.h> /*to create directory*/
#include <dirent.h>
#include <unistd.h> /*to have access to fds of stdin and stderr*/

#include "MiscHeader.h"
#include "JobHeader.h"

void create_directory(char* path,int jobID,int* outfd,int* errfd)
{
	
	/* File Descriptors:
		0:stdin
		1:stdout
		2:stderr
	*/
	
	char* outfile = malloc(NAMESIZE*sizeof(char));
	char* errfile = malloc(NAMESIZE*sizeof(char));
	char* directory = malloc(NAMESIZE*sizeof(char));
	char* current_time = malloc(NAMESIZE*sizeof(char));
	char* date = malloc(NAMESIZE*sizeof(char));
	
	pid_t pid = getpid();
	
	time_t rawtime;   
	time(&rawtime);
	struct tm* timeinfo = localtime(&rawtime);
	
	sprintf(current_time,"%02d%02d%02d",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	sprintf(date,"%04d%02d%02d",timeinfo->tm_year+1900,timeinfo->tm_mon+1,timeinfo->tm_mday);
	sprintf(directory,"%s/sdi1400136_%d_%d_%s_%s",path,jobID,(int)pid,date,current_time);
	
	/*Constructing the path for the files*/
	sprintf(outfile,"%s/stdout_%d",directory,jobID);
	sprintf(errfile,"%s/stderr_%d",directory,jobID);
	
	/*Creating our new directory*/
	if( mkdir(directory,DIRPERMS)== -1 )
	{
		perror("Making directory");
		exit(1);
	}

	/*Creating the files*/
	if( (*outfd=open(outfile,O_CREAT | O_RDWR, FILEPERMS)) == -1)
	{
		perror("Opening outfile");
		exit(1);
	}
	if( (*errfd=open(errfile,O_CREAT | O_RDWR, FILEPERMS)) == -1)
	{
		perror("Opening errfile");
		exit(1);
	}

	free(outfile);
	free(errfile);
	free(directory);
	free(current_time);
	free(date);
}

char** prepare_for_exec(char* path,char* job,int jobID)
{
	char** arguments = malloc(ARGUMENTS*sizeof(char*));
	int i=0;
	int outfd;
	int errfd;
	char* token = malloc(20*sizeof(char));
	//system("rm -rf /home/users1/sdi1400136/results/sdi1400136*");
	
	/* Remove trailing newline, if there. */
	if ((strlen(job)>0) && (job[strlen (job) - 1] == '\n'))job[strlen (job) - 1] = '\0';
	
	/*Create stdout and stderr files*/
	create_directory(path,jobID,&outfd,&errfd);
	
	/*dupping file descriptors*/
	dup2(outfd,STDOUT_FILENO);
	dup2(errfd,STDERR_FILENO);
	close(outfd);
	close(errfd);
	
	
	/*Seperate arguments*/
	for(i=0;i<ARGUMENTS;++i)
	{
		arguments[i] = malloc(20*sizeof(char));
	}
	
	token = strtok(job, " ");
	strcpy(arguments[0],token);
	i=1;
	while (token) 
	{
		token = strtok(NULL, " ");
		if(token)strcpy(arguments[i],token);
		++i;
	}
	arguments[i-1]=NULL;
	return arguments;
}