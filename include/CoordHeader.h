#ifndef COORD_HEADER_H
#define COORD_HEADER_H

/*Number of pools added when reallocating memory*/
#define MORE_POOLS 2

/*Entry of pool_table*/
struct entry
{
	pid_t pool_pid;
	int running;
	int CurrentNumberOfJobs;
	int jobIDUpperBound;
	int jobIDLowerBound;
	int fd_in;
	int fd_out;
};

/*Our pool_table*/
extern struct entry* pool_table;

/*Number of jobs served*/
extern int jobs_served;


/*Number of pools in our table [either running or not yet executed]*/
extern int pools;

/*Number of pools that have been executed during the program */
extern int jobs_sent;


/*Maximum number of jobs per pool*/
extern int jobs_pool;


/*Path for creating fifos and directories*/
extern char* path;

/***************************************************************************************/


/* Communication between console <---> coordinator  <------> pools */
void coord_communication(int,int);

/*  Communication between console <-------> pools */
void create_response(char*,char*);

/*Updates pool table*/
void update_table(pid_t);

/*Finds first available pool*/
int first_available(void);

/*Finds the pool that has the jobID given as argument*/
int find_pool_index(int);

/*Executes pool.exe*/
void create_pool(int);

/*Forbids termination to pools*/
void forbid_exit(int);

#endif