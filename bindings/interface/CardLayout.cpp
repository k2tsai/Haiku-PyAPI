#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <interface/CardLayout.h>
#include <AbstractLayout.h>

namespace py = pybind11;


PYBIND11_MODULE(CardLayout,m)
{
py::class_<BCardLayout, BAbstractLayout>(m, "BCardLayout")
.def(py::init(), "")
.def(py::init<BMessage *>(), "", py::arg("from"))
.def("VisibleItem", &BCardLayout::VisibleItem, "")
.def("VisibleIndex", &BCardLayout::VisibleIndex, "")
.def("SetVisibleItem", py::overload_cast<int32>(&BCardLayout::SetVisibleItem), "", py::arg("index"))
.def("SetVisibleItem", py::overload_cast<BLayoutItem *>(&BCardLayout::SetVisibleItem), "", py::arg("item"))
.def("BaseMinSize", &BCardLayout::BaseMinSize, "")
.def("BaseMaxSize", &BCardLayout::BaseMaxSize, "")
.def("BasePreferredSize", &BCardLayout::BasePreferredSize, "")
.def("BaseAlignment", &BCardLayout::BaseAlignment, "")
.def("HasHeightForWidth", &BCardLayout::HasHeightForWidth, "")
// FIXED (2026-10-25): float* out-params (NULL-deref from Python); now a
// (min, max, preferred) tuple.
.def("GetHeightForWidth", [](BCardLayout& self, float width) -> std::tuple<float, float, float> {
	float min = 0;
	float max = 0;
	float preferred = 0;
	self.GetHeightForWidth(width, &min, &max, &preferred);
	return std::make_tuple(min, max, preferred);
}, "", py::arg("width"))
.def("Archive", &BCardLayout::Archive, "", py::arg("into"), py::arg("deep")=true)
.def_static("Instantiate", &BCardLayout::Instantiate, "", py::arg("from"))
.def("Perform", &BCardLayout::Perform, "", py::arg("d"), py::arg("arg"))
;


}
