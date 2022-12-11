#include <stdio.h>
#include <stdlib.h>

#define PROCFS_NAME_TASK "/proc/mykernelmodule/cputime"
#define PROCFS_NAME_INODE "/proc/mykernelmodule/inode"

FILE *KERN;
char *module_selection;
char *args;
int user_pid;
char *user_output;

enter_module:
	scanf("Enter module: 1 - task_cputime ; 2 - inode: %s", &module_selection);

	if (module_selection == "1") {
		
		scanf("Enter parameters: Process PID: %s", &args);
		
		user_pid = atoi(args);
		
		KERN = fopen(PROCFS_NAME_TASK, "r+");
		
		fprintf(KERN, "%d", user_pid);
		
		fscanf(KERN, "%s", &user_output);
		
		fclose(KERN);
		
		printf(&user_output);
	}
	else {
		if (module_selection == "2") {
			scanf("Enter parameters: File path: %s", &args);
			
			KERN = fopen(PROCFS_NAME_INODE, "r+");
			
			fprintf(KERN, "%d", args);
			
			fscanf(KERN, "%s", &user_output);
			
			fclose(KERN);
			
			printf(&user_output);
		} 
		else {
			goto enter_module;
		}
	}