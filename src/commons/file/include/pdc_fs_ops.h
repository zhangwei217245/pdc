
/* File foo.  */
#ifndef PDC_FS_OPS
#define PDC_FS_OPS

#include "pdc_base_stdlib.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <errno.h>

typedef enum {
    ASC, DESC
} sorting_direction_t;

void pdc_walk_fs_dir(const char *dir_path, int (*selector) (const struct dirent *),
    int (*cmp) (const struct dirent **, const struct dirent **), 
    sorting_direction_t sd, int topk, 
    int (*on_file)(struct dirent *f_entry, const char *parent_path, void *args), 
    int (*on_dir)(struct dirent *d_entry, const char *parent_path, void *args), 
    void *coll_args,
    int (*pre_op)(void *coll_args),
    int (*post_op)(void *coll_args));


int pdc_is_regular_file(const char *path);

size_t pdc_get_file_size(const char *filename);

int pdc_dir_exists(char *dirname);


/* Function with behaviour like `mkdir -p'  */
int
pdc_mkpath(const char *s, mode_t mode);

#endif /* !PDC_FS_OPS */