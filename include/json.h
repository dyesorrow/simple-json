#pragma once

#include <stdio.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>


class json {
    typedef char int8;
    typedef short int16;
    typedef int int32;
    typedef long long int64;
    typedef unsigned char uchar;
    typedef unsigned char uint8;
    typedef unsigned short uint16;
    typedef unsigned int uint;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;

private:
    std::unordered_map<std::string, std::shared_ptr<json>> data_ks;
    std::vector<std::shared_ptr<json>> data_kn;
    std::string data;
    int data_type = 0;    // 0: no origin data, 1: number, 2: string, 3: null

public:
    json() {
        this->set_null();
    }

    json(const std::string &jsn_str) {
        this->set_null();
        this->from_json_str(jsn_str);
    }

    json &operator[](int i) {
        this->data_type = 0;
        if (data_kn.size() <= i) {
            data_kn.resize(i + 1);
            std::shared_ptr<json> t(new json());
            data_kn[i] = t;
            return *(t.get());
        } else {
            return *(data_kn[i].get());
        }
    }

    json &operator[](const std::string &key) {
        this->data_type = 0;
        auto it = data_ks.find(key);
        if (it == data_ks.end()) {
            std::shared_ptr<json> t(new json());
            data_ks[key] = t;
            return *(t.get());
        } else {
            return *(it->second.get());
        }
    }

    json &operator=(const json &val) {
        this->data_type = val.data_type;
        this->data_kn = val.data_kn;
        this->data_ks = val.data_ks;
        this->data = val.data;
        return *this;
    }

    void set_null() {
        this->data_type = 3;
        this->data_kn.clear();
        this->data_ks.clear();
        this->data.clear();
    }

    bool is_null() {
        return this->data_type == 3;
    }

    bool is_empty() {
        if (this->data_type == 3) {
            return true;
        }
        if (this->data_type == 0) {
            if (this->data_kn.empty() and this->data_ks.empty()) {
                return true;
            }
        }
        return false;
    }

    bool is_object() {
        if (this->data_type != 0) {
            return false;
        }
        if (this->data_kn.empty()) {
            return true;
        }
        return false;
    }

    bool is_array() {
        if (this->data_type != 0) {
            return false;
        }
        if (this->data_ks.empty()) {
            return true;
        }
        return false;
    }

    bool is_number() {
        return this->data_type == 1;
    }

    bool is_string() {
        return this->data_type == 2;
    }

    bool has_member(const std::string &name) {
        if (this->data_type != 0) {
            return false;
        }
        if (this->data_ks.find(name) != this->data_ks.end()) {
            return true;
        }
        return false;
    }

    void delete_key(const std::string &name) {
        this->data_ks.erase(name);
    }

    int array_size() {
        if (this->data_type != 0) {
            return 0;
        }
        return this->data_kn.size();
    }

    void for_each(std::function<void(const std::string &key, json &val)> todo) {
        for (auto &&it : data_ks) {
            if (it.second) {
                todo(it.first, *(it.second.get()));
            }
        }
    }

    void for_each(std::function<void(int i, json &val)> todo) {
        if (this->is_array()) {
            for (int i = 0; i < data_kn.size(); i++) {
                auto &it = data_kn[i];
                todo(i, *(it.get()));
            }

        } else {
            todo(0, *this);    // 数组的特殊处理，如果不是数组，则直接将本对象传入
        }
    }

    void remove(int at) {
        if (data_kn.size() <= at) {
            return;
        }
        data_kn.erase(data_kn.begin() + at);
    }

    void append(json &data) {
        int at = data_kn.size();
        (*this)[at] = data;
    }

    // 快捷构建数组
    template <typename... Args>
    static json array(Args... args) {
        json jsn;
        add_array_item(jsn, args...);
        return jsn;
    }

    int from_file(const std::string file_path) {
        std::ifstream in;
        in.open(file_path);
        std::ostringstream tmp;
        tmp << in.rdbuf();
        std::string text = tmp.str();
        this->set_null();
        int ret = this->from_json_str(text);
        return ret;
    }

    std::string dump() {
        if (this->data_type == 1) {
            return this->data;
        } else if (this->data_type == 2) {
            std::string text = "\"";
            for (int i = 0; i < this->data.length(); i++) {
                if (this->data[i] == '"') {
                    text += "\\\"";
                } else if (this->data[i] == '\n') {
                    text += "\\n";
                } else {
                    text += this->data[i];
                }
            }
            text += "\"";
            return text;
        } else if (this->data_type == 3) {
            return "null";
        } else {
            if (data_ks.empty() and data_kn.empty()) {
                return "{}";
            }
            if (!data_ks.empty() and !data_kn.empty()) {
                return "[illegal data!]";
            }
            if (!data_ks.empty() and data_kn.empty()) {
                // 是对象
                std::string text = "{";
                bool has_data = false;
                for (auto &&it : data_ks) {
                    text += "\"";
                    text += it.first;
                    text += "\"";
                    text += ":";
                    if (data_ks[it.first] != nullptr) {
                        text += data_ks[it.first].get()->dump();
                    } else {
                        text += "null";
                    }
                    text += ",";
                    has_data = true;
                }
                if (has_data) {
                    text.erase(text.end() - 1);
                }
                text += "}";
                return text;
            }
            if (data_ks.empty() and !data_kn.empty()) {
                // 是数组
                std::string text = "[";
                bool has_data = false;
                for (int i = 0; i < data_kn.size(); i++) {
                    if (data_kn[i] != nullptr) {
                        text += data_kn[i].get()->dump();
                    } else {
                        text += "null";
                    }
                    text += ",";
                    has_data = true;
                }
                if (has_data) {
                    text.erase(text.end() - 1);
                }
                text += "]";
                return text;
            }
        }

        return "{}";
    }

public:
    template <typename T>
    void operator=(const T &val) {
        if constexpr (std::is_same<int, T>::value or std::is_same<int64, T>::value or std::is_same<float, T>::value or std::is_same<double, T>::value) {
            this->data_type = 1;
            this->data = std::to_string(val);
            return;
        }
        if constexpr (std::is_same<std::string, T>::value) {
            this->data_type = 2;
            this->data = val;
        }
    }

    void operator=(const char *val) {
        this->data_type = 2;
        this->data = val;
    }

    template <typename T>
    bool operator==(const T &val) {
        return this->get<T>() == val;
    }

    template <typename T>
    bool operator!=(const T &val) {
        return !(*this == val);
    }

    template <typename T>
    T get() {
        if constexpr (std::is_same<std::string, T>::value) {
            if (this->data_type != 2) {
                return "";
            }
            return this->data;
        }
        if constexpr (std::is_same<int, T>::value) {
            if (this->data_type != 1) {
                return 0;
            }
            return std::stoi(this->data);
        }
        if constexpr (std::is_same<int64, T>::value) {
            if (this->data_type != 1) {
                return 0;
            }
            return std::stol(this->data);
        }
        if constexpr (std::is_same<float, T>::value) {
            if (this->data_type != 1) {
                return 0;
            }
            return std::stof(this->data);
        }
        if constexpr (std::is_same<double, T>::value) {
            if (this->data_type != 1) {
                return 0;
            }
            return std::stod(this->data);
        }
    }

    template <typename T>
    T get(const T &default_val) {
        if (this->is_null()) {
            return default_val;
        }
        return this->get<T>();
    }

    template <typename T>
    void append(const T &val) {
        int size = this->array_size();
        (*this)[size] = val;
    }

    // 获取错误信息
    std::string error(std::string set = "") {
        thread_local std::string error_info;
        if (set != "") {
            error_info = set;
        }
        return error_info;
    }

private:
    template <typename... Args>
    static std::string format(const char *fmt, const Args... args) {
        int length = std::snprintf(nullptr, 0, fmt, args...);
        char data[length + 1];
        std::snprintf(data, length + 1, fmt, args...);
        data[length] = 0;
        return data;
    }

    static void add_array_item(json &jsn) {
    }

    template <typename T, typename... Args>
    static void add_array_item(json &jsn, T val, Args... args) {
        jsn.append(val);
        add_array_item(jsn, args...);
    }

    int from_json_str(const std::string &jsn_str) {
        enum token_type {
            L_brackets,
            R_brackets,
            L_curly_brackets,
            R_curly_brackets,
            Colon,
            Comma,
            String,
            Number,
            Null,
        };
        struct token {
            int type;
            int start;
            int line;
            std::string text;
            std::string to_string() const {
                return text;
            }
        };

        thread_local std::vector<token> tokens;
        tokens.clear();
        std::string error_str;

        auto lexer = [](std::vector<token> &tokens, const std::string &jsn_str, std::string &error) -> int {
            int line = 1;
            int last_line_end = 0;
            for (int i = 0; i < jsn_str.size(); i++) {
                char c = jsn_str.at(i);
                if (c == ' ' or c == '\t' or c == '\r') {
                    continue;
                }
                if (c == '\n') {
                    ++line;
                    last_line_end = i;
                    continue;
                }
                if (c == '{') {
                    token t;
                    t.type = token_type::L_curly_brackets;
                    t.start = i - last_line_end;
                    t.text = "{";
                    t.line = line;
                    tokens.push_back(t);
                    continue;
                }
                if (c == '}') {
                    token t;
                    t.type = token_type::R_curly_brackets;
                    t.start = i - last_line_end;
                    t.text = "}";
                    t.line = line;
                    tokens.push_back(t);
                    continue;
                }
                if (c == '[') {
                    token t;
                    t.type = token_type::L_brackets;
                    t.start = i - last_line_end;
                    t.text = "[";
                    t.line = line;
                    tokens.push_back(t);
                    continue;
                }
                if (c == ']') {
                    token t;
                    t.type = token_type::R_brackets;
                    t.start = i - last_line_end;
                    t.text = "]";
                    t.line = line;
                    tokens.push_back(t);
                    continue;
                }
                if (c == ':') {
                    token t;
                    t.type = token_type::Colon;
                    t.start = i - last_line_end;
                    t.text = ":";
                    t.line = line;
                    tokens.push_back(t);
                    continue;
                }
                if (c == ',') {
                    token t;
                    t.type = token_type::Comma;
                    t.start = i - last_line_end;
                    t.text = ",";
                    t.line = line;
                    tokens.push_back(t);
                    continue;
                }
                if ((c >= '0' and c <= '9') || c == '-') {
                    token t;
                    t.type = token_type::Number;
                    t.text += c;
                    for (int k = i + 1; k < jsn_str.size(); k++) {
                        char kc = jsn_str.at(k);
                        if ((kc >= '0' and kc <= '9') or kc == '.') {
                            t.text += kc;
                        } else {
                            i = k - 1;
                            break;
                        }
                    }
                    t.start = i - last_line_end;
                    t.line = line;
                    tokens.push_back(t);
                    continue;
                }
                if (c == '\"') {
                    token t;
                    t.type = token_type::String;
                    thread_local std::string text;
                    text.clear();
                    for (int k = i + 1; k < jsn_str.size(); k++) {
                        char kc = jsn_str.at(k);
                        if (kc == '\"' and jsn_str.at(k - 1) != '\\') {
                            i = k;
                            break;
                        } else {
                            text += kc;
                        }
                    }

                    // 去除转义
                    for (int i = 0; i < text.length(); i++) {
                        if (i + 1 < text.length() and text[i] == '\\' and text[i + 1] == '"') {
                            t.text += '"';
                            i++;
                        } else if (i + 1 < text.length() and text[i] == '\\' and text[i + 1] == 'n') {
                            t.text += '\n';
                            i++;
                        } else {
                            t.text += text[i];
                        }
                    }
                    t.start = i - last_line_end;
                    t.line = line;
                    tokens.push_back(t);
                    continue;
                }
                if (c == 'n' and jsn_str.substr(i, 4) == "null") {
                    token t;
                    t.type = token_type::Null;
                    t.text = "null";
                    t.line = line;
                    t.start = i - last_line_end;
                    tokens.push_back(t);
                    i = i + 3;
                    continue;
                }
                if (c == 't' and jsn_str.substr(i, 4) == "true") {
                    token t;
                    t.type = token_type::Number;
                    t.start = i - last_line_end;
                    t.text = "1";
                    t.line = line;
                    tokens.push_back(t);
                    i = i + 3;
                    continue;
                }
                if (c == 'f' and jsn_str.substr(i, 5) == "false") {
                    token t;
                    t.type = token_type::Number;
                    t.start = i - last_line_end;
                    t.text = "0";
                    t.line = line;
                    tokens.push_back(t);
                    i = i + 4;
                    continue;
                }
                error = format("unkown token at {}, c={}, 10 word after it is: {}", i, c, jsn_str.substr(i, 10));
                return -1;
            }
            return 1;
        };

        std::function<int(json & jsn, std::vector<token> & tokens, int &at, std::string &error)> prase;
        prase = [&prase](json &jsn, std::vector<token> &tokens, int &at, std::string &error) -> int {
            if (tokens.size() <= at) {
                error = format("fail when prase token at={}", at);
                return -1;
            }
            auto &t = tokens[at];
            if (t.type == token_type::L_curly_brackets) {
                jsn.data_type = 0;
                while (at < tokens.size()) {
                    at++;
                    auto &t1 = tokens[at];
                    if (t1.type == token_type::R_curly_brackets) {
                        return 1;
                    }
                    if (t1.type != token_type::String) {
                        error = format("expect string hear but get: {}, at={}", t1.to_string(), at);
                        return -2;
                    }

                    at++;
                    auto &t2 = tokens[at];
                    if (t2.type != token_type::Colon) {
                        error = format("expect colon hear but get: {}, at={}", t2.to_string(), at);
                        return -3;
                    }

                    at++;
                    auto ret = prase(jsn[t1.text], tokens, at, error);
                    if (ret < 0) {
                        return -1;
                    }

                    at++;
                    auto &t3 = tokens[at];
                    if (t3.type == token_type::R_curly_brackets) {
                        return 1;
                    }
                    if (t3.type != token_type::Comma) {
                        error = format("expect comma hear but get: {}, at={}", t3.to_string(), at);
                        return -4;
                    }
                }
                return 1;
            }
            if (t.type == token_type::L_brackets) {
                jsn.data_type = 0;
                int index_num = 0;
                while (at < tokens.size()) {
                    at++;
                    auto &t1 = tokens[at];
                    if (t1.type == token_type::R_brackets) {
                        return 1;
                    }

                    auto ret = prase(jsn[index_num++], tokens, at, error);
                    if (ret < 0) {
                        return -1;
                    }

                    at++;
                    auto &t3 = tokens[at];
                    if (t3.type == token_type::R_brackets) {
                        return 1;
                    }
                    if (t3.type != token_type::Comma) {
                        error = format("expect comma hear but get: {}, at={}", t3.to_string(), at);
                        return -5;
                    }
                }
                return 1;
            }
            if (t.type == token_type::Number) {
                jsn.data_type = 1;
                jsn.data = t.text;
                return 1;
            }
            if (t.type == token_type::String) {
                jsn.data_type = 2;
                jsn.data = t.text;
                return 1;
            }
            if (t.type == token_type::Null) {
                jsn.data_type = 3;
                jsn.data = t.text;
                return 1;
            }
            error = format("unkown token when prase: {}, at={}", t.to_string(), at);
            return -6;
        };

        int ret = lexer(tokens, jsn_str, error_str);
        if (ret < 0) {
            error(format("json_praser::lexer return[{}]: {}", ret, error_str));
            return -1;
        }
        int at = 0;
        ret = prase(*this, tokens, at, error_str);
        if (ret < 0) {
            error(format("json_praser::prase return[{}]: {}", ret, error_str));
            return -1;
        }
        tokens.clear();
        return 0;
    }
};
