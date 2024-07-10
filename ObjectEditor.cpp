#include <iostream>
#include "ObjectFormat.h"

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            std::string fileName = argv[i];
            size_t directorySize = fileName.find_last_of("/\\");
            std::string directory = fileName.substr(0, directorySize) + "\\";
            std::string fileWithExtension = fileName.substr(directorySize + 1);
            size_t extensionSplit(fileWithExtension.find_last_of('.'));
            std::string fileNameWithoutextension = fileWithExtension.substr(0, extensionSplit);
            std::string fileNameExtension = fileWithExtension.substr(fileWithExtension.find_last_of(".") + 1);
            if (fileNameExtension == "bin")
            {
                LoadObjects(argv[i]);
            }
            else if (fileNameExtension == "json")
            {
                SaveObjects(argv[i]);
            }
            else if (fileNameExtension == "mesh")
            {
                //MeshHandler(argv[1]);
            }
            else if (fileNameExtension == "mesh_extracted")
            {
                //ConvertMesh(argv[1]);
            }
            else if (fileNameExtension == "meshes")
            {
                std::cout << ".meshes are not currently supported. Sorry for the inconvenience!" << std::endl;
            }
        }
    }
    else
    {
        std::cout << "Please drop a file into the Editor to begin" << std::endl;
    }


    system("pause");
    return 0;
}