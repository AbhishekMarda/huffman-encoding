#include <iostream>
#include "decoder.h"

void Decoder::readPrelogue()
{
    std::string inputLine;

    // get the number of bits used in the last byte
    std::getline(m_input, inputLine);
    m_lastByteBits = static_cast<unsigned char>(inputLine[0]);

    unsigned int prelogueBytes = 0;
    unsigned int sizeBytes = 0;
    std::getline(m_input, inputLine);
    for(int i = 24; i >= 0; i-=8, sizeBytes++)
    {
        unsigned int byte = static_cast<unsigned int>(static_cast<unsigned char>(inputLine[sizeBytes]));
        prelogueBytes |= (byte << i);
    }
    
    readEncodingMaps(prelogueBytes);
}

void Decoder::readEncodingMaps(unsigned int encodingBytes)
{
    unsigned int encodingBytesRead = 0;
    while(encodingBytesRead < encodingBytes)
    {
        // read character
        char cBuf;
        m_input.get(cBuf);

        // read size
        char sizeBuf;
        m_input.get(sizeBuf);
        unsigned char encodingSize = static_cast<unsigned char>(sizeBuf);

        unsigned char bytesInEncoding = encodingSize / 8 + static_cast<unsigned char>((encodingSize % 8 != 0));

        // read and store encoding
        // must use C code. sad. no RAII.
        char* encoding = new char[bytesInEncoding];
        m_input.read(encoding, bytesInEncoding);
        std::string_view encodingStr(encoding, bytesInEncoding);

        // add the encoding to the huffman tree
        addEncodingToTree(cBuf, encodingSize, encodingStr);
        delete [] encoding;

        encodingBytesRead += 2 + bytesInEncoding;
    }
}

void Decoder::addEncodingToTree(char c, unsigned char size, std::string_view& encoding)
{
    short bits = 0;

    std::shared_ptr<Node> curr = m_root;

    for(size_t i = 0; i < encoding.size() && bits < size; i++)
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
            if (curr == nullptr)
            {
                output.flush();
                throw std::runtime_error(
                    "Pointer in the tree ended up null. Should not have happened. Byte: " + 
                    std::to_string(static_cast<unsigned int>(buf)) + 
                    ". Position: " +
                    std::to_string(m_input.tellg())
                );
            }
            if (curr->isLeaf())
            {
                output.put(curr->getVal());
                curr = m_root;
            }
        }
    }
}