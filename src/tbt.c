/* INCLUDES */
#include "tbt.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <err.h>
#include <getopt.h>

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
    printf("Usage: tbt [-a ALGO|-o FILE|-v|-V|-h] FILE|DIR...\n"
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

    Fhdr fhdr;
    FILE *f;
    uint8_t *buf;
    uint64_t len;

    /*if(argc != 3)
            return -1;*/

    f = fopen("/bin/ls", "rb");
    /*f = fopen(argv[1], "rb");
    if (f == NULL)
            errx(EXIT_FAILURE, "error: can't open %s", argv[1]);*/

    buf = readelfsection(f, ".rodata", &len, &fhdr);
    if (buf == NULL)
        return -1;

    /* Code Test */
    printelfhdr(&fhdr);
    printf("\n");
    // print_buf_elf(buf,len);

    freeelf(&fhdr);
    fclose(f);

    return error_exit;
}
