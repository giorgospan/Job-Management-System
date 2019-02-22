#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> 		/*for flags in open() system call*/
#include <sys/types.h> /*for creating named pipe*/
#include <sys/stat.h>

#include "ConsoleHeader.h"
#include "MiscHeader.h"

/*Communication between console <---> coordinator*/
int console_communication(FILE* file,int in,int out)
{
	int nwrite;
	int nread;
	int retval=0;

	char* operation = malloc(MSGSIZE*sizeof(char));
	char* response = malloc(RESPONSESIZE*sizeof(char));
	strcpy(response,"OK");
	// FILE* output=fopen("log","w");

	while(1)
	{
		/*Scan operation, either from stdin or from opfile */
		if(file==stdin) scan_operation(operation);

		/*Break if we've reached EOF [file might not contain shutdown operation]*/
		else if(!fgets(operation,MSGSIZE,file))break;

		/* Remove trailing newline, if there. */
		if ((strlen(operation)>0) && (operation[strlen (operation) - 1] == '\n'))operation[strlen (operation) - 1] = '\0';

		/*Write the next operation*/
		if ((nwrite=write(in,operation, MSGSIZE)) == -1)
		{ perror("Error in writing[console]"); exit(5); }

		/*Read message[info or OK]*/
		if ( (nread=read(out, response, RESPONSESIZE))== -1)
		{
			perror("Error in reading[console]"); exit(4);
		}

		/*Loop until OK has been received*/
		while(strcmp(response,"OK"))
		{
			printf("%s\n\n",response);
			// fprintf(output,"%s\n\n",response);
			if ( (nread=read(out, response, RESPONSESIZE))== -1)
			{
				perror("Error in reading[console]"); exit(4);
			}
		}

		/* Break if last operation sent was "shutdown" */
		if(!strcmp(operation,"shutdown")){retval=1;break;}
	}




	/*Ive received info messages and OK"*/
	/*Waiting for terminating message if shutdown was the last operation scanned*/
	if(retval==1)
	while(nread=read(out, response, RESPONSESIZE))
	{
		if(!strcmp("END_OF_EVERY_POOL",response) )
		{
			if ((nwrite=write(in,"YES", MSGSIZE)) == -1)
			{ perror("Error in writing[console]"); exit(5); }
			break;
		}
	}

	// fclose(output);
	free(operation);
	free(response);
	return retval;
}



/*Reading operations from stdin*/
void scan_operation(char* operation)
{
	int choice=-1;
	int jobID;
	char* job = malloc(200*sizeof(char));
	int duration;

	/*Loop until a valid choice has been  given*/
	while(choice<1 || choice>9)
	{
		printf("---------------------\n");
		printf("Select an operation :\n");
		printf("---------------------\n");

		printf(
			"1.submit\n"
			"2.status\n"
			"3.status-all\n"
			"4.show-active\n"
			"5.show-pools\n"
			"6.show-finished\n"
			"7.suspend\n"
			"8.resume\n"
			"9.shutdown\n\n");

		scanf("%d",&choice);
		switch(choice)
		{
			case 1:
				printf("Enter job terminated with dot \".\"\n");
				scanf("%200[^.]",job);
				sprintf(operation,"submit %s",job);
				break;
			case 2:
				printf("Enter job ID\n");
				scanf("%d",&jobID);
				sprintf(operation,"status %d",jobID);
				break;
			case 3:
				printf("Enter time-duration or -1 if you dont want \n");
				scanf("%d",&duration);
				if(duration)sprintf(operation,"status-all %d",duration);
				else sprintf(operation,"status-all");
				break;
			case 4:
				printf("Active jobs:\n");
				sprintf(operation,"show-active");
				break;
			case 5:
				printf("Pool & NumOfJobs:\n");
				sprintf(operation,"show-pools");
				break;
			case 6:
				printf("Finished jobs:\n");
				sprintf(operation,"show-finished");
				break;
			case 7:
				printf("Enter job ID\n");
				scanf("%d",&jobID);
				sprintf(operation,"suspend %d",jobID);
				printf("Sending suspend signal to job with ID:%d\n",jobID);
				break;
			case 8:
				printf("Enter job ID\n");
				scanf("%d",&jobID);
				sprintf(operation,"resume %d",jobID);
				printf("Sending resume signal to job with ID:%d\n",jobID);
				break;
			case 9:
				printf("System shutting down...\n");
				sprintf(operation,"shutdown");
				break;
			default:
				printf("Error: Your choice was not valid.Try again !\n");
				break;
		}
		/*Clear stdin*/
		int trash;
		while((trash=getchar())!='\n' && trash!=EOF);
	}
	free(job);
}
