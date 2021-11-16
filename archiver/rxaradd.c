#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <fnmatch.h>
#include <string.h>

#include "rxaradd.h"



int addBinaries(char *libraryName) {

    int r;

    char pattern[] = "*.rxbin";

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


    //(whoami = strrchr("", '/')) ? ++whoami : (whoami = argv[0]);

    return(0);

}