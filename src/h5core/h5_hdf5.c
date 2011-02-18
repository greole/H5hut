#include <stdlib.h>
#include <string.h>
#include <hdf5.h>

#include "h5core/h5_core.h"
#include "h5_core_private.h"

/****** G r o u p ************************************************************/

hid_t
hdf5_open_group (
	const hid_t loc_id,
	const char* const group_name
	) {
	HDF5_WRAPPER_ENTER3 (hid_t,
			    "loc_id=%d (%s), group_name=\"%s\"", 
			    loc_id,
			    h5_get_objname (loc_id),
			    group_name);
	hid_t group_id = H5Gopen (loc_id, group_name, H5P_DEFAULT);
	if (group_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot open group \"%s/%s\".",
				h5_get_objname (loc_id),
				group_name));
	HDF5_WRAPPER_RETURN (group_id);
}

hid_t
hdf5_create_group (
	const hid_t loc_id,
	const char* const group_name
	) {
	HDF5_WRAPPER_ENTER3 (hid_t,
			    "loc_id=%d (%s), group_name=\"%s\"", 
			    loc_id,
			    h5_get_objname (loc_id),
			    group_name);
	hid_t group_id = H5Gcreate (
		loc_id, group_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	if (group_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot create group \"%s/%s\".",
				h5_get_objname (loc_id),
				group_name));
	HDF5_WRAPPER_RETURN (group_id);
}

/*!
  Open HDF5 group. If group doesn't exist create it.

  \param[in]	f		file handle
  \param[in]	loc_id		location id
  \param[in]	group_name	name of group to open
*/
hid_t
h5priv_open_group (
	h5_file_t* const f,
	const hid_t loc_id,
	const char* const group_name
	) {
	hid_t group_id;

	H5_PRIV_API_ENTER3 (hid_t,
			    "loc_id=%d (%s), group_name=\"%s\"", 
			    loc_id,
			    h5_get_objname (loc_id),
			    group_name);

	/*
	  check access modes:
			Open	Create
	  H5_O_RDWR	x	x
	  H5_O_RDONLY	x	-
	  H5_O_WRONLY	x	x  (overwrite/append data to existing dataset)
	  H5_O_APPEND	x	x  (append datasets to an existing group)
	*/

        h5_err_t exists;
        TRY (exists = hdf5_link_exists(loc_id, group_name));
	if (exists > 0) {
		TRY (group_id = hdf5_open_group (loc_id, group_name));
	} else {
		CHECK_WRITABLE_MODE (f);
		TRY (group_id = hdf5_create_group (loc_id, group_name));
	}

	H5_PRIV_API_RETURN (group_id);
}

/*!
  Close group.

  \param[in]	f		file handle
  \param[in]	group_id        id of group to close
*/
h5_err_t
hdf5_close_group (
	const hid_t group_id
	) {
	HDF5_WRAPPER_ENTER2 (h5_err_t,
			     "group_id=%d (%s)",
			     group_id,
			     h5_get_objname (group_id));

	if (group_id == 0 || group_id == -1)
		HDF5_WRAPPER_LEAVE (H5_SUCCESS);
	if (H5Gclose (group_id) < 0 ) {
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot terminate access to group \"%s\").",
				h5_get_objname (group_id)));
	}
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_ssize_t
hdf5_get_num_objs_in_group (
	const hid_t group_id
	) {
	HDF5_WRAPPER_ENTER2 (h5_ssize_t,
			     "group_id=%d (%s)",
			     group_id,
			     h5_get_objname (group_id));
	H5G_info_t group_info;
	if (H5Gget_info (group_id, &group_info) < 0) {
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get number of objects in group \"%s\".",
				h5_get_objname(group_id)));
	}
	HDF5_WRAPPER_RETURN ((h5_ssize_t)group_info.nlinks);
}


/*
  Get name of object given by index \c idx in group \c loc_id. If name is \c NULL,
  return size of name.
*/
h5_ssize_t
hdf5_get_objname_by_idx (
	hid_t loc_id,
	hsize_t idx,
	char *name,
	size_t size
	) {
	HDF5_WRAPPER_ENTER3 (h5_ssize_t,
			     "loc_id=%d (%s), idx=%lld",
			     loc_id,
			     h5_get_objname (loc_id),
			     (long long)idx);

	if (name == NULL) {
		size = 0;
	}
	ssize_t len = H5Lget_name_by_idx (loc_id, ".",
				  H5_INDEX_NAME, H5_ITER_INC,
				  idx,
				  name, size,
				  H5P_DEFAULT);
	if (len < 0) 
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get name of object %llu in group \"%s\".",
				(unsigned long long)idx,
				h5_get_objname (loc_id)));
	HDF5_WRAPPER_RETURN (len);
}

/****** D a t a s e t ********************************************************/
/*!
  Open dataset. H5Dopen wrapper.

  \param[in]	f		file handle
  \param[in]	loc_id		location id
  \param[in]	dataset_name	name of dataset to open
 */
hid_t
hdf5_open_dataset (
	const hid_t loc_id,
	const char* const dataset_name
	) {
	HDF5_WRAPPER_ENTER3 (hid_t,
			     "loc_id=%d (%s), dataset_name=\"%s\"",
			     loc_id,
			     h5_get_objname (loc_id),
			     dataset_name);
	hid_t dataset_id = H5Dopen ( 
		loc_id,
		dataset_name,
		H5P_DEFAULT);
	if (dataset_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot open dataset \"%s/%s\".",
				h5_get_objname (loc_id),
				dataset_name));
	HDF5_WRAPPER_RETURN (dataset_id);
}

/*!
  Create new dataset

  \param[in]	f		file handle
  \param[in]	loc_id		id of group or file
  \param[in]	dataset_name	name of dataset
  \param[in]	type_id		type used in dataset
  \param[in]	dataspace_id	dataspace of dataset
  \param[in]	create_prop	property list for dataset creation

 */
hid_t
hdf5_create_dataset (
	hid_t loc_id,
	const char* dataset_name,
	const hid_t type_id,
	const hid_t dataspace_id,
	const hid_t create_proplist
	) {
	HDF5_WRAPPER_ENTER3 (hid_t,
			    "loc_id=%d (%s), dataset_name=\"%s\"", 
			    loc_id,
			    h5_get_objname (loc_id),
			    dataset_name);
	hid_t dataset_id = H5Dcreate ( 
		loc_id,
		dataset_name,
		type_id,
		dataspace_id,
		H5P_DEFAULT,
		create_proplist,
		H5P_DEFAULT);
	if (dataset_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error(
				H5_ERR_HDF5,
				"Cannot create dataset \"%s/%s\"",
				h5_get_objname (loc_id),
				dataset_name));
	HDF5_WRAPPER_RETURN (dataset_id);
}

/*!
  Close dataset.

  \param[in]	f		file handle
  \param[in]	dataset_id	id of dataset to close
*/
h5_err_t
hdf5_close_dataset (
	const hid_t dataset_id
	) {
	HDF5_WRAPPER_ENTER2 (h5_err_t,
			     "dataset_id=%d (%s)", 
			    dataset_id,
			    h5_get_objname (dataset_id));
	if (dataset_id == 0 || dataset_id == -1)
		HDF5_WRAPPER_LEAVE (H5_SUCCESS); 

	if (H5Dclose (dataset_id) < 0) {
		HDF5_WRAPPER_LEAVE (
			h5_error(
				H5_ERR_HDF5,
				"Close of dataset \"%s\" failed.",
				h5_get_objname (dataset_id)));
	}
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

/*!
  Get dataspace of existing dataset

  \param[in]	f		file handle
  \param[in]	dataset_id	id of dataset

 */
hid_t
hdf5_get_dataset_space (
	const hid_t dataset_id
	) {
	HDF5_WRAPPER_ENTER2 (hid_t,
			     "dataset_id=%d (%s)",
			     dataset_id,
			     h5_get_objname(dataset_id));
	hid_t dataspace_id = H5Dget_space (dataset_id);
	if (dataspace_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get dataspace for dataset \"%s\".",
				h5_get_objname (dataset_id)));
	HDF5_WRAPPER_RETURN (dataspace_id);
}

/*!
  Wrapper for H5Dwrite.
  

  \param[in]	f		file handle
  \param[in]	dataset_id	id of dataset
  \param[in]	type_id		type used in dataset
  \param[in]	memspace_id	id of memory space 
  \param[in]	diskspace_id	id of disk space
  \param[in]	xfer_prop	transfer property list
  \param[in]	buf		buffer with date to write

 */
h5_err_t
hdf5_write_dataset (
	const hid_t dataset_id,
	const hid_t type_id,
	const hid_t memspace_id,
	const hid_t diskspace_id,
	const hid_t xfer_prop,
	const void* buf
	) {
	HDF5_WRAPPER_ENTER3 (h5_err_t,
			     "dataset_id=%d (%s) type_id=%d",
			     dataset_id,
			     h5_get_objname(dataset_id),
			     type_id);
			    
	herr_t herr = H5Dwrite (
		dataset_id,
		type_id,
		memspace_id,
		diskspace_id,
		xfer_prop,
		buf);
	if (herr < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Write to dataset \"%s\" failed.",	\
				h5_get_objname (dataset_id)));

	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

/*
  Wrapper for H5Dread
*/
h5_err_t
hdf5_read_dataset (
	const hid_t dataset_id,
	const hid_t type_id,
	const hid_t memspace_id,
	const hid_t diskspace_id,
	const hid_t xfer_prop,
	void* const buf ) {
	HDF5_WRAPPER_ENTER3 (h5_err_t,
			     "dataset_id=%d (%s) type_id=%d",
			     dataset_id,
			     h5_get_objname(dataset_id),
			     type_id);
	herr_t herr = H5Dread (
		dataset_id,
		type_id,
		memspace_id,
		diskspace_id,
		xfer_prop,
		buf);
	if (herr < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error(
				H5_ERR_HDF5,
				"Error reading dataset \"%s\".",
				h5_get_objname (dataset_id)));
	
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

hid_t
hdf5_get_dataset_type ( 
	const hid_t dataset_id
	) {
	HDF5_WRAPPER_ENTER2 (h5_err_t,
			     "dataset_id=%d (%s)",
			     dataset_id,
			     h5_get_objname(dataset_id));
	hid_t datatype_id = H5Dget_type (dataset_id);
	if (datatype_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error(
				H5_ERR_HDF5,
				"Cannot determine dataset type."));

	HDF5_WRAPPER_RETURN (datatype_id);
}


h5_err_t
hdf5_set_dataset_extent (
	hid_t dataset_id,
	const hsize_t* size
	) {
	HDF5_WRAPPER_ENTER3 (h5_err_t,
			     "dataset_id=%d (%s), size=%llu",
			     dataset_id,
			     h5_get_objname(dataset_id),
			     *size);
	if (H5Dset_extent(dataset_id, size) < 0) {
		HDF5_WRAPPER_LEAVE (
			h5_error(
				H5_ERR_HDF5,
				"Changing size of dataset \"%s\" dimensions failed.",
				h5_get_objname (dataset_id)));
	}
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_ssize_t
hdf5_get_npoints_of_dataset (
	hid_t dataset_id
	) {
	HDF5_WRAPPER_ENTER2 (h5_ssize_t,
			     "dataset_id=%d (%s)",
			     dataset_id,
			     h5_get_objname(dataset_id));
	hid_t dspace_id;
	hsize_t size;
	TRY (dspace_id = hdf5_get_dataset_space (dataset_id));
	TRY (size = hdf5_get_npoints_of_dataspace (dspace_id));
	TRY (hdf5_close_dataspace (dspace_id));
	HDF5_WRAPPER_RETURN (size);
}

h5_ssize_t
hdf5_get_npoints_of_dataset_by_name (
	hid_t loc_id,
	char* name
	) {
	HDF5_WRAPPER_ENTER3 (h5_ssize_t,
			     "loc_id=%d (%s), name=\"%s\"",
			     loc_id,
			     h5_get_objname(loc_id),
			     name);
	hid_t dset_id;
	hsize_t size;
	TRY (dset_id = hdf5_open_dataset (loc_id, name));
	TRY (size = hdf5_get_npoints_of_dataset (dset_id));
	TRY (hdf5_close_dataset (dset_id));
	HDF5_WRAPPER_RETURN (size);
}


/****** D a t a s p a c e ****************************************************/
/*!
  Create dataspace for dataset. H5Screate_simple wrapper.

  \param[in]	rank		rank of dataspace
  \param[in]	dims		dimensions of dataspace
  \param[in]	maxdims		maximum dimensions of dataspace

 */
hid_t 
hdf5_create_dataspace (
	const int rank,
	const hsize_t* dims,
	const hsize_t* maxdims 
	) {
	HDF5_WRAPPER_ENTER1 (hid_t,
			    "rank=%d",
			     rank);
	hid_t dataspace_id = H5Screate_simple (rank, dims, maxdims);
	if (dataspace_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot create dataspace with rank %d.",
				rank));
	HDF5_WRAPPER_RETURN (dataspace_id);
}

hid_t 
hdf5_create_dataspace_scalar (
	void
	) {
	HDF5_WRAPPER_ENTER0 (hid_t);
	hid_t dataspace_id = H5Screate (H5S_SCALAR);
	if (dataspace_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot create scalar dataspace."));
	HDF5_WRAPPER_RETURN (dataspace_id);
}

h5_err_t
hdf5_select_hyperslab_of_dataspace (
	hid_t space_id,
	H5S_seloper_t op,
	const hsize_t* start,
	const hsize_t* stride,
	const hsize_t* count,
	const hsize_t* block
	) {
	HDF5_WRAPPER_ENTER1 (h5_err_t,
			    "%d",
			     space_id);
	herr_t herr = H5Sselect_hyperslab (
		space_id,
		op,
		start,
		stride,
		count,
		block);
	if (herr < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot set select hyperslap region or add the "
				"specified region"));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_err_t
hdf5_select_elements_of_dataspace (
	hid_t space_id,
	H5S_seloper_t op,
	hsize_t nelems,
	const hsize_t* indices
	) {
	HDF5_WRAPPER_ENTER1 (h5_err_t,
			    "%d",
			     space_id);
	herr_t herr;
	if ( nelems > 0 ) {
		herr = H5Sselect_elements (
			space_id,
			op,
			nelems,
			indices);
	} else {
		herr = H5Sselect_none ( space_id );
	}
	if (herr < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot set select hyperslap region or add the "
				"specified region"));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_ssize_t
hdf5_get_selected_npoints_of_dataspace (
	hid_t space_id
	) {
	HDF5_WRAPPER_ENTER1 (h5_ssize_t,
			    "%d",
			     space_id);
	hssize_t size = H5Sget_select_npoints (space_id);
	if (size < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error(
				H5_ERR_HDF5,
				"Cannot determine number of "
				"selected elements in dataspace."));
	HDF5_WRAPPER_RETURN (size);
}

h5_ssize_t
hdf5_get_npoints_of_dataspace (
	hid_t space_id
	) {
	HDF5_WRAPPER_ENTER1 (h5_ssize_t,
			    "%d",
			     space_id);
	hssize_t size = H5Sget_simple_extent_npoints (space_id);
	if (size < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error(
				H5_ERR_HDF5,
				"Cannot determine number of"
				"elements in dataspace."));
	HDF5_WRAPPER_RETURN (size);
}

int
hdf5_get_dims_of_dataspace (
	hid_t space_id,
	hsize_t* dims,
	hsize_t* maxdims 
	) {
	HDF5_WRAPPER_ENTER1 (int,
			    "%d",
			     space_id);
	int rank = H5Sget_simple_extent_dims (space_id, dims, maxdims);
	if (rank < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot determine rank of dataspace."));
	HDF5_WRAPPER_RETURN (rank);
}


/*!
  Close space.

  \param[in]	f		file handle
  \param[in]	dataspace_id	id of space to close
*/
h5_err_t
hdf5_close_dataspace (
	const hid_t dataspace_id
	) {
	HDF5_WRAPPER_ENTER1 (h5_err_t,
			    "dataspace_%d",
			     dataspace_id);
	if (dataspace_id <= 0 || dataspace_id == H5S_ALL)
		HDF5_WRAPPER_LEAVE (H5_SUCCESS); 

	herr_t herr = H5Sclose (dataspace_id);
	if (herr < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error(
				H5_ERR_HDF5,
				"Cannot terminate access to dataspace!"));

	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

/****** D a t a t y p e ******************************************************/

char_p
hdf5_get_type_name (
	hid_t type_id
	) {
	if (type_id == H5_INT32_T)
		return "H5_INT32_T";
	if (type_id == H5_INT64_T)
		return "H5_INT64_T";
	if (type_id == H5_FLOAT32_T)
		return "H5_FLOAT32_T";
	if (type_id == H5_FLOAT64_T)
		return "H5_FLOAT64_T";
	if (type_id == H5_STRING_T)
		return "H5_STRING_T";

	h5_warn ("Unknown type id %d", type_id);
	return "[unknown]";
}

static const char*
get_class_type_name (
	const hid_t class_id
	) {
	const char* const map[] = {
		[H5T_INTEGER]	"H5T_INTEGER",
		[H5T_FLOAT]	"H5T_FLOAT",
		[H5T_TIME]	"H5T_TIME",
		[H5T_STRING]	"H5T_STRING",
		[H5T_BITFIELD]	"H5T_BITFIELD",
		[H5T_OPAQUE]    "H5T_OPAQUE",
		[H5T_COMPOUND]  "H5T_COMPOUND",
		[H5T_REFERENCE] "H5T_REFERENCE",
		[H5T_ENUM]      "H5T_ENUM",
		[H5T_VLEN]      "H5T_VLEN",
		[H5T_ARRAY]     "H5T_ARRAY"
	};
	if (class_id < 0 || class_id >= H5T_NCLASSES) {
		return ("[unknown]");
	}
	return map[class_id];
}

/*!
  Create array type. Wrapper for "H5Tarray_create".

  \param[in]	f		file handle
  \param[in]	base_type_id	base type
  \param[in]	rank		rank of array
  \param[in]	dims		dimensions
*/
hid_t
hdf5_create_array_type (
	const hid_t base_type_id,
	const int rank,
	const hsize_t* dims
	) {
	HDF5_WRAPPER_ENTER3 (hid_t,
			     "base_type_id=%d (%s), rank=%d",
			     base_type_id,
			     hdf5_get_type_name (base_type_id),
			     rank);
	hid_t type_id = H5Tarray_create (base_type_id, rank, dims);
	if (type_id < 0) {
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Can't create array datatype object with base "
				"type %s and rank %d",
				hdf5_get_type_name (base_type_id),
				rank));
	}
	HDF5_WRAPPER_RETURN (type_id);
}

hid_t
hdf5_create_type (
	H5T_class_t class,
	const size_t size
	) {
	HDF5_WRAPPER_ENTER2 (hid_t,
			     "class=%d (%s)",
			     class,
			     get_class_type_name (class));
	hid_t type_id = H5Tcreate (class, size);
	if (type_id < 0) {
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Can't create datatype object of class %s.",
				get_class_type_name (class)));
	}
	HDF5_WRAPPER_RETURN (type_id);
}

hid_t
hdf5_create_string_type(
	const hsize_t len
	) {
	HDF5_WRAPPER_ENTER1 (hid_t,
			     "len = %llu", len);
	hid_t type_id = H5Tcopy ( H5T_C_S1 );
	if (type_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error(
				H5_ERR_HDF5,
				"Can't duplicate C string type."));
	
	herr_t herr = H5Tset_size ( type_id, len );
	if (herr < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error(
				H5_ERR_HDF5,
				"Can't set length of C string type."));
	HDF5_WRAPPER_RETURN (type_id);
}

h5_err_t
hdf5_insert_type (
	hid_t type_id,
	const char* name,
	size_t offset,
	hid_t field_id
	) {
	HDF5_WRAPPER_ENTER2 (h5_err_t, "type_id=%d, name=%s", type_id, name);
	herr_t herr = H5Tinsert (type_id, name, offset, field_id);
	if (herr < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error(
				H5_ERR_HDF5,
				"Can't insert field %s to compound datatype.",
				name));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_err_t
hdf5_close_type (
	hid_t dtype_id
	) {
	HDF5_WRAPPER_ENTER1 (h5_err_t, "dtype_id=%d", dtype_id);
	herr_t herr = H5Tclose (dtype_id);
	if (herr < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot release datatype."));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

/****** P r o p e r t y ******************************************************/

hid_t
hdf5_create_property (
	hid_t cls_id
	) {
	HDF5_WRAPPER_ENTER1 (hid_t, "cls_id=%d", cls_id);
	hid_t prop_id = H5Pcreate (cls_id);
	if (prop_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot create property list."));
	HDF5_WRAPPER_RETURN (prop_id);
}

/*!
  Get create properties of existing dataset

  \param[in]	f		file handle
  \param[in]	dataset_id	id of dataset

 */
hid_t
hdf5_get_dataset_create_plist (
	const hid_t dataset_id
	) {
	HDF5_WRAPPER_ENTER2 (hid_t, "dataset_id=%d (%s)",
			    dataset_id,
			    h5_get_objname (dataset_id));
	hid_t plist_id = H5Dget_create_plist (dataset_id);
	if (plist_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get create properties for dataset \"%s\".",
				h5_get_objname (dataset_id)));
	HDF5_WRAPPER_RETURN (plist_id);
}

h5_err_t
hdf5_set_chunk_property (
	hid_t plist,
	int rank,
	hsize_t* dims
	) {
	HDF5_WRAPPER_ENTER3 (h5_err_t,
			    "plist=%d, rank=%d, dims[0]=%llu ...",
			     plist, rank, dims[0]);
	if (H5Pset_chunk (plist, rank, dims) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot add chunking property to list."));

	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_err_t
hdf5_get_chunk_property (
	hid_t plist,
	int rank,
	hsize_t* dims
	) {
	HDF5_WRAPPER_ENTER2 (h5_err_t, "plist=%d, rank=%d", plist, rank);
	if (H5Pget_chunk (plist, rank, dims) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get chunking property from list."));

	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_err_t
hdf5_set_layout_property (
	hid_t plist,
	H5D_layout_t layout
	) {
	HDF5_WRAPPER_ENTER1 (h5_err_t, "plist=%d", plist);
	if (H5Pset_layout (plist, layout) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot add layout property to list."));

	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

#ifdef PARALLEL_IO
h5_err_t
hdf5_set_fapl_mpio_property (
	hid_t fapl_id,
	MPI_Comm comm,
	MPI_Info info
	) {
	HDF5_WRAPPER_ENTER3 (h5_err_t,
			     "fapl_id=%d, comm=%d, info=%d",
			     fapl_id, (int)comm, (int)info);
	if (H5Pset_fapl_mpio (fapl_id, comm, info) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot store IO communicator information to the "
				"file access property list."));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_err_t
hdf5_set_fapl_mpiposix_property (
	hid_t fapl_id,
	MPI_Comm comm,
	hbool_t	use_gpfs
	) {
	HDF5_WRAPPER_ENTER3 (h5_err_t,
			     "fapl_id=%d, comm=%d, use_gpfs=%d",
			     fapl_id, (int)comm, (int)use_gpfs);
	if ( H5Pset_fapl_mpiposix (fapl_id, comm, use_gpfs) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot store IO communicator information to"
				" the file access property list."));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_err_t
hdf5_set_dxpl_mpio_property (
	hid_t dxpl_id,
	H5FD_mpio_xfer_t mode
	) {
	HDF5_WRAPPER_ENTER2 (h5_err_t,
			     "dxpl_id=%d, mode=%d",
			     dxpl_id, (int)mode);
	if (H5Pset_dxpl_mpio (dxpl_id, mode) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot store IO communicator information to"
				" the dataset transfer property list."));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}
#endif

h5_err_t
hdf5_set_mdc_property (
	hid_t fapl_id,
	H5AC_cache_config_t *config
	) {
	HDF5_WRAPPER_ENTER2 (h5_err_t,
			     "fapl_id=%d, config=0x%p",
			     fapl_id, config);
	if (H5Pset_mdc_config (fapl_id, config) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot store metadata cache configuration in"
				" the file access property list."));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_err_t
hdf5_get_mdc_property (
	hid_t fapl_id,
	H5AC_cache_config_t *config
	) {
	HDF5_WRAPPER_ENTER2 (h5_err_t,
			     "fapl_id=%d, config=0x%p",
			     fapl_id, config);
	if (H5Pget_mdc_config (fapl_id, config) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get metadata cache configuration in"
				" the file access property list."));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_err_t
hdf5_set_alignment_property (
	const hid_t fapl_id,
	const hsize_t threshold,
	const hsize_t alignment
	) {
	HDF5_WRAPPER_ENTER3 (h5_err_t,
			     "fapl_id=%d, threshold=%llu, alignment=%llu",
			     fapl_id, threshold, alignment);
	if (H5Pset_alignment (fapl_id, threshold, alignment) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot set alignment in the "
				"file access property list."));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_err_t
hdf5_set_btree_ik_property (
	const hid_t fcpl_id,
	const hsize_t btree_ik
	) {
	HDF5_WRAPPER_ENTER2 (h5_err_t,
			     "fapl_id=%d, btree_ik=%llu",
			     fcpl_id, btree_ik);
	if (H5Pset_istore_k (fcpl_id, btree_ik) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot set btree size in the "
				"file access property list."));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_err_t
hdf5_close_property (
	hid_t prop
	) {
	HDF5_WRAPPER_ENTER1 (h5_err_t, "prop=%d", prop);
	if (H5Pclose (prop) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot close property."));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

/****** F i l e **************************************************************/

h5_err_t
hdf5_close_file (
	hid_t file_id
	) {
	HDF5_WRAPPER_ENTER2 (h5_err_t,
			     "file_id=%d (%s)", 
			     file_id,
			     h5_get_objname (file_id));
	if (H5Fclose (file_id) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot close file \"%s\".",
				h5_get_objname (file_id)));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

/****** E r r o r h a n d l i n g ********************************************/

h5_err_t
hdf5_set_errorhandler (
	hid_t estack_id,
	H5E_auto_t func,
	void* client_data
	) {
	HDF5_WRAPPER_ENTER3 (h5_err_t,
			    "estack_id=%d, func=0x%p, client_data=0x%p",
			    estack_id, func, client_data);
	if (H5Eset_auto (estack_id, func, client_data) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_INIT,
				"Cannot initialize H5."));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

/****** A t t r i b u t e ****************************************************/
hid_t
hdf5_open_attribute (
	hid_t loc_id,
	const char* attrib_name
	) {
	HDF5_WRAPPER_ENTER3 (hid_t,
			     "loc_id=%d (%s), attr_name=\"%s\"",
			     loc_id, h5_get_objname (loc_id), attrib_name);
	hid_t attrib_id = H5Aopen (loc_id, attrib_name, H5P_DEFAULT);
	if (attrib_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot open attribute \"%s\" of \"%s\".",
				attrib_name,
				h5_get_objname (loc_id)));
	HDF5_WRAPPER_RETURN (attrib_id);
}

hid_t
hdf5_open_attribute_idx (
	hid_t loc_id,
	unsigned int idx
	) {
	HDF5_WRAPPER_ENTER3 (hid_t,
			     "loc_id=%d (%s), idx=%u",
			     loc_id, h5_get_objname (loc_id), idx);
	hid_t attr_id = H5Aopen_idx (loc_id, idx);
	if (attr_id < 0) 
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot open attribute \"%u\" of \"%s\".",
				idx,
				h5_get_objname (loc_id)));
	HDF5_WRAPPER_RETURN (attr_id);
}	

hid_t
hdf5_open_attribute_by_name (
	hid_t loc_id,
	const char* obj_name,
	const char* attr_name
	) {
	HDF5_WRAPPER_ENTER4 (hid_t,
			     "loc_id=%d (%s), obj_name=\"%s\", attr_name=\"%s\"",
			     loc_id, h5_get_objname (loc_id),
			     obj_name, attr_name);
	hid_t attr_id = H5Aopen_by_name (
		loc_id,
		obj_name,
		attr_name,
		H5P_DEFAULT,
		H5P_DEFAULT);
	if (attr_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot open attribute \"%s\" of \"%s\".",
				attr_name,
				obj_name));
	HDF5_WRAPPER_RETURN (attr_id);
}

hid_t
hdf5_create_attribute (
	hid_t loc_id,
	const char* attr_name,
	hid_t type_id,
	hid_t space_id,
	hid_t acpl_id,
	hid_t aapl_id
	) {
	HDF5_WRAPPER_ENTER4 (hid_t,
			     "loc_id=%d (%s), attr_name=\"%s\", type_id=%d",
			     loc_id, h5_get_objname (loc_id),
			     attr_name, type_id);
	hid_t attr_id = H5Acreate ( 
		loc_id,
		attr_name,
		type_id,
		space_id,
		acpl_id,
		aapl_id);
	if (attr_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot create attribute \"%s\" for \"%s\".",
				attr_name,
				h5_get_objname (loc_id)));
	HDF5_WRAPPER_RETURN (attr_id);
}

h5_err_t
hdf5_read_attribute (
	hid_t attr_id,
	hid_t mem_type_id,
	void* buf
	) {
	HDF5_WRAPPER_ENTER4 (h5_err_t,
			     "attr_id=%d (%s), mem_type_id=%d, buf=0x%p",
			     attr_id, h5_get_objname (attr_id),
			     mem_type_id, buf);
	if (H5Aread (attr_id, mem_type_id, buf) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot read attribute \"%s\".",
				h5_get_objname (attr_id)));
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

/*
  Wrapper for H5Awrite.
 */
h5_err_t
hdf5_write_attribute (
	hid_t attr_id,
	hid_t mem_type_id,
	const void* buf
	) {
	HDF5_WRAPPER_ENTER4 (h5_err_t,
			     "attr_id=%d (%s), mem_type_id=%d, buf=0x%p",
			     attr_id, h5_get_objname (attr_id),
			     mem_type_id, buf);
	if (H5Awrite (attr_id, mem_type_id, buf) < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot write attribute \"%s\".",
				h5_get_objname (attr_id)));

	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

h5_ssize_t
hdf5_get_attribute_name (
	hid_t attr_id,
	size_t buf_size,
	char *buf
	) {
	HDF5_WRAPPER_ENTER4 (h5_ssize_t,
			     "attr_id=%d (%s), buf_size=%zd, buf=0x%p",
			     attr_id, h5_get_objname (attr_id),
			     buf_size, buf);
	ssize_t size = H5Aget_name ( attr_id, buf_size, buf );
	if (size < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get attribute name." ));
	HDF5_WRAPPER_RETURN ((h5_size_t)size);
}

hid_t
hdf5_get_attribute_type (
	hid_t attr_id
	) {
	HDF5_WRAPPER_ENTER2 (hid_t,
			     "attr_id=%d (%s)",
			     attr_id, h5_get_objname (attr_id));
	hid_t datatype_id = H5Aget_type (attr_id);
	if (datatype_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get type of attribute \"%s\".",
				h5_get_objname (attr_id)));
	HDF5_WRAPPER_RETURN (datatype_id);
}

hid_t
hdf5_get_attribute_dataspace (
	hid_t attr_id
	) {
	HDF5_WRAPPER_ENTER2 (hid_t,
			     "attr_id=%d (%s)",
			     attr_id, h5_get_objname (attr_id));
	hid_t space_id = H5Aget_space (attr_id);
	if (space_id < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get dataspace of attribute \"%s\".",
				h5_get_objname (attr_id)));
	HDF5_WRAPPER_RETURN (space_id);
}

int
hdf5_get_num_attribute (
	hid_t loc_id
	) {
	HDF5_WRAPPER_ENTER2 (int,
			     "loc_id=%d (%s)",
			     loc_id, h5_get_objname (loc_id));
	int num = H5Aget_num_attrs (loc_id);
	if (num < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get number of attributes of \"%s\".",
				h5_get_objname (loc_id)));
	HDF5_WRAPPER_RETURN (num);
}


h5_err_t
hdf5_close_attribute (
	hid_t attr_id
	) {
	HDF5_WRAPPER_ENTER2 (h5_err_t,
			     "attr_id=%d (%s)",
			     attr_id, h5_get_objname (attr_id));
	if (H5Aclose (attr_id))
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot close attribute \"%s\".",
				h5_get_objname (attr_id)));

	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

/****** L i n k **************************************************************/
h5_err_t
hdf5_link_exists (
	const hid_t loc_id,
	const char* name
	) {
	HDF5_WRAPPER_ENTER3 (h5_err_t,
			     "loc_id=%d (%s), name=\"%s\"",
			     loc_id, h5_get_objname (loc_id), name);
	/* Save old error handler */
	H5E_auto2_t old_func;
	void *old_client_data;

	H5Eget_auto2(H5E_DEFAULT, &old_func, &old_client_data);

	/* Turn off error handling */
	H5Eset_auto(H5E_DEFAULT, NULL, NULL);

	/* Probe. Likely to fail, but that’s okay */
	htri_t exists = H5Lexists ( loc_id, name, H5P_DEFAULT );

	/* Restore previous error handler */
	H5Eset_auto(H5E_DEFAULT, old_func, old_client_data);

	if (exists < 0 )
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot query link %s/%s.",
				h5_get_objname (loc_id), name));
	HDF5_WRAPPER_RETURN (exists);
}

h5_err_t
hdf5_delete_link (
	hid_t loc_id,
	const char* name,
	hid_t lapl_id
	) {
	HDF5_WRAPPER_ENTER4 (h5_err_t,
			     "loc_id=%d (%s), name=\"%s\", lapl_id=%d",
			     loc_id, h5_get_objname (loc_id), name, lapl_id);
	if (H5Ldelete (loc_id, name, lapl_id)  < 0)
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot remove link %s/%s.",
				h5_get_objname (loc_id), name));

	HDF5_WRAPPER_RETURN (H5_SUCCESS);
} 

typedef struct op_data {
	int queried_idx;
	int cnt;
	H5O_type_t type;
	char *name;
	size_t len;
	char *prefix;
} op_data_t;

static H5O_type_t
iter_op_get_obj_type (
	const hid_t g_id,
	const char* name,
	const H5L_info_t* info
	) {
	H5_PRIV_FUNC_ENTER3 (H5O_type_t,
			     "g_id=%d, name=\"%s\", info=0x%p",
			     g_id, name, info);
	herr_t herr;
	H5O_info_t objinfo;

	if ( info->type == H5L_TYPE_EXTERNAL ) {
		char *buf;
		TRY (buf = h5_calloc (1, info->u.val_size));

		herr = H5Lget_val(g_id, name, buf,
					info->u.val_size, H5P_DEFAULT);
		if (herr < 0)
			H5_PRIV_FUNC_LEAVE (
				(H5O_type_t)h5_error (
					H5_ERR_HDF5,
					"Can't get external link for object '%s'!",
					name));

		const char *filename;
		const char *objname;
		herr = H5Lunpack_elink_val(buf, info->u.val_size, 0,
					   &filename, &objname);
		if (herr < 0)
			H5_PRIV_FUNC_LEAVE (
				(H5O_type_t)h5_error(
					H5_ERR_HDF5,
					"Can't unpack external link for object '%s'!",
					name));
		
		h5_debug(
			"Followed external link to file '%s' / object '%s'.",
			filename, objname);

		h5_free (buf);

		hid_t obj_id = H5Oopen(g_id, name, H5P_DEFAULT);
		if (obj_id < 0)
			H5_PRIV_FUNC_LEAVE (
				(H5O_type_t)h5_error(
					H5_ERR_HDF5,
					"Can't open external link for object '%s'!",
					name));
		herr = H5Oget_info(obj_id, &objinfo);	
	}
	else { // H5L_TYPE_HARD
		herr = H5Oget_info_by_name(g_id, name, &objinfo, H5P_DEFAULT);
	}
	
	if (herr < 0)
		H5_PRIV_FUNC_LEAVE (
			(H5O_type_t)h5_error(
				H5_ERR_HDF5,
				"Can't query object with name '%s'!", name));

	H5_PRIV_FUNC_RETURN (objinfo.type);
}

static herr_t
iter_op_count (
	hid_t g_id,
	const char* name,
	const H5L_info_t* info,
	void* _op_data
	) {
	H5_PRIV_FUNC_ENTER4 (herr_t,
			     "g_id=%d, name=\"%s\", info=0x%p, _op_data=0x%p",
			     g_id, name, info, _op_data);
	op_data_t* op_data = (op_data_t*)_op_data;
	H5O_type_t type;
	TRY (type = iter_op_get_obj_type (g_id, name, info));
	if (type != op_data->type )
		H5_PRIV_FUNC_LEAVE (0);
	op_data->cnt++;
	H5_PRIV_FUNC_RETURN (0);
}

static herr_t
iter_op_idx (
	hid_t g_id,
	const char* name,
	const H5L_info_t* info,
	void* _op_data
	) {
	H5_PRIV_FUNC_ENTER4 (herr_t,
			     "g_id=%d, name=\"%s\", info=0x%p, _op_data=0x%p",
			     g_id, name, info, _op_data);
	op_data_t* op_data = (op_data_t*)_op_data;
	H5O_type_t type;
	TRY (type = iter_op_get_obj_type (g_id, name, info));
	if (type != op_data->type)
		H5_PRIV_FUNC_LEAVE (0);
	op_data->cnt++;
	/* stop iterating if index is equal cnt */
	if (op_data->queried_idx == op_data->cnt) {
		memset (op_data->name, 0, op_data->len);
		strncpy (op_data->name, name, op_data->len-1);
		H5_PRIV_FUNC_LEAVE (1);
	}
	H5_PRIV_FUNC_RETURN (0);
}

static herr_t
iter_op_count_match (
	hid_t g_id,
	const char* name,
	const H5L_info_t* info,
	void* _op_data
	) {
	H5_PRIV_FUNC_ENTER4 (herr_t,
			     "g_id=%d, name=\"%s\", info=0x%p, _op_data=0x%p",
			     g_id, name, info, _op_data);
	op_data_t* op_data = (op_data_t*)_op_data;
	H5O_type_t type;
	TRY (type = iter_op_get_obj_type (g_id, name, info));
	if (type != op_data->type)
		H5_PRIV_FUNC_LEAVE (0);
	/* count if prefix matches */
	if (strncmp (name, op_data->prefix, strlen(op_data->prefix)) == 0) {
		op_data->cnt++;
	}
	H5_PRIV_FUNC_RETURN (0);
}

ssize_t
h5_get_num_hdf5_groups (
	const hid_t loc_id
	) {
	HDF5_WRAPPER_ENTER2 (ssize_t,
			     "loc_id=%d (%s)", loc_id, h5_get_objname (loc_id));
	op_data_t op_data;
	memset (&op_data, 0, sizeof (op_data));
	op_data.type = H5O_TYPE_GROUP;
	hsize_t start_idx = 0;
	herr_t herr = H5Literate (loc_id, H5_INDEX_NAME, H5_ITER_INC,
				  &start_idx,
				  iter_op_count, &op_data);
	if (herr < 0) {
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get number of groups in \"%s\".",
				h5_get_objname (loc_id)));
	}
	HDF5_WRAPPER_RETURN (op_data.cnt);
}

ssize_t
h5_get_num_hdf5_groups_matching_prefix (
	const hid_t loc_id,
	char* prefix
	) {
	HDF5_WRAPPER_ENTER3 (ssize_t,
			     "loc_id=%d (%s), prefix=\"%s\"",
			     loc_id, h5_get_objname (loc_id), prefix);
	op_data_t op_data;
	memset (&op_data, 0, sizeof (op_data));
	op_data.type = H5O_TYPE_GROUP;
	op_data.prefix = prefix;
	hsize_t start_idx = 0;
	herr_t herr = H5Literate (loc_id, H5_INDEX_NAME, H5_ITER_INC,
				  &start_idx,
				  iter_op_count_match, &op_data);
	if (herr < 0) {
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get number of groups with prefix"
				" \"%s\" in \"%s\".",
				prefix, h5_get_objname (loc_id)));
	}
	HDF5_WRAPPER_RETURN (op_data.cnt);
}

h5_err_t
h5_get_hdf5_groupname_by_idx (
	hid_t loc_id,
	hsize_t idx,
	char *name,
	size_t len
	) {
	HDF5_WRAPPER_ENTER5 (h5_err_t,
			     "loc_id=%d (%s), idx=%llu, name=0x%p, len=%zd",
			     loc_id, h5_get_objname (loc_id),
			     idx, name, len);
	op_data_t op_data;
	memset (&op_data, 0, sizeof (op_data));
	op_data.type = H5O_TYPE_GROUP;
	op_data.cnt = -1;
	op_data.queried_idx = idx;
        op_data.name = name;
        op_data.len = len;
	hsize_t start_idx = 0;
	herr_t herr = H5Literate (loc_id, H5_INDEX_NAME, H5_ITER_INC,
				  &start_idx,
				  iter_op_idx, &op_data);
	if (herr < 0) {
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get name of group with index"
				" \"%lu\" in \"%s\".",
				(long unsigned int)idx,
				h5_get_objname (loc_id)));
	}
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

ssize_t
h5_get_num_hdf5_datasets (
	const hid_t loc_id
	) {
	HDF5_WRAPPER_ENTER2 (ssize_t,
			     "loc_id=%d (%s)", loc_id, h5_get_objname (loc_id));
	op_data_t op_data;
	memset (&op_data, 0, sizeof (op_data));
	op_data.type = H5O_TYPE_DATASET;
	hsize_t start_idx = 0;
	herr_t herr = H5Literate (loc_id, H5_INDEX_NAME, H5_ITER_INC,
				  &start_idx,
				  iter_op_count, &op_data);
	if (herr < 0) {
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get number of datasets in \"%s\".",
				h5_get_objname (loc_id)));
	}
	HDF5_WRAPPER_RETURN (op_data.cnt);
}

/*
  Get name of the \c idx th dataset in group \c loc_id.
 */
h5_err_t
h5_get_hdf5_datasetname_by_idx (
	hid_t loc_id,
	hsize_t idx,
	char *name,
	size_t len
	) {
	HDF5_WRAPPER_ENTER5 (h5_err_t,
			     "loc_id=%d (%s), idx=%llu, name=0x%p, len=%zd",
			     loc_id, h5_get_objname (loc_id),
			     idx, name, len);
	op_data_t op_data;
	memset (&op_data, 0, sizeof (op_data));
	op_data.type = H5O_TYPE_DATASET;
	op_data.cnt = -1;
	op_data.queried_idx = idx;
        op_data.name = name;
        op_data.len = len;
	hsize_t start_idx = 0;
	herr_t herr = H5Literate (loc_id, H5_INDEX_NAME, H5_ITER_INC,
				  &start_idx,
				  iter_op_idx, &op_data);
	if (herr < 0) {
		HDF5_WRAPPER_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Cannot get name of dataset with index"
				" \"%lu\" in \"%s\".",
				(long unsigned int)idx,
				h5_get_objname (loc_id)));
	}
	HDF5_WRAPPER_RETURN (H5_SUCCESS);
}

/****** I d e n t i f i e r **************************************************/

const char *
h5_get_objname (
	hid_t id
	) {
	static char objname[256];

	memset ( objname, 0, sizeof(objname) );
	ssize_t size = H5Iget_name ( id, objname, sizeof(objname) );
	if ( size < 0 ) {
		strcpy ( objname, "[error getting object name]" );
	} else if ( size == 0 ) {
		strcpy ( objname, "[no name associated with identifier]" );
	}

	return objname;
}
