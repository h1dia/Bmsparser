#pragma once
#include <iostream>
#include <string>
#include <vector>

class Bmsdata{
public:
	Bmsdata();
	~Bmsdata();

	void setbmspath(std::string bmspath);
	void setbmsstring();
	void loadheader();

	std::string gettitle();
	int getplaylevel();
	int getsize(int channel);

	std::string bmspath;

private:
	static const int CHANNEL_ELEMENTS = 575;

	struct DATA{
		std::string path;
		unsigned short int id;

		bool operator<(const DATA& next) const	//ソート用のオペレータです。
		{
			return id < next.id;
		}
	};

	struct MAIN{
		unsigned short int measure;
		unsigned int step;			// 位置を示します。 ex.{1}/4
		unsigned int resolution;	// n/rhythm ex.4/{4}
		unsigned short int id;
		double num;

		bool operator<(const MAIN& next) const	//ソート用のオペレータです。
		{
			return measure == next.measure ? (double)step / resolution < (double)next.step / next.resolution : measure < next.measure;
		}
	};

	void header_analysis(std::vector<std::string> headder_array);
	int base_stoi(int base, std::string num);

	std::vector<MAIN> main_data_array[CHANNEL_ELEMENTS];
	std::vector<std::string> header_array, main_array, wav_array, bmp_array;
	std::vector<DATA> bmp_path_array, wav_path_array;
	std::vector<std::string> data;

	int random_count;
	int rank;
	int total;
	int playlevel;
	std::string bmsfolder;
	std::string banner;
	std::string title;
};