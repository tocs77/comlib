#include <iostream>

#include "CommAsync/CommPortAsync.h"
//#include "CommAsync/CommPortAsync.cpp"


int main() {

	TCommPortAsync tCom;
	tCom.setBaudrate(baudrates::BR_115200);
	tCom.setPort(14);
	tCom.Connect();
	std::cout << "Hello! " << std::endl;
}
