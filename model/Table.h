
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <format>
#include <fstream>
#include <algorithm>
#include <sstream>

#include "Btree.h"
using namespace std;
using Index = BSTree<variant<int, string>, int>; // 索引

class Table
{
public:
    enum
    {
        INT = 0,
        STRING = 1
    };
    string name = "";
    string database;
    vector<vector<variant<int, string>>> content;
    vector<string> head;
    vector<int> type;
    int primary = -1; // 如果primary为-1，则表明主键未规定
    Index *idx;       // 索引

    void _saveTable()
    {
        ofstream location_out;
        std::string dirname1 = "./data/" + database + "/";
        std::string filename = dirname1 + this->name + ".txt";
        location_out.open(filename, std::ios::out | std::ios::trunc); // 以重写的方式打开.txt文件，没有的话就创建该文件。

        if (!location_out.is_open()) // 如果文件打开失败则返回
        {
            perror("表格无法保存!");
        }

        location_out << this->name << '\t' << this->primary << endl; // 在第一行写入表名

        for (size_t i = 0; i < this->head.size(); ++i) // 在第二行按照head的顺序写入表头名
        {
            location_out << this->head[i];

            if (i < this->head.size() - 1) // 在除了最后一个元素之后都写入制表符
            {
                location_out << '\t';
            }
            else
            {
                location_out << endl;
            }
        }

        for (size_t i = 0; i < this->type.size(); ++i) // 在第三行按照size的顺序写入表头名
        {
            location_out << to_string(this->type[i]);

            if (i < this->type.size() - 1) // 在除了最后一个元素之后都写入制表符
            {
                location_out << '\t';
            }
            else
            {
                location_out << endl;
            }
        }

        for (int i = 0; i < this->content.size(); ++i) // 在接下来的数行写入表格数据
        {
            for (int j = 0; j < this->content[i].size(); ++j)
            {
                if (std::holds_alternative<int>(content[i][j])) // 如果是 int，将其转换为字符串并写入文件
                {
                    location_out << std::to_string(std::get<int>(content[i][j]));
                }
                else
                { // 如果是字符串
                    location_out << get<string>(this->content[i][j]);
                }
                if (this->content[i].size() - 1)
                {
                    location_out << '\t'; // 在除了最后一个元素之后都写入制表符
                }
            }
            location_out << endl;
        }
        location_out.close();
        return;
    }

public:
    string getname()
    {
        return name;
    }

    int search_column(string name)
    {
        /*
        根据名字寻找表头,
        失败返回-1,成功返回目标列在head中的序号
        */
        for (int i = 0; i < head.size(); i++)
        {
            if (head[i] == name)
            {
                return i;
            }
        }
        return -1;
    }

    int search_type(int column_seq)
    {
        return type[column_seq];
    }

    vector<int> condition_search(int column_seq, string op, std::variant<int, std::string> const_value)
    {
        // 返回符合条件的行的列表
        vector<int> correct_rows;

        // // 如果是主键,存在索引,使用索引搜索
        // if (column_seq == primary && primary != -1)
        // {
        //     if (op == "=")
        //     {
        //         BSTreeNode<variant<int, string>, int> *t = idx->Find(const_value);
        //         correct_rows.push_back(t->_val);
        //     }
        //     else if (op == "<")
        //     {
        //         correct_rows = idx->Find_lower(const_value);
        //     }
        //     else if (op == ">")
        //     {
        //         correct_rows = idx->Find_higher(const_value);
        //     }
        // }
        // else
        {
            // 不是主键则正常搜索
            int i = 0;
            if (op == "=")
            {
                for (i = 0; i < content.size(); i++)
                {
                    if (content[i][column_seq] == const_value)
                    {
                        correct_rows.push_back(i);
                    }
                }
            }
            else if (op == "<")
            {
                for (i = 0; i < content.size(); i++)
                {
                    if (content[i][column_seq] < const_value)
                    {
                        correct_rows.push_back(i);
                    }
                }
            }
            else if (op == ">")
            {
                for (i = 0; i < content.size(); i++)
                {
                    if (content[i][column_seq] > const_value)
                    {
                        correct_rows.push_back(i);
                    }
                }
            }
        }
        return correct_rows;
    }

    void print_columns(vector<int> rows_seq, int column_seq)
    {
        // 方便访问variant的准备工作
        struct VisitPackage
        {
            auto operator()(const string &v) { std::cout << v; }
            auto operator()(const int &v) { std::cout << v; }
        };
        // 打印rows_seq行里的所有第column_seq列的信息
        // 若rows_seq.size()为0,则打印所有行`12
        // 若column_seq为-1,则打印所有列.

        int rows = content.size();
        int cols = content[0].size();
        int headNum = head.size();

        if (rows_seq.size() == 0 && column_seq == -1)
        {
            /*
             ------------------------------
             表头:    |     |      |      |
             ------------------------------
                     |     |      |      |
            -------------------------------
                     |     |      |      |
            */

            std::cout << "-------------------------" << endl;
            for (int col = 0; col < headNum; ++col)
            {
                std::cout << head[col] << " "
                          << "|";
            }
            std::cout << "-------------------------" << endl;

            for (int row = 0; row < rows; ++row)
            {
                for (int col = 0; col < cols; ++col)
                {
                    // 根据variant的实际类型来打印

                    visit(VisitPackage(), content[row][col]);

                    std::cout << " "
                              << "|";
                }
                std::cout << "-------------------------" << endl;
            }
            return;
        }

        if (rows_seq.size() == 0)
        { /*------------------------------------
            表头
            ------------------------------------
            第一行:
            ------------------------------------
            第二行:
         */
            std::cout << "-------------------------" << endl;
            for (int col = 0; col < headNum; ++col)
            {
                std::cout << head[col] << "\t";
            }
            std::cout << "-------------------------" << endl;

            for (int row = 0; row < rows; ++row)
            {
                for (int col = 0; col < cols; ++col)
                {
                    // 根据variant的实际类型来打印

                    visit(VisitPackage(), content[row][col]);
                    std::cout << "\t";
                }
                std::cout << "-------------------------" << endl;
            }
            return;
        }
        else if (column_seq == -1)
        { /*
          表头:  |    |    |    |
                 |    |    |    |
                 |    |    |    |
                 |    |    |    |
                 |    |    |    |
         */

            for (int col = 0; col < headNum; ++col)
            {
                std::cout << " " << head[headNum] << " "
                          << "|";
            }
            std::cout << std::endl;

            for (int row = 0; row < rows; ++row)
            {
                for (int col = 0; col < cols; ++col)
                {
                    std::cout << " ";
                    visit(VisitPackage(), content[row][col]);
                    cout << " |";
                }
                std::cout << std::endl;
            }

            return;
        }
        else
        {
            /*
                表头:
                -----------------------
                第rows_seq第column_seq列内容
            */
            int r1 = rows_seq.size();

            std::cout << head[column_seq] << ": " << endl;
            std::cout << "-------------------------" << endl;
            for (int i = 0; i < r1; ++i)
            {
                // 根据variant的实际类型来打印

                visit(VisitPackage(), content[rows_seq[i]][column_seq]);
                std::cout << endl;
            }

            return;
        }
    }

    void clear_content(vector<int> rows_seq)
    {
        // 删除特定行,如果rows_seq.size()为0,则删除content所有行
        if (rows_seq.empty())
        {
            vector<vector<variant<int, string>>>().swap(content);
            // 删除索引表
            delete idx;
            idx = new Index("./data/" + this->database + "/" + this->name + ".idx");
        }
        else
        {
            // 把没被删的留下来先放到另一个临时容器里,最后再交换
            vector<vector<variant<int, string>>> temp;
            for (int i = 0; i < content.size(); i++)
            {
                if (count(rows_seq.begin(), rows_seq.end(), i) == 0)
                {
                    temp.push_back(content[i]);
                }
                else
                {
                    if (primary != -1)
                    {
                        idx->Erase(content[i][primary]);
                    }
                }
            }
            temp.swap(content);
        }
    }

    void add_row(std::vector<std::variant<int, std::string>> myarray)
    {
        for (int i = 0; i < myarray.size(); i++)
        {
            int j = myarray[i].index();
            if (j != type[i])
            {
                cout << "有数据与其表头的数据类型不匹配!" << endl;
                return;
            }
        }

        if (primary != -1)
        {
            // 检查主键是否重复
            if (idx->Find(myarray[primary]) != nullptr)
            {
                cout << "主键值重复了!" << endl;
                return;
            }
            /*更新索引表...*/
            idx->Insert(myarray[primary], content.size());
        }
        content.push_back(myarray);
        // 保存表格
        _saveTable();
        cout << "添加成功!" << endl;
    }

    void update(vector<int> correct_rows, int column1_seq, std::variant<int, std::string> const_value1)
    {
        // correct_rows.size()为0时更新整个表
        if (correct_rows.size() == 0)
        {
            for (auto ele : content)
            {
                ele[column1_seq] = const_value1;
            }
            if (primary != -1 && column1_seq == primary)
            {
                // 如果更新了主键值,那么删除索引表,换掉idx
                delete idx;
                idx = new Index("./data/" + this->database + "/" + this->name + ".idx");
                for (int i = 0; i < content.size(); i++)
                {
                    idx->Insert(const_value1, i);
                }
            }
        }
        else
        { // 不为0时正常更新
            for (int index : correct_rows)
            {
                content[index][column1_seq] = const_value1;
            }
        }

        // 保存表格
        _saveTable();
    }
    Table(string name, std::vector<std::vector<std::string>> twoDArray, int primary, string database_name)
    {
        this->primary = primary;
        this->name = name;
        this->database = database_name;
        for (auto row : twoDArray)
        {
            head.push_back(row[0]);
            if (row[1] == "int")
            {
                type.push_back(INT);
            }
            else
            {
                type.push_back(STRING);
            }
        }
        // 保存表格
        _saveTable();
        // 创建索引
        ofstream location_out;
        std::string dirname1 = "./data/" + database + "/";
        std::string filename = dirname1 + this->name + ".idx";
        location_out.open(filename, std::ios::out | std::ios::trunc); // 以重写的方式打开.txt文件，没有的话就创建该文件。

        idx = new Index(filename);
    }

    // 从文件创建table
    Table(string filename, string database)
    {
        this->database = database;
        std::ifstream inFile(filename);
        if (!inFile.is_open())
        {
            std::cout << "无法打开表格。" << endl;
            return;
        }

        std::string line; // 逐行读取

        for (int i = 0; std::getline(inFile, line); i++)
        {
            std::istringstream iss(line); // 使用 std::istringstream 对每一行进行逐词分割
            string word;

            vector<variant<int, string>> row;
            for (int j = 0; iss >> word; j++) // 逐个读取每个词并处理
            {
                if (i == 0) // 处理第一行
                {
                    if (j == 0)
                    {
                        this->name = word;
                    }
                    else
                    {
                        this->primary = std::stoi(word);
                    }
                }
                else if (i == 1) // 处理第二行
                {
                    this->head.push_back(word);
                }
                else if (i == 2) // 处理第三行
                {
                    this->type.push_back(std::stoi(word));
                }
                else // 处理表格数据
                {
                    if (this->type[j] == 0)
                    {
                        row.push_back(stoi(word));
                    }
                    else
                    {
                        row.push_back(word);
                    }
                }
            }

            if (i > 2)
            {
                content.push_back(row);
            }
        }

        // 加载索引
        string idx_dirname = "./data/" + database + "/" + name + ".idx";
        ifstream idxfile(idx_dirname);
        if (!idxfile.is_open())
        {
            std::cout << "无法打开索引。" << endl;
            return;
        }
        idx = new Index(idx_dirname, idxfile);
    }
};