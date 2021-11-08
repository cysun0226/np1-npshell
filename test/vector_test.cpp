#include<iostream>
#include<vector>

std::vector<int*> arrays;

void push_test(){
    int *a1 = new int(2);
    int *a2 = new int(2);
    a1[0] = 0;
    a1[1] = 1;
    a2[0] = 2;
    a2[1] = 3;
    arrays.push_back(a1);
    arrays.push_back(a2);
}

int main()
{
    push_test();
    std::cout << arrays[0][0] << " " << arrays[0][1] << std::endl;
    std::cout << arrays[1][0] << " " << arrays[1][1] << std::endl;

    delete arrays.back();
    arrays.pop_back();
    delete arrays.back();
    arrays.pop_back();
    return 0;
}
