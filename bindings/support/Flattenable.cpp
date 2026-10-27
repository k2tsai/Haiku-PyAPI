#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <support/Flattenable.h>

namespace py = pybind11;


PYBIND11_MODULE(Flattenable, m)
{
// BFlattenable is abstract (Flatten/Unflatten/FlattenedSize are pure virtual)
// and stays a plain pybase — subclasses (BStringList, BPath, BUrl via
// BArchivable, BAffineTransform...) register on top of it.
py::class_<BFlattenable>(m, "BFlattenable")
.def("IsFixedSize", &BFlattenable::IsFixedSize, "")
.def("TypeCode", &BFlattenable::TypeCode, "")
.def("FlattenedSize", &BFlattenable::FlattenedSize, "")
// beta6 (Flattenable.h:16-19): Flatten(void* buffer, ssize_t size) /
// Unflatten(type_code, const void*, ssize_t) write/read caller storage —
// the raw void* forms are never bound. Safe form: allocate via
// FlattenedSize() and return (status, bytes).
.def("Flatten", [](const BFlattenable& self) {
	ssize_t size = self.FlattenedSize();
	std::vector<char> buffer(size > 0 ? size : 1);
	status_t status = self.Flatten(buffer.data(), size);
	return std::make_tuple(status, py::bytes(buffer.data(), size));
},"")
.def("AllowsTypeCode", &BFlattenable::AllowsTypeCode, "", py::arg("code"))
.def("Unflatten", [](BFlattenable& self, type_code code, py::bytes data) {
	py::buffer buf(data);
	py::buffer_info info = buf.request();
	status_t status = self.Unflatten(code, info.ptr, (ssize_t)info.size);
	return status; }, "", py::arg("code"), py::arg("data"))
;


}
