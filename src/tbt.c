/* INCLUDES */
#include "tbt.h"
#include "elf_manager.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
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

/**
 * Treatment ELF File.
 * Return false if problems, true otherwise.
 */
static bool treat_file(char *file_path)
{
    if (file_path == NULL)
        return false;

    /* Open ELF File */
    FILE *f = fopen(file_path, "rb");
    if (f == NULL)
        return false;

    /* Get Data */
    elf_data data = elf_get_data(f);
    fclose(f);
    if (data == NULL)
        return false;

    /* Compute Fuzzy Hashing */
    fprintf(stderr,
            "[+] File '%s'\n"
            "[+] Fuzzy Hashing\n",
            file_path);
    /* CTPH */
    fprintf(stderr, "[+] \tCTPH ...\n");
    /* LSH */
    fprintf(stderr, "[+] \tLSH  ...\n");

    /* Free Data */
    elf_free(data);
    return true;
}

/**
 * Treatment Directory
 */
static bool treat_dir(char *dir_path)
{
    if (dir_path == NULL)
        return false;

    /* Open Dir */
    DIR *dir = opendir(dir_path);
    if (dir == NULL)
        return false;

    /* Get File(s) */
    struct dirent *file;
    char path[strlen(dir_path) + 1024];
    while ((file = readdir(dir)) != NULL) {
        sprintf(path, "%s%s", dir_path, file->d_name);
        /* Treat File */
        if (!treat_file(path))
            if (verbose)
                warnx("'%s' is an invalid file", file->d_name);
    }

    /* Close Dir */
    closedir(dir);
    return true;
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
        errx(EXIT_FAILURE, "error: cannot access '%s'", argv[optind]);
    else if (S_ISDIR(info.st_mode)) {
        fprintf(stderr, "'%s' is a directory\n", argv[optind]);

        char dir_path[2048];
        uint16_t i = strlen(argv[optind]);
        if (argv[optind][i - 1] == '/')
            sprintf(dir_path, "%s", argv[optind]);
        else
            sprintf(dir_path, "%s/", argv[optind]);

        if (!treat_dir(dir_path))
            error_exit = EXIT_FAILURE;
    } else if (S_ISREG(info.st_mode)) {
        fprintf(stderr, "'%s' is a regular file\n", argv[optind]);
        if (!treat_file(argv[optind]))
            errx(EXIT_FAILURE, "error: '%s' is an invalid file", argv[optind]);
    } else
        errx(EXIT_FAILURE, "error: invalid file");

    return error_exit;
}