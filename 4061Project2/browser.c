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
    write(channel.child_to_parent_fd[1], &new_uri, sizeof(child_req_to_parent));
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
    write(channel.child_to_parent_fd[1], &new_tab, sizeof(child_req_to_parent));
}


void router_create_tab(comm_channel *channel, int tab_index)
{
	int flags;

    int new_tab_pid = fork();
	
    if (new_tab_pid > 0)  // Parent
	{	
        close(channel -> child_to_parent_fd[1]);
        close(channel -> parent_to_child_fd[0]);
        flags = fcntl(channel -> child_to_parent_fd[0], F_GETFL, 0);
        fcntl(channel -> child_to_parent_fd[0], F_SETFL, flags | O_NONBLOCK);
        printf("Parent is creating a child tab %i\n",tab_index); 
	}
	else if (new_tab_pid == 0) // Child
	{
        close(channel -> child_to_parent_fd[0]);
        close(channel -> parent_to_child_fd[1]);
        flags = fcntl(channel -> parent_to_child_fd[0], F_GETFL, 0);
        fcntl(channel-> parent_to_child_fd[0], F_SETFL, flags | O_NONBLOCK);
        printf("Child is created for tab %i\n", tab_index);
		url_rendering_process(tab_index, channel);
	}
	else
	{
		perror("Fork Error");
	}
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
    child_req_to_parent recBuf;
	while (1) {
		usleep(1000);
		if (read(channel->parent_to_child_fd[0], &recBuf, sizeof(child_req_to_parent)) > 0)
		{
            printf("URL received in url process");
            render_web_page_in_tab(recBuf.req.uri_req.uri, b_window); 
    	}
	    else
            process_single_gtk_event();		
		// TAB_KILLED received
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
	
	int tab_index = 1;
    int pid, flags, command_type, i;
	
	// buffer to store message
    child_req_to_parent temp; 
    
    // Create a pipe b/w router and controller
    if (pipe((channel[0] -> child_to_parent_fd)) == -1)
    {
        perror("pipe error");
        exit(1);
    }
    // Set Non block-read from controller.
    flags = fcntl(channel[0] -> child_to_parent_fd[0], F_GETFL, 0);
    fcntl(channel[0] -> child_to_parent_fd[0], F_SETFL, flags | O_NONBLOCK);
    
	pid = fork();
    if (pid >  0) // parent
    {
		close(channel[0] -> child_to_parent_fd[1]);	// close write-end pipe
        while(1)
        {	
			// Iterate through opened pipe channels to check if there are any commands
			//for (i = 0; i < tab_index; i ++)
            //{
                if (read(channel[0] -> child_to_parent_fd[0], &temp, sizeof(child_req_to_parent)) > 0)
				{
					command_type = temp.type;
                    printf("command type: %i\n", command_type);
					switch (command_type)
					{
						case 0:	// Receive new tab command from controller process
							channel[tab_index] = (comm_channel *) malloc(sizeof(comm_channel));
							if (pipe(channel[tab_index] -> child_to_parent_fd) == -1)
							{
								perror("pipe error");
								break;
							}
							if (pipe(channel[tab_index] -> parent_to_child_fd) == -1)
							{
								perror("pipe error");
								break;
							}
							

							router_create_tab(channel[tab_index], tab_index);
                            tab_index++;
							break;
							
						case 1:	// Receive url command from controller process
                            write(channel[temp.req.uri_req.render_in_tab]->parent_to_child_fd[1], &temp, sizeof(child_req_to_parent));
							break;
							
						case 2: // Tab killed command from url process
				
							break;
							
						default:
							break;
					}
				}	
            //}
			
            usleep(1000);
        }            
    }
    // Child process, controller process
    else if (pid ==  0)
    {     
		close(channel[0] -> child_to_parent_fd[0]);	// Close read-end of pipe
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























