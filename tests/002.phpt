--TEST--
idb_test1() Basic test
--SKIPIF--
<?php
if (!extension_loaded('idb')) {
	echo 'skip';
}
?>
--FILE--
<?php
$ret = idb_test1();

var_dump($ret);
?>
--EXPECT--
The extension idb is loaded and working!
NULL
