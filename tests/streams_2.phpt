--TEST--
compress.xz streams and compatibility
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

$file = dirname(__FILE__) . '/data.xz';

echo "Stream compression + xz_uncompress\n";

var_dump(file_put_contents('compress.xz://' . $file, $data) == strlen($data));

$actual = xz_uncompress(file_get_contents($file));
var_dump($actual === $data);

@unlink($file);

echo "xz_compress + Stream decompression\n";

var_dump(file_put_contents($file, xz_compress($data)));
$decomp = file_get_contents('compress.xz://' . $file);
var_dump($actual == $data);

@unlink($file);
?>
===Done===
--EXPECTF--
Stream compression + xz_uncompress
bool(true)
bool(true)
xz_compress + Stream decompression
int(%d)
bool(true)
===Done===
