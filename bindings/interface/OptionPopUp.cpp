#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <interface/OptionPopUp.h>
#include <OptionControl.h>

namespace py = pybind11;


PYBIND11_MODULE(OptionPopUp,m)
{
py::class_<BOptionPopUp, BOptionControl>(m, "BOptionPopUp")
.def(py::init<BRect, const char *, const char *, BMessage *, unsigned int, unsigned int>(), "", py::arg("frame"), py::arg("name"), py::arg("label"), py::arg("message"), py::arg("resizeMask")=B_FOLLOW_LEFT_TOP, py::arg("flags")=B_WILL_DRAW)
.def(py::init<BRect, const char *, const char *, BMessage *, bool, unsigned int, unsigned int>(), "", py::arg("frame"), py::arg("name"), py::arg("label"), py::arg("message"), py::arg("fixed"), py::arg("resizeMask")=B_FOLLOW_LEFT_TOP, py::arg("flags")=B_WILL_DRAW)
.def(py::init<const char *, const char *, BMessage *, unsigned int>(), "", py::arg("name"), py::arg("label"), py::arg("message"), py::arg("flags")=B_WILL_DRAW)
.def("MenuField", &BOptionPopUp::MenuField, "")
// FIXED (2026-10-25): const char**/int32* out-params; now (name, value, ok).
.def("GetOptionAt", [](BOptionPopUp& self, int32 index) -> std::tuple<std::string, int32, bool> {
	const char* name = nullptr;
	int32 value = 0;
	bool ok = self.GetOptionAt(index, &name, &value);
	return std::make_tuple(ok ? std::string(name ? name : "") : std::string(), value, ok);
}, "", py::arg("index"))
.def("RemoveOptionAt", &BOptionPopUp::RemoveOptionAt, "", py::arg("index"))
.def("CountOptions", &BOptionPopUp::CountOptions, "")
.def("AddOptionAt", &BOptionPopUp::AddOptionAt, "", py::arg("name"), py::arg("value"), py::arg("index"))
.def("AllAttached", &BOptionPopUp::AllAttached, "")
.def("AttachedToWindow", &BOptionPopUp::AttachedToWindow, "")
.def("MessageReceived", &BOptionPopUp::MessageReceived, "", py::arg("message"))
.def("SetLabel", &BOptionPopUp::SetLabel, "", py::arg("text"))
.def("SetValue", &BOptionPopUp::SetValue, "", py::arg("value"))
.def("SetEnabled", &BOptionPopUp::SetEnabled, "", py::arg("enabled"))
// FIXED (2026-10-25): float* out-params were NULL-deref hazards from Python;
// now returns (width, height).
.def("GetPreferredSize", [](BOptionPopUp& self) -> std::tuple<float, float> {
	float width = 0;
	float height = 0;
	self.GetPreferredSize(&width, &height);
	return std::make_tuple(width, height);
}, "")
.def("ResizeToPreferred", &BOptionPopUp::ResizeToPreferred, "")
// FIXED (2026-10-25): const char**/int32* out-params; now (value, name, status).
.def("SelectedOption", [](const BOptionPopUp& self) -> std::tuple<int32, std::string, bool> {
	const char* name = nullptr;
	int32 value = 0;
	bool ok = self.SelectedOption(&name, &value);
	return std::make_tuple(value, ok ? std::string(name ? name : "") : std::string(), ok);
}, "")
;


}
