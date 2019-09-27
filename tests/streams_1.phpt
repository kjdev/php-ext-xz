--TEST--
compress.xz streams basic
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

$file = dirname(__FILE__) . '/data.xz';

echo "Compression with defaul level\n";

var_dump(file_put_contents('compress.xz://' . $file, $data) == strlen($data));
var_dump($size1 = filesize($file));
var_dump($size1 > 1 && $size1 < strlen($data));

echo "Compression with specfic level\n";

$ctx = stream_context_create(
  array(
    "xz" => array(
      "level" => XZ_COMPRESS_LEVEL_MAX,
    )
  )
);

var_dump(file_put_contents('compress.xz://' . $file, $data, 0, $ctx) == strlen($data));
var_dump($size2 = filesize($file));
var_dump($size2 > 1 && $size2 <= $size1);


echo "Decompression\n";

$decomp = file_get_contents('compress.xz://' . $file);
var_dump($decomp == $data);

@unlink($file);
?>
===Done===
--EXPECTF--
Compression with defaul level
bool(true)
int(%d)
bool(true)
Compression with specfic level
bool(true)
int(%d)
bool(true)
Decompression
bool(true)
===Done===
