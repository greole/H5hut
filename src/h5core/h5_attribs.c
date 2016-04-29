/*
  Copyright (c) 2006-2016, The Regents of the University of California,
  through Lawrence Berkeley National Laboratory (subject to receipt of any
  required approvals from the U.S. Dept. of Energy) and the Paul Scherrer
  Institut (Switzerland).  All rights reserved.

  License: see file COPYING in top level of source distribution.
*/

#include "h5core/h5_init.h"
#include "h5core/h5_debug.h"

#include "private/h5_types.h"
#include "private/h5.h"
#include "private/h5_io.h"
#include "private/h5_hdf5.h"
#include "private/h5_attribs.h"



h5_err_t
h5priv_get_attrib_info_by_idx (
	const hid_t id,			/*!< HDF5 object ID */
	const h5_size_t attrib_idx,	/*!< index of attribute */
	char* attrib_name,		/*!< OUT: name of attribute */
	const h5_size_t len_attrib_name,/*!< buffer length */
	h5_int64_t* attrib_type,	/*!< OUT: H5 type of attribute */
	h5_size_t* attrib_nelem		/*!< OUT: number of elements */
	) {
	H5_PRIV_API_ENTER (h5_err_t,
			   "id=%lld, "
			   "attrib_idx=%llu, "
			   "attrib_name=%p, len_attrib_name=%llu, "
			   "attrib_type=%p, attrib_nelem=%p",
			   (long long int)id,
			   (long long unsigned)attrib_idx,
			   attrib_name,
			   (long long unsigned)len_attrib_name,
			   attrib_type,
			   attrib_nelem);
	hid_t attrib_id;
	TRY (attrib_id = hdf5_open_attribute_idx (
		      id,
		      (unsigned int)attrib_idx));

	if (attrib_name) {
		TRY (hdf5_get_attribute_name (
			     attrib_id,
			     (size_t)len_attrib_name,
			     attrib_name));
	}
        H5_PRIV_API_RETURN (get_attrib_info (attrib_id, attrib_type, attrib_nelem));
}

h5_err_t
h5priv_read_attrib (
	const hid_t id,			/*!< HDF5 object ID */
	const char* attrib_name,	/*!< name of HDF5 attribute to read */
	const hid_t attrib_type,	/*!< HDF5 type of attribute */
	void* const attrib_value	/*!< OUT: attribute value */
	) {
	H5_PRIV_API_ENTER (h5_err_t,
			   "id=%lld, attrib_name='%s', attrib_type=%lld, attrib_value=%p",
			   (long long int)id,
			   attrib_name,
			   (long long int)attrib_type,
			   attrib_value);
	hid_t attrib_id;
	hid_t type_id;
	hid_t space_id;
	TRY (attrib_id = hdf5_open_attribute (id, attrib_name));
	TRY (type_id = hdf5_get_attribute_type (attrib_id));

        hid_t h5type_id;
        TRY (h5type_id = h5priv_normalize_h5_type (type_id));
	if (h5type_id != attrib_type)
		H5_PRIV_API_LEAVE (
			h5_error (
				H5_ERR_HDF5,
				"Attribute '%s' has type '%s' but was requested as '%s'.",
				attrib_name,
				hdf5_get_type_name (h5type_id),
				hdf5_get_type_name (attrib_type)));

	TRY (space_id = hdf5_get_attribute_dataspace (attrib_id));
	TRY (hdf5_read_attribute (attrib_id, type_id, attrib_value));
	TRY (hdf5_close_dataspace(space_id));
	TRY (hdf5_close_type (type_id));
	TRY (hdf5_close_attribute (attrib_id));
	H5_PRIV_API_RETURN (H5_SUCCESS);
}

h5_err_t
h5priv_write_attrib (
        const hid_t id,                 /*!< HDF5 object ID */
        const char* attrib_name,        /*!< name of HDF5 attribute to write */
        const hid_t attrib_type,        /*!< HDF5 type of attribute */
        const void* attrib_value,       /*!< value of attribute */
        const hsize_t attrib_nelem,     /*!< number of elements (dimension) */
        const int overwrite
        ) {
	H5_PRIV_API_ENTER (h5_err_t,
	                   "id=%lld, attrib_name='%s', attrib_type=%lld, "
	                   "attrib_value=%p, attrib_nelem=%llu, overwrite=%d",
	                   (long long int)id,
	                   attrib_name,
	                   (long long int)attrib_type,
	                   attrib_value,
	                   attrib_nelem,
	                   overwrite);
	hid_t space_id;
	hid_t attrib_id;
	hid_t type_id;
	if ( attrib_type == H5T_NATIVE_CHAR ) {
		TRY (type_id = hdf5_create_string_type (attrib_nelem));
		TRY (space_id = hdf5_create_dataspace_scalar ());
	} else {
		type_id = attrib_type;
		TRY (space_id = hdf5_create_dataspace (1, &attrib_nelem, NULL));
	}
	h5_err_t exists;
	TRY (exists = hdf5_attribute_exists (id, attrib_name));
	if (exists) {
		if (overwrite) {
			TRY (hdf5_delete_attribute (id, attrib_name));
		} else {
			H5_PRIV_API_LEAVE (
			        h5_error (H5_ERR_H5, "Cannot overwrite attribute %s/%s",
			                  hdf5_get_objname (id), attrib_name));
		}
	}
	TRY (attrib_id = hdf5_create_attribute (
	             id,
	             attrib_name,
	             type_id,
	             space_id,
	             H5P_DEFAULT, H5P_DEFAULT));

	TRY (hdf5_write_attribute (attrib_id, type_id, attrib_value));
	TRY (hdf5_close_attribute (attrib_id));
	TRY (hdf5_close_dataspace (space_id));

	if (attrib_type == H5T_NATIVE_CHAR)
		TRY (hdf5_close_type (type_id));

	H5_PRIV_API_RETURN (H5_SUCCESS);
}

h5_err_t
h5_has_file_attrib (
	const h5_file_t f_,
	const char* const attrib_name
	) {
        h5_file_p f = (h5_file_p)f_;
	H5_CORE_API_ENTER (h5_err_t,
			   "f=%p, "
			   "attrib_name='%s'",
			   f,
			   attrib_name);
	CHECK_FILEHANDLE (f);
	H5_CORE_API_RETURN (hdf5_attribute_exists(f->root_gid, attrib_name));
}
	
h5_err_t
h5_has_step_attrib (
	const h5_file_t f_,
	const char* const attrib_name
	) {
        h5_file_p f = (h5_file_p)f_;
	H5_CORE_API_ENTER (h5_err_t,
			   "f=%p, "
			   "attrib_name='%s'",
			   f,
			   attrib_name);
	CHECK_FILEHANDLE (f);
	H5_CORE_API_RETURN (hdf5_attribute_exists(f->step_gid, attrib_name));
}

h5_err_t
h5_get_file_attrib_info_by_name (
	const h5_file_t f_,			/*!< IN: handle to open file */
	char* attrib_name,			/*!< IN: name of attribute */
	h5_int64_t* attrib_type,		/*!< OUT: H5 type of attribute */
	h5_size_t* attrib_nelem			/*!< OUT: number of elements */
	) {
        h5_file_p f = (h5_file_p)f_;
	H5_CORE_API_ENTER (h5_err_t,
			   "f=%p, "
			   "attrib_name=%s, "
			   "attrib_type=%p, attrib_nelem=%p",
			   f,
			   attrib_name,
			   attrib_type,
			   attrib_nelem);
	CHECK_FILEHANDLE (f);
	TRY (h5priv_get_attrib_info_by_name (
		     f->root_gid, attrib_name,
		     attrib_type, attrib_nelem));
	H5_CORE_API_RETURN (H5_SUCCESS);
}

h5_err_t
h5_get_step_attrib_info_by_name (
	const h5_file_t f_,			/*!< handle to open file */
	char* attrib_name,			/*!< OUT: name of attribute */
	h5_int64_t* attrib_type,		/*!< OUT: H5 type of attribute */
	h5_size_t* attrib_nelem			/*!< OUT: number of elements */
	) {
        h5_file_p f = (h5_file_p)f_;
	H5_CORE_API_ENTER (h5_err_t,
			   "f=%p, "
			   "attrib_name=%p, "
			   "attrib_type=%p, attrib_nelem=%p",
			   f,
			   attrib_name,
			   attrib_type, attrib_nelem);
	CHECK_FILEHANDLE (f);
	CHECK_TIMEGROUP (f);
	TRY (h5priv_get_attrib_info_by_name (
		     f->step_gid,
                     attrib_name,
		     attrib_type, attrib_nelem));
	H5_CORE_API_RETURN (H5_SUCCESS);
}

h5_ssize_t
h5_get_num_file_attribs (
	const h5_file_t f_                      /*!< handle to open file */
	) {
        h5_file_p f = (h5_file_p)f_;
	H5_CORE_API_ENTER (h5_ssize_t, "f=%p", f);
	CHECK_FILEHANDLE (f);
	H5_CORE_API_RETURN (hdf5_get_num_attribute (f->root_gid));
}

h5_ssize_t
h5_get_num_step_attribs (
	const h5_file_t f_                     	/*!< handle to open file */
	) {
        h5_file_p f = (h5_file_p)f_;
	H5_CORE_API_ENTER (h5_ssize_t, "f=%p", f);
	CHECK_FILEHANDLE (f);
	CHECK_TIMEGROUP (f);
	H5_CORE_API_RETURN (hdf5_get_num_attribute (f->step_gid));
}

h5_err_t
h5_get_file_attrib_info_by_idx (
	const h5_file_t f_,			/*!< handle to open file */
	const h5_size_t attrib_idx,		/*!< index of attribute */
	char* attrib_name,			/*!< OUT: name of attribute */
	const h5_size_t len_attrib_name,	/*!< buffer length */
	h5_int64_t* attrib_type,		/*!< OUT: H5 type of attribute */
	h5_size_t* attrib_nelem			/*!< OUT: number of elements */
	) {
        h5_file_p f = (h5_file_p)f_;
	H5_CORE_API_ENTER (h5_err_t,
			   "f=%p, "
			   "attrib_idx=%llu, "
                           "attrib_name=%p, len_attrib_name=%llu, "
			   "attrib_type=%p, attrib_nelem=%p",
			   f,
			   (long long unsigned)attrib_idx,
			   attrib_name, (long long unsigned)len_attrib_name,
			   attrib_type, attrib_nelem);
	CHECK_FILEHANDLE (f);
	TRY (h5priv_get_attrib_info_by_idx (
		     f->root_gid, attrib_idx, attrib_name, len_attrib_name,
		     attrib_type, attrib_nelem));
	H5_CORE_API_RETURN (H5_SUCCESS);
}

h5_err_t
h5_get_step_attrib_info_by_idx (
	const h5_file_t f_,			/*!< handle to open file */
	const h5_size_t attrib_idx,		/*!< index of attribute */
	char* attrib_name,			/*!< OUT: name of attribute */
	const h5_size_t len_attrib_name,	/*!< buffer length */
	h5_int64_t* attrib_type,		/*!< OUT: H5 type of attribute */
	h5_size_t* attrib_nelem			/*!< OUT: number of elements */
	) {
        h5_file_p f = (h5_file_p)f_;
	H5_CORE_API_ENTER (h5_err_t,
			   "f=%p, "
			   "attrib_idx=%llu, attrib_name=%p, len_attrib_name=%llu, "
			   "attrib_type=%p, attrib_nelem=%p",
			   f,
			   (long long unsigned)attrib_idx,
			   attrib_name,
			   (long long unsigned)len_attrib_name,
			   attrib_type,
			   attrib_nelem);
	CHECK_FILEHANDLE (f);
	CHECK_TIMEGROUP (f);
	TRY (h5priv_get_attrib_info_by_idx (
		     f->step_gid, attrib_idx, attrib_name, len_attrib_name,
		     attrib_type, attrib_nelem));
	H5_CORE_API_RETURN (H5_SUCCESS);
}

h5_err_t
h5_read_file_attrib (
	const h5_file_t f_,
	const char* const attrib_name,
	const hid_t attrib_type,
	void* attrib_value
	) {
        h5_file_p f = (h5_file_p)f_;
	H5_CORE_API_ENTER (h5_err_t,
			   "f=%p, attrib_name='%s', attrib_type=%lld, "
			   "attrib_value=%p",
			   f,
			   attrib_name,
			   (long long int)attrib_type,
			   attrib_value);
	CHECK_FILEHANDLE (f);
	H5_CORE_API_RETURN (h5priv_read_attrib (
				    f->root_gid,
				    attrib_name,
				    attrib_type,
				    attrib_value));
}

h5_err_t
h5_read_step_attrib (
	const h5_file_t f_,
	const char *attrib_name,
	const hid_t attrib_type,
	void *attrib_value
	) {
        h5_file_p f = (h5_file_p)f_;
	H5_CORE_API_ENTER (h5_err_t,
			   "f=%p, attrib_name='%s', attrib_type=%lld, "
			   "attrib_value=%p",
			   f,
			   attrib_name,
			   (long long int)attrib_type,
			   attrib_value);
	CHECK_FILEHANDLE (f);
	CHECK_TIMEGROUP (f);
	H5_CORE_API_RETURN (h5priv_read_attrib (
				    f->step_gid,
				    attrib_name,
				    attrib_type,
				    attrib_value));
}


h5_err_t
h5_write_file_attrib (
	const h5_file_t f_,
	const char *attrib_name,
	const hid_t attrib_type,
	const void *attrib_value,
	const hsize_t attrib_nelem
	) {
        h5_file_p f = (h5_file_p)f_;
	H5_CORE_API_ENTER (h5_err_t,
			   "f=%p, attrib_name='%s', attrib_type=%lld, "
			   "attrib_value=%p, attrib_nelem=%llu",
			   f,
			   attrib_name,
			   (long long int)attrib_type,
			   attrib_value,
			   attrib_nelem);
	CHECK_FILEHANDLE (f);
	CHECK_WRITABLE_MODE (f);
	H5_CORE_API_RETURN (h5priv_write_attrib (
				    f->root_gid,
				    attrib_name,
				    attrib_type,
				    attrib_value,
				    attrib_nelem,
				    !is_appendonly (f)));
}

h5_err_t
h5_write_step_attrib (
	const h5_file_t f_,
	const char *attrib_name,
	const hid_t attrib_type,
	const void *attrib_value,
	const hsize_t attrib_nelem
	) {
        h5_file_p f = (h5_file_p)f_;
	H5_CORE_API_ENTER (h5_err_t,
			   "f=%p, attrib_name='%s', attrib_type=%lld, "
			   "attrib_value=%p, attrib_nelem=%llu",
			   f,
			   attrib_name,
			   (long long int)attrib_type,
			   attrib_value,
			   attrib_nelem);
	CHECK_FILEHANDLE (f);
	CHECK_TIMEGROUP (f);
	CHECK_WRITABLE_MODE (f);
	H5_CORE_API_RETURN (h5priv_write_attrib (
				    f->step_gid,
				    attrib_name,
				    attrib_type,
				    attrib_value,
				    attrib_nelem,
				    !is_appendonly (f)));
}


