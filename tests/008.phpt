--TEST--
xz_compress(): compress level
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

function check_compress($data, $level)
{
  $output = (string)xz_compress($data, $level);
  $result = (xz_uncompress($output) === $data);
  echo $level, ' -- ',  var_export($result, true), PHP_EOL;
}

echo "*** Data size ***", PHP_EOL;
echo strlen($data), PHP_EOL;

echo "*** Compression Level ***", PHP_EOL;
for ($level = XZ_COMPRESS_LEVEL_MIN; $level <= XZ_COMPRESS_LEVEL_MAX; $level++) {
  check_compress($data, $level);
}

echo "*** Invalid Compression Level ***", PHP_EOL;
check_compress($data, 100);
check_compress($data, -1);
?>
===Done===
--EXPECTF--
*** Data size ***
3547
*** Compression Level ***
0 -- true
1 -- true
2 -- true
3 -- true
4 -- true
5 -- true
6 -- true
7 -- true
8 -- true
9 -- true
*** Invalid Compression Level ***

Warning: xz_compress: compression level (100) must be within 0..9 in %s on line %d

Warning: xz_uncompress: uncompression error in %s on line 7
100 -- false

Warning: xz_compress: compression level (-1) must be within 0..9 in %s on line %d

Warning: xz_uncompress: uncompression error in %s on line 7
-1 -- false
===Done===
