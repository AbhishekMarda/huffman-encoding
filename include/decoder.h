#pragma once
#include "node.h"

#include <iostream>
#include <ostream>
#include <limits>
#include <memory>
#include <vector>

class Decoder
{
    static constexpr size_t CHAR_SIZE = std::numeric_limits<unsigned char>::max() + 1;

    std::istream& m_input;
    std::vector<bool> m_encoding;

    size_t m_charsToRead = 0;
    unsigned char m_lastByteBits = 0;

    std::shared_ptr<Node> m_root = std::make_shared<Node>(0);    // dummy

    void readPrelogue();
    void readEncodingMaps(unsigned int encodingBytes);
    void addEncodingToTree(char c, unsigned char size, std::string_view& encoding);

public:
    Decoder(std::istream& input): m_input(input) {}
    void write(std::ostream& output);
};