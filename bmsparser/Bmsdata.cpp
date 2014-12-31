#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "Bmsdata.h"
#include <cmath>

Bmsdata::Bmsdata(){
	// ランダムエンジン初期化
	std::random_device r;
	random_engine = std::mt19937(r());
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
	parse_random(temp_array, 0, temp_array.size());
	clean_random(temp_array);
	
	//ヘッダ部
	for (unsigned i = 0; i < temp_array.size(); i++){

		if (starts_with(temp_array.at(i), "BMP"))
			bmp_array.push_back(temp_array.at(i));

		else if (starts_with(temp_array.at(i), "WAV"))
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
	for (unsigned int i = 0; i < bmp_path_array.size(); i++)
		std::sort(bmp_path_array.begin(), bmp_path_array.end());

	for (unsigned int i = 0; i < wav_path_array.size(); i++)
		std::sort(wav_path_array.begin(), wav_path_array.end());

	return;
}

int Bmsdata::base_stoi(int base, std::string str)	//10進数変換
{
	int ans = 0;
	std::string num_check = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	
	for (unsigned int i = 0; i < str.length(); i++){
		for (int j = 0; j < base; j++){
			if (str.at(i) == num_check.at(j))
				ans += (int)(j * pow(36, str.length() - 1 - i));
		}
	}

	return ans;
}

// 1 から max までの範囲で乱数を返します。
// 引数:
//     max: 乱数の最大値。
int Bmsdata::random(int max){
	return random_engine() % max + 1;
}

void Bmsdata::header_analysis(std::vector<std::string>& header_array){
	for (unsigned int i = 0; i < header_array.size(); i++){
		if (starts_with(header_array.at(i), "TITLE"))
			title = header_array.at(i).substr(6);
		else if (starts_with(header_array.at(i), "RANK"))
			rank = stoi(header_array.at(i).substr(5));
		else if (starts_with(header_array.at(i), "PLAYLEVEL"))
			playlevel = stoi(header_array.at(i).substr(10));
	}
}

// RANDOM 命令をパースします。
// 引数:
//     temp_array: BMS 命令の vector。
//     from: パースする範囲の先頭位置。
//     length: パースする範囲の長さ。
// 戻り値:
//     削除した BMS 命令数。
// 詳細:
//     RANDOM 命令を評価し、IF 命令で条件に一致しないものを temp_array から削除します。
//     ネストされた RANDOM、IF 命令をパースできます。temp_array への変更は破壊的変更です。
unsigned int Bmsdata::parse_random(std::vector<std::string>& temp_array, unsigned int from, unsigned int length){
	int random_value = 0;
	unsigned int delete_count = 0;

	for (unsigned int i = from; i < from + length; i++){
		if (starts_with(temp_array.at(i), "RANDOM")){
			// RANDOM 値の生成
			int random_max = stoi(temp_array.at(i).substr(7));
			random_value = random(random_max);
		}
		else if (starts_with(temp_array.at(i), "IF")){
			// IF 値の取得
			int value = stoi(temp_array.at(i).substr(3));

			unsigned int endif = find_endif(temp_array, i);
			unsigned int del = 0;
			if (random_value == value){
				// 条件にヒットしたとき
				// 再帰呼び出しでネストされた分をパース
				del = parse_random(temp_array, i + 1, endif - i - 1);
				// 次を評価するためにカウンタを操作
				i = endif - del;
			}
			else {
				// 条件にヒットしなかったとき
				// IF から ENDIF をバッサリ削除、無かったことにする
				temp_array.erase(temp_array.begin() + i, temp_array.begin() + endif + 1);
				del = endif - i + 2;
				// 次を評価するためにカウンタを操作
				i--;
			}
			// 削除した分 length は短くなる
			length -= del;
			delete_count += del;
		}
	}
	return delete_count;
}

// RANDOM、IF 命令を削除します。
// 引数:
//     temp_array: BMS 命令の vector。
// 詳細:
//     parse_random 関数を使用して残った RANDOM、IF 命令を削除します。
void Bmsdata::clean_random(std::vector<std::string>& temp_array){
	for (unsigned int i = 0; i < temp_array.size(); i++){
		if (starts_with(temp_array.at(i), "RANDOM") || starts_with(temp_array.at(i), "IF") || starts_with(temp_array.at(i), "ENDIF")){
			temp_array.erase(temp_array.begin() + i);
			i--;
		}
	}
}

// 後方の ENDIF 命令の位置を検索します。
// 引数:
//     temp_array: BMS 命令の vector。
//     index: 検索する ENDIF 命令に対応する IF 命令の位置。
// 戻り値:
//     指定した IF 命令に対応する ENDIF 命令の位置。
unsigned int Bmsdata::find_endif(std::vector<std::string>& temp_array, unsigned int index){
	int level = 0;
	for (unsigned int i = index + 1; i < temp_array.size(); i++){
		if (starts_with(temp_array.at(i), "IF")){
			level++;
		}
		else if (starts_with(temp_array.at(i), "ENDIF")){ // ENDIFをさがせ
			if (level == 0)
				return i;
			else
				level--;
		}
	}
	return temp_array.size() - 1;
}

// str が substr で始まる文字列かどうかを返します。
// 引数:
//     str: 評価される文字列。
//     substr: 評価する文字列。
// 戻り値:
//     str が substr で始まるなら true、そうでないなら false。
bool Bmsdata::starts_with(std::string& str, std::string substr){
	return str.substr(0, substr.length()) == substr;
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
