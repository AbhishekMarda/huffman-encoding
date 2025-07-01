#include <iostream>
#include <unistd.h>
#include <string>
#include <istream>
#include <ostream>
#include <fstream>
#include "encoder.h"
#include "decoder.h"
#include "node.h"

int main(int argc, char* argv[])
{

    bool encode = false;
    bool decode = false;
    std::string outputFile;
    std::string inputFile;

    int opt;
    while ((opt = getopt(argc, argv, "edo:i:")) != -1) 
    {
        switch (opt) {
            case 'e':
                encode = true;
                break;
            case 'd':
                decode = true;
                break;
            case 'o':
                outputFile = optarg;
                outputFile.erase(remove(outputFile.begin(), outputFile.end(), ' '), inputFile.end());
                break;
            case 'i':
                inputFile = optarg;
                inputFile.erase(remove(inputFile.begin(), inputFile.end(), ' '), inputFile.end());
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-e] [-d] [-o output_file] [-i input_file]\n";
                return 1;
        }
    }

    if (encode && decode) 
    {
        std::cerr << "Cannot specify both -e (encode) and -d (decode)\n";
        return 1;
    }


    std::ifstream input(inputFile);
    if (!input.is_open()) {
        std::cerr << "Failed to open input file: " << inputFile << std::endl;
        return 1;
    }

    std::ofstream output(outputFile);
    if (!output.is_open()) {
        std::cerr << "Failed to open output file: " << outputFile << std::endl;
        return 1;
    }
    try 
    {
        if (encode)
        {
            std::cout << "Encoding " << inputFile << " into " << outputFile << std::endl;
            Encoder e(input);
            e.encode();
            e.write(output);
        }
        else 
        {
            Decoder d(input);
            d.write(output);
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        std::terminate();
    }
}