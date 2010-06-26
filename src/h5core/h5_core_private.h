#ifndef __H5_CORE_PRIVATE_H
#define __H5_CORE_PRIVATE_H

#define H5_DATANAME_LEN		128
#define H5_STEPNAME_LEN		128
#define H5_STEPNAME		"Step"
#define H5_STEPWIDTH		1
#define H5_BLOCKNAME		"Block"

#include "h5_types_private.h"

#include "h5_errorhandling_private.h"
#include "h5_fcmp_private.h"
#include "h5_hdf5_private.h"
#include "h5_hsearch_private.h"
#include "h5_mpi_private.h"
#include "h5_qsort_private.h"
#include "h5_readwrite_private.h"
#include "h5_syscall_private.h"

#include "h5b_types_private.h"
#include "h5u_types_private.h"

#include "h5b_errorhandling_private.h"

#include "h5t_core_private.h"

#include "h5u_errorhandling_private.h"
#include "h5u_types_private.h"

#define TRY( func )						\
	if ((int64_t)(ptrdiff_t)(func) <= (int64_t)H5_ERR)	\
		return H5_ERR;

#ifdef IPL_XT3
# define SEEK_END 2 
#endif

#endif
