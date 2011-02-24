#include "h5core/h5_core.h"
#include "h5_core_private.h"

const char*
h5tpriv_oid_names[] = {
	"N.N.",
	"vertex",
	"edge",
	"triangle",
	"tetrahedron"
};

const char*
h5tpriv_meshes_grpnames[] = {
	"N.N.",
	"N.N.",
	"N.N.",
	"TriangleMeshes",
	"TetMeshes"
};

const char*
h5tpriv_map_oid2str (
	h5_oid_t oid
	) {
	if ((oid < 0) || (oid >= sizeof (h5tpriv_oid_names) / sizeof (char*))) {
		return "[invalid oid]";
	}
	return h5tpriv_oid_names[oid];
}