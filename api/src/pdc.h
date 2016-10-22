#ifndef _pdc_H
#define _pdc_H

#include <stdint.h>
#include <stdbool.h>
#include "pdc_public.h"
#include "pdc_pdc.h"
#include "pdc_private.h"
#include "pdc_malloc.h"
#include "pdc_error.h"

typedef struct {
} PDC_prop;

typedef struct {
    pdcid_t pdc_id;
} PDC_STRUCT;


typedef enum {
    UNKNOWN = -1,
    MEMORY,
    FLASH,
    DISK,
    FILESYSTEM,
    TAPE
} PDC_loci;

typedef struct {
} obj_handle;

typedef enum {
    ROW_major,
    COL_major
} PDC_major_type;

typedef struct {
// public info
//    PDC_cont_info_t info;
// private stuff
} pdc_container_t;

typedef struct {
    char *name;
} PDC_obj_info_t;

typedef struct {
// public info
    PDC_obj_info_t info;
// private stuff
} pdc_object_t;

typedef struct {
} PDC_loci_info_t;

typedef struct {
    PDC_major_type type;
} PDC_transform;

typedef struct {
    uint64_t offset;
    uint64_t storage_size;
    PDC_loci locus;
} PDC_region;

/* Query match conditions */
typedef enum {
    PDC_Q_MATCH_EQUAL,        /* equal */
    PDC_Q_MATCH_NOT_EQUAL,    /* not equal */
    PDC_Q_MATCH_LESS_THAN,    /* less than */
    PDC_Q_MATCH_GREATER_THAN  /* greater than */
} PDC_query_op_t;

typedef enum {
    PDC_QUERY_OP_AND = 0,
    PDC_QUERY_OP_OR  = 1
} PDC_com_op_mode_t;

/* Query type */
typedef enum {
    PDC_Q_TYPE_DATA_ELEM,  /* selects data elements */
    PDC_Q_TYPE_ATTR_VALUE, /* selects attribute values */
    PDC_Q_TYPE_ATTR_NAME,  /* selects attributes */
    PDC_Q_TYPE_LINK_NAME,  /* selects objects */
    PDC_Q_TYPE_MISC        /* (for combine queries) selects misc objects */
} PDC_query_type_t;



/* Functions in PDC.c */

///////////////////
// PDC functions //
///////////////////

/* Initialize the PDC layer
 * Param PDC_property [IN]: A PDC_property struct  
 * Return: PDC id 
 */
pdcid_t PDC_init(PDC_prop property);

/* Close the PDC layer
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDC_close();

/* Create a type of PDC
 * Param PDC_STRUCT [IN]: A PDC_STRUCT struct
 * Return: PDC type id 
 */
pdcid_t PDCtype_create(PDC_STRUCT pdc_struct);

/* Insert fields in PDC_STRUCT 
 * Param type_id [IN]: Type of PDC, returned by PDCtype_create(struct PDC_STRUCT) 
 * Param name [IN]: Variable name to insert to PDC_STRUCT
 * Param offset [IN]: Offset of the variable in PDC_STRUCT
 * Param var_type [IN]: Variable type (enum type), choosing from PDC_var_type_t, i.e. PDC_int_t, PDC_float_t, etc 
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCtype_struct_field_insert(pdcid_t type_id, const char *name, uint64_t offset, PDC_var_type_t var_type);

/* get number of loci for a PDC
 * Param pdc_id [IN]: Id of the PDC
 * Param nloci [OUT]: Number of loci of the PDC residing at
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCget_loci_count(pdcid_t pdc_id, pdcid_t *nloci);

/* Get PDC info in the locus
 * Param pdc_id [IN]: Id of the PDC
 * Param n [IN]: Location of memory hierarchy of the PDC
 * Param info [OUT]: A PDC_loci_info_t struct
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCget_loci_info(pdcid_t pdc_id, pdcid_t n, PDC_loci_info_t *info);

//////////////////////
// object functions//
//////////////////////

/* Create an object  
 * Param cont_id [IN]: Id of the container
 * Param obj_name [IN]: Name of the object
 * Param obj_create_prop [IN]: Id of object property, returned by PDCprop_create(PDC_OBJ_CREATE)
 * Return: object id 
 */
pdcid_t PDCobj_create(pdcid_t cont_id, const char *obj_name, pdcid_t obj_create_prop); 

/* Set object lifetime 
 * Param obj_create_prop [IN]: Id of object property, returned by PDCprop_create(PDC_OBJ_CREATE)
 * Param obj_lifetime [IN]: Object lifetime (enum type), PDC_PERSIST or PDC_TRANSIENT
 * Return: Non-negative on success/Negative on failure
 */
//perr_t PDCprop_set_obj_lifetime(pdcid_t obj_create_prop, PDC_lifetime obj_lifetime);

/* Set object dimensions 
 * Param obj_create_prop [IN]: Id of object property, returned by PDCprop_create(PDC_OBJ_CREATE)
 * Param ndim [IN]: Number of dimensions
 * Param dims [IN]: Size of each dimension
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_dims(pdcid_t obj_create_prop, PDC_int_t ndim, uint64_t *dims);

/* Set object type 
 * Param obj_create_prop [IN]: Id of object property, returned by PDCprop_create(PDC_OBJ_CREATE)
 * Param type [IN]: Object variable type (enum type), choosing from PDC_var_type_t, i.e. PDC_int_t, PDC_float_t, etc
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_type(pdcid_t obj_create_prop, PDC_var_type_t type);

/* Set an object buffer 
 * Param obj_create_prop [IN]: Id of object property, returned by PDCprop_create(PDC_OBJ_CREATE)
 * Param buf [IN]: Start point of object storage
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_buf(pdcid_t obj_create_prop, void *buf);

/* Retrieve the buffer of an object 
 * Param obj_id [IN]: Id of the object
 * Param buf [IN]: Start point of object storage
 * Param region [IN]: A PDC_region struct
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCobj_buf_retrieve(pdcid_t obj_id, void **buf, PDC_region region);  

/* Open an object within a container
 * Param cont_id [IN]: Id of the container
 * Param obj_name [IN]: Name of the object
 * Return: Object id
 */
pdcid_t PDCobj_open(pdcid_t cont_id, const char *obj_name);

/* Iterate over objects in a container
 * Param cont_id [IN]: Container id, returned by PDCcont_open(pdcid_t pdc_id, const char *cont_name)
 * Return: Object handle 
 */
obj_handle PDCobj_iter_start(pdcid_t cont_id);

/* Check if object handle is pointing to NULL 
 * Param ohandle [IN]: A obj_handle struct, returned by PDCobj_iter_start(pdcid_t cont_id)
 * Return: 1 in case of success or 0 in case of failure
 */
pbool_t PDCobj_iter_null(obj_handle ohandle);

/* Iterate the next object 
 * Param ohandle [IN]: A obj_handle struct, returned by PDCobj_iter_start(pdcid_t cont_id)
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCobj_iter_next(obj_handle ohandle);

/* Get object information
 * Param ohandle [IN]: A obj_handle struct, returned by PDCobj_iter_start(pdcid_t cont_id)
 * Return: Pointer to a PDC_obj_info_t struct
 */
PDC_obj_info_t * PDCobj_iter_get_info(obj_handle ohandle);

/* Query on object 
 * Param pdc_id [IN]: Id of PDC
 * Param query_type [IN]: A PDC_query_type_t struct
 * Param query_op [IN]: A PDC_query_op_t struct
 * Return: Query id
 */
pdcid_t PDC_query_create(pdcid_t pdc_id, PDC_query_type_t query_type, PDC_query_op_t query_op, ...);
//pdcid_t PDC_query_obj(pdcid_t pdc_id, const char *varName, PDC_query_op_t query_op, const char *value);

/* Use result from  PDCquery_obj function
 * Param query1_id [IN]: Query id, returned by PDC_query_obj function
 * Param PDC_com_op_mode_t [IN]: Query Combination type (enum type), PDC_QUERY_OP_AND or PDC_QUERY_OP_OR
 * Param query2_id [IN]: Query id, returned by PDC_query_obj function
 * Return: Query id
 */
pdcid_t PDC_query_combine(pdcid_t query1_id, PDC_com_op_mode_t combine_op, pdcid_t query2_id);

/* View query result
 * Param view_id [IN]: Query id, returned by PDCquery_obj(pdcid_t pdc_id, PDC_match_op_t match_op, ...) 
 * Return: Object handle
 */
obj_handle PDCview_iter_start(pdcid_t view_id);

/* Map an application buffer to an object 
 * Param obj_id [IN]: Id of the object
 * Param buf [IN]: Start point of an application buffer
 * Param region [IN]: A PDC_region struct
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCobj_buf_map(pdcid_t obj_id, void *buf, PDC_region region);  

/* Map an object 
 * Param a [IN]: Id of the source object
 * Param xregion [IN]: A PDC_region struct, region of the source object
 * Param b [IN]: Id of the destination object
 * Param yregion [IN]: A PDC_region struct, region of the destination object
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCobj_map(pdcid_t a, PDC_region xregion, pdcid_t b, PDC_region yregion);

/* Diassociate memory object from PDC container objects 
 * Param obj_id [IN]: Id of the object
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCobj_unmap(pdcid_t obj_id);

/* Release memory buffers from one memory object 
 * Param obj_id [IN]: Id of the object
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCobj_release(pdcid_t obj_id);

/* Update object in the region 
 * Tell the PDC system that the region in memory is updated WRT to the container
 * Param obj_id [IN]: Id of the object
 * Param region [IN]: A PDC_region struct
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCobj_update_region(pdcid_t obj_id, PDC_region region);

/* Tell the PDC system that region in the memory is stale WRT to the container
 * Param obj_id [IN]: Id of the object
 * Param region [IN]: A PDC_region struct
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCobj_invalidate_region(pdcid_t obj_id, PDC_region region);

/* Object Syncranization. 
 * Param obj_id [IN]: Id of the object
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCobj_sync(pdcid_t obj_id);

/* Close an object 
 * Param obj_id [IN]: Id of the object
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCobj_close(pdcid_t obj_id);  


/* Object transform functions */

/* Built-in transform 
 * Param obj_create_prop [IN]: Id of object property, returned by PDCprop_create(PDC_OBJ_CREATE)
 * Param locus [IN]: Object locus setup (enum type), choosing from PDC_loci, i.e. MEMORY, FLASH, FILESYSTEM, TAPE, etc 
 * Param A [IN]: A PDC_transform struct 
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_loci_prop(pdcid_t obj_create_prop, PDC_loci locus, PDC_transform A);

/* User transform 
 * Param obj_create_prop [IN]: Id of object property, returned by PDCprop_create(PDC_OBJ_CREATE)
 * Param locus [IN]: Object source locus (enum type), choosing from PDC_loci, i.e. MEMORY, FLASH, FILESYSTEM, TAPE, etc 
 * Param A [IN]: A PDC_transform struct
 * Param dest_locus [IN]: Destination locus, choosing from PDC_loci, i.e. MEMORY, FLASH, FILESYSTEM, TAPE, etc
 * Return: Non-negative on success/Negative on failure
 */
perr_t PDCprop_set_obj_transform(pdcid_t obj_create_prop, PDC_loci pre_locus, PDC_transform A, PDC_loci dest_locus);


#endif /* end of _pdc_H */ 