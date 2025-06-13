#pragma once
#include "node.h"

#include <istream>
#include <ostream>
#include <limits>
#include <memory>
#include <vector>

class Encoder
{
    static constexpr size_t CHAR_SIZE = std::numeric_limits<unsigned char>::max() + 1;
    unsigned int m_charFrequency[CHAR_SIZE] = {0};

    // to buffer encodings
    unsigned char m_outputSeek = 0;
    unsigned char m_lastByteBits = 0;
    // TODO: make a class from output buffer
    // that would allow the output buffer size
    // to be very variable
    // for now, char is fine as it can just write into the output stream
    char m_outputBuf = 0; 
    size_t m_charsWritten = 0;

    std::vector<bool> m_encoding[CHAR_SIZE];
    std::istream& m_input;

    std::shared_ptr<Node> createHuffmanTree();
    void storeEncodingMap(std::shared_ptr<Node> currNode, std::vector<bool>& currEncoding);
    void storeEncodingMap(std::shared_ptr<Node> huffmanRoot);
    void writePrelogue(std::ostream& output);
    void finishWrite(std::ostream& output);

    void writeEncoding(char c, std::ostream& output);
    inline void flush(std::ostream& output, bool updateCharCount = true);
public: 
    Encoder(std::istream& input);
    void encode();
    void write(std::ostream& output);
};