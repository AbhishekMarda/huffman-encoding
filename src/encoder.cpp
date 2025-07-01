#include <istream>
#include <ostream>
#include <iostream>
#include <queue>
#include <algorithm>
#include <string>

#include "encoder.h"

Encoder::Encoder(std::istream& input) : m_input(input)
{
    char buf;
    while(input.get(buf))
    {
        m_charFrequency[static_cast<unsigned char>(buf)]++;
    }
}

std::shared_ptr<Node> Encoder::createHuffmanTree()
{
    auto compareNodes = [](const std::shared_ptr<Node>& l, const std::shared_ptr<Node>& r) {
        return !((*l) < (*r));
    };

    std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, decltype(compareNodes)> orderedNodes(compareNodes);

    for(char i = 0; i < CHAR_SIZE; i++)
    {
        if (m_charFrequency[i])
            orderedNodes.push(std::make_shared<Node>(m_charFrequency[i], i));
    }
    if (orderedNodes.size() == 0) 
        return nullptr;

    if (orderedNodes.size() == 1)
    {
        // special case where a parent does not get created, 
        // thus the single encoding has 0 characters without this code block
        std::shared_ptr<Node> dummy = std::make_shared<Node>(orderedNodes.top(), nullptr);
        return dummy;
    }

    // take the least frequent two, remove them, create a parent, reinsert
    // using stl heap means that we use a balancing tree
    // balancing tree is important because we want to avoid the tree becoming a linked list
    // that would cause encodings to be large, even when smaller encodings would be available
    while(orderedNodes.size() > 1)
    {
        std::shared_ptr<Node> low1 = orderedNodes.top();
        orderedNodes.pop();
        std::shared_ptr<Node> low2 = orderedNodes.top();
        orderedNodes.pop();

        auto parent = std::make_shared<Node>(low1, low2);

        orderedNodes.push(parent);
    }

    return orderedNodes.top();
}

void Encoder::storeEncodingMap(std::shared_ptr<Node> currNode, std::vector<bool>& currEncoding)
{
    if (currNode == nullptr) 
        return;

    char currChar = currNode->getVal();

    if (currNode->isLeaf())
        m_encoding[currChar] = currEncoding;

    if (currNode->left != nullptr)
    {
        currEncoding.push_back(false);
        storeEncodingMap(currNode->left, currEncoding);
        currEncoding.pop_back();
    }
    
    if(currNode->right != nullptr)
    {
        currEncoding.push_back(true);
        storeEncodingMap(currNode->right, currEncoding);
        currEncoding.pop_back();
    }
}

void Encoder::storeEncodingMap(std::shared_ptr<Node> huffmanRoot)
{
    std::vector<bool> encoding;
    storeEncodingMap(huffmanRoot, encoding);
}

void Encoder::encode()
{
    auto root = createHuffmanTree();
    storeEncodingMap(root);
}

void Encoder::writePrelogue(std::ostream& output)
{
    // reserve 5 bytes to store number of bytes written in prelogue. 4 bytes for integer, one byte for newline.
    // reserve 2 bytes to store number of bits to read in the last byte. One byte for size, one byte for newline

    output.write(std::string(7, '\0').c_str(), 7);
    
    for(unsigned short c = 0; c < CHAR_SIZE; c++)
    {
        /*
        An encoding in the prelogue is represented as
        cNe...e

        Here, 
        c = the ASCII character that has been encoded
        N = a byte holding the number of bits that were flushed in. We need this because 
            we can only write at an 8 bit (char) level granularity to a file. Byte is to
            be taken at raw value (0-255)
        e = chars containing encoding (and optional padding at end as a result of byte granularity)
        */
        if (m_charFrequency[c])
        {
            output.put(c);
            m_prelogueBytes++;
            // this condition should never be true, given that the maximum possible encoding can be 255 bits long, at most
            if (m_encoding[static_cast<size_t>(c)].size() >= CHAR_SIZE)
            {
                throw std::runtime_error("Size of character " + std::to_string(c) + " >= " + std::to_string(CHAR_SIZE) + " bytes, and can't be stored.");
            }
            output.put(static_cast<char>(m_encoding[static_cast<size_t>(c)].size()));
            m_prelogueBytes++;
            int val = 0;
            val += writeEncoding(c, output, true);
            m_prelogueBytes += val;
            val += 2;
            std::cout << "Skipping ahead " << val << " bytes." << std::endl;
        }
    }
}

void Encoder::finishWrite(std::ostream& output)
{
    // use the 2 bytes that were reserved previously
    output.seekp(std::ios::beg);
    output.put(m_lastByteBits);
    output.put('\n');

    // use the 5 bytes the were reserved 
    for(int i=24; i >= 0; i-=8)
    {
        char byte = static_cast<char>((m_prelogueBytes >> i) & 0xFF);
        output.put(byte);
    }
    output.put('\n');
}

void Encoder::write(std::ostream& output)
{
    m_input.clear();
    m_input.seekg(0, std::ios::beg);

    char inputBuf;
    char outputBuf;
    writePrelogue(output);
    while(m_input.get(inputBuf))
    {
        writeEncoding(inputBuf, output);
    }
    flush(output);
    finishWrite(output);
}

int Encoder::writeEncoding(char c, std::ostream& output, bool flushUponComplete)
{
    int writtenBytes = 0;
    const std::vector<bool>& encoding = m_encoding[static_cast<unsigned char>(c)];

    // FUTURE: optimize to operate on many bits at once
    for(bool value : encoding)
    {
        char mask = (static_cast<char>(value) << m_outputSeek);
        m_outputBuf |= mask;
        m_outputSeek--;
        if (m_outputSeek < 0)
        {
            flush(output);
            writtenBytes++;
        }
    }

    if (flushUponComplete)
    {
        if (flush(output))
            writtenBytes++;
    }
    return writtenBytes;
}

inline bool Encoder::flush(std::ostream& output)
{
    if (m_outputSeek == 7)
        return false; // nothing to flush
    if (m_outputSeek > 7)
        throw std::runtime_error("Output seek detected to be larger than available bits in a byte. Aborting.");
    // flush to stream buffer
    output.put(m_outputBuf);
    
    // reset
    m_lastByteBits = 7 - m_outputSeek;
    m_outputSeek = 7; // num bits in a byte (8) - 1
    m_outputBuf = 0;
    return true;
}