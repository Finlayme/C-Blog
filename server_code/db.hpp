///////////////////////////////////////////////////////
// 这个文件相当于 model 层. 
// 只进行数据的基本 CURD , 不涉及到更复杂的数据加工
///////////////////////////////////////////////////////

#pragma once
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <string>
#include <memory>
#include <mysql/mysql.h>
#include <jsoncpp/json/json.h>

namespace blog_system {

static MYSQL* MySQLInit() {
  MYSQL* connect_fd = mysql_init(NULL);
  if (mysql_real_connect(connect_fd, "127.0.0.1", "root", "",
											  "blog_system", 3306, NULL, 0) == NULL) {
    printf("连接失败! %s\n", mysql_error(connect_fd));
    return NULL;
  }
  mysql_set_character_set(connect_fd, "utf8");
  return connect_fd;
}

static void MySQLRelease(MYSQL* mysql) {
  mysql_close(mysql);
}

class BlogTable {
public:
  BlogTable(MYSQL* mysql) : mysql_(mysql) {  }

  bool Insert(const Json::Value& blog) {
    // 如果拿一个完整的课件做实验, 会发现: 
    // 由于博客内容中可能包含一些特殊字符(\n, '', "" 等), 会导致拼装出的 sql 语句有问题.
    // 应该使用 mysql_real_escape_string 对 content 字段来进行转义
    // 转义只是为了让 SQL 语句拼接正确. 实际上插入成功后数据库的内容已经自动转义回来了. 
    const std::string& content = blog["content"].asString();
    // 文档上要求转义的缓冲区长度必须是之前的 2 倍 + 1
    // 使用 unique_ptr 管理内存
    std::unique_ptr<char> content_escape(new char[content.size() * 2 + 1]);
    mysql_real_escape_string(mysql_, content_escape.get(), content.c_str(), content.size());
    
    // 插入的博客内容可能较长, 需要搞个大点的缓冲区(根据用户请求的长度自适应),
    std::unique_ptr<char> sql(new char[content.size() * 2 + 4096]);
    sprintf(sql.get(), "insert into blog_table values(null, '%s', '%s', %d,'%s')",
        blog["title"].asCString(), content_escape.get(),
        blog["tag_id"].asInt(), blog["create_time"].asCString());
    int ret = mysql_query(mysql_, sql.get());
    if (ret != 0) {
      printf("执行 sql 失败! sql=%s, %s\n", sql.get(), mysql_error(mysql_));
      return false;
    }
    return true;
  }

  bool SelectAll(Json::Value* blogs, const std::string& tag_id = "") {
    char sql[1024 * 4] = {0};
    // 可以根据 tag_id 来筛选结果
    if (tag_id.empty()) {
      sprintf(sql, "select blog_id, title, tag_id, create_time from blog_table");
    } else {
      sprintf(sql, "select blog_id, title, tag_id, create_time\
          from blog_table where tag_id = '%s'", tag_id.c_str());
    }
    int ret = mysql_query(mysql_, sql);
    if (ret != 0) {
      printf("执行 sql 失败! %s\n", mysql_error(mysql_));
      return false;
    }
    MYSQL_RES* result = mysql_store_result(mysql_);
    if (result == NULL) {
      printf("获取结果失败! %s\n", mysql_error(mysql_));
      return false;
    }
    int rows = mysql_num_rows(result);
    for (int i = 0; i < rows; ++i) {
      MYSQL_ROW row = mysql_fetch_row(result);
      Json::Value blog;
      blog["blog_id"] = atoi(row[0]);
      blog["title"] = row[1];
      blog["tag_id"] = atoi(row[2]);
      blog["create_time"] = row[3];
      // 遍历结果依次加入到 dishes 中
      blogs->append(blog);
    }
    return true;
  }

  bool SelectOne(int32_t blog_id, Json::Value* blog) {
    char sql[1024 * 4] = {0};
    sprintf(sql, "select * from blog_table where blog_id = %d", blog_id);
    int ret = mysql_query(mysql_, sql);
    if (ret != 0) {
      printf("执行 sql 失败! %s\n", mysql_error(mysql_));
      return false;
    }
    MYSQL_RES* result = mysql_store_result(mysql_);
    if (result == NULL) {
      printf("获取结果失败! %s\n", mysql_error(mysql_));
      return false;
    }
    int rows = mysql_num_rows(result);
    if (rows != 1) {
      printf("查找结果不为 1 条. rows = %d!\n", rows);
      return false;
    }
    MYSQL_ROW row = mysql_fetch_row(result);
    (*blog)["blog_id"] = atoi(row[0]);
    (*blog)["title"] = row[1];
    (*blog)["content"] = row[2];
    (*blog)["tag_id"] = atoi(row[3]);
    (*blog)["create_time"] = row[4];
    return true;
  }

  bool Update(const Json::Value& blog) {
    // 如果拿一个完整的课件做实验, 会发现: 
    // 由于博客内容中可能包含一些特殊字符(\n, '', "" 等), 会导致拼装出的 sql 语句有问题.
    // 应该使用 mysql_real_escape_string 对 content 字段来进行转义
    // 转义只是为了让 SQL 语句拼接正确. 实际上插入成功后数据库的内容已经自动转义回来了. 
    const std::string& content = blog["content"].asString();
    // 文档上要求转义的缓冲区长度必须是之前的 2 倍 + 1
    // 使用 unique_ptr 管理内存
    std::unique_ptr<char> content_escape(new char[content.size() * 2 + 1]);
    mysql_real_escape_string(mysql_, content_escape.get(), content.c_str(), content.size());
    
    // 插入的博客内容可能较长, 需要搞个大点的缓冲区(根据用户请求的长度自适应),
    std::unique_ptr<char> sql(new char[content.size() * 2 + 4096]);
    sprintf(sql.get(), "update blog_table SET title='%s', content='%s',\
        tag_id=%d where blog_id=%d",
        blog["title"].asCString(),
        content_escape.get(),
        blog["tag_id"].asInt(),
        blog["blog_id"].asInt());

    // DEBUG 用于调试
    // printf("[SQL] %s\n", sql);

    int ret = mysql_query(mysql_, sql.get());
    if (ret != 0) {
      printf("执行 sql 失败! sql=%s, %s\n", sql.get(), mysql_error(mysql_));
      return false;
    }
    return true;
  }

  bool Delete(int blog_id) {
    char sql[1024 * 4] = {0};
    sprintf(sql, "delete from blog_table where blog_id=%d", blog_id);
    int ret = mysql_query(mysql_, sql);
    if (ret != 0) {
      printf("执行 sql 失败! sql=%s, %s\n", sql, mysql_error(mysql_));
      return false;
    }
    return true;
  }
private:
  MYSQL* mysql_;
};

class TagTable {
public:
  TagTable(MYSQL* mysql) : mysql_(mysql) {  }

  bool SelectAll(Json::Value* tags) {
    char sql[1024 * 4] = {0};
    sprintf(sql, "select * from tag_table");
    int ret = mysql_query(mysql_, sql);
    if (ret != 0) {
      printf("执行 sql 失败! %s\n", mysql_error(mysql_));
      return false;
    }
    MYSQL_RES* result = mysql_store_result(mysql_);
    if (result == NULL) {
      printf("获取结果失败! %s\n", mysql_error(mysql_));
      return false;
    }
    int rows = mysql_num_rows(result);
    for (int i = 0; i < rows; ++i) {
      MYSQL_ROW row = mysql_fetch_row(result);
      Json::Value tag;
      tag["tag_id"] = atoi(row[0]);  
      tag["tag_name"] = row[1];
      tags->append(tag);
    }
    return true;
  }

  bool Insert(const Json::Value& tag) {
    char sql[1024 * 4] = {0};
    // 此处 dish_ids 需要先转成字符串(本来是一个对象,
    // 形如 [1, 2, 3]. 如果不转, 是无法 asCString)
    sprintf(sql, "insert into tag_table values(null, '%s')",
        tag["tag_name"].asCString());
    int ret = mysql_query(mysql_, sql);
    if (ret != 0) {
      printf("执行 sql 失败! sql=%s, %s\n", sql, mysql_error(mysql_));
      return false;
    }
    return true;
  }

  bool Delete(int tag_id) {
    char sql[1024 * 4] = {0};
    sprintf(sql, "delete from tag_table where tag_id = %d", tag_id);
    int ret = mysql_query(mysql_, sql);
    if (ret != 0) {
      printf("执行 sql 失败! sql=%s, %s\n", sql, mysql_error(mysql_));
      return false;
    }
    return true;
  }
private:
  MYSQL* mysql_;
};
}  // end blog_system
