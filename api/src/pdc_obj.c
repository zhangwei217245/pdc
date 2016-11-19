#include "pdc_obj.h"
#include "pdc_malloc.h"
#include "pdc_prop_pkg.h"

static perr_t PDCobj__close(PDC_obj_info_t *op);

/* PDC object ID class */
static const PDCID_class_t PDC_OBJ_CLS[1] = {{
    PDC_OBJ,                            /* ID class value */
    0,                                  /* Class flags */
    0,                                  /* # of reserved IDs for class */
    (PDC_free_t)PDCobj__close           /* Callback routine for closing objects of this class */
}};

perr_t PDCobj_init(PDC_CLASS_t *pc) {
    perr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER(NULL);

    /* Initialize the atom group for the object IDs */
    if(PDC_register_type(PDC_OBJ_CLS, pc) < 0)
        PGOTO_ERROR(FAIL, "unable to initialize object interface");

done:
    FUNC_LEAVE(ret_value);
} /* end PDCobj_init() */

pdcid_t PDCobj_create(pdcid_t pdc, pdcid_t cont_id, const char *obj_name, pdcid_t obj_create_prop){
    pdcid_t ret_value = SUCCEED;
    PDC_obj_info_t *p = NULL;
    
    FUNC_ENTER(NULL);
    
    p = PDC_MALLOC(PDC_obj_info_t);
    if(!p)
        PGOTO_ERROR(FAIL,"PDC object memory allocation failed\n");
    p->name = strdup(obj_name);
    p->cont = cont_id;
    p->pdc = pdc;
    p->obj_prop = obj_create_prop;
    // will contact server to get ID
    PDC_CLASS_t *pc = (PDC_CLASS_t *)pdc;
    pdcid_t new_id = PDC_id_register(PDC_OBJ, p, pdc);
    ret_value = new_id;
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDC_obj_list_null(pdcid_t pdc) {
    perr_t ret_value = SUCCEED;   /* Return value */
    
    FUNC_ENTER(NULL);
    // list is not empty
    int nelemts = PDC_id_list_null(PDC_OBJ, pdc);
    if(nelemts > 0) {
        printf("%d element(s) in the object list will be automatically closed by PDC_close()\n", nelemts);
        if(PDC_id_list_clear(PDC_OBJ, pdc) < 0)
            PGOTO_ERROR(FAIL, "fail to clear object list");
    }
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCobj__close(PDC_obj_info_t *op) {
    perr_t ret_value = SUCCEED;         /* Return value */
    
    FUNC_ENTER(NULL);
    
    op = PDC_FREE(PDC_obj_info_t, op);
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCobj_close(pdcid_t obj_id, pdcid_t pdc) {
    perr_t ret_value = SUCCEED;   /* Return value */
    
    FUNC_ENTER(NULL);
    
    /* When the reference count reaches zero the resources are freed */
    if(PDC_dec_ref(obj_id, pdc) < 0)
        PGOTO_ERROR(FAIL, "object: problem of freeing id");
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCobj_end(pdcid_t pdc) {
    perr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER(NULL);

    if(PDC_destroy_type(PDC_OBJ, pdc) < 0)
        PGOTO_ERROR(FAIL, "unable to destroy object interface");
done:
    FUNC_LEAVE(ret_value);
} /* end of PDCobj_end() */

pdcid_t PDCobj_open(pdcid_t cont_id, const char *obj_name, pdcid_t pdc) {
    pdcid_t ret_value = SUCCEED;
    
    FUNC_ENTER(NULL);
    
    // should wait for response from server
    // look up in the list for now
    pdcid_t ret_value1;
    ret_value1 = PDC_find_byname(PDC_OBJ, obj_name, pdc);
    if(ret_value1 <= 0)
        PGOTO_ERROR(FAIL, "cannot locate object");
    pdc_inc_ref(ret_value1, pdc);
    ret_value = ret_value1;
done:
    FUNC_LEAVE(ret_value);
} /* end of PDCobj_open() */

obj_handle *PDCobj_iter_start(pdcid_t cont_id, pdcid_t pdc_id) {
    obj_handle *ret_value = NULL;
    obj_handle *objhl = NULL;
    
    FUNC_ENTER(NULL);
    
    PDC_CLASS_t *pc = (PDC_CLASS_t *)pdc_id;
    PDC_id_type_t *type_ptr  = (pc->PDC_id_type_list_g)[PDC_OBJ];
    if(type_ptr == NULL)
        PGOTO_ERROR(NULL, "object list is empty");
    objhl = (&type_ptr->ids)->head;
    while(objhl!=NULL && ((PDC_obj_info_t *)(objhl->obj_ptr))->cont!=cont_id) {
        objhl = PDC_LIST_NEXT(objhl, entry);
    }
    ret_value = objhl;
done:
    FUNC_LEAVE(ret_value);
} /* end of PDCobj_iter_start() */

pbool_t PDCobj_iter_null(obj_handle *ohandle) {
    pbool_t ret_value = FALSE;
    
    FUNC_ENTER(NULL);
    
    if(ohandle == NULL)
        ret_value = TRUE;
done:
    FUNC_LEAVE(ret_value);
} /* end of PDCobj_iter_null() */

obj_handle *PDCobj_iter_next(obj_handle *ohandle, pdcid_t cont_id) {
    obj_handle *ret_value = NULL;
    obj_handle *next = NULL;
    
    FUNC_ENTER(NULL);
    
    if(ohandle == NULL)
        PGOTO_ERROR(NULL, "no next object");
    next = PDC_LIST_NEXT(ohandle, entry);
    while(next!=NULL && ((PDC_obj_info_t *)(next->obj_ptr))->cont!= cont_id) {
        next = PDC_LIST_NEXT(ohandle, entry);
    }
    ret_value = next;
done:
    FUNC_LEAVE(ret_value);
} /* end of PDCobj_iter_next() */

PDC_obj_info_t *PDCobj_iter_get_info(obj_handle *ohandle) {
    PDC_obj_info_t *ret_value = NULL;
    PDC_obj_info_t *info = NULL;
    
    FUNC_ENTER(NULL);
    info = (PDC_obj_info_t *)(ohandle->obj_ptr);
    if(info == NULL)
        PGOTO_ERROR(NULL, "PDC container info memory allocation failed");
    ret_value = info;
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCprop_set_obj_lifetime(pdcid_t obj_prop, PDC_lifetime obj_lifetime, pdcid_t pdc) {
    perr_t ret_value = SUCCEED;         /* Return value */
    
    FUNC_ENTER(NULL);
    
    PDC_CLASS_t *pc = (PDC_CLASS_t *)pdc;
    PDC_id_info_t *info = PDC_find_id(obj_prop, pc);
    if(info == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object property ID");
    ((PDC_obj_prop_t *)(info->obj_ptr))->obj_life = obj_lifetime;
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCprop_set_obj_dims(pdcid_t obj_prop, PDC_int_t ndim, uint64_t *dims, pdcid_t pdc) {
    perr_t ret_value = SUCCEED;         /* Return value */
    
    FUNC_ENTER(NULL);
    
    PDC_CLASS_t *pc = (PDC_CLASS_t *)pdc;
    PDC_id_info_t *info = PDC_find_id(obj_prop, pc);
    if(info == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object property ID");
    PDC_obj_prop_t *prop = (PDC_obj_prop_t *)(info->obj_ptr);
    prop->ndim = ndim;
    prop->dims = (uint64_t *)malloc(ndim * sizeof(uint64_t));
    
    int i = 0;
    for(i=0; i<ndim; i++)
        (prop->dims)[i] = dims[i];
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCprop_set_obj_type(pdcid_t obj_prop, PDC_var_type_t type, pdcid_t pdc) {
    perr_t ret_value = SUCCEED;         /* Return value */
    
    FUNC_ENTER(NULL);
    
    PDC_CLASS_t *pc = (PDC_CLASS_t *)pdc;
    PDC_id_info_t *info = PDC_find_id(obj_prop, pc);
    if(info == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object property ID");
    PDC_obj_prop_t *prop = (PDC_obj_prop_t *)(info->obj_ptr);
    prop->type = type;
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCprop_set_obj_buf(pdcid_t obj_create_prop, void *buf) {
}

perr_t PDCobj_buf_retrieve(pdcid_t obj_id, void **buf, PDC_region region) {
}

obj_handle PDCview_iter_start(pdcid_t view_id) {
}

perr_t PDCobj_buf_map(pdcid_t obj_id, void *buf, PDC_region region) {
}

perr_t PDCobj_map(pdcid_t a, PDC_region xregion, pdcid_t b, PDC_region yregion) {
}

perr_t PDCobj_unmap(pdcid_t obj_id) {
}

perr_t PDCobj_release(pdcid_t obj_id) {
}

perr_t PDCobj_update_region(pdcid_t obj_id, PDC_region region) {
}

perr_t PDCobj_invalidate_region(pdcid_t obj_id, PDC_region region) {
}

perr_t PDCobj_sync(pdcid_t obj_id) {
}

perr_t PDCprop_set_obj_loci_prop(pdcid_t obj_create_prop, PDC_loci locus, PDC_transform A) {
}

perr_t PDCprop_set_obj_transform(pdcid_t obj_create_prop, PDC_loci pre_locus, PDC_transform A, PDC_loci dest_locus) {
}

