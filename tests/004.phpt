--TEST--
xz_uncompress(): basic functionality
--SKIPIF--
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

// Initialise all required variables
$compressed = xz_compress($data);

echo "*** Basic decompress ***", PHP_EOL;
var_dump($data === xz_uncompress($compressed));
?>
===DONE===
--EXPECT--
*** Basic decompress ***
bool(true)
===DONE===
