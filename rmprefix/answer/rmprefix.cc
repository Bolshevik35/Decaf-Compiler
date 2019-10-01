#include <iostream>
#include <string>
#include <cstdlib>

int main() {
    for (std::string line; std::getline(std::cin, line);) {
    	int len = line.length();
    	int i = 0;
    	for (; i < len; i++){
    		 
    		if(line[i] != '\t' && line[i] != ' ')
    			break;
    	}
	line.erase(0,i);
        std::cout << line <<std::endl;
    }
	exit(EXIT_SUCCESS);
}
