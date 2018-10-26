COORD_OBJS	= coord_main.o coord_functions.o coord_operations.o 
CONSOLE_OBJS 	= console_main.o console_functions.o
POOL_OBJS 	= pool_main.o pool_functions.o pool_operations.o job.o

COORD_SOURCE 	= coord_main.c coord_functions.c coord_operations.c 
CONSOLE_SOURCE 	= console_main.c console_functions.c
POOL_SOURCE 	= pool_main.c pool_functions.c pool_operations.c job.c  

HEADERS		= MiscHeader.h JobHeader.h CoordHeader.h CoordOperations.h ConsoleHeader.h PoolHeader.h PoolOperations.h

COORD		= coord
CONSOLE		= console
POOL		= pool

CC 		= gcc
FLAGS 		= -g -c

JMS_IN 		= jms_in
JMS_OUT		= jms_out
DIRPATH 	= /home/users1/sdi1400136/results
OPFILE 		= optest
JOBS_POO	= 20


##########################################
all:$(POOL) run_coord run_cons

run_coord:$(COORD)
	./${COORD} -l $(DIRPATH) -w $(JMS_OUT) -r $(JMS_IN) -n $(JOBS_POOL) &
	
run_cons:$(CONSOLE)
	./${CONSOLE} -w $(JMS_IN) -r $(JMS_OUT) -o $(OPFILE)


########## Executables ##############
$(COORD):$(COORD_OBJS)
	$(CC) -g $(COORD_OBJS) -o $@

$(CONSOLE):$(CONSOLE_OBJS)
	$(CC) -g $(CONSOLE_OBJS) -o $@

$(POOL):$(POOL_OBJS)
	$(CC) -g $(POOL_OBJS) -o $@

############## COORD_OBJS ##############
coord_main.o:coord_main.c
	$(CC) $(FLAGS) coord_main.c

coord_functions.o:coord_functions.c
	$(CC) $(FLAGS) coord_functions.c
	
coord_operations.o:coord_operations.c
	$(CC) $(FLAGS) coord_operations.c
	
############## CONSOLE_OBJS ##############
console_main.o:console_main.c
	$(CC) $(FLAGS) console_main.c

console_functions.o:console_functions.c
	$(CC) $(FLAGS) console_functions.c

############## POOL_OBJS ##############
pool_main.o:pool_main.c
	$(CC) $(FLAGS) pool_main.c

pool_functions.o:pool_functions.c
	$(CC) $(FLAGS) pool_functions.c
	
pool_operations.o:pool_operations.c
	$(CC) $(FLAGS) pool_operations.c
	
job.o:job.c
	$(CC) $(FLAGS) job.c
	
##################### Counting lines of cod #####################
#count:
#	wc $(COORD_SOURCE) $(CONSOLE_SOURCE) $(POOL_SOURCE) $(HEADERS)
