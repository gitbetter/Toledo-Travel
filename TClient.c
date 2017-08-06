#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "utils/Utils.h"

#define START_PORT      9000

const char *agency_commands[] = {"query", "reserve", "return", "list", "chat", "list_available"};
const char *chat_commands[] = {"text", "list", "list_all", "list_offline", "exit_chat"};

char *show_help(void);
int connect_to_server(struct sockaddr *);
char *server_command_interpret(char *);
void *receive_loop(void *args);

int main(int argc, char **argv) {
    struct sockaddr_in conn_addr;
    int start_port;
    int fd = 0;
	pthread_t rec_thread;

	int is_chatting = FALSE;

    if(argc < 2) {
        printusage(argv[0], "client");
        exit(EXIT_FAILURE);
    }

    if(argc > 2) {
        if(!isnumeric(argv[2])) {
            fprintf(stderr, "[ERROR]: Port argument must be a number. Received '%s' instead\n", argv[2]);
            printusage(argv[0], "client");
            exit(EXIT_FAILURE);
        }
        sscanf(argv[2], "%d", &start_port);
    } else {
        srand(time(NULL));
        int random_in_range = rand() % MAX_PORTS;
        start_port = START_PORT + random_in_range;
    }

	printf("\nWelcome to Toledo Travel, where your next destination is at your fingertips. Here is a helpful list of commands to get started..\n\n");
	char *help = show_help();
	printf("%s", help);
	free(help);

    memset(&conn_addr, 0, sizeof(struct sockaddr_in));
    conn_addr.sin_family = AF_INET;
    conn_addr.sin_port = htons(start_port);
    inet_pton(AF_INET, argv[1], &conn_addr.sin_addr);

    char *command_buff = (char *) malloc(MAX_READ_SIZE);
    while(TRUE) {
        memset(command_buff, 0, MAX_READ_SIZE);

        fgets(command_buff, MAX_READ_SIZE, stdin);

		if(*command_buff == '\n')
			continue;

		trimws(command_buff);

		char *command = (char *) malloc(strlen(command_buff) + 1);
		strcpy(command, command_buff);

        char *com = strtok(command, " \t");
		if(com == NULL)
			com = command_buff;
			
		strtolower(com);

        if(strcmp(com, "help") == 0) {
            if(fd) {
				write(fd, command_buff, strlen(command_buff) + 1);
            } else {
                char *help = show_help();
                printf("%s", help);
            }
        }

		else if(containsstr(chat_commands, sizeof(chat_commands) / sizeof(char *), com) || containsstr(agency_commands, sizeof(agency_commands) / sizeof(char *), com)) {
			if(fd) {
				if(strcmp(com, "chat") == 0)
					is_chatting = TRUE;
				else if(strcmp(com, "exit_chat") == 0)
					is_chatting = FALSE;

				write(fd, command_buff, strlen(command_buff) + 1);
			} else {
				printf("You must log in using LOGON before using that command.\n");
			}
        }

        else if(strcmp(com, "logon") == 0) {
            if(!fd) {
                fd = connect_to_server((struct sockaddr *) &conn_addr);
				write(fd, command_buff, strlen(command_buff) + 1);

				pthread_create(&rec_thread, NULL, receive_loop, (void *) &fd);
            } else {
                printf("You are already logged on to Toledo Travel\n");
            }
        }

        else if(strcmp(com, "logoff") == 0) {
			if(!fd) {
				printf("You must log in using LOGON before using that command.\n");
				continue;
			}

			write(fd, command_buff, strlen(command_buff));
            close(fd);
            fd = 0;

			printf("Disconnected from server\n");
        }

        else if(strcmp(com, "exit") == 0) {
			if(fd)
				write(fd, "LOGOFF", 6);

            if(!is_chatting) {
				free(command);
				break;
			} 
        }

		else {
            printf("Invalid command. Type `HELP` for a list of available options.\n");
        }

		free(command);
    }

	pthread_exit(0);
}

int connect_to_server(struct sockaddr *server_addr) {
    int cfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(cfd == -1) die("Error opening chat socket\n");

    if(connect(cfd, (struct sockaddr *) server_addr, sizeof(struct sockaddr_in)) < 0)
        die("Error connecting client to server");

    return cfd;
}

char *show_help() {
    char *help_message = malloc(MAX_READ_SIZE);
	char *delim = malloc(100);
	
	srepeatprint(delim, '-', 98);
	
	strcat(help_message, delim);
	strcat(help_message, "|\n");
    strcat(help_message, "|  TClient Commands:\n");
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
	strcat(help_message, delim);
	strcat(help_message, "\n");

	free(delim);

    return help_message;
}

void *receive_loop(void *args) {
	int cfd = *((int *) args);

	ssize_t read_size;
	char message[MAX_READ_SIZE];
	memset(message, 0, MAX_READ_SIZE);
	while((read_size = recv(cfd, message, MAX_READ_SIZE, 0)) > 0) {
        if(strcmp(message, "exit") == 0) 
			break;

		printf("%s", message);

		memset(message, 0, MAX_READ_SIZE);
	}

	return NULL;
}


