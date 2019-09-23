#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#if ZEND_MODULE_API_NO >= 20141001
#include <ext/standard/php_smart_string.h>
#else
#include <ext/standard/php_smart_str.h>
#endif
#include "php_xz.h"

/* xz */
#include "lzma.h"
#define DEFAULT_COMPRESS_LEVEL LZMA_PRESET_DEFAULT
#define DEFAULT_BUFFER_SIZE 4096

ZEND_DECLARE_MODULE_GLOBALS(xz);

ZEND_INI_BEGIN()
  STD_ZEND_INI_ENTRY("xz.buffer_size", ZEND_TOSTR(DEFAULT_BUFFER_SIZE),
                     ZEND_INI_ALL, OnUpdateLong, buffer_size,
                     zend_xz_globals, xz_globals)
ZEND_INI_END()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xz_compress, 0, 0, 1)
  ZEND_ARG_INFO(0, data)
  ZEND_ARG_INFO(0, level)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_xz_uncompress, 0, 0, 1)
  ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(xz_compress)
{
  zval *data;
  uint8_t *output;
  size_t size, position = 0;
  long level = DEFAULT_COMPRESS_LEVEL;
  lzma_check check = LZMA_CHECK_CRC64;
  lzma_ret result;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                            "z|l", &data, &level) == FAILURE) {
    RETURN_FALSE;
  }

  if (Z_TYPE_P(data) != IS_STRING) {
    zend_error(E_WARNING, "xz_compress: expects parameter to be string.");
    RETURN_FALSE;
  }

  if (level > 9 || level < 0) {
    zend_error(E_WARNING, "xz_compress: compression level (%ld)"
               " must be within 0..9", level);
    RETURN_FALSE;
  }

  size = lzma_stream_buffer_bound(Z_STRLEN_P(data));
  output = (uint8_t *)emalloc(size + 1);
  if (!output) {
    zend_error(E_WARNING, "xz_compress: memory allocate error");
    RETURN_FALSE;
  }

  result = lzma_easy_buffer_encode((uint32_t)level, check, NULL,
                                   (const uint8_t *)Z_STRVAL_P(data),
                                   Z_STRLEN_P(data),
                                   output, &position, size);
  if (result != LZMA_OK) {
    zend_error(E_WARNING, "xz_compress: compression error");
    RETVAL_FALSE;
  } else {
#if ZEND_MODULE_API_NO >= 20141001
    RETVAL_STRINGL(output, position);
#else
    RETVAL_STRINGL(output, position, 1);
#endif
  }

  efree(output);
}

ZEND_FUNCTION(xz_uncompress)
{
  zval *data;
  uint8_t *buffer;
  size_t size = PHP_XZ_G(buffer_size);
#if ZEND_MODULE_API_NO >= 20141001
  smart_string out = {0};
#else
  smart_str out = {0};
#endif
  lzma_stream stream = LZMA_STREAM_INIT;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
                            "z", &data) == FAILURE) {
    RETURN_FALSE;
  }

  if (Z_TYPE_P(data) != IS_STRING) {
    zend_error(E_WARNING, "xz_uncompress: expects parameter to be string.");
    RETURN_FALSE;
  }

  buffer = (uint8_t *)emalloc(size);
  if (!buffer) {
    zend_error(E_WARNING, "xz_uncompress: memory allocate error");
    RETURN_FALSE;
  }

  if (lzma_stream_decoder(&stream, UINT64_MAX, LZMA_CONCATENATED) != LZMA_OK) {
    zend_error(E_WARNING, "xz_uncompress: can not init stream");
    RETURN_FALSE;
  }

  stream.next_in = Z_STRVAL_P(data);
  stream.avail_in = Z_STRLEN_P(data);
  stream.next_out = buffer;
  stream.avail_out = size;

  while (1) {
    lzma_action action = LZMA_RUN;
    if (stream.avail_in == 0) {
      action = LZMA_FINISH;
    }

    lzma_ret ret = lzma_code(&stream, action);
    if (stream.avail_out == 0 || ret == LZMA_STREAM_END) {
      size_t out_size = size - stream.avail_out;
      if (out_size != 0) {
#if ZEND_MODULE_API_NO >= 20141001
        smart_string_appendl(&out, buffer, out_size);
#else
        smart_str_appendl(&out, buffer, out_size);
#endif
      }
      stream.next_out = buffer;
      stream.avail_out = size;
    }
    if (ret == LZMA_STREAM_END) {
      break;
    } else if (ret != LZMA_OK) {
      zend_error(E_WARNING, "xz_uncompress: uncompression error");
      out.len = 0;
      RETVAL_FALSE;
      break;
    }
  }

  lzma_end(&stream);
  efree(buffer);

  if (out.len > 0) {
#if ZEND_MODULE_API_NO >= 20141001
    RETVAL_STRINGL(out.c, out.len);
#else
    RETVAL_STRINGL(out.c, out.len, 1);
#endif
  }

#if ZEND_MODULE_API_NO >= 20141001
  smart_string_free(&out);
#else
  smart_str_free(&out);
#endif
}


typedef struct _php_xz_stream_data {
  lzma_stream strm;
  uint8_t *buf;
  size_t bufsize;
  php_stream *stream;
} php_xz_stream_data;

#define STREAM_DATA_FROM_STREAM() \
  php_xz_stream_data *self = (php_xz_stream_data *) stream->abstract

#define STREAM_NAME "compress.xz"

static int php_xz_decomp_close(php_stream *stream, int close_handle TSRMLS_DC)
{
  STREAM_DATA_FROM_STREAM();

  if (!self) {
    return EOF;
  }

  if (close_handle) {
    if (self->stream) {
      php_stream_close(self->stream);
      self->stream = NULL;
    }
  }

  lzma_end(&self->strm);

  if (self->buf) {
    efree(self->buf);
  }
  efree(self);

  stream->abstract = NULL;

  return EOF;
}

static int php_xz_comp_flush(php_stream *stream TSRMLS_DC)
{
  STREAM_DATA_FROM_STREAM();

  int ret = 0;

  do {
    self->strm.next_out = self->buf;
    self->strm.avail_out = self->bufsize;

    lzma_ret result = lzma_code(&self->strm, LZMA_RUN);
    if (result == LZMA_OK) {
      size_t write = (size_t)(self->strm.next_out - self->buf);
      if (write) {
        php_stream_write(self->stream, self->buf, write);
      }
    } else {
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "xz compress error\n");
      ret = EOF;
    }
  } while (self->strm.avail_in > 0);

  return ret;
}

static int php_xz_comp_close(php_stream *stream, int close_handle TSRMLS_DC)
{
  STREAM_DATA_FROM_STREAM();

  if (!self) {
    return EOF;
  }

  while (1) {
    self->strm.next_out = self->buf;
    self->strm.avail_out = self->bufsize;

    lzma_ret result = lzma_code(&self->strm, LZMA_FINISH);
    if (result == LZMA_OK || result == LZMA_STREAM_END) {
      size_t write = (size_t)(self->strm.next_out - self->buf);
      if (write) {
        php_stream_write(self->stream, self->buf, write);
      }
      if (result == LZMA_STREAM_END) {
        break;
      }
    } else {
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "xz compress error\n");
    }
  }

  if (close_handle) {
    if (self->stream) {
      php_stream_close(self->stream);
      self->stream = NULL;
    }
  }

  lzma_end(&self->strm);

  if (self->buf) {
    efree(self->buf);
  }
  efree(self);

  stream->abstract = NULL;

  return EOF;
}

#if PHP_VERSION_ID < 70400
static size_t php_xz_decomp_read(php_stream *stream, char *buf, size_t count TSRMLS_DC)
{
  size_t ret = 0;
#else
static ssize_t php_xz_decomp_read(php_stream *stream, char *buf, size_t count TSRMLS_DC)
{
  ssize_t ret = 0;
#endif
  STREAM_DATA_FROM_STREAM();

  lzma_action action = LZMA_RUN;
  if (self->strm.avail_in == 0 && !php_stream_eof(self->stream)) {
    self->strm.avail_in = php_stream_read(self->stream,
                                          self->buf + self->strm.avail_in,
                                          self->bufsize - self->strm.avail_in);
    self->strm.next_in = self->buf;
  } else if (php_stream_eof(self->stream)) {
    action = LZMA_FINISH;
  }
  self->strm.avail_out = count;
  self->strm.next_out = (uint8_t *)buf;

  lzma_ret result = lzma_code(&self->strm, action);
  if (result == LZMA_OK || result == LZMA_STREAM_END) {
    ret = (size_t)(self->strm.next_out - (uint8_t *)buf);
  } else {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "xz: uncompression error");
#if PHP_VERSION_ID >= 70400
    return -1;
#else
    return 0;
#endif
  }

  return ret;
}

#if PHP_VERSION_ID < 70400
static size_t php_xz_comp_write(php_stream *stream, const char *buf, size_t count TSRMLS_DC)
{
  size_t ret = 0;
#else
static ssize_t php_xz_comp_write(php_stream *stream, const char *buf, size_t count TSRMLS_DC)
{
  ssize_t ret = 0;
#endif
  STREAM_DATA_FROM_STREAM();

  self->strm.avail_in = count;
  self->strm.next_in = (uint8_t *)buf;

  while (1) {
    self->strm.avail_out = self->bufsize;
    self->strm.next_out = self->buf;

    lzma_ret result = lzma_code(&self->strm, LZMA_RUN);
    if (result == LZMA_OK) {
      size_t write = (size_t)(self->strm.next_out - self->buf);
      if (write) {
        php_stream_write(self->stream, self->buf, write);
      }

      if (self->strm.avail_in == 0) {
        ret = count - self->strm.avail_in;
        break;
      }
    } else {
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "xz compress error\n");
    }
  }

  return ret;
}

static php_stream_ops php_stream_xz_read_ops = {
  NULL,    /* write */
  php_xz_decomp_read,
  php_xz_decomp_close,
  NULL,    /* flush */
  STREAM_NAME,
  NULL,    /* seek */
  NULL,    /* cast */
  NULL,    /* stat */
  NULL     /* set_option */
};

static php_stream_ops php_stream_xz_write_ops = {
  php_xz_comp_write,
  NULL,    /* read */
  php_xz_comp_close,
  php_xz_comp_flush,
  STREAM_NAME,
  NULL,    /* seek */
  NULL,    /* cast */
  NULL,    /* stat */
  NULL     /* set_option */
};

static php_stream *
php_stream_xz_opener(
  php_stream_wrapper *wrapper,
#if PHP_VERSION_ID < 50600
  char *path,
  char *mode,
#else
  const char *path,
  const char *mode,
#endif
  int options,
#if PHP_MAJOR_VERSION < 7
  char **opened_path,
#else
  zend_string **opened_path,
#endif
  php_stream_context *context
  STREAMS_DC TSRMLS_DC)
{
  php_xz_stream_data *self;
  int level = DEFAULT_COMPRESS_LEVEL;
  int compress;

  if (strncasecmp(STREAM_NAME, path, sizeof(STREAM_NAME)-1) == 0) {
    path += sizeof(STREAM_NAME)-1;
    if (strncmp("://", path, 3) == 0) {
      path += 3;
    }
  }

  if (php_check_open_basedir(path TSRMLS_CC)) {
    return NULL;
  }

  if (!strcmp(mode, "w") || !strcmp(mode, "wb")) {
    compress = 1;
  } else if (!strcmp(mode, "r") || !strcmp(mode, "rb")) {
    compress = 0;
  } else {
    php_error_docref(NULL TSRMLS_CC, E_ERROR, "xz: invalid open mode");
    return NULL;
  }

  if (context) {
#if PHP_MAJOR_VERSION >= 7
    zval *tmpzval;
    zend_string *data;

    if (NULL !=
        (tmpzval = php_stream_context_get_option(context, "xz", "level"))) {
      level = zval_get_long(tmpzval);
    }
#else
    zval **tmpzval;

    if (php_stream_context_get_option(context, "xz", "level",
                                      &tmpzval) == SUCCESS) {
      convert_to_long_ex(tmpzval);
      level = Z_LVAL_PP(tmpzval);
    }
#endif
  }

  if (level > 9 || level < 0) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING,
                     "xz: compression level (%d) must be within 0..9", level);
    level = DEFAULT_COMPRESS_LEVEL;
  }

  self = ecalloc(sizeof(*self), 1);
  self->stream = php_stream_open_wrapper(path, mode,
                                         options | REPORT_ERRORS, NULL);
  if (!self->stream) {
    efree(self);
    return NULL;
  }

  self->bufsize = PHP_XZ_G(buffer_size);
  self->buf = emalloc(self->bufsize);
  if (!self->buf) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING,
                     "xz: stream context memory allocate failed");
    php_stream_close(self->stream);
    efree(self);
    return NULL;
  }

  if (compress) {
    lzma_options_lzma opt;
    if (lzma_lzma_preset(&opt, (uint32_t)level)) {
      return NULL;
    }

    lzma_filter filters[] = {
      { .id = LZMA_FILTER_LZMA2, .options = &opt },
      { .id = LZMA_VLI_UNKNOWN, .options = NULL },
    };

    lzma_check check = LZMA_CHECK_CRC64;

    if (lzma_stream_encoder(&self->strm, filters, check) != LZMA_OK) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING,
                       "xz: compression context failed");
      php_stream_close(self->stream);
      efree(self->buf);
      efree(self);
      return NULL;
    }

    return php_stream_alloc(&php_stream_xz_write_ops, self, NULL, mode);

  } else {
    if (lzma_stream_decoder(&self->strm,
                            UINT64_MAX, LZMA_CONCATENATED) != LZMA_OK) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING,
                       "xz: decompression context failed");
      php_stream_close(self->stream);
      efree(self->buf);
      efree(self);
      return NULL;
    }

    return php_stream_alloc(&php_stream_xz_read_ops, self, NULL, mode);
  }

  return NULL;
}

static php_stream_wrapper_ops xz_stream_wops = {
  php_stream_xz_opener,
  NULL,    /* close */
  NULL,    /* fstat */
  NULL,    /* stat */
  NULL,    /* opendir */
  STREAM_NAME,
  NULL,    /* unlink */
  NULL,    /* rename */
  NULL,    /* mkdir */
  NULL     /* rmdir */
#if PHP_VERSION_ID >= 50400
  , NULL
#endif
};

php_stream_wrapper php_stream_xz_wrapper = {
  &xz_stream_wops,
  NULL,
  0 /* is_url */
};

static void
php_xz_init_globals(zend_xz_globals *xz_globals)
{
  xz_globals->buffer_size = DEFAULT_BUFFER_SIZE;
}

ZEND_MINIT_FUNCTION(xz)
{
  ZEND_INIT_MODULE_GLOBALS(xz, php_xz_init_globals, NULL);
  REGISTER_INI_ENTRIES();

  REGISTER_LONG_CONSTANT("XZ_COMPRESS_LEVEL_MIN", 0,
                         CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("XZ_COMPRESS_LEVEL_MAX", 9,
                         CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("XZ_COMPRESS_LEVEL_DEFAULT", DEFAULT_COMPRESS_LEVEL,
                         CONST_CS | CONST_PERSISTENT);

  php_register_url_stream_wrapper(STREAM_NAME, &php_stream_xz_wrapper TSRMLS_CC);

  return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(xz)
{
  UNREGISTER_INI_ENTRIES();
  return SUCCESS;
}

ZEND_MINFO_FUNCTION(xz)
{
  php_info_print_table_start();
  php_info_print_table_row(2, "Xz support", "enabled");
  php_info_print_table_row(2, "Extension Version", PHP_XZ_EXT_VERSION);
  php_info_print_table_row(2, "Interface Version", LZMA_VERSION_STRING);
  php_info_print_table_end();
}

static zend_function_entry xz_functions[] = {
  ZEND_FE(xz_compress, arginfo_xz_compress)
  ZEND_FE(xz_uncompress, arginfo_xz_uncompress)
  ZEND_FALIAS(xz_decompress, xz_uncompress, arginfo_xz_uncompress)

// PHP 5.3+
#if ZEND_MODULE_API_NO >= 20090626
  ZEND_NS_FALIAS(PHP_XZ_NS, compress, xz_compress, arginfo_xz_compress)
  ZEND_NS_FALIAS(PHP_XZ_NS, uncompress, xz_uncompress, arginfo_xz_uncompress)
  ZEND_NS_FALIAS(PHP_XZ_NS, decompress, xz_uncompress, arginfo_xz_uncompress)
#endif
  {NULL, NULL, NULL}
};

zend_module_entry xz_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
  STANDARD_MODULE_HEADER,
#endif
  "xz",
  xz_functions,
  ZEND_MINIT(xz),
  ZEND_MSHUTDOWN(xz),
  NULL,
  NULL,
  ZEND_MINFO(xz),
#if ZEND_MODULE_API_NO >= 20010901
  PHP_XZ_EXT_VERSION,
#endif
  STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_XZ
ZEND_GET_MODULE(xz)
#endif
