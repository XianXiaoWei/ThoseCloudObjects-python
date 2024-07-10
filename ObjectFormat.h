#pragma once
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <iomanip>
#include <filesystem>
#include <sstream>
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

static void ReadClass(std::ifstream& file, ordered_json& jsonData, uint32_t index, std::vector<std::vector<property_t>> AllProperties, std::vector<class_t> classesRef)
{
    uint32_t customSize;
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
                        jsonData[classesRef[index].className][AllProperties[index][x].propertyName] = TempBool ? true : false;
                    break;
                    case 2: //Unsigned short
                        file.read((char*)&TempUint16, sizeof(TempUint16));
                        jsonData[classesRef[index].className][AllProperties[index][x].propertyName] = std::to_string(TempUint16);
                    break;
                    case 4: //Float or Int
                        file.read((char*)&TempUint32, sizeof(TempUint32));
                        if (AllProperties[index][x].propertyName == "bstGuid")
                        {
                            jsonData[classesRef[index].className][AllProperties[index][x].propertyName] = std::to_string(TempUint32);
                        }
                        else
                        {
                            if (std::to_string(ieee_float(TempUint32)) == "0.000000" || std::to_string(ieee_float(TempUint32)) == "-0.000000")
                            {
                                jsonData[classesRef[index].className][AllProperties[index][x].propertyName] = std::to_string(TempUint32);
                            }
                            else
                            {
                                jsonData[classesRef[index].className][AllProperties[index][x].propertyName] = std::to_string(ieee_float(TempUint32));
                            }
                        }
                    break;
                    case 8: //Double
                        file.read((char*)&TempDouble, sizeof(TempDouble));
                        jsonData[classesRef[index].className][AllProperties[index][x].propertyName] = std::to_string(TempDouble);
                    break;
                    case 10: //Long Double
                        file.read((char*)&TempLongDouble, sizeof(TempLongDouble));
                        jsonData[classesRef[index].className][AllProperties[index][x].propertyName] = std::to_string(TempLongDouble);
                    break;
                    case 16: //Vector 4
                        file.read((char*)&vector4Temp, sizeof(vector4Temp));
                        jsonData[classesRef[index].className][AllProperties[index][x].propertyName] = ordered_json{ std::to_string(vector4Temp.x), std::to_string(vector4Temp.y), std::to_string(vector4Temp.z), std::to_string(vector4Temp.w) };
                    break;
                    case 64: //Transform
                        file.read((char*)&transformTemp, sizeof(transformTemp));
                        jsonData[classesRef[index].className][AllProperties[index][x].propertyName] = ordered_json{
                            { std::to_string(transformTemp.x), std::to_string(transformTemp.y), std::to_string(transformTemp.z), std::to_string(transformTemp.w) },
                            { std::to_string(transformTemp.x1), std::to_string(transformTemp.y1), std::to_string(transformTemp.z1), std::to_string(transformTemp.w1) },
                            { std::to_string(transformTemp.x2), std::to_string(transformTemp.y2), std::to_string(transformTemp.z2), std::to_string(transformTemp.w2) },
                            { std::to_string(transformTemp.x3), std::to_string(transformTemp.y3), std::to_string(transformTemp.z3), std::to_string(transformTemp.w3) }
                        };
                    break;
                    default:
                        jsonData[classesRef[index].className]["[Unknown Property]" + AllProperties[index][x].propertyName] = "[Size (Bytes)]" + std::to_string(AllProperties[index][x].objectByteSize);
                        file.ignore(AllProperties[index][x].objectByteSize);
                    break;
                }
            break;
            case 1: //TGCString
                getline(file, TGCString, '\0');
                jsonData[classesRef[index].className][AllProperties[index][x].propertyName] = TGCString;
            break;
            case 2: //Clump
                file.read((char*)&TempUint32, sizeof(TempUint32));
                if (std::to_string(ieee_float(TempUint32)) != "-nan")
                {
                    jsonData[classesRef[index].className]["[CLUMP]" + AllProperties[index][x].propertyName] = std::to_string(TempUint32);
                    //std::cout << "Go to offset - " << std::to_string(TempUint32) << std::endl;
                }  
                else
                {
                    jsonData[classesRef[index].className]["[CLUMP]" + AllProperties[index][x].propertyName] = std::to_string(ieee_float(TempUint32));
                }
            break;
            case 3: //Array
                file.read((char*)&customSize, sizeof(customSize));
                if ((int)AllProperties[index][x].arrayIndex != -1)
                {
                    for (unsigned int y = 0; y < customSize; y++)
                    {
                        ReadClass(file, jsonData[classesRef[index].className][AllProperties[index][x].propertyName][y], AllProperties[index][x].arrayIndex, AllProperties, classesRef);
                    }
                }
                else
                {
                    file.ignore(customSize * 4);
                }
            break;
            default: //Unknown
                jsonData[classesRef[index].className]["[UNKNOWN]" + AllProperties[index][x].propertyName] = "[Size (Bytes)] " + std::to_string(AllProperties[index][x].objectByteSize);
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

static int LoadObjects(std::string FileName)
{
    if (std::ifstream file{ FileName, std::ios::binary })
    {
        ordered_json jsonData;
        BSTHeader_t header = BSTHeader_t(file);
        std::vector<class_t> classes;
        file.seekg(header.classOffset);

        jsonData["version"] = header.version;

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
        std::vector<BSTNodeHandler_t> BSTNodes;

        int version = 1;
        int classCount = jsonData["classes"].size();
        int propertyCount = 0;
        int BSTNodeCount = jsonData["BSTNodes"].size();
        int ObjectPtrCount = 0;
        int classOffset = 44; //This should always be 44
        int offset = 44; //That is after the header

        int nameOffset = 0;
        int classStartingIndex = 0;
        for (ordered_json::iterator it = jsonData["classes"].begin(); it != jsonData["classes"].end(); ++it)
        {
            class_t classData;
            classData.classPropertyCount = jsonData["classes"][it.key()].size();
            classData.classPropertyStartingIndex = classStartingIndex;
            classData.classPropertyNameOffset = nameOffset;

            classData.className = it.key();
            nameOffset += classData.className.length() + 1;
            classStartingIndex += jsonData["classes"][it.key()].size();
            classes.push_back(classData);
            offset += 12;
        }
        int propertyOffset = offset;
        for (unsigned int i = 0; i < classes.size(); i++)
        {
            for (ordered_json::iterator it = jsonData["classes"][classes[i].className].begin(); it != jsonData["classes"][classes[i].className].end(); ++it)
            {
                property_t propertyData;
                propertyData.propertyType = jsonData["classes"][classes[i].className][it.key()]["propertyType"];
                propertyData.propertyNameOffset = nameOffset;
                propertyData.objectByteSize = jsonData["classes"][classes[i].className][it.key()]["objectByteSize"];
                propertyData.arrayIndex = jsonData["classes"][classes[i].className][it.key()]["arrayIndex"];

                propertyData.propertyName = it.key();
                nameOffset += propertyData.propertyName.length() + 1;
                properties.push_back(propertyData);
                offset += 16;
                propertyCount++;
            }
        }
        int propertyNamesOffset = offset;
        for (unsigned int i = 0; i < properties.size(); i++)
        {
            offset += properties[i].propertyName.length() + 1;
        }
        for (unsigned int i = 0; i < classes.size(); i++)
        {
            offset += classes[i].className.length() + 1;
        }
        int BSTNodesOffset = offset;

        for (ordered_json::iterator it = jsonData["BSTNodes"].begin(); it != jsonData["BSTNodes"].end(); ++it)
        {
            BSTNodeHandler_t BSTNode;
            BSTNode.BSTName = it.key();
            for (ordered_json::iterator classit = jsonData["BSTNodes"][it.key()].begin(); classit != jsonData["BSTNodes"][it.key()].end(); ++classit)
            {
                classHandler_t BSTclass;
                BSTclass.className = classit.key();
                offset += BSTclass.className.length();
                for (ordered_json::iterator propertyit = jsonData["BSTNodes"][it.key()][classit.key()].begin(); propertyit != jsonData["BSTNodes"][it.key()][classit.key()].end(); ++propertyit)
                {
                    propertyHandler_t BSTproperty;
                    std::string value = propertyit.value().dump();
                    value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
                    BSTproperty.variableValue = value;
                    BSTproperty.variableType = jsonData["classes"][classit.key()][propertyit.key()]["propertyType"];
                    BSTproperty.variableSize = jsonData["classes"][classit.key()][propertyit.key()]["objectByteSize"];
                    switch (BSTproperty.variableType)
                    {
                        case 0:
                            offset += jsonData["classes"][classit.key()][propertyit.key()]["objectByteSize"];
                        break;
                        case 1:
                            offset += value.length();
                        break;
                        case 2:
                            std::cout << "CLUMP Added but zeroed out" << std::endl;
                            offset += 4;
                        break;
                        case 3:
                            std::cout << "ARRAY Added but zeroed out" << std::endl;
                            offset += jsonData["classes"][classit.key()][propertyit.key()]["objectByteSize"] * 4;
                        break;
                        default:
                            std::cout << "UNKNOWN Added but zeroed out" << std::endl;
                            offset += 4;
                        break;
                    }
                    BSTclass.variables.push_back(BSTproperty);
                }
                BSTNode.classes.push_back(BSTclass);
            }
            BSTNodes.push_back(BSTNode);
        }
        int fileSize = offset; //Size wrong (it needs to be 429
        
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
        for (unsigned int i = 0; i < classes.size(); i++)
        {
            objectLevelBin.write(classes[i].className.c_str(), classes[i].className.length());
            objectLevelBin.write(reinterpret_cast<char*>(&nullTerminator), sizeof(nullTerminator));
        }
        for (unsigned int i = 0; i < properties.size(); i++)
        {
            objectLevelBin.write(properties[i].propertyName.c_str(), properties[i].propertyName.length());
            objectLevelBin.write(reinterpret_cast<char*>(&nullTerminator), sizeof(nullTerminator));
        }
        for (unsigned int i = 0; i < BSTNodes.size(); i++)
        {
            objectLevelBin.write(reinterpret_cast<char*>(&i), sizeof(i));
            objectLevelBin.write(BSTNodes[i].BSTName.c_str(), BSTNodes[i].BSTName.length());
            objectLevelBin.write(reinterpret_cast<char*>(&nullTerminator), sizeof(nullTerminator));
            WriteClass(objectLevelBin, BSTNodes[i]);
        }

        objectLevelBin.close();
    }
    return 1;
}