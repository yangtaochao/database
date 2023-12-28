
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <format>
#include <string.h>
#include <sstream>
#include <fstream>
#include <cstdio>
#include "../model/Table.h"
using namespace std;

class file_process
{
private:
    string current_database = "";
    vector<Table *> TableList;

public:
    void create_database(string name);                                                                                                                              // finished
    void drop_database(string name);                                                                                                                                // finished
    void use_database(string name);                                                                                                                                 // finished
    void create_table(string name, std::vector<std::vector<std::string>> twoDArray, int primary);                                                                   // finished 第二个参数twoDArray是二维字符串数组,第三个是主键在二维数组中对应的行坐标
    void drop_table(string name);                                                                                                                                   // finished
    void select(string column1, string table_name, string column2, string op, std::variant<int, std::string> const_value);                                          // finished
    void deletedata(string table, string column, string op, std::variant<int, std::string> const_value);                                                            // finished
    void insert(string name, std::vector<std::variant<int, std::string>> myarray);                                                                                  // finished
    void update(string table, string column1, std::variant<int, std::string> const_value1, string column2, string op, std::variant<int, std::string> const_value2); // finished
    void _loadTable(DIR *mydir);
};
