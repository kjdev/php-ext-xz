--TEST--
xz_uncompress(): error conditions
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '8.0', '>=')) die('skip PHP is too old');
--FILE--
<?php

echo "*** Testing xz_uncompress() function with Zero arguments ***", PHP_EOL;
var_dump( xz_uncompress() );

echo "*** Testing with incorrect arguments ***", PHP_EOL;
var_dump(xz_uncompress(123));

class Tester
{}

$testclass = new Tester();
var_dump(xz_uncompress($testclass));
?>
===DONE===
--EXPECTF--
*** Testing xz_uncompress() function with Zero arguments ***

Warning: xz_uncompress() expects exactly 1 parameter, 0 given in %s on line %d
bool(false)
*** Testing with incorrect arguments ***

Warning: xz_uncompress: expects parameter to be string. in %s on line %d
bool(false)

Warning: xz_uncompress: expects parameter to be string. in %s on line %d
bool(false)
===DONE===
