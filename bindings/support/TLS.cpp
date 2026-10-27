#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <support/TLS.h>

namespace py = pybind11;
//TODO: Please, test this module
PYBIND11_MODULE(TLS, m)
{
// beta6: TLS.h:21 — max 64 process-wide TLS keys
m.attr("TLS_MAX_KEYS") = TLS_MAX_KEYS;

m.def("tls_allocate", &tls_allocate, "");

m.def("tls_get", &tls_get, "", py::arg("index"));

m.def("tls_address", &tls_address, "", py::arg("index"));

m.def("tls_set", &tls_set, "", py::arg("index"), py::arg("value")); //todo value is a void*

}
