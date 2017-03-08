#include "wrapper.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_TAB 100
extern int errno;

/*
 * Name:                uri_entered_cb
 * Input arguments:     'entry'-address bar where the url was entered
 *                      'data'-auxiliary data sent along with the event
 * Output arguments:    none
 * Function:            When the user hits the enter after entering the url
 *                      in the address bar, 'activate' event is generated
 *                      for the Widget Entry, for which 'uri_entered_cb'
 *                      callback is called. Controller-tab captures this event
 *                      and sends the browsing request to the ROUTER (parent)
 *                      process.
 */
void uri_entered_cb(GtkWidget* entry, gpointer data) {
	if(data == NULL) {	
		return;
	}
	browser_window *b_window = (browser_window *)data;
	// This channel have pipes to communicate with ROUTER. 
	comm_channel channel = ((browser_window*)data)->channel;
	// Get the tab index where the URL is to be rendered
	int tab_index = query_tab_id_for_request(entry, data);
	if(tab_index <= 0) {
		fprintf(stderr, "Invalid tab index (%d).", tab_index);
		return;
	}
	// Get the URL.
	char * uri = get_entered_uri(entry);
	// Append your code here
	// Send 'which' message to 'which' process?
	//
}

/*
 * Name:                create_new_tab_cb
 * Input arguments:     'button' - whose click generated this callback
 *                      'data' - auxillary data passed along for handling
 *                      this event.
 * Output arguments:    none
 * Function:            This is the callback function for the 'create_new_tab'
 *                      event which is generated when the user clicks the '+'
 *                      button in the controller-tab. The controller-tab
 *                      redirects the request to the ROUTER (parent) process
 *                      which then creates a new child process for creating
 *                      and managing this new tab.
 */ 
void create_new_tab_cb(GtkButton *button, gpointer data)
{
	if(data == NULL) {
		return;
	}
	browser_window *b_window = (browser_window *)data;
	// This channel have pipes to communicate with ROUTER. 
	comm_channel channel = ((browser_window*)data)->channel;
	// Append your code here
	// Send 'which' message to 'which' process?
	//
}

/*
 * Name:                url_rendering_process
 * Input arguments:     'tab_index': URL-RENDERING tab index
 *                      'channel': Includes pipes to communctaion with
 *                      Router process
 * Output arguments:    none
 * Function:            This function will make a URL-RENDRERING tab Note.
 *                      You need to use below functions to handle tab event. 
 *                      1. process_all_gtk_events();
 *                      2. process_single_gtk_event();
*/
int url_rendering_process(int tab_index, comm_channel *channel) {
	browser_window * b_window = NULL;
	// Create url-rendering window
	create_browser(URL_RENDERING_TAB, tab_index, NULL, NULL, &b_window, channel);
	child_req_to_parent req;
	while (1) {
		usleep(1000);
		// Append your code here
		// Handle 'which' messages?
		//
	}
	return 0;
}
/*
 * Name:                controller_process
 * Input arguments:     'channel': Includes pipes to communctaion with
 *                      Router process
 * Output arguments:    none
 * Function:            This function will make a CONTROLLER window and 
 *                      be blocked until the program terminates.
 */
int controller_process(comm_channel *channel) {
	// Do not need to change code in this function
	close(channel->child_to_parent_fd[0]);
	close(channel->parent_to_child_fd[1]);
	browser_window * b_window = NULL;
	// Create controler window
	create_browser(CONTROLLER_TAB, 0, G_CALLBACK(create_new_tab_cb), G_CALLBACK(uri_entered_cb), &b_window, channel);
	show_browser();
	return 0;
}

/*
 * Name:                router_process
 * Input arguments:     none
 * Output arguments:    none
 * Function:            This function will make a CONTROLLER window and be blocked until the program terminate.
 */
int router_process() {
	comm_channel *channel[MAX_TAB];
	channel[0] = (comm_channel *) malloc(sizeof(comm_channel));
    // Append your code here
	// Prepare communication pipes with the CONTROLLER process
	// Fork the CONTROLLER process
	//   call controller_process() in the forked CONTROLLER process
	// Poll child processes' communication channels using non-blocking pipes.
	//   handle received messages:
	//     CREATE_TAB:
	//       Prepare communication pipes with a new URL-RENDERING process
	//       Fork the new URL-RENDERING process
	//     NEW_URI_ENTERED:
	//       Send message to the URL-RENDERING process in which the new url is going to be rendered
	//     TAB_KILLED:
	//       If the killed process is the CONTROLLER process, send messages to kill all URL-RENDERING processes
	//       If the killed process is a URL-RENDERING process, send message to the URL-RENDERING to kill it
	//   sleep some time if no message received
	//
    int router_controller[2];
    int pid, flags;
    int proceIndex = 0;

    // Create a pipe b/w router and controller
    if (pipe(router_controller) == -1)
    {
        perror("pipe error");
        exit(1);
    }

    // Set Non block-read from controller.
    flags = fcntl(router_controller[0], F_GETFL, 0);
    fcntl(router_controller[0], F_SETFL, flags | O_NONBLOCK);
    channel[0] -> child_to_parent_fd[0] = router_controller[0];    
    channel[0] -> child_to_parent_fd[1] = router_controller[1];    
    pid = fork();
    // Parents
    if (pid >  0)
    {
        wait(NULL);     
        usleep(1000); // Sleep 1ms
    }
    // Child
    else if (pid ==  0)
    {     
        controller_process(channel[0]);
    }
    // Fork Error
    else
    {
        perror("Fork Error");
    }

	return 0;
}

int main() {
	return router_process();
}























