/* INCLUDES */
#include "tbt.h"
#include "ctph.h"
#include "elf_manager.h"
#include "simhash.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>
/* DEFINES */
#define LINE_BUF_SIZE 400

/* ENUMS */
typedef enum { ALL, CTPH, SIMHASH } algorithm;

/* GLOBAL VARIABLES */
static bool verbose = false, comparision_wanted = false;
static FILE *OUTPUT = NULL;
static algorithm chosen_algorithm = ALL;

/* Structures */
typedef struct {
    char name[LINE_BUF_SIZE];
    char SIMHASH_hash[33];
    char CTPH_hash[150];
} file_info_t;

typedef struct {
    char *name;
    float percentage;
} res_comp_t;

/* FUNCTIONS */

/**
 * Closing OUTPUT if is a file
 */
static void close_output()
{
    if (OUTPUT != stdout)
        fclose(OUTPUT);
}

/**
 * Display the help and exit.
 */
static void help(void)
{
    printf("Usage: tbt [-a ALGO|-o FILE|-c|-v|-V|-h] FILE|DIR\n"
           "Compute Fuzzy Hashing\n\n"
           " -a ALGO,--algorithm ALGO\tALGO : CTPH|SIMHASH|ALL\n"
           " -c ,--compareHashes\t\tCompare the hashes stored in the given "
           "file\n"
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
 * compara function used by the quick_sort() function
 */
static int compare_result(const void *res_1, const void *res_2)
{
    return ((res_comp_t *) res_2)->percentage -
           ((res_comp_t *) res_1)->percentage;
}
/*
 * Outputs the likeness percentage of all files
 */
static void comparision(int nb_files, file_info_t file_content[])
{
    res_comp_t results[nb_files];

    if (chosen_algorithm == ALL || chosen_algorithm == CTPH) {
        fprintf(OUTPUT, "--- CTPH ---\n");

        for (int i = 0; i < nb_files; i++) {
            fprintf(OUTPUT, "\n%s :\n", file_content[i].name);

            /* Sort result */
            for (int j = 0; j < nb_files; j++) {
                results[j].name = file_content[j].name;
                results[j].percentage = (float) ctph_compare(
                    file_content[i].CTPH_hash, file_content[j].CTPH_hash);
            }

            qsort(results, nb_files, sizeof(res_comp_t), compare_result);

            /* Print */
            for (int j = 0; j < nb_files; j++) {
                if (strcmp(file_content[i].name, results[j].name) == 0)
                    continue;
                if (results[j].percentage == 0.0)
                    continue;

                fprintf(OUTPUT, "[ %03.f %% ] %s\n", results[j].percentage,
                        results[j].name);
            }
        }

        fprintf(OUTPUT, "\n");
    }
    if (chosen_algorithm == ALL || chosen_algorithm == SIMHASH) {
        fprintf(OUTPUT, "--- SIMHASH ---\n");

        for (int i = 0; i < nb_files; i++) {
            fprintf(OUTPUT, "\n%s :\n", file_content[i].name);

            /* Sort result */
            for (int j = 0; j < nb_files; j++) {

                results[j].name = file_content[j].name;
                results[j].percentage = simhash_compare(
                    file_content[i].SIMHASH_hash, file_content[j].SIMHASH_hash);
            }

            qsort(results, nb_files, sizeof(res_comp_t), compare_result);

            /* Print */
            for (int j = 0; j < nb_files; j++) {
                if (strcmp(file_content[i].name, results[j].name) == 0)
                    continue;
                if (results[j].percentage == 0.0)
                    continue;

                fprintf(OUTPUT, "[ %06.02f %% ] %s\n", results[j].percentage,
                        results[j].name);
            }
        }
    }
}

/**
 * Returns the number of files concerned by the comparision mode
 * 0 if none
 */
static uint64_t get_nb_files(FILE *in)
{
    uint64_t nb_files = 0;
    char line_buf[LINE_BUF_SIZE];

    /* Returning to the beginning of the file */
    if (fseek(in, 0, SEEK_SET) != 0)
        errx(EXIT_FAILURE, "Couldn't return to the beginning of the file");

    /* Get nb_files */
    fgets(line_buf, LINE_BUF_SIZE, in);
    while (!feof(in)) {
        if (ferror(in)) {
            fprintf(stderr, "Reading error with code %d\n", errno);
            break;
        }
        if (line_buf[0] == '\t')
            fgets(line_buf, LINE_BUF_SIZE, in);
        else {
            nb_files++;
            fgets(line_buf, LINE_BUF_SIZE, in);
        }
    }
    return nb_files;
}

/* DEBUG STRUCT PRINT */
/**
 * static void print_struct(file_info_t file_content[], uint64_t size)
 * {
 *     for (uint64_t i = 0; i < size; i++) {
 *         printf("%s\n", file_content[i].name);
 *         printf("%s\n", file_content[i].CTPH_hash);
 *         // for (int i = 0; i < 32; i++)
 *         printf("%s\n", file_content[i].SIMHASH_hash);
 *         if (i != size - 1)
 *             printf("\n");
 *     }
 * }
 */

/**
 * Parses the file and stores the hashes of each file in the structure
 * file_content
 */
static void get_file_content(FILE *in, file_info_t file_content[],
                             uint64_t nb_files)
{

    char line_buf[LINE_BUF_SIZE];
    uint64_t file_count = 0;
    int ctph_present = false, simhash_present = false;

    /* Returning to the beginning of the file */
    if (fseek(in, 0, SEEK_SET) != 0)
        errx(EXIT_FAILURE, "Couldn't return to the beginning of the file");

    while (!feof(in)) {
        if (ferror(in)) {
            fprintf(stderr, "Reading error with code %d\n", errno);
            break;
        }
        if (file_count > nb_files)
            errx(EXIT_FAILURE, "Wrong number of entry in the file");

        int i = 0, index = 0;
        while (line_buf[i] != '\0') {
            /** Checks if the content of the line is 1 (CTPH)
             * if yes stores it
             */
            if (line_buf[i] == '\t') {
                while (line_buf[i] != '1' && line_buf[i] != '2')
                    i++;
            } else if (line_buf[i] == '1' && i != 0 && file_count > 0) {
                for (i = i + 2, index = 0;
                     line_buf[i] != '\0' && line_buf[i] != '\n'; i++, index++) {
                    file_content[file_count - 1].CTPH_hash[index] = line_buf[i];
                }
                ctph_present = true;
                file_content[file_count - 1].CTPH_hash[index] = '\0';
            }
            /** Checks if the content of the line is 2 (SIMHASH)
             * if yes stores it
             */
            else if (line_buf[i] == '2' && i != 0 && file_count > 0) {
                for (i = i + 2, index = 0;
                     line_buf[i] != '\0' && line_buf[i] != '\n'; i++, index++) {

                    file_content[file_count - 1].SIMHASH_hash[index] =
                        line_buf[i];
                }
                simhash_present = true;
                file_content[file_count - 1].SIMHASH_hash[index] = '\0';
            }
            /** Checks if the content of the line is the name of the file,
             * if yes, Stores it
             */
            else if (i == 0) {
                for (index = 0; line_buf[i] != '\0' && line_buf[i] != '\n';
                     i++, index++) {
                    file_content[file_count].name[index] = line_buf[i];
                }
                file_content[file_count].name[index - 1] = '\0';
                file_count++;

                i++;
            } else {
                i++;
            }
        }
        fgets(line_buf, LINE_BUF_SIZE, in);
    }
    /* check which algorithm was required and if present in the file */
    if (!ctph_present) {
        if (chosen_algorithm == CTPH)
            errx(EXIT_FAILURE,
                 "error: There is not hash for CTPH in this file");
    }
    if (!simhash_present) {
        if (chosen_algorithm == SIMHASH)
            errx(EXIT_FAILURE,
                 "error: There is not hash for SIMHASH in this file");
    }
    if (chosen_algorithm == ALL) {
        if (!ctph_present && simhash_present)
            chosen_algorithm = SIMHASH;
        if (!simhash_present && ctph_present)
            chosen_algorithm = CTPH;
    }
}

/**
 * General call to functions to get the number of files, get the hashes for each
 * file and do the comparision
 */
static void file_parser(char *file_name)
{
    FILE *in = fopen(file_name, "r");
    if (in == NULL)
        errx(EXIT_FAILURE, "problem opening file");

    uint64_t nb_files = get_nb_files(in);
    file_info_t file_content[nb_files];
    get_file_content(in, file_content, nb_files);
    comparision(nb_files, file_content);
    fclose(in);
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
    fprintf(stderr, "[+] Fuzzy hashing of '%s'\n", file_path);

    /* CTPH */
    char *CTPhash = ctph_hash(data);

    /* LSH */
    char *simHash = simhash_compute(data);

    char *temp_file_name = strrchr(file_path, '/');
    temp_file_name = (temp_file_name == NULL) ? file_path : temp_file_name + 1;

    /* Write the hash(es) in the output */
    if (chosen_algorithm == ALL)
        fprintf(OUTPUT,
                "%s:\n\t1:%s\n\t2:"
                "%s\n",
                temp_file_name, CTPhash, simHash);
    else
        fprintf(OUTPUT, "%s:\n\t%d:%s\n", temp_file_name,
                (chosen_algorithm == CTPH) ? 1 : 2,
                (chosen_algorithm == CTPH) ? CTPhash : simHash);
    /* Free Data */
    free(CTPhash);
    free(simHash);
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
        {"output"       , required_argument, NULL, 'o'},
        {"compareHashes", no_argument      , NULL, 'c'},
        {"verbose"      , no_argument      , NULL, 'v'},
        {"version"      , no_argument      , NULL, 'V'},
        {"help"         , no_argument      , NULL, 'h'},
        {"algorithm"    , required_argument, NULL, 'a'},
        { NULL          , 0                , NULL,  0 }
    };
    /* clang-format on */

    OUTPUT = stdout;
    int return_code = EXIT_SUCCESS;

    int optc;
    char *outputoption = NULL;
    const char *options = "o:vVha:c";
    while ((optc = getopt_long(argc, argv, options, long_opts, NULL)) != -1) {

        switch (optc) {
        case 'h':
            help();
            break;

        case 'o':
            outputoption = optarg;
            break;

        case 'v':
            verbose = true;
            break;

        case 'V':
            version();
            break;

        case 'a':
            if (strcmp(optarg, "ALL") == 0 || strcmp(optarg, "all") == 0)
                chosen_algorithm = ALL;
            else if (strcmp(optarg, "CTPH") == 0 || strcmp(optarg, "ctph") == 0)
                chosen_algorithm = CTPH;
            else if (strcmp(optarg, "SIMHASH") == 0 ||
                     strcmp(optarg, "simhash") == 0)
                chosen_algorithm = SIMHASH;
            else
                errx(EXIT_FAILURE, "-a option's [%s] argument is not valid!",
                     optarg);
            break;

        case 'c':
            comparision_wanted = true;
            break;
        default:
            errx(EXIT_FAILURE, "error: invalid option '%s'!", argv[optind - 1]);
        }
    }

    if (argc - optind != 1)
        errx(EXIT_FAILURE, "error: invalid number of files or directory");

    /* Verifying if the output file already exists. If so, it's an error */
    if (outputoption != NULL) {
        if (access(outputoption, F_OK) == 0)
            errx(EXIT_FAILURE, "error: File %s already exists !", outputoption);
        if ((OUTPUT = fopen(outputoption, "w")) == NULL)
            errx(EXIT_FAILURE, "error: can't create and/or open the file '%s'!",
                 outputoption);
    }
    /* COMPARISION MODE */
    if (comparision_wanted == true) {
        file_parser(argv[optind]);
        close_output();
        return return_code;
    }

    /* HASH CREATION MODE */

    /* Check file type */
    struct stat info;

    if (stat(argv[optind], &info) != 0)
        errx(EXIT_FAILURE, "error: cannot access '%s'", argv[optind]);

    if (S_ISDIR(info.st_mode)) {
        fprintf(stderr, "[+] '%s' is a directory\n", argv[optind]);

        char dir_path[2048];
        uint16_t i = strlen(argv[optind]);
        if (argv[optind][i - 1] == '/')
            sprintf(dir_path, "%s", argv[optind]);
        else
            sprintf(dir_path, "%s/", argv[optind]);

        if (!treat_dir(dir_path))
            return_code = EXIT_FAILURE;
    } else if (S_ISREG(info.st_mode)) {
        fprintf(stderr, "[+] '%s' is a regular file\n", argv[optind]);
        if (!treat_file(argv[optind]))
            errx(EXIT_FAILURE, "error: '%s' is an invalid file", argv[optind]);
    } else
        errx(EXIT_FAILURE, "error: invalid file");

    close_output();
    return return_code;
}
