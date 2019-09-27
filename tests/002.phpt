--TEST--
xz_compress(): error conditions
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '8.0', '>=')) die('skip PHP is too old');
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

echo "*** Testing xz_compress() function with Zero arguments ***", PHP_EOL;
var_dump(xz_compress());

echo "*** Testing with incorrect parameters ***", PHP_EOL;

class Tester {
}

$testclass = new Tester();
var_dump(xz_compress($testclass));
?>
===Done===
--EXPECTF--
*** Testing xz_compress() function with Zero arguments ***

Warning: xz_compress() expects at least 1 parameter, 0 given in %s on line %d
bool(false)
*** Testing with incorrect parameters ***

Warning: xz_compress: expects parameter to be string. in %s on line %d
bool(false)
===Done===
