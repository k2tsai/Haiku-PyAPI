#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <support/TypeConstants.h>

namespace py = pybind11;

// beta6 (TypeConstants.h:15-69): type_code 4cc constants. Bound as explicit
// hex ints (the C 'XXXX' multicharacter form is implementation-defined).
PYBIND11_MODULE(TypeConstants, m) {
    m.attr("B_AFFINE_TRANSFORM_TYPE") = 0x414D5458; // beta6: TypeConstants.h:16 ('AMTX')
    m.attr("B_ALIGNMENT_TYPE") = 0x414C474E; // beta6: TypeConstants.h:17 ('ALGN')
    m.attr("B_ANY_TYPE") = 0x414E5954; // beta6: TypeConstants.h:18 ('ANYT')
    m.attr("B_ATOM_TYPE") = 0x41544F4D; // beta6: TypeConstants.h:19 ('ATOM')
    m.attr("B_ATOMREF_TYPE") = 0x41544D52; // beta6: TypeConstants.h:20 ('ATMR')
    m.attr("B_BOOL_TYPE") = 0x424F4F4C; // beta6: TypeConstants.h:21 ('BOOL')
    m.attr("B_CHAR_TYPE") = 0x43484152; // beta6: TypeConstants.h:22 ('CHAR')
    m.attr("B_COLOR_8_BIT_TYPE") = 0x434C5242; // beta6: TypeConstants.h:23 ('CLRB')
    m.attr("B_DOUBLE_TYPE") = 0x44424C45; // beta6: TypeConstants.h:24 ('DBLE')
    m.attr("B_FLOAT_TYPE") = 0x464C4F54; // beta6: TypeConstants.h:25 ('FLOT')
    m.attr("B_GRAYSCALE_8_BIT_TYPE") = 0x47525942; // beta6: TypeConstants.h:26 ('GRYB')
    m.attr("B_INT16_TYPE") = 0x53485254; // beta6: TypeConstants.h:27 ('SHRT')
    m.attr("B_INT32_TYPE") = 0x4C4F4E47; // beta6: TypeConstants.h:28 ('LONG')
    m.attr("B_INT64_TYPE") = 0x4C4C4E47; // beta6: TypeConstants.h:29 ('LLNG')
    m.attr("B_INT8_TYPE") = 0x42595445; // beta6: TypeConstants.h:30 ('BYTE')
    m.attr("B_LARGE_ICON_TYPE") = 0x49434F4E; // beta6: TypeConstants.h:31 ('ICON')
    m.attr("B_MEDIA_PARAMETER_GROUP_TYPE") = 0x424D4347; // beta6: TypeConstants.h:32 ('BMCG')
    m.attr("B_MEDIA_PARAMETER_TYPE") = 0x424D4354; // beta6: TypeConstants.h:33 ('BMCT')
    m.attr("B_MEDIA_PARAMETER_WEB_TYPE") = 0x424D4357; // beta6: TypeConstants.h:34 ('BMCW')
    m.attr("B_MESSAGE_TYPE") = 0x4D534747; // beta6: TypeConstants.h:35 ('MSGG')
    m.attr("B_MESSENGER_TYPE") = 0x4D534E47; // beta6: TypeConstants.h:36 ('MSNG')
    m.attr("B_MIME_TYPE") = 0x4D494D45; // beta6: TypeConstants.h:37 ('MIME')
    m.attr("B_MINI_ICON_TYPE") = 0x4D49434E; // beta6: TypeConstants.h:38 ('MICN')
    m.attr("B_MONOCHROME_1_BIT_TYPE") = 0x4D4E4F42; // beta6: TypeConstants.h:39 ('MNOB')
    m.attr("B_OBJECT_TYPE") = 0x4F505452; // beta6: TypeConstants.h:40 ('OPTR')
    m.attr("B_OFF_T_TYPE") = 0x4F464654; // beta6: TypeConstants.h:41 ('OFFT')
    m.attr("B_PATTERN_TYPE") = 0x5041544E; // beta6: TypeConstants.h:42 ('PATN')
    m.attr("B_POINTER_TYPE") = 0x504E5452; // beta6: TypeConstants.h:43 ('PNTR')
    m.attr("B_POINT_TYPE") = 0x42504E54; // beta6: TypeConstants.h:44 ('BPNT')
    m.attr("B_PROPERTY_INFO_TYPE") = 0x53435444; // beta6: TypeConstants.h:45 ('SCTD')
    m.attr("B_RAW_TYPE") = 0x52415754; // beta6: TypeConstants.h:46 ('RAWT')
    m.attr("B_RECT_TYPE") = 0x52454354; // beta6: TypeConstants.h:47 ('RECT')
    m.attr("B_REF_TYPE") = 0x52524546; // beta6: TypeConstants.h:48 ('RREF')
    m.attr("B_NODE_REF_TYPE") = 0x4E524546; // beta6: TypeConstants.h:49 ('NREF')
    m.attr("B_RGB_32_BIT_TYPE") = 0x52474242; // beta6: TypeConstants.h:50 ('RGBB')
    m.attr("B_RGB_COLOR_TYPE") = 0x52474243; // beta6: TypeConstants.h:51 ('RGBC')
    m.attr("B_SIZE_TYPE") = 0x53495A45; // beta6: TypeConstants.h:52 ('SIZE')
    m.attr("B_SIZE_T_TYPE") = 0x53495A54; // beta6: TypeConstants.h:53 ('SIZT')
    m.attr("B_SSIZE_T_TYPE") = 0x53535A54; // beta6: TypeConstants.h:54 ('SSZT')
    m.attr("B_STRING_TYPE") = 0x43535452; // beta6: TypeConstants.h:55 ('CSTR')
    m.attr("B_STRING_LIST_TYPE") = 0x5354524C; // beta6: TypeConstants.h:56 ('STRL')
    m.attr("B_TIME_TYPE") = 0x54494D45; // beta6: TypeConstants.h:57 ('TIME')
    m.attr("B_UINT16_TYPE") = 0x55534854; // beta6: TypeConstants.h:58 ('USHT')
    m.attr("B_UINT32_TYPE") = 0x554C4E47; // beta6: TypeConstants.h:59 ('ULNG')
    m.attr("B_UINT64_TYPE") = 0x554C4C47; // beta6: TypeConstants.h:60 ('ULLG')
    m.attr("B_UINT8_TYPE") = 0x55425954; // beta6: TypeConstants.h:61 ('UBYT')
    m.attr("B_VECTOR_ICON_TYPE") = 0x5649434E; // beta6: TypeConstants.h:62 ('VICN')
    m.attr("B_XATTR_TYPE") = 0x58415452; // beta6: TypeConstants.h:63 ('XATR')
    m.attr("B_NETWORK_ADDRESS_TYPE") = 0x4E574144; // beta6: TypeConstants.h:64 ('NWAD')
    m.attr("B_MIME_STRING_TYPE") = 0x4D494D53; // beta6: TypeConstants.h:65 ('MIMS')
    m.attr("B_ASCII_TYPE") = 0x54455854; // beta6: TypeConstants.h:68 ('TEXT', deprecated)

    // System-wide MIME types for URL handling (TypeConstants.h:71+: extern const char*)
    m.attr("B_URL_HTTP") = py::cast(B_URL_HTTP);
    m.attr("B_URL_HTTPS") = py::cast(B_URL_HTTPS);
    m.attr("B_URL_FTP") = py::cast(B_URL_FTP);
    m.attr("B_URL_GOPHER") = py::cast(B_URL_GOPHER);
    m.attr("B_URL_MAILTO") = py::cast(B_URL_MAILTO);
    m.attr("B_URL_NEWS") = py::cast(B_URL_NEWS);
    m.attr("B_URL_NNTP") = py::cast(B_URL_NNTP);
    m.attr("B_URL_TELNET") = py::cast(B_URL_TELNET);
    m.attr("B_URL_RLOGIN") = py::cast(B_URL_RLOGIN);
    m.attr("B_URL_TN3270") = py::cast(B_URL_TN3270);
    m.attr("B_URL_WAIS") = py::cast(B_URL_WAIS);
    m.attr("B_URL_FILE") = py::cast(B_URL_FILE);
}
