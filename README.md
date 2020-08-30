## RocksDB PHP Extension
This is a php extension for [rocksdb](https://github.com/facebook/rocksdb)

[ide-helper](https://github.com/isliang/rocksdb-ide-helper)

## Simple Example

```php
use \RocksDB\RocksDB;

try {
    $db = new RocksDB("/path/to/rocksdb");
    //open rocksdb
    if (!$db->open()) {
        echo $db->lastError();
        return;
    }
    //$db->open($readonly=true); //open rocksdb readonly
    //put the value of key "hello" to "world"
    if(!$db->put("hello", "world")) {
        echo $db->lastError();
        return;        
    }
    if(!$db->put("welcome", "db")) {
        echo $db->lastError();
        return;        
    }
    //get the value of key "hello"
    $value = $db->get("hello");
    if ($value === false) {
        echo $db->lastError();
        return;    
    } else {
        echo $value;
    }
    $value = $db->mGet(["hello", "welcome"]);
    if ($value === false) {
        echo $db->lastError();
        return;    
    } else {
        print_r($value);
    }
} catch(\Throwable $e) {
    echo $e->getMessage();
} 
```
