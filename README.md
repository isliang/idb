## RocksDB PHP Extension
This is a php extension for [rocksdb](https://github.com/facebook/rocksdb)

## Simple Example

```php
try {
    $idb = new IDB("/path/to/rocksdb");
    //open rocksdb
    $idb->open();
    //$idb->open(true); //open rocksdb readonly
    //put the value of key "hello" to "world"
    echo $idb->put("hello", "world");
    //get the value of key "hello"
    echo $idb->get("hello");
} catch(Exception $e) {
    return false;
} 
```
