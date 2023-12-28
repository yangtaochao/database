#include "frontside.h"

class myexception : public exception
{
    virtual const char *what() const throw()
    {
        return "My exception happened";
    }
} myexp;

void message::start()
{
    file_process process;
    while (1)
    {
        cout << '>';
        string input;
        getline(std::cin, input);

        // 将逗号和括号替换为空格
        std::replace(input.begin(), input.end(), ',', ' ');
        std::replace(input.begin(), input.end(), '(', ' ');
        std::replace(input.begin(), input.end(), ')', ' ');

        // 使用字符串流分割单词
        stringstream ss(input);
        vector<std::string> words;
        string word;

        while (ss >> word)
        {
            words.push_back(word);
        }

        // 处理提取到的单词
        try
        {
            if (words.size() >= 1)
            {
                if (words[0] == "create") // create
                {
                    if (words.size() >= 2 && words[1] == "database") // create database <dbname>
                    {
                        if (words.size() >= 4)
                        {
                            throw myexp;
                        }
                        else if (words.size() == 3)
                        {
                            process.create_database(words[2]);
                        }
                    }
                    else if (words.size() >= 2 && words[1] == "table") // create table <tbname>(...)
                    {
                        if (words.size() <= 5)
                        {
                            throw myexp;
                        }
                        else
                        {
                            std::vector<std::vector<std::string>> twoDArray;
                            int primary = -1, arraypoint = 4;                // arraypoint为words的第arraypoint个单词
                            for (int i = 0; arraypoint <= words.size(); ++i) // 读取输入缓冲区
                            {
                                // 添加新的行
                                twoDArray.push_back(std::vector<std::string>(2));
                                twoDArray[i][0] = words[arraypoint - 1];
                                arraypoint++;
                                if (arraypoint <= words.size() && (words[arraypoint - 1] == "int" || words[arraypoint - 1] == "string")) // 将数据放入新的行
                                {
                                    twoDArray[i][1] = words[arraypoint - 1];
                                    arraypoint++;
                                }
                                else
                                {
                                    throw myexp;
                                }

                                if (arraypoint <= words.size() && words[arraypoint - 1] == "primary")
                                {
                                    primary = i;
                                    arraypoint++;
                                }
                            }
                            process.create_table(words[2], twoDArray, primary);
                        }
                    }
                    else
                    {
                        throw myexp;
                    }
                }
                else if (words[0] == "drop") // drop
                {
                    if (words.size() >= 2 && words[1] == "database") // drop database <dbname>
                    {
                        if (words.size() >= 4)
                        {
                            throw myexp;
                        }
                        else if (words.size() == 3)
                        {
                            process.drop_database(words[2]);
                        }
                    }
                    else if (words.size() >= 2 && words[1] == "table") // drop tavle <tbname>
                    {
                        if (words.size() >= 4)
                        {
                            throw myexp;
                        }
                        else if (words.size() == 3)
                        {
                            process.drop_table(words[2]);
                        }
                    }
                    else
                    {
                        throw myexp;
                    }
                }
                else if (words[0] == "use") // use database <dbname>
                {
                    if (words.size() >= 2 && words[1] == "database")
                    {
                        if (words.size() >= 4)
                        {
                            throw myexp;
                        }
                        else if (words.size() == 3)
                        {
                            process.use_database(words[2]);
                        }
                    }
                    else
                    {
                        throw myexp;
                    }
                }
                else if (words[0] == "select") // select
                {
                    if (words.size() >= 4 && words[2] == "from")
                    {
                        if (words.size() == 4)
                        {

                            process.select(words[1], words[3], string(), string(), string());
                        }
                        else if (words.size() == 8 && words[4] == "where")
                        {
                            if (words[6] == "<" || words[6] == "=" || words[6] == ">")
                            {
                                std::variant<int, std::string> myvariant2;
                                if (words[7].front() == '\"')
                                {
                                    if (words[7].back() == '\"')
                                    {
                                        words[7] = words[7].substr(1, words[7].length() - 2); // 去掉双引号
                                        myvariant2 = word[7];
                                    }
                                    else
                                    {
                                        throw myexp;
                                    }
                                }
                                else
                                {
                                    int temp = atoi(words[7].c_str());
                                    myvariant2 = word[7];
                                }
                                process.select(words[1], words[3], words[5], words[6], myvariant2);
                            }
                            else
                            {
                                throw myexp;
                            }
                        }
                        else
                        {
                            throw myexp;
                        }
                    }
                    else
                    {
                        throw myexp;
                    }
                }
                else if (words[0] == "delete") // deletedata
                {
                    if (words.size() >= 2)
                    {
                        if (words.size() == 2)
                        {
                            process.deletedata(words[1], string(), string(), string());
                        }
                        else if (words.size() == 6 && words[2] == "where" && (words[4] == "<" || words[4] == "=" || words[4] == ">"))
                        {
                            std::variant<int, std::string> myvariant = words[5];
                            std::visit([](auto &&arg)
                                       {
                                           using T = std::decay_t<decltype(arg)>;

                                           if constexpr (std::is_same_v<T, std::string>)
                                           {
                                               // 如果是字符串，去掉两边的引号
                                               if (arg.front() == '"' && arg.back() == '"')
                                               {
                                                   arg = arg.substr(1, arg.size() - 2);
                                               }
                                               else
                                               {
                                                   throw myexp;
                                               }
                                           }
                                           // 如果是int，什么也不做
                                       },
                                       myvariant);
                            process.deletedata(words[1], words[3], words[4], myvariant);
                        }
                        else
                        {
                            throw myexp;
                        }
                    }
                    else
                    {
                        throw myexp;
                    }
                }
                else if (words[0] == "insert") // insert
                {
                    if (words.size() >= 4 && words[2] == "values")
                    {
                        std::vector<variant<int, std::string>> myVector;
                        for (int i = 3; i < words.size(); i++)
                        {
                            if (words[i].front() == '\"')
                            {
                                if (words[i].back() == '\"')
                                {
                                    words[i] = words[i].substr(1, words[i].length() - 2); // 去掉双引号
                                    myVector.push_back(words[i]);
                                }
                                else
                                {
                                    throw myexp;
                                }
                            }
                            else
                            {
                                int temp = atoi(words[i].c_str());
                                myVector.push_back(temp);
                            }
                        }
                        process.insert(words[1], myVector);
                    }
                    else
                    {
                        throw myexp;
                    }
                }
                else if (words[0] == "update") // update
                {
                    if (words.size() >= 6 && words[2] == "set" && words[4] == "=")
                    {

                        std::variant<int, std::string> myvariant1 = words[3];
                        std::visit([](auto &&arg)
                                   {
                                       using T = std::decay_t<decltype(arg)>;

                                       if constexpr (std::is_same_v<T, std::string>)
                                       {
                                           // 如果是字符串，去掉两边的引号
                                           if (arg.front() == '"' && arg.back() == '"')
                                           {
                                               arg = arg.substr(1, arg.size() - 2);
                                           }
                                           else
                                           {
                                               throw myexp;
                                           }
                                       }
                                       // 如果是int，什么也不做
                                   },
                                   myvariant1);
                        if (words.size() == 6)
                        {
                            process.update(words[1], words[3], myvariant1, string(), string(), string());
                        }
                        else if (words.size() == 10 && words[6] == "where" && (words[4] == "<" || words[4] == "=" || words[4] == ">"))
                        {
                            std::variant<int, std::string> myvariant = words[9];
                            std::visit([](auto &&arg)
                                       {
                                           using T = std::decay_t<decltype(arg)>;

                                           if constexpr (std::is_same_v<T, std::string>)
                                           {
                                               // 如果是字符串，去掉两边的引号
                                               if (arg.front() == '"' && arg.back() == '"')
                                               {
                                                   arg = arg.substr(1, arg.size() - 2);
                                               }
                                               else
                                               {
                                                   throw myexp;
                                               }
                                           }
                                           // 如果是int，什么也不做
                                       },
                                       myvariant);
                            process.update(words[1], words[3], myvariant1, words[7], words[8], myvariant);
                        }
                        else
                        {
                            throw myexp;
                        }
                    }
                    else
                    {
                        throw myexp;
                    }
                }
                else
                {
                    throw myexp;
                }
            }
            else
            {
                throw myexp;
            }
        }
        catch (myexception &e)
        {
            cout << "输入格式有误! 请重新输入命令" << endl;
            continue;
        }
    }
}
