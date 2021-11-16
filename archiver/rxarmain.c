#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <strings.h>

#ifdef _WIN32
 #include <windows.h>
#endif

#include "types.h"
#include "license.h"
#include "rxarmain.h"
#include "rxaradd.h"
#include "rxarlist.h"
#include "rxardel.h"

#include "rxlntest.h"

// internal functions
static ARCHIVE_ACTION parseOptions(int argc, char *argv[], const struct option *options,
                                   char **libraryPath,  char **libraryName, char **libraryExtension);

static BOOL           fileNameHasExtension(const char *fileName);
static const char    *getFileNameExtension(const char *fileName);

static void           handleLibraryName(const char* inputName,
                                        char **libraryPath, char **libraryName, char **libraryExtension);



// global variables
static BOOL verboseFlag = FALSE;

// main function
int main(int argc, char *argv[]) {

    int rc = 0;

    ARCHIVE_ACTION archiveAction;
    char          *libraryPath;
    char          *libraryName;
    char          *libraryExtension;

    archiveAction = parseOptions(argc, argv, archiverOptions,
                                 &libraryPath, &libraryName, &libraryExtension);

    switch (archiveAction) {
        case ADD:
            if (verboseFlag) {
                fprintf(stdout, "Adding binaries to %s \n", libraryName);
            }

            rc = addBinaries(libraryName);
            break;

        case DELETE:
            if (verboseFlag) {
                fprintf(stdout, "Deleting binaries from %s \n", libraryName);
            }

            rc = deleteBinaries(libraryName);
            break;

        case LIST:
            if (verboseFlag) {
                fprintf(stdout, "List binaries in %s \n", libraryName);
            }

            rc = listBinaries(libraryName);
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

        addBinaries(binaries);

    }


    exit(0);
}

static ARCHIVE_ACTION
parseOptions(int argc, char *argv[], const struct option *options,
             char **libraryPath,  char **libraryName, char **libraryExtension) {

    ARCHIVE_ACTION action = UNKNOWN;

    while (TRUE) {
        int optionIndex = 0;

        int option = getopt_long(argc, argv, "vha:l:d:",
                                 options, &optionIndex);

        if (option == -1)
            break;

        switch (option) {
            case 'a':
                handleLibraryName(optarg, libraryPath, libraryName, libraryExtension);

                action = ADD;
                break;

            case 'd':
                handleLibraryName(optarg, libraryPath, libraryName, libraryExtension);

                action = DELETE;
                break;

            case 'l':
                handleLibraryName(optarg, libraryPath, libraryName, libraryExtension);

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
                action = LICENSE;
                break;

            case 'h':
            case '?':
                action = HELP;
                break;

            default:
                abort();
        }
    }

    return action;
}

static BOOL fileNameHasExtension(const char *fileName) {
    const char *dot = strrchr(fileName, '.');
    if(!dot || dot == fileName) return FALSE;
    return TRUE;
}

static const char *getFileNameExtension(const char *fileName) {
    const char *dot = strrchr(fileName, '.');
    if(!dot || dot == fileName) return "";
    return dot + 1;
}

static void handleLibraryName(const char *inputName, char **libraryPath, char **libraryName, char **libraryExtension) {

    size_t pathLength, nameLength, extLength;

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
        *libraryPath = malloc(pathLength + 1 /* NL */);
        snprintf(*libraryPath, pathLength + 1, "%s", inputName);

        if (verboseFlag) {
            fprintf(stdout, "libraryPath: '%s' \n", *libraryPath);
        }
    } else {
        baseName = inputName;
    }

    nameLength = strlen(baseName);
    if (fileNameHasExtension(baseName)) {
        nameLength = nameLength - ( strlen(getFileNameExtension(baseName)) + 1 /* the dot */) ;
    }

    // copy library name
    *libraryName = malloc(nameLength + 1 /* NL */);
    snprintf(*libraryName, nameLength + 1, "%s", baseName);

    if (verboseFlag) {
        fprintf(stdout, "libraryName: '%s' \n", *libraryName);
    }

    if (fileNameHasExtension(baseName)) {
        extLength = strlen(getFileNameExtension(baseName)) + 1 /* the dot */;
    } else {
        extLength = ARCHIVE_EXTENSION_LENGTH;
    }

    // copy library extension
    *libraryExtension = malloc(extLength + 1 /* NL */);
    if (fileNameHasExtension(baseName)) {
        snprintf(*libraryExtension, extLength + 1, "%s", getFileNameExtension(baseName));
    } else {
        snprintf(*libraryExtension, extLength + 1, "%s", ARCHIVE_EXTENSION);
    }

    if (verboseFlag) {
        fprintf(stdout, "libraryExtension: '%s' \n", *libraryExtension);
    }

}
