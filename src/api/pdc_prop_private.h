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

#ifndef PDC_PROP_PRIVATE_H
#define PDC_PROP_PRIVATE_H

/***************************************/
/* Library-private Function Prototypes */
/***************************************/
/**
 * PDC container and object property initialization
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDC_prop_init();

/**
 * PDC container and object property finalize
 *
 * \return Non-negative on success/Negative on failure
 */
perr_t PDC_prop_end();

/**
 * Check if object property list is empty
 *
 * \return SUCCEED if empty/FAIL if not empty
 */
perr_t PDC_prop_obj_list_null();

/**
 * Check if container property list is empty
 *
 * \return SUCCEED if empty/FAIL if not empty
 */
perr_t PDC_prop_cont_list_null();

#endif /* PDC_PROP_PRIVATE_H */
