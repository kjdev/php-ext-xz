--TEST--
unexpected exiting when uncompress the wrong format data
--SKIPIF--
--FILE--
<?php
$str = 'message string';
$input = base64_decode($str);

echo "*** Compress and Uncompress ***", PHP_EOL;
$output = xz_uncompress(xz_compress($input));
var_dump($input === $output);

echo "*** Uncompress ***", PHP_EOL;
$output = xz_uncompress($input);
var_dump($input === $output);
var_dump($output);
?>
===Done===
--EXPECTF--
*** Compress and Uncompress ***
bool(true)
*** Uncompress ***

Warning: xz_uncompress: uncompression error in %s on line %d
bool(false)
bool(false)
===Done===
