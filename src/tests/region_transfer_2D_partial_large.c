/*
 * Copyright Notice for
 * Proactive Data Containers (PDC) Software Library and Utilities
 * -----------------------------------------------------------------------------

 *** Copyright Notice ***

 * Proactive Data Containers (PDC) Copyright (c) 2017, The Regents of the
 * University of California, through Lawrence Berkeley National Laboratory,
 * UChicago Argonne, LLC, operator of Argonne National Laboratory, and The HDF
 * Group (subject to receipt of any required approvals from the U.S. Dept. of
 * Energy).  All rights reserved.

 * If you have questions about your rights to use or distribute this software,
 * please contact Berkeley Lab's Innovation & Partnerships Office at  IPO@lbl.gov.

 * NOTICE.  This Software was developed under funding from the U.S. Department of
 * Energy and the U.S. Government consequently retains certain rights. As such, the
 * U.S. Government has been granted for itself and others acting on its behalf a
 * paid-up, nonexclusive, irrevocable, worldwide license in the Software to
 * reproduce, distribute copies to the public, prepare derivative works, and
 * perform publicly and display publicly, and to permit other to do so.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/time.h>
#include "pdc.h"
#define BUF_LEN 1000 * 1000 *32

int
main(int argc, char **argv)
{
    pdcid_t pdc, cont_prop, cont, obj_prop, reg, reg_global;
    perr_t  ret;
    pdcid_t obj1, obj2;
    char    cont_name[128], obj_name1[128], obj_name2[128];
    pdcid_t transfer_request;

    int rank = 0, size = 1;
    int ret_value = 0;

    uint64_t offset[2], offset_length[2];
    int      ndim = 2;
    uint64_t dims[2];
    offset[0]        = 0;
    offset[1]        = 0;
    offset_length[0] = BUF_LEN;
    offset_length[1] = 2;

    int *data      = (int *)malloc(sizeof(int) * BUF_LEN * 2);
    int *data_read = (int *)malloc(sizeof(int) * BUF_LEN * 2);
    dims[0]        = BUF_LEN;
    dims[1]        = 2;

#ifdef ENABLE_MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
#endif
    // create a pdc
    pdc = PDCinit("pdc");
    printf("create a new pdc\n");

    // create a container property
    cont_prop = PDCprop_create(PDC_CONT_CREATE, pdc);
    if (cont_prop > 0) {
        printf("Create a container property\n");
    }
    else {
        printf("Fail to create container property @ line  %d!\n", __LINE__);
        ret_value = 1;
    }
    // create a container
    sprintf(cont_name, "c%d", rank);
    cont = PDCcont_create(cont_name, cont_prop);
    if (cont > 0) {
        printf("Create a container c1\n");
    }
    else {
        printf("Fail to create container @ line  %d!\n", __LINE__);
        ret_value = 1;
    }
    // create an object property
    obj_prop = PDCprop_create(PDC_OBJ_CREATE, pdc);
    if (obj_prop > 0) {
        printf("Create an object property\n");
    }
    else {
        printf("Fail to create object property @ line  %d!\n", __LINE__);
        ret_value = 1;
    }

    ret = PDCprop_set_obj_type(obj_prop, PDC_INT);
    if (ret != SUCCEED) {
        printf("Fail to set obj type @ line %d\n", __LINE__);
        ret_value = 1;
    }
    PDCprop_set_obj_dims(obj_prop, ndim, dims);
    PDCprop_set_obj_user_id(obj_prop, getuid());
    PDCprop_set_obj_time_step(obj_prop, 0);
    PDCprop_set_obj_app_name(obj_prop, "DataServerTest");
    PDCprop_set_obj_tags(obj_prop, "tag0=1");

    pdcid_t obj_ids[10];

    for (int i = 0; i < 10; i++) {
        // create first object
        sprintf(obj_name1, "o1_%d", i);
        obj1 = PDCobj_create(cont, obj_name1, obj_prop);
        if (obj1 > 0) {
            printf("Create an object o1\n");
        }
        else {
            printf("Fail to create object @ line  %d!\n", __LINE__);
            ret_value = 1;
        }
        obj_ids[i] = obj1;

        offset[0]        = 0;
        offset[1]        = 0;
        offset_length[0] = BUF_LEN;
        offset_length[1] = 2;
        reg              = PDCregion_create(ndim, offset, offset_length);
        offset[0]        = 0;
        offset[1]        = 0;
        offset_length[0] = BUF_LEN;
        offset_length[1] = 2;
        reg_global       = PDCregion_create(ndim, offset, offset_length);

        for (int j = 0; j < BUF_LEN; ++j) {
            data[j] = j;
        }

        transfer_request = PDCregion_transfer_create(data, PDC_WRITE, obj1, reg, reg_global);

        PDCregion_transfer_start(transfer_request);
        PDCregion_transfer_wait(transfer_request);

        PDCregion_transfer_close(transfer_request);

        if (PDCregion_close(reg) < 0) {
            printf("fail to close local region @ line %d\n", __LINE__);
            ret_value = 1;
        }
        else {
            printf("successfully closed local region @ line %d\n", __LINE__);
        }

        if (PDCregion_close(reg_global) < 0) {
            printf("fail to close global region @ line %d\n", __LINE__);
            ret_value = 1;
        }
        else {
            printf("successfully closed global region @ line %d\n", __LINE__);
        }


    }


    // Read data from object
    for (int i = 0; i < 100000; i++) {
        int obj_id = i % 10;
        sprintf(obj_name1, "o1_%d", obj_id);

        obj1 = obj_ids[obj_id];

        offset[0]        = 0;
        offset[1]        = 0;
        offset_length[0] = BUF_LEN / 1000;
        offset_length[1] = 2;
        reg              = PDCregion_create(ndim, offset, offset_length);
        offset[0]        = 0;
        offset[1]        = 0;
        offset_length[0] = BUF_LEN / 1000;
        offset_length[1] = 2;
        reg_global       = PDCregion_create(ndim, offset, offset_length);

        memset(data_read, 0, BUF_LEN / 1000); // TODO: what should be the size of data_read here? 

        transfer_request = PDCregion_transfer_create(data_read, PDC_READ, obj1, reg, reg_global);

        PDCregion_transfer_start(transfer_request);
        PDCregion_transfer_wait(transfer_request);

        PDCregion_transfer_close(transfer_request);

        if (PDCregion_close(reg) < 0) {
            printf("fail to close local region @ line %d\n", __LINE__);
            ret_value = 1;
        }
        else {
            printf("successfully closed local region @ line %d\n", __LINE__);
        }

        if (PDCregion_close(reg_global) < 0) {
            printf("fail to close global region @ line %d\n", __LINE__);
            ret_value = 1;
        }
        else {
            printf("successfully global region @ line %d\n", __LINE__);
        }
    }

    // for (i = 0; i < BUF_LEN / 2; ++i) {
    //     if (data_read[i] != i * 2 + 1) {
    //         printf("wrong value %d!=%d @ line %d\n", data_read[i], i * 2 + 1, __LINE__);
    //         ret_value = 1;
    //         break;
    //     }
    // }

    
    for (int j = 0; j < 10; j++) {
        // close object
        if (PDCobj_close(obj_ids[j]) < 0) {
        printf("fail to close object o1 @ line %d\n", __LINE__);
            ret_value = 1;
        }
        else {
            printf("successfully close object o1 @ line %d\n", __LINE__);
        }
    }
    
    // close a container
    if (PDCcont_close(cont) < 0) {
        printf("fail to close container c1 @ line %d\n", __LINE__);
        ret_value = 1;
    }
    else {
        printf("successfully close container c1 @ line %d\n", __LINE__);
    }
    // close a object property
    if (PDCprop_close(obj_prop) < 0) {
        printf("Fail to close property @ line %d\n", __LINE__);
        ret_value = 1;
    }
    else {
        printf("successfully close object property @ line %d\n", __LINE__);
    }
    // close a container property
    if (PDCprop_close(cont_prop) < 0) {
        printf("Fail to close property @ line %d\n", __LINE__);
        ret_value = 1;
    }
    else {
        printf("successfully close container property @ line %d\n", __LINE__);
    }
    free(data);
    free(data_read);
    // close pdc
    if (PDCclose(pdc) < 0) {
        printf("fail to close PDC @ line %d\n", __LINE__);
        ret_value = 1;
    }
#ifdef ENABLE_MPI
    MPI_Finalize();
#endif
    return ret_value;
}
