/* ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗
  ████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗
  ╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝
  ████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗
  ╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝
   ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "utils/HashMap.h"
#include "utils/ArrayList.h"
#include "utils/Utils.h"

struct fds_args {
    int **fds;
    size_t num_fds;
};

struct handler_args {
    int fd;
    char *type;
};

struct client {
	char *chat_status;
	int fd;
	char *chat_name;
};

pthread_mutex_t users_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flights_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t messages_mutex = PTHREAD_MUTEX_INITIALIZER;

HashMap *users;
HashMap *flight_data;
HashMap *sockets_map;
ArrayList *message_queue;

/* ██████╗ ██████╗ ██████╗ ███████╗
  ██╔════╝██╔═══██╗██╔══██╗██╔════╝
  ██║     ██║   ██║██████╔╝█████╗
  ██║     ██║   ██║██╔══██╗██╔══╝
  ╚██████╗╚██████╔╝██║  ██║███████╗
   ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝*/

void *port_accept_loop(void *);
void *chat_accept_loop(void *);
char *command_interpret(int, char *, char *);
char *server_command_interpret(char *, char *);
void *handle_client(void *);


/* █████╗  ██████╗ ███████╗███╗   ██╗ ██████╗██╗   ██╗     ██████╗ ██████╗ ███╗   ███╗███╗   ███╗ █████╗ ███╗   ██╗██████╗ ███████╗
  ██╔══██╗██╔════╝ ██╔════╝████╗  ██║██╔════╝╚██╗ ██╔╝    ██╔════╝██╔═══██╗████╗ ████║████╗ ████║██╔══██╗████╗  ██║██╔══██╗██╔════╝
  ███████║██║  ███╗█████╗  ██╔██╗ ██║██║      ╚████╔╝     ██║     ██║   ██║██╔████╔██║██╔████╔██║███████║██╔██╗ ██║██║  ██║███████╗
  ██╔══██║██║   ██║██╔══╝  ██║╚██╗██║██║       ╚██╔╝      ██║     ██║   ██║██║╚██╔╝██║██║╚██╔╝██║██╔══██║██║╚██╗██║██║  ██║╚════██║
  ██║  ██║╚██████╔╝███████╗██║ ╚████║╚██████╗   ██║       ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║██║  ██║██║ ╚████║██████╔╝███████║
  ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝  ╚═══╝ ╚═════╝   ╚═╝        ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════*/

char *show_agency_help();
char *log_user_in(int, int, char **);
char *query_flights(int, int, char **);
char *reserve_seats(int, int, char **);
char *reserve_return_seats(int, int, char **);
char *reserve(char *, int);
char *list_flights(int);
char *list_available_flights(int, int, char **);
char *logoff(int);

/* ██████╗██╗  ██╗ █████╗ ████████╗     ██████╗ ██████╗ ███╗   ███╗███╗   ███╗ █████╗ ███╗   ██╗██████╗ ███████╗
  ██╔════╝██║  ██║██╔══██╗╚══██╔══╝    ██╔════╝██╔═══██╗████╗ ████║████╗ ████║██╔══██╗████╗  ██║██╔══██╗██╔════╝
  ██║     ███████║███████║   ██║       ██║     ██║   ██║██╔████╔██║██╔████╔██║███████║██╔██╗ ██║██║  ██║███████╗
  ██║     ██╔══██║██╔══██║   ██║       ██║     ██║   ██║██║╚██╔╝██║██║╚██╔╝██║██╔══██║██║╚██╗██║██║  ██║╚════██║
  ╚██████╗██║  ██║██║  ██║   ██║       ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║██║  ██║██║ ╚████║██████╔╝███████║
   ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝        ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝*/

char *show_chat_help();
char *chat_login(int, int, char **);
char *write_to_chat(int, int, char **);
char *list_online_chatters(int);
char *list_all_chatters(int);
char *list_offline_chatters(int);
char *chat_exit(int);

/*███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗      ██████╗ ██████╗ ███╗   ███╗███╗   ███╗ █████╗ ███╗   ██╗██████╗ ███████╗
  ██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗    ██╔════╝██╔═══██╗████╗ ████║████╗ ████║██╔══██╗████╗  ██║██╔══██╗██╔════╝
  ███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝    ██║     ██║   ██║██╔████╔██║██╔████╔██║███████║██╔██╗ ██║██║  ██║███████╗
  ╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗    ██║     ██║   ██║██║╚██╔╝██║██║╚██╔╝██║██╔══██║██║╚██╗██║██║  ██║╚════██║
  ███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║    ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║██║  ██║██║ ╚████║██████╔╝███████║
  ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝     ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝*/

char *show_help();
char *server_list_users(void);
char *server_list_chat_users(void);
char *write_server_output(int, char **);

/*██╗  ██╗███████╗██╗     ██████╗ ███████╗██████╗ ███████╗
  ██║  ██║██╔════╝██║     ██╔══██╗██╔════╝██╔══██╗██╔════╝
  ███████║█████╗  ██║     ██████╔╝█████╗  ██████╔╝███████╗
  ██╔══██║██╔══╝  ██║     ██╔═══╝ ██╔══╝  ██╔══██╗╚════██║
  ██║  ██║███████╗███████╗██║     ███████╗██║  ██║███████║
  ╚═╝  ╚═╝╚══════╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═╝╚══════╝*/

void read_flight_data(FILE *, HashMap *);
int is_logged_in(int, char *);
int chat_name_taken(char *name);
void write_to_all(char *message);

/* ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗
  ████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗
  ╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝
  ████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗
  ╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝
   ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝*/


int main(int argc, char **argv) {
    struct sockaddr_in **saddresses;
    int num_ports;
    int start_port;
    pthread_t sockets_connect_thread;
    FILE *inf = NULL;

    users = malloc(sizeof(HashMap));
    flight_data = malloc(sizeof(HashMap));
    sockets_map = malloc(sizeof(HashMap));
    message_queue = malloc(sizeof(ArrayList));

    HashMap_New(users);
    HashMap_New(flight_data);
    HashMap_New(sockets_map);

    ArrayList_New(message_queue);

    if(argc < 5) {
        printusage(argv[0], "agency");
        exit(EXIT_FAILURE);
    }

    if(!isnumeric(argv[2])) {
        fprintf(stderr, "[ERROR]: StartingPort argument must be a number. Received '%s' instead\n", argv[2]);
        printusage(argv[0], "agency");
        exit(EXIT_FAILURE);
    }
    sscanf(argv[2], "%d", &start_port);

    if(!isnumeric(argv[3])) {
        fprintf(stderr, "[ERROR]: NumberOfPorts argument must be a number. Received '%s' instead\n", argv[3]);
        printusage(argv[0], "agency");
        exit(EXIT_FAILURE);
    }
    sscanf(argv[3], "%d", &num_ports);
    if(num_ports > MAX_PORTS || num_ports < 1) {
        die("Invalid number of ports. Choose a number between 1 and 8.\n");
	}

	printf("\nReading flight data from %s\n\n", argv[4]);
    if((inf = fopen(argv[4], "r")) == NULL) {
        die("Could not open input file for reading.\n");
	}

    read_flight_data(inf, flight_data);

    saddresses = (struct sockaddr_in **) calloc(MAX_PORTS, sizeof(struct sockaddr_in *));

    /* * * Create, bind and listen on multiple sockets, one for each port, randomly * * */
    for(int i = 0; i < num_ports; i++) {
		int *lfd = malloc(sizeof(int));
        *lfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(*lfd == -1) die("Error opening new socket\n");

		int optval = 1;
		setsockopt(*lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

        saddresses[i] = malloc(sizeof(struct sockaddr_in));
        memset((void *) saddresses[i], 0, sizeof(struct sockaddr_in));
        saddresses[i]->sin_family = AF_INET;
        saddresses[i]->sin_port = htons(start_port + i);
        inet_pton(AF_INET, argv[1], (void *) &(saddresses[i]->sin_addr));

        if(bind(*lfd, (struct sockaddr *) saddresses[i], sizeof(struct sockaddr_in)) == -1)
            die("Could not bind address to socket\n");

        if(listen(*lfd, LISTEN_BACKLOG) == -1)
            die("Could not mark socket for listening\n");

		printf("Now listening on %s;%d\n", inet_ntoa(saddresses[i]->sin_addr), ntohs(saddresses[i]->sin_port));

        char *key = malloc(((int) log10((double)start_port)) + 2);
        sprintf(key, "%d", start_port + i);
        HashMap_Set(sockets_map, key, lfd);
        free(key);
    }
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    struct fds_args *loop_args = malloc(sizeof(struct fds_args));
    loop_args->fds = (int **) HashMap_Values(sockets_map);
    loop_args->num_fds = HashMap_GetSize(sockets_map);

    if(pthread_create(&sockets_connect_thread, NULL, port_accept_loop, loop_args) != 0) {
        die("Could not create main port connections thread\n");
	}

	fprintf(stdout, "\nServer loaded. Here is a list of helpful commands:\n\n");
	char *help = show_help();
	printf("%s", help);
	free(help);

    char *command_buff = malloc(MAX_READ_SIZE);
    while(TRUE) {
        memset((void *) command_buff, 0, (size_t) MAX_READ_SIZE);
        fgets(command_buff, MAX_READ_SIZE, stdin);

		if(*command_buff == '\n')
			continue;

		trimws(command_buff);

        char *response = server_command_interpret(argv[0], command_buff);
		
        if(response == NULL)
            continue;

		if(strcmp(command_buff, "exit") == 0)
            break;

		printf("%s", response);
    }

	free(command_buff);

	char **args = (char **) calloc(1, sizeof(char *));
	args[0] = argv[5];
	if(argc > 5)
		write_server_output(1, args);

	HashMap_Free(users);
	HashMap_Free(sockets_map);
	HashMap_Free(flight_data);
	ArrayList_Free(message_queue);

	return 0;
}

/* ██████╗ ██████╗ ██████╗ ███████╗
  ██╔════╝██╔═══██╗██╔══██╗██╔════╝
  ██║     ██║   ██║██████╔╝█████╗
  ██║     ██║   ██║██╔══██╗██╔══╝
  ╚██████╗╚██████╔╝██║  ██║███████╗
   ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝*/

void *port_accept_loop(void *args) {
    pthread_t c_thread;
    fd_set readfds;
	struct sockaddr_in client_addr;
    struct fds_args loop_args = *((struct fds_args *) args);

	time_t raw_time;
	struct tm *timeinfo;

    while(TRUE) {
        int maxfd, fd;

        FD_ZERO(&readfds);
        maxfd = -1;
        for(int i = 0; i < loop_args.num_fds; i++) {
            FD_SET(*(loop_args.fds[i]), &readfds);
            if(*(loop_args.fds[i]) > maxfd)
                maxfd = *(loop_args.fds[i]);
        }

        int status = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if(status < 0)
            die("Invalid socket selection\n");

        fd = -1;
        for(int i = 0; i < loop_args.num_fds; i++) {
            if(FD_ISSET(*(loop_args.fds[i]), &readfds)) {
                fd = *(loop_args.fds[i]);
                break;
            }
        }

        if(fd == -1)
            die("No ready socket found\n");

        int c = sizeof(struct sockaddr_in);

        int cfd = accept(fd, (struct sockaddr *) &client_addr, (socklen_t *) &c);
        if(cfd < 0)
            die("Could not handle accept\n");

		time(&raw_time);
		timeinfo = localtime(&raw_time);
		printf("New connection from %s:%d on %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), asctime(timeinfo));

        if(pthread_create(&c_thread, NULL, handle_client, &cfd) < 0)
            die("Could not create new thread for client\n");
    }

	return NULL;
}

void *handle_client(void *args) {
    int cfd = *((int *) args);
    char client_message[MAX_READ_SIZE], *response;

	memset(client_message, 0, MAX_READ_SIZE);

	/* * * First message is always logon command * * */
	recv(cfd, client_message, MAX_READ_SIZE, 0);
	response = command_interpret(cfd, client_message, "agency");
	send(cfd, response, strlen(response), 0);
	/* * * * * * * * * * * * * * * * * * * * * * * * */ 

	memset(client_message, 0, MAX_READ_SIZE);


	pthread_mutex_lock(&users_mutex);
	char **usernames = HashMap_Keys(users);
    size_t size = HashMap_GetSize(users);

	struct client *user = NULL;

	for(int i = 0; i < size; i++) {
		struct client *check_user = (struct client *) HashMap_Get(users, usernames[i]);	
		if(check_user->fd == cfd) {
			user = check_user;
			break;
		}
	}
	pthread_mutex_unlock(&users_mutex);

    ssize_t read_size;
    while((read_size = recv(cfd, client_message, MAX_READ_SIZE, 0)) > 0) {
		if(strcmp(user->chat_status, "online") == 0)
	        response = command_interpret(cfd, client_message, "chat");
		else
			response = command_interpret(cfd, client_message, "agency");

        if(response == NULL)
            response = "Error processing command\n";

		if(strcmp(response, "Chat message written") != 0)
			send(cfd, response, strlen(response), 0);

        if(response && strcmp(response, "exit") == 0) 
            break;

		memset(client_message, 0, MAX_READ_SIZE);
    }

    close(cfd);

    return NULL;
}

char *command_interpret(int fd, char *command, char *type) {
	int argcnt = charcount(command, ' ') + charcount(command, '\t');
    char **args = (char **) calloc(argcnt, sizeof(char *));

	char *com;
	if(strchr(command, ' ') || strchr(command, '\t')) {
		com = strtok(command, " \t");	
		char *arg = strtok(NULL, " \t");
		int i = 0;
    	while(arg != NULL) {
        	args[i++] = arg;
        	arg = strtok(NULL, " \t");
    	}
	} else {
		com = command;
	}

    strtolower(com);

    if(strcmp(type, "chat") == 0) {
        if(strcmp(com, "help") == 0) {
            free(args);
            return show_chat_help();
        }

        if(strcmp(com, "text") == 0) {
            return write_to_chat(fd, argcnt, args);
        }

        if(strcmp(com, "list") == 0) {
            free(args);
            return list_online_chatters(fd);
        }

        if(strcmp(com, "list_all") == 0) {
            free(args);
            return list_all_chatters(fd);
        }

        if(strcmp(com, "list_offline") == 0) {
            free(args);
            return list_offline_chatters(fd);
        }

        if(strcmp(com, "exit_chat") == 0) {
            free(args);
            return chat_exit(fd);
        }

		free(args);
		return "Invalid chat command. Type `HELP` for a list of operations.\n";
    } else {
		if(strcmp(com, "help") == 0) {
            free(args);
            return show_agency_help();
        }
        if(strcmp(com, "logon") == 0) {
            return log_user_in(fd, argcnt, args);
        }

        if(strcmp(com, "query") == 0) {
            return query_flights(fd, argcnt, args);
        }

        if(strcmp(com, "reserve") == 0) {
            return reserve_seats(fd, argcnt, args);
        }

        if(strcmp(com, "return") == 0) {
            return reserve_return_seats(fd, argcnt, args);
        }

        if(strcmp(com, "list") == 0) {
            free(args);
            return list_flights(fd);
        }

        if(strcmp(com, "list_available") == 0) {
            return list_available_flights(fd, argcnt, args);
        }

		if(strcmp(com, "chat") == 0) {
            return chat_login(fd, argcnt, args);
        }

        if(strcmp(com, "logoff") == 0) {
            free(args);
            return logoff(fd);
        }

        free(args);
        return "Invalid command. Type `HELP` to see a list of available commands.\n";
    }

    free(args);
    return NULL;
}

char *server_command_interpret(char *prog_name, char *command) {
    char **args = (char **) calloc(2, sizeof(char *));
    int argcnt = 0;

    char *com = strtok(command, " \t");
	if(com != NULL) {
		char *arg = strtok(NULL, " \t");
    	while(arg != NULL) {
        	args[argcnt++] = arg;
        	arg = strtok(NULL, " \t");
    	}
	} else {
		com = command;
	}

    strtolower(com);

    if(strcmp(com, "help") == 0) {
        free(args);
        return show_help(prog_name);
    }

    if(strcmp(com, "list") == 0) {
        free(args);
        return server_list_users();
    }

    if(strcmp(com, "list_chat") == 0) {
        free(args);
        return server_list_chat_users();
    }

    if(strcmp(com, "write") == 0) {
        return write_server_output(argcnt, args);
    }

    if(strcmp(com, "exit") == 0) {
        free(args);
        return "exit";
    }

    free(args);
    return "Invalid command. Type `HELP` to see a list of available commands.\n";
}


/* █████╗  ██████╗ ███████╗███╗   ██╗ ██████╗██╗   ██╗     ██████╗ ██████╗ ███╗   ███╗███╗   ███╗ █████╗ ███╗   ██╗██████╗ ███████╗
  ██╔══██╗██╔════╝ ██╔════╝████╗  ██║██╔════╝╚██╗ ██╔╝    ██╔════╝██╔═══██╗████╗ ████║████╗ ████║██╔══██╗████╗  ██║██╔══██╗██╔════╝
  ███████║██║  ███╗█████╗  ██╔██╗ ██║██║      ╚████╔╝     ██║     ██║   ██║██╔████╔██║██╔████╔██║███████║██╔██╗ ██║██║  ██║███████╗
  ██╔══██║██║   ██║██╔══╝  ██║╚██╗██║██║       ╚██╔╝      ██║     ██║   ██║██║╚██╔╝██║██║╚██╔╝██║██╔══██║██║╚██╗██║██║  ██║╚════██║
  ██║  ██║╚██████╔╝███████╗██║ ╚████║╚██████╗   ██║       ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║██║  ██║██║ ╚████║██████╔╝███████║
  ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝  ╚═══╝ ╚═════╝   ╚═╝        ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════*/

char *show_agency_help() {
   	char *help_message = malloc(MAX_READ_SIZE);
	memset(help_message, 0, MAX_READ_SIZE);

	sprintf(help_message, "\n");
	srepeatprint(help_message, '-', 98);
	strcat(help_message, "|\n");
    strcat(help_message, "|  Agency Commands:\n");
	strcat(help_message, "|\n");
   	strcat(help_message, "|\t  HELP --- Show this help message\n");
   	strcat(help_message, "|\t  LOGON <username> --- Log in to the travel agency for access to data and the following commands\n");
   	strcat(help_message, "|\t  QUERY <name-of-flight> --- Return the number of seats available for this flight, if it exists\n");
   	strcat(help_message, "|\t  RESERVE <name-of-flight> <number-of-seats> --- Reserve the given number of seats for the given flight\n");
   	strcat(help_message, "|\t  RETURN <name-of-flight> <number-of-seats> --- Same as above but for a return flight (i.e. given flight is reversed; MIA-ORL --> ORL-MIA)\n");
   	strcat(help_message, "|\t  LIST --- Lists all flights and available seating information\n");
   	strcat(help_message, "|\t  LIST_AVAILABLE --- List only flights with available seats\n");
   	strcat(help_message, "|\t  LIST_AVAILABLE <number> --- List first <number> flights with available seats\n");
   	strcat(help_message, "|\t  CHAT <nickname> --- Enter the Toledo Travel Chat Room and talk to other travelers!\n");
   	strcat(help_message, "|\t  LOGOFF --- Log out of the travel agency\n");
   	strcat(help_message, "|\t  EXIT --- Exit the program\n");
	strcat(help_message, "|\n");
	srepeatprint(help_message, '-', 98);
	sprintf(help_message, "\n");

   	return help_message;

}

char *log_user_in(int fd, int argc, char **args) {
    if(argc < 1) {
        free(args);
        return "\nCommand Usage: LOGON <username>\n\n";
    }

    pthread_mutex_lock(&users_mutex);

    char *user = args[0];
    if(HashMap_Get(users, user) != NULL) {
        free(args);
        pthread_mutex_unlock(&users_mutex);
        return "\nYou have already logged in\n\n";
    }

	struct client *cl = malloc(sizeof(struct client));
	cl->fd = fd;
	cl->chat_name = malloc(56);
	strcpy(cl->chat_name, "unnamed");
	cl->chat_status = malloc(10);
	strcpy(cl->chat_status, "offline");

    HashMap_Set(users, (char *) user, (void *) cl);

    pthread_mutex_unlock(&users_mutex);
    free(args);

	return "\nNow logged in to the Toledo Travel Agency server. Review flight information or reserve your seats.\n\n";
}

char *query_flights(int fd, int argc, char **args) {
    if(argc < 1) {
        free(args);
        return "\nCommand Usage: QUERY <name-of-flight>\n\n";
    }

    if(!is_logged_in(fd, "agency")) {
        free(args);
        return "\nYou must login using LOGON before executing this command\n\n";
    }

	char *flight = args[0];
	char *message = malloc(300);

    pthread_mutex_lock(&flights_mutex);
    char **flights = HashMap_Keys(flight_data);
    size_t size = HashMap_GetSize(flight_data);
    for(int i = 0; i < size; i++) {
        if(strcmp(flight, flights[i]) == 0) {
            int num_seats = *((int *) HashMap_Get(flight_data, flights[i]));
            char *sing_or_plur = num_seats == 1 ? "seat" : "seats";
            sprintf(message, "\n%s: %d %s available\n\n", flight, num_seats, sing_or_plur);
            free(args); free(flights);
            pthread_mutex_unlock(&flights_mutex);
            return message;
        }
    }

    free(args); free(flights); free(message);
    pthread_mutex_unlock(&flights_mutex);
    return "\nFlight not found.\n\n";
}

char *reserve_seats(int fd, int argc, char **args) {
    if(argc < 2) {
        free(args);
        return "\nCommand Usage: RESERVE <name-of-flight> <number-of-seats>\n\n";
    }

    if(!is_logged_in(fd, "agency")) {
        free(args);
        return "\nYou must login using LOGON before executing this command\n\n";
    }

    char *flight = args[0];
    int num_seats;
    sscanf(args[1], "%d", &num_seats);

    char *message = reserve(flight, num_seats);

    free(args);
    return message;
}

char *reserve_return_seats(int fd, int argcnt, char **args) {
    if(argcnt < 2) {
        free(args);
        return "\nCommand Usage: RETURN <name-of-flight> <number-of-seats>\n\n";
    }

    if(!is_logged_in(fd, "agency")) {
        free(args);
        return "\nYou must login using LOGON before executing this command\n\n";
    }


    /* * * Reverse flight name information to get return flight * * */
    char *flight = args[0];
	char *from = malloc(4), *to = malloc(4);
	sscanf(flight, "%3s-%3s", from, to);

	char *return_flight = malloc(10);

	sprintf(return_flight, "%s-%s", to, from);

	free(from); free(to);
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    int num_seats;
    sscanf(args[1], "%d", &num_seats);

    char *message = reserve(return_flight, num_seats);

    free(args);
    return message;
}

char *reserve(char *flight, int num_seats) {
    pthread_mutex_lock(&flights_mutex);

    char *message = malloc(300);

    char **flights = HashMap_Keys(flight_data);
    size_t size = HashMap_GetSize(flight_data);
    for(int i = 0; i < size; i++) {
        if(strcmp(flight, flights[i]) == 0) {
            int *seats_avail = (int *) HashMap_Get(flight_data, flights[i]);
            char *sing_or_plur;
            if(num_seats > *seats_avail) {
                sing_or_plur = *seats_avail == 1 ? "seat" : "seats";
                sprintf(message, "\n%s: Only %d available %s left\n\n", flight, *seats_avail, sing_or_plur);
            } else {
                int *new_seats = malloc(sizeof(int));
                *new_seats = *seats_avail - num_seats;
                HashMap_Set(flight_data, flight, new_seats);
                free(seats_avail);

                sing_or_plur = num_seats == 1 ? "seat" : "seats";
                sprintf(message, "\n%s: %d %s reserved\n\n", flight, num_seats, sing_or_plur);
            }

            pthread_mutex_unlock(&flights_mutex);
            free(flights);
            return message;
        }
    }

    pthread_mutex_unlock(&flights_mutex);
    free(flights); free(message);
    return "\nCould not reserve seats. Flight not found.\n\n";
}

char *list_flights(int fd) {
    if(!is_logged_in(fd, "agency")) {
        return "\nYou must login using LOGON before executing this command\n\n";
    }

    char *flight_info = malloc(100000);

    char **flights = HashMap_Keys(flight_data);
    size_t size = HashMap_GetSize(flight_data);
    for(int i = 0; i < size; i++) {
        char single_flight[250];
        char *flight = flights[i];
        int num_seats = *((int *) HashMap_Get(flight_data, flight));
        char *sing_or_plur = num_seats == 1 ? "seat" : "seats";
        sprintf(single_flight, "\n%s: %d %s available\n\n", flight, num_seats, sing_or_plur);

        strcat(flight_info, single_flight);
    }

    free(flights);
    return flight_info;
}

char *list_available_flights(int fd, int argcnt, char **args) {
    if(!is_logged_in(fd, "agency")) {
        free(args);
        return "\nYou must login using LOGON before executing this command\n\n";
    }

    int n = 0;
    if(argcnt > 0)
        sscanf(args[0], "%d", &n);

    char *flight_info = malloc(100000);

    char **flights = HashMap_Keys(flight_data);
    size_t size = HashMap_GetSize(flight_data);
    int j = 0;
    for(int i = 0; i < size; i++) {
        char single_flight[250];
        char *flight = flights[i];
        int num_seats = *((int *) HashMap_Get(flight_data, flight));
        if(num_seats == 0)
            continue;

        if(n > 0 && j++ >= n) break;

        char *sing_or_plur = num_seats == 1 ? "seat" : "seats";
        sprintf(single_flight, "\n%s: %d %s available\n\n", flight, num_seats, sing_or_plur);

        strcat(flight_info, single_flight);
    }

    free(flights);
    return flight_info;
}

char *logoff(int fd) {
    if(!is_logged_in(fd, "agency")) {
        return "\nYou must login using LOGON before executing this command\n\n";
    }

	if(is_logged_in(fd, "chat")) 
		chat_exit(fd);


	pthread_mutex_lock(&users_mutex);
    char **usernames = HashMap_Keys(users);
    size_t size = HashMap_GetSize(users);

    for(int i = 0; i < size; i++) {
        struct client *user = (struct client *) HashMap_Get(users, usernames[i]);
        if(fd == user->fd) {
			pthread_mutex_unlock(&users_mutex);
            HashMap_Remove(users, usernames[i]);
            free(user); free(usernames);
            return "exit";
        }
    }
	pthread_mutex_unlock(&users_mutex);

    free(usernames);
    return NULL;
}

/* ██████╗██╗  ██╗ █████╗ ████████╗     ██████╗ ██████╗ ███╗   ███╗███╗   ███╗ █████╗ ███╗   ██╗██████╗ ███████╗
  ██╔════╝██║  ██║██╔══██╗╚══██╔══╝    ██╔════╝██╔═══██╗████╗ ████║████╗ ████║██╔══██╗████╗  ██║██╔══██╗██╔════╝
  ██║     ███████║███████║   ██║       ██║     ██║   ██║██╔████╔██║██╔████╔██║███████║██╔██╗ ██║██║  ██║███████╗
  ██║     ██╔══██║██╔══██║   ██║       ██║     ██║   ██║██║╚██╔╝██║██║╚██╔╝██║██╔══██║██║╚██╗██║██║  ██║╚════██║
  ╚██████╗██║  ██║██║  ██║   ██║       ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║██║  ██║██║ ╚████║██████╔╝███████║
   ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝        ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝*/

char *show_chat_help() {
    char *help_message = malloc(MAX_READ_SIZE);
	memset(help_message, 0, MAX_READ_SIZE);

	sprintf(help_message, "\n");
	srepeatprint(help_message, '-', 98);
	strcat(help_message, "|\n");
    strcat(help_message, "|  Chat Commands:\n");
	strcat(help_message, "|\n");
    strcat(help_message, "|\t  HELP --- Show this help message\n");
    strcat(help_message, "|\t  TEXT <message> --- Post the <message> text to the chat room\n");
    strcat(help_message, "|\t  LIST --- List all online members of the Toledo Travel Chat Room\n");
    strcat(help_message, "|\t  LIST_ALL --- List all the members of the Toledo Travel Chat Room\n");
    strcat(help_message, "|\t  LIST_OFFLINE --- List all offline members of the Toledo Travel Chat Room\n");
    strcat(help_message, "|\t  EXIT_CHAT --- Exit the Toledo Travel Chat Room\n");
	strcat(help_message, "|\n");
	srepeatprint(help_message, '-', 98);
	strcat(help_message, "\n");

    return help_message;
}


char *chat_login(int fd, int argc, char **args) {
    if(argc < 1) {
        free(args);
        return "\nCommand Usage: CHAT <username>\n\n";
    }

	if(!is_logged_in(fd, "agency")) {
		free(args);
	    return "\nYou must login using LOGON before executing this command\n\n";
	}

    char *message = malloc(1024);
	char *help = show_chat_help();
	strcat(message, "\nYou are now in the Toledo Travel chat room. Here is a list of helpful commands to get started\n\n");
	strcat(message, help);

	free(help);

	char *username = args[0];

    pthread_mutex_lock(&users_mutex);
	char **usernames = HashMap_Keys(users);
    size_t size = HashMap_GetSize(users);

	struct client *user = NULL;

	for(int i = 0; i < size; i++) {
		struct client *check_user = (struct client *) HashMap_Get(users, usernames[i]);	
		if(check_user->fd == fd) {
			user = (struct client *) HashMap_Get(users, usernames[i]);
			break;
		}
	}
	pthread_mutex_unlock(&users_mutex);

	if(chat_name_taken(username)) {
		char buff[150];
        sprintf(buff, "Username '%s' is taken.\n", username);
        strcat(message, buff);
		int i = 1;
		do {
            sprintf(username, "%s%d", args[0], i++);
		} while(chat_name_taken(username));
	} else {
		strcpy(user->chat_name, username);
		strcpy(user->chat_status, "online");
	}

	char buff[150];
    sprintf(buff, "Now chatting as '%s'\n\n", username);
	strcat(message, buff);

    free(args);

    return message;
}

char *write_to_chat(int fd, int argc, char **args) {
    if(argc < 1) {
        free(args);
        return "\nCommand Usage: TEXT <chat_text>\n\n";
    }

	if(!is_logged_in(fd, "chat")) {
		free(args);
		if(!is_logged_in(fd, "agency"))
	        return "\nYou must login using LOGON before executing this command\n\n";
		return "\nYou must enter the chatroom using CHAT before using this command\n\n";
    }

    char *chat_text = strjoin(args, argc);

    pthread_mutex_lock(&users_mutex);
    char **usernames = HashMap_Keys(users);
    size_t size = HashMap_GetSize(users);

    for(int i = 0; i < size; i++) {
        struct client *user = (struct client *) HashMap_Get(users, usernames[i]);
        if(fd == user->fd) {
			pthread_mutex_unlock(&users_mutex);

            char new_message[MAX_READ_SIZE];
			memset(new_message, 0, MAX_READ_SIZE);

            sprintf(new_message, "%s: %s\n", usernames[i], chat_text);

			pthread_mutex_lock(&messages_mutex);
			ArrayList_Add(message_queue, new_message);
			pthread_mutex_unlock(&messages_mutex);

			write_to_all(new_message);

            free(usernames); free(args);
            return "Chat message written";
        }
    }

    free(usernames); free(args);
    pthread_mutex_unlock(&users_mutex);
    pthread_mutex_unlock(&messages_mutex);

    return NULL;
}

char *list_online_chatters(int fd) {
    if(!is_logged_in(fd, "chat")) {
		if(!is_logged_in(fd, "agency"))
	        return "\nYou must login using LOGON before executing this command\n\n";
		return "\nYou must enter the chatroom using CHAT before using this command\n\n";
    }

    char *message = malloc(MAX_READ_SIZE);
	strcat(message, "\n");

    pthread_mutex_lock(&users_mutex);
    char **usernames = (char **) HashMap_Keys(users);
    size_t size = HashMap_GetSize(users);
    for(int i = 0; i < size; i++) {
        struct client *user = (struct client *) HashMap_Get(users, usernames[i]);
        if(strcmp(user->chat_status, "online") == 0) {
            char user_message[150];
            sprintf(user_message, "%s (%s)\n", user->chat_name, user->chat_status);
            strcat(message, user_message);
        }
    }

    pthread_mutex_unlock(&users_mutex);
    free(usernames);

	strcat(message, "\n\n");


    return message;
}

char *list_all_chatters(int fd) {
    if(!is_logged_in(fd, "chat")) {
		if(!is_logged_in(fd, "agency"))
	        return "\nYou must login using LOGON before executing this command\n\n";
		return "\nYou must enter the chatroom using CHAT before using this command\n\n";
    }

    char *message = malloc(MAX_READ_SIZE);
	strcat(message, "\n");

    pthread_mutex_lock(&users_mutex);
    char **usernames = (char **) HashMap_Keys(users);
    size_t size = HashMap_GetSize(users);
    for(int i = 0; i < size; i++) {
        struct client *user = (struct client *) HashMap_Get(users, usernames[i]);
        char user_message[150];
		if(strcmp(user->chat_name, "unnamed") == 0)
	       	sprintf(user_message, "(unnamed) (%s)\n", user->chat_status);
		else
			sprintf(user_message, "%s (%s)\n", user->chat_name, user->chat_status);

        strcat(message, user_message);
    }

    pthread_mutex_unlock(&users_mutex);
    free(usernames);

	strcat(message, "\n\n");


    return message;
}

char *list_offline_chatters(int fd) {
    if(!is_logged_in(fd, "chat")) {
		if(!is_logged_in(fd, "agency"))
	        return "\nYou must login using LOGON before executing this command\n\n";
		return "\nYou must enter the chatroom using CHAT before using this command\n\n";
    }

    char *message = malloc(MAX_READ_SIZE);
	strcat(message, "\n");

    pthread_mutex_lock(&users_mutex);
    char **usernames = (char **) HashMap_Keys(users);
    size_t size = HashMap_GetSize(users);
    for(int i = 0; i < size; i++) {
        struct client *user = (struct client *) HashMap_Get(users, usernames[i]);
        if(strcmp(user->chat_status, "offline") == 0) {
            char user_message[150];
            sprintf(user_message, "%s (%s)\n", user->chat_name, user->chat_status);
            strcat(message, user_message);
        }
    }
    pthread_mutex_unlock(&users_mutex);
    free(usernames);

	strcat(message, "\n\n");

    return message;
}

char *chat_exit(int fd) {
    if(!is_logged_in(fd, "chat")) {
		if(!is_logged_in(fd, "agency"))
	        return "\nYou must login using LOGON before executing this command\n\n";
		return "\nYou must enter the chatroom using CHAT before using this command\n\n";
    }

    pthread_mutex_lock(&users_mutex);
    char **usernames = (char **) HashMap_Keys(users);
    size_t size = HashMap_GetSize(users);
    for(int i = 0; i < size; i++) {
        struct client *user = (struct client *) HashMap_Get(users, usernames[i]);
        if(user->fd == fd) {
			strcpy(user->chat_status, "offline");
            free(usernames);
            pthread_mutex_unlock(&users_mutex);
            return "\nYou have left the chat room.\n\n";
        }
    }

    pthread_mutex_unlock(&users_mutex);
    free(usernames);

    return NULL;
}

/*███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗      ██████╗ ██████╗ ███╗   ███╗███╗   ███╗ █████╗ ███╗   ██╗██████╗ ███████╗
  ██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗    ██╔════╝██╔═══██╗████╗ ████║████╗ ████║██╔══██╗████╗  ██║██╔══██╗██╔════╝
  ███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝    ██║     ██║   ██║██╔████╔██║██╔████╔██║███████║██╔██╗ ██║██║  ██║███████╗
  ╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗    ██║     ██║   ██║██║╚██╔╝██║██║╚██╔╝██║██╔══██║██║╚██╗██║██║  ██║╚════██║
  ███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║    ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║██║  ██║██║ ╚████║██████╔╝███████║
  ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝     ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝*/

char *show_help() {
    char *help_message = malloc(MAX_READ_SIZE);
	memset(help_message, 0, MAX_READ_SIZE);

	strcat(help_message, "\n");
	srepeatprint(help_message, '-', 100);
	strcat(help_message, "|\n");
    strcat(help_message, "|  Server Commands:\n");
	strcat(help_message, "|\n");
    strcat(help_message, "|\t  HELP --- Show this help message\n");
    strcat(help_message, "|\t  LIST --- List all users connected to Toledo Travel\n");
    strcat(help_message, "|\t  LIST_CHAT --- List all members of the Toledo Travel Chat Room\n");
	strcat(help_message, "|\t  WRITE <filename> --- Write the current flight data to <filename>\n");
    strcat(help_message, "|\t  EXIT --- Quit the server instance\n");
	strcat(help_message, "|\n");
	srepeatprint(help_message, '-', 100);
	strcat(help_message, "\n");

    return help_message;
}

char *server_list_users() {
    char *message = malloc(MAX_READ_SIZE);
	char *header = "Connected Users";
    sprintheader(message, header);

    pthread_mutex_lock(&users_mutex);
    char **usernames = (char **) HashMap_Keys(users);
    size_t size = HashMap_GetSize(users);
	if(size == 0) {
		strcat(message, "There are currently no clients online\n\n");
	} else {
    	for(int i = 0; i < size; i++) {
        	char user_message[150];
			struct client *user = (struct client *) HashMap_Get(users, usernames[i]);
       	    sprintf(user_message, "%d %s\n", user->fd, usernames[i]);
       	    strcat(message, user_message);
    	}

		strcat(message, "\n");
	}

    pthread_mutex_unlock(&users_mutex);
    free(usernames);

    return message;
}

char *server_list_chat_users() {
    char *message = malloc(MAX_READ_SIZE);
	char *header = "Connected Chat Room Members";
    sprintheader(message, header);

    pthread_mutex_lock(&users_mutex);
    char **usernames = (char **) HashMap_Keys(users);
    size_t size = HashMap_GetSize(users);
	if(size == 0) {
		strcat(message, "There are currently no clients online\n\n");
	} else {
    	for(int i = 0; i < size; i++) {
        	char user_message[150];
			struct client *user = (struct client *) HashMap_Get(users, usernames[i]);
			if(strcmp(user->chat_name, "unnamed") == 0)
	        	sprintf(user_message, "%d (unnamed) (%s)\n", user->fd, user->chat_status);
			else
				sprintf(user_message, "%d %s (%s)\n", user->fd, user->chat_name, user->chat_status);

        	strcat(message, user_message);
		}

		strcat(message, "\n");
    }

    pthread_mutex_unlock(&users_mutex);
    free(usernames);

    return message;
}

char *write_server_output(int argc, char **args) {
    if(argc < 1) {
        free(args);
        return "\nCommand Usage: WRITE <filename>\n\n";
    }

	char *fname = args[0];
	FILE *outf;
	if((outf = fopen(fname, "w")) == NULL)
            die("\nCould not open file for writing.\n\n");

	pthread_mutex_lock(&flights_mutex);
	char **flights = HashMap_Keys(flight_data);
	size_t size = HashMap_GetSize(flight_data);

	for(int i = 0; i < size; i++) {
		int seats = *((int *) HashMap_Get(flight_data, flights[i]));
		fprintf(outf, "%s %d\n", flights[i], seats);
	}
	pthread_mutex_unlock(&flights_mutex);

	fclose(outf);

    free(args);
    return NULL;
}


/*██╗  ██╗███████╗██╗     ██████╗ ███████╗██████╗ ███████╗
  ██║  ██║██╔════╝██║     ██╔══██╗██╔════╝██╔══██╗██╔════╝
  ███████║█████╗  ██║     ██████╔╝█████╗  ██████╔╝███████╗
  ██╔══██║██╔══╝  ██║     ██╔═══╝ ██╔══╝  ██╔══██╗╚════██║
  ██║  ██║███████╗███████╗██║     ███████╗██║  ██║███████║
  ╚═╝  ╚═╝╚══════╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═╝╚══════╝*/


void read_flight_data(FILE *fd, HashMap *map) {
    char *key;
    char line[75];

    while(fgets(line, sizeof(line), fd) != NULL) {
        int *val = malloc(sizeof(int));
		key = malloc(32);
        sscanf(line, "%s %d", key, val);
        if(!HashMap_Set(map, key, val)) {
            fclose(fd); free(key);
            die("Error populating flight data\n");
        }
		free(key);
    }
    
    fclose(fd);
}

int is_logged_in(int fd, char *type) {
	pthread_mutex_lock(&users_mutex);

    char **usernames = HashMap_Keys(users);
    size_t size = HashMap_GetSize(users);

    for(int i = 0; i < size; i++) {
		struct client *user = (struct client *) HashMap_Get(users, usernames[i]);
        if(user->fd == fd) {
			pthread_mutex_unlock(&users_mutex);
			if(strcmp(type, "chat") == 0) {
				if(strcmp(user->chat_status, "online") == 0)
	            	return TRUE;
				return FALSE;
			} else {
            	return TRUE;
			}
		}
    }

	pthread_mutex_unlock(&users_mutex);	
    return FALSE;
}

int chat_name_taken(char *name) {

	pthread_mutex_lock(&users_mutex);
	char **usernames = HashMap_Keys(users);
    size_t size = HashMap_GetSize(users);

	for(int i = 0; i < size; i++) {
		struct client *user = (struct client *) HashMap_Get(users, usernames[i]);	
		if(user->chat_name != NULL && strcmp(user->chat_name, name) == 0) {
			pthread_mutex_unlock(&users_mutex);
			free(usernames);
			return TRUE;
		}
	}
	pthread_mutex_unlock(&users_mutex);

	free(usernames);	
	return FALSE;
}

void write_to_all(char *message) {
	pthread_mutex_lock(&users_mutex);

	char **usernames = (char **) HashMap_Keys(users);
	size_t size = HashMap_GetSize(users);

	for(int i = 0; i < size; i++) {
		struct client *user = (struct client *) HashMap_Get(users, usernames[i]);	
		if(write(user->fd, message, strlen(message)) < 0)
			fprintf(stderr, "Error writing to %s", usernames[i]);
	}

	pthread_mutex_unlock(&users_mutex);
}

/* ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗  ██╗ ██╗
  ████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗
  ╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝
  ████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗████████╗
  ╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝╚██╔═██╔╝
   ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝  ╚═╝ ╚═╝*/
