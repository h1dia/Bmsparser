#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <list>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <cctype>

class Bmsdata{
public:
	Bmsdata();
	~Bmsdata();

	void setbmspath(std::string bmspath);
	void setbmsstring();
	void loadheader();

	std::string get_headder_s(std::string command);
	double get_headder_d(std::string command);
	std::string gettitle();
	int getplaylevel();
	int getsize(int channel);

	std::string bmspath;

private:
	static const int CHANNEL_ELEMENTS = 575;
	const char* kNotAvailable = "<N/A>";

	struct HEADER{
		std::string command;
		std::string str;
		bool val;
	};

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

	void parse_random(std::vector<std::string>& temp_array, unsigned int from, unsigned int length);
	unsigned int find_endif(std::vector<std::string>& temp_array, unsigned int index);
	void header_analysis(std::vector<std::string>& header_array);
	int base_stoi(int base, std::string num);
	void normalize_data(std::vector<std::string>& data_array, std::vector<DATA> & normalized_array);
	int random(int max);

	static bool starts_with(std::string& str, std::string substr);
	bool judge_disused_command(std::string command);

	std::vector<CHANNEL> channel_data_array[CHANNEL_ELEMENTS];
	std::vector<DATA> bmp_path_array, wav_path_array;
	std::vector<HEADER> header_list;
	int random_count;
	int rank;
	int total;
	int playlevel;
	bool load_fail;
	std::string bmsfolder;
	std::string banner;
	std::string title;
	std::mt19937 random_engine;
};
