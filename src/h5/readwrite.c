#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>	/* va_arg - System dependent ?! */
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <hdf5.h>

#include "H5PartTypes.h"
#include "H5BlockTypes.h"
#include "H5Part.h"
#include "H5Block.h"
#include "H5PartPrivate.h"
#include "H5BlockPrivate.h"
#include "H5PartErrors.h"
#include "H5BlockErrors.h"
#include "H5.h"


h5part_int64_t
H5_write_data (
	H5PartFile *f,		/*!< IN: Handle to open file */
	const char *name,	/*!< IN: Name to associate array with */
	const void *array,	/*!< IN: Array to commit to disk */
	const hid_t type,	/*!< IN: Type of data */
	const hid_t groupid,
	const hid_t shape,
	const hid_t memshape,
	const hid_t diskshape
	) {
	herr_t herr;
	hid_t dataset_id;

	H5_print_debug ( "Create a dataset[%s] mounted on the "
			      "timestep %lld",
			      name, (long long)f->timestep );

	dataset_id = H5Dcreate ( 
		groupid,
		name,
		type,
		shape,
		H5P_DEFAULT );
	if ( dataset_id < 0 )
		return HANDLE_H5D_CREATE_ERR ( name, f->timestep );

	herr = H5Dwrite (
		dataset_id,
		type,
		memshape,
		diskshape,
		f->xfer_prop,
		array );

	if ( herr < 0 ) return HANDLE_H5D_WRITE_ERR ( name, f->timestep );

	herr = H5Dclose ( dataset_id );
	if ( herr < 0 ) return HANDLE_H5D_CLOSE_ERR;

	f->empty = 0;

	return H5PART_SUCCESS;
}


/*!
  \ingroup h5part_kernel

  Iterator for \c H5Giterate().
*/
herr_t
H5_iteration_operator (
	hid_t group_id,		/*!< [in]  group id */
	const char *member_name,/*!< [in]  group name */
	void *operator_data	/*!< [in,out] data passed to the iterator */
	) {

	struct _iter_op_data *data = (struct _iter_op_data*)operator_data;
	herr_t herr;
	H5G_stat_t objinfo;

	if ( data->type != H5G_UNKNOWN ) {
		herr = H5Gget_objinfo ( group_id, member_name, 1, &objinfo );
		if ( herr < 0 ) return (herr_t)HANDLE_H5G_GET_OBJINFO_ERR ( member_name );

		if ( objinfo.type != data->type )
			return 0;/* don't count, continue iteration */
	}

	if ( data->name && (data->stop_idx == data->count) ) {
		memset ( data->name, 0, data->len );
		strncpy ( data->name, member_name, data->len-1 );
		
		return 1;	/* stop iteration */
	}
	/*
	  count only if pattern is NULL or member name matches
	*/
	if ( !data->pattern ||
	     (strncmp (member_name, data->pattern, strlen(data->pattern)) == 0)
	      ) {
		data->count++;
	}
	return 0;		/* continue iteration */
}

/*!
  \ingroup h5part_kernel

  Iterator for \c H5Giterate().
*/
h5part_int64_t
H5_get_num_objects (
	hid_t group_id,
	const char *group_name,
	const hid_t type
	) {

	return H5_get_num_objects_matching_pattern (
		group_id,
		group_name,
		type,
		NULL );
}

/*!
  \ingroup h5part_kernel

  Iterator for \c H5Giterate().
*/
h5part_int64_t
H5_get_num_objects_matching_pattern (
	hid_t group_id,
	const char *group_name,
	const hid_t type,
	char * const pattern
	) {

	h5part_int64_t herr;
	int idx = 0;
	struct _iter_op_data data;

	memset ( &data, 0, sizeof ( data ) );
	data.type = type;
	data.pattern = pattern;

	herr = H5Giterate ( group_id, group_name, &idx,
			    H5_iteration_operator, &data );
	if ( herr < 0 ) return herr;
	
	return data.count;
}

/*!
  \ingroup h5part_kernel

  Iterator for \c H5Giterate().
*/
h5part_int64_t
H5_get_object_name (
	hid_t group_id,
	const char *group_name,
	const hid_t type,
	const h5part_int64_t idx,
	char *obj_name,
	const h5part_int64_t len_obj_name
	) {

	herr_t herr;
	struct _iter_op_data data;
	int iterator_idx = 0;

	memset ( &data, 0, sizeof ( data ) );
	data.stop_idx = (hid_t)idx;
	data.type = type;
	data.name = obj_name;
	data.len = (size_t)len_obj_name;

	herr = H5Giterate ( group_id, group_name, &iterator_idx,
			    H5_iteration_operator,
			    &data );
	if ( herr < 0 ) return (h5part_int64_t)herr;

	if ( herr == 0 ) HANDLE_H5PART_NOENTRY_ERR( group_name,
						    type, idx );

	return H5PART_SUCCESS;
}

h5part_int64_t
H5_set_step (
	H5PartFile *f,			/*!< [in]  Handle to open file */
	const h5part_int64_t step	/*!< [in]  Time-step to set. */
	) {

	sprintf (
		f->index_name,
		"%s#%0*lld",
		f->groupname_step, f->stepno_width, (long long) step );
	herr_t herr = H5Gget_objinfo( f->file, f->index_name, 1, NULL );
	if ( (f->mode != H5PART_READ) && ( herr >= 0 ) ) {
		return HANDLE_H5PART_STEP_EXISTS_ERR ( step );
	}

	if ( f->timegroup >= 0 ) {
		herr = H5Gclose ( f->timegroup );
		if ( herr < 0 ) return HANDLE_H5G_CLOSE_ERR;
	}
	f->timegroup = -1;
	f->timestep = step;

	if( f->mode == H5PART_READ ) {
		H5_print_info (
			"Proc[%d]: Set step to #%lld for file %lld",
			f->myproc,
			(long long)step,
			(long long)(size_t) f );

		f->timegroup = H5Gopen ( f->file, f->index_name ); 
		if ( f->timegroup < 0 ) return HANDLE_H5G_OPEN_ERR( f->index_name );
	}
	else {
		H5_print_debug (
			"Proc[%d]: Create step #%lld for file %lld", 
			f->myproc,
			(long long)step,
			(long long)(size_t) f );

		f->timegroup = H5Gcreate ( f->file, f->index_name, 0 );
		if ( f->timegroup < 0 )
			return HANDLE_H5G_CREATE_ERR ( f->index_name );
	}

	return H5PART_SUCCESS;
}

/*!
   Normalize HDF5 type
*/
hid_t
H5_normalize_h5_type (
	hid_t type
	) {
	H5T_class_t tclass = H5Tget_class ( type );
	int size = H5Tget_size ( type );

	switch ( tclass ){
	case H5T_INTEGER:
		if ( size==8 ) {
			return H5T_NATIVE_INT64;
		}
		else if ( size==1 ) {
			return H5T_NATIVE_CHAR;
		}
		break;
	case H5T_FLOAT:
		return H5T_NATIVE_DOUBLE;
	default:
		; /* NOP */
	}
	H5_print_warn ( "Unknown type %d", (int)type );

	return -1;
}


h5part_int64_t
H5_get_dataset_type(
	hid_t group_id,
	const char *dataset_name
	) {
	hid_t dataset_id = H5Dopen ( group_id, dataset_name );
	if ( dataset_id < 0 ) HANDLE_H5D_OPEN_ERR ( dataset_name );

	hid_t hdf5_type = H5Dget_type ( dataset_id );
	if ( hdf5_type < 0 ) HANDLE_H5D_GET_TYPE_ERR;

	h5part_int64_t type = (h5part_int64_t) H5_normalize_h5_type ( hdf5_type );

	herr_t herr = H5Tclose(hdf5_type);
	if ( herr < 0 ) HANDLE_H5T_CLOSE_ERR;

	herr = H5Dclose(dataset_id);
	if ( herr < 0 ) HANDLE_H5D_CLOSE_ERR;

	return type;
}

h5part_int64_t
H5_has_index (
	H5PartFile *f,		/*!< [in]  Handle to open file */
	h5part_int64_t step	/*!< [in]  Step number to query */
	) {
	char name[128];
	sprintf ( name,
		  "%s#%0*lld",
		  f->groupname_step, f->stepno_width, (long long) step );
	return ( H5Gget_objinfo( f->file, name, 1, NULL ) >= 0 );
}