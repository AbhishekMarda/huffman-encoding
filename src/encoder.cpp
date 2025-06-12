#include <istream>
#include <ostream>
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

void Encoder::storeEncodingMap(std::shared_ptr<Node> currNode, unsigned int currSize, unsigned int currEncoding)
{
    if (currNode == nullptr) 
        return;

    char currChar = currNode->getVal();

    m_encoding[currChar][0] = currSize;
    m_encoding[currChar][1] = currEncoding;

    if (currNode->left != nullptr)
    {
        storeEncodingMap(currNode->left, currSize + 1, (currEncoding << 1) | 0);
    }
    
    if(currNode->right != nullptr)
    {
        storeEncodingMap(currNode->right, currSize + 1, (currEncoding << 1) | 1);
    }
}

void Encoder::storeEncodingMap(std::shared_ptr<Node> huffmanRoot)
{
    storeEncodingMap(huffmanRoot, 0, 0);
}

void Encoder::encode()
{
    auto root = createHuffmanTree();
    storeEncodingMap(root);
}

void Encoder::writePrelogue(std::ostream& output)
{
    // reserve 8 bytes to store number of bytes in encoding. 8 bytes is more than the maximum possible size in base 2
    // that can be stored in a single file. This assumes that the size can be put in size_t, which supports upto 2^64
    
    // reserve 1 more byte to write the number of bits that are used by the last byte
    // lastly, reserve 2 bytes for newlines
    
    output.write(std::string(11, '\0').c_str(), 11);
    output.put('\n');
    
    for(unsigned short c = 0; c < CHAR_SIZE; c++)
    {
        /*
        A line output in prelogue would look as follows
        cNe...e\n

        Here, 
        c = the ASCII character that has been encoded
        N = a byte holding the number of bits that were flushed in. We need this because 
            we can only write at an 8 bit (char) level granularity to a file. Byte is to
            be taken at raw value (0-255)
        e = chars containing encoding (and optional padding as a result of byte granularity)
        */
        if (m_charFrequency[c])
        {
            output.put(c);

            output.put(static_cast<char>(m_encoding[static_cast<size_t>(c)].size()));
            
            writeEncoding(c, output);
            flush(output, false);
            output.put('\n');
        }
    }
    output.put('\n');
}

void Encoder::finishWrite(std::ostream& output)
{
    // use the 11 bytes that were reserved previously
    output.seekp(std::ios::beg);
    
    // individually write every byte inside size_t. 
    // a valid point is that at this point, I'm strongly assuming size_t == 64 bits
    // which is NOT the point of using a templated data type
    // but, for MVP, doing this is fine. 
    // FUTURE: dynamically change this behavior at compile time based on size of size_t

    for (int shift = 56; shift >=0; shift -= 8)
        output.put(static_cast<char>((m_charsWritten >> shift) & 0xFF));

    output.put('\n');
    output.put(m_lastByteBits);
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

void Encoder::writeEncoding(char c, std::ostream& output)
{
    const std::vector<bool>& encoding = m_encoding[static_cast<size_t>(c)];

    // FUTURE: optimize to operate on many bits at once
    for(size_t i = 0; i < encoding.size(); i++)
    {
        m_outputBuf <<= 1;
        m_outputBuf |= encoding[i];
        m_outputSeek++;
        if (m_outputSeek >= 8 /* Number of bits in a byte */)
        {
            flush(output);
        }
    }
}

inline void Encoder::flush(std::ostream& output, bool updateCharCount)
{
    if (m_outputSeek == 0)
        return; // nothing to flush

    // flush to stream buffer
    output.put(m_outputBuf);
    
    if (updateCharCount)
        m_charsWritten++;
    // reset
    m_lastByteBits = m_outputSeek;
    m_outputSeek = 0;
    m_outputBuf = 0;
}