// BMS �֘A�̗p��͈ȉ��ɂĎQ�Ƃ��������B
// BMS command memo (JP) - http://hitkey.nekokan.dyndns.info/cmdsJP.htm

#pragma once
#include "Bmsdata.h"
#include <chrono>


Bmsdata::Bmsdata(std::string filepath){
	// �t�@�C���p�X�ǂݍ���
	if (!(filepath.find_last_of("/") == std::string::npos))
		filefolder = filepath.substr(0, filepath.find_last_of("/") + 1);
	else
		filefolder = "";
	filename = filepath.substr(filepath.find_last_of("/") + 1);

	// vector ������
	channel_data_array.resize(MAX_CHANNEL);
	bmp_path_array.resize(MAX_ID);
	wav_path_array.resize(MAX_ID);

	// �����_���G���W��������
	std::random_device r;
	random_engine = std::mt19937(r());
}

Bmsdata::~Bmsdata(){
}

void Bmsdata::setbmsstring(){
	auto start = std::chrono::system_clock::now();

	// �t�@�C������
	std::ifstream ifs(filefolder + filename);
	std::string tempstring;
	std::vector<std::string> temp_array, header_array, channel_array, wav_array, bmp_array;
	
	// ���s������
	if (ifs.fail())
		load_fail = true;
	else
		load_fail = false;

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
		
		if (control_flow_exsist){
			if (judge_disused_command(temp_array.at(i)))
				continue;
		}
		
		if (starts_with(temp_array.at(i), "BMP"))
			bmp_array.push_back(temp_array.at(i));

		else if (starts_with(temp_array.at(i), "WAV"))
			wav_array.push_back(temp_array.at(i));

		else if (std::all_of(temp_array.at(i).cbegin(), temp_array.at(i).cbegin() + 5, isdigit))
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
		else {
			//�𑜓x�̌v�Z
			temp.resolution = channel_array.at(i).substr(6).length() / 2;

			//ID �y�т��̈ʒu�̌v�Z
			for (unsigned j = 0; j < temp.resolution; j++){
				temp.id = base_stoi(36, channel_array.at(i).substr(6 + j * 2, 2));
				//ID 00 �𖳎�����
				if (temp.id != 0){
					temp.step = j;
					channel_data_array.at(channel).push_back(temp);
				}
			}
		}
	}

	// �\�[�g
	for (int i = 0; i < MAX_CHANNEL; i++){
		if (channel_data_array.at(i).size())
			std::sort(channel_data_array.at(i).begin(), channel_data_array.at(i).end());
	}

	return;
}

// std::string �ŋL�q���ꂽ�C�ӂ̐i���̒l���A10�i���ŕԂ��܂��B
// ����:
//		str: �C�ӂ̐i���ŋL�q���ꂽ�l�B
//		base: str �̐i���B
int Bmsdata::base_stoi(int base, std::string str){
	int ans = 0;

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
	control_flow_exsist = false;
	int random_value = 0;

	for (unsigned int i = from; i < from + length; i++){
		if (starts_with(temp_array.at(i), "RANDOM")){
			control_flow_exsist = true;
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
		//#ENDIF #END IF #IFEND �͑S�ē��`�ł��B
		else if (starts_with(temp_array.at(i), "END") || starts_with(temp_array.at(i), "IFEND")){
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
void Bmsdata::normalize_data(std::vector<std::string> &data_array, std::vector<std::string> &normalized_array){
	for (unsigned i = 0; i < data_array.size(); i++){
		int tempid = base_stoi(36, data_array.at(i).substr(3, 2));
		std::string temppath = data_array.at(i).substr(6);

		normalized_array.at(tempid) = temppath;
	}
}

// HEADER, CHANNEL ���߂��p�[�X�����ŁA�s�v�Ȗ��߂��ǂ����𔻒肵�܂��B
// parse_random �֐�������ɌĂяo�����ꍇ�͖���`�̓���ł��B
// ����:
//     command: �]������镶����B
// �߂�l:
//     command ���s�v�ȃR�}���h�̏����Ɉ�v����Ȃ� true�A�����łȂ��Ȃ� false�B
bool Bmsdata::judge_disused_command(std::string command){
	if (kNotAvailable == command
		|| starts_with(command, "RANDOM")
		|| starts_with(command, "IF")
		|| starts_with(command, "ELSEIF")
		|| starts_with(command, "ENDIF")
		|| starts_with(command, "SETRANDOM")
		|| starts_with(command, "ENDRANDOM")
		|| starts_with(command, "SWITCH")
		|| starts_with(command, "CASE")
		|| starts_with(command, "SKIP")
		|| starts_with(command, "DEF")
		|| starts_with(command, "SETSWITCH")
		|| starts_with(command, "ENDSW")
		)
		return true;
	else
		return false;
}


// HEADER ���߂̂��� ID ������Ȃ����߂��A���ׂ� header_list �Ɋi�[���܂��B
// ���p�X�y�[�X���f���~�^�Ƃ��āA���߂� command �ɁAstr �ɖ��߂̓��e���A
// ������ str �����l�Ȃ�� val true ���A�����łȂ��Ȃ� val �� false �������܂��B
// ����:
//		headder_array: <HEADER> �ɑ����� BMS ���߂� vector
void Bmsdata::header_analysis(std::vector<std::string>& header_array){
	HEADER temp;
	for (unsigned int i = 0; i < header_array.size(); i++){
		std::string header = header_array.at(i);

		temp.command = header.substr(0, header.find_first_of(" "));
		temp.str = header.substr(header.find_first_of(" ") + 1);

		try{
			stod(temp.str);
			temp.val = true;
		}
		catch (std::invalid_argument&){
			temp.val = false;
		}

		header_list.push_back(temp);
	}
}

// HEADER ���߂̓��e�� string �^�Ŏ擾���܂��B
// ����:
//     command: �Ăяo���ׂ����߂̓��e�B
// �߂�l:
//     ���߂̓��e�����݂��Ă���Γ��e�� string�A�����łȂ��Ȃ� kNotAvailable�B
std::string Bmsdata::get_header_s(std::string command){
	for (unsigned int i = 0; i < header_list.size(); i++){
		if (header_list.at(i).command == command)
			return header_list.at(i).str;
	}
	return kNotAvailable;
}

// HEADER ���߂̓��e�� double �^�Ŏ擾���܂��B
// ����:
//     command: �Ăяo���ׂ����߂̓��e�B
// �߂�l:
//     ���߂̓��e�����݂��Ă��āA�����l�Ȃ�Γ��e�� double�A�����łȂ��Ȃ� NAN�B
double Bmsdata::get_header_d(std::string command){
	for (unsigned int i = 0; i < header_list.size(); i++){
		if (header_list.at(i).command == command){
			if (header_list.at(i).val)
				stod(header_list.at(i).str);
			else
				return NAN;
		}
	}
	return NAN;
}

int Bmsdata::getsize(int channel){
	return channel_data_array[channel].size();
}
