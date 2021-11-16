#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#ifdef _WIN32
 #include <windows.h>
#endif

#include "version.h"
#include "types.h"
#include "license.h"

static void help() {
    char *helpMessage =
            "\n"
            "cREXX " CREXX_VERSION " Archiver\n"
            "Usage   : rxar [options] library-name rxbin-file...\n"
            "Options :\n"

            "      --license   Display Copyright & License information.\n"
            "  -h  --help      Display this usage information.\n"
            "  -a  --add       Add binary to library.\n"
            "  -l  --list      List content of library.\n"
            "  -d  --delete    Delete binary from library.\n"
            "  -v  --verbose   Enable verbose output.\n";

    printf("%s", helpMessage);
}

static void error_and_exit(int rc, char *message) {
    fprintf(stderr, "ERROR: %s - try \"rxdas -h\"\n", message);
    exit(rc);
}



int main(int argc, char *argv[]) {
    int c;

    BOOL verbose_flag = FALSE;
    BOOL license_flag = FALSE;
    BOOL valid_option = FALSE;

    struct option long_options[] =
            {
                    {"license", no_argument,            &license_flag, TRUE},
                    {"verbose", no_argument,       0,         'v'},
                    {"help",    no_argument,       0,         'h'},
                    {"add",     required_argument, 0,         'a'},
                    {"list",    required_argument, 0,         'l'},
                    {"delete",  required_argument, 0,         'd'},
                    {0, 0, 0, 0}
            };

    while (1) {
        int option_index = 0;

        c = getopt_long(argc, argv, "vha:l:d:",
                        long_options, &option_index);

        /* End of the options? */
        if (c == -1)
            break;

        switch (c) {
            case 0:

                printf("option -a for `%s' \n", optarg);
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;

                printf ("WHAT?! \n");

                break;

            case 'a':
                printf("option -a for `%s' \n", optarg);
                valid_option = 1;

                break;

            case 'l':
                printf("option -l for `%s' \n", optarg);
                valid_option = 1;

                break;

            case 'd':
                printf("option -d for `%s' \n", optarg);
                valid_option = 1;

                break;

            case 'v':
                verbose_flag = 1;
                break;

            case 'h':
            case '?':
                help();
                exit(0);

            default:
                abort();
        }
    }

    if (verbose_flag) {
        printf("FOO> verbose output activated\n");
    }

    if (license_flag) {
        license();
        exit (0);
    }

    if (!valid_option) {
        printf("No valid option specified. \n");
        help();
    }



    if (optind < argc) {
        printf("further elements: ");
        while (optind < argc) {
            printf("%s ", argv[optind++]);
        }
        putchar('\n');
    }

    exit(0);
}