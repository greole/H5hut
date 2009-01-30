#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>	/* va_arg - System dependent ?! */
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <hdf5.h>

#include "h5_core.h"
#include "h5_core_private.h"

/*!
  \ingroup h5_core
  \defgroup h5_core_filehandling
*/

/*!
  \ingroup h5_core_filehandling

  Check whether \c f points to a valid file handle.

  \return	H5_SUCCESS or error code
*/
h5_err_t
h5_check_filehandle (
	h5_file_t * const f	/*!< filehandle  to check validity of */
	) {

	if ( f == NULL )
		return HANDLE_H5_BADFD_ERR( f );
	if ( f->file == 0 )
		return HANDLE_H5_BADFD_ERR( f );

	return H5_SUCCESS;
}


/*!
  Initialize H5Part
*/
static herr_t
_h5_error_handler (
	hid_t estack_id,
	void* __f
	) {
	if ( h5_get_debuglevel () >= 5 ) {
		H5Eprint (estack_id, stderr);
	}
	return 0;
}

static h5_int64_t
_init ( void ) {
	static int __init = 0;

	herr_t r5;
	if ( ! __init ) {
		r5 = H5Eset_auto ( H5E_DEFAULT, _h5_error_handler, NULL );
		if ( r5 < 0 ) return H5_ERR_INIT;
	}
	__init = 1;
	return H5_SUCCESS;
}

/*!
  \ingroup h5_private

  \internal

  Initialize unstructured data internal data structure.

  \return	H5_SUCCESS or error code
*/
static h5_int64_t
_h5u_open_file (
	h5_file_t *f			/*!< IN: file handle */
	) {
 	f->shape = 0;
	f->diskshape = H5S_ALL;
	f->memshape = H5S_ALL;
	f->viewstart = -1;
	f->viewend = -1;
	f->pnparticles =
		(h5_int64_t*) malloc (f->nprocs * sizeof (h5_int64_t));
	if (f->pnparticles == NULL) {
		return HANDLE_H5_NOMEM_ERR( f );
	}
	return H5_SUCCESS;
}

/*!
  \ingroup h5_private

  \internal

  Initialize H5Block internal structure.

  \return	H5_SUCCESS or error code
*/
static h5_int64_t
_h5b_open_file (
	h5_file_t *f			/*!< IN: file handle */
	) {
	struct h5b_fdata *b; 

	if ( (f == 0) || (f->file == 0) ) return HANDLE_H5_BADFD_ERR( f );
	if ( f->b ) return H5_SUCCESS;

	f->b = (struct h5b_fdata*) malloc( sizeof (*f->b) );
	if ( f->b == NULL ) {
		return HANDLE_H5_NOMEM_ERR( f );
	}
	b = f->b;
	memset ( b, 0, sizeof (*b) );
	b->user_layout = (struct h5b_partition*) malloc (
		f->nprocs * sizeof (b->user_layout[0]) );
	if ( b->user_layout == NULL ) {
		return HANDLE_H5_NOMEM_ERR( f );
	}
	b->write_layout = (struct h5b_partition*) malloc (
		f->nprocs * sizeof (b->write_layout[0]) );
	if ( b->write_layout == NULL ) {
		return HANDLE_H5_NOMEM_ERR( f );
	}
	b->step_idx = -1;
	b->blockgroup = -1;
	b->shape = -1;
	b->diskshape = -1;
	b->memshape = -1;
	b->field_group_id = -1;
	b->have_layout = 0;

	return H5_SUCCESS;
}

/*!
  \ingroup h5_core_filehandling
  
  Open file with name \c filename. This function is available in the paralell
  and serial version. In the serial case \c comm may have any value.

  \return File handle.
  \return NULL on error.
*/
 
h5_file_t *
h5_open_file (
	const char *filename,	/*!< The name of the data file to open. */
	h5_int32_t flags,	/*!< The access mode for the file. */
	MPI_Comm comm,		/*!< MPI communicator */
	const char *funcname	/*!< calling function name */
	) {


	h5_file_t *f = NULL;

	f = (h5_file_t*) malloc( sizeof (h5_file_t) );
	if( f == NULL ) {
		fprintf(
			stderr,
			"E: %s: Can't open file %s. Not enough memory!",
			funcname,
			filename );
		return NULL;
	}
	memset (f, 0, sizeof (h5_file_t));

	if ( _init() < 0 ) {
		HANDLE_H5_INIT_ERR( f );
		return NULL;
	}

	f->__funcname = funcname;
	h5_info ( f, "Opening file %s.", filename );


	f->prefix_step_name = strdup ( H5PART_GROUPNAME_STEP );
	if( f->prefix_step_name == NULL ) {
		HANDLE_H5_NOMEM_ERR( f );
		goto error_cleanup;
	}
	f->width_step_idx = 0;

	f->xfer_prop = f->create_prop = f->access_prop = H5P_DEFAULT;

	f->comm = 0;		/* init values for serial case */
	f->nprocs = 1;
	f->myproc = 0;

#ifdef PARALLEL_IO
		f->comm = comm;
		if (MPI_Comm_size (comm, &f->nprocs) != MPI_SUCCESS) {
			HANDLE_MPI_COMM_SIZE_ERR;
			goto error_cleanup;
		}
		if (MPI_Comm_rank (comm, &f->myproc) != MPI_SUCCESS) {
			HANDLE_MPI_COMM_RANK_ERR;
			goto error_cleanup;
		}


		/* for the SP2... perhaps different for linux */
		MPI_Info info = MPI_INFO_NULL;

		/* ks: IBM_large_block_io */
		MPI_Info_create(&info);
		MPI_Info_set(info, "IBM_largeblock_io", "true" );
		if (H5Pset_fapl_mpio (f->access_prop, comm, info) < 0) {
			HANDLE_H5P_SET_FAPL_MPIO_ERR;
			goto error_cleanup;
		}
		MPI_Info_free(&info);

		f->access_prop = H5Pcreate (H5P_FILE_ACCESS);
		if (f->access_prop < 0) {
			HANDLE_H5P_CREATE_ERR;
			goto error_cleanup;
		}

		/* f->create_prop = H5Pcreate(H5P_FILE_CREATE); */
		f->create_prop = H5P_DEFAULT;

		/* xfer_prop:  also used for parallel I/O, during actual writes
		   rather than the access_prop which is for file creation. */
		f->xfer_prop = H5Pcreate (H5P_DATASET_XFER);
		if (f->xfer_prop < 0) {
			HANDLE_H5P_CREATE_ERR;
			goto error_cleanup;
		}

#ifdef COLLECTIVE_IO
		if (H5Pset_dxpl_mpio (f->xfer_prop,H5FD_MPIO_COLLECTIVE) < 0) {
			HANDLE_H5P_SET_DXPL_MPIO_ERR;
			goto error_cleanup;
		}
#endif /* COLLECTIVE_IO */

#endif /* PARALLEL_IO */

	if ( flags == H5_O_RDONLY ) {
		f->file = H5Fopen (filename, H5F_ACC_RDONLY, f->access_prop);
	}
	else if ( flags == H5_O_WRONLY ){
		f->file = H5Fcreate (filename, H5F_ACC_TRUNC, f->create_prop,
				     f->access_prop);
		f->empty = 1;
	}
	else if ( flags == H5_O_APPEND || flags == H5_O_RDWR ) {
		int fd = open (filename, O_RDONLY, 0);
		if ( (fd == -1) && (errno == ENOENT) ) {
			f->file = H5Fcreate(filename, H5F_ACC_TRUNC,
					    f->create_prop, f->access_prop);
			f->empty = 1;
		}
		else if (fd != -1) {
			close (fd);
			f->file = H5Fopen (filename, H5F_ACC_RDWR,
					   f->access_prop);
		}
	}
	else {
		HANDLE_H5_FILE_ACCESS_TYPE_ERR ( f, flags );
		goto error_cleanup;
	}

	if (f->file < 0) {
		HANDLE_H5F_OPEN_ERR ( f, filename, flags );
		goto error_cleanup;
	}
	f->root_gid = H5Gopen( f->file, "/", H5P_DEFAULT );
	if ( f->root_gid < 0 ) {
		HANDLE_H5G_OPEN_ERR ( f, "", "" );
		goto error_cleanup;
	}
	f->mode = flags;
	f->step_gid = -1;

	sprintf (
		f->step_name,
		"%s#%0*lld",
		f->prefix_step_name, f->width_step_idx, (long long) f->step_idx );

	if ( _h5u_open_file ( f ) < 0 ) {
		goto error_cleanup;
	}

	if ( _h5b_open_file ( f ) < 0 ) {
		goto error_cleanup;
	}

	if ( _h5t_open_file ( f ) < 0 ) {
		goto error_cleanup;
	}

	return f;

 error_cleanup:
	if (f != NULL ) {
		if (f->prefix_step_name) {
			free (f->prefix_step_name);
		}
		if (f->pnparticles != NULL) {
			free (f->pnparticles);
		}
		free (f);
	}
	return NULL;
}

/*!
  \ingroup h5_private

  \internal

  De-initialize H5Block internal structure.  Open HDF5 objects are 
  closed and allocated memory freed.

  \return	H5_SUCCESS or error code
*/
static h5_int64_t
_h5u_close_file (
	h5_file_t *f		/*!< file handle */
	) {
	herr_t herr;
	f->__errno = H5_SUCCESS;
	if( f->shape > 0 ) {
		herr = H5Sclose( f->shape );
		if ( herr < 0 ) HANDLE_H5S_CLOSE_ERR( f );
		f->shape = 0;
	}
	if( f->diskshape != H5S_ALL ) {
		herr = H5Sclose( f->diskshape );
		if ( herr < 0 ) HANDLE_H5S_CLOSE_ERR( f );
		f->diskshape = 0;
	}
	if( f->memshape != H5S_ALL ) {
		herr = H5Sclose( f->memshape );
		if ( herr < 0 ) HANDLE_H5S_CLOSE_ERR( f );
		f->memshape = 0;
	}
	if( f->pnparticles ) {
		free( f->pnparticles );
	}
	return f->__errno;
}

/*!
  \ingroup h5block_private

  \internal

  De-initialize H5Block internal structure.  Open HDF5 objects are 
  closed and allocated memory freed.

  \return	H5_SUCCESS or error code
*/
static h5_int64_t
_h5b_close_file (
	h5_file_t *f		/*!< IN: file handle */
	) {

	herr_t herr;
	struct h5b_fdata *b = f->b;

	if ( b->blockgroup >= 0 ) {
		herr = H5Gclose ( b->blockgroup );
		if ( herr < 0 ) return HANDLE_H5G_CLOSE_ERR( f );
		b->blockgroup = -1;
	}
	if ( b->shape >= 0 ) {
		herr = H5Sclose ( b->shape );
		if ( herr < 0 ) return HANDLE_H5S_CLOSE_ERR( f );
		b->shape = -1;
	}
	if ( b->diskshape >= 0 ) {
		herr = H5Sclose ( b->diskshape );
		if ( herr < 0 ) return HANDLE_H5S_CLOSE_ERR( f );
		b->diskshape = -1;
	}
	if ( b->memshape >= 0 ) {
		herr = H5Sclose ( b->memshape );
		if ( herr < 0 ) return HANDLE_H5S_CLOSE_ERR( f );
		b->memshape = -1;
	}
	free ( f->b );
	f->b = NULL;

	return H5_SUCCESS;
}

/*!
  \ingroup h5_core_filehandling

  The h5_close_file() call writes all buffered data to disk, releases 
  all previously allocated memory and terminates access to the associated
  HDF5 file.

  \return	H5_SUCCESS or error code
*/
h5_int64_t
h5_close_file (
	h5_file_t *f		/*!< file handle */
	) {
	herr_t r = 0;
	f->__errno = H5_SUCCESS;

	CHECK_FILEHANDLE ( f );

	_h5_close_step ( f );

	_h5u_close_file ( f );
	_h5b_close_file ( f );
	_h5t_close_file ( f );

	if( f->step_gid >= 0 ) {
		r = H5Gclose( f->step_gid );
		if ( r < 0 ) HANDLE_H5G_CLOSE_ERR( f );
		f->step_gid = -1;
	}
	if( f->xfer_prop != H5P_DEFAULT ) {
		r = H5Pclose( f->xfer_prop );
		if ( r < 0 ) HANDLE_H5P_CLOSE_ERR ( f, "f->xfer_prop" );
		f->xfer_prop = H5P_DEFAULT;
	}
	if( f->access_prop != H5P_DEFAULT ) {
		r = H5Pclose( f->access_prop );
		if ( r < 0 ) HANDLE_H5P_CLOSE_ERR ( f, "f->access_prop" );
		f->access_prop = H5P_DEFAULT;
	}  
	if( f->create_prop != H5P_DEFAULT ) {
		r = H5Pclose( f->create_prop );
		if ( r < 0 ) HANDLE_H5P_CLOSE_ERR ( f, "f->create_prop" );
		f->create_prop = H5P_DEFAULT;
	}
	if ( f->root_gid >= 0 ) {
		r = H5Gclose ( f->root_gid );
		if ( r < 0 ) HANDLE_H5G_CLOSE_ERR( f );
		f->root_gid = 0;
	}
	if ( f->file ) {
		r = H5Fclose( f->file );
		if ( r < 0 ) HANDLE_H5F_CLOSE_ERR( f );
		f->file = 0;
	}
	if (f->prefix_step_name) {
		free (f->prefix_step_name);
	}
	free( f );

	return f->__errno;
}

/*!
  \ingroup h5_core_filehandling

  Define format of the step names.

  Example: ==H5FedDefineStepNameFormat( f, "Step", 6 )== defines step names 
  like ==Step#000042==.

  \return \c H5_SUCCESS or error code
*/
h5_int64_t
h5_set_stepname_fmt (
	h5_file_t *f,
	const char *name,
	const h5_int64_t width
	) {
	f->prefix_step_name = strdup ( name );
	if( f->prefix_step_name == NULL ) {
		return HANDLE_H5_NOMEM_ERR( f );
	}
	f->width_step_idx = (int)width;
	
	return H5_SUCCESS;
}

/*!
  \ingroup h5_core_filehandling

  Get format of the step names.

  \return \c H5_SUCCESS or error code
*/
h5_err_t
h5_get_stepname_fmt (
	h5_file_t *f,			/*!< Handle to file		*/
	char *name,			/*!< OUT: Prefix		*/
	const h5_size_t l_name,		/*!< length of buffer name	*/
	h5_size_t *width		/*!< OUT: Width of the number	*/
	) {
	return -1;
}

/*!
  \ingroup h5_core_filehandling

  Get current step number.

  \return Current step number or error code
*/
h5_id_t
h5_get_step (
	h5_file_t * f			/*!< file handle		*/
	) {
	return -1;
}
	
/*!
  \ingroup h5_core_filehandling

  Check whether step with number \c stepno exists.

  \return True (value != 0) if step with \c stepno exists.
  \return False (0) otherwise
*/
h5_err_t
h5_has_step (
	h5_file_t * f,			/*!< file handle		*/
	h5_id_t stepno			/*!< step number to check	*/
	) {
	char name[128];
        sprintf ( name, "%s#%0*ld",
		  f->prefix_step_name, f->width_step_idx, (long) stepno );
	return ( H5Gget_info_by_name( f->file, name, NULL, H5P_DEFAULT ) >= 0 );
}

/*!
  \ingroup h5_core_filehandling

  Start traversing steps.

  \return \c H5_SUCCESS or error code 
*/
h5_err_t
h5_start_traverse_steps (
	h5_file_t * f			/*!< file handle		*/
	) {
	return -1;
}

/*!
  \ingroup h5_core_filehandling

  Go to next step.

  \return \c H5_SUCCESS or error code 
*/
h5_err_t
h5_traverse_steps (
	h5_file_t * f			/*!< file handle		*/
	) {
	return -1;
}