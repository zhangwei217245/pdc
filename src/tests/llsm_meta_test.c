#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <pcre.h>

typedef struct {
    char   FilePath[512];
    char   Filename[256];
    double StageX_um_;
    double StageY_um_;
    double StageZ_um_;
    double ObjectiveX_um_;
    double ObjectiveY_um_;
    double ObjectiveZ_um_;
    // Breakdown of Filename:
    char prefix[128];
    int  Iter;
    char subIter[128];
    char camera[2]; // Assuming camera is either a single char or 'N'
    int  ch;
    int  stack;
    int  laser;
    int  abstime;
    int  fpgatime;
    int  x, y, z, t;
} TiffMetadata;

void
process_filename(TiffMetadata *record)
{
    const char *error;
    int         erroroffset;
    int         ovector[30];
    int         rc;

    char *pattern1 = "(\\w*)Scan_Iter_(\\d+)(_?(\\d+_)*\\d+?)_Cam(\\w+)_ch(\\d+)_CAM\\d_stack(\\d+)_(\\d+)nm_"
                     "(\\d+)msec_(\\d+)msecAbs_(-?\\d+)x_(-?\\d+)y_(-?\\d+)z_(\\d+)t(\\w*)";
    char *pattern2 = "(\\w*)Scan_Iter_(\\d+)(_?(\\d+_)*\\d+?)_ch(\\d+)_CAM\\d_stack(\\d+)_(\\d+)nm_(\\d+)"
                     "msec_(\\d+)msecAbs_(-?\\d+)x_(-?\\d+)y_(-?\\d+)z_(\\d+)t(\\w*)";

    pcre *re;

    // Check against the first pattern
    re = pcre_compile(pattern1, 0, &error, &erroroffset, NULL);
    if (!re) {
        printf("PCRE compilation failed at offset %d: %s\n", erroroffset, error);
        return;
    }

    rc = pcre_exec(re, NULL, record->Filename, strlen(record->Filename), 0, 0, ovector,
                   sizeof(ovector) / sizeof(ovector[0]));

    if (rc >= 0) { // Match found with the first pattern
        pcre_copy_substring(record->Filename, ovector, rc, 1, record->prefix, sizeof(record->prefix));
        pcre_copy_substring(record->Filename, ovector, rc, 2, record->Iter, sizeof(record->Iter));
        pcre_copy_substring(record->Filename, ovector, rc, 3, record->subIter, sizeof(record->subIter));
        pcre_copy_substring(record->Filename, ovector, rc, 5, record->camera, sizeof(record->camera));
        pcre_copy_substring(record->Filename, ovector, rc, 6, record->ch, sizeof(record->ch));
        // Continue for other substrings...
    }
    else {
        pcre_free(re);

        // If not matched with the first pattern, try the second
        re = pcre_compile(pattern2, 0, &error, &erroroffset, NULL);
        if (!re) {
            printf("PCRE compilation failed at offset %d: %s\n", erroroffset, error);
            return;
        }

        rc = pcre_exec(re, NULL, record->Filename, strlen(record->Filename), 0, 0, ovector,
                       sizeof(ovector) / sizeof(ovector[0]));

        if (rc >= 0) { // Match found with the second pattern
            pcre_copy_substring(record->Filename, ovector, rc, 1, record->prefix, sizeof(record->prefix));
            pcre_copy_substring(record->Filename, ovector, rc, 2, record->Iter, sizeof(record->Iter));
            pcre_copy_substring(record->Filename, ovector, rc, 3, record->subIter, sizeof(record->subIter));
            strcpy(record->camera, "N");
            pcre_copy_substring(record->Filename, ovector, rc, 4, record->ch, sizeof(record->ch));
            // Continue for other substrings...
        }
    }
    pcre_free(re);
}

void
read_csv(const char *file_path)
{
    FILE *file = fopen(file_path, "r");
    if (!file) {
        printf("Failed to open file: %s\n", file_path);
        return;
    }

    char line[1024];

    // Assuming the first line is always header
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file)) {
        records = realloc(records, (record_count + 1) * sizeof(TiffMetadata));
        if (!records) {
            printf("Memory allocation failed.\n");
            fclose(file);
            return;
        }

        sscanf(line, "%[^,],%[^,],%lf,%lf,%lf,%lf,%lf,%lf", records[record_count].FilePath,
               records[record_count].Filename, &records[record_count].StageX_um_,
               &records[record_count].StageY_um_, &records[record_count].StageZ_um_,
               &records[record_count].ObjectiveX_um_, &records[record_count].ObjectiveY_um_,
               &records[record_count].ObjectiveZ_um_);

        process_filename(&records[record_count]);
        record_count++;
    }

    fclose(file);
}

void
read_csv_files_from_directory(const char *dir_path)
{
    struct dirent *entry;
    DIR *          dir = opendir(dir_path);
    if (dir == NULL) {
        printf("Error opening directory\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".csv")) {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
            read_csv(full_path);
        }
    }

    closedir(dir);
}

int
main()
{
    int           record_count = 0;
    TiffMetadata *records      = NULL;

    read_csv_files_from_directory("/path/to/csv/directory", &record_count, &records);

    // Don't forget to free dynamically allocated memory
    free(records);
    return 0;
}