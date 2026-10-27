#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <PathFinder.h>
#include <FindDirectory.h>
#include <package/PackageResolvableExpression.h>
#include <Path.h>

namespace py = pybind11;
//using namespace BPrivate;
//using namespace BPrivate::Storage;
//using namespace BPrivate::Storage::Mime;
//using namespace BPackageKit;

class BPathFinderWrapper {
public:
    static std::tuple<status_t, BStringList> FindPaths(const char* architecture, path_base_directory baseDirectory, const char* subPath, uint32 flags, BStringList& _paths) {
        //BStringList _paths;
        status_t result = BPathFinder::FindPaths(architecture, baseDirectory, subPath, flags, _paths);
        return std::make_tuple(result, _paths);
    }
    static std::tuple<status_t, BStringList> FindPaths(path_base_directory baseDirectory, const char* subPath, uint32 flags, BStringList& _paths) {
        //BStringList _paths;
        status_t result = BPathFinder::FindPaths(baseDirectory, subPath, flags, _paths);
        return std::make_tuple(result, _paths);
    }
    static std::tuple<status_t, BStringList> FindPaths(path_base_directory baseDirectory, const char* subPath, BStringList& _paths) {
        //BStringList _paths;
        status_t result = BPathFinder::FindPaths(baseDirectory, subPath, _paths);
        return std::make_tuple(result, _paths);
    }
    static std::tuple<status_t, BStringList> FindPaths(path_base_directory baseDirectory, BStringList& _paths) {
        //BStringList _paths;
        status_t result = BPathFinder::FindPaths(baseDirectory, _paths);
        return std::make_tuple(result, _paths);
    }
};


PYBIND11_MODULE(PathFinder, m)
{
//m.attr("BPackageResolvableExpression") = py::cast(BPackageResolvableExpression);

py::class_<BPathFinder>(m, "BPathFinder")
// FIXED (2026-10-25): `py::arg("dependency")=NULL` (const char* default)
// broke the common short forms — BPathFinder(path), pf.SetTo(path): the
// nullptr default is stored as int 0, which the const char* caster rejects
// -> TypeError. Split into short forms (C++ defaults fill dependency) and
// full forms. Same fix as Path.cpp.
.def(py::init<>(), "")
.def(py::init<const void *>(), "", py::arg("codePointer"))
.def(py::init<const void *, const char *>(), "", py::arg("codePointer"), py::arg("dependency"))
.def(py::init<const char *>(), "", py::arg("path"))
.def(py::init<const char *, const char *>(), "", py::arg("path"), py::arg("dependency"))
.def(py::init<const entry_ref &>(), "", py::arg("ref"))
.def(py::init<const entry_ref &, const char *>(), "", py::arg("ref"), py::arg("dependency"))
//.def(py::init<const BPackageKit::BPackageResolvableExpression &, const char *>(), "", py::arg("expression"), py::arg("dependency")=NULL) //BResolvableExpression
.def("SetTo", [](BPathFinder& self) { return self.SetTo((const void*)NULL); }, "")
.def("SetTo", [](BPathFinder& self, const void* codePointer) { return self.SetTo(codePointer); }, "", py::arg("codePointer"))
.def("SetTo", [](BPathFinder& self, const void* codePointer, const char* dependency) { return self.SetTo(codePointer, dependency); }, "", py::arg("codePointer"), py::arg("dependency"))
.def("SetTo", [](BPathFinder& self, const char* path) { return self.SetTo(path); }, "", py::arg("path"))
.def("SetTo", [](BPathFinder& self, const char* path, const char* dependency) { return self.SetTo(path, dependency); }, "", py::arg("path"), py::arg("dependency"))
.def("SetTo", [](BPathFinder& self, const entry_ref& ref) { return self.SetTo(ref); }, "", py::arg("ref"))
.def("SetTo", [](BPathFinder& self, const entry_ref& ref, const char* dependency) { return self.SetTo(ref, dependency); }, "", py::arg("ref"), py::arg("dependency"))
//.def("SetTo", py::overload_cast<const BPackageKit::BPackageResolvableExpression &, const char *>(&BPathFinder::SetTo), "", py::arg("expression"), py::arg("dependency")=NULL) //BResolvableExpression
.def("FindPath", [](BPathFinder& self,const char * architecture,path_base_directory baseDirectory,const char * subPath,unsigned int flags) {
    BPath  _path;
    status_t r = self.FindPath(architecture, baseDirectory, subPath, flags, _path);
    return std::make_tuple(r,_path);
}
, "", py::arg("architecture"), py::arg("baseDirectory"), py::arg("subPath"), py::arg("flags"))
.def("FindPath", [](BPathFinder& self,path_base_directory baseDirectory,const char * subPath,unsigned int flags) {
    BPath  _path;
    status_t r = self.FindPath(baseDirectory, subPath, flags, _path);
    return std::make_tuple(r,_path);
}
, "", py::arg("baseDirectory"), py::arg("subPath"), py::arg("flags"))
.def("FindPath", [](BPathFinder& self,path_base_directory baseDirectory,const char * subPath) {
    BPath  _path;
    status_t r = self.FindPath(baseDirectory, subPath, _path);
    return std::make_tuple(r,_path);
}
, "", py::arg("baseDirectory"), py::arg("subPath"))
.def("FindPath", [](BPathFinder& self,path_base_directory baseDirectory) {
    BPath  _path;
    status_t r = self.FindPath(baseDirectory, _path);
    return std::make_tuple(r,_path);
}
, "", py::arg("baseDirectory"))
/*.def_static("FindPaths", [](BPathFinder& self,const char * architecture,path_base_directory baseDirectory,const char * subPath,uint32 flags) {
    BStringList  _paths;
    status_t r = self.FindPaths(architecture, baseDirectory, subPath, flags, _paths);
    return std::make_tuple(r,_paths);
}
, "", py::arg("architecture"), py::arg("baseDirectory"), py::arg("subPath"), py::arg("flags"))*/
.def_static("FindPaths", py::overload_cast<const char*, path_base_directory, const char*, uint32, BStringList&>(&BPathFinderWrapper::FindPaths), "", py::arg("architecture"), py::arg("baseDirectory"), py::arg("subPath"), py::arg("flags"), py::arg("_paths"))
/*.def("FindPaths", [](BPathFinder& self,path_base_directory baseDirectory,const char * subPath,uint32 flags) {
    BStringList  _paths;
    status_t r = self.FindPaths(baseDirectory, subPath, flags, _paths);
    return std::make_tuple(r,_paths);
}
, "", py::arg("baseDirectory"), py::arg("subPath"), py::arg("flags"))*/
.def_static("FindPaths", py::overload_cast<path_base_directory,const char*, uint32, BStringList&>(&BPathFinderWrapper::FindPaths), "", py::arg("baseDirectory"), py::arg("subPath"), py::arg("flags"), py::arg("_paths"))
/*.def("FindPaths", [](BPathFinder& self,path_base_directory baseDirectory,const char * subPath) {
    BStringList  _paths;
    status_t r = self.FindPaths(baseDirectory, subPath, _paths);
    return std::make_tuple(r,_paths);
}
, "", py::arg("baseDirectory"), py::arg("subPath"))*/
.def_static("FindPaths", py::overload_cast<path_base_directory, const char*, BStringList&>(&BPathFinderWrapper::FindPaths), "", py::arg("baseDirectory"), py::arg("subPath"), py::arg("_paths"))
/*.def("FindPaths", [](BPathFinder& self,path_base_directory baseDirectory) {
    BStringList  _paths;
    status_t r = self.FindPaths(baseDirectory, _paths);
    return std::make_tuple(r,_paths);
}
, "", py::arg("baseDirectory"))*/
.def_static("FindPaths", py::overload_cast<path_base_directory, BStringList&>(&BPathFinderWrapper::FindPaths), "", py::arg("baseDirectory"), py::arg("_paths"))
;

}
