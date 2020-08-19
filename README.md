## RocksDB PHP Extension
This is a php extension for [rocksdb](https://github.com/facebook/rocksdb)

## Simple Example

```php
try {
    $idb = new IDB("/path/to/rocksdb");
    //open rocksdb
    if (!$idb->open()) {
        echo $idb->lastError();
        return;
    }
    //$idb->open($readonly=true); //open rocksdb readonly
    //put the value of key "hello" to "world"
    if(!$idb->put("hello", "world")) {
        echo $idb->lastError();
        return;        
    }
    if(!$idb->put("welcome", "idb")) {
        echo $idb->lastError();
        return;        
    }
    //get the value of key "hello"
    $value = $idb->get("hello");
    if ($value === false) {
        echo $idb->lastError();
        return;    
    } else {
        echo $value;
    }
    $value = $idb->mGet(["hello", "welcome"]);
    if ($value === false) {
        echo $idb->lastError();
        return;    
    } else {
        print_r($value);
    }
} catch(\Throwable $e) {
    echo $e->getMessage();
} 
```
