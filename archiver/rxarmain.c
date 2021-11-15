#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#ifdef _WIN32
#include <windows.h>
#endif

static void help() {
    char *helpMessage =
            "cREXX Archiver vX.X.X\n"
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

static void license() {
    char *message =
            "cREXX License (MIT)\n"
            "Copyright (c) 2020-2021 Adrian Sutherland\n\n"

            "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
            "of this software and associated documentation files (the \"Software\"), to deal\n"
            "in the Software without restriction, including without limitation the rights\n"
            "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
            "copies of the Software, and to permit persons to whom the Software is\n"
            "furnished to do so, subject to the following conditions:\n\n"

            "The above copyright notice and this permission notice shall be included in all\n"
            "copies or substantial portions of the Software.\n\n"

            "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
            "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
            "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
            "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
            "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
            "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
            "SOFTWARE.\n\n"
            "See https://github.com/adesutherland/CREXX for project details\n";

    printf("%s", message);
}

static int verbose_flag;
static int license_flag;

static struct option long_options[] =
        {
                /* These options are setting the flags, directly. */
                {"license", no_argument,       &license_flag, 1},
                /* These options are used with their corresponding short names  */
                {"verbose", no_argument,       0,        'v'},
                {"help",    no_argument,       0,        'h'},
                {"add",     required_argument, 0,        'a'},
                {"list",    required_argument, 0,        'l'},
                {"delete",  required_argument, 0,        'd'},
                {0,         0,                 0,        0}
        };

int main(int argc, char *argv[]) {
    int c;

    int valid_option = 0;

    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "ha:l:d:",
                        long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;

                /*
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                */
                printf ("FOO> WHAT?! \n");

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