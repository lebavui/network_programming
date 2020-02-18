#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <windows.h>

int main()
{
    /*LPWIN32_FIND_DATAA data;
    
    FindFirstFileA()*/

    struct stat fileStat;
    
    stat("C:\\Test\\hello.txt", &fileStat);

    printf("---------------------------\n");
    printf("File Size: \t\t%d bytes\n", fileStat.st_size);
    printf("Number of Links: \t%d\n", fileStat.st_nlink);
    printf("File inode: \t\t%d\n", fileStat.st_ino);

    printf("File Permissions: \t\t%d\n", fileStat.st_mode);

    return 0;
}