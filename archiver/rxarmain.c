#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <strings.h>

#include "types.h"
#include "license.h"
#include "rxarmain.h"
#include "rxaradd.h"
#include "rxarlist.h"
#include "rxardel.h"

#include "platform.h"

#include "rxlntest.h"

// internal functions
static ARCHIVE_ACTION parseOptions(int argc, char *argv[], const struct option *options, VFILE *library);

// global variables
static BOOL verboseFlag = FALSE;

// main function
int main(int argc, char *argv[]) {

    int rc = 0;

    ARCHIVE_ACTION  archiveAction;
    VFILE           library;

    archiveAction = parseOptions(argc, argv, archiverOptions, &library);

    switch (archiveAction) {
        case ADD:
            if (verboseFlag) {
                fprintf(stdout, "Adding binaries to %s \n", library.basename);
            }

            rc = addBinaries(&library, NULL);
            break;

        case DELETE:
            if (verboseFlag) {
                fprintf(stdout, "Deleting binaries from %s \n", library.basename);
            }

            rc = deleteBinaries(library.basename);
            break;

        case LIST:
            if (verboseFlag) {
                fprintf(stdout, "List binaries in %s \n", library.basename);
            }

            rc = listBinaries(library.basename);
            break;

        case LICENSE:
            license();
            break;

        case HELP:
            help();
            break;

        default:
            error_and_exit(-1, "Unknown option selected.");
    }

   return rc;

    if (optind < argc) {

        int    index = 0;
        char **binaries = calloc(argc - optind, sizeof(char *) );

        printf("further elements: ");

        while (optind < argc) {
            binaries[index] = argv[optind];

            index++;
            optind++;
        }
    }


    exit(0);
}

static ARCHIVE_ACTION
parseOptions(int argc, char *argv[], const struct option *options, VFILE *library) {

    ARCHIVE_ACTION action = UNKNOWN;

    while (TRUE) {
        int optionIndex = 0;

        int option = getopt_long(argc, argv, "v?ha:l:d:",
                                 options, &optionIndex);

        if (option == -1)
            break;

        switch (option) {
            case 'a':
                vfnew(optarg, library, NULL, RXLIB_EXT);

                if (action == UNKNOWN)
                    action = ADD;

                break;

            case 'd':
                vfnew(optarg, library, NULL, RXLIB_EXT);

                if (action == UNKNOWN)
                    action = DELETE;

                break;

            case 'l':
                vfnew(optarg, library, NULL, RXLIB_EXT);

                if (action == UNKNOWN)
                    action = LIST;

                break;

            case 't':
                if (verboseFlag) {
                    fprintf(stdout, "Calling cREXX Linker test function. \n");
                }

                test(optarg);
                exit (0);

            case 'v':
                verboseFlag = 1;
                break;

            case 'c':

                if (action == UNKNOWN)
                    action = LICENSE;

                break;

            case 'h':
            case '?':

                if (action == UNKNOWN)
                    action = HELP;

                break;

            default:
                abort();
        }
    }

    return action;
}
