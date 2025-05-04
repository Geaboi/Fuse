#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

struct Node{
    int elementOffset;
    int elementLength;
    std::string elementName;
    int isDirectory;
    Node(int elementOffset, int elementLength, std::string elementName, int Directory){
        this->elementOffset = elementOffset;
        this->elementLength = elementLength;
        this->elementName = elementName;
        isDirectory = Directory;
    }
    std::vector<Node*> children;
};

class Wad {
public:
    static Wad* loadWad(const std::string &path);
    bool isContent(const std::string &path);
    void printTree(Node* node, int depth);
    void printAll();
    void buildPathMap(Node* node, const std::string& currentPath);
    bool isDirectory(const std::string &path);
    int getSize(const std::string &path);
    int getContents(const std::string &path, char *buffer, int length, int offset = 0);
    int getDirectory(const std::string &path, std::vector<std::string> *directory);
    void createDirectory(const std::string &path);
    void createFile(const std::string &path);
    int writeToFile(const std::string &path, const char *buffer, int length, int offset = 0);
    std::string getMagic();
private:
    Wad(const std::string &path);
    int descriptorCount;
    int descriptorOffset; 
    std::string Magic;
    Node* root;
    std::fstream file;
    std::unordered_map<std::string, Node*> pathMap;

};