#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
 #include <dirent.h> 
 #include <sys/stat.h> 

int rightsFile(const char * s)
{
    
    char * a = "rwxrwxrwx";
    char * a1 = "qwe";
    int size = strlen(a);
    
    int size1 = sizeof(a1) - 1;
    int len;
    if ((len = strlen(s)) != size){
        fprintf(stderr, "-1\n");
        exit(1);    
    }
    char modes[ ] = {'r', 'w', 'x'};
    int answer = 0;
    for (int i = 0; i < len; i++){
        if ((modes[i% size1]!= s[i]) && (s[i] != '-')){
        fprintf(stderr, "-1\n");
        exit(1);    
    }
    char c = (modes[i%3]== s[i]) ?1:0;
    answer = answer *2 +c;
    
    }
    return answer;
}

char *int_to_rwx(int mode) 
{ 
    if (mode < 0 || mode > 0777) { return NULL; } 
    char *rwx = (char *)malloc(10); 
    int y = 64;
    for (int i = 0; i<9;i+=3) {
        
     if (mode & (4*y)) rwx[i] = 'r'; else rwx[i] = '-'; 
     if (mode & 2*y) rwx[i+1] = 'w'; else rwx[i+1] = '-'; 
     if (mode & 1*y) rwx[i+2] = 'x'; else rwx[i+2] = '-'; 
    y=y>>3;
 }
     rwx[9] = '\0'; 
     return rwx; 
}/*
int rwx_to_int(const char *str) {
    if (str == NULL || strlen(str) != 9) { return -1; } 
    int mode = 0; 
    
    int y = 64;
    for (int i = 0; i<9;i+=3) {
    if (str[0] == 'r') mode |= 4; 
    if (str[1] == 'w') mode |= 2; 
    if (str[2] == 'x') mode |= 1; 
      y=y>>3;
 }
    return mode; 
} 
*/

void task2(const char *path) 
{ 
    DIR *dir = opendir(path); 
    if (dir == NULL) { 
        perror("opendir"); 
        return;
    } 
    struct dirent *entry; 
    struct stat file_stat; 
    char full_path[PATH_MAX]; 
    while ((entry = readdir(dir)) != NULL) { 
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name); 
        if (stat(full_path, &file_stat) == -1) { 
            perror("stat"); 
            continue; 
        } 
        char *type = NULL;
        if (S_ISREG(file_stat.st_mode)) type = "regular";
        else
        if (S_ISLNK(file_stat.st_mode)) type = "LInked";
        else if (S_ISDIR(file_stat.st_mode)) type = "directory";
        else type = "else";
        
        printf("Name: %s, Pid: %ld, Mode: %s, Type: %s\n", entry->d_name,file_stat.st_ino, 
            int_to_rwx(file_stat.st_mode & 0777),  type); 
        }closedir(dir); 
} 
        
int list_files(const char *path) 
{ 
    int cnt = 0;
    DIR *dir = opendir(path); 
    if (dir == NULL) { perror("opendir"); return -1; } 
    struct dirent *entry; 
    struct stat file_stat; 
    char full_path[1024]; 
    while ((entry = readdir(dir)) != NULL) { 
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name); 
        if (stat(full_path, &file_stat) == -1) { 
            perror("stat"); 
            continue; 
        } 
        if (file_stat.st_size < 1024 && S_ISREG(file_stat.st_mode) && (entry->d_name[strlen(entry->d_name)-1] == 'c') && (entry->d_name[strlen(entry->d_name)-2] == '.')) { 
            cnt++;
            printf("Name: %s, Size: %ld bytes\n", entry->d_name, file_stat.st_size); 
            } 
    } 
        closedir(dir);
        return cnt;
} 
int main(int argc, char *argv[]) { 
    if (argc != 2) { 
        fprintf(stderr, "Usage: %s <path>\n", argv[0]); 
        return 1; } 
    char * a = "-w-ruxr-x";
    //printf("%o\n", rightsFile(a));
    
    char * a1 = "qwe";
    int size1 = sizeof(a1)/2 - 1;
    printf("%d\n", size1);
    //task2(argv[1]); 
    //printf("Number of files: %d\n", list_files(argv[1])); 
    
return 0; 
}
