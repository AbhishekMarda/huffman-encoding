#include "node.h"
#include <memory>

Node::Node(std::shared_ptr<Node> leftChild, std::shared_ptr<Node> rightChild)
{
    this->m_frequency = leftChild->m_frequency + rightChild->m_frequency;
    this->left = leftChild;
    this->right = rightChild;
}

bool Node::operator<(const Node& other) const
{
    return this->m_frequency < other.m_frequency;
}