#include <iostream>
#include <Windows.h>
#include "Bmsdata.h"

using namespace std;

int main(){
	Bmsdata data;
	LARGE_INTEGER sysfreq, ltime, lresult;

	cout << "load start" << endl;
	
	QueryPerformanceFrequency(&sysfreq);
	data.bmspath = "dive_air04FD.bms";
	QueryPerformanceCounter(&ltime);
	data.setbmsstring();
	QueryPerformanceCounter(&lresult);

	int notes = 0;
	for (int i = 11; i < 20; i++){
		notes += data.getsize(i);
	}

	cout << "title   : " << data.gettitle() << endl;
	cout << "notes   : " << notes << endl;
	cout << "level   : " << data.getplaylevel() << endl;
	cout << "calctime: " << (double)(lresult.QuadPart - ltime.QuadPart) / (double)sysfreq.QuadPart << "(sec)" << endl;
	return 0;
}