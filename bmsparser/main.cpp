#include <iostream>
#include "Bmsdata.h"

using namespace std;

int main(){
	Bmsdata data;

	data.bmspath = "dive_air04FD.bms";
	data.setbmsstring();
	
	int notes = 0;
	for (int i = 11; i < 20; i++){
		notes += data.getsize(i);
	}

	cout << "ƒm[ƒg”‚Í " << notes <<endl;

	return 0;
}