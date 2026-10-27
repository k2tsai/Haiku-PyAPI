#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <interface/Deskbar.h>
#include <Rect.h>
#include <View.h>

namespace py = pybind11;


PYBIND11_MODULE(Deskbar,m)
{
py::enum_<deskbar_location>(m, "deskbar_location", "")
.value("B_DESKBAR_TOP", deskbar_location::B_DESKBAR_TOP, "")
.value("B_DESKBAR_BOTTOM", deskbar_location::B_DESKBAR_BOTTOM, "")
.value("B_DESKBAR_LEFT_TOP", deskbar_location::B_DESKBAR_LEFT_TOP, "")
.value("B_DESKBAR_RIGHT_TOP", deskbar_location::B_DESKBAR_RIGHT_TOP, "")
.value("B_DESKBAR_LEFT_BOTTOM", deskbar_location::B_DESKBAR_LEFT_BOTTOM, "")
.value("B_DESKBAR_RIGHT_BOTTOM", deskbar_location::B_DESKBAR_RIGHT_BOTTOM, "")
.export_values();

py::class_<BDeskbar>(m, "BDeskbar")
.def(py::init(), "")
.def("IsRunning", &BDeskbar::IsRunning, "")
.def("Frame", &BDeskbar::Frame, "")
// FIXED (2026-10-25): bool* _isExpanded out-param; now (location, expanded).
.def("Location", [](const BDeskbar& self) -> std::tuple<deskbar_location, bool> {
	bool isExpanded = false;
	deskbar_location location = self.Location(&isExpanded);
	return std::make_tuple(location, isExpanded);
}, "")
.def("SetLocation", &BDeskbar::SetLocation, "", py::arg("location"), py::arg("expanded")=false)
.def("IsExpanded", &BDeskbar::IsExpanded, "")
.def("Expand", &BDeskbar::Expand, "", py::arg("expand"))
.def("IsAlwaysOnTop", &BDeskbar::IsAlwaysOnTop, "")
.def("SetAlwaysOnTop", &BDeskbar::SetAlwaysOnTop, "", py::arg("alwaysOnTop"))
.def("IsAutoRaise", &BDeskbar::IsAutoRaise, "")
.def("SetAutoRaise", &BDeskbar::SetAutoRaise, "", py::arg("autoRaise"))
.def("IsAutoHide", &BDeskbar::IsAutoHide, "")
.def("SetAutoHide", &BDeskbar::SetAutoHide, "", py::arg("autoHide"))
// FIXED (2026-10-25): const char**/int32* out-params; now tuples
// (Deskbar.h:54-55).
.def("GetItemInfo", [](const BDeskbar& self, int32 id) -> std::tuple<std::string, status_t> {
	const char* name = nullptr;
	status_t result = self.GetItemInfo(id, &name);
	return std::make_tuple(result == B_OK ? std::string(name ? name : "") : std::string(), result);
}, "", py::arg("id"))
.def("GetItemInfo", [](const BDeskbar& self, const char* name) -> std::tuple<int32, status_t> {
	int32 id = 0;
	status_t result = self.GetItemInfo(name, &id);
	return std::make_tuple(id, result);
}, "", py::arg("name"))
.def("HasItem", py::overload_cast<int32>(&BDeskbar::HasItem, py::const_), "", py::arg("id"))
.def("HasItem", py::overload_cast<const char *>(&BDeskbar::HasItem, py::const_), "", py::arg("name"))
.def("CountItems", &BDeskbar::CountItems, "")
.def("MaxItemWidth", &BDeskbar::MaxItemWidth, "")
.def("MaxItemHeight", &BDeskbar::MaxItemHeight, "")
// FIXED (2026-10-25): int32* _id out-param; now (id, status) tuples.
.def("AddItem", [](BDeskbar& self, BView* archivableView) -> std::tuple<int32, status_t> {
	int32 id = 0;
	status_t result = self.AddItem(archivableView, &id);
	return std::make_tuple(id, result);
}, "", py::arg("archivableView"))
.def("AddItem", [](BDeskbar& self, entry_ref& addOn) -> std::tuple<int32, status_t> {
	int32 id = 0;
	status_t result = self.AddItem(&addOn, &id);
	return std::make_tuple(id, result);
}, "", py::arg("addOn"))
.def("RemoveItem", py::overload_cast<int32>(&BDeskbar::RemoveItem), "", py::arg("id"))
.def("RemoveItem", py::overload_cast<const char *>(&BDeskbar::RemoveItem), "", py::arg("name"))
;


}
