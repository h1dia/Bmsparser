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
	Bmsdata(std::string filepath);
	~Bmsdata();

	void setbmsstring();
	void loadheader();

	std::string search_header_s(std::string command);
	double search_header_d(std::string command);
	int getsize(int channel);


private:
	std::string filename;
	std::string filefolder;
	static const int MAX_ID = 1296;
	static const int MAX_CHANNEL = 575;
	const char* kNotAvailable = "<N/A>";
	const std::string num_check = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	struct HEADER{
		std::string command;
		std::string str;
		bool val;
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
	void normalize_data(std::vector<std::string>& data_array, std::vector<std::string> & normalized_array);
	int random(int max);

	static bool starts_with(std::string& str, std::string substr);
	bool judge_disused_command(std::string command);

	std::vector<std::vector<CHANNEL>> channel_data_array;
	std::vector<std::string> bmp_path_array, wav_path_array;
	std::vector<HEADER> header_list;
	bool load_fail;
	bool control_flow_exsist;
	std::string bmsfolder;
	std::string banner;
	std::string title;
	std::mt19937 random_engine;
};
