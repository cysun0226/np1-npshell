#include <iostream>
#include <cstdlib>

int main(){
  char* ptr;
  ptr = getenv ("PATH");
  std::cout << "Original PATH" << std::endl;
  std::cout << ptr << std::endl;
  char new_path[] = "PATH=./bin:.";
  putenv(new_path);
  ptr = getenv ("PATH");
  std::cout << "Current PATH" << std::endl;
  std::cout << ptr << std::endl;
  return 0;
}

