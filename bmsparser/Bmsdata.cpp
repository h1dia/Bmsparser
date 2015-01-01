// BMS 関連の用語は以下にて参照ください。
// BMS command memo (JP) - http://hitkey.nekokan.dyndns.info/cmdsJP.htm

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

	getline(ifs, bmspath);

}

void Bmsdata::setbmsstring()
{
	// ファイル入力
	std::ifstream ifs(bmspath);
	std::string tempstring;
	std::vector<std::string> temp_array;

	if (ifs.fail())
		return;

	// temp_array にコマンド行のみを格納
	while (getline(ifs, tempstring))
	{
		if (tempstring.find_first_of("#") == 0){
			// #の削除
			tempstring.erase(0, 1);
			temp_array.push_back(tempstring);
		}
	}

	// CONTROL FLOWの処理
	parse_random(temp_array, 0, temp_array.size());
	
	// 配列への振り分け
	for (unsigned i = 0; i < temp_array.size(); i++){

		if (starts_with(temp_array.at(i), kNotAvailable))
			continue;

		if (starts_with(temp_array.at(i), "BMP"))
			bmp_array.push_back(temp_array.at(i));

		else if (starts_with(temp_array.at(i), "WAV"))
			wav_array.push_back(temp_array.at(i));

		// コマンドの先頭5文字
		else if (atoi(temp_array.at(i).substr(0, 5).c_str()) > 0)
			channel_array.push_back(temp_array.at(i));

		else
			header_array.push_back(temp_array.at(i));
	}

	// HEADER 文解析
	header_analysis(header_array);

	// ID 及び PATH の正規化
	normalize_data(bmp_array, bmp_path_array);
	normalize_data(wav_array, wav_path_array);

	// CHANNEL 文解析
	for (unsigned i = 0; i < channel_array.size(); i++){
		CHANNEL temp = {};
		// 小節
		temp.measure = stoi(channel_array.at(i).substr(0, 3));

		// チャンネル
		int channel = stoi(channel_array.at(i).substr(3, 2));

		// #xxx02 では小数のみが与えられる
		if (channel == 2){
			temp.num = stod(channel_array.at(i).substr(6));
			channel_data_array[channel].push_back(temp);
		}
		else{
			//解像度の計算
			temp.resolution = channel_array.at(i).substr(6).length() / 2;

			//ID 及びその位置の計算
			for (unsigned j = 0; j < temp.resolution; j++){
				temp.step = j;
				temp.id = base_stoi(36, channel_array.at(i).substr(6 + j * 2, 2));

				//ID 00 を無視する
				if (temp.id != 0)
					channel_data_array[channel].push_back(temp);
			}
		}
	}

	// ソート
	// TODO: 関数化
	for (int i = 0; i < CHANNEL_ELEMENTS; i++){
		if (channel_data_array[i].size())
			std::sort(channel_data_array[i].begin(), channel_data_array[i].end());
	}
	for (unsigned int i = 0; i < bmp_path_array.size(); i++)
		std::sort(bmp_path_array.begin(), bmp_path_array.end());

	for (unsigned int i = 0; i < wav_path_array.size(); i++)
		std::sort(wav_path_array.begin(), wav_path_array.end());

	return;
}

// std::string で記述された任意の進数の値を、10進数で返します。
// 引数:
//		str: 任意の進数で記述された値。
//		base: str の進数。
int Bmsdata::base_stoi(int base, std::string str){
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

// RANDOM 命令をパースします。
// 引数:
//     temp_array: BMS 命令の vector。
//     from: パースする範囲の先頭位置。
//     length: パースする範囲の長さ。
// 詳細:
//     RANDOM 命令を評価し、IF 命令で条件に一致しないものを利用不可としてマークします。
//     ネストされた RANDOM、IF 命令をパースできます。temp_array への変更は破壊的変更です。
void Bmsdata::parse_random(std::vector<std::string>& temp_array, unsigned int from, unsigned int length){
	int random_value = 0;

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
			if (random_value == value){
				// 条件にヒットしたとき
				// 再帰呼び出しでネストされた分をパース
				parse_random(temp_array, i + 1, endif - i - 1);
				// 次を評価するためにカウンタを操作
				i = endif;
			}
			else {
				// 条件にヒットしなかったとき
				// IF から ENDIF の間に利用不可としてマーク
				for (unsigned int j = i; j <= endif; j++){
					temp_array[j] = kNotAvailable;
				}
				// 次を評価するためにカウンタを操作
				i = endif;
			}
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
		else if (starts_with(temp_array.at(i), "ENDIF")){
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

// ID と PATH が記述された命令を正規化します。
// 引数:
//		data_array: ID と PATH が記述されている命令の vector
//		normalized_array: 正規化された命令を格納する vector
void Bmsdata::normalize_data(std::vector<std::string> &data_array, std::vector<DATA> &normalized_array){
	for (unsigned i = 0; i < data_array.size(); i++){
		DATA tempdata;
		tempdata.id = base_stoi(36, data_array.at(i).substr(3, 2));
		tempdata.path = data_array.at(i).substr(6);

		normalized_array.push_back(tempdata);
	}
}

// HEADER命令を解析し、適当な変数に結果を代入します。
// 引数:
//		headder_array: <HEADER> に属した BMS 命令の vector
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

int Bmsdata::getsize(int channel){
	return channel_data_array[channel].size();
}

// 削除予定の関数です。
std::string Bmsdata::gettitle(){
	return title;
}

// 削除予定の関数です。
int Bmsdata::getplaylevel(){
	return playlevel;
}