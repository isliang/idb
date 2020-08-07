## RocksDB PHP Extension
This is a php extension for [rocksdb](https://github.com/facebook/rocksdb)

## Simple Example

```php
try {
    $idb = new IDB("/path/to/rocksdb");
    //open rocksdb, param true for open rocksdb readonly, false for open rocksdb for read and write
    //default param value is false
    $idb->open(false);//or$idb->open(); 
    //put the value of key "hello" to "world"
    $idb->put("hello", "world");
    //get the value of key "hello"
    echo $idb->get("hello");
} catch(Exception $e) {
    return false;
} 
```
