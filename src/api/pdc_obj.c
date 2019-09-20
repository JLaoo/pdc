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

#include "config.h"
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "../server/utlist.h"
#include "pdc_obj.h"
#include "pdc_malloc.h"
#include "pdc_prop_pkg.h"
#include "pdc_obj_private.h"
#include "pdc_interface.h"
#include "pdc_transforms_pkg.h"
#include "pdc_atomic.h"
#include "pdc_client_connect.h"
#include "pdc_analysis_common.h"

static perr_t pdc_obj_close(struct PDC_obj_info *op);

static perr_t pdc_region_close(struct PDC_region_info *op);

perr_t PDC_obj_init()
{
    perr_t ret_value = SUCCEED; 

    FUNC_ENTER(NULL);

    /* Initialize the atom group for the object IDs */
    if(PDC_register_type(PDC_OBJ, (PDC_free_t)pdc_obj_close) < 0)
        PGOTO_ERROR(FAIL, "unable to initialize object interface");

done:
    FUNC_LEAVE(ret_value);
} 

perr_t PDC_region_init()
{
    perr_t ret_value = SUCCEED;
    
    FUNC_ENTER(NULL);
    
    /* Initialize the atom group for the region IDs */
    if(PDC_register_type(PDC_REGION, (PDC_free_t)pdc_region_close) < 0)
        PGOTO_ERROR(FAIL, "unable to initialize region interface");
    
done:
    FUNC_LEAVE(ret_value);
} 

pdcid_t PDCobj_create(pdcid_t cont_id, const char *obj_name, pdcid_t obj_prop_id)
{
    uint64_t meta_id;
    pdcid_t ret_value = 0;
    struct PDC_cont_info *cont_info;
    struct PDC_obj_prop *obj_prop;
    struct PDC_obj_info *p = NULL;
    struct PDC_id_info *id_info = NULL;
    perr_t ret;
    size_t i;

    FUNC_ENTER(NULL);

    p = PDC_MALLOC(struct PDC_obj_info);
    if(!p)
        PGOTO_ERROR(0, "PDC object memory allocation failed\n");
    p->name = strdup(obj_name);
    p->metadata = NULL;
    p->region_list_head = NULL;

    if (cont_id == 0) {
        meta_id = 0;
    }
    else {
        id_info = PDC_find_id(cont_id);
        cont_info = (struct PDC_cont_info *)(id_info->obj_ptr);
        p->cont = PDC_CALLOC(struct PDC_cont_info);
        if(!p->cont)
            PGOTO_ERROR(0, "PDC object container memory allocation failed\n");
        memcpy(p->cont, cont_info, sizeof(struct PDC_cont_info));
        if(cont_info->name)
            p->cont->name = strdup(cont_info->name);
        
        p->cont->cont_pt = PDC_CALLOC(struct PDC_cont_prop);
        if(!p->cont->cont_pt)
            PGOTO_ERROR(0, "PDC object container property memory allocation failed\n");
        memcpy(p->cont->cont_pt, cont_info->cont_pt, sizeof(struct PDC_cont_prop));
        
        p->cont->cont_pt->pdc = PDC_CALLOC(struct PDC_class);
        if(!p->cont->cont_pt->pdc)
            PGOTO_ERROR(0, "PDC object container property pdc memory allocation failed\n");
        p->cont->cont_pt->pdc->name = strdup(cont_info->cont_pt->pdc->name);
        p->cont->cont_pt->pdc->local_id = cont_info->cont_pt->pdc->local_id;
        meta_id = p->cont->meta_id;
    }
   
    id_info = PDC_find_id(obj_prop_id);
    obj_prop = (struct PDC_obj_prop *)(id_info->obj_ptr);
    p->obj_pt = PDC_CALLOC(struct PDC_obj_prop);
    if(!p->obj_pt)
        PGOTO_ERROR(0, "PDC object property memory allocation failed");
    memcpy(p->obj_pt, obj_prop, sizeof(struct PDC_obj_prop));
    if(obj_prop->app_name)
        p->obj_pt->app_name = strdup(obj_prop->app_name);
    p->obj_pt->pdc = PDC_CALLOC(struct PDC_class);
    if(!p->obj_pt->pdc)
        PGOTO_ERROR(0, "cannot allocate ret_value->pdc");
    p->obj_pt->pdc->name = strdup(obj_prop->pdc->name);
    p->obj_pt->pdc->local_id = obj_prop->pdc->local_id;

    p->obj_pt->dims = malloc(obj_prop->ndim*sizeof(uint64_t));
    if(!p->obj_pt->dims)
        PGOTO_ERROR(0, "cannot allocate ret_value->dims");
    for(i=0; i<obj_prop->ndim; i++)
        p->obj_pt->dims[i] = obj_prop->dims[i];
    
    if(obj_prop->app_name)
        p->obj_pt->app_name = strdup(obj_prop->app_name);
    if(obj_prop->data_loc)
        p->obj_pt->data_loc = strdup(obj_prop->data_loc);
    if(obj_prop->tags)
        p->obj_pt->tags = strdup(obj_prop->tags);

    ret = PDC_Client_send_name_recv_id(obj_name, meta_id, obj_prop_id, &(p->meta_id));
    if (ret == FAIL)
        PGOTO_ERROR(0, "Unable to create object on server!\n");
    
    p->local_id = PDC_id_register(PDC_OBJ, p);
    ret_value = p->local_id;
//  PDC_Client_attach_metadata_to_local_obj((char *)obj_name, p->meta_id, p->cont->meta_id, p);

done:
    FUNC_LEAVE(ret_value);
}

pdcid_t PDC_obj_create(pdcid_t cont_id, const char *obj_name, pdcid_t obj_prop_id, PDCobj_location location)
{
    pdcid_t ret_value = 0;
    struct PDC_obj_info *p = NULL;
    struct PDC_id_info *id_info = NULL;
    struct PDC_cont_info *cont_info = NULL;
    struct PDC_obj_prop *obj_prop;
    uint64_t meta_id;
    size_t i;
    perr_t ret = SUCCEED;

    FUNC_ENTER(NULL);

    p = PDC_MALLOC(struct PDC_obj_info);
    if(!p)
        PGOTO_ERROR(0, "PDC object memory allocation failed\n");
    p->name = strdup(obj_name);
    p->metadata = NULL;
    p->location = location;
    p->region_list_head = NULL;

    if (cont_id == 0) {
        meta_id = 0;
    }
    else {
        id_info = PDC_find_id(cont_id);
        cont_info = (struct PDC_cont_info *)(id_info->obj_ptr);
            p->cont = PDC_CALLOC(struct PDC_cont_info);
        if(!p->cont)
            PGOTO_ERROR(0, "PDC object container memory allocation failed");
        memcpy(p->cont, cont_info, sizeof(struct PDC_cont_info));
        if(cont_info->name)
            p->cont->name = strdup(cont_info->name);
        
        p->cont->cont_pt = PDC_CALLOC(struct PDC_cont_prop);
        if(!p->cont->cont_pt)
            PGOTO_ERROR(0, "PDC object container property memory allocation failed");
        memcpy(p->cont->cont_pt, cont_info->cont_pt, sizeof(struct PDC_cont_prop));
        
        p->cont->cont_pt->pdc = PDC_CALLOC(struct PDC_class);
        if(!p->cont->cont_pt->pdc)
            PGOTO_ERROR(0, "PDC object container property pdc memory allocation failed\n");
        if(cont_info->cont_pt->pdc->name)
            p->cont->cont_pt->pdc->name = strdup(cont_info->cont_pt->pdc->name);
        p->cont->cont_pt->pdc->local_id = cont_info->cont_pt->pdc->local_id;
    }
    
    id_info = PDC_find_id(obj_prop_id);
    obj_prop = (struct PDC_obj_prop *)(id_info->obj_ptr);
    p->obj_pt = PDC_CALLOC(struct PDC_obj_prop);
    if(!p->obj_pt)
        PGOTO_ERROR(0, "PDC object property memory allocation failed");
    memcpy(p->obj_pt, obj_prop, sizeof(struct PDC_obj_prop));

    p->obj_pt->pdc = PDC_CALLOC(struct PDC_class);
    if(!p->obj_pt->pdc)
        PGOTO_ERROR(0, "cannot allocate ret_value->pdc");
    if(obj_prop->pdc->name)
        p->obj_pt->pdc->name = strdup(obj_prop->pdc->name);
    p->obj_pt->pdc->local_id = obj_prop->pdc->local_id;

    p->obj_pt->dims = malloc(obj_prop->ndim*sizeof(uint64_t));
    if(!p->obj_pt->dims)
        PGOTO_ERROR(0, "cannot allocate ret_value->dims");
    for(i=0; i<obj_prop->ndim; i++)
        p->obj_pt->dims[i] = obj_prop->dims[i];
    
    if(obj_prop->app_name)
        p->obj_pt->app_name = strdup(obj_prop->app_name);
    if(obj_prop->data_loc)
        p->obj_pt->data_loc = strdup(obj_prop->data_loc);
    if(obj_prop->tags)
        p->obj_pt->tags = strdup(obj_prop->tags);

    p->obj_pt->locus = PDC_get_execution_locus();
    p->meta_id = 0;
    p->local_id = PDC_id_register(PDC_OBJ, p);

    if(location == PDC_OBJ_GLOBAL) {
        ret = PDC_Client_send_name_recv_id(obj_name, p->cont->meta_id, obj_prop_id, &(p->meta_id));
        if (ret == FAIL)
            PGOTO_ERROR(0, "Unable to create object on server!\n");
    }
    
    PDC_Client_attach_metadata_to_local_obj(obj_name, p->meta_id, meta_id, p);

    ret_value = p->local_id;

done:
    FUNC_LEAVE(ret_value);
}

perr_t PDC_obj_list_null()
{
    perr_t ret_value = SUCCEED;
    int nelemts;
    
    FUNC_ENTER(NULL);
    
    // list is not empty
    nelemts = PDC_id_list_null(PDC_OBJ);
    if(nelemts > 0) {
        if(PDC_id_list_clear(PDC_OBJ) < 0)
            PGOTO_ERROR(FAIL, "fail to clear object list");
    }
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDC_region_list_null()
{
    perr_t ret_value = SUCCEED;
    int nelemts;
    
    FUNC_ENTER(NULL);

    // list is not empty
    nelemts = PDC_id_list_null(PDC_REGION);
    if(nelemts > 0) {
        if(PDC_id_list_clear(PDC_REGION) < 0)
            PGOTO_ERROR(FAIL, "fail to clear object list");
    }
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t pdc_obj_close(struct PDC_obj_info *op)
{
    perr_t ret_value = SUCCEED;
    
    FUNC_ENTER(NULL);
   
    free((void*)(op->name));
    free(op->cont->name);
    free(op->cont->cont_pt->pdc->name);
    op->cont->cont_pt->pdc = PDC_FREE(struct PDC_class, op->cont->cont_pt->pdc);
    op->cont->cont_pt = PDC_FREE(struct PDC_cont_prop, op->cont->cont_pt);
    op->cont = PDC_FREE(struct PDC_cont_info, op->cont);
 
    free(op->obj_pt->pdc->name);
    op->obj_pt->pdc = PDC_FREE(struct PDC_class, op->obj_pt->pdc);
    free(op->obj_pt->dims);
    free(op->obj_pt->app_name);
    free(op->obj_pt->data_loc);
    free(op->obj_pt->tags);
    op->obj_pt = PDC_FREE(struct PDC_obj_prop, op->obj_pt);
    if (op->metadata != NULL) free(op->metadata);

    op = PDC_FREE(struct PDC_obj_info, op);
    
    FUNC_LEAVE(ret_value);
}

perr_t pdc_region_close(struct PDC_region_info *op)
{
    perr_t ret_value = SUCCEED;
    
    FUNC_ENTER(NULL);
    
    free(op->size);
    free(op->offset);
    if(op->obj!=NULL)
        op->obj = PDC_FREE(struct PDC_obj_info, op->obj);
    op = PDC_FREE(struct PDC_region_info, op);
    
    FUNC_LEAVE(ret_value);
}

perr_t PDCobj_close(pdcid_t obj_id)
{
    perr_t ret_value = SUCCEED;
    
    FUNC_ENTER(NULL);
    
    /* When the reference count reaches zero the resources are freed */
    if(PDC_dec_ref(obj_id) < 0)
        PGOTO_ERROR(FAIL, "object: problem of freeing id");
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCregion_close(pdcid_t region_id)
{
    perr_t ret_value = SUCCEED;
    
    FUNC_ENTER(NULL);
    
    /* When the reference count reaches zero the resources are freed */
    if(PDC_dec_ref(region_id) < 0)
        PGOTO_ERROR(FAIL, "object: problem of freeing id");
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDC_obj_end()
{
    perr_t ret_value = SUCCEED;

    FUNC_ENTER(NULL);

    if(PDC_destroy_type(PDC_OBJ) < 0)
        PGOTO_ERROR(FAIL, "unable to destroy object interface");
    
done:
    FUNC_LEAVE(ret_value);
} 

perr_t PDC_region_end()
{
    perr_t ret_value = SUCCEED;
    
    FUNC_ENTER(NULL);
    
    if(PDC_destroy_type(PDC_REGION) < 0)
        PGOTO_ERROR(FAIL, "unable to destroy region interface");
    
done:
    FUNC_LEAVE(ret_value);
} 

pdcid_t PDCobj_open(const char *obj_name, pdcid_t pdc)
{
    pdcid_t ret_value = 0;
    perr_t ret = SUCCEED;
    struct PDC_obj_info *p = NULL;
    pdc_metadata_t *out = NULL;
    pdcid_t obj_prop;
    size_t i;
    
    FUNC_ENTER(NULL);
        
    p = PDC_MALLOC(struct PDC_obj_info);
    if(!p)
        PGOTO_ERROR(0, "PDC object memory allocation failed\n");
    p->cont = PDC_CALLOC(struct PDC_cont_info);
    if(!p->cont)
        PGOTO_ERROR(0, "PDC object container memory allocation failed");
    p->cont->cont_pt = PDC_CALLOC(struct PDC_cont_prop);
    if(!p->cont->cont_pt)
        PGOTO_ERROR(0, "PDC object container property memory allocation failed");
    p->cont->cont_pt->pdc = PDC_CALLOC(struct PDC_class);
    if(!p->cont->cont_pt->pdc)
        PGOTO_ERROR(0, "PDC object container property pdc memory allocation failed\n");
    p->obj_pt = PDC_CALLOC(struct PDC_obj_prop);
    if(!p->obj_pt)
        PGOTO_ERROR(0, "PDC object property memory allocation failed");
    p->obj_pt->pdc = PDC_CALLOC(struct PDC_class);
    if(!p->obj_pt->pdc)
        PGOTO_ERROR(0, "cannot allocate ret_value->pdc");
    
    // contact metadata server
    ret = PDC_Client_query_metadata_name_timestep_agg(obj_name, 0, &out);
    if(ret == FAIL)
        PGOTO_ERROR(0, "query object failed");
        
    obj_prop = PDCprop_create(PDC_OBJ_CREATE, pdc);
    PDCprop_set_obj_dims(obj_prop, out->ndim, out->dims);
    PDCprop_set_obj_type(obj_prop, out->data_type);

    /* 'obj_name' is a char array */
    if(strlen(out->obj_name) > 0)
        p->name = strdup(out->obj_name);
    p->meta_id = out->obj_id;

    p->cont->meta_id = out->cont_id;
    p->obj_pt->ndim = out->ndim;
    p->obj_pt->dims = malloc(out->ndim*sizeof(uint64_t));
    if(!p->obj_pt->dims)
        PGOTO_ERROR(0, "cannot allocate ret_value->dims");
    for(i=0; i<out->ndim; i++)
        p->obj_pt->dims[i] = out->dims[i];
    /* 'app_name' is a char array */
    if(strlen(out->app_name) > 0)
        p->obj_pt->app_name = strdup(out->app_name);
    p->obj_pt->type = out->data_type;
    p->obj_pt->time_step = out->time_step;
    p->obj_pt->user_id = out->user_id;
    
    if (out->transform_state > 0) {
        p->obj_pt->locus = SERVER_MEMORY;
        p->obj_pt->data_state = out->transform_state;
        p->obj_pt->transform_prop.storage_order = out->current_state.storage_order;
        p->obj_pt->transform_prop.dtype = out->current_state.dtype;
        p->obj_pt->transform_prop.ndim = out->current_state.ndim;
	for(i=0; i < out->current_state.ndim; i++)
            p->obj_pt->transform_prop.dims[i] = out->current_state.dims[i];
    }
    p->metadata = out;
    p->local_id = PDC_id_register(PDC_OBJ, p);
    ret_value = p->local_id;

done:

    FUNC_LEAVE(ret_value);
} 

obj_handle *PDCobj_iter_start(pdcid_t cont_id)
{
    obj_handle *ret_value = NULL;
    obj_handle *objhl = NULL;
    struct PDC_id_type *type_ptr;
    
    FUNC_ENTER(NULL);
    
    type_ptr  = (pdc_id_list_g->PDC_id_type_list_g)[PDC_OBJ];
    if(type_ptr == NULL)
        PGOTO_ERROR(NULL, "object list is empty");
    objhl = (&type_ptr->ids)->head;
    
    while(objhl!=NULL && ((struct PDC_obj_info *)(objhl->obj_ptr))->cont->local_id!=cont_id) {
        objhl = PDC_LIST_NEXT(objhl, entry);
    }
    
    ret_value = objhl;
    
done:
    FUNC_LEAVE(ret_value);
} 

pbool_t PDCobj_iter_null(obj_handle *ohandle)
{
    pbool_t ret_value = FALSE;
    
    FUNC_ENTER(NULL);
    
    if(ohandle == NULL)
        ret_value = TRUE;
    
    FUNC_LEAVE(ret_value);
}

obj_handle *PDCobj_iter_next(obj_handle *ohandle, pdcid_t cont_id)
{
    obj_handle *ret_value = NULL;
    obj_handle *next = NULL;
    
    FUNC_ENTER(NULL);
    
    if(ohandle == NULL)
        PGOTO_ERROR(NULL, "no next object");
    next = PDC_LIST_NEXT(ohandle, entry);
    
    while(next!=NULL && ((struct PDC_obj_info *)(next->obj_ptr))->cont->local_id!=cont_id) {
        next = PDC_LIST_NEXT(ohandle, entry);
    }
   
    ret_value = next;
    
done:
    FUNC_LEAVE(ret_value);
}

struct PDC_obj_info *PDCobj_iter_get_info(obj_handle *ohandle)
{
    struct PDC_obj_info *ret_value = NULL;
    struct PDC_obj_info *info = NULL;
    
    FUNC_ENTER(NULL);
    info = (struct PDC_obj_info *)(ohandle->obj_ptr);
    if(info == NULL)
        PGOTO_ERROR(NULL, "PDC container info memory allocation failed");
    
    ret_value = info;
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCprop_set_obj_lifetime(pdcid_t obj_prop, PDC_lifetime obj_lifetime)
{
    perr_t ret_value = SUCCEED;   
    struct PDC_id_info *info;
    
    FUNC_ENTER(NULL);
    
    info = PDC_find_id(obj_prop);
    if(info == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object property ID");
    ((struct PDC_obj_prop *)(info->obj_ptr))->obj_life = obj_lifetime;
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCprop_set_obj_user_id(pdcid_t obj_prop, uint32_t user_id)
{
    perr_t ret_value = SUCCEED;   
    struct PDC_id_info *info;
    
    FUNC_ENTER(NULL);
    
    info = PDC_find_id(obj_prop);
    if(info == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object property ID");
    ((struct PDC_obj_prop *)(info->obj_ptr))->user_id = user_id;
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCprop_set_obj_app_name(pdcid_t obj_prop, char *app_name)
{
    perr_t ret_value = SUCCEED;  
    struct PDC_id_info *info;
    
    FUNC_ENTER(NULL);
    
    info = PDC_find_id(obj_prop);
    if(info == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object property ID");
    ((struct PDC_obj_prop *)(info->obj_ptr))->app_name = strdup(app_name);
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCprop_set_obj_time_step(pdcid_t obj_prop, uint32_t time_step)
{
    perr_t ret_value = SUCCEED; 
    struct PDC_id_info *info;
    
    FUNC_ENTER(NULL);
    
    info = PDC_find_id(obj_prop);
    if(info == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object property ID");
    ((struct PDC_obj_prop *)(info->obj_ptr))->time_step = time_step;
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCprop_set_obj_data_loc(pdcid_t obj_prop, char *loc) 
{
    perr_t ret_value = SUCCEED;     
    struct PDC_id_info *info;
    
    FUNC_ENTER(NULL);
    
    info = PDC_find_id(obj_prop);
    if(info == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object property ID");
    ((struct PDC_obj_prop *)(info->obj_ptr))->data_loc = strdup(loc);
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCprop_set_obj_tags(pdcid_t obj_prop, char *tags)
{
    perr_t ret_value = SUCCEED;  
    struct PDC_id_info *info;
    
    FUNC_ENTER(NULL);
    
    info = PDC_find_id(obj_prop);
    if(info == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object property ID");
    ((struct PDC_obj_prop *)(info->obj_ptr))->tags = strdup(tags);
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCprop_set_obj_dims(pdcid_t obj_prop, PDC_int_t ndim, uint64_t *dims)
{
    perr_t ret_value = SUCCEED; 
    struct PDC_id_info *info;
    struct PDC_obj_prop *prop;
    
    FUNC_ENTER(NULL);
    
    info = PDC_find_id(obj_prop);
    if(info == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object property ID");
    prop = (struct PDC_obj_prop *)(info->obj_ptr);
    prop->ndim = ndim;
    prop->dims = (uint64_t *)malloc(ndim * sizeof(uint64_t));
    
    int i = 0;
    for(i=0; i<ndim; i++)
        (prop->dims)[i] = dims[i];
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCprop_set_obj_type(pdcid_t obj_prop, PDC_var_type_t type)
{
    perr_t ret_value = SUCCEED;
    struct PDC_id_info *info;
    struct PDC_obj_prop *prop;
    
    FUNC_ENTER(NULL);
    
    info = PDC_find_id(obj_prop);
    if(info == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object property ID");
    prop = (struct PDC_obj_prop *)(info->obj_ptr);
    prop->type = type;
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCprop_set_obj_buf(pdcid_t obj_prop, void *buf)
{
    perr_t ret_value = SUCCEED;
    struct PDC_id_info *info;
    struct PDC_obj_prop *prop;
    
    FUNC_ENTER(NULL);
    
    info = PDC_find_id(obj_prop);
    if(info == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object property ID");
    prop = (struct PDC_obj_prop *)(info->obj_ptr);
    prop->buf = buf;
    
done:
    FUNC_LEAVE(ret_value);
}

void **PDCobj_buf_retrieve(pdcid_t obj_id)
{
    void **ret_value = NULL;
    struct PDC_id_info *info;
    struct PDC_obj_info *object;
    
    FUNC_ENTER(NULL);
    
    info = PDC_find_id(obj_id);
    if(info == NULL)
        PGOTO_ERROR(NULL, "cannot locate object ID");
    object = (struct PDC_obj_info *)(info->obj_ptr);
    void **buffer = &(object->obj_pt->buf);
    ret_value = buffer;
    
done:
    FUNC_LEAVE(ret_value);
}

pdcid_t PDCregion_create(size_t ndims, uint64_t *offset, uint64_t *size)
{
    pdcid_t ret_value = 0;    
    struct PDC_region_info *p = NULL;
    pdcid_t new_id;
    size_t i = 0;
 
    FUNC_ENTER(NULL);
    
    p = PDC_MALLOC(struct PDC_region_info);
    if(!p)
        PGOTO_ERROR(ret_value, "PDC region memory allocation failed\n");
    p->ndim = ndims;
    p->obj = NULL;
    p->offset = (uint64_t *)malloc(ndims * sizeof(uint64_t));
    p->size = (uint64_t *)malloc(ndims * sizeof(uint64_t));
    p->mapping = 0;
    p->local_id = 0;
    for(i=0; i<ndims; i++) {
        (p->offset)[i] = offset[i];
        (p->size)[i] = size[i];
    }
    new_id = PDC_id_register(PDC_REGION, p);
    p->local_id = new_id;
    ret_value = new_id;
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCbuf_obj_map(void *buf, PDC_var_type_t local_type, pdcid_t local_reg, pdcid_t remote_obj, pdcid_t remote_reg)
{
    pdcid_t ret_value = SUCCEED;    
    size_t i;
    struct PDC_id_info *objinfo2;
    struct PDC_obj_info *obj2;
    pdcid_t remote_meta_id;

    PDC_var_type_t remote_type;
    struct PDC_id_info *reginfo1, *reginfo2;
    struct PDC_region_info *reg1, *reg2;
    
    FUNC_ENTER(NULL);

    reginfo1 = PDC_find_id(local_reg);
    reg1 = (struct PDC_region_info *)(reginfo1->obj_ptr);

    objinfo2 = PDC_find_id(remote_obj);
    if(objinfo2 == NULL)
        PGOTO_ERROR(FAIL, "cannot locate remote object ID");
    obj2 = (struct PDC_obj_info *)(objinfo2->obj_ptr);
    remote_meta_id = obj2->meta_id;
    remote_type = obj2->obj_pt->type;
  
    reginfo2 = PDC_find_id(remote_reg);
    reg2 = (struct PDC_region_info *)(reginfo2->obj_ptr);
    if(obj2->obj_pt->ndim != reg2->ndim)
        PGOTO_ERROR(FAIL, "remote object dimension and region dimension does not match");
    for(i=0; i<reg2->ndim; i++)
          if((obj2->obj_pt->dims)[i] < (reg2->size)[i])
            PGOTO_ERROR(FAIL, "remote object region size error");

    ret_value = PDC_Client_buf_map(local_reg, remote_meta_id, reg1->ndim, reg1->size, reg1->offset, local_type, buf, remote_type, reg1, reg2);

    if(ret_value == SUCCEED) {
        /* 
	 * For analysis and/or transforms, we only identify the target region as being mapped.
	 * The lock/unlock protocol for writing will protect the target from being written by
	 * more than one source.
	 */
        PDC_check_transform(PDC_DATA_MAP, reg2);
        PDC_inc_ref(remote_obj);
        PDC_inc_ref(remote_reg);
    }
done:
    FUNC_LEAVE(ret_value);
}

struct PDC_region_info *PDCregion_get_info(pdcid_t reg_id)
{
    struct PDC_region_info *ret_value = NULL;
    struct PDC_region_info *info =  NULL;
    struct PDC_id_info *region;
    
    FUNC_ENTER(NULL);
    
    region = PDC_find_id(reg_id);
    if(region == NULL)
        PGOTO_ERROR(NULL, "cannot locate region");
    
    info = (struct PDC_region_info *)(region->obj_ptr);
    ret_value = info;
    
done:
    FUNC_LEAVE(ret_value);
} 

perr_t PDCbuf_obj_unmap(pdcid_t remote_obj_id, pdcid_t remote_reg_id)
{
    perr_t ret_value = SUCCEED;   
    struct PDC_id_info *info1;
    struct PDC_obj_info *object1;
    struct PDC_region_info *reginfo;
    PDC_var_type_t data_type;

    FUNC_ENTER(NULL);

    info1 = PDC_find_id(remote_obj_id);
    if(info1 == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object ID");
    object1 = (struct PDC_obj_info *)(info1->obj_ptr);
    data_type = object1->obj_pt->type;

    info1 = PDC_find_id(remote_reg_id);
    if(info1 == NULL)
        PGOTO_ERROR(FAIL, "cannot locate region ID");
    reginfo = (struct PDC_region_info *)(info1->obj_ptr);

    ret_value = PDC_Client_buf_unmap(object1->meta_id, remote_reg_id, reginfo, data_type);

    if(ret_value == SUCCEED) { 
        PDC_dec_ref(remote_obj_id);
        PDC_dec_ref(remote_reg_id); 
    } 
done:
    FUNC_LEAVE(ret_value);
}

struct PDC_obj_info *PDC_obj_get_info(pdcid_t obj_id)
{
    struct PDC_obj_info *ret_value = NULL;
    struct PDC_obj_info *info =  NULL;
    struct PDC_id_info *obj;
    size_t i;
    
    FUNC_ENTER(NULL);
    
    obj = PDC_find_id(obj_id);
    if(obj == NULL)
        PGOTO_ERROR(NULL, "cannot locate object");
    
    info = (struct PDC_obj_info *)(obj->obj_ptr);
    ret_value = PDC_CALLOC(struct PDC_obj_info);
    if(ret_value)
        memcpy(ret_value, info, sizeof(struct PDC_obj_info));
    else
        PGOTO_ERROR(NULL, "cannot allocate ret_value");
    if(info->name)
        ret_value->name = strdup(info->name);
    else
        ret_value->name = NULL;
    // fill in by query function
    ret_value->metadata = NULL;
    
    // fill in struct PDC_cont_info field in ret_value->cont
    ret_value->cont = PDC_CALLOC(struct PDC_cont_info);
    if(ret_value->cont)
        memcpy(ret_value->cont, info->cont, sizeof(struct PDC_cont_info));
    else
        PGOTO_ERROR(NULL, "cannot allocate ret_value->cont");
    if(info->cont->name)
        ret_value->cont->name = strdup(info->cont->name);
    else
        ret_value->cont->name = NULL;
    ret_value->cont->cont_pt = PDC_CALLOC(struct PDC_cont_prop);
    if(ret_value->cont->cont_pt)
        memcpy(ret_value->cont->cont_pt, info->cont->cont_pt, sizeof(struct PDC_cont_prop));
    else
        PGOTO_ERROR(NULL, "cannot allocate ret_value->cont->cont_pt");
    ret_value->cont->cont_pt->pdc = PDC_CALLOC(struct PDC_class);
    if(ret_value->cont->cont_pt->pdc) {
        ret_value->cont->cont_pt->pdc->local_id = info->cont->cont_pt->pdc->local_id;
        if(info->cont->cont_pt->pdc->name)
            ret_value->cont->cont_pt->pdc->name = strdup(info->cont->cont_pt->pdc->name);
        else
            ret_value->cont->cont_pt->pdc->name = NULL;
    }
    else
        PGOTO_ERROR(NULL, "cannot allocate ret_value->cont->cont_pt->pdc");
    
    // fill in struct PDC_obj_prop field in ret_value->obj_pt
    ret_value->obj_pt = PDC_CALLOC(struct PDC_obj_prop);
    if(ret_value->obj_pt)
        memcpy(ret_value->obj_pt, info->obj_pt, sizeof(struct PDC_obj_prop));
    else
        PGOTO_ERROR(NULL, "cannot allocate ret_value->obj_pt");
    ret_value->obj_pt->pdc = PDC_CALLOC(struct PDC_class);
    if(ret_value->obj_pt->pdc) {
        ret_value->obj_pt->pdc->local_id = info->obj_pt->pdc->local_id;
        if(info->obj_pt->pdc->name)
            ret_value->obj_pt->pdc->name = strdup(info->obj_pt->pdc->name);
        else
            ret_value->obj_pt->pdc->name = NULL;
    }
    else
        PGOTO_ERROR(NULL, "cannot allocate ret_value->obj_pt->pdc");

    ret_value->obj_pt->dims = malloc(ret_value->obj_pt->ndim*sizeof(uint64_t));
    if(ret_value->obj_pt->dims) {
        for(i=0; i<ret_value->obj_pt->ndim; i++) {
            ret_value->obj_pt->dims[i] = info->obj_pt->dims[i];
        }
    }
    else
        PGOTO_ERROR(NULL, "cannot allocate ret_value->obj_pt->dims");
    if(info->obj_pt->app_name)
        ret_value->obj_pt->app_name = strdup(info->obj_pt->app_name);
    else
        ret_value->obj_pt->app_name = NULL;
    if(info->obj_pt->data_loc)
        ret_value->obj_pt->data_loc = strdup(info->obj_pt->data_loc);
    else
        ret_value->obj_pt->data_loc = NULL;
    if(info->obj_pt->tags)
        ret_value->obj_pt->tags = strdup(info->obj_pt->tags);
    else
        ret_value->obj_pt->tags = NULL;
    
    ret_value->region_list_head = NULL;
    
done:
    FUNC_LEAVE(ret_value);
} 

perr_t PDC_free_obj_info(struct PDC_obj_info *obj)
{
    perr_t ret_value = TRUE;
    
    assert(obj);
    
    if(obj->name != NULL)
        free(obj->name);
    
    if(obj->metadata != NULL)
        free(obj->metadata);
    
    if(obj->cont != NULL){
        if(obj->cont->name != NULL)
            free(obj->cont->name);
        if(obj->cont->cont_pt->pdc->name != NULL)
            free(obj->cont->cont_pt->pdc->name);
        free(obj->cont->cont_pt->pdc);
        free(obj->cont->cont_pt);
        free(obj->cont);
    }
    
    if(obj->obj_pt != NULL) {
        if(obj->obj_pt->pdc != NULL) {
            if(obj->obj_pt->pdc->name != NULL)
                free(obj->obj_pt->pdc->name);
            free(obj->obj_pt->pdc);
        }
        if(obj->obj_pt->dims != NULL)
            free(obj->obj_pt->dims);
        if(obj->obj_pt->app_name != NULL)
            free(obj->obj_pt->app_name);
        if(obj->obj_pt->data_loc != NULL)
            free(obj->obj_pt->data_loc);
        if(obj->obj_pt->tags != NULL)
            free(obj->obj_pt->tags);
        free(obj->obj_pt);
    }
    
    if(obj->region_list_head != NULL)
        free(obj->region_list_head);

    PDC_FREE(struct PDC_obj_info, obj);
    
    FUNC_LEAVE(ret_value);
}

struct PDC_obj_info *PDCobj_get_info(const char *obj_name)
{
    struct PDC_obj_info *ret_value = NULL;
    pdcid_t obj_id;
    
    FUNC_ENTER(NULL);
    
    obj_id = PDC_find_byname(PDC_OBJ, obj_name);
    
    ret_value = PDC_obj_get_info(obj_id);
    
    FUNC_LEAVE(ret_value);
}

perr_t PDCobj_free_info(struct PDC_obj_info *obj)
{
    perr_t ret_value;
    
    ret_value = PDC_free_obj_info(obj);
    
    FUNC_LEAVE(ret_value);
}

perr_t PDCobj_release(pdcid_t obj_id)
{
    perr_t ret_value = SUCCEED;         /* Return value */
    struct PDC_id_info *info;
    
    FUNC_ENTER(NULL);
    
    info = PDC_find_id(obj_id);
    if(info == NULL)
        PGOTO_ERROR(FAIL, "cannot locate object ID");
    ((struct PDC_obj_info *)(info->obj_ptr))->obj_pt->buf = NULL;
    
done:
    FUNC_LEAVE(ret_value);
}

perr_t PDCreg_obtain_lock(pdcid_t obj_id, pdcid_t reg_id, PDC_access_t access_type, PDC_lock_mode_t lock_mode)
{
    perr_t ret_value = SUCCEED;         /* Return value */
    // pdcid_t meta_id;
    struct PDC_obj_info *object_info;
    struct PDC_region_info *region_info;
    PDC_var_type_t data_type;
    pbool_t obtained;
    
    FUNC_ENTER(NULL);
    
    object_info = PDC_obj_get_info(obj_id);
    data_type = object_info->obj_pt->type;
    region_info = PDCregion_get_info(reg_id);
    ret_value = PDC_Client_region_lock(object_info, region_info, access_type, lock_mode, data_type, &obtained);

    PDC_free_obj_info(object_info);
    
    FUNC_LEAVE(ret_value);
}

perr_t PDCreg_release_lock(pdcid_t obj_id, pdcid_t reg_id, PDC_access_t access_type)
{
    perr_t ret_value = SUCCEED;      
    pbool_t released;
    struct PDC_obj_info *object_info;
    struct PDC_region_info *region_info;
    PDC_var_type_t data_type;
 
    FUNC_ENTER(NULL);
    
    object_info = PDC_obj_get_info(obj_id);
    data_type = object_info->obj_pt->type;
    region_info = PDCregion_get_info(reg_id);
    
    ret_value = PDC_Client_region_release(object_info, region_info, access_type, data_type, &released);
 
    PDC_free_obj_info(object_info);
    
    FUNC_LEAVE(ret_value);
}
