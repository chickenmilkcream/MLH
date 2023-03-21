#ifndef AVL_NODE_H
#define AVL_NODE_H

#include <memory>
#include <vector>

#define DB_KEY_MIN INT64_MIN
#define DB_KEY_MAX INT64_MAX

using namespace std;

typedef int64_t db_key_t;
typedef int64_t db_val_t;

class AVLNode : public enable_shared_from_this<AVLNode>
{
public:
  db_key_t key;
  db_val_t val;

  AVLNode(db_key_t key, db_val_t val);
  bool has(db_key_t key);
  db_val_t get(db_key_t key);
  shared_ptr<AVLNode> put(db_key_t key, db_val_t val);
  void scan(db_key_t min_key, db_key_t max_key,
            vector<pair<db_key_t, db_val_t> > &pairs);
  void print(string prefix, bool is_left);

private:
  shared_ptr<AVLNode> left;
  shared_ptr<AVLNode> right;
  size_t height;

  shared_ptr<AVLNode> rotate_left();
  shared_ptr<AVLNode> rotate_right();
};

#endif
