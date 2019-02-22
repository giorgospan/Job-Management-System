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
	int init_time; /*Time when executed*/
	int last_suspended; /*Last time suspended*/
	int active_time; /*Time being active*/
	char* job;
};

/*max number of jobs for pool [given as command line argument]*/
extern int maxjobs;

/*Number of pool*/
extern int pool_first_job;

/*Number of jobs currently in the pool*/
extern int jobs;

/*Number of terminated jobs in the pool*/
extern int finished;

/*Path for creating directories */
extern char path[PATHSIZE];

/*1: if pool can exit, 0: if pool must continue running*/
extern int can_exit;

void catch_term_signal(int);

void pool_coord_comm(int,int);

void parse_operation(char*,char*,int out);

void update_table(pid_t p);


void find_status(int,int,char*);

#endif