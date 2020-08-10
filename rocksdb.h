#ifndef IDB_ROCKSDB_H
#define IDB_ROCKSDB_H
#include <rocksdb/db.h>
#include <rocksdb/rocksdb_namespace.h>

using namespace ROCKSDB_NAMESPACE;
using namespace std;

struct ValueData {
    zend_bool is_success;
    string value;
};

class RocksDB
{
private:
    DB* m_rdb;
    Status status;
    zend_bool is_open = false;
    string m_last_error;
    char *path;
public:
    zend_bool open(zend_bool readonly, Options options);
    zend_bool put(char *key, char *value);
    ValueData get(char *key);
    zend_string* lastError(void);
    void setPath(char *_path);
    void close(void);
};
#endif //IDB_ROCKSDB_H
