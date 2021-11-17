#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <ftw.h>
#ifndef _WIN32
# define _XOPEN_SOURCE 700 /* 700 */
# include <fnmatch.h>
#else
# include <windows.h>
# include <fileapi.h>
#endif

#include "platform.h"
#include "rxaradd.h"

int addBinaries(VFILE *library, VFILE *binaries[]) {

    char pattern[] = "*.rxbin";

#ifndef _WIN32
    int r;
    DIR *d;
    struct dirent *dir;

    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            r = fnmatch(pattern, dir->d_name, 0);
            if( r==0 )
                printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
#else
    HANDLE fileHandle;
    WIN32_FIND_DATA ffd;

    fileHandle = FindFirstFile("./*.rxbin", &ffd);

    if (INVALID_HANDLE_VALUE == fileHandle)
        printf("Invalid File Handle Value \n");

    do
    {
        printf("%s\n", ffd.cFileName);
    } while (FindNextFile(fileHandle, &ffd) != 0);
#endif

    return(0);
}