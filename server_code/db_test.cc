// 实现一个数据库接口测试程序, 用来验证前面的数据库接口是否正确
#include <iostream>
#include "util.hpp"
#include "db.hpp"
using namespace blog_system;

void TestBlogTable() {
  bool ret = false;
  // 更友好的格式化显示 Json
  Json::StyledWriter writer;
  MYSQL* mysql = MySQLInit();

  Json::Value blog;
  blog["title"] = "初识 C 语言";
  std::string content;
  FileUtil::ReadFile("./test_data/1.md", &content);
  blog["content"] = content;
  blog["tag_id"] = 1;
  blog["create_time"] = "2019/05/14 12:00";

  std::cout << "==============测试插入=====================" << std::endl;
  BlogTable blog_table(mysql);
  ret = blog_table.Insert(blog);
  std::cout << "Insert: " << ret << std::endl;

  std::cout << "==============测试查找=====================" << std::endl;
  Json::Value blogs;
  ret = blog_table.SelectAll(&blogs);
  std::cout << "SelectAll: " << ret << std::endl
            << writer.write(blogs) << std::endl;

  std::cout << "==============测试更新=====================" << std::endl;
  blog["blog_id"] = 1;
  blog["title"] = "测试更新博客";
  blog["content"] = content;
  blog["tag_id"] = 2;
  blog["create_time"] = "2019/05/20 12:00";
  Json::Value blog_out;
  ret = blog_table.Update(blog);
  std::cout << "Update: " << ret << std::endl;
  ret = blog_table.SelectOne(1, &blog_out);
  std::cout << "SelectOne: " << ret << std::endl
            << writer.write(blog_out) << std::endl;

  std::cout << "==============测试删除=====================" << std::endl;
  int blog_id = 6;
  ret = blog_table.Delete(blog_id);
  std::cout << "Delete: " << ret << std::endl;

  MySQLRelease(mysql);
}

void TestTagTable() {
  bool ret = false;
  Json::StyledWriter writer;
  MYSQL* mysql = MySQLInit();
  TagTable tag_table(mysql);

  std::cout << "==============测试插入=====================" << std::endl;
  Json::Value tag;
  tag["tag_name"] = "Java";
  ret = tag_table.Insert(tag);
  std::cout << "Insert: " << ret << std::endl;

  std::cout << "==============测试查看=====================" << std::endl;
  Json::Value tags;
  ret = tag_table.SelectAll(&tags);
  std::cout << "SelectAll: " << ret << std::endl
            << writer.write(tags) << std::endl;

  std::cout << "==============测试删除状态=====================" << std::endl;
  ret = tag_table.Delete(5);
  std::cout << "ChangeState ret:" << ret << std::endl;

  MySQLRelease(mysql);
}

int main() {
  // TestBlogTable();
  TestTagTable();
  return 0;
}
