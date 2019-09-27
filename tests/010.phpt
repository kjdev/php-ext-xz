--TEST--
compress level constants
--SKIPIF--
--FILE--
<?php
echo 'Min: ', XZ_COMPRESS_LEVEL_MIN, PHP_EOL;
echo 'Max: ', XZ_COMPRESS_LEVEL_MAX, PHP_EOL;
echo 'Default: ', XZ_COMPRESS_LEVEL_DEFAULT, PHP_EOL;
?>
===DONE===
--EXPECT--
Min: 0
Max: 9
Default: 6
===DONE===
