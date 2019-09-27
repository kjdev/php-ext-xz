--TEST--
namespace: Xz\compress()/uncompress()
--SKIPIF--
<?php if (PHP_VERSION_ID < 50300) die("Skipped: PHP 5.3+ required."); ?>
--FILE--
<?php
include(dirname(__FILE__) . '/data.inc');

// Initialise all required variables
$smallstring = "A small string to compress\n";


// Calling \Xz\compress() with all possible arguments
$level = 1;

// Compressing a big string
echo "*** Compression ***", PHP_EOL;
$output = \Xz\compress($data, $level);
var_dump(\Xz\uncompress($output) === $data);

// Compressing a smaller string
echo "*** Compression ***", PHP_EOL;
$output = \Xz\compress($smallstring, $level);
var_dump(\Xz\uncompress($output) === $smallstring);
?>
===Done===
--EXPECTF--
*** Compression ***
bool(true)
*** Compression ***
bool(true)
===Done===
