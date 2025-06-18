#include <iostream>
#include "decoder.h"

void Decoder::readPrelogue()
{
    std::string inputLine;

    // get the number of bits used in the last byte
    std::getline(m_input, inputLine);
    m_lastByteBits = static_cast<unsigned char>(inputLine[0]);


    // start reading the encoding
    while(std::getline(m_input, inputLine) && !inputLine.empty())
    {
        addEncodingToTree(inputLine);
    }
}

void Decoder::addEncodingToTree(std::string& encoding)
{
    char c = encoding[0];
    unsigned char size = static_cast<unsigned char>(encoding[1]);
    short bits = 0;

    std::shared_ptr<Node> curr = m_root;

    for(size_t i = 2; i < encoding.size() && bits < size; i++)
    {
        char currByte = encoding[i];
        for(int shift = 7; shift >= 0 && bits < size; shift--, bits++)
        {
            bool direction = (currByte >> shift) & 0x1;
            if (direction)
            {
                if(!curr->right) curr->right = std::make_shared<Node>(0);
                curr = curr->right;
            }
            else 
            {
                if (!curr->left) curr->left = std::make_shared<Node>(0);
                curr = curr->left;
            }
        }
    }
    curr->setVal(c);
}

void Decoder::write(std::ostream& output)
{
    // file pointer will be at the start of the encoded bits now
    readPrelogue();
    char buf;
    std::shared_ptr<Node> curr = m_root;
    while(m_input.get(buf))
    {
        unsigned char numBits = 8;
        if (m_input.peek() == EOF)
        {
            numBits = m_lastByteBits;
        }
        for(int shift = 7; shift >= 0 && numBits > 0; shift--, numBits--)
        {
            bool direction = (buf >> shift) & 0x1;
            if (direction)
                curr = curr->right;
            else
                curr = curr->left;
            
            if (curr->isLeaf())
            {
                output.put(curr->getVal());
                curr = m_root;
            }
        }
    }
}