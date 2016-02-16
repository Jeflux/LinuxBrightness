/*
 * No arguments display current brightness level in percentage
 * usage: +[percentage] raises brightness by percentage
 *        -[percentage] lowers brightness by percentage
 *         [percentage] sets brightness to percentage
 *
 * example: brightness +10 - Raises brightness by 10%
 *
 */


#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <algorithm>

enum OP { INC, DEC, SET, UNDEF };

std::string BP = "/sys/class/backlight/intel_backlight/brightness";
std::string MAXBP = "/sys/class/backlight/intel_backlight/max_brightness";
int BMAX = 0;
int BCURRENT = 0;

std::string runCommand(const std::string &command) {
    FILE *fp;
    char buff[512];
    std::string ret = "";

    fp = popen(command.c_str(), "r");
    if (fp == NULL) {
        return NULL;
    }

    while (fgets(buff, sizeof(buff), fp) != NULL) {
        ret = std::string(buff);
    }

    pclose(fp);

    return ret;
}

OP checkOP(const std::string& s) {
    std::string::const_iterator it = s.begin();
    OP op = OP::SET;

    if (s[0] == '+') {
        op = OP::INC;
        it++;
    }
    else if (s[0] == '-') {
        op = OP::DEC;
        it++;
    }

    while (it != s.end() && std::isdigit(*it)) ++it;
    if (!(!s.empty() && it == s.end()))
        op = OP::UNDEF;

    return op;
}

void setPercentage(float perc) {
    // Get root access and set brightness
    std::string brightnessCommand = "echo ";
    brightnessCommand += std::to_string(std::max(1, std::min(BMAX, (int) (perc * BMAX))));
    brightnessCommand += " > ";
    brightnessCommand += BP;

    setreuid(geteuid(), getuid());
    system(brightnessCommand.c_str()); 
    setreuid(geteuid(), getuid());
}

void increasePercentage(float perc) {
    int now = atoi(runCommand("cat " + BP).c_str());
    setPercentage(((float) now / (float) BMAX) + perc);
}

int main(int argc, char* argv[]) {
    BMAX = atoi(runCommand("cat " + MAXBP).c_str());

    if (argc == 2) {
        OP op = checkOP(argv[1]);
        switch (op) {
            case OP::SET:
                setPercentage(std::stoi(argv[1]) / 100.0f);
                break;
            case OP::INC:
                increasePercentage(std::stoi(argv[1]) / 100.0f);
                break;
            case OP::DEC:
                increasePercentage(std::stoi(argv[1]) / 100.0f);
                break;
        }
        return 0;
    }    
    
    std::cout << std::stof(runCommand("cat " + BP)) / BMAX * 100 << std::endl;
    
    return 0;
}
