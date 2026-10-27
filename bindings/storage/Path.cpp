#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <Path.h>
#include <Directory.h>

namespace py = pybind11;
/*using namespace BPrivate;
using namespace BPrivate::Storage;
using namespace BPrivate::Storage::Mime;*/

class PyBPath : public BPath{
	public:
        using BPath::BPath;
        bool			IsFixedSize() const override {
        	PYBIND11_OVERLOAD(bool, BPath, IsFixedSize);
        }
        type_code		TypeCode() const override {
        	PYBIND11_OVERLOAD(type_code, BPath, TypeCode);
        }
        ssize_t			FlattenedSize() const override {
        	PYBIND11_OVERLOAD(ssize_t, BPath, FlattenedSize);
        }
        status_t		Flatten(void* buffer, ssize_t size) const override {
        	PYBIND11_OVERLOAD(status_t, BPath, Flatten, buffer, size);
        }
        bool			AllowsTypeCode(type_code code) const override {
        	PYBIND11_OVERLOAD(bool, BPath, AllowsTypeCode, code);
        }
        status_t		Unflatten(type_code code, const void* buffer, ssize_t size) override {
        	PYBIND11_OVERLOAD(status_t, BPath, Unflatten, code, buffer, size);
        }
};

PYBIND11_MODULE(Path, m)
{
py::class_<BPath, BFlattenable>(m, "BPath")
.def(py::init(), "")
.def(py::init<const BPath &>(), "", py::arg("path"))
.def(py::init<const entry_ref *>(), "", py::arg("ref"))
.def(py::init<const BEntry *>(), "", py::arg("entry"))
// FIXED (2026-10-25): the `py::arg("leaf")=NULL` defaults broke single-arg
// calls (BPath(path), p.SetTo(path)): pybind11 stored the nullptr default as
// int 0, which the const char* caster rejects -> TypeError. Split into a
// 1-arg form (C++ default args fill leaf/normalize) and a full-arg form.
.def(py::init<const char *>(), "", py::arg("path"))
.def(py::init<const char *, const char *, bool>(), "", py::arg("path"), py::arg("leaf"), py::arg("normalize")=false)
.def(py::init<const BDirectory *>(), "", py::arg("dir"))
.def(py::init<const BDirectory *, const char *, bool>(), "", py::arg("dir"), py::arg("leaf"), py::arg("normalize")=false)
.def("InitCheck", &BPath::InitCheck, "")
.def("SetTo", py::overload_cast<const entry_ref *>(&BPath::SetTo), "", py::arg("ref"))
.def("SetTo", py::overload_cast<const BEntry *>(&BPath::SetTo), "", py::arg("entry"))
.def("SetTo", [](BPath& self, const char* path) { return self.SetTo(path); }, "", py::arg("path"))
.def("SetTo", [](BPath& self, const char* path, const char* leaf, bool normalize) { return self.SetTo(path, leaf, normalize); }, "", py::arg("path"), py::arg("leaf"), py::arg("normalize")=false)
.def("SetTo", [](BPath& self, const BDirectory* dir) { return self.SetTo(dir); }, "", py::arg("dir"))
.def("SetTo", [](BPath& self, const BDirectory* dir, const char* leaf, bool normalize) { return self.SetTo(dir, leaf, normalize); }, "", py::arg("dir"), py::arg("leaf"), py::arg("normalize")=false)
.def("Unset", &BPath::Unset, "")
.def("Append", &BPath::Append, "", py::arg("path"), py::arg("normalize")=false)
.def("Path", &BPath::Path, "")
.def("Leaf", &BPath::Leaf, "")
.def("GetParent", &BPath::GetParent, "", py::arg("path"))
.def("IsAbsolute", &BPath::IsAbsolute, "")
.def("__eq__", py::overload_cast<const BPath &>(&BPath::operator==, py::const_), "", py::arg("item"))
.def("__eq__", py::overload_cast<const char *>(&BPath::operator==, py::const_), "", py::arg("path"))
.def("__ne__", py::overload_cast<const BPath &>(&BPath::operator!=, py::const_), "", py::arg("item"))
.def("__ne__", py::overload_cast<const char *>(&BPath::operator!=, py::const_), "", py::arg("path"))
.def("__copy__", py::overload_cast<const BPath &>(&BPath::operator=), "", py::arg("item"))
.def("__copy__", py::overload_cast<const char *>(&BPath::operator=), "", py::arg("path"))
.def("IsFixedSize", &BPath::IsFixedSize, "")
.def("TypeCode", &BPath::TypeCode, "")
.def("FlattenedSize", &BPath::FlattenedSize, "")
.def("Flatten", &BPath::Flatten, "", py::arg("buffer"), py::arg("size"))
.def("AllowsTypeCode", &BPath::AllowsTypeCode, "", py::arg("code"))
.def("Unflatten", &BPath::Unflatten, "", py::arg("code"), py::arg("buffer"), py::arg("size"))
;


}
