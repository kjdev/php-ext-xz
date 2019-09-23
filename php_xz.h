#ifndef PHP_XZ_H
#define PHP_XZ_H

#define PHP_XZ_EXT_VERSION "0.1.0"
#define PHP_XZ_NS "Xz"

extern zend_module_entry xz_module_entry;
#define phpext_xz_ptr &xz_module_entry

#ifdef PHP_WIN32
#   define PHP_XZ_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_XZ_API __attribute__ ((visibility("default")))
#else
#   define PHP_XZ_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(xz)
  zend_long buffer_size;
ZEND_END_MODULE_GLOBALS(xz)

#ifdef ZTS
#define PHP_XZ_G(v) TSRMG(xz_globals_id, zend_xz_globals *, v)
#else
#define PHP_XZ_G(v) (xz_globals.v)
#endif

#if ZEND_MODULE_API_NO >= 20190128
#ifndef TSRMLS_CC
#define TSRMLS_CC
#endif
#ifndef TSRMLS_DC
#define TSRMLS_DC
#endif
#endif

#endif  /* PHP_XZ_H */
