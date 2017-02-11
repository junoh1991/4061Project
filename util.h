/********************
 * util.h
 *
 * You may put your utility function definitions here
 * also your structs, if you create any
 *********************/
#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_NODES 10




/* 
 * Check if file exist
 *
 * Input:
 *   Filename
 *
 * Return:
 *    0, file exist
 *
 *   -1 file does not exist
 */
int does_file_exist(char *FileName);

/*
 * Compare timestamp of File1 and File2
 *
 * Input:
 *   Filename 1, Filename 2
 *
 * Return:
 *   -1, File1 and/or File2 do not exist;
 *    0, File1 and File2 have identical timestamp;
 *    1, File1 is modified later than File2;
 *    2, File1 is modified earlier than File2.
 */
int compare_modification_time(char *FileName1, char *FileName2);

/* 
 * A Vertex in the DAG
 */
typedef struct target{
  char TargetName[64];          /* Target name */
  int  DependencyCount;         /* Number of dependencies */
  char DependencyNames[10][64]; /* Names of all the dependencies */
  char Command[256];            /* Command that needs to be executed for this target */
  int  Status;                  /* Status of the target (Ready for execution,
                                 * Finished etc. based on your implementation) */
}target_t;



/*
 * Parse Makefile, build the DAG of target dependencies
 *
 * Input:
 *   Makefile, makefile filename
 *
 *   targets, a static array saving target objects
 *
 * Return:
 *   On SUCCESS, nTargetCount - the number of targets in Makefile
 *
 *   On FAILURE, -1 (error in the Makefile)
 */
int parse(char *Makefile, target_t targets[]);

/*
 * Find a given target in all targets
 *
 * Input:
 *   targets, targets array in main()
 *
 *   nTargetCount, number of targets returned by 'parse'
 *
 * Return:
 *   On SUCCESS, the index of the given target in targets array
 *
 *   On FAILURE, -1 (not found)
 */
int find_target(char *TargetName, target_t targets[], int nTargetCount);

/*
 * Display target objects
 *
 * Input:
 *   targets, targets array in main()
 *
 *   nTargetCount, number of targets returned by 'parse'
 */
void show_targets(target_t targets[], int nTargetCount);

/*
 * Tokenize 'Command' string
 *
 * Input:
 *   target.Command
 *
 * Return:
 *   On SUCCESS, 'char **argv' to be used to call 'execvp'
 *
 *   On FAILURE, NULL
 */
char **build_argv(const char *Command);

/*
 * Recursively build target in target_t targets[]
 *
 * Input: 
 *	 targetName, in target array in main()
 *	 
 *	 targets, targets array in main()
 *
 *   nTargetCount, number of targets returned by 'parse'
 *
 *	 Command, targets' member variable
 *
 */
void buildTarget(char* TargetName, target_t targets[], int nTargetCount);

int check_build_time(target_t targets[], int targetIndex);

int createProcess(char* Command, char* TargetName);

void show_error_message(char * ExecName);
#endif
