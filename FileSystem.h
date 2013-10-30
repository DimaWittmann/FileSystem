#define block_size 16
#define block_count 16
#define fs_size block_size*block_count

#define file_name_length 8
#define file_block_count 4
#define max_file_size file_block_count*block_size

#define max_opened_files 8

#define blockmap_size block_count/32
#define desc_count 16

#define max_file_count desc_count*2

struct block{
    unsigned char data[block_size];
};

struct blockmap{
    int blocks[file_block_count];
};

struct descriptor{
    int type;  //0 - невизначений, 1 -файл, 2 - директорія
    int link_count;
    int size;
    struct blockmap map;
};


struct link{
    char file_name[file_name_length+1];
    int desc_id;
};
 

/**
 * Підключити файлову систему, збережену на диску
 */
void FS_mount (char* file_name);
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

/**
 * Відкрити файл для роботи
 * @param file_name
 * @return числовий дексриптор для роботи з цим файлом
 */
int FS_open(char *file_name);
void FS_close(int id);

/**
 * Прочитати дані з файлв
 * @param fd числовий дескриптор
 * @param offset 
 * @param length
 */
unsigned char* FS_read(int fd, int offset, int length);
void FS_write(int fd, int offset, int length, unsigned char* data);

/**
 * Створення посилання на існуючий файл
 * @param existing_file
 * @param new_file
 */
void FS_link(char *existing_file, char *new_file);
void FS_unlink(char *file_name);
/**
 * Змінити розмір файлу
 * @param file_name
 * @param size
 */
void FS_truncate(char *file_name, int size);

/* Внутрішні функції*/
/**
 * Створити файл - файлову систему  
 */
void FS_save_FS();

/**
 * Відкрити файлову систему
 */
void FS_open_FS(char *file_name);

/**
 * Об'єднування двох String
 * @param str1 
 * @param str2
 * @return
 */
char *FS_catStrings(const char *str1, const char *str2);
/**
 * Розширити файл для можливості запису нової інформації
 * @param file_id
 * @param offset
 * @param length
 */
void FS_expandFile(int file_id, int offset, int length);
/**
 * Пошук номер файлу за іменем файлу в таблиці імен файлів
 * @param file_name
 * @return числовий дексриптор, або -1 одиницю у випадку відсутності
 */
int FS_findFID(char* file_name);
/**
 * Записати інформацію в блок за номером
 * @param nblock номер
 * @param data інформацію
 */
void FS_write_block(int nblock, char *data);
/**
 * Прочитати інформацію в блок за номером
 * @param nblock номер
 * @param data інформацію
 */
void FS_read_block(int nblock, char *data);

