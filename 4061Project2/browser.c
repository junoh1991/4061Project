/* CSci4061 S2017 Assignment 2
* date: 3/20/2017
* name: Ari Bible, Tristan Mansfield, Jun Oh
* id: bible012, mansf043, ohxxx371 */

// include files
#include "wrapper.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define MAX_TAB 100

extern int errno;

/*
 * Name:                uri_entered_cb
 * Input arguments:     'entry': address bar where the url was entered
 *                      'data': auxiliary data sent along with the event
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
    // This channel has pipes to communicate with ROUTER 
    comm_channel channel = ((browser_window*)data)->channel;
    // Get tab index where URL is to be rendered
    int tab_index = query_tab_id_for_request(entry, data);
    if(tab_index <= 0) {
        fprintf(stderr, "Invalid tab index (%d).", tab_index);
        return;
    }
    // Get URL
    char * uri = get_entered_uri(entry);
    // Create struct with info for creating new tab
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
 * Input arguments:     'button': whose click generated this callback
 *                      'data': auxillary data passed along for handling
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
    // This channel has pipes to communicate with ROUTER.
    comm_channel channel = ((browser_window*)data)->channel;  
    // Create struct with info for creating new tab
    create_new_tab_req a;
    child_request b;
    b.new_tab_req = a;
    child_req_to_parent new_tab = {CREATE_TAB, b};
    // Send struct to ROUTER
    write(channel.child_to_parent_fd[1], &new_tab, sizeof(child_req_to_parent));
}

/*
 * Name:                router_create_tab
 * Input arguments:     'channel': Includes pipes to communicate with Router process
 *                      'tab_index': the tab number of the new url_rendering process
 * Output arguments:    '-1' : fork error
 *                      >0: pid of the forked child
 * Function:            Thsi function forks to create a url_rendering process. 
 *                      Will close appropirate pipe channels for parent and child. 
 *                      Child process calls  url_rendering_process
*/
int router_create_tab(comm_channel *channel, int tab_index)
{
	int flags;

    int new_tab_pid = fork();
	
    if (new_tab_pid > 0)  // Parent
	{	
        close(channel -> child_to_parent_fd[1]);
        close(channel -> parent_to_child_fd[0]);
        flags = fcntl(channel -> child_to_parent_fd[0], F_GETFL, 0);
        fcntl(channel -> child_to_parent_fd[0], F_SETFL, flags | O_NONBLOCK);
    #ifdef DEBUG
        printf("Parent is creating a child tab %i\n",tab_index); 
    #endif
	}
	else if (new_tab_pid == 0) // Child
	{
        close(channel -> child_to_parent_fd[0]);
        close(channel -> parent_to_child_fd[1]);
        flags = fcntl(channel -> parent_to_child_fd[0], F_GETFL, 0);
        fcntl(channel-> parent_to_child_fd[0], F_SETFL, flags | O_NONBLOCK);
    #ifdef DEBUG
        printf("Child is created for tab %i\n", tab_index);
    #endif
		url_rendering_process(tab_index, channel);
	}
	else
	{
		perror("Fork Error");
        return -1;
	}

    return new_tab_pid;
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
            if (recBuf.type == TAB_KILLED)
            {
                process_all_gtk_events();
                exit(0);
            }
            else if (recBuf.type == NEW_URI_ENTERED)
                render_web_page_in_tab(recBuf.req.uri_req.uri, b_window); 
	    }
        else
            process_single_gtk_event();		
        
	}
	return 0;
}

/*
 * Name:                controller_process
 * Input arguments:     'channel': Includes pipes to communctaion with Router
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
 * Function:            This function will make a CONTROLLER window and be blocked
 *			until the program terminates.
 */
int router_process() {
	comm_channel *channel[MAX_TAB];
	channel[0] = (comm_channel *) malloc(sizeof(comm_channel));
	
	int tab_index = 1;
    int pid, flags, command_type, i;
    int status = 0;
	int child_pids[MAX_TAB];
    int killed_index, tab_number;
    child_req_to_parent temp; 
    
    // Create pipe between router and controller
    if (pipe((channel[0] -> child_to_parent_fd)) == -1)
    {
        perror("pipe error");
        exit(1);
    }
    // Set non-blocking read from controller
    flags = fcntl(channel[0] -> child_to_parent_fd[0], F_GETFL, 0);
    fcntl(channel[0] -> child_to_parent_fd[0], F_SETFL, flags | O_NONBLOCK);
    
	child_pids[0]  = fork();
    if (child_pids[0]  >  0) // parent
    {
		close(channel[0] -> child_to_parent_fd[1]);	// close write-end pipe
        while(1)
        {	
			// Iterate through opened pipe channels to check for any commands
			for (i = 0; i < tab_index; i ++)
            {
                // Check if the tab is closed at index
                if(child_pids[i]== 0)
                    continue; 

                if (read(channel[i] -> child_to_parent_fd[0], &temp, sizeof(child_req_to_parent)) > 0)
				{
					command_type = temp.type;
                #ifdef DEBUG
                    printf("command type: %i\n", command_type);
                #endif
					switch (command_type)
					{
                    case 0:	// Receive new tab command from controller process
                        channel[tab_index] = (comm_channel *) malloc(sizeof(comm_channel));

                        // Open bidrectional pipe for communication
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
                        
                        // Fork
                        pid = router_create_tab(channel[tab_index], tab_index);
                        if (pid == -1)
                        {
                            perror("Fork Failed\n");
                            exit(-1);
                        }
                        else
                        {
                            child_pids[tab_index] = pid; 
                            tab_index++;
                        }
                        break;
                        
                    case 1:	// URL command from controller
                        // Verify tab number
                        tab_number = temp.req.uri_req.render_in_tab;
                        if (tab_number >=  tab_index || child_pids[tab_number] == 0)
                            perror("Wrong tab number");
                        else
                            write(channel[tab_number]->parent_to_child_fd[1], &temp, sizeof(child_req_to_parent));
                        break;
                        
                    case 2: // Tab killed command from controller or url process 
                        killed_index = temp.req.killed_req.tab_index;

                        if (killed_index == 0) // From controller. Kill all url_renering processes
                        {
                            for (int j = 1; j < tab_index; j++)
                            {
                                if (child_pids[j] == 0)
                                    continue;
                                else if (waitpid(child_pids[j], &status, WNOHANG) >  0) // Check if process defunct
                                {
                                #ifdef DEBUG
                                    printf("child process at tab %i was a zombie. Cleaned succsfully\n", j);
                                #endif
                                    close(channel[j] -> child_to_parent_fd[0]);
                                    close(channel[j] -> parent_to_child_fd[1]);
                                    child_pids[j] = 0;
                                    free(channel[j]);
                                    continue;
                                }
                                else
                                {
                                    write(channel[j] -> parent_to_child_fd[1], &temp, sizeof(child_req_to_parent));
                                    waitpid(child_pids[j], &status,0);
                                #ifdef DEBUG
                                    printf("child process at tab %i  exited successfully\n", j );
                                #endif
                                    // Close pipes, free channel
                                    close(channel[j] -> child_to_parent_fd[0]);
                                    close(channel[j] -> parent_to_child_fd[1]);
                                    child_pids[j] = 0;
                                    free(channel[j]);
                                } 
                            }
                            // Free controller process stuffs
                            close(channel[0] -> child_to_parent_fd[0]);
                            close(channel[0] -> child_to_parent_fd[1]); 
                            free(channel[0]);
                            waitpid(child_pids[0], &status, 0);
                            exit(0);
                        }
                        else // From url. Kill this tab
                        {
                            write(channel[killed_index]->parent_to_child_fd[1], &temp, sizeof(child_req_to_parent));
                            waitpid(child_pids[killed_index], &status, 0);
                        #ifdef DEBUG
                            printf("child process exited successfully\n");
                        #endif
                            // Close pipes, free channel
                            close(channel[killed_index] -> child_to_parent_fd[0]);
                            close(channel[killed_index] -> parent_to_child_fd[1]);
                            child_pids[killed_index] = 0;
                            free(channel[killed_index]);
                        }

                        break;
                        
                    default:
                        break;
					}
				}	
            }
			
            usleep(1000);
        }            
    }
    // Child process, controller process
    else if (child_pids[0] ==  0)
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
