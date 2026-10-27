#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <StorageDefs.h>
#include <fcntl.h>
#include <sys/param.h>
#include <limits.h>

namespace py = pybind11;
//using namespace BPrivate;
//using namespace BPrivate::Storage;
//using namespace BPrivate::Storage::Mime;
//using namespace BPackageKit;



PYBIND11_MODULE(StorageDefs,m)
{
m.attr("B_DEV_NAME_LENGTH") = B_DEV_NAME_LENGTH; //128
m.attr("B_FILE_NAME_LENGTH") = B_FILE_NAME_LENGTH; //NAME_MAX
m.attr("B_PATH_NAME_LENGTH") = B_PATH_NAME_LENGTH; //MAXPATHLEN
m.attr("B_ATTR_NAME_LENGTH") = B_ATTR_NAME_LENGTH; //(B_FILE_NAME_LENGTH - 1)
m.attr("B_MIME_TYPE_LENGTH") = B_MIME_TYPE_LENGTH; //(B_ATTR_NAME_LENGTH - 15)
m.attr("B_MAX_SYMLINKS") = B_MAX_SYMLINKS; //SYMLOOP_MAX


py::enum_<node_flavor>(m, "node_flavor", "")
.value("B_FILE_NODE", node_flavor::B_FILE_NODE, "")
.value("B_SYMLINK_NODE", node_flavor::B_SYMLINK_NODE, "")
.value("B_DIRECTORY_NODE", node_flavor::B_DIRECTORY_NODE, "")
.value("B_ANY_NODE", node_flavor::B_ANY_NODE, "")
.export_values();
// FIXED (2026-10-20): beta6 defines the open-mode constants as POSIX O_*
// flags (B_CREATE_FILE = O_CREAT = 0x200, B_ERASE_FILE = O_TRUNC = 0x400,
// ...); these were hardcoded classic-era values (8/64/128), so Python
// file-creation opens failed with B_ERROR while C++ worked.
m.attr("B_READ_ONLY") = B_READ_ONLY;
m.attr("B_WRITE_ONLY") = B_WRITE_ONLY;
m.attr("B_READ_WRITE") = B_READ_WRITE;
m.attr("B_FAIL_IF_EXISTS") = B_FAIL_IF_EXISTS;
m.attr("B_CREATE_FILE") = B_CREATE_FILE;
m.attr("B_ERASE_FILE") = B_ERASE_FILE;
m.attr("B_OPEN_AT_END") = B_OPEN_AT_END;
//m.attr("B_CLONE_KIND") = 1048576;
}

