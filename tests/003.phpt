--TEST--
xz_compress(): variation
--SKIPIF--
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

echo "*** Testing multiple compression ***", PHP_EOL;
$level = 1;
$output = xz_compress($data, $level);
$output2 = xz_compress($output);
var_dump(xz_uncompress($output) === $data);
var_dump(xz_uncompress($output2) === $output);
?>
===Done===
--EXPECTF--
*** Testing multiple compression ***
bool(true)
bool(true)
===Done===
