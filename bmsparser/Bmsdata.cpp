#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "Bmsdata.h"
#include <cmath>

Bmsdata::Bmsdata(){
}

Bmsdata::~Bmsdata(){
}

void Bmsdata::setbmspath(std::string setbmspath){
	std::ifstream ifs(setbmspath);

	// 失敗時処理
	if (ifs.fail())
		return;

	//パスは1行だけ読み込む
	getline(ifs, bmspath);

}

void Bmsdata::setbmsstring()
{
	// ファイル入力
	std::ifstream ifs(bmspath);
	std::string tempstring;
	std::vector<std::string> temp_array;

	// 失敗時処理
	if (ifs.fail())
		return;

	//配列に一時格納
	while (getline(ifs, tempstring))
	{
		if (tempstring.find_first_of("#") == 0){	//コマンド判定
			tempstring.erase(0, 1);					//さよなら#

			temp_array.push_back(tempstring);
		}
	}

	//カウンタの初期化
	random_count = 0;

	//ControlFlow切り出し
	//TODO:真面目に制御フローを読み込むならば、データとパーサを分ける必要がありそう
	for (unsigned int i = 0; i < temp_array.size(); i++){

		if (temp_array.at(i).substr(0, 6) == "RANDOM"){
			;
		}

		if (temp_array.at(i).substr(0, 2) == "IF"){
			bool endflag = true;
			int j = i + 1;

			while (endflag){						//ENDIFをさがせ
				if (temp_array.at(j).substr(0, 3) == "ENDIF")
					endflag = false;
				j++;
			}
			//ENDIFが見つかったらそこを切り取る
			for (int k = i; j > k; k++){
			}
		}
		
	}
	
	//ヘッダ部
	for (unsigned i = 0; i < temp_array.size(); i++){

		if (temp_array.at(i).substr(0,3) == "BMP")
			bmp_array.push_back(temp_array.at(i));

		else if (temp_array.at(i).substr(0, 3) == "WAV")
			wav_array.push_back(temp_array.at(i));

		else if (atoi(temp_array.at(i).substr(0, 5).c_str()) > 0)
			main_array.push_back(temp_array.at(i));

		else
			header_array.push_back(temp_array.at(i));
	}
	//header解析
	header_analysis(header_array);

	//path解析
	for (unsigned i = 0; i < bmp_array.size(); i++){
		DATA tempdata;
		tempdata.id = base_stoi(36, bmp_array.at(i).substr(3, 2));
		tempdata.path = bmp_array.at(i).substr(6);

		int j = i;
		//idと配列の場所を同じにする
		bmp_path_array.push_back(tempdata);
	}
	for (unsigned i = 0; i < wav_array.size(); i++){
		DATA tempdata;
		tempdata.id = base_stoi(36, wav_array.at(i).substr(3, 2));
		tempdata.path = wav_array.at(i).substr(6);

		wav_path_array.push_back(tempdata);
	}

	//data解析
	for (unsigned i = 0; i < main_array.size(); i++){
		MAIN temp = {};
		//チャンネル
		int channel = stoi(main_array.at(i).substr(3, 2));
		//小節
		temp.measure = stoi(main_array.at(i).substr(0, 3));

		//#xxx02では小数のみが与えられる
		if (channel == 2){
			temp.num = stod(main_array.at(i).substr(6));
			main_data_array[channel].push_back(temp);
		}
		else{
			//解像度の計算
			temp.resolution = main_array.at(i).substr(6).length() / 2;

			//ID及びその位置の計算
			for (unsigned j = 0; j < temp.resolution; j++){
				temp.step = j;
				temp.id = base_stoi(36, main_array.at(i).substr(6 + j * 2, 2));

				//IDが00でなければ構造体の配列に格納する
				if (temp.id != 0)
					main_data_array[channel].push_back(temp);
			}
		}
	}

	//ソート
	for (int i = 0; i < CHANNEL_ELEMENTS; i++){
		if (main_data_array[i].size())
			std::sort(main_data_array[i].begin(), main_data_array[i].end());
	}
	for (int i = 0; i < bmp_path_array.size(); i++)
		std::sort(bmp_path_array.begin(), bmp_path_array.end());

	for (int i = 0; i < wav_path_array.size(); i++)
		std::sort(wav_path_array.begin(), wav_path_array.end());

	return;
}

int Bmsdata::base_stoi(int base, std::string str)	//10進数変換
{
	int ans = 0;
	std::string num_check = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	
	for (int i = 0; i < str.length(); i++){
		for (int j = 0; j < base; j++){
			if (str.at(i) == num_check.at(j))
				ans += j * pow(36, str.length() - 1 - i);
		}
	}

	return ans;
}

void Bmsdata::header_analysis(std::vector<std::string> header_array){
	for (int i = 0; i < header_array.size(); i++){
		if (header_array.at(i).substr(0, 5) == "TITLE")
			title = header_array.at(i).substr(6);
		else if (header_array.at(i).substr(0, 4) == "RANK")
			rank = stoi(header_array.at(i).substr(5));
		else if (header_array.at(i).substr(0, 9) == "PLAYLEVEL")
			playlevel = stoi(header_array.at(i).substr(10));
	}
}

std::string Bmsdata::gettitle(){
	return title;
}

int Bmsdata::getplaylevel(){
	return playlevel;
}

int Bmsdata::getsize(int channel){
	return main_data_array[channel].size();
}