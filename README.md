<div align="center">
  
# HttpServer

[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg?style=for-the-badge)](LICENSE)
[![Language](https://img.shields.io/badge/language-C++-orange.svg?style=for-the-badge)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows-blue?style=for-the-badge)]()
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=for-the-badge)](http://makeapullrequest.com)

一个用 C++17 实现的轻量级 HTTP/1.1 静态文件服务器
</div>

目前只支持单线程，WinGW，不支持跨平台，后面有时间更新。
## 编译运行
```bash
# 编译
g++ -std=c++17 -o server.exe main.cpp -lws2_32

# 运行
server.exe

# 或者带端口号（默认8080）
server.exe 80
```
