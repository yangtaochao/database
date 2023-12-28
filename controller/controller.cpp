#include "controller.h"

void file_process::create_database(string name)
{
    const char *pathname = format("./data/{}", name).c_str();
    int ret = 0;
    DIR *mydir = NULL;
    mydir = opendir(pathname); // 打开目录
    if (mydir == NULL)
    {
        ret = mkdir(pathname, 0755); // 创建目录
        if (ret != 0)
        {
            std::cout << "数据库创建失败" << std::endl;
        }
        else
        {
            std::cout << "数据库创建成功" << std::endl;
        }
    }
    else
    {
        std::cout << "数据库已存在" << std::endl;
    }
    closedir(mydir); // 关闭目录
}

void file_process::drop_database(string name)
{
    const char *pathname = format("./data/{}", name).c_str();
    int ret = 0;
    DIR *mydir = NULL;
    mydir = opendir(pathname); // 打开目录
    if (mydir != NULL)
    {
        ret = rmdir(pathname); // 创建目录
        if (ret != 0)
        {
            std::cout << "数据库删除成功" << std::endl;
        }
        else
        {
            std::cout << "数据库删除失败" << std::endl;
        }
    }
    else
    {
        std::cout << "数据库不存在" << std::endl;
    }
    closedir(mydir); // 关闭目录
}

void file_process::use_database(string name)
{
    const char *pathname = format("./data/{}", name).c_str();
    int ret = 0;
    DIR *mydir = NULL;
    mydir = opendir(pathname); // 打开目录
    if (mydir != NULL)
    {
        current_database = name;
        _loadTable(mydir);
        cout << "切换成功!" << endl;
    }
    else
    {
        cout << "该数据库不存在!" << endl;
    }
}

void file_process::drop_table(string table_name)
{
    // 查看表是否存在
    for (int i = 0; i < TableList.size(); i++)
    {
        if (table_name == TableList[i]->getname())
        {
            delete (TableList[i]);
            swap(*(begin(TableList) + i), *(end(TableList) - 1));
            TableList.pop_back();
            i--;

            /*删除文件中的表*/
            string deletename = "../data/" + current_database + "/" + table_name + ".txt";
            if (remove(deletename.c_str()) != 0)
            {
                std::cout << "Table '" << table_name << "' 删除失败." << std::endl; // 如果删除失败，输出错误信息
            }
            else
            {
                std::cout << "Table '" << table_name << "' 成功删除." << std::endl;
            }

            return;
        }
    }
}
void file_process::create_table(string name, std::vector<std::vector<std::string>> twoDArray, int primary)
{
    if (current_database.length() == 0)
    {
        cout << "请先打开一个数据库!" << endl;
        return;
    }
    Table *tempTable = new Table(name, twoDArray, primary, current_database);
    TableList.push_back(tempTable);
    cout << "表格创建成功!" << endl;
}

void file_process::select(string column1, string table_name, string column2, string op, std::variant<int, std::string> const_value)
{ // select <column> from <table> [ where <cond> ]
  // select <column1> from <table> where <column2> <op> <const-value>

    // 从众多表中查找该表
    Table *correct_Table = nullptr;
    for (Table *tab : TableList)
    {
        if (tab->getname() == table_name)
        {
            correct_Table = tab;
        }
    }

    if (correct_Table == nullptr)
    {
        cout << "此表不存在!" << endl;
        return;
    }

    if (op.empty()) // 如果没有where
    {
        if (column1 == "*")
        {
            // 如果是*,则打印整张表
            vector<int> t;
            correct_Table->print_columns(t, -1);
            return;
        }
        /*
        如果不是*,那么就寻找匹配的表头,然后打印这列所有内容
        */
        int column_seq = correct_Table->search_column(column1);
        if (column_seq == -1)
        {
            cout << "该列不存在!" << endl;
            return;
        }
        vector<int> t;
        correct_Table->print_columns(t, column_seq);
    }
    else // 如果有where
    {
        /*
        首先查找符合条件的行,然后再打印这些行里的特定列
        */
        // 首先检查条件列是否存在
        int column2_seq = correct_Table->search_column(column2);
        if (column2_seq == -1)
        {
            cout << "条件列不存在!" << endl;
            return;
        }
        // 检查常量是否与条件列类型匹配
        if (correct_Table->search_type(column2_seq) != const_value.index())
        {
            cout << "条件列类型与常量类型不匹配!" << endl;
            return;
        }
        // 然后查找符合条件的行
        vector<int> correct_rows = correct_Table->condition_search(column2_seq, op, const_value);
        if (correct_rows.empty())
        {
            // 如果没找到
            cout << "没有符合条件的数据." << endl;
            return;
        }

        // 如果找到了
        if (column1 == "*")
        {
            // 如果目标列式*,则打印这些行的所有列
            correct_Table->print_columns(correct_rows, -1);
        }
        else
        {
            // 不是,则检查目标列是否存在
            int column1_seq = correct_Table->search_column(column1);
            if (column1_seq == -1)
            {
                cout << "目标列不存在!" << endl;
                return;
            }
            // 之后打印这些行中的目标列
            correct_Table->print_columns(correct_rows, column1_seq);
        }
    }
}

void file_process::_loadTable(DIR *mydir)
{
    for (Table *tablePtr : TableList) // 清空 vector，并确保指针置为 nullptr
    {
        delete tablePtr;
    }
    vector<Table *>().swap(TableList);

    std::vector<std::string> txtFiles; // txtFiles用于保存读到的txt文件名

    if (mydir)
    {
        dirent *entry;
        while ((entry = readdir(mydir)) != nullptr) // 遍历文件夹中的文件
        {
            if (strstr(entry->d_name, ".txt") != nullptr) // 判断当前文件名后缀是否为.txt
            {
                txtFiles.push_back(entry->d_name); // 将文件名存入txtfiles
            }
        }
        for (string txtfile : txtFiles) // 对每个 txtfile 执行操作
        {
            std::string filename = "./data/" + current_database + "/" + txtfile; // 表对应的.txt文件名
            Table *new_table = new Table(filename, current_database);
            TableList.push_back(new_table);
        }
    }
}

void file_process::deletedata(string table_name, string column, string op, std::variant<int, std::string> const_value)
{
    // 从众多表中查找该表
    Table *correct_Table = nullptr;
    for (Table *tab : TableList)
    {
        if (tab->getname() == table_name)
        {
            correct_Table = tab;
        }
    }

    if (correct_Table == nullptr)
    {
        cout << "此表不存在!" << endl;
        return;
    }

    if (op.empty())
    {
        // 如果没有where,则删除所有行
        correct_Table->clear_content(vector<int>());
    }
    else
    {
        // 有where,则根据条件查询

        // 检查column是否存在
        int column_seq = correct_Table->search_column(column);
        if (column_seq == -1)
        {
            cout << "条件列不存在!" << endl;
            return;
        }
        // 检查常量是否与条件列类型匹配
        if (correct_Table->search_type(column_seq) != const_value.index())
        {
            cout << "条件列类型与常量类型不匹配!" << endl;
            return;
        }

        // 然后查找符合条件的行
        vector<int> correct_rows = correct_Table->condition_search(column_seq, op, const_value);
        if (correct_rows.empty())
        {
            // 如果没找到
            cout << "没有符合条件的数据." << endl;
            return;
        }

        // 如果找到了,则删除这些行
        correct_Table->clear_content(correct_rows);
        // 打印删除成功信息
        cout << "记录删除成功!" << endl;
    }
}

void file_process::insert(string table_name, std::vector<std::variant<int, std::string>> myarray)
{
    /*主键不能重复*/

    // 从众多表中查找该表
    Table *correct_Table = nullptr;
    for (Table *tab : TableList)
    {
        if (tab->getname() == table_name)
        {
            correct_Table = tab;
        }
    }

    if (correct_Table == nullptr)
    {
        cout << "此表不存在!" << endl;
        return;
    }

    // 添加元素
    correct_Table->add_row(myarray);
}

void file_process::update(string table_name, string column1, std::variant<int, std::string> const_value1, string column2, string op, std::variant<int, std::string> const_value2)
{
    // 从众多表中查找该表
    Table *correct_Table = nullptr;
    for (Table *tab : TableList)
    {
        if (tab->getname() == table_name)
        {
            correct_Table = tab;
        }
    }

    if (correct_Table == nullptr)
    {
        cout << "此表不存在!" << endl;
        return;
    }

    //  检查column1是否存在
    int column1_seq = correct_Table->search_column(column1);
    if (column1_seq == -1)
    {
        cout << "目标列不存在!" << endl;
        return;
    }
    // 检查常量是否与目标列类型匹配
    if (correct_Table->search_type(column1_seq) != const_value1.index())
    {
        cout << "目标列类型与常量类型不匹配!" << endl;
        return;
    }

    if (op.empty())
    { // 如果没有where,就更新所有行的值
        correct_Table->update(vector<int>(), column1_seq, const_value1);
    }
    else
    { // 如果有where
        // 检查运算符
        if (op != "=" || op != "<" || op != ">")
        {
            cout << "非法的运算符!" << endl;
            return;
        }
        //  检查column2是否存在
        int column2_seq = correct_Table->search_column(column2);
        if (column2_seq == -1)
        {
            cout << "条件列不存在!" << endl;
            return;
        }
        // 检查常量是否与条件列类型匹配
        if (correct_Table->search_type(column2_seq) != const_value2.index())
        {
            cout << "条件列类型与常量类型不匹配!" << endl;
            return;
        }

        // 然后查找符合条件的行
        vector<int> correct_rows = correct_Table->condition_search(column2_seq, op, const_value2);
        if (correct_rows.empty())
        {
            // 如果没找到
            cout << "没有符合条件的数据." << endl;
            return;
        }
        // 更新数据
        correct_Table->update(correct_rows, column1_seq, const_value1);
        // 输出更新成功信息
        cout << "更新成功!" << endl;
    }
}