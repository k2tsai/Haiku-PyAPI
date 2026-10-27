#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <support/DataIO.h>

namespace py = pybind11;

class PyBDataIO : public BDataIO {
public:
	using BDataIO::BDataIO;
    ssize_t				Read(void* buffer, size_t size) override {
        PYBIND11_OVERLOAD(ssize_t, BDataIO, Read, buffer, size);
    }
	ssize_t				Write(const void* buffer, size_t size) override {
        PYBIND11_OVERLOAD(ssize_t, BDataIO, Write, buffer, size);
    }
    status_t			Flush() override {
    	PYBIND11_OVERLOAD(status_t, BDataIO, Flush);
    }
};


class PyBPositionIO : public BPositionIO {
public:
//	using BPositionIO::Seek;
//    using BPositionIO::Position;
    using BPositionIO::BPositionIO;
    ssize_t				Read(void* buffer, size_t size) override {
        PYBIND11_OVERLOAD(ssize_t, BPositionIO, Read, buffer, size);
    }
	ssize_t				Write(const void* buffer, size_t size) override {
        PYBIND11_OVERLOAD(ssize_t, BPositionIO, Write, buffer, size);
    }
    ssize_t ReadAt(off_t position, void* buffer, size_t size) override {
        PYBIND11_OVERLOAD_PURE(ssize_t, BPositionIO, ReadAt, position, buffer, size);
    }
    ssize_t WriteAt(off_t position, const void* buffer, size_t size) override {
        PYBIND11_OVERLOAD_PURE(ssize_t, BPositionIO, WriteAt, position, buffer, size);
    }
    off_t Seek(off_t position, uint32 seekMode) override {
        PYBIND11_OVERLOAD_PURE(off_t, BPositionIO, Seek, position, seekMode);
    }
    off_t Position() const override {
        PYBIND11_OVERLOAD_PURE(off_t, BPositionIO, Position);
    }
    status_t SetSize(off_t size) override {
        PYBIND11_OVERLOAD(status_t, BPositionIO, SetSize, size);
    }
    status_t			GetSize(off_t* size) const override {
        PYBIND11_OVERLOAD(status_t, BPositionIO, GetSize, size);
    }
};

class PyBMemoryIO : public BMemoryIO {
public:
	using BMemoryIO::BMemoryIO;
    ssize_t				ReadAt(off_t position, void* buffer, size_t size) override {
        PYBIND11_OVERLOAD(ssize_t, BMemoryIO, ReadAt, position, buffer, size);
    }
	ssize_t				WriteAt(off_t position, const void* buffer, size_t size) override {
        PYBIND11_OVERLOAD(ssize_t, BMemoryIO, WriteAt, position, buffer, size);
    }
    off_t				Seek(off_t position, uint32 seekMode) override {
    	PYBIND11_OVERLOAD(off_t, BMemoryIO, Seek, position, seekMode);
    }
    off_t				Position() const override {
    	PYBIND11_OVERLOAD(off_t, BMemoryIO, Position);
    }
    status_t			SetSize(off_t size) override {
    	PYBIND11_OVERLOAD(status_t, BMemoryIO, SetSize, size);
    }
};

class PyBMallocIO : public BMallocIO {
public:
	using BMallocIO::BMallocIO;
    ssize_t				ReadAt(off_t position, void* buffer, size_t size) override {
        PYBIND11_OVERLOAD(ssize_t, BMallocIO, ReadAt, position, buffer, size);
    }
	ssize_t				WriteAt(off_t position, const void* buffer, size_t size) override {
        PYBIND11_OVERLOAD(ssize_t, BMallocIO, WriteAt, position, buffer, size);
    }
    off_t				Seek(off_t position, uint32 seekMode) override {
    	PYBIND11_OVERLOAD(off_t, BMallocIO, Seek, position, seekMode);
    }
    off_t				Position() const override {
    	PYBIND11_OVERLOAD(off_t, BMallocIO, Position);
    }
    status_t			SetSize(off_t size) override {
    	PYBIND11_OVERLOAD(status_t, BMallocIO, SetSize, size);
    }
};

// ---------------------------------------------------------------------------
// Buffer-protocol wrappers.
//
// beta6 (DataIO.h): Read(void* buffer, size_t) / Write(const void*, size_t)
// use caller-owned storage. Bind them through the PEP 3118 buffer protocol:
//   - a bytearray comes back MUTATED (in-place read/write)
//   - Read of a read-only buffer (bytes) is a user error -> clear exception
//   - ReadExactly/WriteExactly return (status, n) tuples (the C out-param
//     size_t* _bytesRead is absorbed)
//
// NOTE: BMemoryIO wraps the pointer, it does NOT copy (beta6 DataIO.cpp).
// Keep the Python buffer object alive for as long as the BMemoryIO is used.
// ---------------------------------------------------------------------------

static inline py::buffer_info
require_bytes_buffer(const py::buffer& buf, bool writable)
{
	py::buffer_info info = buf.request();
	if (info.ndim != 1 || info.itemsize != 1)
		throw std::invalid_argument("buffer must be a 1-byte wide 1-D object");
	if (writable && info.readonly)
		throw std::invalid_argument("buffer must be writable (use a bytearray)");
	return info;
}

static inline size_t
buffer_size_or(const py::object& size, size_t fallback)
{
	if (size.is_none())
		return fallback;
	return size.cast<size_t>();
}

// BDataIO / BPositionIO
static ssize_t
buf_read(BDataIO& self, const py::buffer& buf, const py::object& size)
{
	py::buffer_info info = require_bytes_buffer(buf, true);
	return self.Read(info.ptr, buffer_size_or(size, (size_t)info.size));
}

static ssize_t
buf_write(BDataIO& self, const py::buffer& buf, const py::object& size)
{
	py::buffer_info info = require_bytes_buffer(buf, false);
	return self.Write(info.ptr, buffer_size_or(size, (size_t)info.size));
}

static py::tuple
buf_read_exactly(BDataIO& self, const py::buffer& buf, const py::object& size)
{
	py::buffer_info info = require_bytes_buffer(buf, true);
	size_t n = 0;
	status_t err = self.ReadExactly(info.ptr, buffer_size_or(size, (size_t)info.size), &n);
	return py::make_tuple(err, n);
}

static py::tuple
buf_write_exactly(BDataIO& self, const py::buffer& buf, const py::object& size)
{
	py::buffer_info info = require_bytes_buffer(buf, false);
	size_t n = 0;
	status_t err = self.WriteExactly(info.ptr, buffer_size_or(size, (size_t)info.size), &n);
	return py::make_tuple(err, n);
}

static ssize_t
buf_read_at(BPositionIO& self, off_t pos, const py::buffer& buf, const py::object& size)
{
	py::buffer_info info = require_bytes_buffer(buf, true);
	return self.ReadAt(pos, info.ptr, buffer_size_or(size, (size_t)info.size));
}

static ssize_t
buf_write_at(BPositionIO& self, off_t pos, const py::buffer& buf, const py::object& size)
{
	py::buffer_info info = require_bytes_buffer(buf, false);
	return self.WriteAt(pos, info.ptr, buffer_size_or(size, (size_t)info.size));
}

static py::tuple
buf_read_at_exactly(BPositionIO& self, off_t pos, const py::buffer& buf, const py::object& size)
{
	py::buffer_info info = require_bytes_buffer(buf, true);
	size_t n = 0;
	status_t err = self.ReadAtExactly(pos, info.ptr, buffer_size_or(size, (size_t)info.size), &n);
	return py::make_tuple(err, n);
}

static py::tuple
buf_write_at_exactly(BPositionIO& self, off_t pos, const py::buffer& buf, const py::object& size)
{
	py::buffer_info info = require_bytes_buffer(buf, false);
	size_t n = 0;
	status_t err = self.WriteAtExactly(pos, info.ptr, buffer_size_or(size, (size_t)info.size), &n);
	return py::make_tuple(err, n);
}

static py::tuple
buf_get_size(const BPositionIO& self)
{
	off_t size = 0;
	status_t err = self.GetSize(&size);
	return py::make_tuple(err, size);
}

PYBIND11_MODULE(DataIO, m)
{
py::class_<BDataIO, PyBDataIO>(m, "BDataIO")
.def(py::init(), "")
.def("Read", &buf_read, "read into a writable buffer (bytearray); returns bytes read (>=0) or <0 on error", py::arg("buffer"), py::arg("size")=py::none())
.def("Write", &buf_write, "write from a readable buffer; returns bytes written (>=0) or <0 on error", py::arg("buffer"), py::arg("size")=py::none())
.def("Flush", &BDataIO::Flush, "")
.def("ReadExactly", &buf_read_exactly, "read exactly size bytes; returns (status, bytesRead)", py::arg("buffer"), py::arg("size")=py::none())
.def("WriteExactly", &buf_write_exactly, "write exactly size bytes; returns (status, bytesWritten)", py::arg("buffer"), py::arg("size")=py::none())
;

py::class_<BPositionIO, PyBPositionIO, BDataIO>(m, "BPositionIO")
.def(py::init(), "")
.def("Read", &buf_read, py::arg("buffer"), py::arg("size")=py::none())
.def("Write", &buf_write, py::arg("buffer"), py::arg("size")=py::none())
.def("ReadAt", &buf_read_at, "read at position into a writable buffer; returns bytes read or <0 on error", py::arg("position"), py::arg("buffer"), py::arg("size")=py::none())
.def("WriteAt", &buf_write_at, "write at position from a readable buffer; returns bytes written or <0 on error", py::arg("position"), py::arg("buffer"), py::arg("size")=py::none())
.def("ReadAtExactly", &buf_read_at_exactly, "returns (status, bytesRead)", py::arg("position"), py::arg("buffer"), py::arg("size")=py::none())
.def("WriteAtExactly", &buf_write_at_exactly, "returns (status, bytesWritten)", py::arg("position"), py::arg("buffer"), py::arg("size")=py::none())
.def("Seek", &BPositionIO::Seek, "", py::arg("position"), py::arg("seekMode"))
.def("Position", &BPositionIO::Position, "")
.def("SetSize", &BPositionIO::SetSize, "", py::arg("size"))
.def("GetSize", &buf_get_size, "returns (status, size)")
;

py::class_<BMemoryIO, PyBMemoryIO, BPositionIO>(m, "BMemoryIO")
// buffer-protocol ctor: bytearray -> writable, bytes -> read-only.
// BMemoryIO WRAPS the pointer (no copy): keep the buffer object alive.
.def(py::init([](py::buffer b, size_t length) -> BMemoryIO* {
	py::buffer_info info = require_bytes_buffer(b, false);
	if (info.readonly)
		return new BMemoryIO(const_cast<const void*>(info.ptr), length);
	return new BMemoryIO(info.ptr, length);
}), "create a memory stream over a buffer (bytearray writable, bytes read-only)", py::arg("data"), py::arg("length"))
.def(py::init<void *, size_t>(), "", py::arg("data"), py::arg("length"))
.def(py::init<const void *, size_t>(), "", py::arg("data"), py::arg("length"))
.def("ReadAt", &buf_read_at, py::arg("position"), py::arg("buffer"), py::arg("size")=py::none())
.def("WriteAt", &buf_write_at, py::arg("position"), py::arg("buffer"), py::arg("size")=py::none())
.def("Seek", &BMemoryIO::Seek, "", py::arg("position"), py::arg("seekMode"))
.def("Position", &BMemoryIO::Position, "")
.def("SetSize", &BMemoryIO::SetSize, "", py::arg("size"))
;

py::class_<BMallocIO,PyBMallocIO, BPositionIO>(m, "BMallocIO")
.def(py::init(), "")
.def("ReadAt", &buf_read_at, py::arg("position"), py::arg("buffer"), py::arg("size")=py::none())
.def("WriteAt", &buf_write_at, py::arg("position"), py::arg("buffer"), py::arg("size")=py::none())
.def("Seek", &BMallocIO::Seek, "", py::arg("position"), py::arg("seekMode"))
.def("Position", &BMallocIO::Position, "")
.def("SetSize", &BMallocIO::SetSize, "", py::arg("size"))
.def("SetBlockSize", &BMallocIO::SetBlockSize, "", py::arg("blockSize"))
.def("Buffer", &BMallocIO::Buffer, "raw internal buffer (capsule; use BufferBytes instead)")
.def("BufferBytes", [](const BMallocIO& self) {
	const char* p = (const char*)self.Buffer();
	return py::bytes(p ? p : "", self.BufferLength());
}, "contents of the malloc'd buffer as bytes")
.def("BufferLength", &BMallocIO::BufferLength, "")
;

}
