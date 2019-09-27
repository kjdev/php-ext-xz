--TEST--
xz_compress(): compress level
--SKIPIF--
<?php
include(dirname(__FILE__) . '/version_number.inc');
if ($xz_version_number < 10304) die("Skipped: using Zstandard 1.3.4 or older.");
?>
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');
function check_compress($data, $level)
{
  $output = (string)xz_compress($data, $level);
  echo $level, ' -- ',
    var_export(xz_uncompress($output) === $data, true), PHP_EOL;
}

echo "*** Data size ***", PHP_EOL;
echo strlen($data), PHP_EOL;

echo "*** Compression Level ***", PHP_EOL;
for ($level = XZ_COMPRESS_LEVEL_MIN; $level <= XZ_COMPRESS_LEVEL_MAX; $level++) {
  check_compress($data, $level);
}

echo "*** Faster compression Level ***", PHP_EOL;
for ($level = -1; $level >= -5; $level--) {
  check_compress($data, $level);
}

echo "*** Invalid Compression Level ***", PHP_EOL;
check_compress($data, 100);
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
*** Faster compression Level ***
-1 -- true
-2 -- true
-3 -- true
-4 -- true
-5 -- true
*** Invalid Compression Level ***

Warning: xz_compress: compression level (100) must be within 0..9 or smaller then 0 in %s on line %d
100 --
Warning: xz_uncompress: it was not compressed by xz in %s
false
===Done===
