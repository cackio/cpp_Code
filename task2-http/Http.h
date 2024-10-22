#pragma once
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
class Http
{
private:
const std::string My_Path = "/root/http_dir";//客户能访问的文件的根目录
std::string File_Path;              //客户端请求的文件的绝对路径
std::string File_Type;              //客户端请求的文件的类型 
std::string Http_Header;              //http头
int File_Size;                      
int File_Fd;
struct stat file_stat;          //储存文件信息的结构体

private:
void Header(int flag);         //状态行
void Add_Header(const std::string& Content); //向http头添加内容
void Handle_Header();                        //对http头进行处理
void send_response(int client_fd,const std::string& status, const std::string& content_type,const std::string& body);//发送自定义http响应报文
void send_response(int client_fd,const std::string& body);       //发送http响应报文
void Add_Filepath(const std::string& Req_File);                  //添加文件路径
void Get_FileType(const std::string& Req_File);                  //判断文件类型
std::string list_directory(const std::string& path);             //若是请求文件夹则调用该函数

public:
bool Handle_request(int client_fd,char* buffer);       //只给出该函数接口
};