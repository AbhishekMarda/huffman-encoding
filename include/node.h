#pragma once
#include <memory>

class Node 
{
    unsigned int m_frequency = 0;
    char m_val = 0;
public:
    std::shared_ptr<Node> left = nullptr; 
    std::shared_ptr<Node> right = nullptr;
    Node(unsigned int frequency, char val = 0): m_frequency(frequency), m_val(val) {};
    Node(std::shared_ptr<Node> leftChild, std::shared_ptr<Node> rightChild);
    bool operator<(const Node& other) const;
    inline char getVal() const { return this->m_val; }
    inline void setVal(char c) noexcept { this->m_val = c; }
    inline bool isLeaf() const 
    { 
        return 
            (this->left == nullptr) && 
            (this->right == nullptr); 
    }
};