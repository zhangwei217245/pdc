#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pdc.h"
#include "pdc_prop.h"


int main() {
    PDC_prop p;
    PDCinit(p);

    // create an object
    PDC_prop_type type = PDC_OBJ_CREATE;
    pdcid_t create_prop1 = PDCprop_create(type);
    if(create_prop1 > 0) {
        if(type == PDC_CONT_CREATE)
            printf("Create a container, id is %lld\n", create_prop1);
        else if(type == PDC_OBJ_CREATE)
            printf("Create an object, id is %lld\n", create_prop1);
    }
    else {
        printf("Fail to create @ line %d\n", __LINE__);
    }
    // create another object
    pdcid_t create_prop2 = PDCprop_create(type);
    if(create_prop2 > 0) {
        if(type == PDC_CONT_CREATE)
            printf("Create a container, id is %lld\n", create_prop2);
        else if(type == PDC_OBJ_CREATE)
            printf("Create an object, id is %lld\n", create_prop2);
    }
    else {
        printf("Fail to create @ line %d\n", __LINE__);
    }

    if(PDCprop_close(create_prop1)<0)
        printf("Fail to close property @ line %d\n", __LINE__);
    else
        printf("successfully close property # %lld\n", create_prop1);
    if(PDCprop_close(create_prop2)<0)
        printf("Fail to close property @ line %d\n", __LINE__);
    else
        printf("successfully close property # %lld\n", create_prop2);

    type = PDC_CONT_CREATE;
    pdcid_t create_prop = PDCprop_create(type);
    if(create_prop > 0) {
        if(type == PDC_CONT_CREATE)
            printf("Create a container, id is %lld\n", create_prop);
        else if(type == PDC_OBJ_CREATE)
            printf("Create an object, id is %lld\n", create_prop);
    }
    else
        printf("Fail to create @ line  %d!\n", __LINE__);

   if(PDCprop_close(create_prop)<0)
       printf("Fail to close property @ line %d\n", __LINE__);
   else
       printf("successfully close property # %lld\n", create_prop);
}
