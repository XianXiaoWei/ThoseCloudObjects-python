#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ThoseCloudObjects - Python 版入口
支持 Termux / Linux / Windows / macOS
用法:
    python object_editor.py <file.bin>    # 转换为 JSON
    python object_editor.py <file.json>   # 转换回 bin
"""

import os
import sys

from object_format import load_objects, save_objects


def main():
    if len(sys.argv) < 2:
        print("用法: python object_editor.py <文件路径>")
        print("  .bin   -> 转换为 JSON")
        print("  .json  -> 转换回 .bin")
        print("示例:")
        print("  python object_editor.py Objects.level.bin")
        print("  python object_editor.py Objects.level.bin.json")
        sys.exit(1)

    for arg in sys.argv[1:]:
        file_path = arg
        ext = os.path.splitext(file_path)[1].lower()

        if ext == '.bin':
            load_objects(file_path)
        elif ext == '.json':
            save_objects(file_path)
        elif ext == '.mesh':
            print(".mesh 暂不支持")
        elif ext == '.mesh_extracted':
            print(".mesh_extracted 暂不支持")
        elif ext == '.meshes':
            print(".meshes 暂不支持，抱歉!")
        else:
            print(f"不支持的文件类型: {ext}")


if __name__ == '__main__':
    main()
