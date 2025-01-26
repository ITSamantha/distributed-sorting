#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <sstream>
#include <chrono>

void runApplication(std::string command, int delaySeconds = 0) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (delaySeconds > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(delaySeconds));
    }

    std::cout << "Executing command: " << command << std::endl;

    int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Command failed with exit code: " << result << std::endl;
    }
}

void merge(int a, int b) {

    std::stringstream command1_stream, command2_stream;
    command1_stream << "./main " << a << " " << b << " 1";
    command2_stream << "./main " << b << " " << a << " 0";

    std::string command1 = command1_stream.str();
    std::string command2 = command2_stream.str();

    std::thread thread1(runApplication, command1, 0);
    std::thread thread2(runApplication, command2, 1); 

    thread1.join();
    thread2.join();
}

void merge(int a, int b, int c, int d) {
    merge(a, c);
    merge(b, d);
    merge(b, c);
}

void merge(int a, int b, int c, int d, int e, int f, int g, int h) {
    merge(a, b, e, f);
    merge(c, d, g, h);
    merge(c, d, e, f);
}


int main() {

    merge(1, 2);
    merge(3, 4);
    merge(5, 6);
    merge(7, 8);
    
    merge(1, 2, 3, 4);
    merge(5, 6, 7, 8);

    merge(1, 2, 3, 4, 5, 6, 7, 8);

    return 0;
}
