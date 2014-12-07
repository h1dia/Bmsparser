#pragma once
#include <iostream>
#include <string>
#include <vector>

class Bmsdata{
public:
	struct RETURN_DATA{
		std::string path;
		double position;
	};

	Bmsdata();
	~Bmsdata();

	void setbmspath(std::string bmspath);
	void setbmsstring();
	void loadheadder();

	std::string gettitle();
	int getsize(int channel);
	RETURN_DATA getmaindata(int channel, int position);

	std::string bmspath;

private:
	struct RANDOM{
		std::vector<std::string> string;
		int	id;
	};

	struct DATA{
		std::string path;
		unsigned short int id;
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

	void parse_random(int start_random_array, std::vector<std::string> random_array);

	int base_stoi(int base, std::string num);
	bool asc(const MAIN&, const MAIN&);

	static const int CHANNEL_ELEMENTS = 575;
	std::vector<MAIN> main_data_array[CHANNEL_ELEMENTS];
	std::vector<std::string> headder_array, main_array, wav_array, bmp_array;
	std::vector<RANDOM> random_array;
	std::vector<DATA> bmp_path_array, wav_path_array;
	std::vector<std::string> data;

	int random_count;
	std::string bmsfolder;
	std::string title;
};