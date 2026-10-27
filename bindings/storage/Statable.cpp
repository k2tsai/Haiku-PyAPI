#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <Statable.h>
#include <Node.h>
#include <Volume.h>

namespace py = pybind11;
/*using namespace BPrivate;
using namespace BPrivate::Storage;
using namespace BPrivate::Storage::Mime;
using namespace BPackageKit;*/

// FIXED (2026-10-25): beta6's BStatable getters (os/storage/Statable.h) all
// take a NON-CONST trailing pointer out-param: GetStat(stat*), GetSize(off_t*),
// GetOwner(uid_t*), GetGroup(gid_t*), GetPermissions(mode_t*),
// Get{Access,Creation,Modification}Time(time_t*), GetNodeRef(node_ref*),
// GetVolume(BVolume*). The raw bindings null-deref on None and (for
// GetSize etc.) TypeError on anything else. Rebound with internal buffers,
// no args, tuple return:
//   numeric  -> (status, value)   [GetSize pinned (status,size) in
//                                  test_storage.py, matching BFile::GetSize]
//   objects  -> (value, status)   [2026-10-25 Node.cpp pattern]
// The old raw GetStat(stat-object) form is kept where a registered object
// type exists (stat), as an arity-distinguished overload.
PYBIND11_MODULE(Statable, m)
{
py::class_<BStatable>(m, "BStatable")
.def("GetStat", [](BStatable& self)->py::tuple{
	struct stat st = {};
	status_t ret = self.GetStat(&st);
	return py::make_tuple(st, ret);
},"")
.def("GetStat", &BStatable::GetStat, "", py::arg("stat"))
.def("IsFile", &BStatable::IsFile, "")
.def("IsDirectory", &BStatable::IsDirectory, "")
.def("IsSymLink", &BStatable::IsSymLink, "")
.def("GetNodeRef", [](BStatable& self)->py::tuple{
	node_ref ref;
	status_t ret = self.GetNodeRef(&ref);
	return py::make_tuple(ref, ret);
},"")
.def("GetOwner", [](BStatable& self)->py::tuple{
	uid_t owner = 0;
	status_t ret = self.GetOwner(&owner);
	return py::make_tuple((int)owner, ret);
},"")
.def("SetOwner", &BStatable::SetOwner, "", py::arg("owner"))
.def("GetGroup", [](BStatable& self)->py::tuple{
	gid_t group = 0;
	status_t ret = self.GetGroup(&group);
	return py::make_tuple((int)group, ret);
},"")
.def("SetGroup", &BStatable::SetGroup, "", py::arg("group"))
.def("GetPermissions", [](BStatable& self)->py::tuple{
	mode_t permissions = 0;
	status_t ret = self.GetPermissions(&permissions);
	return py::make_tuple((int)permissions, ret);
},"")
.def("SetPermissions", &BStatable::SetPermissions, "", py::arg("permissions"))
.def("GetSize", [](BStatable& self)->py::tuple{
	off_t size = 0;
	status_t ret = self.GetSize(&size);
	return py::make_tuple(ret, size);
},"")
.def("GetModificationTime", [](BStatable& self)->py::tuple{
	time_t mtime = 0;
	status_t ret = self.GetModificationTime(&mtime);
	return py::make_tuple((int)mtime, ret);
},"")
.def("SetModificationTime", &BStatable::SetModificationTime, "", py::arg("mtime"))
.def("GetCreationTime", [](BStatable& self)->py::tuple{
	time_t ctime = 0;
	status_t ret = self.GetCreationTime(&ctime);
	return py::make_tuple((int)ctime, ret);
},"")
.def("SetCreationTime", &BStatable::SetCreationTime, "", py::arg("ctime"))
.def("GetAccessTime", [](BStatable& self)->py::tuple{
	time_t atime = 0;
	status_t ret = self.GetAccessTime(&atime);
	return py::make_tuple((int)atime, ret);
},"")
.def("SetAccessTime", &BStatable::SetAccessTime, "", py::arg("atime"))
.def("GetVolume", [](BStatable& self)->py::tuple{
	BVolume volume;
	status_t ret = self.GetVolume(&volume);
	return py::make_tuple(volume, ret);
},"")
//.def_readwrite("Private", &BStatable::Private, "")
;


}
