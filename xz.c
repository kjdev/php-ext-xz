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
/*
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
*/
#include "lzma.h"
/*
#define FRAME_HEADER_SIZE 5
#define BLOCK_HEADER_SIZE 3
#define MAX_HEADER_SIZE FRAME_HEADER_SIZE+3
*/
#define DEFAULT_COMPRESS_LEVEL LZMA_PRESET_DEFAULT

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
    zend_error(E_WARNING, "xz_compress: memory error");
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
  size_t size = 4096;
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
    zend_error(E_WARNING, "xz_uncompress: memory error");
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
      size_t used_out = size - stream.avail_out;
      if (used_out != 0) {
#if ZEND_MODULE_API_NO >= 20141001
        smart_string_appendl(&out, buffer, used_out);
#else
        smart_str_appendl(&out, buffer, used_out);
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

#if 0
typedef struct _php_xz_stream_data {
    char *bufin, *bufout;
    size_t sizein, sizeout;
    XZ_CCtx* cctx;
    XZ_DCtx* dctx;
    XZ_inBuffer input;
    XZ_outBuffer output;
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

    XZ_freeDCtx(self->dctx);
    efree(self->bufin);
    efree(self->bufout);
    efree(self);
    stream->abstract = NULL;

    return EOF;
}

static int php_xz_comp_flush_or_end(php_xz_stream_data *self, int end TSRMLS_DC)
{
    size_t res;
    int ret = 0;

#if XZ_VERSION_NUMBER < 10400
    /* Compress remaining data */
    if (self->input.size)  {
        self->input.pos = 0;
        do {
            self->output.size = self->sizeout;
            self->output.pos  = 0;
            res = XZ_compressStream(self->cctx, &self->output, &self->input);
            if (XZ_isError(res)) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "libxz error %s\n", XZ_getErrorName(res));
                ret = EOF;
            }
            php_stream_write(self->stream, self->bufout, self->output.pos);
        } while (self->input.pos != self->input.size);
    }
#endif

    /* Flush / End */
    do {
        self->output.size = self->sizeout;
        self->output.pos  = 0;
#if XZ_VERSION_NUMBER >= 10400
        res = XZ_compressStream2(self->cctx, &self->output, &self->input, end ? XZ_e_end : XZ_e_flush);
#else
        if (end) {
            res = XZ_endStream(self->cctx, &self->output);
        } else {
            res = XZ_flushStream(self->cctx, &self->output);
        }
#endif
        if (XZ_isError(res)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "libxz error %s\n", XZ_getErrorName(res));
            ret = EOF;
        }
        php_stream_write(self->stream, self->bufout, self->output.pos);
    } while (res > 0);

    self->input.pos = 0;
    self->input.size = 0;

    return ret;
}


static int php_xz_comp_flush(php_stream *stream TSRMLS_DC)
{
    STREAM_DATA_FROM_STREAM();

    return php_xz_comp_flush_or_end(self, 0 TSRMLS_CC);
}


static int php_xz_comp_close(php_stream *stream, int close_handle TSRMLS_DC)
{
    STREAM_DATA_FROM_STREAM();

    if (!self) {
        return EOF;
    }

    php_xz_comp_flush_or_end(self, 1 TSRMLS_CC);

    if (close_handle) {
        if (self->stream) {
            php_stream_close(self->stream);
            self->stream = NULL;
        }
    }

    XZ_freeCCtx(self->cctx);
    efree(self->bufin);
    efree(self->bufout);
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
    size_t x, res;
    STREAM_DATA_FROM_STREAM();

    while (count > 0) {
        x = self->output.size - self->output.pos;
        /* enough available */
        if (x >= count) {
            memcpy(buf, self->bufout + self->output.pos, count);
            self->output.pos += count;
            ret += count;
            return ret;
        }
        /* take remaining from out  */
        if (x) {
            memcpy(buf, self->bufout + self->output.pos, x);
            self->output.pos += x;
            ret += x;
            buf += x;
            count -= x;
        }
        /* decompress */
        if (self->input.pos < self->input.size) {
            /* for xz */
            self->output.pos = 0;
            self->output.size = self->sizeout;
            res = XZ_decompressStream(self->dctx, &self->output , &self->input);
            if (XZ_isError(res)) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "libxz error %s\n", XZ_getErrorName(res));
#if PHP_VERSION_ID >= 70400
                return -1;
#endif
            }
            /* for us */
            self->output.size = self->output.pos;
            self->output.pos = 0;
        }  else {
            /* read */
            self->input.pos = 0;
            self->input.size = php_stream_read(self->stream, self->bufin, self->sizein);
            if (!self->input.size) {
                /* EOF */
                count = 0;
            }
        }
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
    size_t x, res;

    STREAM_DATA_FROM_STREAM();

    while(count > 0) {
        /* enough room for full data */
        if (self->input.size + count < self->sizein) {
            memcpy(self->bufin + self->input.size, buf, count);
            self->input.size += count;
            ret += count;
            count = 0;
            break;
        }

        /* fill input buffer */
        x = self->sizein - self->input.size;
        memcpy(self->bufin + self->input.size, buf, x);
        self->input.size += x;
        buf += x;
        count -= x;
        ret += x;

        /* compress and write */
        self->input.pos = 0;
        do {
            self->output.size = self->sizeout;
            self->output.pos  = 0;
#if XZ_VERSION_NUMBER >= 10400
            res = XZ_compressStream2(self->cctx, &self->output, &self->input, XZ_e_continue);
#else
            res = XZ_compressStream(self->cctx, &self->output, &self->input);
#endif
            if (XZ_isError(res)) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "libxz error %s\n", XZ_getErrorName(res));
#if PHP_VERSION_ID >= 70400
                return -1;
#endif
            }
            php_stream_write(self->stream, self->bufout, self->output.pos);
        } while (self->input.pos != self->input.size);

        self->input.pos = 0;
        self->input.size = 0;
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
    int level = XZ_CLEVEL_DEFAULT;
    int compress;
#if XZ_VERSION_NUMBER >= 10400
    XZ_CDict *cdict = NULL;
    XZ_DDict *ddict = NULL;
#endif

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

        if (NULL != (tmpzval = php_stream_context_get_option(context, "xz", "level"))) {
            level = zval_get_long(tmpzval);
        }
#if XZ_VERSION_NUMBER >= 10400
        if (NULL != (tmpzval = php_stream_context_get_option(context, "xz", "dict"))) {
            data = zval_get_string(tmpzval);
            if (compress) {
                cdict = XZ_createCDict(ZSTR_VAL(data), ZSTR_LEN(data), level);
            } else {
                ddict = XZ_createDDict(ZSTR_VAL(data), ZSTR_LEN(data));
            }
            zend_string_release(data);
        }
#endif
#else
        zval **tmpzval;

        if (php_stream_context_get_option(context, "xz", "level", &tmpzval) == SUCCESS) {
            convert_to_long_ex(tmpzval);
            level = Z_LVAL_PP(tmpzval);
        }
#if XZ_VERSION_NUMBER >= 10400
        if (php_stream_context_get_option(context, "xz", "dict", &tmpzval) == SUCCESS) {
            convert_to_string(*tmpzval);
            if (compress) {
                cdict = XZ_createCDict(Z_STRVAL_PP(tmpzval), Z_STRLEN_PP(tmpzval), level);
            } else {
                ddict = XZ_createDDict(Z_STRVAL_PP(tmpzval), Z_STRLEN_PP(tmpzval));
            }
        }
#endif
#endif
    }

    if (level > XZ_maxCLevel()) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "xz: compression level (%d) must be less than %d", level, XZ_maxCLevel());
        level = XZ_maxCLevel();
    }

    self = ecalloc(sizeof(*self), 1);
    self->stream = php_stream_open_wrapper(path, mode, options | REPORT_ERRORS, NULL);
    if (!self->stream) {
        efree(self);
        return NULL;
    }

    /* File */
    if (compress) {
        self->dctx = NULL;
        self->cctx = XZ_createCCtx();
        if (!self->cctx) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "xz: compression context failed");
            php_stream_close(self->stream);
            efree(self);
            return NULL;
        }
#if XZ_VERSION_NUMBER >= 10400
        XZ_CCtx_reset(self->cctx, XZ_reset_session_only);
        XZ_CCtx_refCDict(self->cctx, cdict);
        XZ_CCtx_setParameter(self->cctx, XZ_c_compressionLevel, level);
#else
        XZ_initCStream(self->cctx, level);
#endif
        self->bufin = emalloc(self->sizein = XZ_CStreamInSize());
        self->bufout = emalloc(self->sizeout = XZ_CStreamOutSize());
        self->input.src  = self->bufin;
        self->input.pos   = 0;
        self->input.size  = 0;
        self->output.dst = self->bufout;
        self->output.pos  = 0;
        self->output.size = 0;

        return php_stream_alloc(&php_stream_xz_write_ops, self, NULL, mode);

    } else {
        self->dctx = XZ_createDCtx();
        if (!self->dctx) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "xz: compression context failed");
            php_stream_close(self->stream);
            efree(self);
            return NULL;
        }
        self->cctx = NULL;
        self->bufin = emalloc(self->sizein = XZ_DStreamInSize());
        self->bufout = emalloc(self->sizeout = XZ_DStreamOutSize());
#if XZ_VERSION_NUMBER >= 10400
        XZ_DCtx_reset(self->dctx, XZ_reset_session_only);
        XZ_DCtx_refDDict(self->dctx, ddict);
#else
        XZ_initDStream(self->dctx);
#endif
        self->input.src   = self->bufin;
        self->input.pos   = 0;
        self->input.size  = 0;
        self->output.dst  = self->bufout;
        self->output.pos  = 0;
        self->output.size = 0;

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
    NULL    /* rmdir */
#if PHP_VERSION_ID >= 50400
    , NULL
#endif
};


php_stream_wrapper php_stream_xz_wrapper = {
    &xz_stream_wops,
    NULL,
    0 /* is_url */
};
#endif

ZEND_MINIT_FUNCTION(xz)
{
  /*
  REGISTER_LONG_CONSTANT("XZ_COMPRESS_LEVEL_MIN",
                         1,
                         CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("XZ_COMPRESS_LEVEL_MAX",
                         XZ_maxCLevel(),
                         CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("XZ_COMPRESS_LEVEL_DEFAULT",
                         DEFAULT_COMPRESS_LEVEL,
                         CONST_CS | CONST_PERSISTENT);

  REGISTER_LONG_CONSTANT("LIBXZ_VERSION_NUMBER",
                         XZ_VERSION_NUMBER,
                         CONST_CS | CONST_PERSISTENT);
  REGISTER_STRING_CONSTANT("LIBXZ_VERSION_STRING",
                         XZ_VERSION_STRING,
                         CONST_CS | CONST_PERSISTENT);

  php_register_url_stream_wrapper(STREAM_NAME, &php_stream_xz_wrapper TSRMLS_CC);
  */
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
  NULL,
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
