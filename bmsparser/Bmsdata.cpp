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

	//�p�X��1�s�����ǂݍ���
	getline(ifs, bmspath);

}

void Bmsdata::setbmsstring()
{
	// �t�@�C������
	std::ifstream ifs(bmspath);
	std::string tempstring;
	std::vector<std::string> temp_array;

	// ���s������
	if (ifs.fail())
		return;

	//�z��Ɉꎞ�i�[
	while (getline(ifs, tempstring))
	{
		if (tempstring.find_first_of("#") == 0){	//�R�}���h����
			tempstring.erase(0, 1);					//����Ȃ�#

			temp_array.push_back(tempstring);
		}
	}

	//�J�E���^�̏�����
	random_count = 0;

	//ControlFlow�؂�o��
	parse_random(temp_array, 0, temp_array.size());
	clean_random(temp_array);
	
	//�w�b�_��
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
	//header���
	header_analysis(header_array);

	//path���
	for (unsigned i = 0; i < bmp_array.size(); i++){
		DATA tempdata;
		tempdata.id = base_stoi(36, bmp_array.at(i).substr(3, 2));
		tempdata.path = bmp_array.at(i).substr(6);

		int j = i;
		//id�Ɣz��̏ꏊ�𓯂��ɂ���
		bmp_path_array.push_back(tempdata);
	}
	for (unsigned i = 0; i < wav_array.size(); i++){
		DATA tempdata;
		tempdata.id = base_stoi(36, wav_array.at(i).substr(3, 2));
		tempdata.path = wav_array.at(i).substr(6);

		wav_path_array.push_back(tempdata);
	}

	//data���
	for (unsigned i = 0; i < main_array.size(); i++){
		MAIN temp = {};
		//�`�����l��
		int channel = stoi(main_array.at(i).substr(3, 2));
		//����
		temp.measure = stoi(main_array.at(i).substr(0, 3));

		//#xxx02�ł͏����݂̂��^������
		if (channel == 2){
			temp.num = stod(main_array.at(i).substr(6));
			main_data_array[channel].push_back(temp);
		}
		else{
			//�𑜓x�̌v�Z
			temp.resolution = main_array.at(i).substr(6).length() / 2;

			//ID�y�т��̈ʒu�̌v�Z
			for (unsigned j = 0; j < temp.resolution; j++){
				temp.step = j;
				temp.id = base_stoi(36, main_array.at(i).substr(6 + j * 2, 2));

				//ID��00�łȂ���΍\���̂̔z��Ɋi�[����
				if (temp.id != 0)
					main_data_array[channel].push_back(temp);
			}
		}
	}

	//�\�[�g
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

int Bmsdata::base_stoi(int base, std::string str)	//10�i���ϊ�
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

// 1 ���� max �܂ł͈̔͂ŗ�����Ԃ��܂��B
// ����:
//     max: �����̍ő�l�B
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

// RANDOM ���߂��p�[�X���܂��B
// ����:
//     temp_array: BMS ���߂� vector�B
//     from: �p�[�X����͈͂̐擪�ʒu�B
//     length: �p�[�X����͈͂̒����B
// �߂�l:
//     �폜���� BMS ���ߐ��B
// �ڍ�:
//     RANDOM ���߂�]�����AIF ���߂ŏ����Ɉ�v���Ȃ����̂� temp_array ����폜���܂��B
//     �l�X�g���ꂽ RANDOM�AIF ���߂��p�[�X�ł��܂��Btemp_array �ւ̕ύX�͔j��I�ύX�ł��B
unsigned int Bmsdata::parse_random(std::vector<std::string>& temp_array, unsigned int from, unsigned int length){
	int random_value = 0;
	unsigned int delete_count = 0;

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
			unsigned int del = 0;
			if (random_value == value){
				// �����Ƀq�b�g�����Ƃ�
				// �ċA�Ăяo���Ńl�X�g���ꂽ�����p�[�X
				del = parse_random(temp_array, i + 1, endif - i - 1);
				// ����]�����邽�߂ɃJ�E���^�𑀍�
				i = endif - del;
			}
			else {
				// �����Ƀq�b�g���Ȃ������Ƃ�
				// IF ���� ENDIF ���o�b�T���폜�A�����������Ƃɂ���
				temp_array.erase(temp_array.begin() + i, temp_array.begin() + endif + 1);
				del = endif - i + 2;
				// ����]�����邽�߂ɃJ�E���^�𑀍�
				i--;
			}
			// �폜������ length �͒Z���Ȃ�
			length -= del;
			delete_count += del;
		}
	}
	return delete_count;
}

// RANDOM�AIF ���߂��폜���܂��B
// ����:
//     temp_array: BMS ���߂� vector�B
// �ڍ�:
//     parse_random �֐����g�p���Ďc���� RANDOM�AIF ���߂��폜���܂��B
void Bmsdata::clean_random(std::vector<std::string>& temp_array){
	for (unsigned int i = 0; i < temp_array.size(); i++){
		if (starts_with(temp_array.at(i), "RANDOM") || starts_with(temp_array.at(i), "IF") || starts_with(temp_array.at(i), "ENDIF")){
			temp_array.erase(temp_array.begin() + i);
			i--;
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
		else if (starts_with(temp_array.at(i), "ENDIF")){ // ENDIF��������
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

std::string Bmsdata::gettitle(){
	return title;
}

int Bmsdata::getplaylevel(){
	return playlevel;
}

int Bmsdata::getsize(int channel){
	return main_data_array[channel].size();
}
