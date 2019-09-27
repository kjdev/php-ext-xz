--TEST--
xz_compress(): basic functionality
--SKIPIF--
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

// Initialise all required variables
$smallstring = "A small string to compress\n";

// Calling xz_compress() with all possible arguments
$level = 1;

// Compressing a big string
echo "*** Compression ***", PHP_EOL;
$output = xz_compress($data, $level);
var_dump(xz_uncompress($output) === $data);

// Compressing a smaller string
echo "*** Compression ***", PHP_EOL;
$output = xz_compress($smallstring, $level);
var_dump(xz_uncompress($output) === $smallstring);

?>
===Done===
--EXPECTF--
*** Compression ***
bool(true)
*** Compression ***
bool(true)
===Done===
