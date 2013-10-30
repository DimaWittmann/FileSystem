#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "FileSystem.h"

/**
 * Карта занятості блоків
 */
unsigned char blockmap [block_size];

/**
 * Відображення числових дескрипторів відкритим файлам
 */
int open_filesmap [max_opened_files];

/**
 * Карта дескрипторів
 */
struct descriptor descmap [desc_count];

/**
 * Карта створених файлів
 */
struct link filemap [max_file_count];

/**
 * Посилання на блоки
 */
struct block *blocks;

FILE *fs_info;
FILE *fs_data;
char *fs_name = 0;

char *info_ext = ".info";
char *data_ext = ".data";


void FS_mount(char *file_name){
    fs_name = calloc(20, sizeof(char));
    strcpy(fs_name, file_name);
    
    char *buffer = FS_catStrings(file_name, info_ext);
    fs_info = fopen(buffer, "rb");
    free(buffer);
    
    if(!fs_info){
        FS_save_FS(file_name);
    }
    FS_open_FS(file_name);
}



void FS_unmount(){
    if (fs_name == 0){
        printf("FS is not mounted\n");
    }else{
        FS_save_FS(fs_name);
        free(fs_name);
        fs_name = 0;
    }
    memset(&blockmap,0,sizeof(char)*block_size);
    memset(&open_filesmap,0,sizeof(int)*max_opened_files);
    memset(&descmap,0,sizeof(struct descriptor)*desc_count);
    memset(&filemap,0,sizeof(struct link )*max_file_count);
    
}



void FS_filestat(int id){
    printf("N %d: type: %d links: %d size: %d \n", id, descmap[id].type, descmap[id].link_count, descmap[id].size);
}

void FS_ls(){
    int i;
    for(i=0;i<max_file_count;i++){
        if(filemap[i].file_name[0] != 0){
            printf("file: %s, desc: %d\n", filemap[i].file_name, filemap[i].desc_id);
        }
    }
}

void FS_create(char* file_name){
    
    if(FS_findFID(file_name)> -1){
        fprintf(stderr, "%s is existing\n", file_name);
        return;
    }
    
    int i = 0;
    while(i < desc_count){
        if(descmap[i].type == 0){
            break;
        }
        i++;
    }
    
    if(i == desc_count){
        fprintf(stderr, "All descriptors are employed\n");
        return;
    }
    int j = 0;
    while(j < max_file_count){
        if(filemap[j].file_name[0] == 0){
            break;
        }
        j++;
    }
    if(j == max_file_count){
        fprintf(stderr, "Can`t create files more. Delete old files\n");
        return;
    }
    
    strcpy(&filemap[j].file_name, file_name);
    filemap[j].desc_id = i; 
    descmap[i].type = 1;
    descmap[i].link_count = 1;
}

int FS_open(char* file_name){
    
    int file_i = 0;
    for(file_i=0;file_i<max_file_count;file_i++){
        if(!strcmp(filemap[file_i].file_name, file_name)){
            break;
        }

    }
    
    int open_file_i = 0;
    while (open_file_i < max_opened_files){
        if(open_filesmap[open_file_i] == 0){
            break;
        }
        open_file_i++;
    }
    open_filesmap[open_file_i] = file_i+1;    //щоб виділяти дескриптор на нульовий файл
    
    return open_file_i;
}

void FS_close(int id){
    open_filesmap[id] = 0;   
}


void FS_write(int fd, int offset, int length, unsigned char* data){
    int file_id = open_filesmap[fd]-1;                  //див open()
    if(filemap[file_id].file_name[0] == 0){
        fprintf(stderr, "Can`t find file\n");
        return;
    }
    
    int offset_in_block = offset;
    int curr_block = 0;
    while (offset_in_block > block_size){
        curr_block++;
        offset_in_block -= block_size;
    }
    
    FS_expandFile(file_id, offset, length);
    
    while (length > 0){
        int i = 0;
        int curr_block_id = descmap[filemap[file_id].desc_id].map.blocks[curr_block]-1; //додата 1 при віділенні блоку
        while(offset_in_block < block_size){
            blocks[curr_block_id].data[offset_in_block] = data[i];
            offset_in_block++;
            i++;
            length--;
            if(length == 0){
                break;
            }
        }
        
        offset_in_block = 0;
        curr_block++ ;
    }
    
}

unsigned char* FS_read(int fd, int offset, int length){
    int file_id = open_filesmap[fd]-1;                  //див open()
    if(filemap[file_id].file_name[0] == 0){
        fprintf(stderr, "Can`t find file\n");
        return;
    }
    
    int offset_in_block = offset;
    int curr_block = 0;
    while (offset_in_block > block_size){
        curr_block++;
        offset_in_block -= block_size;
    }
    
    unsigned char* data = calloc(length, sizeof(unsigned char));
    
    while (length > 0){
        int i = 0;
        int curr_block_id = descmap[filemap[file_id].desc_id].map.blocks[curr_block]-1; //додата 1 при віділенні блоку
        while(offset_in_block < block_size){
            data[i] = blocks[curr_block_id].data[offset_in_block];
            offset_in_block++;
            i++;
            length--;
            if(length == 0){
                break;
            }
        }
        
        offset_in_block = 0;
        curr_block++ ;
    }
    return data;
    
}

void FS_link(char *existing_file, char *new_file){
    int file_id = FS_findFID(existing_file);
    if(file_id < 0){
        fprintf(stderr, "%s is not existing\n", existing_file);
        return;
    }
    
    if(FS_findFID(new_file) > -1){
        fprintf(stderr, "%s is existing\n", new_file);
        return;
    }
    
    int j = 0;
    while(j < max_file_count){
        if(filemap[j].file_name[0] == 0){
            break;
        }
        j++;
    }
    if(j == max_file_count){
        fprintf(stderr, "Can`t create files more. Delete old files\n");
        return;
    }
    strcpy(&filemap[j].file_name, new_file);
    
    int desc_id = filemap[file_id].desc_id;
    filemap[j].desc_id = desc_id; 
    descmap[desc_id].link_count++;
}


void FS_unlink(char *file_name){
    int file_id = FS_findFID(file_name);
    if(file_id < 0){
        fprintf(stderr, "%s is not existing\n", file_name);
        return;
    }
    int desc_id = filemap [file_id].desc_id;
    
    filemap [file_id].desc_id = 0;
    int i =0;
    for(i=0;i<file_name_length;i++){
        filemap[file_id].file_name[i]=0;
    }
    descmap[desc_id].link_count--;
    if(descmap[desc_id].link_count == 0){
        int i;
        for (i=0;i<file_block_count;i++){
            int block_id = descmap[desc_id].map.blocks[i]-1;
            if (block_id >= 0){
                blockmap[block_id] = 0;
                descmap[desc_id].map.blocks[i] = 0;
            }
        }
        descmap[desc_id].size = 0;
        descmap[desc_id].type = 0;
        
    }
}



void FS_truncate(char *file_name, int size){
    int file_id = FS_findFID(file_name);
    if(file_id < 0){
        fprintf(stderr, "%s is not existing\n", file_name);
        return;
    }
    
    int desc_id = filemap[file_id].desc_id;
    int old_size = descmap[desc_id].size;
    if(old_size < size){
        FS_expandFile(file_id, 0, size);
    }else{
        int new_block_count = ceil((double)size/block_size);
        int old_block_count = ceil((double)old_size/block_size);
        if(new_block_count > 4){
            fprintf(stderr, "Can`t find file\n");
            return;
        }
        if(new_block_count < old_block_count){
            int i = old_block_count;
            while(new_block_count<i){
                int block_id = descmap[filemap[file_id].desc_id].map.blocks[i]-1;
                if(block_id >0){
                    blockmap[block_id] = 0;
                    descmap[desc_id].map.blocks[i] = 0;
                }
                i--;
            }
        }
        descmap[filemap[file_id].desc_id].size = size;
    }
}


/* Внутрішні функції*/

void FS_save_FS(char *fs_name){

    char *buffer = FS_catStrings(fs_name, info_ext);
    fs_info = fopen(buffer, "wb");
    free(buffer);

    fwrite(&blockmap ,sizeof(char), block_count, fs_info);

    fwrite(&open_filesmap ,sizeof(int), max_opened_files, fs_info); 
    
    fwrite(&descmap ,sizeof(struct descriptor), desc_count, fs_info);
    
    fwrite(&filemap ,sizeof(struct link), max_file_count, fs_info);
    
    
    buffer = FS_catStrings(fs_name, data_ext);
    fs_data= fopen(buffer, "wb");
    free(buffer);    

    if(blocks == NULL){
        blocks = calloc(block_count,sizeof(struct block));
    }
    
    fwrite(blocks, sizeof(struct block), block_count, fs_data);

    fclose(fs_data);
    fclose(fs_info);
}


void FS_open_FS(char *file_name){
    char *buffer = FS_catStrings(file_name, info_ext);
    fs_info = fopen(buffer, "rb");
    free(buffer);
    
    fread(blockmap ,sizeof(char), block_count, fs_info);

    fread(open_filesmap ,sizeof(int), max_opened_files, fs_info); 
    
    fread(descmap ,sizeof(struct descriptor), desc_count, fs_info);
    
    fread(filemap ,sizeof(struct link), max_file_count, fs_info);
    
    fclose(fs_info);
    
    buffer = FS_catStrings(file_name, data_ext);
    fs_data = fopen(buffer, "rb");
    free(buffer);   
    
    blocks = malloc(sizeof(struct block)*block_count);
    
    fread(blocks, sizeof(struct block), block_count, fs_data);
    fclose(fs_data);
}

char *FS_catStrings(const char* str1, const char *str2 ){
    char *buffer = calloc(strlen(str1)+strlen(str2)+1,sizeof(char));
    strcpy(buffer, str1);
    strcat(buffer, str2);
    return buffer;
}

void FS_expandFile(int file_id, int offset, int length){
    int new_size = offset + length;
    int old_size = descmap[filemap[file_id].desc_id].size; 
    if(old_size < new_size){
        int new_block_count = ceil((double)new_size/block_size);
        int old_block_count = ceil((double)old_size/block_size);
        if(new_block_count > 4){
            fprintf(stderr, "Can`t find file\n");
            return;
        }
        if(new_block_count > old_block_count){
            int i = old_block_count;
            do{
                int addr_new_block = 0;
                while(blockmap[addr_new_block] ){
                    addr_new_block++;
                    if(addr_new_block == block_count){
                        fprintf(stderr, "Can`t find free block\n");
                        return;
                    }
                }
                descmap[filemap[file_id].desc_id].map.blocks[i] = addr_new_block + 1; //щоб відокремити не зайняті блоки
                blockmap[addr_new_block] = 1;
                i++;
            }while (i < new_block_count);
        }
        descmap[filemap[file_id].desc_id].size = new_size;
    }
}

int FS_findFID(char* file_name){
    int i;
    
    for(i=0; i< max_file_count;i++){
        if(!strcmp(filemap[i].file_name, file_name)){
            return i;
        }
    }
    return -1;
}