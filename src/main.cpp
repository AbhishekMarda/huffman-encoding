#include <iostream>
#include "encoder.h"
#include "node.h"
int main()
{
    Node n(0, 'a');
    std::cout << "Hello cmake!!!" << n.getVal() << std::endl;
}