#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <strings.h>
#include <unistd.h>

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
static void           createVFile(const char* inputName, VFILE *library);



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

        addBinaries(binaries, NULL);

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
                createVFile(optarg, library);

                if (action == UNKNOWN)
                    action = ADD;

                break;

            case 'd':
                createVFile(optarg, library);

                if (action == UNKNOWN)
                    action = DELETE;

                break;

            case 'l':
                createVFile(optarg, library);

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

static void createVFile(const char *inputName, VFILE *library) {

    size_t pathLength, nameLength, extLength, fullLength;

    const char *pathEndPtr, *baseName;

    if (inputName == NULL) {
        error_and_exit(-42, "Internal error while building library name.");
    }

    // find last file separator
    pathEndPtr = strrchr(inputName, FILE_SEPARATOR);

    // path present
    if (pathEndPtr > 0) {

        // save pointer to basename
        baseName = pathEndPtr + 1;

        pathLength = baseName - inputName;

        // copy library path
        library->path = calloc(1, pathLength + 1 /* EOS */);
        snprintf(library->path, pathLength, "%s", inputName);

    } else {
        library->path = calloc(1, 3);
        snprintf(library->path, 2, "%c%c", '.', FILE_SEPARATOR);

        baseName = inputName;
    }

    nameLength = strlen(baseName);
    if ((baseName)) {
        if (fnext(baseName)) {
            nameLength = nameLength - (strlen(fnext(baseName)) + 1 /* the dot */) ;
        }
    }

    // copy library name
    library->basename = calloc(1, nameLength + 1 /* EOS */);
    snprintf(library->basename, nameLength, "%s", baseName);

    if (fnext(baseName)) {
        extLength = strlen(fnext(baseName)) + 1 /* the dot */;
    } else {
        extLength = ARCHIVE_EXTENSION_LENGTH;
    }

    // copy library extension
    library->extension = calloc(1, extLength + 1 /* EOS */);
    if (fnext(baseName)) {
        snprintf(library->extension, extLength, "%s", fnext(baseName));
    } else {
        snprintf(library->extension, extLength, "%s", ARCHIVE_EXTENSION);
    }

    // creating full name
    fullLength = pathLength + nameLength + extLength;
    library->fullname = calloc( 1, fullLength + 1 /* EOS */);
    snprintf(library->fullname, fullLength, "%s%s.%s", library->path, library->basename, library->extension);

    if (access(library->fullname, F_OK) == 0) {
        library->exists = TRUE;
    } else {
        library->exists = FALSE;
    }

    if (verboseFlag) {
        fprintf(stdout, "VFILE created for '%s' \n", library->fullname);
    }

}
