///////////////////////////////////////////////////////
// 构建 HTTP 服务器提供约定的 API 接口
///////////////////////////////////////////////////////

#include <signal.h>
#include <jsoncpp/json/json.h>
#include "util.hpp"
#include "db.hpp"
#include "httplib.h"

MYSQL* mysql = NULL;

int main() {
  using namespace httplib; 
  using namespace blog_system;
  Server server;
  
  // 1. 数据库客户端初始化和释放
  mysql = MySQLInit();
  signal(SIGINT, [](int) { MySQLRelease(mysql); exit(0);});
  BlogTable blog_table(mysql);
  TagTable tag_table(mysql);

  // 2. 设置路由
  // 新增博客
  server.Post("/blog", [&blog_table](const Request& req, Response& resp) {
    LOG(INFO) << "新增博客: " << req.body << std::endl;
    Json::Reader reader;
    Json::FastWriter writer;
    Json::Value req_json;
    Json::Value resp_json;
    // 1. 请求解析成 Json 格式
    bool ret = reader.parse(req.body, req_json);
    if (!ret) {
      // 请求解析出错, 返回一个400响应
      resp_json["ok"] = false;
      resp_json["reason"] = "parse Request failed!\n";
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 2. 进行参数校验
    if (req_json["title"].empty() || req_json["content"].empty()
        || req_json["tag_id"].empty() || req_json["create_time"].empty()) {
      resp_json["ok"] = false;
      resp_json["reason"] = "Request fields error!\n";
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 3. 调用数据库接口进行操作数据
    ret = blog_table.Insert(req_json);
    if (!ret) {
      resp_json["ok"] = false;
      resp_json["reason"] = "Insert failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 4. 封装正确的返回结果
    resp_json["ok"] = true;
    resp.set_content(writer.write(resp_json), "application/json");
    return;
  });
  // 查看所有博客(可以按标签筛选)
  server.Get("/blog", [&blog_table](const Request& req, Response& resp) {
    LOG(INFO) << "查看所有博客" << std::endl;
    Json::Reader reader;
    Json::FastWriter writer;
    Json::Value resp_json;
    // 如果没传 tag_id, 返回的是空字符串
    const std::string& tag_id = req.get_param_value("tag_id");
    // 对于查看博客来说 API 没有请求参数, 不需要解析参数和校验了, 直接构造结果即可
    // 1. 调用数据库接口查询数据
    Json::Value blogs;
    bool ret = blog_table.SelectAll(&blogs, tag_id);
    if (!ret) {
      resp_json["ok"] = false;
      resp_json["reason"] = "SelectAll failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 2. 构造响应结果
    resp.set_content(writer.write(blogs), "application/json");
    return;
  });

  // 查看一篇博客内容
  server.Get(R"(/blog/(\d+))", [&blog_table](const Request& req, Response& resp) {
    Json::Value resp_json;
    Json::FastWriter writer;
    // 1. 解析获取 blog_id
    int blog_id = std::stoi(req.matches[1]);
    LOG(INFO) << "查看指定博客: " << blog_id << std::endl;
    // 2. 调用数据库接口查看博客
    bool ret = blog_table.SelectOne(blog_id, &resp_json);
    if (!ret) {
      resp_json["ok"] = false;
      resp_json["reason"] = "SelectOne failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 3. 包装正确的响应
    resp_json["ok"] = true;
    resp.set_content(writer.write(resp_json), "application/json");
    return;
  });

  // 删除博客
  // raw string(c++ 11), 转义字符不生效. 用来表示正则表达式正好合适
  // 关于正则表达式, 只介绍最基础概念即可. \d+ 表示匹配一个数字
  // http://help.locoy.com/Document/Learn_Regex_For_30_Minutes.htm
  server.Delete(R"(/blog/(\d+))", [&blog_table](const Request& req, Response& resp) {
    Json::Value resp_json;
    Json::FastWriter writer;
    // 1. 解析获取 blog_id
    // 使用 matches[1] 就能获取到 blog_id
    // LOG(INFO) << req.matches[0] << "," << req.matches[1] << "\n";
    int blog_id = std::stoi(req.matches[1]);
    LOG(INFO) << "删除指定博客: " << blog_id << std::endl;
    // 2. 调用数据库接口删除博客
    bool ret = blog_table.Delete(blog_id);
    if (!ret) {
      resp_json["ok"] = false;
      resp_json["reason"] = "Delete failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 3. 包装正确的响应
    resp_json["ok"] = true;
    resp.set_content(writer.write(resp_json), "application/json");
    return;
  });
  // 修改博客
  server.Put(R"(/blog/(\d+))", [&blog_table](const Request& req, Response& resp) {
    Json::Reader reader;
    Json::FastWriter writer;
    Json::Value req_json;
    Json::Value resp_json;
    // 1. 获取到博客 id
    int blog_id = std::stoi(req.matches[1]);
    LOG(INFO) << "修改博客 " << blog_id << "|" << req.body << std::endl;
    // 2. 解析博客信息
    bool ret = reader.parse(req.body, req_json);
    if (!ret) {
      // 请求解析出错, 返回一个400响应
      resp_json["ok"] = false;
      resp_json["reason"] = "parse Request failed!\n";
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // [注意!!] 一定要记得补充上 dish_id
    req_json["blog_id"] = blog_id;
    // 3. 校验博客信息
    if (req_json["title"].empty() || req_json["content"].empty()
        || req_json["tag_id"].empty()) {
      // 请求解析出错, 返回一个400响应
      resp_json["ok"] = false;
      resp_json["reason"] = "Request has no name or price!\n";
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 4. 调用数据库接口进行修改
    ret = blog_table.Update(req_json);
    if (!ret) {
      resp_json["ok"] = false;
      resp_json["reason"] = "Update failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 5. 封装正确的数据
    resp_json["ok"] = true;
    resp.set_content(writer.write(resp_json), "application/json");
    return;
  });
  // 新增标签
  server.Post("/tag", [&tag_table](const Request& req, Response& resp) {
    LOG(INFO) << "新增订单: " << req.body << std::endl;
    Json::Reader reader;
    Json::FastWriter writer;
    Json::Value req_json;
    Json::Value resp_json;
    // 1. 请求解析成 Json 格式
    bool ret = reader.parse(req.body, req_json);
    if (!ret) {
      // 请求解析出错, 返回一个400响应
      resp_json["ok"] = false;
      resp_json["reason"] = "parse Request failed!\n";
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 2. 校验标签格式
    if (req_json["tag_name"].empty()) {
      // 请求解析出错, 返回一个400响应
      resp_json["ok"] = false;
      resp_json["reason"] = "Request has no table_id or time or dish_ids!\n";
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 3. 调用数据库接口, 插入标签
    ret = tag_table.Insert(req_json);
    if (!ret) {
      resp_json["ok"] = false;
      resp_json["reason"] = "TagTable Insert failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 4. 返回正确的结果
    resp_json["ok"] = true;
    resp.set_content(writer.write(resp_json), "application/json");
  });

  // 删除标签
  server.Delete(R"(/tag/(\d+))", [&tag_table](const Request& req, Response& resp) {
    Json::Value resp_json;
    Json::FastWriter writer;
    // 1. 解析出 tag_id
    int tag_id = std::stoi(req.matches[1]);
    LOG(INFO) << "删除指定标签: " << tag_id << std::endl;
    // 2. 执行数据库操作删除标签
    bool ret = tag_table.Delete(tag_id);
    if (!ret) {
      resp_json["ok"] = false;
      resp_json["reason"] = "TagTable Insert failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 3. 包装正确的结果
    resp_json["ok"] = true;
    resp.set_content(writer.write(resp_json), "application/json");
    return;
  });

  // 获取所有标签
  server.Get("/tag", [&tag_table](const Request& req, Response& resp) {
    (void) req;
    LOG(INFO) << "查看所有订单" << std::endl;
    Json::Reader reader;
    Json::FastWriter writer;
    Json::Value resp_json;
    // 1. 调用数据库接口查询数据
    Json::Value tags;
    bool ret = tag_table.SelectAll(&tags);
    if (!ret) {
      resp_json["ok"] = false;
      resp_json["reason"] = "SelectAll failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 2. 构造响应结果
    resp.set_content(writer.write(tags), "application/json");
    return;
  });

  // 设置静态文件目录
  server.set_base_dir("./wwwroot");

  server.listen("0.0.0.0", 9093);
  return 0;
}

