#ifndef ROCKSDB_ROCKSDB_H
#define ROCKSDB_ROCKSDB_H
#include <rocksdb/db.h>

using namespace rocksdb;
using namespace std;

class RocksDB
{
private:
    DB* m_rdb;
    bool is_open = false;
    string m_last_error;
    char *path;
    vector<string> column_families;
    vector<ColumnFamilyHandle*> handles;
    int handleIndex(string column_family);
public:
    zend_bool open(zend_bool readonly, Options options);
    zend_bool put(char *key, char *value);
    zend_bool put(string column_family, char *key, char *value);
    zend_bool get(char *key, string* value);
    zend_bool get(string column_family, char *key, string* value);
    zend_bool mGet(vector<Slice>& keys, vector<zval *>* values);
    zend_bool mGet(vector<string>& column_families, vector<Slice>& keys, vector<zval *>* values);
    char* lastError(void);
    void setPath(char *_path);
    void close(void);
};
#endif //ROCKSDB_ROCKSDB_H
