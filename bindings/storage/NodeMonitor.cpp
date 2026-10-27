#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <NodeMonitor.h>
#include <StorageDefs.h>
#include <Handler.h>
#include <Looper.h>

namespace py = pybind11;
//using namespace BPrivate;
//using namespace BPrivate::Storage;
//using namespace BPrivate::Storage::Mime;

PYBIND11_MODULE(NodeMonitor, m)
{
m.def("watch_volume", py::overload_cast<dev_t, uint32, BMessenger>(&watch_volume), "", py::arg("volume"), py::arg("flags"), py::arg("target"));

// FIXED (2026-10-27): `py::arg("looper")=NULL` stored the nullptr default as
// int 0, which the class-pointer caster rejects ("looper: BLooper = 0") ->
// the short handler form was permanently uncallable. Arity split (the
// Path.cpp NULL-default defect class): 3-arg lambda applies the C++ NULL
// looper default; the full-arg form keeps looper explicit (pass None).
m.def("watch_volume", [](dev_t volume, uint32 flags, const BHandler* handler) {
    return watch_volume(volume, flags, handler, (const BLooper*)NULL);
}, "", py::arg("volume"), py::arg("flags"), py::arg("handler"));
m.def("watch_volume", py::overload_cast<dev_t, uint32, const BHandler *, const BLooper *>(&watch_volume), "", py::arg("volume"), py::arg("flags"), py::arg("handler"), py::arg("looper"));

m.def("watch_node", py::overload_cast<const node_ref *, uint32, BMessenger>(&watch_node), "", py::arg("node"), py::arg("flags"), py::arg("target"));

m.def("watch_node", [](const node_ref* node, uint32 flags, const BHandler* handler) {
    return watch_node(node, flags, handler, (const BLooper*)NULL);
}, "", py::arg("node"), py::arg("flags"), py::arg("handler"));
m.def("watch_node", py::overload_cast<const node_ref *, uint32, const BHandler *, const BLooper *>(&watch_node), "", py::arg("node"), py::arg("flags"), py::arg("handler"), py::arg("looper"));

m.def("stop_watching", py::overload_cast<BMessenger>(&stop_watching), "", py::arg("target"));

m.def("stop_watching", [](const BHandler* handler) {
    return stop_watching(handler, (const BLooper*)NULL);
}, "", py::arg("handler"));
m.def("stop_watching", py::overload_cast<const BHandler *, const BLooper *>(&stop_watching), "", py::arg("handler"), py::arg("looper"));

m.attr("B_STOP_WATCHING") = 0x0000; //these have been hardcoded due to fault on loading module
m.attr("B_WATCH_NAME") = 0x0001;
m.attr("B_WATCH_STAT") = 0x0002;
m.attr("B_WATCH_ATTR") = 0x0004;
m.attr("B_WATCH_DIRECTORY") = 0x0008;
m.attr("B_WATCH_ALL") = 0x000f;
m.attr("B_WATCH_MOUNT") = 0x0010;
m.attr("B_WATCH_INTERIM_STAT") = 0x0020;
m.attr("B_WATCH_CHILDREN") = 0x0040;

m.attr("B_STAT_MODE") = 0x0001;
m.attr("B_STAT_UID") = 0x0002;
m.attr("B_STAT_GID") = 0x0004;
m.attr("B_STAT_SIZE") = 0x0008;
m.attr("B_STAT_ACCESS_TIME") = 0x0010;
m.attr("B_STAT_MODIFICATION_TIME") = 0x0020;
m.attr("B_STAT_CREATION_TIME") = 0x0040;
m.attr("B_STAT_CHANGE_TIME") = 0x0080;
m.attr("B_STAT_INTERIM_UPDATE") = 0x1000;

// TODO (feature miss): the B_NODE_MONITOR *notification opcodes* are not
// exported. NodeMonitor.h:36-42 defines them as #defines:
//   B_ENTRY_CREATED 1, B_ENTRY_REMOVED 2, B_ENTRY_MOVED 3,
//   B_STAT_CHANGED 4, B_ATTR_CHANGED 5, B_DEVICE_MOUNTED 6,
//   B_DEVICE_UNMOUNTED 7
// A handler dispatching on a B_NODE_MONITOR message needs these to decode its
// `opcode` field (see headers/private/storage/NodeMonitorHandler.cpp). Until
// added — as m.attr(...) like the B_WATCH_*/B_STAT_* flags above; they are
// #defines, so .value() cannot name them the way AppDefs.cpp does for
// B_NODE_MONITOR — consumers must hardcode them (e.g. haiku-llm's
// files_provider folder-watcher). B_NODE_MONITOR ('NDMN') itself IS exported:
// bindings/app/AppDefs.cpp.
}
