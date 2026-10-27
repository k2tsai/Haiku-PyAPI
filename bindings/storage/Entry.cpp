#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <Entry.h>

#include <SupportDefs.h>
#include <Statable.h>

#include <Directory.h>
#include <Path.h>

namespace py = pybind11;


PYBIND11_MODULE(Entry,m)
{
py::class_<entry_ref>(m, "entry_ref")
.def(py::init(), "")
.def(py::init<dev_t, ino_t, const char *>(), "", py::arg("dev"), py::arg("dir"), py::arg("name"))
.def(py::init<const entry_ref &>(), "", py::arg("ref"))
.def("set_name", &entry_ref::set_name, "", py::arg("name"))
.def("__eq__", &entry_ref::operator==, "", py::arg("ref"))
.def("__ne__", &entry_ref::operator!=, "", py::arg("ref"))
.def("__copy__", &entry_ref::operator=, "", py::arg("ref"))
.def_readwrite("device", &entry_ref::device, "")
.def_readwrite("directory", &entry_ref::directory, "")
.def_readwrite("name", &entry_ref::name, "")
;

py::class_<BEntry, BStatable>(m, "BEntry")
.def(py::init(), "")
.def(py::init<const BDirectory *, const char *, bool>(), "", py::arg("dir"), py::arg("path"), py::arg("traverse")=false)
.def(py::init<const entry_ref *, bool>(), "", py::arg("ref"), py::arg("traverse")=false)
.def(py::init<const char *, bool>(), "", py::arg("path"), py::arg("traverse")=false)
.def(py::init<const BEntry &>(), "", py::arg("entry"))
.def("InitCheck", &BEntry::InitCheck, "")
.def("Exists", &BEntry::Exists, "")
.def("Name", &BEntry::Name, "")
// FIXED (2026-10-25): beta6 BEntry::GetStat(struct stat*) (Entry.h:55) is an
// out-param. The raw binding null-derefs on None. No-arg form added with an
// internal stat buffer -> (stat, status); the stat object-arg form (safe
// borrow, stat module loaded) is kept as an arity-distinguished overload.
.def("GetStat", [](BEntry& self)->py::tuple{
	struct stat st = {};
	status_t ret = self.GetStat(&st);
	return py::make_tuple(st, ret);
},"")
.def("GetStat", &BEntry::GetStat, "", py::arg("stat"))
.def("SetTo", py::overload_cast<const BDirectory *, const char *, bool>(&BEntry::SetTo), "", py::arg("dir"), py::arg("path"), py::arg("traverse")=false)
.def("SetTo", py::overload_cast<const entry_ref *, bool>(&BEntry::SetTo), "", py::arg("ref"), py::arg("traverse")=false)
.def("SetTo", py::overload_cast<const char *, bool>(&BEntry::SetTo), "", py::arg("path"), py::arg("traverse")=false)
.def("Unset", &BEntry::Unset, "")
// FIXED (2026-10-25): GetRef(entry_ref*) / GetPath(BPath*) (Entry.h:64-65)
// are out-params. No-arg forms added with an internal object -> (obj, status)
// (2026-10-25 Node.cpp pattern); the object-arg forms (safe borrows) are kept.
.def("GetRef", [](BEntry& self)->py::tuple{
	entry_ref ref;
	status_t ret = self.GetRef(&ref);
	return py::make_tuple(ref, ret);
},"")
.def("GetRef", &BEntry::GetRef, "",py::arg("ref"))
.def("GetPath", [](BEntry& self)->py::tuple{
	BPath path;
	status_t ret = self.GetPath(&path);
	return py::make_tuple(path, ret);
},"")
.def("GetPath", &BEntry::GetPath, "",py::arg("path"))
.def("GetParent", py::overload_cast<BEntry *>(&BEntry::GetParent, py::const_), "",py::arg("entry"))
.def("GetParent", py::overload_cast<BDirectory *>(&BEntry::GetParent, py::const_), "",py::arg("dir"))
//.def("GetName", &BEntry::GetName, "", py::arg("buffer"))
// FIXED (2026-10-25): internal-buffer lambda (was raw char* out-param);
// return shape normalized to the standard (value, status) — (name, status).
.def("GetName", [](BEntry& self)->py::tuple {
	char name[B_FILE_NAME_LENGTH];
	status_t ret = self.GetName(name);
	return py::make_tuple(ret == B_OK ? py::str(name) : py::str(), ret);
},"")
.def("Rename", &BEntry::Rename, "", py::arg("path"), py::arg("clobber")=false)
// FIXED (2026-10-25): same NULL-default break as BPath (Entry.h MoveTo path
// = NULL): split 1-arg / full-arg forms so e.MoveTo(dir) works.
.def("MoveTo", [](BEntry& self, BDirectory* dir) { return self.MoveTo(dir); }, "", py::arg("dir"))
.def("MoveTo", [](BEntry& self, BDirectory* dir, const char* path, bool clobber) { return self.MoveTo(dir, path, clobber); }, "", py::arg("dir"), py::arg("path"), py::arg("clobber")=false)
.def("Remove", &BEntry::Remove, "")
.def("__eq__", &BEntry::operator==, "", py::arg("item"))
.def("__ne__", &BEntry::operator!=, "", py::arg("item"))
.def("__copy__", &BEntry::operator=, "", py::arg("item"))
;

m.def("get_ref_for_path", &get_ref_for_path, "", py::arg("path"), py::arg("ref"));

m.def("__lt__", &operator<, "", py::arg("a"), py::arg("b"));

}
