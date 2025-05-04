#include "Wad.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include <regex>
#include <sys/types.h>
#include <sstream>
#include <unordered_map>



void Wad::buildPathMap(Node* node, const std::string& currentPath) {
    std::string path = currentPath + node->elementName;
    if (node->isDirectory != 0) path += "/"; 
    pathMap[path] = node;
    for (Node* child : node->children) {
        buildPathMap(child, path);
    }
}

Wad::Wad(const std::string &path) {
    if (!pathMap.empty()) pathMap.clear();

    this->file = std::fstream(path, std::ios::binary | std::ios::in | std::ios::out);
    if (!this->file.is_open()) {
        std::cerr << "Failed to open WAD file: " << path << std::endl;
        return;
    }

    const int HeaderBytes = 12;
    std::vector<char> buffer(HeaderBytes);
    if (!this->file.read(buffer.data(), HeaderBytes)) {
        std::cerr << "Failed to read WAD header." << std::endl;
        return;
    }

    Magic = std::string(buffer.begin(), buffer.begin() + 4);
    descriptorCount = *reinterpret_cast<int*>(&buffer[4]);
    descriptorOffset = *reinterpret_cast<int*>(&buffer[8]);

    this->file.seekg(descriptorOffset);

    const int DescriptorBytes = 16;
    std::vector<char> dBuffer(DescriptorBytes);
    std::stack<Node*> currentFolder;
    std::regex pattern("^E[0-9]M[0-9]$");

    bool currentMapMarker = false;
    int nextTen = 0;

    root = new Node(0, 0, "", 1);
    pathMap["/"] = root;

    for (int i = 0; i < descriptorCount; i++) {
        if (!this->file.read(dBuffer.data(), DescriptorBytes)) {
            std::cerr << "Failed to read descriptor " << i << std::endl;
            break;
        }

        int elementOffset = *reinterpret_cast<int*>(&dBuffer[0]);
        int elementLength = *reinterpret_cast<int*>(&dBuffer[4]);
        
        std::string rawName(dBuffer.begin() + 8, dBuffer.begin() + 16);
        std::string elementName = std::string(rawName.c_str());
        elementName = elementName.substr(0, elementName.find('\0'));


        if (elementName.find("_START") != std::string::npos) {
            std::string dirName = elementName.substr(0, elementName.find("_START"));
            Node* newDir = new Node(elementOffset, elementLength, dirName, 1);

            if (!currentFolder.empty()) {
                currentFolder.top()->children.push_back(newDir);
            } else {
                root->children.push_back(newDir);
            }

            currentFolder.push(newDir);
            continue;
        }

        if (elementName.find("_END") != std::string::npos) {
            if (!currentFolder.empty()) currentFolder.pop();
            else std::cerr << "Warning: Unexpected _END with empty stack" << std::endl;
            continue;
        }

        if (std::regex_match(elementName, pattern)) {
            Node* mapNode = new Node(elementOffset, elementLength, elementName, 2);
            if (!currentFolder.empty()) {
                currentFolder.top()->children.push_back(mapNode);
            } else {
                root->children.push_back(mapNode);
            }
            currentFolder.push(mapNode);
            currentMapMarker = true;
            nextTen = 0;
            continue;
        }

        Node* fileNode = new Node(elementOffset, elementLength, elementName, 0);

        if (currentMapMarker && !currentFolder.empty()) {
            currentFolder.top()->children.push_back(fileNode);
            nextTen++;
            if (nextTen == 10) {
                currentFolder.pop();
                currentMapMarker = false;
            }
        } else if (!currentFolder.empty()) {
            currentFolder.top()->children.push_back(fileNode);
        } else {
            root->children.push_back(fileNode);
        }
    }

    buildPathMap(root, "");


}


Wad* Wad::loadWad(const std::string &path) {

    return new Wad(path);
}
bool Wad::isContent(const std::string &path){

   if(pathMap.count(path) == 1 && pathMap[path]->isDirectory == 0){
    return true;
   }

    return false;
}

bool Wad::isDirectory(const std::string &path) {
    if (path == "/") return true;

    std::string adjustedPath = path;
    if (!adjustedPath.empty() && adjustedPath.back() != '/') {
        adjustedPath += '/';
    }


    if (pathMap.count(adjustedPath)) {
        int type = pathMap[adjustedPath]->isDirectory;
        return (type == 1 || type == 2);
    }

    return false;
}

void Wad::printTree(Node* node, int depth = 0) {
    if (!node) return;
    std::cout << std::string(depth * 2, ' ') << "- " << node->elementName << "\n";
    for (Node* child : node->children) {
        printTree(child, depth + 1);
    }
}

void Wad::printAll() {
        printTree(root);
}


int Wad::getSize(const std::string &path) {
    if(isContent(path)){
        return pathMap[path]->elementLength;
    }
    return -1;
}

int Wad::getContents(const std::string &path, char *buffer, int length, int offset){
    if(!isContent(path)) return -1;

    Node* node =  pathMap[path];
    int location = node->elementOffset;
    int size = node->elementLength;
    this->file.seekg(location + offset);

    if(offset >= size){
        return 0;
    }
    if (offset + length > size){
        length = size - offset;
    }

    if(length < 0) return 0;
    std::vector<char> tempBuffer(length);

    if (!this->file.read(tempBuffer.data(), length)) {
        return -1;
    }
    
    for (int i = 0; i < length; ++i) {
        buffer[i] = tempBuffer[i];
    }    
    return length;
}

std::string Wad::getMagic(){
    return Magic;
}

int Wad::getDirectory(const std::string &path, std::vector<std::string> *directory) {
    if (path == "/") {
        for (auto j : root->children) {
            directory->push_back(j->elementName);
        }
        return root->children.size();
    }

    std::string adjustedPath = path;
    if (!adjustedPath.empty() && adjustedPath.back() != '/') {
        adjustedPath += '/';
    }

    if (!pathMap.count(adjustedPath)) return -1;


    Node* dirNode = pathMap[adjustedPath];
    if (dirNode->isDirectory == 0 ) return -1;

    for (auto i : dirNode->children) {
        directory->push_back(i->elementName);
    }

    return dirNode->children.size();
}


void Wad::createDirectory(const std::string &path) {
    if (path.empty() || path == "/") return;

    std::string trimmedPath = path;
    if (trimmedPath.back() == '/') trimmedPath.pop_back();

    int lastSlash = trimmedPath.find_last_of('/');
    if (lastSlash == std::string::npos) return;

    std::string ParentDirectory = trimmedPath.substr(0, lastSlash + 1);
    std::string NewDirectory = trimmedPath.substr(lastSlash + 1);

    if (!ParentDirectory.empty() && ParentDirectory.back() != '/') {
        ParentDirectory += '/';
    }

    std::string newDirPath = ParentDirectory + NewDirectory + "/";

    if (!pathMap.count(ParentDirectory) || pathMap.count(newDirPath) || NewDirectory.length() > 2) {
        return;
    }

    Node* newDir = new Node(0, 0, NewDirectory, 1);
    pathMap[newDirPath] = newDir;
    pathMap[ParentDirectory]->children.push_back(newDir);

    std::string startName = NewDirectory + "_START";
    std::string endName = NewDirectory + "_END";

    int insertIndex = -1;

    if (ParentDirectory == "/") {
        insertIndex = descriptorCount;
    } else {
        std::stack<std::string> pathStack;
        std::string currentPath = "/";

        this->file.seekg(descriptorOffset);
        for (int i = 0; i < descriptorCount; ++i) {
            char entry[16];
            this->file.read(entry, 16);
            std::string name(entry + 8, entry + 16);
            name = std::string(name.c_str());

            if (name.find("_START") != std::string::npos) {
                std::string dirName = name.substr(0, name.find("_START"));
                currentPath += dirName + "/";
                pathStack.push(currentPath);
            } else if (name.find("_END") != std::string::npos) {
                std::string dirName = name.substr(0, name.find("_END"));
                if (!pathStack.empty()) {
                    std::string topPath = pathStack.top();
                    pathStack.pop();

                    if (topPath == ParentDirectory) {
                        insertIndex = i;
                        break;
                    }

                    size_t lastSlash = topPath.find_last_of('/', topPath.length() - 2);
                    if (lastSlash != std::string::npos) {
                        currentPath = topPath.substr(0, lastSlash + 1);
                    } else {
                        currentPath = "/";
                    }
                }
            }
        }
    }

    if (insertIndex == -1) {
        return;
    }

    int bytesToMove = (descriptorCount - insertIndex) * 16;
    std::vector<char> movedData(bytesToMove);
    this->file.seekg(descriptorOffset + insertIndex * 16);
    this->file.read(movedData.data(), bytesToMove);
    this->file.seekp(descriptorOffset + (insertIndex + 2) * 16);
    this->file.write(movedData.data(), bytesToMove);

    char startDescriptor[16] = {0};
    char endDescriptor[16] = {0};
    std::memcpy(startDescriptor + 8, startName.c_str(), std::min((size_t)8, startName.size()));
    std::memcpy(endDescriptor + 8, endName.c_str(), std::min((size_t)8, endName.size()));

    this->file.seekp(descriptorOffset + insertIndex * 16);
    this->file.write(startDescriptor, 16);
    this->file.write(endDescriptor, 16);

    descriptorCount += 2;
    this->file.seekp(4);
    this->file.write(reinterpret_cast<char*>(&descriptorCount), 4);

    pathMap.clear();
    buildPathMap(root, "");
}




void Wad::createFile(const std::string &path) {
    if (path.empty() || path == "/") return;

    std::string trimmedPath = path;
    if (trimmedPath.back() == '/') trimmedPath.pop_back();

    int lastSlash = trimmedPath.find_last_of('/');
    if (lastSlash == std::string::npos) return;

    std::string parentPath = trimmedPath.substr(0, lastSlash + 1);
    std::string fileName = trimmedPath.substr(lastSlash + 1);

    if (!parentPath.empty() && parentPath.back() != '/') parentPath += '/';

    std::string fullFilePath = parentPath + fileName;
    if (!pathMap.count(parentPath) || pathMap.count(fullFilePath) || fileName.size() > 8) return;

    if (fileName.find("_START") != std::string::npos ||
        fileName.find("_END") != std::string::npos ||
        std::regex_match(fileName, std::regex("^E[0-9]M[0-9]$"))) return;

    int insertIndex = -1;

    if (parentPath == "/") {
        insertIndex = descriptorCount;
    } else {
        std::stack<std::string> pathStack;
        std::string currentPath = "/";

        this->file.seekg(descriptorOffset);
        for (int i = 0; i < descriptorCount; ++i) {
            char entry[16];
            this->file.read(entry, 16);
            std::string name(entry + 8, entry + 16);
            name = std::string(name.c_str());


            if (name.find("_START") != std::string::npos) {
                std::string dirName = name.substr(0, name.find("_START"));
                currentPath += dirName + "/";
                pathStack.push(currentPath);
            } else if (name.find("_END") != std::string::npos) {
                std::string dirName = name.substr(0, name.find("_END"));

                if (!pathStack.empty()) {
                    std::string topPath = pathStack.top();
                    if (topPath == parentPath) {
                        insertIndex = i;
                        break;
                    }
                    pathStack.pop();

                    size_t lastSlash = topPath.find_last_of('/', topPath.length() - 2);
                    currentPath = (lastSlash != std::string::npos) ? topPath.substr(0, lastSlash + 1) : "/";
                }
            }
        }
    }

    if (insertIndex == -1) {
        return;
    }

    int bytesToMove = (descriptorCount - insertIndex) * 16;
    std::vector<char> movedData(bytesToMove);
    this->file.seekg(descriptorOffset + insertIndex * 16);
    this->file.read(movedData.data(), bytesToMove);
    this->file.seekp(descriptorOffset + (insertIndex + 1) * 16);
    this->file.write(movedData.data(), bytesToMove);

    char descriptor[16] = {0};
    int offset = 0, length = 0;
    std::memcpy(descriptor + 0, &offset, 4);
    std::memcpy(descriptor + 4, &length, 4);
    std::memcpy(descriptor + 8, fileName.c_str(), std::min(size_t(8), fileName.size()));
    this->file.seekp(descriptorOffset + insertIndex * 16);
    this->file.write(descriptor, 16);

    descriptorCount += 1;
    this->file.seekp(4);
    this->file.write(reinterpret_cast<char*>(&descriptorCount), 4);

    Node* fileNode = new Node(0, 0, fileName, 0);
    pathMap[fullFilePath] = fileNode;
    pathMap[parentPath]->children.push_back(fileNode);

    pathMap.clear();
    buildPathMap(root, "");
}


int Wad::writeToFile(const std::string &path, const char *buffer, int length, int offset) {
    if (!pathMap.count(path) || pathMap[path]->isDirectory != 0) return -1;

    Node* fileNode = pathMap[path];
    if (fileNode->elementLength > 0 || offset >= length) return 0;

    int newDataOffset = descriptorOffset;
    int newDescriptorOffset = descriptorOffset + length;
    int maxWritable = newDescriptorOffset - (newDataOffset + offset);
    int actualLength = std::min(length, maxWritable);
    if (actualLength <= 0) return 0;

    int bytesToMove = descriptorCount * 16;
    std::vector<char> descriptorData(bytesToMove);
    this->file.seekg(descriptorOffset);
    this->file.read(descriptorData.data(), bytesToMove);
    this->file.seekp(newDescriptorOffset);
    this->file.write(descriptorData.data(), bytesToMove);

    this->file.seekp(newDataOffset + offset);
    this->file.write(buffer, actualLength);

    fileNode->elementOffset = newDataOffset;
    fileNode->elementLength = actualLength;

    std::stack<std::string> pathStack;
    std::string currentPath = "/";
    this->file.seekg(newDescriptorOffset);

    for (int i = 0; i < descriptorCount; ++i) {
        int entryOffset = newDescriptorOffset + i * 16;
        char entry[16];
        this->file.seekg(entryOffset);
        this->file.read(entry, 16);

        std::string name(entry + 8, entry + 16);
        name = std::string(name.c_str());

        if (name.find("_START") != std::string::npos) {
            std::string dir = name.substr(0, name.find("_START"));
            currentPath += dir + "/";
            pathStack.push(currentPath);
        } else if (name.find("_END") != std::string::npos) {
            if (!pathStack.empty()) {
                std::string top = pathStack.top();
                pathStack.pop();
                size_t lastSlash = top.find_last_of('/', top.length() - 2);
                currentPath = (lastSlash != std::string::npos) ? top.substr(0, lastSlash + 1) : "/";
            }
        } else {
            std::string fullPath = currentPath + name;
            if (fullPath == path) {
                std::memcpy(entry, &fileNode->elementOffset, 4);
                std::memcpy(entry + 4, &fileNode->elementLength, 4);
                this->file.seekp(entryOffset);
                this->file.write(entry, 16);
                break;
            }
        }
    }

    this->file.seekp(8);
    this->file.write(reinterpret_cast<char*>(&newDescriptorOffset), 4);
    descriptorOffset = newDescriptorOffset;

    return actualLength;
}
