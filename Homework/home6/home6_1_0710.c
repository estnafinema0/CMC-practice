#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
/*Печать соответвующей информации организована в двух отдельных функциях. 
В main при необходимости можно сменить директорию для обхода.
*/
const char *MONTHS[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

char * getTime(time_t t);
void printSymlink(const char *path, const char *symlink);
void printExecuteFile(const char *path, const struct stat *file_stat);
void traverseDirectorie(const char *current_dir_path);

int main()
{
	const char * current_dir = "./Рабочий стол";
	//const char * current_dir = ".";

	traverseDirectorie(current_dir);
	
	return 0;
} 


char * getTime(time_t t)
{
	struct tm buffer;
	struct tm *tm = gmtime_r(&t, &buffer);
	if (tm == NULL) {
        fprintf(stderr, "[ERROR]: gmtime_r failed.\n");
        exit(1);
    }
	char *str = malloc(16 * sizeof(char));
	
	if (str == NULL) {
        fprintf(stderr, "[ERROR]: Memory allocation failed.\n");
        exit(1);
    }
    snprintf(str, 16, "%02d %s %4d", tm->tm_mday, MONTHS[tm->tm_mon], tm->tm_year + 1900);
    return str;
}

void printSymlink(const char *path, const char *symlink)
{
	char link_real_name[PATH_MAX];
	char full_path[PATH_MAX];
	
	snprintf(full_path, sizeof(full_path), "%s/%s", path, symlink);
	ssize_t nbytes = readlink(full_path, link_real_name, sizeof(link_real_name) - 1);
	if (nbytes != -1){
		link_real_name[nbytes] = '\0';
        printf("Symbolic link: %s -> %s\n", symlink, link_real_name);
	}
	else perror("readlink failed.");
	
	return;
}

void printExecuteFile(const char *path, const struct stat *file_stat)
{
	char * time_str = getTime(file_stat->st_mtime);
    printf("Executable file: %s\n	Last modified: %s\n", path, time_str);
    free(time_str);
	return;
}

void traverseDirectorie(const char *current_dir_path)
{
	struct dirent *entry;
	DIR * dir = opendir(current_dir_path);
	if (!dir){
		perror("[ERROR]: Opendir failed.");
		return;
	}
	
	while((entry = readdir(dir)) != NULL)
	{
		char full_path[PATH_MAX];
		snprintf(full_path, sizeof(full_path), "%s/%s", current_dir_path, entry->d_name);
		
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
			
		struct stat stat_info;
		if (lstat(full_path, &stat_info) == -1){
			perror("lstat failed.");
			return;
		}
			
		if (S_ISLNK(stat_info.st_mode))
			printSymlink(current_dir_path, entry->d_name);
		
		
		if (S_ISDIR(stat_info.st_mode)){
			traverseDirectorie(full_path);
		}
		if (S_ISREG(stat_info.st_mode) && (stat_info.st_mode & 0550) == 0550)
			printExecuteFile(full_path, &stat_info);
    
	}
	closedir(dir);
	return;
}
