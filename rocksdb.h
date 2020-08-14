#ifndef IDB_ROCKSDB_H
#define IDB_ROCKSDB_H
#include <rocksdb/db.h>

using namespace rocksdb;
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
    zend_bool mGet(vector<Slice>& keys, vector<zval *>* values);
    zend_string* lastError(void);
    void setPath(char *_path);
    void close(void);
};
#endif //IDB_ROCKSDB_H
