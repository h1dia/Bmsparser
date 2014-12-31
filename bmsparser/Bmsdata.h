#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <random>

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

	struct CHANNEL{
		unsigned short int measure;
		unsigned int step;			// 位置を示します。 ex.{1}/4
		unsigned int resolution;	// n/rhythm ex.4/{4}
		unsigned short int id;
		double num;

		bool operator<(const CHANNEL& next) const	//ソート用のオペレータです。
		{
			return measure == next.measure ? (double)step / resolution < (double)next.step / next.resolution : measure < next.measure;
		}
	};

	unsigned int parse_random(std::vector<std::string>& temp_array, unsigned int from, unsigned int length);
	void clean_random(std::vector<std::string>& temp_array);
	unsigned int find_endif(std::vector<std::string>& temp_array, unsigned int index);
	void header_analysis(std::vector<std::string>& header_array);
	int base_stoi(int base, std::string num);
	int random(int max);

	static bool starts_with(std::string& str, std::string substr);

	std::vector<CHANNEL> channel_data_array[CHANNEL_ELEMENTS];
	std::vector<std::string> header_array, channel_array, wav_array, bmp_array;
	std::vector<DATA> bmp_path_array, wav_path_array;
	std::vector<std::string> data;

	int random_count;
	int rank;
	int total;
	int playlevel;
	std::string bmsfolder;
	std::string banner;
	std::string title;
	std::mt19937 random_engine;
};
