#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <support/parsedate.h>

namespace py = pybind11;


// Funzione di esempio che accetta un array di stringhe in stile C
extern void set_dateformats(const char **table);

// Converte uno std::vector<char*> in un array di stringhe in stile C
void set_dateformats_wrapper(const std::vector<std::string>& formats) {
    std::vector<const char*> c_formats;
    for (const auto& format : formats) {
        c_formats.push_back(format.c_str());
    }
    c_formats.push_back(nullptr);  // Aggiunge un terminatore nullo

    set_dateformats(c_formats.data());
}

// Funzione di esempio che restituisce un array di stringhe in stile C
extern const char **get_dateformats(void);

// Converte l'array di stringhe in stile C in uno std::vector<char*>
std::vector<char*> get_dateformats_wrapper() {
    const char **dateformats = get_dateformats();
    std::vector<char*> result;

    // Assume che la fine dell'array sia segnata da un puntatore nullo
    for (size_t i = 0; dateformats[i] != nullptr; ++i) {
        result.push_back(const_cast<char*>(dateformats[i]));
    }

    return result;
}

PYBIND11_MODULE(parsedate, m)
{
// beta6: parsedate.h:19-20 — both take (const char*, time_t); `now` has NO
// default in the header, so it stays required.
m.attr("PARSEDATE_RELATIVE_TIME") = PARSEDATE_RELATIVE_TIME;
m.attr("PARSEDATE_MINUTE_RELATIVE_TIME") = PARSEDATE_MINUTE_RELATIVE_TIME;
m.attr("PARSEDATE_INVALID_DATE") = PARSEDATE_INVALID_DATE;

m.def("parsedate", &parsedate, "", py::arg("dateString"), py::arg("now"));

// Pythonic form: absorb the int* _storedFlags out-param, return (time, flags)
m.def("parsedate_etc", [](const char* dateString, time_t now) -> py::tuple {
    int flags = 0;
    time_t t = parsedate_etc(dateString, now, &flags);
    return py::make_tuple(t, flags);
}, "", py::arg("dateString"), py::arg("now"));

// raw form kept for completeness (flags is a capsule/int pointer)
m.def("parsedate_etc", &parsedate_etc, "", py::arg("dateString"), py::arg("now"), py::arg("_storedFlags"));

m.def("set_dateformats", &set_dateformats_wrapper, "", py::arg("table"));

m.def("get_dateformats", &get_dateformats_wrapper, "", py::return_value_policy::take_ownership);

}
