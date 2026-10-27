#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/chrono.h>

#include <kernel/fs_attr.h>
#include <storage/StorageDefs.h>
#include <support/String.h>
#include <support/TypeConstants.h>
#include <Node.h>
#include <Statable.h>
#include <Entry.h>
#include <Directory.h>
#include <String.h>

namespace py = pybind11;


PYBIND11_MODULE(Node,m)
{
py::class_<node_ref>(m, "node_ref")
.def(py::init(), "")
.def(py::init<dev_t, ino_t>(), "", py::arg("device"), py::arg("node"))
.def(py::init<const node_ref &>(), "", py::arg("other"))
.def("__eq__", &node_ref::operator==, "", py::arg("other"))
.def("__ne__", &node_ref::operator!=, "", py::arg("other"))
.def("__lt__", &node_ref::operator<, "", py::arg("other"))
.def("__copy__", &node_ref::operator=, "", py::arg("other"))
.def_readwrite("device", &node_ref::device, "")
.def_readwrite("node", &node_ref::node, "")
;

py::class_<BNode>(m, "BNode") //Commented out BStatable verify if needed
.def(py::init(), "")
.def(py::init<const entry_ref *>(), "", py::arg("ref"))
.def(py::init<const BEntry *>(), "", py::arg("entry"))
.def(py::init<const char *>(), "", py::arg("path"))
.def(py::init<const BDirectory *, const char *>(), "", py::arg("dir"), py::arg("path"))
.def(py::init<const BNode &>(), "", py::arg("node"))
.def("InitCheck", &BNode::InitCheck, "")
// FIXED (2026-10-25): beta6 BNode::GetStat(struct stat*) (Node.h:49) is an
// out-param; the raw binding null-derefs on None. No-arg form with an
// internal stat buffer -> (stat, status); the stat object-arg form (safe
// borrow) kept as an arity-distinguished overload.
.def("GetStat", [](BNode& self)->py::tuple{
	struct stat st = {};
	status_t ret = self.GetStat(&st);
	return py::make_tuple(st, ret);
},"")
.def("GetStat", &BNode::GetStat, "", py::arg("st"))
.def("SetTo", py::overload_cast<const entry_ref *>(&BNode::SetTo), "", py::arg("ref"))
.def("SetTo", py::overload_cast<const BEntry *>(&BNode::SetTo), "", py::arg("entry"))
.def("SetTo", py::overload_cast<const char *>(&BNode::SetTo), "", py::arg("path"))
.def("SetTo", py::overload_cast<const BDirectory *, const char *>(&BNode::SetTo), "", py::arg("dir"), py::arg("path"))
.def("Unset", &BNode::Unset, "")
.def("Lock", &BNode::Lock, "")
.def("Unlock", &BNode::Unlock, "")
.def("Sync", &BNode::Sync, "")
//.def("WriteAttr", &BNode::WriteAttr, "", py::arg("name"), py::arg("type"), py::arg("offset"), py::arg("buffer"), py::arg("length"))
.def("WriteAttr", [](BNode& self, const char* name, type_code type, off_t offset, py::buffer buffer) {
        // Estrai i dati dal buffer
        py::buffer_info info = buffer.request(); //use var = bytearray(b"Hello, World!")
        const void* data = info.ptr;
        size_t length = info.size;
        // Chiamare il metodo WriteAttr con i dati estratti
        return self.WriteAttr(name, type, offset, data, length);
}, "",py::arg("name"), py::arg("type"), py::arg("offset"), py::arg("buffer"))//, py::arg("length"))
//.def("ReadAttr", &BNode::ReadAttr, "", py::arg("name"), py::arg("type"), py::arg("offset"), py::arg("buffer"), py::arg("length"))
/*
.def("ReadAttr", [](BNode& self, const char* name, type_code type, off_t offset, void* buffer, size_t length)->py::object{
		void* tmp = malloc(length);
		if (tmp == nullptr){
			throw std::runtime_error("Error allocating memory");
		}
		ssize_t result = self.ReadAttr(name, type, offset, tmp, length);
		if (result < 0) {
			free(tmp);
			throw std::runtime_error("Error calling ReadAttr");
		}
		py::object ret;
		switch (type) {
			//test Int64 for reinterpretation in int32*
			case B_INT64_TYPE:
			case B_INT32_TYPE:
			case B_INT16_TYPE:
			case B_INT8_TYPE:
				ret = py::int_(*reinterpret_cast<int32*>(PyLong_AsVoidPtr(PyLong_FromVoidPtr(tmp))));
				break;
			case B_STRING_TYPE:
			case B_MIME_STRING_TYPE:
			case B_ASCII_TYPE:
				ret = py::str(static_cast<const char*>(tmp));
				//ret = PyUnicode_FromString(static_cast<const char*>(tmp));
				break;
			case B_BOOL_TYPE:
				ret = py::bool_(PyBool_FromLong(*reinterpret_cast<int32*>(PyLong_AsVoidPtr(PyLong_FromVoidPtr(tmp)))));
				break;
			case B_FLOAT_TYPE:
				ret = py::float_(*reinterpret_cast<float*>(tmp));
				break;
			case B_DOUBLE_TYPE:
				ret = py::float_(*reinterpret_cast<double*>(tmp));
				break;
			case B_TIME_TYPE:{
				//bigtime_t timeValue = *reinterpret_cast<bigtime_t*>(tmp);
				time_t timeValue = *reinterpret_cast<time_t*>(tmp);
				std::chrono::system_clock::time_point timePoint =
				std::chrono::system_clock::time_point(std::chrono::seconds(timeValue)); //this was microseconds
				// Calcola il tempo trascorso dalla mezzanotte del 1 gennaio 1970 in secondi
				auto seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count();
				ret = py::module::import("datetime").attr("datetime").attr("fromtimestamp")(seconds_since_epoch);
    			break;
				
				//bigtime_t timeValue=*reinterpret_cast<bigtime_t*>(tmp);
				//std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::from_time_t(timeValue / 1000000);
				//ret = py::module::import("datetime").attr("datetime").attr("fromtimestamp")(std::chrono::system_clock::to_time_t(timePoint));
				//break;
			}
			case B_RAW_TYPE:{
				ret = py::bytes(reinterpret_cast<const char*>(tmp), length);
				break;
			}
			default:
				ret = py::bytes(reinterpret_cast<const char*>(tmp), length);
				//ret = py::bytes(static_cast<const char*>(tmp));
				//py::none();
				break;
		}
		free(tmp);
		return ret;
		//return py::reinterpret_steal<py::object>(ret);
}, "", py::arg("name"), py::arg("type"), py::arg("offset"), py::arg("buffer")=NULL, py::arg("length"))		
*/
.def("ReadAttr", [](BNode& self, const char* name, type_code type, off_t offset, void* buffer, size_t length)->std::tuple<py::object,ssize_t>{
		void* tmp = malloc(length);
		if (tmp == nullptr){
			throw std::runtime_error("Error allocating memory");
		}
		ssize_t result = self.ReadAttr(name, type, offset, tmp, length);
		//if (result < 0) {
		//	free(tmp);
		//	throw std::runtime_error("Error calling ReadAttr");
		//}
		py::object ret;
		switch (type) {
			//test Int64 for reinterpretation in int32*
			case B_INT64_TYPE:
				ret = py::int_(*reinterpret_cast<int64_t*>(PyLong_AsVoidPtr(PyLong_FromVoidPtr(tmp))));
				break;
			case B_INT32_TYPE:
			case B_INT16_TYPE:
			case B_INT8_TYPE:
				ret = py::int_(*reinterpret_cast<int32*>(PyLong_AsVoidPtr(PyLong_FromVoidPtr(tmp))));
				break;
			case B_STRING_TYPE:
			case B_MIME_STRING_TYPE:
			case B_ASCII_TYPE:
				{
				//py::bytes bytes_obj(reinterpret_cast<const char*>(tmp),length);
				//ret = py::str(bytes_obj);
				BString bstring(reinterpret_cast<const char*>(tmp),length);
				ret = py::str(bstring.String());
				}
				//ret = py::str(static_cast<const char*>(tmp));
				//ret = py::str(reinterpret_cast<const char *>(tmp));
				//ret = PyUnicode_FromString(reinterpret_cast<const char*>(tmp));
				break;
			case B_BOOL_TYPE:
				ret = py::bool_(*reinterpret_cast<bool*>(tmp));
				break;
			case B_FLOAT_TYPE:
				ret = py::float_(*reinterpret_cast<float*>(tmp));
				break;
			case B_DOUBLE_TYPE:
				ret = py::float_(*reinterpret_cast<double*>(tmp));
				break;
			case B_TIME_TYPE:{
				//bigtime_t timeValue = *reinterpret_cast<bigtime_t*>(tmp);
				time_t timeValue = *reinterpret_cast<time_t*>(tmp);
				std::chrono::system_clock::time_point timePoint =
				std::chrono::system_clock::time_point(std::chrono::seconds(timeValue)); //this was microseconds
				// Calcola il tempo trascorso dalla mezzanotte del 1 gennaio 1970 in secondi
				auto seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch()).count();
				ret = py::module::import("datetime").attr("datetime").attr("fromtimestamp")(seconds_since_epoch);
    			break;
				
				//bigtime_t timeValue=*reinterpret_cast<bigtime_t*>(tmp);
				//std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::from_time_t(timeValue / 1000000);
				//ret = py::module::import("datetime").attr("datetime").attr("fromtimestamp")(std::chrono::system_clock::to_time_t(timePoint));
				//break;
			}
			case B_RAW_TYPE:{
				ret = py::bytes(reinterpret_cast<const char*>(tmp), length);
				break;
			}
			default:
				ret = py::bytes(reinterpret_cast<const char*>(tmp), length);
				//ret = py::bytes(static_cast<const char*>(tmp));
				//py::none();
				break;
		}
		free(tmp);
		return std::make_tuple(ret,result);
		//return py::reinterpret_steal<py::object>(ret);
}, "", py::arg("name"), py::arg("type"), py::arg("offset"), py::arg("buffer")=NULL, py::arg("length"))		
.def("RemoveAttr", &BNode::RemoveAttr, "", py::arg("name"))
//.def("RenameAttr", py::overload_cast<const char *,const char *>(&BNode::RenameAttr), "", py::arg("oldName"), py::arg("newName"))
.def("RenameAttr", &BNode::RenameAttr, "", py::arg("oldName"), py::arg("newName"))
//.def("GetAttrInfo", &BNode::GetAttrInfo, "", py::arg("name"), py::arg("info"))
//.def("GetAttrInfo", py::overload_cast<const char *, struct attr_info *>(&BNode::GetAttrInfo, py::const_), "", py::arg("name"), py::arg("info"))
/*.def("GetAttrInfo", [](BNode& self, const char* attr) -> std::pair<status_t, attr_info> {
    attr_info info;
    status_t result = self.GetAttrInfo(attr, &info);
    return {result, info};
}, "", py::arg("name"))
*/
/*
.def("GetAttrInfo", [](BNode& self, const char* attr, struct attr_info* info){
	status_t result = self.GetAttrInfo(attr, info);
	if (result == 0) {
		return *info;
	} else {
		throw std::runtime_error("Errore durante la chiamata a GetAttrInfo");
	}
}, py::arg("attr"), py::arg("info")=attr_info())
*/
// FIXED (2026-10-20): the old binding took `struct attr_info* info` as an
// out-param defaulted to `attr_info()` — a struct value where a pointer is
// expected. pybind11 had to cast that default at REGISTRATION time, so the
// fs_attr module needed importing first or `import Node` failed; and the
// default was unusable at call time (None would null-deref). Now: no
// out-param, local buffer, tuple return (info, status).
.def("GetAttrInfo", [](BNode& self, const char* attr) -> std::tuple<attr_info, status_t> {
	attr_info info = {};
	status_t result = self.GetAttrInfo(attr, &info);
	return std::make_tuple(info, result);
}, "", py::arg("attr"))
//.def("GetNextAttrName", &BNode::GetNextAttrName, "",py::arg("buffer"))
//.def("GetNextAttrName", py::overload_cast<char *>(&BNode::GetNextAttrName), "",py::arg("buffer"))
/*.def("GetNextAttrName", [](BNode& self, char* buffer){
	status_t result = self.GetNextAttrName(buffer);
	if (result == 0) {
		return std::string(buffer);
	} else {
		throw std::runtime_error("Errore durante la chiamata a GetNextAttrName");
	}
}, py::arg("buffer")="")//, py::return_value_policy::reference_internal*/
// FIXED (2026-10-20): the old binding passed the caller's `char*` (a Python
// str's internal buffer) to GetNextAttrName, which WRITES into it — heap
// corruption. Now: internal buffer, tuple return (name, status).
.def("GetNextAttrName", [](BNode& self) -> std::tuple<std::string, status_t> {
	char buffer[B_ATTR_NAME_LENGTH];
	status_t result = self.GetNextAttrName(buffer);
	return std::make_tuple(result == B_OK ? std::string(buffer) : std::string(), result);
}, "")
.def("RewindAttrs", &BNode::RewindAttrs, "")
// FIXED (2026-10-20): beta6's WriteAttrString takes `const BString*`, so the
// raw binding forced callers to construct a BString (String module) just to
// pass a string. Now accepts a plain Python str.
.def("WriteAttrString", [](BNode& self, const char* name, const std::string& data) {
	BString s(data.c_str());
	return self.WriteAttrString(name, &s);
}, "", py::arg("name"), py::arg("data"))
// FIXED (2026-10-20): beta6's BNode::ReadAttrString(name, BString* result) is
// an out-param API; the old binding exposed `result` as an input arg (the char*
// caster wrote into the caller's str buffer — corruption). Now: internal
// BString, tuple return (string, status).
.def("ReadAttrString", [](BNode& self, const char* name)
     -> std::tuple<std::string, status_t> {
	BString result;
	status_t r = self.ReadAttrString(name, &result);
	return std::make_tuple(r == B_OK ? std::string(result.String()) : std::string(), r);
}, "", py::arg("name"))
.def("__copy__", &BNode::operator=, "", py::arg("node"))
.def("__eq__", &BNode::operator==, "", py::arg("node"))
.def("__ne__", &BNode::operator!=, "", py::arg("node"))
.def("Dup", &BNode::Dup, "")
;

}
