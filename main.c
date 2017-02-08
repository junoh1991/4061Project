#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>

#include "util.h"
void buildTarget(char* targetName, target_t targets[], int nTargetCount);

void show_error_message(char * ExecName) {
  fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", ExecName);
  fprintf(stderr, "-f FILE\t\tRead FILE as a makefile.\n");
  fprintf(stderr, "-h\t\tPrint this message and exit.\n");
  exit(0);
}

target_t targets[MAX_NODES];
int nTargetCount;

int main(int argc, char *argv[]) {
  nTargetCount = 0;
  
  /* Variables you'll want to use */
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

  /*
   * Now, the file has been parsed and the targets have been named.
   * You'll now want to check all dependencies (whether they are 
   * available targets or files) and then execute the target that 
   * was specified on the command line, along with their dependencies, 
   * etc. Else if no target is mentioned then build the first target 
   * found in Makefile.
   */

  /*
   * INSERT YOUR CODE HERE
   */
  buildTarget(TargetName, targets, nTargetCount);
  
  return 0;
}

void buildTarget(char* targetName, target_t targets[], int nTargetCount)
{
  // Find target.
  int targetIndex = find_target(targetName, targets, nTargetCount);;
  if (targetIndex != -1)	// Found target
  {
	int i;
    for (i = 0; i<targets[targetIndex].DependencyCount; i++)
	    buildTarget(targets[targetIndex].DependencyNames[i], targets, nTargetCount);
  }
  
  // Reached here after traversing all leaf nodes. 
  // Build using command
  // printf("%s\n", targets[targetIndex].Command);
  createProcess(targetIndex, targets[targetIndex].Command);
}

int createProcess(int targetIndex, char* command)
{

  pid_t childpid;
  childpid = fork();
  if(childpid == -1)
  {
    perror("Failed to fork");
    return 1;
  }
  if (childpid == 0)
  {
    execl("/bin/bash", command);
    perror("Child failed to exec all_ids");
    return 1;
  }
  if(childpid != wait(NULL))
  {
    perror("parent failed to wait due to signal or error");
    return 1;
  }
}
