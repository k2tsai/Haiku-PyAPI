#ifndef PYAPI_TRAMPOLINES_H
#define PYAPI_TRAMPOLINES_H
// Shared trampoline policy for the whole farm. Include AFTER
// <pybind11/pybind11.h> — it redefines the PYBIND11_OVERLOAD* macros.

#include <pybind11/pybind11.h>

#include <app/Handler.h>
#include <app/Looper.h>
#include <app/Message.h>

// ---- F-008: key events are byte buffers, not C strings ----
// Haiku encodes special keys as 2 raw bytes that may contain NUL
// (arrows are 0x2400..0x2700); str-marshalling truncates at the NUL and
// collides with ASCII 36-39. Python side receives
// (payload: bytes, numBytes: int, repeat: bool); `repeat` is Haiku's
// key_repeat flag from the dispatching message (auto-repeat deliveries,
// e.g. the double KeyDown when Shift is held — csi F-008 RE), False for
// direct calls with no message context.
// NOTE (F-011): key overrides are dispatched WITHOUT the GIL held. Because
// these helpers stand in for the PYBIND11_OVERLOAD macros, they must do the
// two things those macros do for every other trampoline: acquire the GIL, and
// apply the F-009 fail-soft policy. See key_event / key_event_pure below.
namespace pyapi {

// The dispatching message of the current virtual call, via the PUBLIC
// BLooper::CurrentMessage() (the kit-internal be_current_message() is
// not a libbe export). Non-BHandler trampolines (ellipsis overload) and
// out-of-dispatch calls get nullptr -> repeat=false.
inline BMessage* dispatch_msg(BHandler* h) {
    if (h == nullptr) {
        return nullptr;
    }
    BLooper* lp = h->Looper();
    return lp != nullptr ? lp->CurrentMessage() : nullptr;
}
inline BMessage* dispatch_msg(...) {
    return nullptr;
}

template <class Base>
inline bool key_repeat_flag(Base* self) {
    BMessage* msg = dispatch_msg(self);
    bool repeat = false;
    if (msg != nullptr && msg->FindBool("key_repeat", &repeat) != B_OK) {
        repeat = false;
    }
    return repeat;
}

template <class Base, class SelfT>
inline void key_event(const char* override_name,
                      void (Base::*base)(const char *, int32),
                      const char* bytes, int32 numBytes, SelfT* self) {
    // F-011: BView::KeyDown is dispatched on BWindow's message thread, which
    // never entered Python -> the GIL is NOT held. get_override / bytes(...) /
    // calling a pybind function ALL require the GIL -- which is exactly what
    // the PYBIND11_OVERRIDE macro this helper stands in for provides via
    // gil_scoped_acquire (pybind11.h PYBIND11_OVERRIDE_IMPL). Without it the
    // C-API calls fault natively the instant a real key event is delivered,
    // before the override body ever runs (no Python error, no guard output).
    // F-009: this path also bypasses the PYBIND11_OVERLOAD macros that carry
    // the farm's fail-soft policy, so a raise inside a key override would
    // still reach the noexcept dispatch frame and abort(). Apply the same
    // catch/print/return-here guard here (void => just return).
    pybind11::gil_scoped_acquire gil;
    auto f = pybind11::get_override(self, override_name);
    if (f) {
        try {
            f(pybind11::bytes(bytes, static_cast<size_t>(numBytes)), numBytes,
              key_repeat_flag(self));
        } catch (pybind11::error_already_set & pyapi_err_) {
            pyapi_err_.restore();
            PyErr_Print();
        }
        return;
    }
    (self->*base)(bytes, numBytes);
}

// pure-virtual variant: no base implementation to fall through to
template <class SelfT>
inline void key_event_pure(const char* override_name,
                           const char* bytes, int32 numBytes, SelfT* self) {
    // F-011 / F-009: same GIL acquisition + fail-soft policy as key_event
    // above (this path likewise bypasses the PYBIND11_OVERLOAD macros).
    pybind11::gil_scoped_acquire gil;
    auto f = pybind11::get_override(self, override_name);
    if (f) {
        try {
            f(pybind11::bytes(bytes, static_cast<size_t>(numBytes)), numBytes,
              key_repeat_flag(self));
        } catch (pybind11::error_already_set & pyapi_err_) {
            pyapi_err_.restore();
            PyErr_Print();
        }
    }
}

// public-API direction: Python (bytes, n) -> raw C++ call. `repeat` is
// accepted for signature symmetry with the override delivery and ignored
// (a direct call has no dispatching message).
template <class Base>
inline void key_event_from_python(Base& self, pybind11::bytes payload,
                                  int32 numBytes,
                                  void (Base::*base)(const char *, int32)) {
    const std::string s(payload);
    (self.*base)(s.c_str(), numBytes);
}

template <class R> struct zero_return { static R get() { return R{}; } };
template <> struct zero_return<void> { static void get() {} };

}  // namespace pyapi

// ---- F-009: trampoline error policy ----
// A Python exception escaping an override propagates through the C++
// caller's frame; where that frame is noexcept (app_server dispatch),
// it reaches std::terminate and kills the whole process. Policy: print
// the error the way CPython would (stderr + traceback), then return the
// type's zero value — the platform's fail-soft contract. Consumers see
// a loud stderr trace, not a silent swallow.
#undef PYBIND11_OVERLOAD
#undef PYBIND11_OVERLOAD_PURE

#define PYBIND11_OVERLOAD(rettype, classtype, fn, ...)                        \
    try {                                                                     \
        PYBIND11_OVERRIDE(rettype, classtype, fn, __VA_ARGS__);               \
    } catch (pybind11::error_already_set & pyapi_err_) {                 \
        pyapi_err_.restore();                                                 \
        PyErr_Print();                                                        \
        return pyapi::zero_return<rettype>::get();                            \
    }

#define PYBIND11_OVERLOAD_PURE(rettype, classtype, fn, ...)                   \
    try {                                                                     \
        PYBIND11_OVERRIDE_PURE(rettype, classtype, fn, __VA_ARGS__);          \
    } catch (pybind11::error_already_set & pyapi_err_) {                 \
        pyapi_err_.restore();                                                 \
        PyErr_Print();                                                        \
        return pyapi::zero_return<rettype>::get();                            \
    }

#endif  // PYAPI_TRAMPOLINES_H
