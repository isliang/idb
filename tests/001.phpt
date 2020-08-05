--TEST--
Check if idb is loaded
--SKIPIF--
<?php
if (!extension_loaded('idb')) {
	echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "idb" is available';
?>
--EXPECT--
The extension "idb" is available
