#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#ifdef LLSM_ENABLE_MPI
#include "mpi.h"
#endif

#include "pdc.h"
// #include "pdc_client_server_common.h"
// #include "pdc_client_connect.h"

#include "llsm_aux/parallelReadTiff.h"
#include "llsm_aux/pdc_list.h"
#include "llsm_aux/csvReader.h"
#include <libgen.h>

typedef struct llsm_importer_args_t {
    char *        directory_path;
    csv_header_t *csv_header;
} llsm_importer_args_t;

int rank = 0, size = 1;

pdcid_t         pdc_id_g = 0, cont_prop_g = 0, cont_id_g = 0, obj_prop_g = 0;
struct timespec ts;

double
getDoubleTimestamp()
{
    clock_gettime(CLOCK_MONOTONIC, &ts);
    double timestamp = (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
    return timestamp;
}

int
parse_console_args(int argc, char *argv[], char **file_name, int *flag_m)
{
    int c, parse_code = -1;

    while ((c = getopt(argc, argv, "f:m")) != -1) {
        switch (c) {
            case 'f':
                *file_name = optarg;
                parse_code = 0;
                break;
            case 'm':
                *flag_m    = 1;
                parse_code = 0;
                break;
            default:
                fprintf(stderr, "Usage: %s [-m] [-f filename]\n", argv[0]);
                parse_code = -1;
                exit(EXIT_FAILURE);
        }
    }
    return parse_code;
}

void
import_to_pdc(image_info_t *image_info, csv_cell_t *fileName_cell)
{
    double duration, start;

    start = getDoubleTimestamp(); // start timing the operation

    obj_prop_g = PDCprop_create(PDC_OBJ_CREATE, pdc_id_g);

    psize_t  ndims      = 3;
    uint64_t offsets[3] = {0, 0, 0};
    // FIXME: we should support uint64_t.
    uint64_t dims[3] = {image_info->x, image_info->y, image_info->z};

    // psize_t ndims = 1;
    // uint64_t offsets[1] = {0};
    // // FIXME: we should support uint64_t.
    // uint64_t dims[1] = {image_info->x * image_info->y * image_info->z};

    // FIXME: we should change the ndims parameter to psize_t type.
    PDCprop_set_obj_dims(obj_prop_g, (PDC_int_t)ndims, dims);
    pdc_var_type_t pdc_type = PDC_UNKNOWN;
    switch (image_info->bits) {
        case 8:
            pdc_type = PDC_UINT8;
            break;
        case 16:
            pdc_type = PDC_UINT16;
            break;
        case 32:
            pdc_type = PDC_FLOAT;
            break;
        case 64:
            pdc_type = PDC_DOUBLE;
            break;
        default:
            printf("Error: unsupported data type.\n");
            exit(-1);
    }
    PDCprop_set_obj_type(obj_prop_g, pdc_type);
    PDCprop_set_obj_time_step(obj_prop_g, 0);
    PDCprop_set_obj_user_id(obj_prop_g, getuid());
    PDCprop_set_obj_app_name(obj_prop_g, "LLSM");

    // uint64_t *offsets   = (uint64_t *)malloc(sizeof(uint64_t) * ndims);
    // uint64_t *num_bytes = (uint64_t *)malloc(sizeof(uint64_t) * ndims);
    // for (int i = 0; i < ndims; i++) {
    //     offsets[i]   = 0;
    //     num_bytes[i] = dims[i] * image_info->bits / 8;
    // }

    // create object
    // FIXME: There are many attributes currently in one file name,
    // and we should do some research to see what would be a good object name for each image.
    pdcid_t cur_obj_g = PDCobj_create(cont_id_g, fileName_cell->field_value, obj_prop_g);

    // write data to object
    pdcid_t local_region  = PDCregion_create(ndims, offsets, dims);
    pdcid_t remote_region = PDCregion_create(ndims, offsets, dims);
    pdcid_t transfer_request =
        PDCregion_transfer_create(image_info->tiff_ptr, PDC_WRITE, cur_obj_g, local_region, remote_region);
    PDCregion_transfer_start(transfer_request);
    PDCregion_transfer_wait(transfer_request);

    duration = getDoubleTimestamp() - start; // end timing the operation and calculate duration in nanoseconds

    printf("[Rank %4d] Region_Transfer %s_[%ld_Bytes] Done! Time taken: %.4f seconds\n", rank,
           fileName_cell->field_value, image_info->tiff_size, duration);

    // add metadata tags based on the csv row
    csv_cell_t *cell = fileName_cell;
    while (cell != NULL) {
        char *field_name  = cell->header->field_name;
        char  data_type   = cell->header->field_type;
        char *field_value = cell->field_value;
        switch (data_type) {
            case 'i':
                int ivalue = atoi(field_value);
                PDCobj_put_tag(cur_obj_g, field_name, &ivalue, PDC_INT, sizeof(int));
                break;
            case 'f':
                double fvalue = atof(field_value);
                PDCobj_put_tag(cur_obj_g, field_name, &fvalue, PDC_DOUBLE, sizeof(double));
                break;
            case 's':
                PDCobj_put_tag(cur_obj_g, field_name, field_value, PDC_STRING, strlen(field_value));
                break;
            default:
                break;
        }
        cell = cell->next;
    }

    // add extra metadata tags based on the image_info struct
    PDCobj_put_tag(cur_obj_g, "x", &(image_info->x), PDC_UINT64, sizeof(uint64_t));
    PDCobj_put_tag(cur_obj_g, "y", &(image_info->y), PDC_UINT64, sizeof(uint64_t));
    PDCobj_put_tag(cur_obj_g, "z", &(image_info->z), PDC_UINT64, sizeof(uint64_t));
    PDCobj_put_tag(cur_obj_g, "bits", &(image_info->bits), PDC_UINT64, sizeof(uint64_t));
    PDCobj_put_tag(cur_obj_g, "startSlice", &(image_info->startSlice), PDC_UINT64, sizeof(uint64_t));
    PDCobj_put_tag(cur_obj_g, "stripeSize", &(image_info->stripeSize), PDC_UINT64, sizeof(uint64_t));
    PDCobj_put_tag(cur_obj_g, "is_imageJ", &(image_info->is_imageJ), PDC_UINT64, sizeof(uint64_t));
    PDCobj_put_tag(cur_obj_g, "imageJ_Z", &(image_info->imageJ_Z), PDC_UINT64, sizeof(uint64_t));

    // close object
    PDCobj_close(cur_obj_g);

    // get timing
    duration = getDoubleTimestamp() - start; // end timing the operation calculate duration in nanoseconds

    printf("[Rank %4d] Create_object %s Done! Time taken: %.4f seconds\n", rank, fileName_cell->field_value,
           duration);

    // free memory
    // free(offsets);
    // free(num_bytes);
    // PDCregion_close(local_region);
    // PDCregion_close(remote_region);
    // PDCregion_transfer_close(transfer_request);
    PDCprop_close(obj_prop_g);
}

void
on_csv_row(csv_row_t *row, llsm_importer_args_t *llsm_args)
{
    csv_print_row(row, 1);

    char *dirname = strdup(llsm_args->directory_path);
    char  filepath[256];
    // calling tiff loading process.
    image_info_t *image_info = NULL;
    int           i          = 0;
    double        duration, start;
    // Filepath,Filename,StageX_um_,StageY_um_,StageZ_um_,ObjectiveX_um_,ObjectiveY_um_,ObjectiveZ_um_

    // get the file name from the csv row
    csv_cell_t *fileName_cell = csv_get_field_value_by_name(row, llsm_args->csv_header, "Filename");

    // check if the path ends with a forward slash
    if (dirname[strlen(dirname) - 1] != '/') {
        strcat(dirname, "/"); // add a forward slash to the end of the path
    }

    strcpy(filepath, dirname);                    // copy the directory path to the file path
    strcat(filepath, fileName_cell->field_value); // concatenate the file name to the file path

    start = getDoubleTimestamp(); // start timing the operation

    parallel_TIFF_load(filepath, 1, NULL, &image_info);

    duration = getDoubleTimestamp() - start; // end timing the operation and calculate duration in nanoseconds

    printf("[Rank %4d] Read %s Done! Time taken: %.4f seconds\n", rank, filepath, duration);

    if (image_info == NULL || image_info->tiff_ptr == NULL) {
        return;
    }

    printf("first few bytes ");
    for (i = 0; i < 10; i++) {
        printf("%d ", ((uint8_t *)image_info->tiff_ptr)[i]);
    }
    printf("\n");

    // import the image to PDC
    import_to_pdc(image_info, fileName_cell);

    // free the image info
    free(image_info->tiff_ptr);
    free(image_info);
    free(dirname);
}

void
read_txt(char *txtFileName, PDC_LIST *list, int *max_row_length)
{
    FILE *file = fopen(txtFileName, "r");

    int row_length = 0;

    if (file == NULL) {
        printf("Error: could not open file %s\n", txtFileName);
        return;
    }
    char buffer[1024];
    // Read the lines of the file
    while (fgets(buffer, sizeof(buffer), file)) {
        pdc_list_add(list, strdup(buffer));
        if (row_length < strlen(buffer)) {
            row_length = strlen(buffer);
        }
    }

    fclose(file);

    // Find the maximum row length
    *max_row_length = row_length + 5;
}

int
main(int argc, char *argv[])
{

#ifdef ENABLE_MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
#endif

    char *                file_name    = NULL;
    PDC_LIST *            list         = pdc_list_new();
    char *                csv_line     = NULL;
    int                   num_row_read = 0;
    csv_header_t *        csv_header   = NULL;
    csv_row_t *           csv_row      = NULL;
    llsm_importer_args_t *llsm_args    = NULL;
    int                   bcast_count  = 512;
    double                duration = 0, start = 0;
    char                  csv_field_types[] = {'s', 's', 'f', 'f', 'f', 'f', 'f', 'f'};
    int flag_m = 0; // flag for metadata. if 1, then only read metadata from CSV without reading images.
    // parse console argument
    int parse_code = parse_console_args(argc, argv, &file_name, &flag_m);
    if (parse_code) {
        return parse_code;
    }
    char *directory_path = dirname(strdup(file_name));

    // print file name for validating purpose
    printf("Filename: %s\n", file_name ? file_name : "(none)");
    printf("Directory: %s\n", directory_path ? directory_path : "(none)");

    // create a pdc
    pdc_id_g = PDCinit("pdc");

    // create a container property
    cont_prop_g = PDCprop_create(PDC_CONT_CREATE, pdc_id_g);
    if (cont_prop_g <= 0)
        printf("Fail to create container property @ line  %d!\n", __LINE__);

    // create a container
    cont_id_g = PDCcont_create("c1", cont_prop_g);
    if (cont_id_g <= 0)
        printf("Fail to create container @ line  %d!\n", __LINE__);

    // Rank 0 reads the filename list and distribute data to other ranks
    if (rank == 0) {
        read_txt(file_name, list, &bcast_count);
        // print bcast_count
        printf("bcast_count: %d \n", bcast_count);

#ifdef ENABLE_MPI
        // broadcast the number of lines
        int num_lines = pdc_list_size(list);
        MPI_Bcast(&num_lines, 1, MPI_INT, 0, MPI_COMM_WORLD);
        // broadcast the bcast_count
        MPI_Bcast(&bcast_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
        // broadcast the file names
        PDC_LIST_ITERATOR *iter = pdc_list_iterator_new(list);
        while (pdc_list_iterator_has_next(iter)) {
            char *csv_line = (char *)pdc_list_iterator_next(iter);
            MPI_Bcast(csv_line, bcast_count, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
#endif
    }
    else {
#ifdef ENABLE_MPI
        // other ranks receive the number of files
        int num_lines;
        MPI_Bcast(&num_lines, 1, MPI_INT, 0, MPI_COMM_WORLD);
        // receive the bcast_count
        MPI_Bcast(&bcast_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
        // receive the file names
        int i;
        for (i = 0; i < num_lines; i++) {
            csv_line = (char *)malloc(bcast_count * sizeof(char));
            MPI_Bcast(csv_line, bcast_count, MPI_CHAR, 0, MPI_COMM_WORLD);
            pdc_list_add(list, csv_line);
        }
#endif
    }
    // parse the csv
    csv_table_t *csv_table = csv_parse_list(list, csv_field_types);
    if (csv_table == NULL) {
        printf("Fail to parse csv file @ line  %d!\n", __LINE__);
        return -1;
    }
    llsm_args                 = (llsm_importer_args_t *)malloc(sizeof(llsm_importer_args_t));
    llsm_args->directory_path = directory_path;
    llsm_args->csv_header     = csv_table->first_header;

#ifdef ENABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
#else
    start    = getDoubleTimestamp();
#endif
    // go through the csv table
    csv_row_t *current_row = csv_table->first_row;
    while (current_row != NULL) {
        if (num_row_read % size == rank) {
            on_csv_row(current_row, llsm_args);
        }
        num_row_read++;
        current_row = current_row->next;
    }

#ifdef ENABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    duration = MPI_Wtime() - start;
#else
    duration = getDoubleTimestamp() - start;
#endif

    if (rank == 0) {
        printf("[Completion Time] LLSM IMPORTER FINISHES! Time taken: %.4f seconds\n", duration);
    }
    // free memory for csv table
    csv_free_table(csv_table);

    // close the container
    PDCcont_close(cont_id_g);
    // close the container property
    PDCprop_close(cont_prop_g);
    // close the pdc
    PDCclose(pdc_id_g);

#ifdef ENABLE_MPI
    MPI_Finalize();
#endif

    return 0;
}
