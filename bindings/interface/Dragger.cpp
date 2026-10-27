#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <interface/Dragger.h>
#include <GraphicsDefs.h>
#include <Locker.h>
#include <List.h>
#include <View.h>
#include <PopUpMenu.h>
#include <Bitmap.h>

namespace py = pybind11;
using namespace BPrivate;

class PyBDragger : public BDragger {
public:
    using BDragger::BDragger;

    status_t			Archive(BMessage* data, bool deep = true) const override {
        PYBIND11_OVERRIDE_PURE(status_t, BDragger, Archive, data, deep);
    }
    void				AttachedToWindow() override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, AttachedToWindow);
    }
    void				DetachedFromWindow() override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, DetachedFromWindow);
    }
    void				Draw(BRect updateRect) override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, Draw, updateRect);
    }
    void				MouseDown(BPoint where) override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, MouseDown, where);
    }
    void				MouseUp(BPoint where) override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, MouseUp, where);
    }
    void				MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage) override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, MouseMoved, where, transit, dragMessage);
    }
    void				MessageReceived(BMessage* message) override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, MessageReceived, message);
    }
    void				FrameMoved(BPoint newPosition) override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, FrameMoved, newPosition);
    }
    void				FrameResized(float newWidth, float newHeight) override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, FrameResized, newWidth, newHeight);
    }
    BHandler*			ResolveSpecifier(BMessage* message, int32 index, BMessage* specifier, int32 form, const char* property) override {
        PYBIND11_OVERRIDE_PURE(BHandler*, BDragger, ResolveSpecifier, message, index, specifier, form, property);
    }
    status_t			GetSupportedSuites(BMessage* data) override {
        PYBIND11_OVERRIDE_PURE(status_t, BDragger, GetSupportedSuites, data);
    }
    status_t			Perform(perform_code code, void* data) override {
        PYBIND11_OVERRIDE_PURE(status_t, BDragger, Perform, code, data);
    }
    void				ResizeToPreferred() override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, ResizeToPreferred);
    }
    void				GetPreferredSize(float* _width, float* _height) override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, GetPreferredSize, _width, _height);
    }
    void				MakeFocus(bool focus = true) override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, MakeFocus, focus);
    }
    void				AllAttached() override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, AllAttached);
    }
    void				AllDetached() override {
        PYBIND11_OVERRIDE_PURE(void, BDragger, AllDetached);
    }
    BBitmap*			DragBitmap(BPoint* offset, drawing_mode* mode) override {
        PYBIND11_OVERRIDE_PURE(BBitmap*, BDragger, DragBitmap, offset, mode);
    }
};

PYBIND11_MODULE(Dragger,m)
{
//m.attr("replicant_data") = replicant_data;

//m.attr("ShelfContainerViewFilter") = ShelfContainerViewFilter;

py::class_<BDragger, BView>(m, "BDragger")
.def(py::init<BRect, BView *, unsigned int, unsigned int>(), "", py::arg("frame"), py::arg("target"), py::arg("resizingMode")=B_FOLLOW_NONE, py::arg("flags")=B_WILL_DRAW)
.def(py::init<BView *, unsigned int>(), "", py::arg("target"), py::arg("flags")=B_WILL_DRAW)
.def(py::init<BMessage *>(), "", py::arg("data"))
.def_static("Instantiate", &BDragger::Instantiate, "", py::arg("data"))
.def("Archive", &BDragger::Archive, "", py::arg("data"), py::arg("deep")=true)
.def("AttachedToWindow", &BDragger::AttachedToWindow, "")
.def("DetachedFromWindow", &BDragger::DetachedFromWindow, "")
.def("Draw", &BDragger::Draw, "", py::arg("updateRect"))
.def("MouseDown", &BDragger::MouseDown, "", py::arg("where"))
.def("MouseUp", &BDragger::MouseUp, "", py::arg("where"))
.def("MouseMoved", &BDragger::MouseMoved, "", py::arg("where"), py::arg("transit"), py::arg("dragMessage"))
.def("MessageReceived", &BDragger::MessageReceived, "", py::arg("message"))
.def("FrameMoved", &BDragger::FrameMoved, "", py::arg("newPosition"))
.def("FrameResized", &BDragger::FrameResized, "", py::arg("newWidth"), py::arg("newHeight"))
.def_static("ShowAllDraggers", &BDragger::ShowAllDraggers, "")
.def_static("HideAllDraggers", &BDragger::HideAllDraggers, "")
.def_static("AreDraggersDrawn", &BDragger::AreDraggersDrawn, "")
.def("ResolveSpecifier", &BDragger::ResolveSpecifier, "", py::arg("message"), py::arg("index"), py::arg("specifier"), py::arg("form"), py::arg("property"))
.def("GetSupportedSuites", &BDragger::GetSupportedSuites, "", py::arg("data"))
.def("Perform", &BDragger::Perform, "", py::arg("code"), py::arg("data"))
.def("ResizeToPreferred", &BDragger::ResizeToPreferred, "")
// FIXED (2026-10-25): float* out-params were NULL-deref hazards from Python;
// now returns (width, height).
.def("GetPreferredSize", [](BDragger& self) -> std::tuple<float, float> {
	float width = 0;
	float height = 0;
	self.GetPreferredSize(&width, &height);
	return std::make_tuple(width, height);
}, "")
.def("MakeFocus", &BDragger::MakeFocus, "", py::arg("focus")=true)
.def("AllAttached", &BDragger::AllAttached, "")
.def("AllDetached", &BDragger::AllDetached, "")
.def("SetPopUp", &BDragger::SetPopUp, "", py::arg("contextMenu"))
.def("PopUp", &BDragger::PopUp, "")
.def("InShelf", &BDragger::InShelf, "")
.def("Target", &BDragger::Target, "")
// FIXED (2026-10-25): beta6 DragBitmap(BPoint* offset, drawing_mode* mode)
// (Dragger.h:73) — both params are out-pointers; the drawing_mode* one was
// passed a pointer into a static enum value (corruption). Now internal
// buffers, tuple (bitmap, offset, mode); the bitmap is returned by reference
// (the dragger manages its lifetime).
.def("DragBitmap", [](BDragger& self) -> std::tuple<py::object, BPoint, drawing_mode> {
	BPoint offset;
	// beta6: B_OP_OVERWRITE was renamed B_OP_OVER (GraphicsDefs.h:309-320)
	drawing_mode mode = B_OP_OVER;
	BBitmap* bitmap = self.DragBitmap(&offset, &mode);
	return std::make_tuple(
		py::cast(bitmap, py::return_value_policy::reference), offset, mode);
}, "")
//.def_readwrite("Private", &BDragger::Private, "")
;


}
