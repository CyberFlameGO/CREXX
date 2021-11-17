#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "platform.h"
#include "rxaradd.h"

extern BOOL verboseFlag;
int addBinaries(VFILE *library, VFILE *binaries) {

    if (verboseFlag) {
        fprintf(stdout, "Library '%s' will be created with following binaries:\n", library->fullname);

        while (TRUE) {

            if (binaries) {
                if (!binaries->wildcarded) {
                    fprintf(stdout, "\t\t- '%s' \n", binaries->fullname);
                }

                binaries = binaries->next;
            } else {
                break;
            }
        }
    }

    return(0);
}