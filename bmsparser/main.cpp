#include <iostream>
#include <chrono>
#include "Bmsdata.h"

using namespace std;

int main(){
	Bmsdata data("dive_air04FD.bms");


	cout << "load start" << endl;
	
	auto start = chrono::system_clock::now();
	data.setbmsstring();
	auto end = chrono::system_clock::now();

	auto diff = end - start;

	int notes = 0;
	for (int i = 11; i < 20; i++){
		notes += data.getsize(i);
	}

	cout << "title   : " << data.search_header_s("TITLE") << endl;
	cout << "notes   : " << notes << endl;
	cout << "level   : " << data.search_header_s("PLAYLEVEL") << endl;
	cout << "errtest : " << data.search_header_s("ERRTEST") << endl;
	cout << "calctime: " << chrono::duration_cast<std::chrono::milliseconds>(diff).count() << "(ms)" << endl;

	return 0;
}
