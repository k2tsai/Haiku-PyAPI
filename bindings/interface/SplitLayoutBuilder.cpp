#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <interface/SplitLayoutBuilder.h>
#include <SplitView.h>

namespace py = pybind11;


PYBIND11_MODULE(SplitLayoutBuilder,m)
{
py::class_<BSplitLayoutBuilder>(m, "BSplitLayoutBuilder")
.def(py::init<orientation, float>(), "", py::arg("orientation")=B_HORIZONTAL, py::arg("spacing")=B_USE_DEFAULT_SPACING)
.def(py::init<BSplitView *>(), "", py::arg("view"))
// FIXED (2026-10-25): the builder OWNS the BSplitView; the raw bindings
// returned it with pybind's automatic (take-ownership) policy, so the Python
// side would delete a C++ object still alive in the builder. Now a reference
// return (no ownership transfer); GetSplitView no longer takes the
// BSplitView** out-param.
.def("SplitView", [](const BSplitLayoutBuilder& self) -> py::object {
	return py::cast(self.SplitView(), py::return_value_policy::reference);
}, "")
.def("GetSplitView", [](const BSplitLayoutBuilder& self) -> py::object {
	return py::cast(self.SplitView(), py::return_value_policy::reference);
}, "")
.def("Add", py::overload_cast<BView *>(&BSplitLayoutBuilder::Add), "", py::arg("view"))
.def("Add", py::overload_cast<BView *, float>(&BSplitLayoutBuilder::Add), "", py::arg("view"), py::arg("weight"))
.def("Add", py::overload_cast<BLayoutItem *>(&BSplitLayoutBuilder::Add), "", py::arg("item"))
.def("Add", py::overload_cast<BLayoutItem *, float>(&BSplitLayoutBuilder::Add), "", py::arg("item"), py::arg("weight"))
.def("SetCollapsible", &BSplitLayoutBuilder::SetCollapsible, "", py::arg("collapsible"))
.def("SetInsets", &BSplitLayoutBuilder::SetInsets, "", py::arg("left"), py::arg("top"), py::arg("right"), py::arg("bottom"))
.def("operatorBSplitView*", &BSplitLayoutBuilder::operatorBSplitView*, "")
;


}
