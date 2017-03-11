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
    // Create struct with information for creating new tab
    new_uri_req a;
    strcpy(a.uri, uri);
    a.render_in_tab = tab_index;
    child_request b;
    b.uri_req = a;
    child_req_to_parent new_uri = {NEW_URI_ENTERED, b};
    // Send the struct to ROUTER
    write(channel.child_to_parent_fd[1], &new_uri, sizeof(&new_uri));
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
    // Create struct with information for creating new tab
    create_new_tab_req a;
    child_request b;
    b.new_tab_req = a;
    child_req_to_parent new_tab = {CREATE_TAB, b};
    // Send the struct to ROUTER
    write(channel.child_to_parent_fd[1], &new_tab, sizeof(&new_tab));
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
    
    int router_controller[2];
    int pid, flags;
    int proceIndex = 0;
    int read_error;
    child_req_to_parent *temp = (child_req_to_parent*)malloc(sizeof(child_req_to_parent));   
 
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
        while(1)
        {
            read_error = read(channel[0] -> child_to_parent_fd[0], temp, sizeof(temp));
            if (read_error > 0)
            {
                printf("%d\n", temp -> type);
            }
            usleep(10);
        }            
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























