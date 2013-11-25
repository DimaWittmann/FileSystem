#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "FileSystem.h"

//TODO все дуже страшно, реалізувати доп процедури для пошуку файлів в директорій
//TODO запис даних відразу в декілька блоків
//TODO розділити на декілька файлів
char blockmap [blockmap_size];

struct descriptor root;
struct descriptor *curr_dir;
char *fs_name = "D:\\fs";



void FS_mount(){
    FILE *fs = fopen(fs_name, "rb");
    
    fread(&blockmap, sizeof(char), blockmap_size, fs);
    fread(&root, sizeof(struct descriptor), 1, fs);
    curr_dir = &root;
    
    fclose(fs);
}



void FS_unmount(){
    FILE *fs = fopen(fs_name, "rb+");
    
    fwrite(&blockmap, sizeof(char), blockmap_size, fs);
    fwrite(&root, sizeof(struct descriptor), 1, fs);
    
    fclose(fs);
}

void FS_mkdir(char* file_name){
    struct descriptor dot[2];
    
    dot[0].attributes = 0x2;
    memset(&dot[0].name,0 ,8);
    int i;
    for(i=0; i<8; i++){
        if(file_name[i] != 0){
            dot[0].name[i] = file_name[i];
        }else{
            break;
        }
    }
    dot[0].block = 0;
    dot[0].block2 = 0;
    dot[0].size = 0;
    

    dot[1].attributes = root.attributes;
    memset(&dot[1].name,0 ,8);
    dot[1].name[0] = '.';
    dot[1].name[1] = '.';
    dot[1].block = root.block;
    dot[1].block2 = root.block2;
    dot[1].size = root.size;
    
    FS_truncate(&dot[0], sizeof(struct descriptor)*2);
    
    FS_write_block(dot[0].block, dot, 0, sizeof(struct descriptor)*2);
    
    //TODO реалізувати для файлів більше одного блоку
    int old_size = curr_dir->size;
    if(curr_dir->size + sizeof(struct descriptor) < block_size){
        FS_truncate(curr_dir, old_size+sizeof(struct descriptor));
        for(i = old_size; i<old_size+sizeof(struct descriptor);i++){
            FS_write_block(curr_dir->block, &dot[0], old_size, sizeof(struct descriptor));
        }
    }
}



void FS_ls(){
    
}
//TODO об'єднати, позбавитися повторюваного коду
void FS_create(char* file_name){
    struct descriptor dot[2];
    
    dot[0].attributes = 0x1;
    memset(&dot[0].name,0 ,8);
    int i;
    for(i=0; i<8; i++){
        if(file_name[i] != 0){
            dot[0].name[i] = file_name[i];
        }else{
            break;
        }
    }
    dot[0].block = 0;
    dot[0].block2 = 0;
    dot[0].size = 0;
    
    //TODO реалізувати для файлів більше одного блоку
    int old_size = curr_dir->size;
    if(curr_dir->size + sizeof(struct descriptor) < block_size){
        FS_truncate(curr_dir, old_size+sizeof(struct descriptor));
        for(i = old_size; i<old_size+sizeof(struct descriptor);i++){
            FS_write_block(curr_dir->block, &dot[0], old_size, sizeof(struct descriptor));
        }
    }
}



void FS_write(char* file_name, int offset, int length, unsigned char* data){
    
    
    //TODO виділити окрему функцію пошуку 
    int block = curr_dir->block;
    void *df = calloc(block_size, 1);
    FS_read_block(block, df);
    int nfiles = curr_dir->size/sizeof(struct descriptor);
    
    int i;
    for(i=0; i<nfiles;i++){
        if(!strncmp(((struct descriptor*)df)[i].name, file_name, 4)){
            FS_truncate(&((struct descriptor*)df)[i], offset+length);
            FS_write_block(((struct descriptor*)df)[i].block, data, offset, length);
            break;
        }
    }
    
}

unsigned char* FS_read(char* file_name, int offset, int length){
    //TODO implement!)
}

void FS_truncate(struct descriptor *file, int size){
    int old_size = file->size;

    if(old_size == 0){
        int block_number = size/block_size + 1;
        file->block = FS_get_free_block();
        FS_set_block_status(file->block, 1);
        block_number--;
        
        if(block_number > 0){
            file->block2 = FS_get_free_block();
            FS_set_block_status(file->block2, 1);
            
            int i = 0;
            for(i=0; i<block_number;i++){
                char new_block = FS_get_free_block();
                FS_set_block_status(new_block, 1);
                FS_write_block_char(file->block2, new_block, i);
            }
        }
        file->size = size;
        return;
    }else{
        if(old_size/block_size == size/block_size){
            file->size = size;
            return;
        }else{
            //TODO реалізувати для зміни розміру файлу
        }
    }
    
}



/* Внутрішні функції*/


void FS_init(){
    
    FILE *fs = fopen(fs_name, "wb");
    
    
    FS_set_block_status(0, 1);
    fwrite(&blockmap, sizeof(char), blockmap_size, fs);
    fwrite(&root, sizeof(struct descriptor), 1, fs);
    
    void *block = calloc(block_count, block_size);
    memset(block, 0x0, block_count*block_size);
    fwrite(block, block_size, block_count, fs);
    free(block);
    fclose(fs);  
    
    void * data = calloc(sizeof(char), block_size);
    memset(data ,0xFF, block_size);
    FS_write_block(0, data, 0, block_size);
    
      
    
    FS_mount();
    
    root.attributes = 0x2;
    memset(&root.name,0 ,8);
    root.name[0] = 'r';
    root.name[1] = 'o';
    root.name[2] = 'o';
    root.name[3] = 't';
    root.block = 0;
    root.block2 = 0;
    root.size = 0;

    FS_truncate(&root, sizeof(struct descriptor)*2);
    
    struct descriptor dot[2];
    
    FS_copy_descriptors(&root, &dot[0]);

    dot[1].attributes = root.attributes;
    memset(&dot[1].name,0 ,8);
    dot[1].name[0] = '.';
    dot[1].name[1] = '.';
    dot[1].block = root.block;
    dot[1].block2 = root.block2;
    dot[1].size = root.size;
    
  
    FS_write_block(root.block, dot, 0, sizeof(struct descriptor)*2);
    
    FS_unmount();
    
}



char *FS_catStrings(const char* str1, const char *str2 ){
    char *buffer = calloc(strlen(str1)+strlen(str2)+1,sizeof(char));
    strcpy(buffer, str1);
    strcat(buffer, str2);
    return buffer;
}

void FS_write_block(int nblock, char *data,int start, int count){
    if(nblock > block_count-1){
        printf("Out of blocks");
        return;
    }
    
    FILE *fs_data= fopen(fs_name, "rb+"); 
    
    int status = fseek(fs_data, start+blockmap_size+sizeof(struct descriptor)+nblock*block_size, SEEK_SET);
    if(status != 0){
        printf("Error %d", status);
        return;
    }
    
    status = fwrite(data, sizeof(char), count, fs_data);
    if(status != count){
        printf("Writed not all data");
        return;
    }
        
    fclose(fs_data);
}

void FS_write_block_char(int nblock, char data, int pos){
    if(nblock > block_count-1){
        printf("Out of blocks");
        return;
    }
    if(pos > block_size-1){
        printf("Position of block");
        return;
    }
    
    FILE *fs_data= fopen(fs_name, "rb+"); 
    
    int status = fseek(fs_data, blockmap_size+sizeof(struct descriptor)+nblock*block_size + pos, SEEK_SET);
    if(status != 0){
        printf("Error %d", status);
        return;
    }
    
    fwrite(&data, sizeof(char), 1, fs_data);

    fclose(fs_data);
}

void FS_read_block(int nblock, char* data){
    if(nblock > block_count-1){
        printf("Out of blocks");
        return;
    }
    
    FILE *fs_data= fopen(fs_name, "rb+"); 
    
    int status = fseek(fs_data,blockmap_size+sizeof(struct descriptor)+ nblock*block_size, SEEK_SET);
    if(status != 0){
        printf("Error %d", status);
        return;
    }
    
    status = fread(data, sizeof(char), block_size, fs_data);
    if(status != block_size){
        printf("Writed not all data");
        return;
    }
        
    fclose(fs_data);
}


int FS_read_block_int(int nblock, int pos){
    if(nblock > block_count-1){
        printf("Out of blocks");
        return;
    }
    if(pos > block_size-1){
        printf("Position of block");
        return;
    }
    
    FILE *fs_data= fopen(fs_name, "rb+"); 
    
    int status = fseek(fs_data, blockmap_size+sizeof(struct descriptor)+nblock*block_size + pos, SEEK_SET);
    if(status != 0){
        printf("Error %d", status);
        return;
    }
    int data;
    fread(&data, sizeof(int), 1, fs_data);

    fclose(fs_data);
    return data;
}


char FS_block_status(int nblock){
    if(nblock > block_count-1){
        printf("Out of blocks");
        return;
    }
    
    
    char byte = nblock/8 - 1; 
    char bit = nblock%8 - 1;
    
    unsigned char mask = 0x1;
    mask = mask << bit;
    
    mask = mask & blockmap[byte];
    
    return mask;
}

void FS_set_block_status(int nblock, char status){
    if(nblock > block_count-1){
        printf("Out of blocks");
        return;
    }
    
    
    char byte = nblock/8; 
    char bit = nblock%8;
    
    if(status&0xFF != 0){
        status = 0x1;
    }else{
        status = 0x0;
    }
    
    status = status << bit;
    
    blockmap[byte] = status | blockmap[byte];
}



unsigned char FS_get_free_block(){
    int i = 0;
    while(blockmap[i] == 0xFF){
        i++;
        if(i == blockmap_size){
            printf("All block occupied");
            return;
        }
    }
    
    unsigned char j = 0;
    unsigned char bit = 0x1;
    while (bit & blockmap[i]){
        bit = bit << 1;
        j++;
    }
    
    return i*8 + j;
    
}

void FS_copy_descriptors(struct descriptor *desc, struct descriptor *new_desc){
    new_desc->attributes = desc->attributes;
    int i;
    for(i=0;i<8;i++){
        new_desc->name[i] = desc->name[i];
    }
    new_desc->block = desc->block;
    new_desc->block2 = desc->block2;
    new_desc->size = desc->size;
}