# source files
COORD_SOURCE   := $(wildcard ./coordinator/*.c)
CONSOLE_SOURCE := $(wildcard ./console/*.c)
POOL_SOURCE    := $(wildcard ./pool/*.c)

# object files
COORD_OBJS     := $(COORD_SOURCE:%.c=%.o)
CONSOLE_OBJS   := $(CONSOLE_SOURCE:%.c=%.o)
POOL_OBJS      := $(POOL_SOURCE:%.c=%.o)

# headers
HEADERS	       := $(wildcard include/*.h)

# build directory
BUILDDIR       := ./build

# targets
COORD		       := jms_coord
CONSOLE		     := jms_console
POOL		       := pool

CC 		         := gcc
CFLAGS 		     := -I./include -g

# JMS_IN	     := jms_in
# JMS_OU	     := jms_out
# JOBS_POOL	   := 10
OUTDIR 	       := ./output
OPFILE 		     := opfile


# 1. Cleanup
# 2. Create output directory
# 3. Create pool program
# 4. Create and run coordinator program
# 5. Create and run console program
all:clean $(OUTDIR) $(BUILDDIR)/$(POOL) coordinator console

$(OUTDIR):
	mkdir $@

############## COORDINATOR ##############

coordinator:$(BUILDDIR)/$(COORD)

$(BUILDDIR)/$(COORD):$(COORD_OBJS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -o $@ $(COORD_OBJS)

$(COORD_OBJS):./coordinator/%.o : ./coordinator/%.c
	$(CC) $(CFLAGS) -c $< -o $@

############## CONSOLE ##############

console:$(BUILDDIR)/$(CONSOLE)

$(BUILDDIR)/$(CONSOLE):$(CONSOLE_OBJS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -o $@ $(CONSOLE_OBJS)

$(CONSOLE_OBJS):./console/%.o : ./console/%.c
		$(CC) $(CFLAGS) -c $< -o $@


############## POOL ##############

$(BUILDDIR)/$(POOL):$(POOL_OBJS) | $(BUILDDIR)
	$(CC) $(CFLAGS) -o $@ $(POOL_OBJS)

$(POOL_OBJS):./pool/%.o : ./pool/%.c
			$(CC) $(CFLAGS) -c $< -o $@

# this rule will be triggered in case build dir is not created yet
$(BUILDDIR):
	@mkdir $@

# count
.PHONY: count
count:
	@wc $(COORD_SOURCE) $(CONSOLE_SOURCE) $(POOL_SOURCE) $(HEADERS)


# delete object files and executables
# delete all named pipes in the current directory
# delete output directories
.PHONY: clean
clean:
	@rm -f $(COORD_OBJS) $(CONSOLE_OBJS) $(POOL_OBJS) $(BUILDDIR)/*

	@find ./* -type p -delete

	@sudo rm -rf $(OUTDIR)/*

	@echo Cleanup completed

# kill any processes from previous runs
.PHONY: kill
kill:
	sudo pkill coord;sudo pkill pool
