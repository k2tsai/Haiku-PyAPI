#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <interface/Screen.h>
#include <Accelerant.h>
#include <GraphicsDefs.h>
#include <Rect.h>
#include <OS.h>
#include <Bitmap.h>
#include <Window.h>

namespace py = pybind11;
using namespace BPrivate;

PYBIND11_MODULE(Screen,m)
{
//m.attr("BPrivateScreen") = BPrivateScreen;

py::class_<BScreen>(m, "BScreen")
.def(py::init<screen_id>(), "", py::arg("id")=B_MAIN_SCREEN_ID)
.def(py::init<BWindow *>(), "", py::arg("window"))
.def("IsValid", &BScreen::IsValid, "")
.def("SetToNext", &BScreen::SetToNext, "")
.def("ColorSpace", &BScreen::ColorSpace, "")
.def("Frame", &BScreen::Frame, "")
.def("ID", &BScreen::ID, "")
.def("WaitForRetrace", py::overload_cast<>(&BScreen::WaitForRetrace), "")
.def("WaitForRetrace", py::overload_cast<bigtime_t>(&BScreen::WaitForRetrace), "", py::arg("timeout"))
.def("IndexForColor", py::overload_cast<rgb_color>(&BScreen::IndexForColor), "", py::arg("color"))
.def("IndexForColor", py::overload_cast<unsigned char, unsigned char, unsigned char, unsigned char>(&BScreen::IndexForColor), "", py::arg("red"), py::arg("green"), py::arg("blue"), py::arg("alpha")=255)
.def("ColorForIndex", &BScreen::ColorForIndex, "", py::arg("index"))
.def("InvertIndex", &BScreen::InvertIndex, "", py::arg("index"))
.def("ColorMap", &BScreen::ColorMap, "")
//.def("GetBitmap", &BScreen::GetBitmap, "", py::arg("_bitmap"), py::arg("drawCursor")=true, py::arg("frame")=NULL)
// FIXED (2026-10-25): the old lambda did `new BBitmap(*frame, ...)` while
// `frame` defaulted to NULL (segfault when omitted), and the pre-allocated
// bitmap was leaked anyway — beta6's GetBitmap(BBitmap**) allocates the
// bitmap itself (Screen.h:47). Now: internal buffer, (bitmap, status) tuple,
// optional frame.
.def("GetBitmap", [](BScreen& self, bool drawCursor) -> std::tuple<BBitmap*, status_t> {
            BBitmap* buffer = nullptr;
            status_t result = self.GetBitmap(&buffer, drawCursor);
            return std::make_tuple(buffer, result);
        }, "", py::arg("drawCursor")=true)
.def("GetBitmap", [](BScreen& self, bool drawCursor, BRect& frame) -> std::tuple<BBitmap*, status_t> {
            BBitmap* buffer = nullptr;
            status_t result = self.GetBitmap(&buffer, drawCursor, &frame);
            return std::make_tuple(buffer, result);
        }, "", py::arg("drawCursor")=true, py::arg("frame"))
/*
.def("GetBitmap", [](BScreen& self, bool drawCursor, BRect* frame) -> BBitmap* {
            // Chiamare la funzione originale
            //BBitmap* buffer;
            BBitmap* buffer = new BBitmap(*frame,B_RGBA32);
            status_t result = self.GetBitmap(&buffer,drawCursor,frame);

            // Gestire gli errori
            if (result != B_OK) {
                throw std::runtime_error("Errore durante la chiamata a GetBitmap");
            }
            return buffer;
            //return py::capsule(buffer,""BBitmap", &deleteBBitmap);
        }, py::arg("drawCursor")=true, py::arg("frame")=NULL)
*/
.def("ReadBitmap", &BScreen::ReadBitmap, "", py::arg("bitmap"), py::arg("drawCursor")=true, py::arg("frame")=NULL)
.def("DesktopColor", py::overload_cast<>(&BScreen::DesktopColor), "")
.def("DesktopColor", py::overload_cast<uint32>(&BScreen::DesktopColor), "", py::arg("workspace"))
.def("SetDesktopColor", py::overload_cast<rgb_color, bool>(&BScreen::SetDesktopColor), "", py::arg("color"), py::arg("stick")=true)
.def("SetDesktopColor", py::overload_cast<rgb_color, uint32, bool>(&BScreen::SetDesktopColor), "", py::arg("color"), py::arg("workspace"), py::arg("stick")=true)
.def("ProposeMode", &BScreen::ProposeMode, "", py::arg("target"), py::arg("low"), py::arg("high"))
//.def("GetModeList", &BScreen::GetModeList, "", py::arg("_modeList"), py::arg("_count"))
.def("GetModeList", [](BScreen& self, uint32 _count) -> std::tuple<std::vector<display_mode>, status_t>{
            // Chiamare la funzione originale
            // uint32 _count; <- removed as passed by argument
            display_mode* _modeList;
            status_t result = self.GetModeList(&_modeList, &_count);

            // Gestire gli errori
            //if (result != B_OK) {
            //    throw std::runtime_error("Errore durante la chiamata a GetModeList");
            //}

            // Convertire il risultato in un vettore di display_mode
            std::vector<display_mode> resultVector(_modeList, _modeList + _count);

            // Liberare la memoria allocata da GetModeList
            free(_modeList);

            return std::make_tuple(resultVector,result);
        }
)
/*
.def("GetModeList", [](BScreen& self, uint32 _count) -> std::vector<display_mode> {
            // Chiamare la funzione originale
            // uint32 _count; <- removed as passed by argument
            display_mode* _modeList;
            status_t result = self.GetModeList(&_modeList, &_count);

            // Gestire gli errori
            if (result != B_OK) {
                throw std::runtime_error("Errore durante la chiamata a GetModeList");
            }

            // Convertire il risultato in un vettore di display_mode
            std::vector<display_mode> resultVector(_modeList, _modeList + _count);

            // Liberare la memoria allocata da GetModeList
            free(_modeList);

            return resultVector;
        }
)
*/
// FIXED (2026-10-25): display_mode* out-param (unregistered from Python ->
// None -> NULL-deref); now (mode, status) tuples (Screen.h:66-68).
.def("GetMode", [](BScreen& self) -> std::tuple<display_mode, status_t> {
            display_mode mode = {};
            status_t result = self.GetMode(&mode);
            return std::make_tuple(mode, result);
        }, "")
.def("GetMode", [](BScreen& self, uint32 workspace) -> std::tuple<display_mode, status_t> {
            display_mode mode = {};
            status_t result = self.GetMode(workspace, &mode);
            return std::make_tuple(mode, result);
        }, "", py::arg("workspace"))
.def("SetMode", py::overload_cast<display_mode *, bool>(&BScreen::SetMode), "", py::arg("mode"), py::arg("makeDefault")=false)
.def("SetMode", py::overload_cast<uint32, display_mode *, bool>(&BScreen::SetMode), "", py::arg("workspace"), py::arg("mode"), py::arg("makeDefault")=false)
// FIXED (2026-10-25): out-params (accelerant_device_info*/monitor_info*/*
// uint32*/display_timing_constraints*) were NULL-deref hazards from Python;
// now internal buffers, tuples (values..., status) (Screen.h:74-82).
.def("GetDeviceInfo", [](BScreen& self) -> std::tuple<accelerant_device_info, status_t> {
            accelerant_device_info info = {};
            status_t result = self.GetDeviceInfo(&info);
            return std::make_tuple(info, result);
        }, "")
.def("GetMonitorInfo", [](BScreen& self) -> std::tuple<monitor_info, status_t> {
            monitor_info info = {};
            status_t result = self.GetMonitorInfo(&info);
            return std::make_tuple(info, result);
        }, "")
.def("GetPixelClockLimits", [](BScreen& self, display_mode& mode) -> std::tuple<uint32, uint32, status_t> {
            uint32 low = 0;
            uint32 high = 0;
            status_t result = self.GetPixelClockLimits(&mode, &low, &high);
            return std::make_tuple(low, high, result);
        }, "", py::arg("mode"))
.def("GetTimingConstraints", [](BScreen& self) -> std::tuple<display_timing_constraints, status_t> {
            display_timing_constraints timingConstraints = {};
            status_t result = self.GetTimingConstraints(&timingConstraints);
            return std::make_tuple(timingConstraints, result);
        }, "")
.def("SetDPMS", &BScreen::SetDPMS, "", py::arg("state"))
.def("DPMSState", &BScreen::DPMSState, "")
.def("DPMSCapabilites", &BScreen::DPMSCapabilites, "")
// FIXED (2026-10-25): float* out-param; now (brightness, status).
.def("GetBrightness", [](BScreen& self) -> std::tuple<float, status_t> {
            float brightness = 0;
            status_t result = self.GetBrightness(&brightness);
            return std::make_tuple(brightness, result);
        }, "")
.def("SetBrightness", &BScreen::SetBrightness, "", py::arg("brightness"))
;


}
