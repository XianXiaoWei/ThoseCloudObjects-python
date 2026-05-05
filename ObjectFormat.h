#pragma once
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include <iomanip>
#include <filesystem>
#include <sstream>
#include <limits>
#include "json.hpp"
#include <map>
#include <stdio.h>

using ordered_json = nlohmann::ordered_json;

struct class_t {
    uint32_t classPropertyNameOffset;
    uint32_t classPropertyStartingIndex;
    uint32_t classPropertyCount;
    std::string className;

    class_t()
    {
        classPropertyNameOffset = 0;
        classPropertyStartingIndex = 0;
        classPropertyCount = 0;
        className = "";
    }
    class_t(std::ifstream& file)
    {
        file.read((char*)&classPropertyNameOffset, sizeof(classPropertyNameOffset));
        file.read((char*)&classPropertyStartingIndex, sizeof(classPropertyStartingIndex));
        file.read((char*)&classPropertyCount, sizeof(classPropertyCount));
    }
};

struct property_t {
    uint32_t propertyType;
    uint32_t propertyNameOffset;
    uint32_t objectByteSize;
    uint32_t arrayIndex;
    std::string propertyName;

    property_t()
    {
        propertyType = 0;
        propertyNameOffset = 0;
        objectByteSize = 0;
        arrayIndex = 0;
        propertyName = "";
    }
    property_t(std::ifstream& file)
    {
        file.read((char*)&propertyType, sizeof(propertyType));
        file.read((char*)&propertyNameOffset, sizeof(propertyNameOffset));
        file.read((char*)&objectByteSize, sizeof(objectByteSize));
        file.read((char*)&arrayIndex, sizeof(arrayIndex));
    }
};

struct vector4_t{
    float x;
    float y;
    float z;
    float w;

    vector4_t()
    {
        x = 0;
        y = 0;
        z = 0;
        w = 0;
    }
    vector4_t(std::string data)
    {
        ordered_json tempData = ordered_json::parse(data);
        x = std::stof(tempData[0].get<std::string>());
        y = std::stof(tempData[1].get<std::string>());
        z = std::stof(tempData[2].get<std::string>());
        w = std::stof(tempData[3].get<std::string>());
    }
};

struct transform_t {
    float x;
    float y;
    float z;
    float w;

    float x1;
    float y1;
    float z1;
    float w1;

    float x2;
    float y2;
    float z2;
    float w2;

    float x3;
    float y3;
    float z3;
    float w3;

    transform_t()
    {
        x = 0;
        y = 0;
        z = 0;
        w = 0;

        x1 = 0;
        y1 = 0;
        z1 = 0;
        w1 = 0;

        x2 = 0;
        y2 = 0;
        z2 = 0;
        w2 = 0;

        x3 = 0;
        y3 = 0;
        z3 = 0;
        w3 = 0;
    }
    transform_t(std::string data)
    {
        ordered_json tempData = ordered_json::parse(data);
        x = std::stof(tempData[0][0].get<std::string>());
        y = std::stof(tempData[0][1].get<std::string>());
        z = std::stof(tempData[0][2].get<std::string>());
        w = std::stof(tempData[0][3].get<std::string>());

        x1 = std::stof(tempData[1][0].get<std::string>());
        y1 = std::stof(tempData[1][1].get<std::string>());
        z1 = std::stof(tempData[1][2].get<std::string>());
        w1 = std::stof(tempData[1][3].get<std::string>());

        x2 = std::stof(tempData[2][0].get<std::string>());
        y2 = std::stof(tempData[2][1].get<std::string>());
        z2 = std::stof(tempData[2][2].get<std::string>());
        w2 = std::stof(tempData[2][3].get<std::string>());

        x3 = std::stof(tempData[3][0].get<std::string>());
        y3 = std::stof(tempData[3][1].get<std::string>());
        z3 = std::stof(tempData[3][2].get<std::string>());
        w3 = std::stof(tempData[3][3].get<std::string>());
    }
};

struct BSTHeader_t {
    char magic[4];
    uint32_t version;
    uint32_t classLength;
    uint32_t propertyCount;
    uint32_t BSTNodeCount;
    uint32_t objectPtrCount;
    uint32_t classOffset;
    uint32_t propertyOffset;
    uint32_t PropertyNameOffset;
    uint32_t BSTNodeOffset;
    uint32_t FileSize;

    BSTHeader_t(std::ifstream& file)
    {
        file.read((char*)&magic, sizeof(magic));
        file.read((char*)&version, sizeof(version));
        file.read((char*)&classLength, sizeof(classLength));
        file.read((char*)&propertyCount, sizeof(propertyCount));
        file.read((char*)&BSTNodeCount, sizeof(BSTNodeCount));
        file.read((char*)&objectPtrCount, sizeof(objectPtrCount));
        file.read((char*)&classOffset, sizeof(classOffset));
        file.read((char*)&propertyOffset, sizeof(propertyOffset));
        file.read((char*)&PropertyNameOffset, sizeof(PropertyNameOffset));
        file.read((char*)&BSTNodeOffset, sizeof(BSTNodeOffset));
        file.read((char*)&FileSize, sizeof(FileSize));
    }
};

float ieee_float(uint32_t f)
{
    static_assert(sizeof(float) == sizeof f, "`float` has a weird size.");
    float ret;
    std::memcpy(&ret, &f, sizeof(float));
    return ret;
}

static bool ShouldReadUint32PropertyAsInteger(const std::string& propertyName)
{
    if (propertyName == "bstGuid")
    {
        return true;
    }

    return propertyName.find("BstGuid") != std::string::npos;
}

static void ReadClass(std::ifstream& file, ordered_json& jsonData, uint32_t index, std::vector<std::vector<property_t>> AllProperties, std::vector<class_t> classesRef, bool wrapClassName = true)
{
    uint32_t customSize;
    ordered_json& classJson = wrapClassName ? jsonData[classesRef[index].className] : jsonData;
    for (unsigned int x = 0; x < AllProperties[index].size(); x++)
    {
        std::string TGCString;
        uint8_t TempBool;
        uint16_t TempUint16;
        uint32_t TempUint32;
        double TempDouble;
        long double TempLongDouble;
        vector4_t vector4Temp;
        transform_t transformTemp;

        switch (AllProperties[index][x].propertyType)
        {
            case 0: //General Value (float, bool, etc.)
                switch (AllProperties[index][x].objectByteSize)
                {
                    case 1: //Bool
                        file.read((char*)&TempBool, sizeof(TempBool));
                        if (TempBool <= 1)
                        {
                            classJson[AllProperties[index][x].propertyName] = TempBool ? true : false;
                        }
                        else
                        {
                            std::ostringstream unknownValue;
                            unknownValue << "[Unknown]" << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(TempBool);
                            classJson[AllProperties[index][x].propertyName] = unknownValue.str();
                        }
                    break;
                    case 2: //Unsigned short
                        file.read((char*)&TempUint16, sizeof(TempUint16));
                        classJson[AllProperties[index][x].propertyName] = std::to_string(TempUint16);
                    break;
                    case 4: //Float or Int
                        file.read((char*)&TempUint32, sizeof(TempUint32));
                        if (ShouldReadUint32PropertyAsInteger(AllProperties[index][x].propertyName))
                        {
                            classJson[AllProperties[index][x].propertyName] = std::to_string(TempUint32);
                        }
                        else
                        {
                            if (std::to_string(ieee_float(TempUint32)) == "0.000000" || std::to_string(ieee_float(TempUint32)) == "-0.000000")
                            {
                                classJson[AllProperties[index][x].propertyName] = std::to_string(TempUint32);
                            }
                            else
                            {
                                classJson[AllProperties[index][x].propertyName] = std::to_string(ieee_float(TempUint32));
                            }
                        }
                    break;
                    case 8: //Double
                        file.read((char*)&TempDouble, sizeof(TempDouble));
                        classJson[AllProperties[index][x].propertyName] = std::to_string(TempDouble);
                    break;
                    case 10: //Long Double
                        file.read((char*)&TempLongDouble, sizeof(TempLongDouble));
                        classJson[AllProperties[index][x].propertyName] = std::to_string(TempLongDouble);
                    break;
                    case 16: //Vector 4
                        file.read((char*)&vector4Temp, sizeof(vector4Temp));
                        classJson[AllProperties[index][x].propertyName] = ordered_json{ std::to_string(vector4Temp.x), std::to_string(vector4Temp.y), std::to_string(vector4Temp.z), std::to_string(vector4Temp.w) };
                    break;
                    case 64: //Transform
                        file.read((char*)&transformTemp, sizeof(transformTemp));
                        classJson[AllProperties[index][x].propertyName] = ordered_json{
                            { std::to_string(transformTemp.x), std::to_string(transformTemp.y), std::to_string(transformTemp.z), std::to_string(transformTemp.w) },
                            { std::to_string(transformTemp.x1), std::to_string(transformTemp.y1), std::to_string(transformTemp.z1), std::to_string(transformTemp.w1) },
                            { std::to_string(transformTemp.x2), std::to_string(transformTemp.y2), std::to_string(transformTemp.z2), std::to_string(transformTemp.w2) },
                            { std::to_string(transformTemp.x3), std::to_string(transformTemp.y3), std::to_string(transformTemp.z3), std::to_string(transformTemp.w3) }
                        };
                    break;
                    default:
                        classJson["[Unknown Property]" + AllProperties[index][x].propertyName] = "[Size (Bytes)]" + std::to_string(AllProperties[index][x].objectByteSize);
                        file.ignore(AllProperties[index][x].objectByteSize);
                    break;
                }
            break;
            case 1: //TGCString
                getline(file, TGCString, '\0');
                classJson[AllProperties[index][x].propertyName] = TGCString;
            break;
            case 2: //Clump
                file.read((char*)&TempUint32, sizeof(TempUint32));
                if (std::to_string(ieee_float(TempUint32)) != "-nan")
                {
                    classJson["[CLUMP]" + AllProperties[index][x].propertyName] = std::to_string(TempUint32);
                    //std::cout << "Go to offset - " << std::to_string(TempUint32) << std::endl;
                }  
                else
                {
                    classJson["[CLUMP]" + AllProperties[index][x].propertyName] = std::to_string(ieee_float(TempUint32));
                }
            break;
            case 3: //Array
                file.read((char*)&customSize, sizeof(customSize));
                if ((int)AllProperties[index][x].arrayIndex != -1)
                {
                    classJson[AllProperties[index][x].propertyName] = ordered_json::array();
                    for (unsigned int y = 0; y < customSize; y++)
                    {
                        ReadClass(file, classJson[AllProperties[index][x].propertyName][y], AllProperties[index][x].arrayIndex, AllProperties, classesRef, false);
                    }
                }
                else
                {
                    classJson[AllProperties[index][x].propertyName]["Num"] = std::to_string(customSize);
                    classJson[AllProperties[index][x].propertyName]["[CLUMP]data"] = ordered_json::array();
                    for (unsigned int y = 0; y < customSize; y++)
                    {
                        file.read((char*)&TempUint32, sizeof(TempUint32));
                        if (std::to_string(ieee_float(TempUint32)) != "-nan")
                        {
                            classJson[AllProperties[index][x].propertyName]["[CLUMP]data"].push_back(std::to_string(TempUint32));
                        }
                        else
                        {
                            classJson[AllProperties[index][x].propertyName]["[CLUMP]data"].push_back(std::to_string(ieee_float(TempUint32)));
                        }
                    }
                }
            break;
            default: //Unknown
                classJson["[UNKNOWN]" + AllProperties[index][x].propertyName] = "[Size (Bytes)] " + std::to_string(AllProperties[index][x].objectByteSize);
                if (AllProperties[index][x].objectByteSize > 0)
                {
                    file.ignore(AllProperties[index][x].objectByteSize);
                    //logFile << "Bytes skipped: " << AllProperties[index][x].objectByteSize << std::endl;
                }
                else
                {
                    file.ignore(4);
                    //logFile << "Bytes skipped: " << 4 << std::endl;
                }
            break;
        }
    }
}

static std::string ResolveClumpIndexStringToNodeName(const std::string& value, const std::vector<std::string>& bstNodeNames)
{
    if (value == "-nan" || value == "nan")
    {
        return "-nan";
    }

    try
    {
        uint32_t index = static_cast<uint32_t>(std::stoul(value));
        if (index < bstNodeNames.size())
        {
            return bstNodeNames[index];
        }
    }
    catch (...)
    {
    }

    return value;
}

static void ResolveClumpRefsInClassData(
    ordered_json& classData,
    const ordered_json& allClassesMeta,
    const ordered_json& classMeta,
    const std::vector<class_t>& classes,
    const std::vector<std::string>& bstNodeNames)
{
    for (ordered_json::const_iterator propertyIt = classMeta.begin(); propertyIt != classMeta.end(); ++propertyIt)
    {
        const std::string propertyName = propertyIt.key();
        const uint32_t propertyType = (*propertyIt)["propertyType"];
        const uint32_t arrayIndex = (*propertyIt)["arrayIndex"];

        if (propertyType == 2)
        {
            const std::string clumpPropertyName = "[CLUMP]" + propertyName;
            auto clumpIt = classData.find(clumpPropertyName);
            if (clumpIt != classData.end() && clumpIt->is_string())
            {
                *clumpIt = ResolveClumpIndexStringToNodeName(clumpIt->get<std::string>(), bstNodeNames);
            }
        }
        else if (propertyType == 3)
        {
            auto arrayIt = classData.find(propertyName);
            if (arrayIt == classData.end())
            {
                continue;
            }

            if (arrayIndex == 0xFFFFFFFFu)
            {
                if (arrayIt->is_object())
                {
                    auto clumpDataIt = arrayIt->find("[CLUMP]data");
                    if (clumpDataIt != arrayIt->end() && clumpDataIt->is_array())
                    {
                        for (ordered_json::iterator clumpIt = clumpDataIt->begin(); clumpIt != clumpDataIt->end(); ++clumpIt)
                        {
                            if (clumpIt->is_string())
                            {
                                *clumpIt = ResolveClumpIndexStringToNodeName(clumpIt->get<std::string>(), bstNodeNames);
                            }
                        }
                    }
                }
            }
            else if (arrayIt->is_array() && arrayIndex < classes.size())
            {
                const std::string nestedClassName = classes[arrayIndex].className;
                auto nestedClassMetaIt = allClassesMeta.find(nestedClassName);
                if (nestedClassMetaIt == allClassesMeta.end() || !nestedClassMetaIt->is_object())
                {
                    continue;
                }

                for (ordered_json::iterator elementIt = arrayIt->begin(); elementIt != arrayIt->end(); ++elementIt)
                {
                    if (elementIt->is_object())
                    {
                        ResolveClumpRefsInClassData(*elementIt, allClassesMeta, *nestedClassMetaIt, classes, bstNodeNames);
                    }
                }
            }
        }
    }
}

static bool EndsWithString(const std::string& value, const std::string& suffix)
{
    return value.size() >= suffix.size() && value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static std::vector<std::filesystem::path> GetOriginalBinCandidates(const std::string& jsonFilePath)
{
    namespace fs = std::filesystem;

    fs::path inputPath(jsonFilePath);
    std::vector<fs::path> candidates;

    fs::path basePath = inputPath;
    if (basePath.extension() == ".json")
    {
        basePath.replace_extension("");
    }

    candidates.push_back(basePath);

    const std::vector<std::string> removableSuffixes = { ".parsed", ".parser" };
    const std::string baseString = basePath.string();
    for (const std::string& suffix : removableSuffixes)
    {
        if (EndsWithString(baseString, suffix))
        {
            candidates.push_back(fs::path(baseString.substr(0, baseString.size() - suffix.size())));
        }
    }

    return candidates;
}

static bool LoadOriginalStringPoolOrder(const std::string& jsonFilePath, std::vector<std::string>& outNames)
{
    namespace fs = std::filesystem;
    std::vector<fs::path> candidates = GetOriginalBinCandidates(jsonFilePath);

    for (const fs::path& candidate : candidates)
    {
        if (!fs::exists(candidate) || !fs::is_regular_file(candidate))
        {
            continue;
        }

        std::ifstream file(candidate, std::ios::binary);
        if (!file)
        {
            continue;
        }

        BSTHeader_t header(file);
        if (std::string(header.magic, header.magic + 4) != "TGCL")
        {
            continue;
        }

        file.seekg(header.PropertyNameOffset);
        while (static_cast<uint32_t>(file.tellg()) < header.BSTNodeOffset && file.good())
        {
            std::string pooledName;
            std::getline(file, pooledName, '\0');
            if (!pooledName.empty())
            {
                outNames.push_back(pooledName);
            }
        }

        return !outNames.empty();
    }

    return false;
}

static std::string MakeOriginalValueKey(const std::string& nodeName, const std::string& className, const std::string& propertyName)
{
    return nodeName + "\x1F" + className + "\x1F" + propertyName;
}

static void CaptureOriginalTopLevelUint32ValuesInClass(
    std::ifstream& file,
    const std::string& nodeName,
    uint32_t classIndex,
    const std::vector<std::vector<property_t>>& allProperties,
    const std::vector<class_t>& classesRef,
    std::map<std::string, uint32_t>& outValues,
    bool captureTopLevel)
{
    if (classIndex >= allProperties.size())
    {
        return;
    }

    uint32_t customSize = 0;
    for (unsigned int x = 0; x < allProperties[classIndex].size(); x++)
    {
        const property_t& property = allProperties[classIndex][x];
        switch (property.propertyType)
        {
            case 0:
                if (property.objectByteSize == 4)
                {
                    uint32_t rawValue = 0;
                    file.read(reinterpret_cast<char*>(&rawValue), sizeof(rawValue));
                    if (captureTopLevel)
                    {
                        outValues[MakeOriginalValueKey(nodeName, classesRef[classIndex].className, property.propertyName)] = rawValue;
                    }
                }
                else
                {
                    file.ignore(property.objectByteSize > 0 ? property.objectByteSize : 4);
                }
            break;
            case 1:
            {
                std::string ignoredString;
                std::getline(file, ignoredString, '\0');
            }
            break;
            case 2:
                file.ignore(4);
            break;
            case 3:
                file.read(reinterpret_cast<char*>(&customSize), sizeof(customSize));
                if ((int)property.arrayIndex != -1)
                {
                    for (unsigned int y = 0; y < customSize; y++)
                    {
                        CaptureOriginalTopLevelUint32ValuesInClass(
                            file,
                            nodeName,
                            property.arrayIndex,
                            allProperties,
                            classesRef,
                            outValues,
                            false);
                    }
                }
                else
                {
                    file.ignore(customSize * 4);
                }
            break;
            default:
                file.ignore(property.objectByteSize > 0 ? property.objectByteSize : 4);
            break;
        }
    }
}

static bool LoadOriginalTopLevelUint32Values(const std::string& jsonFilePath, std::map<std::string, uint32_t>& outValues)
{
    namespace fs = std::filesystem;
    std::vector<fs::path> candidates = GetOriginalBinCandidates(jsonFilePath);

    for (const fs::path& candidate : candidates)
    {
        if (!fs::exists(candidate) || !fs::is_regular_file(candidate))
        {
            continue;
        }

        std::ifstream file(candidate, std::ios::binary);
        if (!file)
        {
            continue;
        }

        BSTHeader_t header(file);
        if (std::string(header.magic, header.magic + 4) != "TGCL")
        {
            continue;
        }

        std::vector<class_t> classes;
        for (unsigned int i = 0; i < header.classLength; i++)
        {
            classes.push_back(class_t(file));
        }

        std::vector<std::vector<property_t>> allProperties;
        for (unsigned int i = 0; i < classes.size(); i++)
        {
            std::vector<property_t> properties;
            file.seekg(header.propertyOffset + (classes[i].classPropertyStartingIndex * 16));
            for (unsigned int x = 0; x < classes[i].classPropertyCount; x++)
            {
                properties.push_back(property_t(file));
            }
            for (unsigned int x = 0; x < properties.size(); x++)
            {
                file.seekg(header.PropertyNameOffset + properties[x].propertyNameOffset);
                getline(file, properties[x].propertyName, '\0');
            }
            allProperties.push_back(properties);
            file.seekg(header.PropertyNameOffset + classes[i].classPropertyNameOffset);
            getline(file, classes[i].className, '\0');
        }

        file.seekg(header.BSTNodeOffset);
        for (unsigned int i = 0; i < header.BSTNodeCount; i++)
        {
            uint32_t index = 0;
            file.read(reinterpret_cast<char*>(&index), sizeof(index));

            std::string bstName;
            std::getline(file, bstName, '\0');

            CaptureOriginalTopLevelUint32ValuesInClass(file, bstName, index, allProperties, classes, outValues, true);
        }

        return !outValues.empty();
    }

    return false;
}

static int LoadObjects(std::string FileName)
{
    if (std::ifstream file{ FileName, std::ios::binary })
    {
        ordered_json jsonData;
        BSTHeader_t header = BSTHeader_t(file);
        std::vector<class_t> classes;
        file.seekg(header.classOffset);

        jsonData["version"] = header.version;
        jsonData["MemorySize"] = std::to_string(header.objectPtrCount);

        /*jsonData["header"]["version"] = header.version;
        jsonData["header"]["classLength"] = header.classLength;
        jsonData["header"]["propertyCount"] = header.propertyCount;
        jsonData["header"]["BSTNodeCount"] = header.BSTNodeCount;
        jsonData["header"]["objectPtrCount"] = header.objectPtrCount;
        jsonData["header"]["classOffset"] = header.classOffset;
        jsonData["header"]["propertyOffset"] = header.propertyOffset;
        jsonData["header"]["PropertyNameOffset"] = header.PropertyNameOffset;
        jsonData["header"]["BSTNodeOffset"] = header.BSTNodeOffset;
        jsonData["header"]["FileSize"] = header.FileSize;*/

        for (unsigned int i = 0; i < header.classLength; i++)
        {
            classes.push_back(class_t(file));
        }
        std::vector<std::vector<property_t>> AllProperties;
        for (unsigned int i = 0; i < classes.size(); i++)
        {
            std::vector<property_t> properties;
            file.seekg(header.propertyOffset + (classes[i].classPropertyStartingIndex * 16));
            for (unsigned int x = 0; x < classes[i].classPropertyCount; x++)
            {
                properties.push_back(property_t(file));
            }
            for (unsigned int x = 0; x < properties.size(); x++)
            {
                file.seekg(header.PropertyNameOffset + properties[x].propertyNameOffset);
                getline(file, properties[x].propertyName, '\0');
                //std::cout << "PROPERTY - Name: " << properties[x].propertyName << std::endl;
            }
            AllProperties.push_back(properties);
            file.seekg(header.PropertyNameOffset + classes[i].classPropertyNameOffset);
            getline(file, classes[i].className, '\0');
            //std::cout << "CLASS - Name: " << classes[i].className << std::endl;

            if (properties.empty())
            {
                jsonData["classes"][classes[i].className] = nullptr;
            }

            for (unsigned int x = 0; x < properties.size(); x++)
            {
                jsonData["classes"][classes[i].className][properties[x].propertyName]["propertyType"] = properties[x].propertyType;
                jsonData["classes"][classes[i].className][properties[x].propertyName]["objectByteSize"] = properties[x].objectByteSize;
                jsonData["classes"][classes[i].className][properties[x].propertyName]["arrayIndex"] = properties[x].arrayIndex;
            }
        }
        std::ofstream logFile(FileName + "_output.log", std::ios::out | std::ofstream::binary);
        file.seekg(header.BSTNodeOffset);
        for (unsigned int i = 0; i < header.BSTNodeCount; i++)
        {
            uint32_t index;
            file.read((char*)&index, sizeof(index));

            std::string BSTName;
            std::getline(file, BSTName, '\0');

            ReadClass(file, jsonData["BSTNodes"][BSTName], index, AllProperties, classes);
        }

        std::vector<std::string> bstNodeNames;
        for (ordered_json::iterator it = jsonData["BSTNodes"].begin(); it != jsonData["BSTNodes"].end(); ++it)
        {
            bstNodeNames.push_back(it.key());
        }
        for (ordered_json::iterator bstIt = jsonData["BSTNodes"].begin(); bstIt != jsonData["BSTNodes"].end(); ++bstIt)
        {
            if (!bstIt.value().is_object())
            {
                continue;
            }
            for (ordered_json::iterator classIt = bstIt.value().begin(); classIt != bstIt.value().end(); ++classIt)
            {
                auto classMetaIt = jsonData["classes"].find(classIt.key());
                if (classMetaIt == jsonData["classes"].end() || !classMetaIt->is_object() || !classIt.value().is_object())
                {
                    continue;
                }

                ResolveClumpRefsInClassData(classIt.value(), jsonData["classes"], *classMetaIt, classes, bstNodeNames);
            }
        }

        std::string jsonDataString = jsonData.dump(1);
        std::ofstream jsonFile(FileName + ".json", std::ios::out | std::ofstream::binary);
        jsonFile.write((char*)&jsonDataString[0], jsonDataString.length());
        jsonFile.close();
    }
    return 1;
}

typedef struct {
    std::string variableValue;
    int variableType;
    int variableSize;
    uint32_t variableArray;
} propertyHandler_t;

typedef struct {
    std::string className;
    std::vector<propertyHandler_t> variables;
} classHandler_t;

typedef struct {
    std::string BSTName;
    std::vector<classHandler_t> classes;
} BSTNodeHandler_t;

static std::string JsonValueToString(const ordered_json& value)
{
    if (value.is_string())
    {
        return value.get<std::string>();
    }
    if (value.is_boolean())
    {
        return value.get<bool>() ? "1" : "0";
    }
    if (value.is_number_integer() || value.is_number_unsigned() || value.is_number_float())
    {
        return value.dump();
    }
    if (value.is_null())
    {
        return "";
    }
    return value.dump();
}

static std::string NormalizeNumericText(std::string value)
{
    const std::string unknownPrefix = "[Unknown]";
    if (value.rfind(unknownPrefix, 0) == 0)
    {
        value = value.substr(unknownPrefix.length());
    }
    return value;
}

static bool IsUnknownHexValue(const std::string& value)
{
    return value.rfind("[Unknown]", 0) == 0;
}

static bool IsNanClumpString(std::string value)
{
    value.erase(
        std::remove_if(
            value.begin(),
            value.end(),
            [](unsigned char ch)
            {
                return std::isspace(ch) != 0;
            }),
        value.end());
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char ch)
        {
            return (char)std::tolower(ch);
        });
    return value.empty() || value == "-nan" || value == "nan" || value == "null";
}

static bool IsNanNumericText(std::string value)
{
    value.erase(
        std::remove_if(
            value.begin(),
            value.end(),
            [](unsigned char ch)
            {
                return std::isspace(ch) != 0;
            }),
        value.end());
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char ch)
        {
            return (char)std::tolower(ch);
        });
    return value == "-nan" || value == "nan";
}

static uint32_t NullClumpSentinel()
{
    return 0xFFFFFFFFu;
}

static uint32_t NanFloatSentinelBits()
{
    return 0xFFFFFFF6u;
}

static ordered_json ResolvePropertyJson(const ordered_json& classData, const std::string& propertyName, uint32_t propertyType)
{
    if (classData.is_object())
    {
        auto direct = classData.find(propertyName);
        if (direct != classData.end())
        {
            return *direct;
        }

        if (propertyType == 2)
        {
            std::string clumpPropertyName = "[CLUMP]" + propertyName;
            auto clump = classData.find(clumpPropertyName);
            if (clump != classData.end())
            {
                return *clump;
            }
        }
    }

    return ordered_json();
}

static uint32_t ResolveClumpReference(const ordered_json& value, const std::map<std::string, uint32_t>& nodeIndexByName)
{
    if (value.is_null())
    {
        return NullClumpSentinel();
    }

    std::string text = JsonValueToString(value);
    if (IsNanClumpString(text))
    {
        return NullClumpSentinel();
    }

    auto foundNode = nodeIndexByName.find(text);
    if (foundNode != nodeIndexByName.end())
    {
        return foundNode->second;
    }

    try
    {
        return static_cast<uint32_t>(std::stoul(text));
    }
    catch (...)
    {
        return NullClumpSentinel();
    }
}

static ordered_json ResolveNestedArrayElement(const ordered_json& element, const std::string& nestedClassName)
{
    if (element.is_object())
    {
        auto wrapped = element.find(nestedClassName);
        if (wrapped != element.end() && wrapped->is_object())
        {
            return *wrapped;
        }
    }

    return element;
}

static uint32_t CountObjectPointersInClassData(
    const ordered_json& jsonData,
    const std::vector<class_t>& classes,
    const std::string& className,
    const ordered_json& classData)
{
    auto classMetaIt = jsonData["classes"].find(className);
    if (classMetaIt == jsonData["classes"].end() || !classMetaIt->is_object())
    {
        return 0;
    }

    uint32_t objectPtrCount = 0;
    for (ordered_json::const_iterator propertyIt = classMetaIt->begin(); propertyIt != classMetaIt->end(); ++propertyIt)
    {
        const std::string propertyName = propertyIt.key();
        const uint32_t propertyType = (*propertyIt)["propertyType"];
        const uint32_t arrayIndex = (*propertyIt)["arrayIndex"];
        ordered_json value = ResolvePropertyJson(classData, propertyName, propertyType);

        if (propertyType == 2)
        {
            objectPtrCount += 1;
        }
        else if (propertyType == 3)
        {
            if (arrayIndex == 0xFFFFFFFFu)
            {
                if (value.is_object())
                {
                    auto dataIt = value.find("[CLUMP]data");
                    if (dataIt != value.end() && dataIt->is_array())
                    {
                        objectPtrCount += static_cast<uint32_t>(dataIt->size());
                    }
                }
                else if (value.is_array())
                {
                    objectPtrCount += static_cast<uint32_t>(value.size());
                }
            }
            else if (value.is_array() && arrayIndex < classes.size())
            {
                const std::string nestedClassName = classes[arrayIndex].className;
                for (ordered_json::const_iterator elementIt = value.begin(); elementIt != value.end(); ++elementIt)
                {
                    objectPtrCount += CountObjectPointersInClassData(
                        jsonData,
                        classes,
                        nestedClassName,
                        ResolveNestedArrayElement(*elementIt, nestedClassName));
                }
            }
        }
    }

    return objectPtrCount;
}

static uint32_t ComputeClassSerializedSize(
    const ordered_json& jsonData,
    const std::vector<class_t>& classes,
    const std::string& className,
    const ordered_json& classData)
{
    auto classMetaIt = jsonData["classes"].find(className);
    if (classMetaIt == jsonData["classes"].end() || !classMetaIt->is_object())
    {
        return 0;
    }

    uint32_t size = 0;
    for (ordered_json::const_iterator propertyIt = classMetaIt->begin(); propertyIt != classMetaIt->end(); ++propertyIt)
    {
        const std::string propertyName = propertyIt.key();
        const uint32_t propertyType = (*propertyIt)["propertyType"];
        const uint32_t objectByteSize = (*propertyIt)["objectByteSize"];
        const uint32_t arrayIndex = (*propertyIt)["arrayIndex"];
        ordered_json value = ResolvePropertyJson(classData, propertyName, propertyType);

        switch (propertyType)
        {
            case 0:
                size += objectByteSize;
            break;
            case 1:
                size += static_cast<uint32_t>(JsonValueToString(value).length() + 1);
            break;
            case 2:
                size += 4;
            break;
            case 3:
                size += 4;
                if (arrayIndex == 0xFFFFFFFFu)
                {
                    if (value.is_object())
                    {
                        auto dataIt = value.find("[CLUMP]data");
                        if (dataIt != value.end() && dataIt->is_array())
                        {
                            size += static_cast<uint32_t>(dataIt->size() * 4);
                        }
                    }
                    else if (value.is_array())
                    {
                        size += static_cast<uint32_t>(value.size() * 4);
                    }
                }
                else if (value.is_array() && arrayIndex < classes.size())
                {
                    const std::string nestedClassName = classes[arrayIndex].className;
                    for (ordered_json::const_iterator elementIt = value.begin(); elementIt != value.end(); ++elementIt)
                    {
                        size += ComputeClassSerializedSize(
                            jsonData,
                            classes,
                            nestedClassName,
                            ResolveNestedArrayElement(*elementIt, nestedClassName));
                    }
                }
            break;
            default:
                size += objectByteSize > 0 ? objectByteSize : 4;
            break;
        }
    }

    return size;
}

static void WriteClass(std::ofstream& objectLevelBin, BSTNodeHandler_t BSTNode)
{
    char nullTerminator = '\0';

    uint8_t TempBool;
    uint16_t TempUint16;
    uint32_t TempUint32;
    float TempFloat;
    double TempDouble;
    long double TempLongDouble;
    vector4_t vector4Temp;
    transform_t transformTemp;
    ordered_json TempJson;

    for (unsigned int x = 0; x < BSTNode.classes.size(); x++)
    {
        for (unsigned int y = 0; y < BSTNode.classes[x].variables.size(); y++)
        {
            switch (BSTNode.classes[x].variables[y].variableType)
            {
                case 0: //General Type
                    switch (BSTNode.classes[x].variables[y].variableSize)
                    {
                        case 1: //uint8_t
                            TempBool = std::atoi(BSTNode.classes[x].variables[y].variableValue.c_str());
                            objectLevelBin.write(reinterpret_cast<char*>(&TempBool), BSTNode.classes[x].variables[y].variableSize);
                        break;
                        case 2: //uint16_t
                            TempUint16 = std::stoi(BSTNode.classes[x].variables[y].variableValue.c_str());
                            objectLevelBin.write(reinterpret_cast<char*>(&TempUint16), BSTNode.classes[x].variables[y].variableSize);
                        break;
                        case 4: //uint32_t
                            if (BSTNode.classes[x].variables[y].variableValue.find('.') != std::string::npos)
                            {
                                TempFloat = std::stof(BSTNode.classes[x].variables[y].variableValue);
                                objectLevelBin.write(reinterpret_cast<char*>(&TempFloat), BSTNode.classes[x].variables[y].variableSize);
                            }
                            else
                            {
                                TempUint32 = std::stoi(BSTNode.classes[x].variables[y].variableValue);
                                objectLevelBin.write(reinterpret_cast<char*>(&TempUint32), BSTNode.classes[x].variables[y].variableSize);
                            }
                        break;
                        case 8: //Double
                            TempDouble = std::stod(BSTNode.classes[x].variables[y].variableValue);
                            objectLevelBin.write(reinterpret_cast<char*>(&TempDouble), BSTNode.classes[x].variables[y].variableSize);
                        break;
                        case 10: //Long Double
                            TempLongDouble = std::stold(BSTNode.classes[x].variables[y].variableValue);
                            objectLevelBin.write(reinterpret_cast<char*>(&TempLongDouble), BSTNode.classes[x].variables[y].variableSize);
                        break;
                        case 16: //vector4_t
                            vector4Temp = vector4_t(BSTNode.classes[x].variables[y].variableValue);
                            objectLevelBin.write(reinterpret_cast<char*>(&vector4Temp), BSTNode.classes[x].variables[y].variableSize);
                        break;
                        case 64: //transform_t
                            transformTemp = transform_t(BSTNode.classes[x].variables[y].variableValue);
                            objectLevelBin.write(reinterpret_cast<char*>(&transformTemp), BSTNode.classes[x].variables[y].variableSize);
                        break;
                        default:
                            objectLevelBin.write(reinterpret_cast<char*>(&BSTNode.classes[x].variables[y].variableValue), BSTNode.classes[x].variables[y].variableSize);
                        break;
                    }
                break;
                case 1: //TGCString
                    objectLevelBin.write(BSTNode.classes[x].variables[y].variableValue.c_str(), BSTNode.classes[x].variables[y].variableValue.length());
                    objectLevelBin.write(reinterpret_cast<char*>(&nullTerminator), sizeof(nullTerminator));
                break;
                case 2: //Clump
                    std::cout << "Clumps not supported!" << std::endl;
                    objectLevelBin.write(reinterpret_cast<char*>(&BSTNode.classes[x].variables[y].variableValue), BSTNode.classes[x].variables[y].variableSize);
                break;
                case 3: //Array
                    if (BSTNode.classes[x].variables[y].variableSize != -1)
                    {
                        std::cout << "Arrays not supported!" << std::endl;
                        objectLevelBin.write(reinterpret_cast<char*>(&BSTNode.classes[x].variables[y].variableValue), 4 * BSTNode.classes[x].variables[y].variableSize);
                    }
                    else
                    {
                        objectLevelBin.write(reinterpret_cast<char*>(&BSTNode.classes[x].variables[y].variableValue), 4 * BSTNode.classes[x].variables[y].variableSize);
                    }
                break;
                default: //Unknown
                    std::cout << "UNKNOWN" << std::endl;
                    if (BSTNode.classes[x].variables[y].variableSize > 0)
                    {
                        objectLevelBin.write(reinterpret_cast<char*>(&BSTNode.classes[x].variables[y].variableValue), BSTNode.classes[x].variables[y].variableSize);
                    }
                    else
                    {
                        objectLevelBin.write(reinterpret_cast<char*>(&BSTNode.classes[x].variables[y].variableValue), 4);
                    }
                break;
            }
        }
    }

    /*uint32_t customSize;
    for (unsigned int x = 0; x < BSTNode.variables.size(); x++)
    {
        switch (properties[index][x].propertyType)
        {
            case 2: //Clump
                //TempUint32
            break;
            case 3: //Array
                //file.read((char*)&customSize, sizeof(customSize));
                //if ((int)AllProperties[index][x].arrayIndex != -1)
                //{
                //    for (unsigned int y = 0; y < customSize; y++)
                //    {
                //        ReadClass(file, jsonData[classesRef[index].className][AllProperties[index][x].propertyName][y], AllProperties[index][x].arrayIndex, AllProperties, classesRef);
                //    }
                //}
                //else
                //{
                //    file.ignore(customSize * 4);
                //}
            break;
            default: //Unknown
                //jsonData[classesRef[index].className]["[UNKNOWN]" + AllProperties[index][x].propertyName] = "[Size (Bytes)] " + std::to_string(AllProperties[index][x].objectByteSize);
               // if (AllProperties[index][x].objectByteSize > 0)
                //{
                    //file.ignore(AllProperties[index][x].objectByteSize);
                    //logFile << "Bytes skipped: " << AllProperties[index][x].objectByteSize << std::endl;
               // }
               // else
               // {
                    //file.ignore(4);
                    //logFile << "Bytes skipped: " << 4 << std::endl;
               // }
            break;
        }
    }*/
}

static void WriteClass(
    std::ofstream& objectLevelBin,
    const ordered_json& jsonData,
    const std::vector<class_t>& classes,
    const std::map<std::string, uint32_t>& nodeIndexByName,
    const std::map<std::string, uint32_t>& originalTopLevelUint32Values,
    const std::string& nodeName,
    const std::string& className,
    const ordered_json& classData)
{
    char nullTerminator = '\0';

    uint8_t TempBool;
    uint16_t TempUint16;
    uint32_t TempUint32;
    float TempFloat;
    double TempDouble;
    long double TempLongDouble;
    vector4_t vector4Temp;
    transform_t transformTemp;

    auto classMetaIt = jsonData["classes"].find(className);
    if (classMetaIt == jsonData["classes"].end() || !classMetaIt->is_object())
    {
        return;
    }

    for (ordered_json::const_iterator propertyIt = classMetaIt->begin(); propertyIt != classMetaIt->end(); ++propertyIt)
    {
        const std::string propertyName = propertyIt.key();
        const uint32_t propertyType = (*propertyIt)["propertyType"];
        const uint32_t objectByteSize = (*propertyIt)["objectByteSize"];
        const uint32_t arrayIndex = (*propertyIt)["arrayIndex"];
        ordered_json valueJson = ResolvePropertyJson(classData, propertyName, propertyType);
        std::string rawValueText = JsonValueToString(valueJson);
        bool valueIsUnknownHex = IsUnknownHexValue(rawValueText);
        std::string valueText = NormalizeNumericText(rawValueText);

        switch (propertyType)
        {
            case 0:
                try
                {
                    switch (objectByteSize)
                    {
                        case 1:
                            TempBool = valueIsUnknownHex
                                ? static_cast<uint8_t>(std::stoul(valueText.empty() ? "0" : valueText, nullptr, 16))
                                : static_cast<uint8_t>(std::atoi(valueText.c_str()));
                            objectLevelBin.write(reinterpret_cast<char*>(&TempBool), objectByteSize);
                        break;
                        case 2:
                            TempUint16 = valueIsUnknownHex
                                ? static_cast<uint16_t>(std::stoul(valueText.empty() ? "0" : valueText, nullptr, 16))
                                : static_cast<uint16_t>(std::stoi(valueText.empty() ? "0" : valueText));
                            objectLevelBin.write(reinterpret_cast<char*>(&TempUint16), objectByteSize);
                        break;
                        case 4:
                            if (IsNanNumericText(valueText))
                            {
                                auto originalValueIt = originalTopLevelUint32Values.find(MakeOriginalValueKey(nodeName, className, propertyName));
                                TempUint32 = originalValueIt != originalTopLevelUint32Values.end()
                                    ? originalValueIt->second
                                    : NanFloatSentinelBits();
                                objectLevelBin.write(reinterpret_cast<char*>(&TempUint32), objectByteSize);
                            }
                            else if (!valueIsUnknownHex && (valueText.find('.') != std::string::npos || valueText.find('e') != std::string::npos || valueText.find('E') != std::string::npos))
                            {
                                TempFloat = std::stof(valueText.empty() ? "0" : valueText);
                                objectLevelBin.write(reinterpret_cast<char*>(&TempFloat), objectByteSize);
                            }
                            else
                            {
                                TempUint32 = static_cast<uint32_t>(std::stoul(valueText.empty() ? "0" : valueText, nullptr, valueIsUnknownHex ? 16 : 10));
                                objectLevelBin.write(reinterpret_cast<char*>(&TempUint32), objectByteSize);
                            }
                        break;
                        case 8:
                            TempDouble = IsNanNumericText(valueText)
                                ? std::numeric_limits<double>::quiet_NaN()
                                : std::stod(valueText.empty() ? "0" : valueText);
                            objectLevelBin.write(reinterpret_cast<char*>(&TempDouble), objectByteSize);
                        break;
                        case 10:
                            TempLongDouble = IsNanNumericText(valueText)
                                ? std::numeric_limits<long double>::quiet_NaN()
                                : std::stold(valueText.empty() ? "0" : valueText);
                            objectLevelBin.write(reinterpret_cast<char*>(&TempLongDouble), objectByteSize);
                        break;
                        case 16:
                            vector4Temp = valueText.empty() ? vector4_t() : vector4_t(valueText);
                            objectLevelBin.write(reinterpret_cast<char*>(&vector4Temp), objectByteSize);
                        break;
                        case 64:
                            transformTemp = valueText.empty() ? transform_t() : transform_t(valueText);
                            objectLevelBin.write(reinterpret_cast<char*>(&transformTemp), objectByteSize);
                        break;
                        default:
                        {
                            std::vector<char> zeroBytes(objectByteSize > 0 ? objectByteSize : 4, 0);
                            objectLevelBin.write(zeroBytes.data(), zeroBytes.size());
                        }
                        break;
                    }
                }
                catch (const std::exception&)
                {
                    std::vector<char> zeroBytes(objectByteSize > 0 ? objectByteSize : 4, 0);
                    objectLevelBin.write(zeroBytes.data(), zeroBytes.size());
                }
            break;
            case 1:
                objectLevelBin.write(valueText.c_str(), valueText.length());
                objectLevelBin.write(reinterpret_cast<char*>(&nullTerminator), sizeof(nullTerminator));
            break;
            case 2:
                TempUint32 = ResolveClumpReference(valueJson, nodeIndexByName);
                objectLevelBin.write(reinterpret_cast<char*>(&TempUint32), sizeof(TempUint32));
            break;
            case 3:
                if (arrayIndex == 0xFFFFFFFFu)
                {
                    std::vector<ordered_json> clumpValues;
                    if (valueJson.is_object())
                    {
                        auto dataIt = valueJson.find("[CLUMP]data");
                        if (dataIt != valueJson.end() && dataIt->is_array())
                        {
                            for (ordered_json::const_iterator clumpIt = dataIt->begin(); clumpIt != dataIt->end(); ++clumpIt)
                            {
                                clumpValues.push_back(*clumpIt);
                            }
                        }
                    }
                    else if (valueJson.is_array())
                    {
                        for (ordered_json::const_iterator clumpIt = valueJson.begin(); clumpIt != valueJson.end(); ++clumpIt)
                        {
                            clumpValues.push_back(*clumpIt);
                        }
                    }

                    TempUint32 = static_cast<uint32_t>(clumpValues.size());
                    objectLevelBin.write(reinterpret_cast<char*>(&TempUint32), sizeof(TempUint32));
                    for (const ordered_json& clumpValue : clumpValues)
                    {
                        uint32_t clumpIndex = ResolveClumpReference(clumpValue, nodeIndexByName);
                        objectLevelBin.write(reinterpret_cast<char*>(&clumpIndex), sizeof(clumpIndex));
                    }
                }
                else
                {
                    TempUint32 = valueJson.is_array() ? static_cast<uint32_t>(valueJson.size()) : 0;
                    objectLevelBin.write(reinterpret_cast<char*>(&TempUint32), sizeof(TempUint32));

                    if (valueJson.is_array() && arrayIndex < classes.size())
                    {
                        const std::string nestedClassName = classes[arrayIndex].className;
                        for (ordered_json::const_iterator elementIt = valueJson.begin(); elementIt != valueJson.end(); ++elementIt)
                        {
                            WriteClass(
                                objectLevelBin,
                                jsonData,
                                classes,
                                nodeIndexByName,
                                originalTopLevelUint32Values,
                                nodeName,
                                nestedClassName,
                                ResolveNestedArrayElement(*elementIt, nestedClassName));
                        }
                    }
                }
            break;
            default:
            {
                std::vector<char> zeroBytes(objectByteSize > 0 ? objectByteSize : 4, 0);
                objectLevelBin.write(zeroBytes.data(), zeroBytes.size());
            }
            break;
        }
    }
}

static int SaveObjects(std::string fileName)
{
    size_t directorySize = fileName.find_last_of("/\\");
    std::string directory = fileName.substr(0, directorySize) + "\\";
    std::string fileWithExtension = fileName.substr(directorySize + 1);
    size_t extensionSplit(fileWithExtension.find_last_of('.'));
    std::string fileNameWithoutextension = fileWithExtension.substr(0, extensionSplit);
    std::string fileNameExtension = fileWithExtension.substr(fileWithExtension.find_last_of(".") + 1);
    if (std::ifstream file{ fileName, std::ios::binary })
    {
        std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        ordered_json jsonData = ordered_json::parse(data);
        std::ofstream objectLevelBin(directory + fileNameWithoutextension + "_new.level.bin", std::ios::out | std::ofstream::binary);

        std::vector<class_t> classes;
        std::vector<property_t> properties;
        std::map<std::string, uint32_t> classIndexByName;
        std::map<std::string, uint32_t> nodeIndexByName;
        std::map<std::string, uint32_t> pooledNameOffsets;
        std::map<std::string, uint32_t> originalTopLevelUint32Values;
        std::vector<std::string> pooledNames;

        int version = jsonData.contains("version") ? jsonData["version"].get<int>() : 1;
        int propertyCount = 0;
        int classOffset = 44; // This should always be 44
        int offset = 44; // That is after the header

        int classStartingIndex = 0;
        for (ordered_json::iterator it = jsonData["classes"].begin(); it != jsonData["classes"].end(); ++it)
        {
            class_t classData;
            const ordered_json& classMeta = jsonData["classes"][it.key()];
            classData.classPropertyCount = classMeta.is_object() ? static_cast<uint32_t>(classMeta.size()) : 0;
            classData.classPropertyStartingIndex = classStartingIndex;
            classData.classPropertyNameOffset = 0;

            classData.className = it.key();
            classStartingIndex += classData.classPropertyCount;
            classIndexByName[classData.className] = static_cast<uint32_t>(classes.size());
            classes.push_back(classData);
            offset += 12;
        }
        int classCount = static_cast<int>(classes.size());
        int propertyOffset = offset;
        for (unsigned int i = 0; i < classes.size(); i++)
        {
            ordered_json& classMeta = jsonData["classes"][classes[i].className];
            if (!classMeta.is_object())
            {
                continue;
            }

            for (ordered_json::iterator it = classMeta.begin(); it != classMeta.end(); ++it)
            {
                property_t propertyData;
                propertyData.propertyType = classMeta[it.key()]["propertyType"];
                propertyData.propertyNameOffset = 0;
                propertyData.objectByteSize = classMeta[it.key()]["objectByteSize"];
                propertyData.arrayIndex = classMeta[it.key()]["arrayIndex"];

                propertyData.propertyName = it.key();
                properties.push_back(propertyData);
                offset += 16;
                propertyCount++;
            }
        }
        uint32_t pooledNameOffset = 0;
        std::vector<std::string> originalStringPoolOrder;
        auto registerPooledName = [&](const std::string& name) -> uint32_t
        {
            auto found = pooledNameOffsets.find(name);
            if (found != pooledNameOffsets.end())
            {
                return found->second;
            }

            uint32_t assignedOffset = pooledNameOffset;
            pooledNameOffsets[name] = assignedOffset;
            pooledNames.push_back(name);
            pooledNameOffset += static_cast<uint32_t>(name.length() + 1);
            return assignedOffset;
        };

        LoadOriginalStringPoolOrder(fileName, originalStringPoolOrder);
        LoadOriginalTopLevelUint32Values(fileName, originalTopLevelUint32Values);
        for (const std::string& pooledName : originalStringPoolOrder)
        {
            bool isReferenced = false;
            for (unsigned int i = 0; i < properties.size() && !isReferenced; i++)
            {
                isReferenced = properties[i].propertyName == pooledName;
            }
            for (unsigned int i = 0; i < classes.size() && !isReferenced; i++)
            {
                isReferenced = classes[i].className == pooledName;
            }
            if (isReferenced)
            {
                registerPooledName(pooledName);
            }
        }
        for (unsigned int i = 0; i < properties.size(); i++)
        {
            properties[i].propertyNameOffset = registerPooledName(properties[i].propertyName);
        }
        for (unsigned int i = 0; i < classes.size(); i++)
        {
            classes[i].classPropertyNameOffset = registerPooledName(classes[i].className);
        }
        int propertyNamesOffset = offset;
        offset += static_cast<int>(pooledNameOffset);
        int BSTNodesOffset = offset;

        uint32_t ObjectPtrCount = 0;
        int BSTNodeCount = static_cast<int>(jsonData["BSTNodes"].size());
        uint32_t bstIndex = 0;
        for (ordered_json::iterator it = jsonData["BSTNodes"].begin(); it != jsonData["BSTNodes"].end(); ++it)
        {
            nodeIndexByName[it.key()] = bstIndex++;

            if (!it.value().is_object() || it.value().empty())
            {
                continue;
            }

            ordered_json::const_iterator classIt = it.value().begin();
            const std::string className = classIt.key();
            auto classIndexIt = classIndexByName.find(className);
            if (classIndexIt == classIndexByName.end())
            {
                std::cout << "Unknown class in BSTNode " << it.key() << ": " << className << std::endl;
                return 0;
            }

            if (std::next(classIt) != it.value().end())
            {
                std::cout << "Warning: multi-class BSTNode not fully supported yet, using first class only for " << it.key() << std::endl;
            }

            offset += 4;
            offset += static_cast<int>(it.key().length() + 1);
            offset += static_cast<int>(ComputeClassSerializedSize(jsonData, classes, className, classIt.value()));
            ObjectPtrCount += CountObjectPointersInClassData(jsonData, classes, className, classIt.value());
        }
        int fileSize = offset;

        //Header
        objectLevelBin.write("TGCL", 4);
        objectLevelBin.write(reinterpret_cast<char*>(&version), sizeof(version));
        objectLevelBin.write(reinterpret_cast<char*>(&classCount), sizeof(classCount));
        objectLevelBin.write(reinterpret_cast<char*>(&propertyCount), sizeof(propertyCount));
        objectLevelBin.write(reinterpret_cast<char*>(&BSTNodeCount), sizeof(BSTNodeCount));
        objectLevelBin.write(reinterpret_cast<char*>(&ObjectPtrCount), sizeof(ObjectPtrCount));
        objectLevelBin.write(reinterpret_cast<char*>(&classOffset), sizeof(classOffset));
        objectLevelBin.write(reinterpret_cast<char*>(&propertyOffset), sizeof(propertyOffset));
        objectLevelBin.write(reinterpret_cast<char*>(&propertyNamesOffset), sizeof(propertyNamesOffset));
        objectLevelBin.write(reinterpret_cast<char*>(&BSTNodesOffset), sizeof(BSTNodesOffset));
        objectLevelBin.write(reinterpret_cast<char*>(&fileSize), sizeof(fileSize));
        //

        for (unsigned int i = 0; i < classes.size(); i++)
        {
            objectLevelBin.write(reinterpret_cast<char*>(&classes[i].classPropertyNameOffset), sizeof(classes[i].classPropertyNameOffset));
            objectLevelBin.write(reinterpret_cast<char*>(&classes[i].classPropertyStartingIndex), sizeof(classes[i].classPropertyStartingIndex));
            objectLevelBin.write(reinterpret_cast<char*>(&classes[i].classPropertyCount), sizeof(classes[i].classPropertyCount));
        }

        for (unsigned int i = 0; i < properties.size(); i++)
        {
            objectLevelBin.write(reinterpret_cast<char*>(&properties[i].propertyType), sizeof(properties[i].propertyType));
            objectLevelBin.write(reinterpret_cast<char*>(&properties[i].propertyNameOffset), sizeof(properties[i].propertyNameOffset));
            objectLevelBin.write(reinterpret_cast<char*>(&properties[i].objectByteSize), sizeof(properties[i].objectByteSize));
            objectLevelBin.write(reinterpret_cast<char*>(&properties[i].arrayIndex), sizeof(properties[i].arrayIndex));
        }

        char nullTerminator = '\0';
        for (unsigned int i = 0; i < pooledNames.size(); i++)
        {
            objectLevelBin.write(pooledNames[i].c_str(), pooledNames[i].length());
            objectLevelBin.write(reinterpret_cast<char*>(&nullTerminator), sizeof(nullTerminator));
        }
        for (ordered_json::iterator it = jsonData["BSTNodes"].begin(); it != jsonData["BSTNodes"].end(); ++it)
        {
            if (!it.value().is_object() || it.value().empty())
            {
                continue;
            }

            ordered_json::const_iterator classIt = it.value().begin();
            auto classIndexIt = classIndexByName.find(classIt.key());
            if (classIndexIt == classIndexByName.end())
            {
                std::cout << "Unknown class in BSTNode " << it.key() << ": " << classIt.key() << std::endl;
                return 0;
            }

            uint32_t classIndex = classIndexIt->second;
            objectLevelBin.write(reinterpret_cast<char*>(&classIndex), sizeof(classIndex));
            objectLevelBin.write(it.key().c_str(), it.key().length());
            objectLevelBin.write(reinterpret_cast<char*>(&nullTerminator), sizeof(nullTerminator));
            WriteClass(
                objectLevelBin,
                jsonData,
                classes,
                nodeIndexByName,
                originalTopLevelUint32Values,
                it.key(),
                classIt.key(),
                classIt.value());
        }

        objectLevelBin.close();
    }
    return 1;
}
