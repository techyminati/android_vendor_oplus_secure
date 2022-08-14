#ifndef _PROXY_H
#define _PROXY_H
/// @file proxy.idl
///
#include <AEEStdDef.h>
#include "AEEStdDef.h"
#include "remote.h"
#ifndef __QAIC_HEADER
#define __QAIC_HEADER(ff) ff
#endif //__QAIC_HEADER

#ifndef __QAIC_HEADER_EXPORT
#define __QAIC_HEADER_EXPORT
#endif // __QAIC_HEADER_EXPORT

#ifndef __QAIC_HEADER_ATTRIBUTE
#define __QAIC_HEADER_ATTRIBUTE
#endif // __QAIC_HEADER_ATTRIBUTE

#ifndef __QAIC_IMPL
#define __QAIC_IMPL(ff) ff
#endif //__QAIC_IMPL

#ifndef __QAIC_IMPL_EXPORT
#define __QAIC_IMPL_EXPORT
#endif // __QAIC_IMPL_EXPORT

#ifndef __QAIC_IMPL_ATTRIBUTE
#define __QAIC_IMPL_ATTRIBUTE
#endif // __QAIC_IMPL_ATTRIBUTE
#ifdef __cplusplus
extern "C" {
#endif
#if !defined(__QAIC_DMAHANDLE1_OBJECT_DEFINED__) && !defined(__DMAHANDLE1_OBJECT__)
#define __QAIC_DMAHANDLE1_OBJECT_DEFINED__
#define __DMAHANDLE1_OBJECT__
typedef struct _dmahandle1_s {
   int fd;
   uint32 offset;
   uint32 len;
} _dmahandle1_t;
#endif /* __QAIC_DMAHANDLE1_OBJECT_DEFINED__ */
enum dsp_cmd_type {
   SEC_FP_HVX_ALGO_INIT,
   SEC_FP_HVX_ALGO_DEINIT,
   SEC_FP_HVX_GET_FEATURE_TWO,
#ifdef SUPPORT_DSP_COMPATIBLE_VERSION_G3
   SEC_FP_HVX_GET_FEATURE_FOUR,
   SEC_FP_HVX_AGING_TEST,
   SEC_FP_HVX_GET_VERSION,
   SEC_PROCESS_MAP_BUF,
   SEC_PROCESS_UNMAP_BUF,
   SEC_ENABLE_LOG_DUMP,
   SEC_DISABLE_LOG_DUMP,
   SEC_FP_SET_CHIP_TYPE,
   SEC_CMD_MAX,
#else
   SEC_FP_HVX_GET_FEATURE_FORE,
   SEC_FP_HVX_AGING_TEST,
   SEC_FP_HVX_GET_VERSION,
   SEC_PROCESS_MAP_BUF,
   SEC_PROCESS_UNMAP_BUF,
   SEC_PROCESS_UNKNOWN,
#endif
   _32BIT_PLACEHOLDER_dsp_cmd_type = 0x7fffffff
};
typedef enum dsp_cmd_type dsp_cmd_type;
/**
    * Opens the handle in the specified domain.  If this is the first
    * handle, this creates the session.  Typically this means opening
    * the device, aka open("/dev/adsprpc-smd"), then calling ioctl
    * device APIs to create a PD on the DSP to execute our code in,
    * then asking that PD to dlopen the .so and dlsym the skel function.
    *
    * @param uri, <interface>_URI"&_dom=aDSP"
    *    <interface>_URI is a QAIC generated uri, or
    *    "file:///<sofilename>?<interface>_skel_handle_invoke&_modver=1.0"
    *    If the _dom parameter is not present, _dom=DEFAULT is assumed
    *    but not forwarded.
    *    Reserved uri keys:
    *      [0]: first unamed argument is the skel invoke function
    *      _dom: execution domain name, _dom=mDSP/aDSP/DEFAULT
    *      _modver: module version, _modver=1.0
    *      _*: any other key name starting with an _ is reserved
    *    Unknown uri keys/values are forwarded as is.
    * @param h, resulting handle
    * @retval, 0 on success
    */
__QAIC_HEADER_EXPORT int __QAIC_HEADER(proxy_open)(const char* uri, remote_handle64* h) __QAIC_HEADER_ATTRIBUTE;
/** 
    * Closes a handle.  If this is the last handle to close, the session
    * is closed as well, releasing all the allocated resources.

    * @param h, the handle to close
    * @retval, 0 on success, should always succeed
    */
__QAIC_HEADER_EXPORT int __QAIC_HEADER(proxy_close)(remote_handle64 h) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(proxy_init)(remote_handle64 _h) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(proxy_send_command)(remote_handle64 _h, dsp_cmd_type cmd_type) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(proxy_mapunmap_phys)(remote_handle64 _h, dsp_cmd_type cmd_type, int din, uint32 dinOffset, uint32 dinLen) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(proxy_deinit)(remote_handle64 _h) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(proxy_hap_disable_clocks)(remote_handle64 _h) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(proxy_hap_set_clocks)(remote_handle64 _h, int32_t power_level, int32_t latency, int32_t dcvs_enable) __QAIC_HEADER_ATTRIBUTE;
#ifndef proxy_URI
#define proxy_URI "file:///libproxy_skel.so?proxy_skel_handle_invoke&_modver=1.0"
#endif /*proxy_URI*/
#ifdef __cplusplus
}
#endif
#endif //_PROXY_H
