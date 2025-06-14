#pragma once
#include <memory>

class Node 
{
    unsigned int m_frequency = 0;
    char m_val = 0;
public:
    std::shared_ptr<Node> left; 
    std::shared_ptr<Node> right;
    Node(unsigned int frequency, char val = 0): m_frequency(frequency), m_val(val) {};
    Node(std::shared_ptr<Node>& leftChild, std::shared_ptr<Node>& rightChild);
    inline char getVal() const { return this->m_val; }
    bool operator<(const Node& other) const;
};