/*
  Copyright 2007-2008
 	Paul Scherrer Institut, Villigen, Switzerland;
 	Benedikt Oswald;
 	Achim Gsell
 	All rights reserved.
 
  Authors
 	Achim Gsell
  
  Warning
	This code is under development.
 
 */

#ifndef __H5_H
#define __H5_H

h5_file_t *
H5OpenFile (
	const char * filename,
	h5_int32_t flag,
	MPI_Comm comm
	);

h5_err_t
H5CloseFile (
	h5_file_t * f
	);

h5_err_t
H5CheckFile (
	h5_file_t * f
	);

h5_err_t
H5SetStepNameFormat (
	h5_file_t *f,
	const char *name,
	const h5_int64_t width
	);

h5_err_t
H5GetStepNameFormat (
	h5_file_t *f,
	char *name,
	const h5_size_t l_name,
	int *width
	);

h5_err_t
H5SetStep (
	h5_file_t *f,
	const h5_id_t step
	);

h5_int64_t
H5GetStep (
	h5_file_t *f
	);

int
H5GetNumProcs (
	h5_file_t * const f
	);

h5_ssize_t
H5GetNumSteps (
	h5_file_t * const f
	);

h5_err_t
H5HasStep (
	h5_file_t * const f,
	h5_id_t step
	);

h5_err_t
H5StartTraverseSteps (
	h5_file_t *f
	);

h5_id_t
H5TraverseSteps (
	h5_file_t *f
	);

h5_err_t
H5EndTraverseSteps (
	h5_file_t *f
	);

h5_err_t
H5SetVerbosityLevel (
	const h5_id_t level
	);

h5_err_t
H5SetErrorHandler (
	h5_errorhandler_t handler
	);

h5_errorhandler_t
H5GetErrorHandler (
	void
	);

h5_err_t
H5ReportErrorhandler (
	const char *fmt,
	va_list ap
	);

h5_err_t
H5AbortErrorhandler (
	const char *fmt,
	va_list ap
	);

h5_err_t
H5GetErrno (
	h5_file_t * const f
	);

#ifdef PARALLEL_IO
h5_err_t
H5SetThrottle (
	h5_file_t* f,
	int factor
	);
#endif

#endif