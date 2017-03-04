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
 *	 TargetName, in target array in main()
 *	 
 *	 targets, targets array in main()
 *
 *   nTargetCount, number of targets returned by 'parse'
 *
 */
void buildTarget(char* TargetName, target_t targets[], int nTargetCount);

/*
 * Check relative build time of the target versus its dependencies to check_build_time
 * if target needs to be built or called
 *
 * Input: 
 *	 targets, targets array in main()
 *
 *   targetIndex, speicfic index of targets
 *
 * Return:
 *	 1, if the input file doesn't exist or newer than the target
 *   0, if the target is up to date
 *	 
 */
int check_build_time(target_t targets[], int targetIndex);

/*
 * Create a child proccess for running commands. 
 * Prints the commands and, if applicable, errors.
 *
 * Input:
 *  Command, the name of the target's command from target_t Command
 *
 *  TargetName, in target array in main()
 *
 */
void createProcess(char* Command, char* TargetName);

/*
 * Prints out helpful messages for multiple targets and -h helper utility
 * 
 * Input:
 *  ExecName, argv[] to the main.c
 *  
 *
 * Note: Provided by the initial file.
 */
void show_error_message(char * ExecName);
#endif
