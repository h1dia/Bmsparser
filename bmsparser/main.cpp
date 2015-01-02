#include <iostream>
#include <Windows.h>
#include "Bmsdata.h"

using namespace std;

int main(){
	Bmsdata data("dive_air04FD.bms");
	LARGE_INTEGER sysfreq, ltime, lresult;

	cout << "load start" << endl;
	
	QueryPerformanceFrequency(&sysfreq);
	QueryPerformanceCounter(&ltime);
	data.setbmsstring();
	QueryPerformanceCounter(&lresult);

	int notes = 0;
	for (int i = 11; i < 20; i++){
		notes += data.getsize(i);
	}

	cout << "title   : " << data.get_header_s("TITLE") << endl;
	cout << "notes   : " << notes << endl;
	cout << "level   : " << data.get_header_s("PLAYLEVEL") << endl;
	cout << "errtest : " << data.get_header_s("ERRTEST") << endl;
	cout << "calctime: " << (double)(lresult.QuadPart - ltime.QuadPart) / (double)sysfreq.QuadPart << "(sec)" << endl;

	return 0;
}
