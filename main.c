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
	      printf("-f filename: filename will be the name of the makefile, otherwise the default name “Makefile” is assumed. \n \
		    specificTarget: specificTarget will be the name of any single target in the Makefile. \n \
		    -h: print make program options available. \n");
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

  buildTarget(TargetName, targets, nTargetCount, targets[0].Command);	// FIX last parameter
  return 0;
}

int createProcess(char* Command)
{  
  /*
   * For debugging purpose. Delete later
   */
  printf("%s\n", Command);
  
  pid_t pid;
  pid = fork();
  if (pid > 0) // Parent
  {
    int wstatus;
    wait(&wstatus);
    if (WEXITSTATUS(wstatus) != 0)
    {
      printf("Child failed to execute\n");
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

void buildTarget(char* TargetName, target_t targets[], int nTargetCount, char *Command)
{
  int i;	
  // Find target.
  int targetIndex = find_target(TargetName, targets , nTargetCount);
  if (targetIndex >= 0)	// Found target
  {
    for (i = 0; i<targets[targetIndex].DependencyCount; i++)
    {
      buildTarget(targets[targetIndex].DependencyNames[i], targets, nTargetCount, targets[targetIndex].Command);
    }  
    createProcess(targets[targetIndex].Command);
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
  fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", ExecName);
  fprintf(stderr, "-f FILE\t\tRead FILE as a makefile.\n");
  fprintf(stderr, "-h\t\tPrint this message and exit.\n");
  exit(0);
}
