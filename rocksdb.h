#ifndef IDB_ROCKSDB_H
#define IDB_ROCKSDB_H
#include <rocksdb/db.h>

struct ValueData {
    zend_bool is_success;
    std::string value;
};

class RocksDB
{
private:
    rocksdb::Status status;
    zend_bool is_open = false;
    std::string m_last_error;
    char *path;
public:
    zend_bool open(zend_bool readonly, rocksdb::Options options);
    zend_bool put(char *key, char *value);
    ValueData get(char *key);
    zend_string* lastError(void);
    void setPath(char *_path);
};
#endif //IDB_ROCKSDB_H
