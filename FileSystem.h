#define block_size 0x80
#define block_count 0x100
#define fs_size block_size*block_count

#define file_name_length 8

#define blockmap_size 64
#define desc_count 32


struct descriptor{
    char name[8];
    char block;
    char block2;
    char attributes;
    int size;
};


/**
 * Підключити файлову систему, збережену на диску
 */
void FS_mount ();
void FS_unmount();

/**
 * Вивести інформацію про дескриптор номер id
 */
void FS_filestat(int id);

/**
 * Вивести список файлів з номерами дескрипторів
 */
void FS_ls();

/**
 * Створити файл
 */
void FS_create(char *file_name);

void FS_mkdir(char *file_name);

/**
 * Прочитати дані з файлв
 * @param fd числовий дескриптор
 * @param offset 
 * @param length
 */
unsigned char* FS_read(char* file_name, int offset, int length);
void FS_write(char* file_name, int offset, int length, unsigned char* data);

/**
 * Змінити розмір файлу
 * @param file_name
 * @param size
 */
void FS_truncate(struct descriptor *file, int size);

/* Внутрішні функції*/
/**
 * Створити файл - файлову систему  
 */
void FS_init();


int FS_findFID(char* file_name);
/**
 * Записати інформацію в блок за номером
 * @param nblock номер
 * @param data інформацію
 */
void FS_write_block(int nblock, char *data,int start, int count);
void FS_write_block_char(int nblock, char data, int pos);
/**
 * Прочитати інформацію в блок за номером
 * @param nblock номер
 * @param data інформацію
 */
void FS_read_block(int nblock, char *data);
int FS_read_block_int(int nblock, int pos);

char FS_block_status(int nblock);

void FS_set_block_status(int nblock, char status);

unsigned char FS_get_free_block();


void FS_copy_descriptors(struct descriptor *desc, struct descriptor *new_desc);