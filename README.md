# Job Management System

## About

This is the second assignment of "Unix Systems Programming" course (spring 2017).

The main goal of this assignment is to create a *job management system* , which will handle a handful of jobs (i.e: unix commands or user programs) submitted by the user.

Jobs will be processed by a hierarchy of processes created dynamically using *exec()* and *fork()* system calls. Those processes will be able to communicate via named pipes and signals.


## Implementation

The application consists of three main parts :

  * jms_console : Provides an interface for the user to submit the jobs.

  * jms_coord   : Responsible for coordinating the jobs. Creates *pool* processes which will take care of the execution of the jobs.

  * pool        : Each pool handles the execution of a number of jobs. Returns statistics back to the coordinator upon finishing.


## Operations

Operations file will contain operations of the following types :

  * submit <job\>
  * status <JobID\>
  * status-all [time-duration\]
  * show-active
  * show-pools
  * show-finished
  * suspend <JobID\>
  * resume <JobID\>
  * shutdown

## Usage

<<<<<<< HEAD
* `./jms console -w <jms in> -r <jms out> -o <operations file>`

* `./jms coord -l <path> -n <jobs pool> -w <jms out> -r <jms in>`

* `./jms console -w \<jms in> -r \<jms out> -o \<operations file>`

* `./jms coord -l \<path> -n \<jobs pool> -w \<jms out> -r \<jms in>`


  Modify makefile constants to match your needs (e.g: pipe names, output directory e.t.c)
