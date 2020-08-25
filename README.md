# XZ Extension for PHP

[![Build Status](https://secure.travis-ci.org/kjdev/php-ext-xz.png?branch=master)](https://travis-ci.org/kjdev/php-ext-xz)

This extension allows XZ (liblzma).

Documentation for XZ can be found at [» https://tukaani.org/xz](https://tukaani.org/xz/).


## Build from sources

``` bash
% git clone --depth=1 https://github.com/kjdev/php-ext-xz.git
% phpize
% ./configure
% make
% make install
```

> You need to install xz-devel using dnf/yum (Fedora, RHEL, CentOS...),
> or liblzma-dev using apt-get or aptitude (Debian, Ubuntu...)

## Configration

php.ini:

```
extension=xz.so
```

## Constant

Name                         | Description
-----------------------------| ----------------------------
XZ\_COMPRESS\_LEVEL\_MIN     | Minimal compress level value
XZ\_COMPRESS\_LEVEL\_MAX     | Maximal compress level value
XZ\_COMPRESS\_LEVEL\_DEFAULT | Default compress level value

## Function

* xz\_compress — XZ compression
* xz\_uncompress — XZ decompression

### xz\_compress — XZ compression

#### Description

string **zstd\_compress** ( string _$data_ [, int _$level_ = 6 ] )

XZ compression.

#### Pameters

* _data_

  The string to compress.

* _level_

  The level of compression (0-9).
  (Defaults to 6)

#### Return Values

Returns the compressed data or FALSE if an error occurred.


### xz\_uncompress — XZ decompression

#### Description

string **xz\_uncompress** ( string _$data_ )

XZ decompression.

> Alias: xz\_decompress

#### Pameters

* _data_

  The compressed string.

#### Return Values

Returns the decompressed data or FALSE if an error occurred.


## Namespace

```
Namespace Xz;

function compress( $data [, $level = 6 ] )
function uncompress( $data )
```

`xz_compress` and `xz_uncompress` function alias.

## Streams

XZ compression and uncompression are available using the
`compress.xz://` stream prefix.

## Examples

```php
// Using functions
$data = xz_compress('test');
xz_uncompress($data);

// Using namespaced functions
$data = \Xz\compress('test');
\Xz\uncompress($data);

// Using streams
file_put_contents('compress.xz:///patch/to/data.xz', $data);
readfile('compress.xz:///patch/to/data.xz');
```
