#ifndef __H5T_TAGS_H 
#define __H5T_TAGS_H

typedef struct h5t_tagset h5t_tagset_t;
typedef struct h5t_tagcontainer h5t_tagcontainer_t;

h5_err_t
h5t_mtagset_exists (
	h5_file_t* const f,
	h5t_tagcontainer_t* ctn,
	char* name
	);
h5_err_t
h5t_add_mtagset (
	h5_file_t* const f,
	char name[],
	h5_id_t type
	);
h5_err_t
h5t_remove_mtagset (
	h5_file_t* const f,
	const char name[]
	);
h5_err_t
h5t_open_mtagset (
	h5_file_t* const f,
	const char *name,
	h5t_tagset_t** retval
	);
h5_ssize_t
h5t_get_num_mtagsets (
	h5_file_t* const f
	);
h5_ssize_t
h5t_get_mtagsets (
	h5_file_t* const f,
	char** names[]
	);
h5_err_t
h5t_get_mtagset_info (
	h5_file_t* const f,
	const h5_id_t idx,
	char** names,
	h5_id_t* type
	);
h5_id_t
h5t_get_mtagset_type_by_name (
	h5_file_t* const f,
	char name[]
	);

h5_err_t
h5t_set_mtag_by_name (
	h5_file_t* const f,
	char name[],
	const h5_loc_id_t entity_id,
	const size_t dim,
	void* value
	);

h5_err_t
h5t_get_mtag_by_name (
	h5_file_t* const f,
	const char name[],
	const h5_loc_id_t entity_id,
	size_t* dim,
	void* vals );

h5_err_t
h5t_remove_mtag_by_name (
	h5_file_t* const f,
	const char name[],
	const h5_loc_id_t entity_id
	);

h5_ssize_t
h5t_get_tag (
	h5_file_t *const f,
	const h5t_tagset_t *tagset,
	const h5_loc_id_t entity_id,
	size_t* const dim,
	void* const vals
	);

h5_err_t
h5t_remove_mtag (
	h5_file_t* const f,
	h5t_tagset_t* tagset,
	const h5_loc_id_t entity_id
	);

h5_ssize_t
h5t_get_mtagset_names_of_entity (
	h5_file_t* const f,
	const h5_loc_id_t entity_id,
	char *names[],
	const h5_size_t dim
	);

#endif