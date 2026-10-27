#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <VolumeRoster.h>

namespace py = pybind11;
using namespace BPrivate;
using namespace BPrivate::Storage;
using namespace BPrivate::Storage::Mime;
//using namespace BPackageKit;

PYBIND11_MODULE(VolumeRoster, m)
{
py::class_<BVolumeRoster>(m, "BVolumeRoster")
.def(py::init(), "")
.def("GetNextVolume", &BVolumeRoster::GetNextVolume, "", py::arg("volume"))
.def("Rewind", &BVolumeRoster::Rewind, "")
.def("GetBootVolume", &BVolumeRoster::GetBootVolume, "", py::arg("volume"))
// FIXED (2026-10-25): `py::arg("messenger")=be_app_messenger` made pybind11
// convert the BMessenger default at REGISTRATION time — BMessenger is
// registered by the Messenger module, which stock __init__ imports AFTER
// VolumeRoster, so the full `import Be` crashed here ("type not registered
// yet?"). No Python-side default: the no-arg overload lets the C++
// be_app_messenger default apply at CALL time.
.def("StartWatching", [](BVolumeRoster& self, BMessenger messenger) { return self.StartWatching(messenger); }, "", py::arg("messenger"))
.def("StartWatching", [](BVolumeRoster& self) { return self.StartWatching(); }, "")
.def("StopWatching", &BVolumeRoster::StopWatching, "")
.def("Messenger", &BVolumeRoster::Messenger, "")
;


}
