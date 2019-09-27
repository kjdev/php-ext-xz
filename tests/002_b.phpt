--TEST--
xz_compress(): error conditions
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '8.0', '<')) die('skip PHP is too new');
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

echo "*** Testing xz_compress() function with Zero arguments ***", PHP_EOL;
try {
  var_dump(xz_compress());
} catch (Error $e) {
  echo $e, PHP_EOL;
}

echo "*** Testing with incorrect parameters ***", PHP_EOL;

class Tester {
}

$testclass = new Tester();
try {
  var_dump(xz_compress($testclass));
} catch (Error $e) {
  echo $e, PHP_EOL;
}
?>
===Done===
--EXPECTF--
*** Testing xz_compress() function with Zero arguments ***
ArgumentCountError: xz_compress() expects at least 1 parameter, 0 given in %s:%d
Stack trace:
#0 %s(%d): xz_compress()
#1 {main}
*** Testing with incorrect parameters ***

Warning: xz_compress: expects parameter to be string. in %s on line %d
bool(false)
===Done===
