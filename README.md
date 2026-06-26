# ThoseCloudObjects (Python 版)

# 县小威移植

Objects.level.bin Editor 的 Python 重构版本，支持 Termux。

## 功能

- 将 `.bin` (TGCL 格式) 转换为可读的 JSON
- 将编辑后的 JSON 转换回 `.bin`
- 完全兼容原版 C++ 版本的读写逻辑
- 支持跨平台: Linux / Termux / Windows / macOS

## 安装

无需额外依赖，仅使用 Python 标准库（Python 3.7+）。

在 Termux 中:
```bash
pkg install python
```

## 用法

### bin -> json
```bash
python object_editor.py Objects.level.bin
```
会生成 `Objects.level.bin.json`

### json -> bin
```bash
python object_editor.py Objects.level.bin.json
```
会生成 `Objects.level_new.level.bin`

## 文件说明

| 文件 | 说明 |
|---|---|
| `object_editor.py` | 入口脚本 |
| `object_format.py` | 核心解析/序列化逻辑 |

## 与原版的区别

- 使用 Python `struct` 替代 C++ 二进制读写
- 使用标准库 `json` 替代 `nlohmann/json`
- 路径处理使用 `os.path`，兼容 Windows 和 Unix（包括 Termux）
- 80-bit long double 在 Python 中无原生支持，以 hex 字符串或近似方式处理
