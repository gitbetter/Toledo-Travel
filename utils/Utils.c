#include "Utils.h"

void sprintheader(char *str, char *message) {
    size_t message_length = strlen(message);
	char *buff = malloc(message_length + 18);

    srepeatprint(str, '=', (int) message_length + 16);

    sprintf(buff, "\t%s\t\n", message);
	strcat(str, buff);

    srepeatprint(str, '=', (int) message_length + 16);

    strcat(str, "\n");
	free(buff);
}

void srepeatprint(char *str, char c, int times) {
	char buff[3];
 	for(int i = 0; i < times; i++) {
    	if(times - 1 == i)
			sprintf(buff, "%c\n", c);
    	else
			sprintf(buff, "%c", c);

		strcat(str, buff);
    }
}

void die(char *err_message) {
    perror(err_message);
    exit(EXIT_FAILURE);
}

void printusage(char *prog_name, char *type) {
    if(strcmp(type, "server") == 0)
        fprintf(stderr, "Usage: %s.exe IP StartingPort NumberOfPorts InputFile [OutputFile]\n", prog_name);
    else
        fprintf(stderr, "Usage: %s.exe IP [Port]\n", prog_name);
}

int isnumeric(const char *num) {
    for(int i = 0; num[i] != '\0'; i++)
        if(!isdigit(num[i]))
            return 0;

    return 1;
}

void strtolower(char *str) {
    char *p = str;
	for( ; *p; ++p) *p = tolower(*p);
}

void strtoupper(char *str) {
    char *p = str;
    while(*p) {
        *p = toupper(*p);
        ++p;
    }
}

int containsstr(const char *arr[], size_t size, char *str) {
    for(int i = 0; i < size; i++) {
        if(strcmp(arr[i], str) == 0)
            return TRUE;
    }

    return FALSE;
}

char *trimws(char *str) {
	while(isspace(*str)) str++;

	if(*str == '\0')
		return str;

	char *end = str + strlen(str) - 1;

	while(end > str && isspace(*end)) end--;

	*(end+1) = '\0';

	return str;
}

int charcount(char *str, char c) {
	char *p = str;
	int i;
	for(i = 0; p[i]; p[i] == c ? i++ : *p++);
	return i;
}

char *strjoin(char **parts, int num_parts) {
	if(parts[0] == NULL)
		return NULL;

	int current_size = strlen(parts[0]);
	char *result = (char *) malloc(current_size);
	memset(result, 0, current_size);
	strcat(result, parts[0]);
	for(int i = 1; i < num_parts; i++) {
		current_size += strlen(parts[i]) + 1;
		result = realloc(result, current_size); 	
		strcat(result, " ");
		strcat(result, parts[i]);
	}
	strcat(result, "\n");

	return result;
}
