--TEST--
alias functionality
--SKIPIF--
<?php if (PHP_VERSION_ID < 50300) die("Skipped: PHP 5.3+ required."); ?>
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

echo "*** Decompression ***", PHP_EOL;
$compressed = xz_compress($data);
var_dump(xz_uncompress($compressed) === $data);
var_dump(xz_decompress($compressed) === $data);

echo "*** Namespace Decompression ***", PHP_EOL;
$compressed = \Xz\compress($data);
var_dump(\Xz\uncompress($compressed) === $data);
var_dump(\Xz\decompress($compressed) === $data);
?>
===Done===
--EXPECT--
*** Decompression ***
bool(true)
bool(true)
*** Namespace Decompression ***
bool(true)
bool(true)
===Done===
