#include <stdio.h>#include "FileSystem.h"int main(){    /*FS_mount("D:\\FS");        FS_create("file6");    int fid = FS_open("file6");    char * data = FS_read(fid, 0, 10);    printf(data);        FS_link("file6", "ololo");    int fid1 = FS_open("ololo");    data = FS_read(fid, 0, 10);    printf(data);        FS_ls();    FS_close(fid);    FS_close(fid1);    FS_unmount();     */    char command [10];    char argument1 [50];    char argument2 [50];    while(strncmp(command, "exit", 4)){        scanf("%s", &command);                if(!strncmp(&command, "mount", 5)){            printf("FS_mount(char * file_name) \n");            scanf("%s", &argument1);            FS_mount(argument1);        }else        if(!strncmp(&command, "unmount", 5)){            printf("FS_unmount \n");            FS_unmount();        }else        if(!strncmp(&command, "filestat", 8)){            printf("FS_filestat(int id) \n");            int fd;            scanf("%i", &fd);            FS_filestat(fd);        }else        if(!strncmp(&command, "ls", 2)){            printf("FS_ls \n");            FS_ls();        }else        if(!strncmp(&command, "open", 4)){            printf("FS_open(char *file_name) \n");            scanf("%s", &argument1);            int i = FS_open(argument1);            printf("%i \n", i);        }else        if(!strncmp(&command, "close", 5)){            printf("FS_close(int id) \n");            int fd;            scanf("%i", &fd);            FS_close(fd);        }else        if(!strncmp(&command, "read", 4)){            printf("FS_read(int fd, int offset, int length) \n");            int fd;            int offset;            int length;            scanf("%i %i %i", &fd, &offset, &length);                        char *data = FS_read(fd, offset, length);            int i;            for(i=0;i<length;i++){                putchar(data[i]);            }            putchar('\n');            free(data);        }else        if(!strncmp(&command, "write", 5)){            printf("FS_write(int fd, int offset, int length, unsigned char* data) \n");            int fd;            int offset;            int length;            scanf("%i %i %i", &fd, &offset, &length);                        char *data = calloc(length, sizeof(char));            scanf("%s", data);            FS_write(fd, offset, length, data);            free(data);        }else        if(!strncmp(&command, "link", 4)){            printf( "FS_link(char *existing_file, char *new_file) \n");            scanf("%s %s", &argument1, argument2);            FS_link(argument1, argument2);        }else        if(!strncmp(&command, "unlink", 6)){            printf( "FS_unlink(char *file_name) \n");            scanf("%s", &argument1);            FS_unlink(argument1);        }else        if(!strncmp(&command, "truncate", 8)){            printf( "FS_truncate(char *file_name, int size) \n");            int size;            scanf("%s %i", &argument1, &size);            FS_truncate(argument1, size);        }else{            printf("%s is unknow command\n", &command);        }    }}