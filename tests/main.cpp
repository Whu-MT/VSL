#include <iostream>

extern "C"
{
    int sum(int, int);
}

int main(int argc, char const *argv[])
{
    std::cout<<"sum of 3 and 4 is: "<<sum(3, 4)<<std::endl;
    return 0;
}
