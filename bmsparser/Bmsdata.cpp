// BMS �֘A�̗p��͈ȉ��ɂĎQ�Ƃ��������B
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
	// �����_���G���W��������
	std::random_device r;
	random_engine = std::mt19937(r());
}

Bmsdata::~Bmsdata(){
}

void Bmsdata::setbmspath(std::string setbmspath){
	std::ifstream ifs(setbmspath);

	// ���s������
	if (ifs.fail())
		return;

	getline(ifs, bmspath);

}

void Bmsdata::setbmsstring()
{
	// �t�@�C������
	std::ifstream ifs(bmspath);
	std::string tempstring;
	std::vector<std::string> temp_array;

	if (ifs.fail())
		return;

	// temp_array �ɃR�}���h�s�݂̂��i�[
	while (getline(ifs, tempstring))
	{
		if (tempstring.find_first_of("#") == 0){
			// #�̍폜
			tempstring.erase(0, 1);
			temp_array.push_back(tempstring);
		}
	}

	// CONTROL FLOW�̏���
	parse_random(temp_array, 0, temp_array.size());
	
	// �z��ւ̐U�蕪��
	for (unsigned i = 0; i < temp_array.size(); i++){

		if (starts_with(temp_array.at(i), kNotAvailable))
			continue;

		if (starts_with(temp_array.at(i), "BMP"))
			bmp_array.push_back(temp_array.at(i));

		else if (starts_with(temp_array.at(i), "WAV"))
			wav_array.push_back(temp_array.at(i));

		// �R�}���h�̐擪5����
		else if (atoi(temp_array.at(i).substr(0, 5).c_str()) > 0)
			channel_array.push_back(temp_array.at(i));

		else
			header_array.push_back(temp_array.at(i));
	}

	// HEADER �����
	header_analysis(header_array);

	// ID �y�� PATH �̐��K��
	normalize_data(bmp_array, bmp_path_array);
	normalize_data(wav_array, wav_path_array);

	// CHANNEL �����
	for (unsigned i = 0; i < channel_array.size(); i++){
		CHANNEL temp = {};
		// ����
		temp.measure = stoi(channel_array.at(i).substr(0, 3));

		// �`�����l��
		int channel = stoi(channel_array.at(i).substr(3, 2));

		// #xxx02 �ł͏����݂̂��^������
		if (channel == 2){
			temp.num = stod(channel_array.at(i).substr(6));
			channel_data_array[channel].push_back(temp);
		}
		else{
			//�𑜓x�̌v�Z
			temp.resolution = channel_array.at(i).substr(6).length() / 2;

			//ID �y�т��̈ʒu�̌v�Z
			for (unsigned j = 0; j < temp.resolution; j++){
				temp.step = j;
				temp.id = base_stoi(36, channel_array.at(i).substr(6 + j * 2, 2));

				//ID 00 �𖳎�����
				if (temp.id != 0)
					channel_data_array[channel].push_back(temp);
			}
		}
	}

	// �\�[�g
	// TODO: �֐���
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

// std::string �ŋL�q���ꂽ�C�ӂ̐i���̒l���A10�i���ŕԂ��܂��B
// ����:
//		str: �C�ӂ̐i���ŋL�q���ꂽ�l�B
//		base: str �̐i���B
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

// 1 ���� max �܂ł͈̔͂ŗ�����Ԃ��܂��B
// ����:
//     max: �����̍ő�l�B
int Bmsdata::random(int max){
	return random_engine() % max + 1;
}

// RANDOM ���߂��p�[�X���܂��B
// ����:
//     temp_array: BMS ���߂� vector�B
//     from: �p�[�X����͈͂̐擪�ʒu�B
//     length: �p�[�X����͈͂̒����B
// �ڍ�:
//     RANDOM ���߂�]�����AIF ���߂ŏ����Ɉ�v���Ȃ����̂𗘗p�s�Ƃ��ă}�[�N���܂��B
//     �l�X�g���ꂽ RANDOM�AIF ���߂��p�[�X�ł��܂��Btemp_array �ւ̕ύX�͔j��I�ύX�ł��B
void Bmsdata::parse_random(std::vector<std::string>& temp_array, unsigned int from, unsigned int length){
	int random_value = 0;

	for (unsigned int i = from; i < from + length; i++){
		if (starts_with(temp_array.at(i), "RANDOM")){
			// RANDOM �l�̐���
			int random_max = stoi(temp_array.at(i).substr(7));
			random_value = random(random_max);
		}
		else if (starts_with(temp_array.at(i), "IF")){
			// IF �l�̎擾
			int value = stoi(temp_array.at(i).substr(3));

			unsigned int endif = find_endif(temp_array, i);
			if (random_value == value){
				// �����Ƀq�b�g�����Ƃ�
				// �ċA�Ăяo���Ńl�X�g���ꂽ�����p�[�X
				parse_random(temp_array, i + 1, endif - i - 1);
				// ����]�����邽�߂ɃJ�E���^�𑀍�
				i = endif;
			}
			else {
				// �����Ƀq�b�g���Ȃ������Ƃ�
				// IF ���� ENDIF �̊Ԃɗ��p�s�Ƃ��ă}�[�N
				for (unsigned int j = i; j <= endif; j++){
					temp_array[j] = kNotAvailable;
				}
				// ����]�����邽�߂ɃJ�E���^�𑀍�
				i = endif;
			}
		}
	}
}

// ����� ENDIF ���߂̈ʒu���������܂��B
// ����:
//     temp_array: BMS ���߂� vector�B
//     index: �������� ENDIF ���߂ɑΉ����� IF ���߂̈ʒu�B
// �߂�l:
//     �w�肵�� IF ���߂ɑΉ����� ENDIF ���߂̈ʒu�B
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

// str �� substr �Ŏn�܂镶���񂩂ǂ�����Ԃ��܂��B
// ����:
//     str: �]������镶����B
//     substr: �]�����镶����B
// �߂�l:
//     str �� substr �Ŏn�܂�Ȃ� true�A�����łȂ��Ȃ� false�B
bool Bmsdata::starts_with(std::string& str, std::string substr){
	return str.substr(0, substr.length()) == substr;
}

// ID �� PATH ���L�q���ꂽ���߂𐳋K�����܂��B
// ����:
//		data_array: ID �� PATH ���L�q����Ă��閽�߂� vector
//		normalized_array: ���K�����ꂽ���߂��i�[���� vector
void Bmsdata::normalize_data(std::vector<std::string> &data_array, std::vector<DATA> &normalized_array){
	for (unsigned i = 0; i < data_array.size(); i++){
		DATA tempdata;
		tempdata.id = base_stoi(36, data_array.at(i).substr(3, 2));
		tempdata.path = data_array.at(i).substr(6);

		normalized_array.push_back(tempdata);
	}
}

// HEADER���߂���͂��A�K���ȕϐ��Ɍ��ʂ������܂��B
// ����:
//		headder_array: <HEADER> �ɑ����� BMS ���߂� vector
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

// �폜�\��̊֐��ł��B
std::string Bmsdata::gettitle(){
	return title;
}

// �폜�\��̊֐��ł��B
int Bmsdata::getplaylevel(){
	return playlevel;
}