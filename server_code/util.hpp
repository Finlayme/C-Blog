#pragma once

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <cassert>

#include <sys/time.h>

#include <boost/algorithm/string.hpp>

///////////////////////////////////////////////////////
// 封装时间戳获取函数
///////////////////////////////////////////////////////

class TimeUtil {
public:
  static int64_t TimeStamp() {
    struct timeval tv;
    ::gettimeofday(&tv, NULL);
    return tv.tv_sec;
  }

  static int64_t TimeStampMS() {
    struct timeval tv;
    ::gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
  }
};

///////////////////////////////////////////////////////
// 封装一个简单的日志库
///////////////////////////////////////////////////////

enum LogLevel {
  INFO,
  WARNING,
  ERROR,
  FATAL,
};

// 日志格式形如:
// [I1544678896 util.hpp:24] hello
inline std::ostream& Log(LogLevel level, const std::string& file_name,
    int line) {
  std::string log_prefix = "[";
  if (level == INFO) {
    log_prefix += "I";
  } else if (level == WARNING) {
    log_prefix += "W";
  } else if (level == ERROR) {
    log_prefix += "E";
  } else if (level == FATAL) {
    log_prefix += "F";
  }
  log_prefix += std::to_string(TimeUtil::TimeStamp());
  log_prefix += " ";
  log_prefix += file_name;
  log_prefix += ":";
  log_prefix += std::to_string(line);
  log_prefix += "] ";
  std::cout << log_prefix;
  return std::cout;
}

#define LOG(level) Log(level, __FILE__, __LINE__)

///////////////////////////////////////////////////////
// 封装文件读写操作
///////////////////////////////////////////////////////
class FileUtil {
public:
  static bool ReadFile(const std::string& file_name, std::string* content) {
    content->clear();
    std::ifstream file(file_name.c_str());
    if (!file.is_open()) {
      return false;
    }
    std::string line;
    while (std::getline(file, line)) {
      (*content) += line + "\n";
    }
    file.close();
    return true;
  }

  static bool WriteFile(const std::string& file_name, const std::string& content) {
    std::ofstream file(file_name.c_str());
    if (!file.is_open()) {
      return false;
    }
    file.write(content.data(), content.size());
    file.close();
    return true;
  }
};

///////////////////////////////////////////////////////
// 封装 string 常用操作
///////////////////////////////////////////////////////
class StringUtil {
public:
  static void Split(const std::string& input, const std::string& split_char,
             std::vector<std::string>* output) {
    boost::split(*output, input, boost::is_any_of(split_char), boost::token_compress_off);
  }

  static void Replace(const std::string& input, const std::string& old_pattern,
      const std::string& new_pattern, std::string* output) {
    *output = boost::replace_all_copy(input, old_pattern, new_pattern);
  }
};


///////////////////////////////////////////////////////
// 封装 url 相关操作
// 参考 https://blog.csdn.net/gemo/article/details/8468311
///////////////////////////////////////////////////////
class UrlUtil {
public:
	static std::string UrlEncode(const std::string& str) {
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++) {
			if (isalnum((unsigned char)str[i]) || 
					(str[i] == '-') ||
					(str[i] == '_') || 
					(str[i] == '.') || 
					(str[i] == '~'))
				strTemp += str[i];
			else if (str[i] == ' ')
				strTemp += "+";
			else {
				strTemp += '%';
				strTemp += ToHex((unsigned char)str[i] >> 4);
				strTemp += ToHex((unsigned char)str[i] % 16);
			}
		}
		return strTemp;
	}

	static std::string UrlDecode(const std::string& str) {
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (str[i] == '+') strTemp += ' ';
			else if (str[i] == '%')
			{
				assert(i + 2 < length);
				unsigned char high = FromHex((unsigned char)str[++i]);
				unsigned char low = FromHex((unsigned char)str[++i]);
				strTemp += high*16 + low;
			}
			else strTemp += str[i];
		}
		return strTemp;
	}

  // 将 Post 请求中的 body 直接进行解析
  // 使用 boost 来直接 split. 
  // #include <boost/algorithm/string.hpp>
  static void ParseBody(const std::string& body,
      std::unordered_map<std::string, std::string>* params) {
    // 先进行字符串切分解析
    std::vector<std::string> tokens;
    StringUtil::Split(body, "&", &tokens);
    for (const auto& token : tokens) {
      std::vector<std::string> kv;
      StringUtil::Split(token, "=", &kv);
      if (kv.size() != 2) {
        continue;
      }
      // 针对获取到的结果进行 urldecode
      (*params)[kv[0]] = UrlDecode(kv[1]);
    }
  }
private:
	static unsigned char ToHex(unsigned char x) { 
		return  x > 9 ? x + 55 : x + 48; 
	}

	static unsigned char FromHex(unsigned char x) { 
		unsigned char y = '\0';
		if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
		else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
		else if (x >= '0' && x <= '9') y = x - '0';
		else assert(0);
		return y;
	}
};
