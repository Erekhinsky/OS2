#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#define PROCFS_NAME_TASK "/proc/mykernelmodule/cputime"
#define PROCFS_NAME_INODE "/proc/mykernelmodule/inode"

char* get_newline(char* str) {

	int asciiVal = 10;
	
	for (int i = 0; i < strlen(str); i++) {
		
		if ((str[i] == '^')) {
			str[i] = asciiVal;
		}
	}

	return str;
}

int main() {

	FILE *KERN;

	int* module_selection;

	char args[100];
	int user_pid;

	char *pid_path;
	size_t pid_path_size = 50;	
	pid_path = (char *)malloc(pid_path_size);

	char *user_output;
	size_t user_output_size = 300;		

	while (1) {

		printf("\nEnter module: 1 - task_cputime ; 2 - inode ; 3 - exit: ");
		scanf("%d", module_selection);

		if (module_selection[0] == 1) {
				
			printf("\nEnter parameters: Process PID:");
			scanf("%d", &user_pid);
			
			sprintf(pid_path, "/proc/%d", user_pid);

			if (access(pid_path, F_OK) != 0) {
				printf("PID: %d not found\n", user_pid);
				free(pid_path);
				continue;
			}
			
			KERN = fopen(PROCFS_NAME_TASK, "r+");

			fprintf(KERN, "%d", user_pid);

			user_output = (char *)malloc(user_output_size);
				
			fgets(user_output, user_output_size, KERN);

			user_output = get_newline(user_output);

			printf("%s\n", user_output);

			free(user_output);

			fclose(KERN);
		}
		else {
			if (module_selection[0] == 2) {

				printf("\nEnter parameters: File path: ");
				scanf("%s", args);

				if (access(args, F_OK) != 0) {
					printf("Path: %s not found\n", args);
					continue;
				}

				KERN = fopen(PROCFS_NAME_INODE, "r+");
					
				fprintf(KERN, "%s", args);


				user_output = (char *)malloc(user_output_size);

				fgets(user_output, user_output_size, KERN);

				user_output = get_newline(user_output);

				printf("%s\n", user_output);

				free(user_output);

				printf("%s", user_output);

			} else {
				if (module_selection[0] == 3) {
					break;
				} else {
					printf("Error");
				}
			}
		}
	}

 
  	return 0;
}
