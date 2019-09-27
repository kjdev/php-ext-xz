--TEST--
phpinfo() displays xz info
--SKIPIF--
--FILE--
<?php
phpinfo();
--EXPECTF--
%a
xz

Xz support => enabled
Extension Version => %d.%d.%d
Interface Version => %d.%d.%d
%a
