#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

float letter_frequencies[26] = { 0.0804, 0.0154, 0.0306, 0.0399, 0.1251,
                                 0.0230, 0.0196, 0.0549, 0.0726, 0.0016,
                                 0.0067, 0.0414, 0.0253, 0.0709, 0.0760,
                                 0.0200, 0.0011, 0.0612, 0.0654, 0.0925,
                                 0.0271, 0.0099, 0.0192, 0.0019, 0.0173,
                                 0.0009                                  };

void hex_to_bytes(unsigned char* str) {
	size_t idx;
	for(idx = 0; str[idx] != '\0'; idx++) {
		if(str[idx] <= '9') {
			str[idx] -= 48;
		} else {
			str[idx] -= 87;
		}
	}
	size_t size = idx;

	for(idx = 0; idx < size / 2; idx++) {
		str[idx] = str[idx * 2] << 4;
		str[idx] += str[idx * 2 + 1];
	}
	str[idx] = '\0';
}

void bytes_to_hex(unsigned char* str, size_t size) {
	size_t idx = size;
	str[idx * 2] = '\0';
	idx--;
	while(1) {
		str[idx * 2 + 1] = str[idx] & 15;
		str[idx * 2] = str[idx] >> 4;
		if(idx == 0) {
			break;
		}
		idx--;
	}

	for(idx = 0; idx < size * 2; idx++) {
		if(str[idx] < 10) {
			str[idx] += '0';
		} else {
			str[idx] += 'a' - 10;
		}
	}
}

float english_score(char* str) {
	size_t single_count[26];
	for(size_t idx = 0; idx < 26; idx++) {
		single_count[idx] = 0;
	}
	size_t letter_count = 0;
	size_t size = strlen(str);
	for(size_t idx = 0; idx < size; idx++) {
		char current_letter = str[idx];
		if((current_letter >= '\0' && current_letter <= 8) ||
		   current_letter < '\0') {
			return 0;
		}
		if(current_letter >= 'A' && current_letter <= 'Z') {
			current_letter += 97 - 65;
		}
		if(current_letter >= 'a' && current_letter <= 'z') {
			single_count[current_letter - 'a']++;
			letter_count++;
		}
	}
	float score = 26;
	for(size_t idx = 0; idx < 26; idx++) {
		score -= fabs(((float) single_count[idx]) / letter_count - letter_frequencies[idx]);
	}
	return score;
}

unsigned char** parse_lines(char* filename) {
	static unsigned char* lines[256];
	size_t idx = 0;
	FILE* infile = fopen(filename, "r");
	while(!feof(infile)) {
		lines[idx] = malloc(256 * sizeof(unsigned char));
		fscanf(infile, "%255[^\n]\n", lines[idx]);
		idx++;
	}
	fclose(infile);
	lines[idx] = '\0';
	return lines;
}

void s1c1() {
	unsigned char** input = parse_lines("inputs1c1.txt");
	hex_to_bytes(input[0]);
	unsigned char output[256];

	size_t i_idx = 0;
	size_t o_idx = 0;
	while(input[0][i_idx]     != '\0' && input[0][i_idx + 1] != '\0' &&
	      input[0][i_idx + 2] != '\0' ) {
		output[o_idx] = input[0][i_idx] >> 2;
		output[o_idx + 1] = (input[0][i_idx] & 3) << 4;
		output[o_idx + 1] += input[0][i_idx + 1] >> 4;
		output[o_idx + 2] = (input[0][i_idx + 1] & 15) << 2;
		output[o_idx + 2] += input[0][i_idx + 2] >> 6;
		output[o_idx + 3] = input[0][i_idx + 2] & 63;
		i_idx += 3;
		o_idx += 4;
	}
	if(input[0][i_idx] != '\0') {
		output[o_idx] = input[0][i_idx] >> 2;
		output[o_idx + 1] = (input[0][i_idx] & 3) << 4;
		i_idx++;
		o_idx++;
		if(input[0][i_idx] != '\0') {
			output[o_idx] += input[0][i_idx] >> 4;
			output[o_idx + 1] = (input[0][i_idx] & 15) << 2;
			i_idx++;
			o_idx++;
			output[o_idx] = '=';
			output[o_idx + 1] = '=';
			o_idx += 2;
		} else {
			output[o_idx] = '=';
			o_idx++;
		}
	}
	output[o_idx] = '\0';

	for(size_t o_idx2 = 0; o_idx2 < o_idx; o_idx2++) {
		if(output[o_idx2] < 26) {
			output[o_idx2] += 'A';
		} else if(output[o_idx2] < 52) {
			output[o_idx2] += 'a' - 26;
		} else if(output[o_idx2] < 62) {
			output[o_idx2] += '0' - 52;
		} else if(output[o_idx2] == 62) {
			output[o_idx2] = '+';
		} else {
			output[o_idx2] = '/';
		}
	}

	printf("%s\n", output);
}

void s1c2() {
	unsigned char** input = parse_lines("inputs1c2.txt");
	size_t original_size = strlen((const char*) input[0]);
	hex_to_bytes(input[0]);
	hex_to_bytes(input[1]);
	unsigned char output[256];

	for(size_t idx = 0; idx < original_size / 2; idx++) {
		output[idx] = input[0][idx] ^ input[1][idx];
	}

	bytes_to_hex(output, original_size / 2);
	printf("%s\n", output);
}

void s1c3() {
	unsigned char** input = parse_lines("inputs1c3.txt");
	hex_to_bytes(input[0]);

	unsigned char highest_key = 0;
	float highest_score = 0;
	unsigned char current_key = 0;
	size_t input_size = strlen((const char*) input[0]);
	while(1) {
		unsigned char output[256];
		for(size_t idx = 0; idx < input_size; idx++) {
			output[idx] = input[0][idx] ^ current_key;
		}
		output[input_size] = '\0';
		float current_score = english_score((char*) output);
		if(current_score > highest_score) {
			highest_key = current_key;
			highest_score = current_score;
		}
		if(current_key == 255) {
			break;
		}
		current_key++;
	}

	unsigned char output[256];
	for(size_t idx = 0; idx < input_size; idx++) {
		output[idx] = input[0][idx] ^ highest_key;
	}
	output[input_size] = '\0';
	printf("%s\n", output);
}

void s1c4() {
	unsigned char** input = parse_lines("inputs1c4.txt");

	size_t highest_line = 0;
	size_t highest_key = 0;
	float highest_score = 0;
	for(size_t current_line = 0; input[current_line] != '\0'; current_line++) {
		hex_to_bytes(input[current_line]);
		unsigned char current_key = 0;
		size_t input_size = strlen((const char*) input[current_line]);
		while(1) {
			unsigned char output[256];
			for(size_t idx = 0; idx < input_size; idx++) {
				output[idx] = input[current_line][idx] ^ current_key;
			}
			output[input_size] = '\0';
			float current_score = english_score((char*) output);
			if(current_score > highest_score) {
				highest_line = current_line;
				highest_key = current_key;
				highest_score = current_score;
			}
			if(current_key == 255) {
				break;
			}
			current_key++;
		}
	}

	size_t input_size = strlen((const char*) input[highest_line]);
	unsigned char output[256];
	for(size_t idx = 0; idx < input_size; idx++) {
		output[idx] = input[highest_line][idx] ^ highest_key;
	}
	output[input_size] = '\0';
	printf("%s\n", output);
}

void s1c5() {
	FILE* infile = fopen("inputs1c5.txt", "r");
	unsigned char input[256];
	input[fread(input, 1, 255, infile)] = '\0';
	fclose(infile);

	unsigned char* key = (unsigned char*) "ICE";
	unsigned char output[256];
	size_t size = strlen((const char*) input);
	for(size_t idx = 0; idx < size; idx++) {
		output[idx] = input[idx] ^ key[idx % 3];
	}
	output[size] = '\0';
	bytes_to_hex(output, size);
	printf("%s\n", output);
}

int main() {
	s1c1();
	s1c2();
	s1c3();
	s1c4();
	s1c5();
	return 0;
}
