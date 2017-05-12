#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pdc.h"

int main() {
    struct PDC_prop p;
    // create a pdc
    pdcid_t pdc = PDC_init(p);
    printf("create a new pdc, pdc id is: %lld\n", pdc);

    // create a container property
    pdcid_t cont_prop = PDCprop_create(PDC_CONT_CREATE, pdc);
    if(cont_prop > 0)
        printf("Create a container property, id is %lld\n", cont_prop);
    else
        printf("Fail to create container property @ line  %d!\n", __LINE__);

    // create a container
    pdcid_t cont = PDCcont_create(pdc, "c1", cont_prop);
    if(cont > 0)
        printf("Create a container, id is %lld\n", cont);
    else
        printf("Fail to create container @ line  %d!\n", __LINE__);
    
    // create an object property
    pdcid_t obj_prop = PDCprop_create(PDC_OBJ_CREATE, pdc);
    if(obj_prop > 0)
        printf("Create an object property, id is %lld\n", obj_prop);
    else
        printf("Fail to create object property @ line  %d!\n", __LINE__);
    
    // create first object
    pdcid_t obj1 = PDCobj_create(pdc, cont, "o1", obj_prop);
    if(obj1 > 0)
        printf("Create an object, id is %lld\n", obj1);
    else
        printf("Fail to create object @ line  %d!\n", __LINE__);
    
    // create second object
    pdcid_t obj2 = PDCobj_create(pdc, cont, "o2", obj_prop);
    if(obj2 > 0)
        printf("Create an object, id is %lld\n", obj2);
    else
        printf("Fail to create object @ line  %d!\n", __LINE__);
    
    // open first object twice
    pdcid_t open11 = PDCobj_open(cont, "o1", pdc);
    if(open11 < 0)
        printf("Fail to open object o1\n");
    else
        printf("Open object o1, id is %lld\n", open11);
    
    pdcid_t open12 = PDCobj_open(cont, "o1", pdc);
    if(open12 < 0)
        printf("Fail to open object o1\n");
    else
        printf("Open object o1, id is %lld\n", open12);
    
    // open second object once
    pdcid_t open21 = PDCobj_open(cont, "o2", pdc);
    if(open21 < 0)
        printf("Fail to open object o2\n");
    else
        printf("Open object o1, id is %lld\n", open21);
    
    // close first object
    if(PDCobj_close(obj1, pdc) < 0)
        printf("fail to close object %lld\n", obj1);
    else
        printf("successfully close object # %lld\n", obj1);
    
    // close second object
    if(PDCobj_close(obj2, pdc) < 0)
        printf("fail to close object %lld\n", obj2);
    else
        printf("successfully close object # %lld\n", obj2);
       
    // close a container
    if(PDCcont_close(cont, pdc) < 0)
        printf("fail to close container %lld\n", cont);
    else
        printf("successfully close container # %lld\n", cont);
    
    // close a object property
    if(PDCprop_close(obj_prop, pdc) < 0)
        printf("Fail to close property @ line %d\n", __LINE__);
    else
        printf("successfully close object property # %lld\n", obj_prop);

    // close a container property
    if(PDCprop_close(cont_prop, pdc) < 0)
        printf("Fail to close property @ line %d\n", __LINE__);
    else
        printf("successfully close container property # %lld\n", cont_prop);

    // close pdc
    if(PDC_close(pdc) < 0)
       printf("fail to close PDC\n");
    else
       printf("PDC is closed\n");
}
