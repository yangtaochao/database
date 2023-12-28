#include <iostream>
#include <vector>
#include <string>
using namespace std;
// 二叉排序树的结点
template <class K, typename V>
struct BSTreeNode
{
    BSTreeNode<K, V> *_left;
    BSTreeNode<K, V> *_right;
    K _key; // 值域
    V _val;
    BSTreeNode(const K &key, const V &val)
        : _left(nullptr), _right(nullptr),
          _key(key),
          _val(val)
    {
    }
};

// 二叉排序树
template <class K, typename V>
class BSTree
{
    typedef BSTreeNode<K, V> Node;
    string filename;

public:
    BSTree(string filename) // 构造函数
        : _root(nullptr)
    {
        this->filename = filename;
        save_tree();
    }
    BSTree(string filename, ifstream &idxfile)
    {

        this->filename = filename;
        _loadTree(idxfile, _root);
    }
    ~BSTree() // 析构函数
    {
        _destroy(_root);
    }
    /*
    向二叉搜树插入数据，一定插入到叶子结点的位置
    这个操作就类似单链表的插入链接好关系即可
    */
    bool Insert(const K &key, const V &val)
    {
        if (_root == nullptr)
        {
            _root = new Node(key, val);
            save_tree();
            return true;
        }
        Node *prev = nullptr;
        Node *cur = _root;
        // 寻找要插入的位置
        while (cur)
        {
            if (cur->_key < key)
            {
                prev = cur;
                cur = cur->_right;
            }
            else if (cur->_key > key)
            {
                prev = cur;
                cur = cur->_left;
            }
            else
            {
                return false;
            }
        }
        // 退出循环表示cur到空，那么就开辟要插入结点的值
        cur = new Node(key, val);
        // 到底链接到prev的左子树还是右子树，那么就判断以下叭
        if (prev->_key < key)
        {
            prev->_right = cur;
        }
        else
        {
            prev->_left = cur;
        }
        save_tree();
        return true;
    }
    // 查找
    Node *Find(const K &key)
    {
        Node *cur = _root;
        if (_root == nullptr)
            return nullptr;
        if (cur->_key == key)
            return cur;
        while (cur)
        {
            if (cur->_key < key)
            {
                cur = cur->_right;
            }
            else if (cur->_key > key)
            {
                cur = cur->_left;
            }
            else
            {
                return cur;
            }
        }
        return nullptr;
    }

    // 找小于key的节点
    vector<V> Find_lower(const K &key)
    {
        Node *cur = _root;
        cur = _findlow(cur, key);
        vector<V> ret;
        while (cur != nullptr && cur->_key < key)
        {
            Node *t = cur;
            while (t != nullptr)
            {
                ret.push_back(t->_val);
                t = t->_left;
            }
            cur = cur->_right;
        }
        return ret;
    }
    // 找大于key的节点
    vector<V> Find_higher(const K &key)
    {
        Node *cur = _root;
        cur = _findhigher(cur, key);
        vector<V> ret;
        while (cur != nullptr && cur->_key > key)
        {
            Node *t = cur;
            while (t != nullptr)
            {
                ret.push_back(t->_val);
                t = t->_right;
            }
            cur = cur->_left;
        }
        return ret;
    }
    bool Erase(const K &key)
    {
        Node *prev = nullptr;
        Node *cur = _root;

        while (cur)
        {
            if (cur->_key < key)
            {
                prev = cur;
                cur = cur->_right;
            }
            else if (cur->_key > key)
            {
                prev = cur;
                cur = cur->_left;
            }
            else
            { // 找到key，开始删除
                if (cur->_left == nullptr)
                {
                    if (prev == nullptr)
                        _root = cur->_right;
                    else
                    {
                        // 判断 prev是左指针链接还是有指针要链接
                        if (prev->_left == cur)
                            prev->_left = cur->_right;
                        else
                            prev->_right = cur->_right;
                    }
                    delete cur; // 释放要删除的结点
                }
                else if (cur->_right == nullptr)
                {
                    if (prev == nullptr)
                        _root = cur->_left;
                    else
                    {
                        // 判断 prev是左指针链接还是有指针要链接
                        if (prev->_left == cur)
                            prev->_left = cur->_left;
                        else
                            prev->_right = cur->_left;
                    }
                    delete cur; // 释放要删除的结点
                }
                else
                { // 要删除的结点左右子树不为空
                    /*思路：只要找到要删除结点的左子树最大值，或者右子树的最小值，用它来替换要删的结点cur
                    //再把左子树最大值，或者右子树最小值删掉即可；
                    //上面步骤做成功后，就可以满足要删的结点（被替换后的），左子树小于它，右子树也小于它
                    */
                    // 这里采用思路去右子树找最小值来替代(也就是去右子树中找最左结点注意这个最左结点不一定是叶子，但是最左结点的左子树一定为null)
                    // Node* minParent = nullptr; 直接赋值为空，可能会有空指针访问出错问题，这个问题会出现在，要删除结点的右子树没有左子树情况
                    Node *minParent = cur; // 右子树最小值结点的父节点

                    Node *minRight = cur->_right; // minRight表示要删除删除结点的右子树最小的值
                    // 右子树的最小值，一定是在它的左子树那，所以我们只要迭代去找到左子树即可
                    while (minRight->_left)
                    {
                        minParent = minRight;
                        minRight = minRight->_left;
                    }
                    // 退出循环，表示找到了minRight
                    // 把minRigth的值替换到要删除的结点cur
                    cur->_key = minRight->_key;
                    // 替换成功后，要删除cur的变成要把minRihgt删除了，
                    // 那么就是处理minRight的父节点minParent左右指针的链接关系了
                    // 对于minRight的结点，我们只能保证它的左指针是nullptr，不能保证右指针一定为nullptr，可能也有结点
                    if (minParent->_left == minRight)
                        minParent->_left = minRight->_right;
                    else // 这里处理的是要删除结点的cur右子树没有左子树的情况
                        minParent->_right = minRight->_right;

                    delete minRight;
                }

                // 删除后保存idx
                save_tree();
                return true;
            }
        }
        return false;
    }
    // 中序遍历；
    void InOrder()
    {
        _InOrder(_root);
        cout << endl;
    }

private:
    void _loadTree(ifstream &idxfile, Node *cur)
    {
        std::string line; // 逐行读取

        getline(idxfile, line);
        std::istringstream iss(line); // 使用 std::istringstream 对每一行进行逐词分割
        string word;
        variant<int, string> Key;
        iss >> word;
        if (word == "#")
        {
            cur = nullptr;
            return;
        }
        if (word.back() == '\"')
        {
            word.pop_back();
            Key = word;
        }
        else
        {
            Key = stoi(word);
        }

        iss >> word;
        int val = stoi(word);
        cur = new Node(Key, val);
        _loadTree(idxfile, cur->_left);
        _loadTree(idxfile, cur->_right);
    }
    // 先序遍历保存idx
    void save_tree_(Node *node, ofstream &location_out)
    {
        if (node != nullptr)
        {
            string t;
            if (node->_key.index() == 1)
            {
                t = get<string>(node->_key);
                t.push_back('\"');
            }
            else
            {
                t = to_string(get<int>(node->_key));
            }
            location_out << t << " " << node->_val << endl;
            save_tree_(node->_left, location_out);
            save_tree_(node->_right, location_out);
        }
        else
        {
            location_out << "# #" << endl;
        }
    }
    void save_tree()
    {
        ofstream location_out;
        location_out.open(filename, std::ios::out | std::ios::trunc); // 以重写的方式打开.txt文件，没有的话就创建该文件。
        if (location_out.is_open())
        {
            save_tree_(_root, location_out);
        }
        else
        {
            perror("无法保存索引文件");
        }
    }
    Node *_findhigher(Node *cur, const K &key)
    {
        if (cur->_key == key)
        {
            return cur->_right;
        }
        else if (cur->_key < key)
        {
            if (cur->_right == nullptr)
            {
                return nullptr;
            }
            else
            {
                return _findhigher(cur->_right, key);
            }
        }
        return cur;
    }
    Node *_findlow(Node *cur, const K &key)
    {
        if (cur->_key == key)
        {
            return cur->_left;
        }
        else if (cur->_key > key)
        {
            if (cur->_left == nullptr)
            {
                return nullptr;
            }
            else
            {
                return _findlow(cur, key);
            }
        }
        return cur;
    }
    // 后续遍历销毁二叉树
    void _destroy(Node *root)
    {
        if (root == nullptr)
            return;

        _destroy(root->_left);
        _destroy(root->_right);
        delete root;
    }
    void _InOrder(Node *root)
    {
        if (root == nullptr)
            return;
        _InOrder(root->_left);
        std::cout << root->_key << " " << root->_val;
        _InOrder(root->_right);
    }

private:
    Node *_root;
};
