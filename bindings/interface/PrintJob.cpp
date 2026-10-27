#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>

#include <interface/PrintJob.h>
#include <Picture.h>
#include <View.h>

namespace py = pybind11;


PYBIND11_MODULE(PrintJob,m)
{
//m.attr("") = ;

py::class_<print_file_header>(m, "print_file_header")
.def_readwrite("version", &print_file_header::version, "")
.def_readwrite("page_count", &print_file_header::page_count, "")
.def_readwrite("first_page", &print_file_header::first_page, "")
//.def_readwrite("_reserved", &print_file_header::_reserved, "")
//.def_readwrite("_reserved", &print_file_header::_reserved, py::size_t(sizeof(print_file_header::_reserved)), "")
.def_property(
            "_reserved",
            [](const print_file_header &header) {
                py::array_t<int32_t> result(3);
                std::memcpy(result.mutable_data(), &header._reserved, sizeof(header._reserved));
                return result;
            },
            [](print_file_header &header, py::array_t<int32_t> value) {
                if (value.size() != 3) {
                    throw std::runtime_error("Array must have size 3");
                }
                std::memcpy(&header._reserved, value.data(), sizeof(header._reserved));
            }
        )
;

py::class_<BPrintJob>(m, "BPrintJob")
.def(py::init<const char *>(), "", py::arg("name"))
.def("BeginJob", &BPrintJob::BeginJob, "")
.def("CommitJob", &BPrintJob::CommitJob, "")
.def("ConfigJob", &BPrintJob::ConfigJob, "")
.def("CancelJob", &BPrintJob::CancelJob, "")
.def("ConfigPage", &BPrintJob::ConfigPage, "")
.def("SpoolPage", &BPrintJob::SpoolPage, "")
.def("CanContinue", &BPrintJob::CanContinue, "")
.def("DrawView", &BPrintJob::DrawView, "", py::arg("view"), py::arg("rect"), py::arg("where"))
.def("Settings", &BPrintJob::Settings, "")
.def("SetSettings", &BPrintJob::SetSettings, "", py::arg("archive"))
.def("IsSettingsMessageValid", &BPrintJob::IsSettingsMessageValid, "", py::arg("archive"))
.def("PaperRect", &BPrintJob::PaperRect, "")
.def("PrintableRect", &BPrintJob::PrintableRect, "")
// FIXED (2026-10-25): int32* out-params; now (xDPI, yDPI) tuple.
.def("GetResolution", [](BPrintJob& self) -> std::tuple<int32, int32> {
	int32 xDPI = 0;
	int32 yDPI = 0;
	self.GetResolution(&xDPI, &yDPI);
	return std::make_tuple(xDPI, yDPI);
}, "")
.def("FirstPage", &BPrintJob::FirstPage, "")
.def("LastPage", &BPrintJob::LastPage, "")
// FIXED (2026-10-25): void* type was a caller-allocated string buffer
// out-param (unusable from Python); now an internal buffer, returns
// (type string, type code).
.def("PrinterType", [](const BPrintJob& self) -> std::tuple<std::string, int32> {
	char buffer[256];
	int32 code = self.PrinterType(buffer);
	return std::make_tuple(std::string(buffer), code);
}, "")
;


}
