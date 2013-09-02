/*
  Copyright (c) 2006-2013, The Regents of the University of California,
  through Lawrence Berkeley National Laboratory (subject to receipt of any
  required approvals from the U.S. Dept. of Energy) and the Paul Scherrer
  Institut (Switzerland).  All rights reserved.

  License: see file COPYING in top level of source distribution.
*/

#ifndef __H5CORE_H5_H
#define __H5CORE_H5_H

#include "h5core/h5_types.h"
#include <hdf5.h>

#define UNUSED_ARGUMENT(x) (void)x


#ifdef __cplusplus
extern "C" {
#endif

extern int h5_initialized;

h5_err_t
h5_initialize (void);

hid_t
h5_get_hdf5_file(
	const h5_file_t);

h5_prop_t
h5_create_prop (
        const h5_int64_t);

h5_err_t
h5_set_prop_file_throttle (
        h5_prop_t, const h5_int64_t);

h5_err_t
h5_set_prop_file_align (
        h5_prop_t, const h5_int64_t);

h5_err_t
h5_set_prop_file_mpio (
        h5_prop_t, MPI_Comm* const);

h5_err_t
h5_close_prop (
        h5_prop_t);

h5_file_t
h5_open_file (
	const char*, const h5_int32_t, 	MPI_Comm, const h5_size_t);

h5_file_t
h5_open_file2 (
	const char*, const h5_int32_t, h5_prop_t prop);

h5_err_t
h5_check_filehandle (
	const h5_file_t);

h5_err_t
h5_close_file (
	const h5_file_t);

h5_err_t
h5_close_hdf5 (
	void);

h5_err_t
h5_flush_step (
	const h5_file_t);

h5_err_t
h5_flush_file (
	const h5_file_t);

h5_err_t
h5_set_stepname_fmt (
	const h5_file_t, const char*, const int);

h5_err_t
h5_get_stepname_fmt (
	const h5_file_t, char* const, const int, int* const);

int
h5_get_num_procs (
	const h5_file_t);

h5_ssize_t
h5_get_num_steps (
	const h5_file_t);

h5_int64_t
h5_has_step (
	const h5_file_t, const h5_int64_t);

h5_int64_t
h5_get_step (
	const h5_file_t);

h5_err_t
h5_start_traverse_steps (
	const h5_file_t);

h5_err_t
h5_traverse_steps (
	const h5_file_t);

h5_err_t
h5_set_throttle (
        h5_file_t f,
        int factor
        );

#ifdef __cplusplus
}
#endif

#endif