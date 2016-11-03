#ifndef _pdc_prop_pkg_H
#define _pdc_prop_pkg_H

#include "PDC_life.h"

typedef enum {
    PDC_CONT_CREATE = 0,
    PDC_OBJ_CREATE
} PDC_prop_type;

typedef struct PDC_cont_prop_t {
    PDC_lifetime    cont_life;
}PDC_cont_prop_t;

typedef struct PDC_obj_prop_t {
    PDC_lifetime    obj_life;
}PDC_obj_prop_t;

#endif
