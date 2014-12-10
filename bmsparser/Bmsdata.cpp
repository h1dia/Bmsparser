#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "Bmsdata.h"

Bmsdata::Bmsdata(){
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

	//ControllFlow�؂�o��
	for (unsigned int i = 0; i < temp_array.size(); i++){

		if (temp_array.at(i).substr(0, 6) == "RANDOM"){
			;
		}

		if (temp_array.at(i).substr(0, 2) == "IF"){
			bool endflag = true;
			int j = i + 1;

			while (endflag){						//ENDIF��������
				if (temp_array.at(j).substr(0, 3) == "ENDIF")
					endflag = false;
				j++;
			}
			//ENDIF�����������炻����؂���
			for (int k = i; j > k; k++){
			}
		}
		
	}
	
	//�w�b�_��
	for (unsigned i = 0; i < temp_array.size(); i++){

		if (temp_array.at(i).substr(0,3) == "BMP"){
			bmp_array.push_back(temp_array.at(i));
		}
		else if (temp_array.at(i).substr(0, 3) == "WAV"){
			wav_array.push_back(temp_array.at(i));
		}
		else if (temp_array.at(i).substr(0, 5) == "TITLE"){
			title = temp_array.at(i).substr(6);
		}
		else if (atoi(temp_array.at(i).substr(0, 5).c_str()) > 0)
			main_array.push_back(temp_array.at(i));
		else
			header_array.push_back(temp_array.at(i));
	}

	//path���
	for (unsigned i = 0; i < bmp_array.size(); i++){
		DATA tempdata;
		tempdata.id = base_stoi(36, bmp_array.at(i).substr(3, 2));
		tempdata.path = bmp_array.at(i).substr(6);

		bmp_path_array.push_back(tempdata);
	}
	for (unsigned i = 0; i < wav_array.size(); i++){
		DATA tempdata;
		tempdata.id = base_stoi(36, wav_array.at(i).substr(3, 2));
		tempdata.path = wav_array.at(i).substr(6);

		bmp_path_array.push_back(tempdata);
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

	//data�\�[�g
	for (int i = 0; i < CHANNEL_ELEMENTS; i++){
		if (main_data_array[i].size())
			std::sort(main_data_array[i].begin(), main_data_array[i].end());
	}

	return;
}

int Bmsdata::base_stoi(int base, std::string str)	//10�i���ϊ�
{
	int ans = 0;
	std::string num_check = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	
	for (int i = 0; i < 2; i++){
		for (int j = 0; j < base; j++){
			if (str.at(i) == num_check.at(j))
				ans += j * (i + 1);
		}
	}

	return ans;
}

std::string Bmsdata::gettitle(){
	return title;
}

int Bmsdata::getsize(int channel){
	return main_data_array[channel].size();
}