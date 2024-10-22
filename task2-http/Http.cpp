#include "Http.h"

#include <netinet/in.h>
#include <dirent.h>
#include <regex>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <iostream>
void Http::Add_Header(const std::string& Content)
{
    //为Http_Header添加内容
    if (!Content.empty())
    {
        Http_Header += Content;
        Http_Header += "\r\n";
    }
   
    else
    {
        Http_Header += "\r\n";
    }
}

void Http::Header(int flag)
{
    //状态行，这里只给出200和404两种
    if(flag == 200)
    { 
        Http_Header = "HTTP/1.1 200 OK\r\n";
    }
    else if(flag==404)
    {
        Http_Header = "HTTP/1.1 404 NOTFOUND\r\nContent-Length:0\r\n\r\n";
    }
    //待扩展
}

void Http::Handle_Header()
{
    //这里只设置了txt和png文件
    std::string ContentType = "Content-Type:";
    if (File_Type == "txt")
    {
        ContentType += "text/plain;charset=utf-8";
    }
    else if(File_Type== "png")
    {
        ContentType += "image/" + File_Type;
    }
    Add_Header(ContentType);
    File_Size=file_stat.st_size;
    std::string ContentLength = "Content-Length:" + std::to_string(File_Size);
    Add_Header(ContentLength);
    Add_Header("");
}

void Http::Add_Filepath(const std::string& Req_File)
{
    File_Path += Req_File;
}

void Http::Get_FileType(const std::string& Req_File)
{
    for (int i = 0; i < Req_File.size(); ++i)
    {
        if (Req_File[i] == '.')
        {
            // 获取请求文件以什么结尾的，也就是说该文件是什么类型的
            File_Type = Req_File.substr(i + 1);
        }
    }
}

void Http::send_response(int client_fd,const std::string& status, const std::string& content_type,const std::string& body) 
{
    std::string response = "HTTP/1.1 " + status + "\r\n";
    response += "Content-Type: " + content_type + ";charset=utf-8" + "\r\n";         //注意字符格式
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    response += "\r\n";
    response += body;
    send(client_fd, response.c_str(), response.size(), 0);
}
void Http::send_response(int client_fd,const std::string& body)
{
    std::string response = Http_Header;
    response+=body;
    send(client_fd, response.c_str(), response.size(), 0);
}
std::string Http::list_directory(const std::string& path) {
    DIR* dir = opendir(path.c_str());     //dir目录流
    if (dir == nullptr) {
        return "无法打开文件\n";
    }

    struct dirent* entry;               //目录流实体entry
    std::string body = "<html><body><h1>Directory listing for " + path + "</h1><ul>";

    while ((entry = readdir(dir)) != nullptr) {
        body += "<li><a href=\"" + std::string(entry->d_name) + "\">" + std::string(entry->d_name) + "</a></li>";   //打开文件夹后文件的超链接
    }
    body += "</ul></body></html>";
    closedir(dir);
    return body;
}

bool Http::Handle_request(int client_fd,char* buffer)
{
    std::string buffer_str(buffer);
    // 正则表达式匹配请求行和请求头
    std::string pattern = "^([A-Z]+) (/[-A-Za-z0-9._]*)";
    std::regex reg(pattern);                
    std::smatch mas;     //储存匹配结果
    std::regex_search(buffer_str,mas,reg);
    // 因为下标0是代表匹配的整体
    if(mas.size() < 3){
        std::cout<<"不是正常请求"<<std::endl;
        return false;
    }
    std::string method = mas[1];       //GET方法

    // 请求的具体文件
    std::string requestFile = mas[2];    //文件路径
    bool flag;
    if (requestFile == "/")
    { 
        File_Path.clear(); // 清零
        File_Path = My_Path;
    }
    else
    {
        File_Path.clear(); // 清零
        File_Path = My_Path;
        Add_Filepath(requestFile);
    }
    if(stat(File_Path.c_str(),&file_stat)==-1)
    {
        // 文件不存在，返回404
        std::string not_found_body = "<html><body><h1>404 Not Found</h1></body></html>";
        send_response(client_fd, "404 Not Found", "text/html", not_found_body);
    }
    if (S_ISDIR(file_stat.st_mode))    //判断该文件模式是否是文件夹
    {
        std::string body = list_directory(File_Path);
        send_response(client_fd, "200 OK", "text/html", body);
        std::cout<<"文件夹数据已发送"<<std::endl;
        return true;
    }
    else{
         // 如果是文件，读取文件内容并返回
            int file = ::open(File_Path.c_str(), O_RDONLY); //O_RDONLY只读
            if (file == -1) //如果文件不存在
            {
                std::cout<<"未找到客户要的文件："<<File_Path<<std::endl;
                std::string not_found_body = "<html><body><h1>404 Not Found</h1></body></html>";
                send_response(client_fd, "404 Not Found", "text/html", not_found_body);
                return false;
            } 
            else {
                //文件存在
                Header(200);
                std::string body;
                char file_buffer[BUFFER_SIZE];
                int bytes_read;
                Get_FileType(File_Path);
                Handle_Header();
                while ((bytes_read = read(file, file_buffer, BUFFER_SIZE)) > 0) {
                    body.append(file_buffer, bytes_read);
                }
                close(file);
                send_response(client_fd, body);
                std::cout<<"文件数据已发送"<<std::endl;
                return true;
            }
    }
}