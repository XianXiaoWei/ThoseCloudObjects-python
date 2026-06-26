#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ThoseCloudObjects - Python 版
Objects.level.bin 编辑器（支持 Termux）
基于 C++ 原版重构
"""

import struct
import json
import os
import sys
import math


def ieee_float(f):
    """将 uint32 位模式解释为 float"""
    return struct.unpack('<f', struct.pack('<I', f))[0]


def float_to_ieee(f):
    """将 float 打包为 uint32 位模式"""
    return struct.unpack('<I', struct.pack('<f', f))[0]


class BSTHeader:
    """TGCL 文件头"""
    def __init__(self):
        self.magic = b''
        self.version = 0
        self.class_length = 0
        self.property_count = 0
        self.bst_node_count = 0
        self.object_ptr_count = 0
        self.class_offset = 0
        self.property_offset = 0
        self.property_name_offset = 0
        self.bst_node_offset = 0
        self.file_size = 0

    @classmethod
    def from_file(cls, f):
        h = cls()
        h.magic = f.read(4)
        (h.version, h.class_length, h.property_count,
         h.bst_node_count, h.object_ptr_count, h.class_offset,
         h.property_offset, h.property_name_offset,
         h.bst_node_offset, h.file_size) = struct.unpack('<10I', f.read(40))
        return h

    def write(self, f):
        f.write(b'TGCL')
        f.write(struct.pack('<10I',
                            self.version, self.class_length, self.property_count,
                            self.bst_node_count, self.object_ptr_count, self.class_offset,
                            self.property_offset, self.property_name_offset,
                            self.bst_node_offset, self.file_size))


class ClassDef:
    """类定义"""
    def __init__(self):
        self.class_property_name_offset = 0
        self.class_property_starting_index = 0
        self.class_property_count = 0
        self.class_name = ''

    @classmethod
    def from_file(cls, f):
        c = cls()
        (c.class_property_name_offset,
         c.class_property_starting_index,
         c.class_property_count) = struct.unpack('<3I', f.read(12))
        return c

    def write(self, f):
        f.write(struct.pack('<3I',
                            self.class_property_name_offset,
                            self.class_property_starting_index,
                            self.class_property_count))


class PropertyDef:
    """属性定义"""
    def __init__(self):
        self.property_type = 0
        self.property_name_offset = 0
        self.object_byte_size = 0
        self.array_index = 0
        self.property_name = ''

    @classmethod
    def from_file(cls, f):
        p = cls()
        (p.property_type, p.property_name_offset,
         p.object_byte_size, p.array_index) = struct.unpack('<4I', f.read(16))
        return p

    def write(self, f):
        f.write(struct.pack('<4I',
                            self.property_type, self.property_name_offset,
                            self.object_byte_size, self.array_index))


def read_null_terminated_string(f):
    """读取以 \0 结尾的字符串"""
    result = bytearray()
    while True:
        ch = f.read(1)
        if not ch or ch == b'\x00':
            break
        result.extend(ch)
    return result.decode('utf-8', errors='replace')


def should_read_uint32_as_integer(property_name):
    """判断属性名是否需要以整数形式读取 uint32"""
    if property_name == 'bstGuid':
        return True
    return 'BstGuid' in property_name


def read_class(f, json_data, index, all_properties, classes_ref, wrap_class_name=True):
    """递归读取类数据"""
    class_json = json_data.setdefault(classes_ref[index].class_name, {}) if wrap_class_name else json_data

    for prop in all_properties[index]:
        if prop.property_type == 0:  # General Value
            if prop.object_byte_size == 1:  # Bool
                val = struct.unpack('<B', f.read(1))[0]
                if val <= 1:
                    class_json[prop.property_name] = bool(val)
                else:
                    class_json[prop.property_name] = f"[Unknown]{val:02X}"
            elif prop.object_byte_size == 2:  # Unsigned short
                val = struct.unpack('<H', f.read(2))[0]
                class_json[prop.property_name] = str(val)
            elif prop.object_byte_size == 4:  # Float or Int
                val = struct.unpack('<I', f.read(4))[0]
                if should_read_uint32_as_integer(prop.property_name):
                    class_json[prop.property_name] = str(val)
                else:
                    fval = ieee_float(val)
                    s = f"{fval:.6f}"
                    if s == "0.000000" or s == "-0.000000":
                        class_json[prop.property_name] = str(val)
                    else:
                        class_json[prop.property_name] = s
            elif prop.object_byte_size == 8:  # Double
                val = struct.unpack('<d', f.read(8))[0]
                class_json[prop.property_name] = str(val)
            elif prop.object_byte_size == 10:  # Long Double (80-bit, 按 10 字节处理)
                # Python struct 不支持 long double，按 10 字节原始读取
                raw = f.read(10)
                # 简单处理：存为十六进制字符串
                class_json[prop.property_name] = raw.hex()
            elif prop.object_byte_size == 16:  # Vector4
                x, y, z, w = struct.unpack('<4f', f.read(16))
                class_json[prop.property_name] = [str(x), str(y), str(z), str(w)]
            elif prop.object_byte_size == 64:  # Transform
                vals = struct.unpack('<16f', f.read(64))
                class_json[prop.property_name] = [
                    [str(vals[0]), str(vals[1]), str(vals[2]), str(vals[3])],
                    [str(vals[4]), str(vals[5]), str(vals[6]), str(vals[7])],
                    [str(vals[8]), str(vals[9]), str(vals[10]), str(vals[11])],
                    [str(vals[12]), str(vals[13]), str(vals[14]), str(vals[15])],
                ]
            else:
                class_json[f"[Unknown Property]{prop.property_name}"] = f"[Size (Bytes)]{prop.object_byte_size}"
                f.seek(prop.object_byte_size, 1)
        elif prop.property_type == 1:  # TGCString
            s = read_null_terminated_string(f)
            class_json[prop.property_name] = s
        elif prop.property_type == 2:  # Clump
            val = struct.unpack('<I', f.read(4))[0]
            fval = ieee_float(val)
            s = f"{fval:.6f}"
            if s != "-nan":
                class_json[f"[CLUMP]{prop.property_name}"] = str(val)
            else:
                class_json[f"[CLUMP]{prop.property_name}"] = s
        elif prop.property_type == 3:  # Array
            custom_size = struct.unpack('<I', f.read(4))[0]
            if prop.array_index != 0xFFFFFFFF:
                arr = []
                for _ in range(custom_size):
                    elem = {}
                    read_class(f, elem, prop.array_index, all_properties, classes_ref, False)
                    arr.append(elem)
                class_json[prop.property_name] = arr
            else:
                class_json[prop.property_name] = {"Num": str(custom_size), "[CLUMP]data": []}
                for _ in range(custom_size):
                    val = struct.unpack('<I', f.read(4))[0]
                    fval = ieee_float(val)
                    s = f"{fval:.6f}"
                    if s != "-nan":
                        class_json[prop.property_name]["[CLUMP]data"].append(str(val))
                    else:
                        class_json[prop.property_name]["[CLUMP]data"].append(s)
        else:
            class_json[f"[UNKNOWN]{prop.property_name}"] = f"[Size (Bytes)] {prop.object_byte_size}"
            skip = prop.object_byte_size if prop.object_byte_size > 0 else 4
            f.seek(skip, 1)


def resolve_clump_index_string_to_node_name(value, bst_node_names):
    """将 clump 索引解析为节点名称"""
    if value in ("-nan", "nan"):
        return "-nan"
    try:
        index = int(value)
        if 0 <= index < len(bst_node_names):
            return bst_node_names[index]
    except (ValueError, OverflowError):
        pass
    return value


def resolve_clump_refs_in_class_data(class_data, all_classes_meta, class_meta,
                                     classes, bst_node_names):
    """递归解析 clump 引用"""
    if not isinstance(class_meta, dict):
        return
    for property_name, meta in class_meta.items():
        property_type = meta.get("propertyType", 0)
        array_index = meta.get("arrayIndex", 0)

        if property_type == 2:
            clump_key = f"[CLUMP]{property_name}"
            if clump_key in class_data and isinstance(class_data[clump_key], str):
                class_data[clump_key] = resolve_clump_index_string_to_node_name(
                    class_data[clump_key], bst_node_names)
        elif property_type == 3:
            if property_name not in class_data:
                continue
            if array_index == 0xFFFFFFFF:
                if isinstance(class_data[property_name], dict):
                    clump_data = class_data[property_name].get("[CLUMP]data", [])
                    if isinstance(clump_data, list):
                        for i, v in enumerate(clump_data):
                            if isinstance(v, str):
                                clump_data[i] = resolve_clump_index_string_to_node_name(v, bst_node_names)
            elif isinstance(class_data[property_name], list) and array_index < len(classes):
                nested_class_name = classes[array_index].class_name
                nested_meta = all_classes_meta.get(nested_class_name)
                if not isinstance(nested_meta, dict):
                    continue
                for elem in class_data[property_name]:
                    if isinstance(elem, dict):
                        resolve_clump_refs_in_class_data(elem, all_classes_meta, nested_meta,
                                                         classes, bst_node_names)


def get_original_bin_candidates(json_file_path):
    """获取可能的原始 bin 文件路径"""
    base = json_file_path
    if base.lower().endswith('.json'):
        base = base[:-5]
    candidates = [base]
    for suffix in ('.parsed', '.parser'):
        if base.lower().endswith(suffix):
            candidates.append(base[:-len(suffix)])
    return candidates


def load_original_string_pool_order(json_file_path):
    """加载原始 bin 的字符串池顺序"""
    for candidate in get_original_bin_candidates(json_file_path):
        if not os.path.isfile(candidate):
            continue
        try:
            with open(candidate, 'rb') as f:
                header = BSTHeader.from_file(f)
                if header.magic != b'TGCL':
                    continue
                f.seek(header.property_name_offset)
                names = []
                while f.tell() < header.bst_node_offset:
                    s = read_null_terminated_string(f)
                    if s:
                        names.append(s)
                return names
        except Exception:
            continue
    return []


def capture_original_top_level_uint32_values_in_class(f, node_name, class_index,
                                                      all_properties, classes_ref,
                                                      out_values, capture_top_level):
    """捕获原始顶层 uint32 值"""
    if class_index >= len(all_properties):
        return
    for prop in all_properties[class_index]:
        if prop.property_type == 0 and prop.object_byte_size == 4:
            val = struct.unpack('<I', f.read(4))[0]
            if capture_top_level:
                key = f"{node_name}\x1F{classes_ref[class_index].class_name}\x1F{prop.property_name}"
                out_values[key] = val
        elif prop.property_type == 0:
            f.seek(max(prop.object_byte_size, 4) if prop.object_byte_size > 0 else 4, 1)
        elif prop.property_type == 1:
            read_null_terminated_string(f)
        elif prop.property_type == 2:
            f.seek(4, 1)
        elif prop.property_type == 3:
            custom_size = struct.unpack('<I', f.read(4))[0]
            if prop.array_index != 0xFFFFFFFF:
                for _ in range(custom_size):
                    capture_original_top_level_uint32_values_in_class(
                        f, node_name, prop.array_index, all_properties,
                        classes_ref, out_values, False)
            else:
                f.seek(custom_size * 4, 1)
        else:
            f.seek(max(prop.object_byte_size, 4) if prop.object_byte_size > 0 else 4, 1)


def load_original_top_level_uint32_values(json_file_path):
    """加载原始顶层 uint32 值映射"""
    out_values = {}
    for candidate in get_original_bin_candidates(json_file_path):
        if not os.path.isfile(candidate):
            continue
        try:
            with open(candidate, 'rb') as f:
                header = BSTHeader.from_file(f)
                if header.magic != b'TGCL':
                    continue
                classes = [ClassDef.from_file(f) for _ in range(header.class_length)]
                all_properties = []
                for cls in classes:
                    props = []
                    f.seek(header.property_offset + cls.class_property_starting_index * 16)
                    for _ in range(cls.class_property_count):
                        props.append(PropertyDef.from_file(f))
                    for p in props:
                        f.seek(header.property_name_offset + p.property_name_offset)
                        p.property_name = read_null_terminated_string(f)
                    all_properties.append(props)
                    f.seek(header.property_name_offset + cls.class_property_name_offset)
                    cls.class_name = read_null_terminated_string(f)

                f.seek(header.bst_node_offset)
                for _ in range(header.bst_node_count):
                    index = struct.unpack('<I', f.read(4))[0]
                    bst_name = read_null_terminated_string(f)
                    capture_original_top_level_uint32_values_in_class(
                        f, bst_name, index, all_properties, classes, out_values, True)
                return out_values
        except Exception:
            continue
    return out_values


def load_objects(file_name):
    """将 .bin 文件加载为 JSON"""
    if not os.path.isfile(file_name):
        print(f"文件不存在: {file_name}")
        return 0

    with open(file_name, 'rb') as f:
        header = BSTHeader.from_file(f)
        classes = []
        f.seek(header.class_offset)

        json_data = {
            "version": header.version,
            "MemorySize": str(header.object_ptr_count),
            "classes": {},
            "BSTNodes": {},
        }

        for _ in range(header.class_length):
            classes.append(ClassDef.from_file(f))

        all_properties = []
        for cls in classes:
            props = []
            f.seek(header.property_offset + cls.class_property_starting_index * 16)
            for _ in range(cls.class_property_count):
                props.append(PropertyDef.from_file(f))
            for p in props:
                f.seek(header.property_name_offset + p.property_name_offset)
                p.property_name = read_null_terminated_string(f)
            all_properties.append(props)
            f.seek(header.property_name_offset + cls.class_property_name_offset)
            cls.class_name = read_null_terminated_string(f)

            if not props:
                json_data["classes"][cls.class_name] = None
            for p in props:
                json_data["classes"].setdefault(cls.class_name, {})[p.property_name] = {
                    "propertyType": p.property_type,
                    "objectByteSize": p.object_byte_size,
                    "arrayIndex": p.array_index,
                }

        f.seek(header.bst_node_offset)
        for _ in range(header.bst_node_count):
            index = struct.unpack('<I', f.read(4))[0]
            bst_name = read_null_terminated_string(f)
            read_class(f, json_data["BSTNodes"].setdefault(bst_name, {}), index, all_properties, classes)

        bst_node_names = list(json_data["BSTNodes"].keys())
        for bst_name, bst_data in json_data["BSTNodes"].items():
            if not isinstance(bst_data, dict):
                continue
            for class_name, class_data in bst_data.items():
                class_meta = json_data["classes"].get(class_name)
                if not isinstance(class_meta, dict) or not isinstance(class_data, dict):
                    continue
                resolve_clump_refs_in_class_data(class_data, json_data["classes"], class_meta,
                                                 classes, bst_node_names)

    out_path = file_name + ".json"
    with open(out_path, 'w', encoding='utf-8') as out:
        json.dump(json_data, out, indent=1, ensure_ascii=False)
    print(f"已保存 JSON: {out_path}")
    return 1


# ===================== Save (JSON -> bin) =====================

def json_value_to_string(value):
    """将 JSON 值转为字符串"""
    if isinstance(value, str):
        return value
    if isinstance(value, bool):
        return "1" if value else "0"
    if isinstance(value, (int, float)):
        if isinstance(value, bool):
            return "1" if value else "0"
        return str(value)
    if value is None:
        return ""
    return json.dumps(value, ensure_ascii=False)


def normalize_numeric_text(value):
    """标准化数字文本"""
    if isinstance(value, str) and value.startswith("[Unknown]"):
        return value[len("[Unknown]"):]
    return str(value)


def is_unknown_hex_value(value):
    """判断是否为未知十六进制值"""
    return isinstance(value, str) and value.startswith("[Unknown]")


def is_nan_clump_string(value):
    """判断 clump 字符串是否为 nan/null"""
    if not isinstance(value, str):
        return False
    v = value.replace(" ", "").replace("\t", "").lower()
    return v in ("", "-nan", "nan", "null")


def is_nan_numeric_text(value):
    """判断数字文本是否为 nan"""
    if not isinstance(value, str):
        return False
    v = value.replace(" ", "").replace("\t", "").lower()
    return v in ("-nan", "nan")


def null_clump_sentinel():
    return 0xFFFFFFFF


def nan_float_sentinel_bits():
    return 0xFFFFFFF6


def resolve_property_json(class_data, property_name, property_type):
    """从 class_data 中解析属性值"""
    if not isinstance(class_data, dict):
        return None
    if property_name in class_data:
        return class_data[property_name]
    if property_type == 2:
        clump_key = f"[CLUMP]{property_name}"
        if clump_key in class_data:
            return class_data[clump_key]
    return None


def resolve_clump_reference(value, node_index_by_name):
    """解析 clump 引用为索引"""
    if value is None:
        return null_clump_sentinel()
    text = json_value_to_string(value)
    if is_nan_clump_string(text):
        return null_clump_sentinel()
    if text in node_index_by_name:
        return node_index_by_name[text]
    try:
        return int(text) & 0xFFFFFFFF
    except (ValueError, TypeError):
        return null_clump_sentinel()


def resolve_nested_array_element(element, nested_class_name):
    """解析嵌套数组元素"""
    if isinstance(element, dict) and nested_class_name in element:
        return element[nested_class_name]
    return element


def count_object_pointers_in_class_data(json_data, classes, class_name, class_data):
    """计算类数据中的对象指针数量"""
    class_meta = json_data.get("classes", {}).get(class_name)
    if not isinstance(class_meta, dict):
        return 0
    count = 0
    for property_name, meta in class_meta.items():
        property_type = meta.get("propertyType", 0)
        array_index = meta.get("arrayIndex", 0)
        value = resolve_property_json(class_data, property_name, property_type)
        if property_type == 2:
            count += 1
        elif property_type == 3:
            if array_index == 0xFFFFFFFF:
                if isinstance(value, dict):
                    data = value.get("[CLUMP]data", [])
                    if isinstance(data, list):
                        count += len(data)
                elif isinstance(value, list):
                    count += len(value)
            elif isinstance(value, list) and array_index < len(classes):
                nested_class_name = classes[array_index].class_name
                for elem in value:
                    count += count_object_pointers_in_class_data(
                        json_data, classes, nested_class_name,
                        resolve_nested_array_element(elem, nested_class_name))
    return count


def compute_class_serialized_size(json_data, classes, class_name, class_data):
    """计算类序列化后的大小"""
    class_meta = json_data.get("classes", {}).get(class_name)
    if not isinstance(class_meta, dict):
        return 0
    size = 0
    for property_name, meta in class_meta.items():
        property_type = meta.get("propertyType", 0)
        object_byte_size = meta.get("objectByteSize", 0)
        array_index = meta.get("arrayIndex", 0)
        value = resolve_property_json(class_data, property_name, property_type)

        if property_type == 0:
            size += object_byte_size
        elif property_type == 1:
            size += len(json_value_to_string(value)) + 1
        elif property_type == 2:
            size += 4
        elif property_type == 3:
            size += 4
            if array_index == 0xFFFFFFFF:
                if isinstance(value, dict):
                    data = value.get("[CLUMP]data", [])
                    if isinstance(data, list):
                        size += len(data) * 4
                elif isinstance(value, list):
                    size += len(value) * 4
            elif isinstance(value, list) and array_index < len(classes):
                nested_class_name = classes[array_index].class_name
                for elem in value:
                    size += compute_class_serialized_size(
                        json_data, classes, nested_class_name,
                        resolve_nested_array_element(elem, nested_class_name))
        else:
            size += max(object_byte_size, 4) if object_byte_size > 0 else 4
    return size


def make_original_value_key(node_name, class_name, property_name):
    return f"{node_name}\x1F{class_name}\x1F{property_name}"


def write_class(object_level_bin, json_data, classes, node_index_by_name,
                original_top_level_uint32_values, node_name, class_name, class_data):
    """递归写入类数据到 bin"""
    class_meta = json_data.get("classes", {}).get(class_name)
    if not isinstance(class_meta, dict):
        return

    for property_name, meta in class_meta.items():
        property_type = meta.get("propertyType", 0)
        object_byte_size = meta.get("objectByteSize", 0)
        array_index = meta.get("arrayIndex", 0)
        value_json = resolve_property_json(class_data, property_name, property_type)
        raw_value_text = json_value_to_string(value_json)
        value_is_unknown_hex = is_unknown_hex_value(raw_value_text)
        value_text = normalize_numeric_text(raw_value_text)

        if property_type == 0:
            try:
                if object_byte_size == 1:
                    val = int(value_text, 16) if value_is_unknown_hex else (int(value_text) if value_text else 0)
                    object_level_bin.write(struct.pack('<B', val & 0xFF))
                elif object_byte_size == 2:
                    val = int(value_text, 16) if value_is_unknown_hex else (int(value_text) if value_text else 0)
                    object_level_bin.write(struct.pack('<H', val & 0xFFFF))
                elif object_byte_size == 4:
                    if is_nan_numeric_text(value_text):
                        key = make_original_value_key(node_name, class_name, property_name)
                        val = original_top_level_uint32_values.get(key, nan_float_sentinel_bits())
                        object_level_bin.write(struct.pack('<I', val))
                    elif not value_is_unknown_hex and ('.' in value_text or 'e' in value_text.lower()):
                        val = float(value_text) if value_text else 0.0
                        object_level_bin.write(struct.pack('<f', val))
                    else:
                        base = 16 if value_is_unknown_hex else 10
                        val = int(value_text, base) if value_text else 0
                        object_level_bin.write(struct.pack('<I', val & 0xFFFFFFFF))
                elif object_byte_size == 8:
                    if is_nan_numeric_text(value_text):
                        val = float('nan')
                    else:
                        val = float(value_text) if value_text else 0.0
                    object_level_bin.write(struct.pack('<d', val))
                elif object_byte_size == 10:
                    # 80-bit extended float: 尝试解析 hex 或 float，否则写 0
                    if is_nan_numeric_text(value_text):
                        # 写 10 字节 NaN 模式
                        object_level_bin.write(bytes([0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]))
                    elif value_text:
                        try:
                            if all(c in '0123456789abcdefABCDEF' for c in value_text) and len(value_text) == 20:
                                object_level_bin.write(bytes.fromhex(value_text))
                            else:
                                # 无法精确编码 long double，写 0
                                object_level_bin.write(bytes(10))
                        except Exception:
                            object_level_bin.write(bytes(10))
                    else:
                        object_level_bin.write(bytes(10))
                elif object_byte_size == 16:
                    if value_text:
                        try:
                            data = json.loads(value_text)
                            vals = [float(v) for v in data]
                            while len(vals) < 4:
                                vals.append(0.0)
                            object_level_bin.write(struct.pack('<4f', *vals[:4]))
                        except Exception:
                            object_level_bin.write(bytes(16))
                    else:
                        object_level_bin.write(bytes(16))
                elif object_byte_size == 64:
                    if value_text:
                        try:
                            data = json.loads(value_text)
                            flat = []
                            for row in data:
                                flat.extend([float(v) for v in row])
                            while len(flat) < 16:
                                flat.append(0.0)
                            object_level_bin.write(struct.pack('<16f', *flat[:16]))
                        except Exception:
                            object_level_bin.write(bytes(64))
                    else:
                        object_level_bin.write(bytes(64))
                else:
                    object_level_bin.write(bytes(max(object_byte_size, 4) if object_byte_size > 0 else 4))
            except Exception:
                object_level_bin.write(bytes(max(object_byte_size, 4) if object_byte_size > 0 else 4))
        elif property_type == 1:
            text = value_text if value_text else ""
            object_level_bin.write(text.encode('utf-8'))
            object_level_bin.write(b'\x00')
        elif property_type == 2:
            val = resolve_clump_reference(value_json, node_index_by_name)
            object_level_bin.write(struct.pack('<I', val))
        elif property_type == 3:
            if array_index == 0xFFFFFFFF:
                clump_values = []
                if isinstance(value_json, dict):
                    data = value_json.get("[CLUMP]data", [])
                    if isinstance(data, list):
                        clump_values = data
                elif isinstance(value_json, list):
                    clump_values = value_json
                object_level_bin.write(struct.pack('<I', len(clump_values)))
                for cv in clump_values:
                    val = resolve_clump_reference(cv, node_index_by_name)
                    object_level_bin.write(struct.pack('<I', val))
            else:
                arr = value_json if isinstance(value_json, list) else []
                object_level_bin.write(struct.pack('<I', len(arr)))
                if arr and array_index < len(classes):
                    nested_class_name = classes[array_index].class_name
                    for elem in arr:
                        write_class(object_level_bin, json_data, classes, node_index_by_name,
                                    original_top_level_uint32_values, node_name, nested_class_name,
                                    resolve_nested_array_element(elem, nested_class_name))
        else:
            object_level_bin.write(bytes(max(object_byte_size, 4) if object_byte_size > 0 else 4))


def save_objects(file_name):
    """将 JSON 保存为 .bin 文件"""
    if not os.path.isfile(file_name):
        print(f"文件不存在: {file_name}")
        return 0

    directory = os.path.dirname(file_name) or "."
    base_name = os.path.splitext(os.path.basename(file_name))[0]

    with open(file_name, 'r', encoding='utf-8') as f:
        json_data = json.load(f)

    out_path = os.path.join(directory, base_name + "_new.level.bin")
    with open(out_path, 'wb') as object_level_bin:
        classes = []
        properties = []
        class_index_by_name = {}
        node_index_by_name = {}
        pooled_name_offsets = {}
        original_top_level_uint32_values = {}
        pooled_names = []

        version = json_data.get("version", 1)
        property_count = 0
        class_offset = 44
        offset = 44

        class_starting_index = 0
        for class_name, class_meta in json_data.get("classes", {}).items():
            class_data = ClassDef()
            class_data.class_name = class_name
            class_data.class_property_count = len(class_meta) if isinstance(class_meta, dict) else 0
            class_data.class_property_starting_index = class_starting_index
            class_data.class_property_name_offset = 0
            class_starting_index += class_data.class_property_count
            class_index_by_name[class_name] = len(classes)
            classes.append(class_data)
            offset += 12

        class_count = len(classes)
        property_offset = offset
        for cls in classes:
            class_meta = json_data["classes"].get(cls.class_name)
            if not isinstance(class_meta, dict):
                continue
            for prop_name, prop_meta in class_meta.items():
                prop = PropertyDef()
                prop.property_type = prop_meta.get("propertyType", 0)
                prop.property_name_offset = 0
                prop.object_byte_size = prop_meta.get("objectByteSize", 0)
                prop.array_index = prop_meta.get("arrayIndex", 0)
                prop.property_name = prop_name
                properties.append(prop)
                offset += 16
                property_count += 1

        pooled_name_offset = 0
        original_string_pool_order = load_original_string_pool_order(file_name)
        original_top_level_uint32_values = load_original_top_level_uint32_values(file_name)

        def register_pooled_name(name):
            nonlocal pooled_name_offset
            if name in pooled_name_offsets:
                return pooled_name_offsets[name]
            assigned = pooled_name_offset
            pooled_name_offsets[name] = assigned
            pooled_names.append(name)
            pooled_name_offset += len(name) + 1
            return assigned

        for pooled_name in original_string_pool_order:
            is_referenced = any(p.property_name == pooled_name for p in properties)
            is_referenced = is_referenced or any(c.class_name == pooled_name for c in classes)
            if is_referenced:
                register_pooled_name(pooled_name)

        for p in properties:
            p.property_name_offset = register_pooled_name(p.property_name)
        for c in classes:
            c.class_property_name_offset = register_pooled_name(c.class_name)

        property_names_offset = offset
        offset += pooled_name_offset
        bst_nodes_offset = offset

        object_ptr_count = 0
        bst_nodes = json_data.get("BSTNodes", {})
        bst_node_count = len(bst_nodes)
        bst_index = 0
        for node_name, node_data in bst_nodes.items():
            node_index_by_name[node_name] = bst_index
            bst_index += 1
            if not isinstance(node_data, dict) or not node_data:
                continue
            # 取第一个类（原版也只支持第一个）
            class_it = next(iter(node_data.items()))
            class_name, class_data_val = class_it
            if class_name not in class_index_by_name:
                print(f"未知类在 BSTNode {node_name}: {class_name}")
                return 0
            if len(node_data) > 1:
                print(f"警告: 多类 BSTNode 尚未完全支持，仅使用第一个类: {node_name}")
            offset += 4 + len(node_name) + 1
            offset += compute_class_serialized_size(json_data, classes, class_name, class_data_val)
            object_ptr_count += count_object_pointers_in_class_data(json_data, classes, class_name, class_data_val)

        file_size = offset

        # Header
        header = BSTHeader()
        header.magic = b'TGCL'
        header.version = version
        header.class_length = class_count
        header.property_count = property_count
        header.bst_node_count = bst_node_count
        header.object_ptr_count = object_ptr_count
        header.class_offset = class_offset
        header.property_offset = property_offset
        header.property_name_offset = property_names_offset
        header.bst_node_offset = bst_nodes_offset
        header.file_size = file_size
        header.write(object_level_bin)

        for c in classes:
            c.write(object_level_bin)
        for p in properties:
            p.write(object_level_bin)
        for name in pooled_names:
            object_level_bin.write(name.encode('utf-8'))
            object_level_bin.write(b'\x00')

        for node_name, node_data in bst_nodes.items():
            if not isinstance(node_data, dict) or not node_data:
                continue
            class_it = next(iter(node_data.items()))
            class_name, class_data_val = class_it
            if class_name not in class_index_by_name:
                print(f"未知类在 BSTNode {node_name}: {class_name}")
                return 0
            class_index = class_index_by_name[class_name]
            object_level_bin.write(struct.pack('<I', class_index))
            object_level_bin.write(node_name.encode('utf-8'))
            object_level_bin.write(b'\x00')
            write_class(object_level_bin, json_data, classes, node_index_by_name,
                        original_top_level_uint32_values, node_name, class_name, class_data_val)

    print(f"已保存 bin: {out_path}")
    return 1
