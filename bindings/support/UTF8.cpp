#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <support/UTF8.h>
#include <InterfaceDefs.h>

namespace py = pybind11;

// beta6: UTF8.h:51-57 — the C functions take int32* out-params for the
// lengths and a char* dest. The Pythonic forms below absorb them and return
// (status, dest_bytes, state). `substitute` defaults to B_SUBSTITUTE
// (InterfaceDefs.h:72 = 0x1a).
static inline py::buffer_info
source_bytes_info(const py::bytes& source)
{
	py::buffer buf(source);
	return buf.request();
}

PYBIND11_MODULE(UTF8, m)
{
m.attr("B_ISO1_CONVERSION") = 0;//py::cast(B_ISO....);
m.attr("B_ISO2_CONVERSION") = 1;
m.attr("B_ISO3_CONVERSION") = 2;
m.attr("B_ISO4_CONVERSION") = 3;
m.attr("B_ISO5_CONVERSION") = 4;
m.attr("B_ISO6_CONVERSION") = 5;
m.attr("B_ISO7_CONVERSION") = 6;
m.attr("B_ISO8_CONVERSION") = 7;
m.attr("B_ISO9_CONVERSION") = 8;
m.attr("B_ISO10_CONVERSION") = 9;
m.attr("B_MAC_ROMAN_CONVERSION") = 10;
m.attr("B_SJIS_CONVERSION") = 11;
m.attr("B_EUC_CONVERSION") = 12;
m.attr("B_JIS_CONVERSION") = 13;
m.attr("B_MS_WINDOWS_CONVERSION") = 14;
m.attr("B_UNICODE_CONVERSION") = 15;
m.attr("B_KOI8R_CONVERSION") = 16;
m.attr("B_MS_WINDOWS_1251_CONVERSION") = 17;
m.attr("B_MS_DOS_866_CONVERSION") = 18;
m.attr("B_MS_DOS_CONVERSION") = 19;
m.attr("B_EUC_KR_CONVERSION") = 20;
m.attr("B_ISO13_CONVERSION") = 21;
m.attr("B_ISO14_CONVERSION") = 22;
m.attr("B_ISO15_CONVERSION") = 23;
m.attr("B_BIG5_CONVERSION") = 24;
m.attr("B_GBK_CONVERSION") = 25;
m.attr("B_UTF16_CONVERSION") = 26;
m.attr("B_MS_WINDOWS_1250_CONVERSION") = 27;

m.def("convert_to_utf8", [](uint32 sourceEncoding, py::bytes source,
        int32 state, char substitute = B_SUBSTITUTE) -> py::tuple {
    py::buffer_info sinfo = source_bytes_info(source);
    int32 srcLen = (int32)sinfo.size;
    std::vector<char> dest((size_t)srcLen * 4 + 4);
    int32 destCap = (int32)dest.size();
    int32 st = state;
    status_t err = convert_to_utf8(sourceEncoding, (const char*)sinfo.ptr, &srcLen,
        dest.data(), &destCap, &st, substitute);
    if (err != B_OK)
        return py::make_tuple(err, py::bytes(), st);
    return py::make_tuple(err, py::bytes(dest.data(), destCap), st);
}, "convert source text in sourceEncoding to UTF-8; returns (status, utf8_bytes, state)",
   py::arg("sourceEncoding"), py::arg("source"), py::arg("state")=0,
   py::arg("substitute")=(char)B_SUBSTITUTE);

m.def("convert_from_utf8", [](uint32 destEncoding, py::bytes source,
        int32 state, char substitute = B_SUBSTITUTE) -> py::tuple {
    py::buffer_info sinfo = source_bytes_info(source);
    int32 srcLen = (int32)sinfo.size;
    std::vector<char> dest((size_t)srcLen * 4 + 4);
    int32 destCap = (int32)dest.size();
    int32 st = state;
    status_t err = convert_from_utf8(destEncoding, (const char*)sinfo.ptr, &srcLen,
        dest.data(), &destCap, &st, substitute);
    if (err != B_OK)
        return py::make_tuple(err, py::bytes(), st);
    return py::make_tuple(err, py::bytes(dest.data(), destCap), st);
}, "convert UTF-8 source text to destEncoding; returns (status, dest_bytes, state)",
   py::arg("destEncoding"), py::arg("source"), py::arg("state")=0,
   py::arg("substitute")=(char)B_SUBSTITUTE);

// raw C forms kept for completeness (lengths/state are int32 pointers)
m.def("convert_to_utf8", &convert_to_utf8, "", py::arg("sourceEncoding"), py::arg("source"), py::arg("sourceLength"), py::arg("dest"), py::arg("destLength"), py::arg("state"), py::arg("substitute")=(char)B_SUBSTITUTE);

m.def("convert_from_utf8", &convert_from_utf8, "", py::arg("destEncoding"), py::arg("source"), py::arg("sourceLength"), py::arg("dest"), py::arg("destLength"), py::arg("state"), py::arg("substitute")=(char)B_SUBSTITUTE);

}
