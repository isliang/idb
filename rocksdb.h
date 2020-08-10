#ifndef IDB_ROCKSDB_H
#define IDB_ROCKSDB_H
#include <rocksdb/db.h>
#include <rocksdb/rocksdb_namespace.h>

using namespace ROCKSDB_NAMESPACE;
using namespace std;

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
    zend_bool get(char *key, string* value);
    vector<zend_bool> mGet(vector<Slice>& keys, vector<string>* values);
    zend_string* lastError(void);
    void setPath(char *_path);
    void close(void);
};
#endif //IDB_ROCKSDB_H
