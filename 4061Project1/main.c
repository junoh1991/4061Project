#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>

#include "util.h"

int main(int argc, char *argv[]) {
  /* Variables you'll want to use */
  target_t targets[MAX_NODES];
  int nTargetCount = 0;
  char Makefile[64] = "Makefile";
  char TargetName[64];

  /* Declarations for getopt */
  extern int optind;
  extern char * optarg;
  int ch;
  char * format = "f:h";

  while((ch = getopt(argc, argv, format)) != -1) {
    switch(ch) {
      case 'f':
        strcpy(Makefile, strdup(optarg));
        break;
      case 'h':
      default:
        show_error_message(argv[0]);
        exit(1);
    }
  }

  argc -= optind;
  if(argc > 1) {
    show_error_message(argv[0]);
    return -1;
  }

  /* Init Targets */
  memset(targets, 0, sizeof(targets));

  /* Parse graph file or die */
  if((nTargetCount = parse(Makefile, targets)) == -1) {
    return -1;
  }

  /* Comment out the following line before submission */
  // show_targets(targets, nTargetCount);

  /*
   * Set Targetname
   * If target is not set, set it to default (first target from makefile)
   */
  if(argc == 1) {
    strcpy(TargetName, argv[optind]);
  } else {
    strcpy(TargetName, targets[0].TargetName);
  }
    
  // Set Status of 'top' target to 1
  int targetIndex = find_target(TargetName, targets , nTargetCount);
  targets[targetIndex].Status = 1;
    
  buildTarget(TargetName, targets, nTargetCount);
  return 0;
}

void createProcess(char* Command, char* TargetName)
{  
  printf("%s\n", Command);
  
  pid_t pid;
  pid = fork();
  if (pid > 0) // Parent
  {
    int wstatus;
    wait(&wstatus);
    if (WEXITSTATUS(wstatus) != 0)
    {
      printf("make4061: *** [%s] Error %d\n", TargetName, WEXITSTATUS(wstatus));
      exit(-1);
    }
  }  
  else if (pid == 0) // Child
  {
    execvp(*build_argv(Command), build_argv(Command));
    printf("Child failed to exec all_ids\n");
    exit(-1);
  }
  else
  {
    printf("Failed to fork\n");
    exit(-1);
  }
}

int check_build_time(target_t targets[], int targetIndex)
{
  int i;
  for (i = 0; i < targets[targetIndex].DependencyCount; i++)
  {
	// If the input file doesnt exist or newer, return 1 at any time of the loop. 
	if (compare_modification_time(targets[targetIndex].TargetName, 
	    targets[targetIndex].DependencyNames[i]) == -1 || 
	    compare_modification_time(targets[targetIndex].TargetName, 
	    targets[targetIndex].DependencyNames[i]) == 2)
		return 1;	
  }
  
  // Check if target has no dependencies
  if(targets[targetIndex].DependencyCount == 0)
    return 1;
    
  // Only returns up-to-date message if 'top' target requires no updating
  if (targets[targetIndex].Status == 1) 
    printf("make4061: '%s' is up to date\n", targets[targetIndex].TargetName);
  return 0;
}

void buildTarget(char* TargetName, target_t targets[], int nTargetCount)
{
  int i;	
  // Find target.
  int targetIndex = find_target(TargetName, targets , nTargetCount);
  if (targetIndex >= 0)	// Found target
  {
    for (i = 0; i<targets[targetIndex].DependencyCount; i++)
      buildTarget(targets[targetIndex].DependencyNames[i], targets, nTargetCount);
    
    // If target is not up to date execute command
    if (check_build_time(targets, targetIndex) == 1)
      createProcess(targets[targetIndex].Command, targets[targetIndex].TargetName);
  }
  else
  {
    // Check if dependency was a file instead of a target (such as a .h or .cc file)
    if(does_file_exist(TargetName) == -1)
    {
      printf("Dependency '%s' not found\n", TargetName);
      exit(-1);
    }
  }
}

void show_error_message(char * ExecName) {
  fprintf(stderr, "Usage: \n%s [options] [target] : only single target is allowed.\n", ExecName);
  fprintf(stderr, "-f FILE\t\tRead FILE as a makefile.\n");
  fprintf(stderr, "-h\t\tPrint this message and exit.\n");
  exit(0);
}
