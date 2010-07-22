#ifdef H5_USE_LUSTRE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#define __USE_GNU
#include <fcntl.h>
#undef __USE_GNU
#include <lustre/liblustreapi.h>

#include "h5core/h5_core.h"
#include "h5_core_private.h"

#define MSG_HEADER "optimize for lustre: "

static void
_print_stripe_info(struct lov_user_md *lum)
{
	fprintf (stderr, "lmm_magic: %u\n", (unsigned)lum->lmm_magic);
	fprintf (stderr, "lmm_pattern: %u\n", (unsigned)lum->lmm_pattern);
	fprintf (stderr, "lmm_object_id: %lu\n", (unsigned long)lum->lmm_object_id);
	fprintf (stderr, "lmm_object_gr: %lu\n", (unsigned long)lum->lmm_object_gr);
	fprintf (stderr, "lmm_stripe_size: %u\n", (unsigned)lum->lmm_stripe_size);
	fprintf (stderr, "lmm_stripe_count: %u\n", (unsigned)lum->lmm_stripe_count);
	fprintf (stderr, "lmm_stripe_offset: %u\n", (unsigned)lum->lmm_stripe_offset);
}

static ssize_t
_get_lustre_stripe_size ( const char *path )
{
	size_t nbytes = sizeof(struct lov_user_md) +
				INIT_ALLOC_NUM_OSTS * sizeof(struct lov_user_ost_data);
	struct lov_user_md *lum;
	TRY( lum = h5priv_alloc(f, NULL, nbytes) );
	lum->lmm_magic = LOV_USER_MAGIC;

	int fd = open64(path, O_RDONLY);
	if (fd < 0) {
		extern int errno;
		if (errno == EINVAL)
			h5_error(f, H5_ERR_INTERNAL, MSG_HEADER
				"open64: a flag is invalid!");
		else if (errno == EACCES)
			h5_error(f, H5_ERR_INTERNAL, MSG_HEADER
				"open64: access denied or file does not exist!");
		else if (errno == ENAMETOOLONG)
			h5_error(f, H5_ERR_INTERNAL, MSG_HEADER
				"open64: path is too long!");
		else
			h5_error(f, H5_ERR_INTERNAL, MSG_HEADER
				"open64: unspecific error!");
		return -1;
	}
		
	int ret = ioctl(fd, LL_IOC_LOV_GETSTRIPE, lum);
	if (ret == -1) {
		extern int errno;
		if (errno == EBADF)
			h5_error(f, H5_ERR_INTERNAL, MSG_HEADER
				"ioctl: bad file handle!");
		else if (errno == EINVAL)
			h5_error(f, H5_ERR_INTERNAL, MSG_HEADER
				"ioctl: invalid argument!");
		else if (errno == EIO)
			h5_error(f, H5_ERR_INTERNAL, MSG_HEADER
				"ioctl: physical I/O problem!");
		else if (errno == ENOTTY)
			h5_error(f, H5_ERR_INTERNAL, MSG_HEADER
				"ioctl: file handle does not accept control functions!");
		else if (errno == ENODEV)
			h5_error(f, H5_ERR_INTERNAL, MSG_HEADER
				"ioctl: driver doesn't support control functions!");
		else
			h5_error(f, H5_ERR_INTERNAL, MSG_HEADER
				"ioctl: unspecific error!");
		return -1;
	}

	close(fd);

	if ( f->myproc == 0 && h5_get_debuglevel() >= 5 )
		_print_stripe_info(lum);

	ssize_t stripe_size = (ssize_t)lum->lmm_stripe_size;
	free(lum);

	return stripe_size;
}

herr_t
h5_optimize_for_lustre (
	h5_file_t *const f,
	const char *filename
	) {

	ssize_t stripe_size;
	if ( f->myproc == 0 )
	{
		char *path = malloc(strlen(filename)+4);
		strcpy(path, filename);
		/* check for existing file */
		FILE *test = fopen(path, "r");
		if (!test) {
			/* use directory as path */
			int i = strlen(path)-1;
			while (i >= 0) {
			   if (path[i] != '/') path[i] = '\0'; 
			   else break;
			   i--;
			}
			if (strlen(path) == 0) sprintf(path, ".");
		}
		else fclose(test);

		stripe_size = _get_lustre_stripe_size(path);

		free(path);
	}

	TRY( h5priv_mpi_bcast(f, &stripe_size, 1, MPI_LONG_LONG, 0, f->comm) );
	h5_info(f, "Found lustre stripe size of %lld bytes", (long long)stripe_size);

	hsize_t btree_ik = (stripe_size - 4096) / 96;
	hsize_t btree_bytes = 64 + 96*btree_ik;
	h5_info(f,
		"Setting HDF5 btree ik to %lld (= %lld bytes at rank 3)",
		(long long)btree_ik, (long long)btree_bytes);
	TRY( h5priv_set_hdf5_btree_ik_property(f, f->create_prop, btree_ik) );

	/* set alignment to lustre stripe size */
	TRY( h5priv_set_hdf5_alignment_property(f,
				f->access_prop, 0, stripe_size) );

	if (_verbose) {
		fprintf(MSG_STREAM, MSG_HEADER "disabling metadata cache flushes.\n");
	}
	/* disable metadata cache flushes */
	/* defer metadata writes */
	H5AC_cache_config_t config;
	config.version = H5AC__CURR_CACHE_CONFIG_VERSION;
	TRY( H5Pget_mdc_config( fapl_id, &config ) );
	config.set_initial_size = 1;
	config.initial_size = 16 * 1024 * 1024;
	config.evictions_enabled = 0;
	config.incr_mode = H5C_incr__off;
	config.decr_mode = H5C_decr__off;
	config.flash_incr_mode = H5C_flash_incr__off;
	TRY( H5Pset_mdc_config( fapl_id, &config ) );

	return H5_SUCCESS;
}

#endif // H5_USE_LUSTRE