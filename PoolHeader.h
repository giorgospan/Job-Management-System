#ifndef POOL_HEADER_H
#define POOL_HEADER_H



/*
Assumptions for job status:
 1. Active
 2. Finished
 3. Suspended
*/

extern struct entry* job_table;
struct entry
{
	pid_t pid;
	int jobID;
	int status;
	int init_time;    /*Time when executed*/
	int last_suspended; /*Last time suspended*/
	int active_time; /*Time being active*/
	char* job;
};

extern int maxjobs; 			/*max number of jobs for pool [given as command line argument]*/
extern int pool_number; 		/*Number of pool*/
extern int jobs; 		 		/*Number of jobs currently in the pool*/
extern int finished;  			/*Number of terminated jobs in the pool*/
extern char path[PATHSIZE]; 	/*Path for creating directories */
extern int can_exit; 			/*1: if pool can exit, 0: if pool must continue running*/

void catch_term_signal(int);

void pool_coord_comm(int,int);

void process_response(char*,char*,int out);

void update_table(pid_t p);


void find_status(int,int,char*);

#endif