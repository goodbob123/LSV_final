#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

// find the number of AND gate from print_state
// 現在是參數吃檔案名稱, 爬裡面的and gate數量再print出來
int main(int argc, char** argv) {
    if (argc == 1)
        cout << "please enter the file name" << endl;
    else {
        ifstream file(argv[1]);
        if (!file.is_open()) {
            cerr << "Unable to open the file" << endl;
            return 1;
        }

        string line;
        int line_num = 0;
        while (getline(file, line)) {
            istringstream iss(line);
            string token;
            string buff;
            while (iss >> token) {
                if (token == "and") {
                    iss >> buff;
                    if (iss >> token) {
                        cout << "[" << line_num << "] and = " << token << endl;
                        line_num++;
                    } else {
                        cerr << "Error: No number after 'and ='" << endl;
                    }
                }
            }
        }

        file.close();
    }

    return 0;
}