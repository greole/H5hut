/*
  Copyright (c) 2006-2015, The Regents of the University of California,
  through Lawrence Berkeley National Laboratory (subject to receipt of any
  required approvals from the U.S. Dept. of Energy) and the Paul Scherrer
  Institut (Switzerland).  All rights reserved.

  License: see file COPYING in top level of source distribution.
*/

#ifndef __H5CORE_H5B_IO_H
#define __H5CORE_H5B_IO_H

#include "h5core/h5_types.h"

#ifdef __cplusplus
extern "C" {
#endif

h5_err_t
h5b_write_scalar_data (
	const h5_file_t,
	const char*, const void*, const hid_t);

h5_err_t
h5b_write_vector3d_data (
	const h5_file_t,
	const char*, const void*, const void*, const void*, const hid_t);

h5_err_t
h5b_read_scalar_data (
	const h5_file_t,
	const char*, void*, const hid_t);

h5_err_t
h5b_read_vector3d_data (
	const h5_file_t,
	const char*, void*, void*, void*, const hid_t);

#ifdef __cplusplus
}
#endif

#endif