/* INCLUDES */
#include "tbt.h"
#include "elf_manager.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <err.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libelf/elf.h>

/* ENUMS */

/* GLOBAL VARIABLES */
static bool verbose = false;
static FILE *OUTPUT = NULL;

/* FUNCTIONS */

/**
 * Display the help and exit.
 */
static void help(void)
{
    printf("Usage: tbt [-a ALGO|-o FILE|-v|-V|-h] FILE|DIR\n"
           "Compute Fuzzy Hashing\n\n"
           " -a ALGO,--algorithm ALGO\tALGO : CTPH|LSH\n"
           " -o FILE,--output FILE\t\twrite result to FILE\n"
           " -v,--verbose\t\t\tverbose output\n"
           " -V,--version\t\t\tdisplay version and exit\n"
           " -h,--help\t\t\tdisplay this help\n");

    exit(EXIT_SUCCESS);
}

/**
 * Display the program's version and exit.
 */
static void version(void)
{
    printf("Triaging Binaries Toolkit %d.%d.%d\n", VERSION, SUBVERSION,
           REVISION);
    exit(EXIT_SUCCESS);
}

/* MAIN */
int main(int argc, char *argv[])
{
    /* clang-format off */
    const struct option long_opts[] = {
        {"output"   , required_argument, NULL, 'o'},
        {"verbose"  , no_argument      , NULL, 'v'},
        {"version"  , no_argument      , NULL, 'V'},
        {"help"     , no_argument      , NULL, 'h'},
        {"algorithm", required_argument, NULL, 'a'},
        { NULL      , 0                , NULL,  0 }
    };
    /* clang-format on */

    OUTPUT = stdout;
    int error_exit = EXIT_SUCCESS;

    int optc;
    const char *options = "o:vVha:";
    while ((optc = getopt_long(argc, argv, options, long_opts, NULL)) != -1) {

        switch (optc) {
        case 'h':
            help();
            break;

        case 'o':
            OUTPUT = fopen(optarg, "w");
            if (!OUTPUT)
                errx(EXIT_FAILURE,
                     "error: can't create and/or open the file '%s'!", optarg);
            break;

        case 'v':
            verbose = true;
            break;

        case 'V':
            version();
            break;

        case 'a':
            printf("Option -a not implemented yet!\n");
            break;

        default:
            errx(EXIT_FAILURE, "error: invalid option '%s'!", argv[optind - 1]);
        }
    }

    if (argc - optind != 1)
        errx(EXIT_FAILURE, "error: invalid number of files or directory");

    /* Check file type */
    struct stat info;

    if (stat(argv[optind], &info) != 0)
        errx(EXIT_FAILURE, "error: cannot access %s", argv[optind]);
    else if (S_ISDIR(info.st_mode))
        printf("%s is a directory\n", argv[optind]);
    else if (S_ISREG(info.st_mode))
        printf("%s is a regular file\n", argv[optind]);
    else
        errx(EXIT_FAILURE, "error: invalid file");

    // FILE *f = fopen("/bin/wc", "rb");
    // elf_data data = elf_get_data(f);
    // fclose(f);
    // elf_print_data(data);
    // elf_free(data);

    return error_exit;
}