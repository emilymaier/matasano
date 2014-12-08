#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "rijndael.h"
#include "util.h"

float letter_frequencies[26] = { 0.0804, 0.0154, 0.0306, 0.0399, 0.1251,
                                 0.0230, 0.0196, 0.0549, 0.0726, 0.0016,
                                 0.0067, 0.0414, 0.0253, 0.0709, 0.0760,
                                 0.0200, 0.0011, 0.0612, 0.0654, 0.0925,
                                 0.0271, 0.0099, 0.0192, 0.0019, 0.0173,
                                 0.0009                                  };

size_t hex_to_bytes(unsigned char* str) {
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
	return idx;
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

float english_score(unsigned char* str, size_t size) {
	size_t single_count[26];
	for(size_t idx = 0; idx < 26; idx++) {
		single_count[idx] = 0;
	}
	size_t letter_count = 0;
	int fail = 0;
	for(size_t idx = 0; idx < size; idx++) {
		unsigned char current_letter = str[idx];
		if(current_letter == '\0') {
			return -1;
		}
		if((current_letter > '\0' && current_letter <= 8) ||
		   (current_letter >= 14 && current_letter <= 31) ||
		   current_letter >= 128)                            {
			fail = 1;
		}
		if(current_letter >= 'A' && current_letter <= 'Z') {
			current_letter += 97 - 65;
		}
		if(current_letter >= 'a' && current_letter <= 'z') {
			single_count[current_letter - 'a']++;
			letter_count++;
		}
	}
	if(letter_count == 0 || fail) {
		return 0;
	}
	float score = 26;
	for(size_t idx = 0; idx < 26; idx++) {
		score -= fabs(((float) single_count[idx]) / letter_count - letter_frequencies[idx]);
	}
	return score;
}

unsigned char** parse_lines(char* filename) {
	static unsigned char* lines[1024];
	size_t idx = 0;
	FILE* infile = fopen(filename, "r");
	while(!feof(infile)) {
		lines[idx] = malloc(1024 * sizeof(unsigned char));
		fscanf(infile, "%1023[^\n]\n", lines[idx]);
		idx++;
	}
	fclose(infile);
	lines[idx] = '\0';
	return lines;
}

size_t hamming_distance(unsigned char* str1, unsigned char* str2, size_t size) {
	size_t distance = 0;
	for(size_t idx = 0; idx < size; idx++) {
		unsigned char xor = str1[idx] ^ str2[idx];
		for(size_t bit = 0; bit < 8; bit++) {
			distance += (xor >> bit) & 1;
		}
	}
	return distance;
}

unsigned char* bytes_to_base64(unsigned char* str, size_t size) {
	static unsigned char output[256];
	size_t i_idx = 0;
	size_t o_idx = 0;
	while(i_idx + 3 <= size) {
		output[o_idx] = str[i_idx] >> 2;
		o_idx++;
		output[o_idx] = (str[i_idx] & 3) << 4;
		i_idx++;
		output[o_idx] += str[i_idx] >> 4;
		o_idx++;
		output[o_idx] = (str[i_idx] & 15) << 2;
		i_idx++;
		output[o_idx] += str[i_idx] >> 6;
		o_idx++;
		output[o_idx] = str[i_idx] & 63;
		o_idx++;
		i_idx++;
	}
	if(i_idx < size) {
		output[o_idx] = str[i_idx] >> 2;
		o_idx++;
		output[o_idx] = (str[i_idx] & 3) << 4;
		i_idx++;
		if(i_idx < size) {
			output[o_idx] += str[i_idx] >> 4;
			o_idx++;
			output[o_idx] = (str[i_idx] & 15) << 2;
			o_idx++;
			output[o_idx] = '=';
			o_idx++;
			output[o_idx] = '=';
			o_idx++;
		} else {
			o_idx++;
			output[o_idx] = '=';
			o_idx++;
		}
	}
	output[o_idx] = '\0';

	for(size_t idx = 0; idx < o_idx; idx++) {
		if(output[idx] < 26) {
			output[idx] += 'A';
		} else if(output[idx] < 52) {
			output[idx] += 'a' - 26;
		} else if(output[idx] < 62) {
			output[idx] += '0' - 52;
		} else if(output[idx] == 62) {
			output[idx] = '+';
		} else {
			output[idx] = '/';
		}
	}
	return output;
}

size_t base64_to_bytes(unsigned char* str, unsigned char* output) {
	size_t size = strlen((const char*) str);
	size_t i_idx = 0;
	size_t o_idx = 0;
	for(; i_idx < size; i_idx++) {
		if(str[i_idx] >= 'A' && str[i_idx] <= 'Z') {
			output[o_idx] = str[i_idx] - 'A';
		} else if(str[i_idx] >= 'a' && str[i_idx] <= 'z') {
			output[o_idx] = str[i_idx] - 'a' + 26;
		} else if(str[i_idx] >= '0' && str[i_idx] <= '9') {
			output[o_idx] = str[i_idx] - '0' + 52;
		} else if(str[i_idx] == '+') {
			output[o_idx] = 62;
		} else if(str[i_idx] == '/') {
			output[o_idx] = 63;
		} else if(str[i_idx] == '=') {
			output[o_idx] = 0;
			output[o_idx + 1] = 0;
		} else {
			o_idx--;
		}
		o_idx++;
	}
	size = o_idx;

	i_idx = 0;
	o_idx = 0;
	while(i_idx < size) {
		output[o_idx] = output[i_idx] << 2;
		i_idx++;
		if(i_idx == size) {
			break;
		}
		output[o_idx] += output[i_idx] >> 4;
		o_idx++;
		output[o_idx] = output[i_idx] << 4;
		i_idx++;
		if(i_idx == size) {
			break;
		}
		output[o_idx] += output[i_idx] >> 2;
		o_idx++;
		output[o_idx] = output[i_idx] << 6;
		i_idx++;
		if(i_idx == size) {
			break;
		}
		output[o_idx] += output[i_idx];
		i_idx++;
		o_idx++;
	}
	return o_idx;
}

unsigned char crack_single_xor(unsigned char* str, size_t size) {
	unsigned char highest_key = 0;
	float highest_score = FLT_MIN;
	unsigned char current_key = 0;
	while(1) {
		unsigned char* plaintext = malloc(size);
		for(size_t idx = 0; idx < size; idx++) {
			plaintext[idx] = str[idx] ^ current_key;
		}
		float current_score = english_score(plaintext, size);
		if(current_score > highest_score) {
			highest_key = current_key;
			highest_score = current_score;
		}
		if(current_key == 255) {
			break;
		}
		current_key++;
	}
	return highest_key;
}

void s1c1() {
	unsigned char** input = parse_lines("inputs1c1.txt");
	printf("%s\n", bytes_to_base64(input[0], hex_to_bytes(input[0])));
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
	size_t size = hex_to_bytes(input[0]);
	unsigned char key = crack_single_xor(input[0], size);
	unsigned char output[256];
	for(size_t idx = 0; idx < size; idx++) {
		output[idx] = input[0][idx] ^ key;
	}
	output[size] = '\0';
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
			float current_score = english_score(output, input_size);
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

void s1c6() {
	FILE* infile = fopen("inputs1c6.txt", "r");
	fseek(infile, 0L, SEEK_END);
	size_t size = ftell(infile);
	fseek(infile, 0L, SEEK_SET);
	unsigned char* input = malloc(size + 1);
	fread(input, 1, size, infile);
	fclose(infile);
	input[size] = '\0';
	unsigned char* bytes = malloc(size);
	size = base64_to_bytes(input, bytes);

	size_t smallest_key_len = -1;
	float smallest_score = FLT_MAX;
	unsigned char test_blocks[4][40];
	for(size_t test_key_len = 2; test_key_len <= 40; test_key_len++) {
		memcpy(test_blocks[0], bytes, test_key_len);
		memcpy(test_blocks[1], bytes + test_key_len, test_key_len);
		memcpy(test_blocks[2], bytes + 2 * test_key_len, test_key_len);
		memcpy(test_blocks[3], bytes + 3 * test_key_len, test_key_len);
		float score = hamming_distance(test_blocks[0], test_blocks[1], test_key_len);
		score += hamming_distance(test_blocks[0], test_blocks[2], test_key_len);
		score += hamming_distance(test_blocks[0], test_blocks[3], test_key_len);
		score += hamming_distance(test_blocks[1], test_blocks[2], test_key_len);
		score += hamming_distance(test_blocks[1], test_blocks[3], test_key_len);
		score += hamming_distance(test_blocks[2], test_blocks[3], test_key_len);
		score /= test_key_len;
		if(score < smallest_score) {
			smallest_key_len = test_key_len;
			smallest_score = score;
		}
	}

	unsigned char key[41];
	for(size_t idx = 0; idx < smallest_key_len; idx++) {
		unsigned char* subkey_input = malloc(size / smallest_key_len);
		for(size_t idx2 = 0; idx2 < size / smallest_key_len; idx2++) {
			subkey_input[idx2] = bytes[idx2 * smallest_key_len + idx];
		}
		key[idx] = crack_single_xor(subkey_input, size / smallest_key_len);
	}
	key[smallest_key_len] = '\0';

	for(size_t idx = 0; idx < size; idx++) {
		bytes[idx] = bytes[idx] ^ key[idx % smallest_key_len];
	}
	bytes[size] = '\0';
	printf("%s\n", bytes);
}

void s1c7() {
	FILE* infile = fopen("inputs1c7.txt", "r");
	fseek(infile, 0L, SEEK_END);
	size_t size = ftell(infile);
	fseek(infile, 0L, SEEK_SET);
	unsigned char* input = malloc(size + 1);
	fread(input, 1, size, infile);
	fclose(infile);
	input[size] = '\0';
	unsigned char* bytes = malloc(size);
	size = base64_to_bytes(input, bytes);
	rijndael_decrypt_ecb(bytes, size, (unsigned char*) "YELLOW SUBMARINE");
	bytes[size] = '\0';
	printf("%s\n", bytes);
}

void s1c8() {
	unsigned char** input = parse_lines("inputs1c8.txt");
	for(size_t current_line = 0; input[current_line] != '\0'; current_line++) {
		unsigned char* bytes = malloc(strlen((const char*) input[current_line]));
		size_t line_length = base64_to_bytes(input[current_line], bytes);
		size_t matches = 0;
		for(size_t start = 0; start < line_length - 16; start += 16) {
			for(size_t next = start + 16; next < line_length; next += 16) {
				if(!memcmp((const char*) (bytes + start),
				           (const char*) (bytes + next), 16)) {
					matches++;
				}
			}
		}
		if(matches) {
			printf("%s\n", input[current_line]);
		}
	}
}

void s2c9() {
	char input[16] = "YELLOW SUBMARINE";
	char output[21];
	memcpy(output, input, 16);
	output[20] = '\0';
	char pad = 20 - 16;
	for(size_t idx = 16; idx < 20; idx++) {
		output[idx] = pad;
	}
	printf("%s\n", output);
}

void s2c10() {
	FILE* infile = fopen("inputs2c10.txt", "r");
	fseek(infile, 0L, SEEK_END);
	size_t size = ftell(infile);
	fseek(infile, 0L, SEEK_SET);
	unsigned char* input = malloc(size + 1);
	fread(input, 1, size, infile);
	fclose(infile);
	input[size] = '\0';
	unsigned char* bytes = malloc(size);
	size = base64_to_bytes(input, bytes);
	unsigned char iv[16] = { 0, 0, 0, 0, 0, 0, 0, 0,
	                         0, 0, 0, 0, 0, 0, 0, 0  };
	rijndael_decrypt_cbc(bytes, size, (unsigned char*) "YELLOW SUBMARINE", iv);
	bytes[size] = '\0';
	printf("%s\n", bytes);
}

void s2c11() {
	unsigned char* data = (unsigned char*) "YELLOW SUBMARINEYELLOW SUBMARINEYELLOW SUBMARINEYELLOW SUBMARINE";
	for(size_t round = 0; round < 16; round++) {
		int round_type = rand() % 2;
		if(round_type == 0) {
			fputs("Using ECB... ", stdout);
		} else {
			fputs("Using CBC... ", stdout);
		}
		unsigned char padded_data[80];
		size_t pad_start = 4 + (rand() % 8);
		memcpy(padded_data + pad_start, data, 64);
		for(size_t idx = 0; idx < pad_start; idx++) {
			padded_data[idx] = rand() % 256;
		}
		for(size_t idx = pad_start + 64; idx < 80; idx++) {
			padded_data[idx] = rand() % 256;
		}
		unsigned char key[16];
		unsigned char iv[16];
		for(size_t idx = 0; idx < 16; idx++) {
			key[idx] = rand() % 256;
			iv[idx] = rand() % 256;
		}
		if(round_type == 0) {
			rijndael_encrypt_ecb(padded_data, 80, key);
		} else {
			rijndael_encrypt_cbc(padded_data, 80, key, iv);
		}
		int detected_type;
		if(!memcmp(padded_data + 32, padded_data + 48, 16)) {
			detected_type = 0;
		} else {
			detected_type = 1;
		}
		if(round_type == detected_type) {
			puts("successful detection");
		} else {
			puts("unsuccessful detection");
		}
	}
}

void s2c12() {
	FILE* infile = fopen("inputs2c12.txt", "r");
	fseek(infile, 0L, SEEK_END);
	size_t size = ftell(infile);
	fseek(infile, 0L, SEEK_SET);
	unsigned char* input = malloc(size + 1);
	fread(input, 1, size, infile);
	fclose(infile);
	input[size] = '\0';
	unsigned char* secret_data = malloc(size);
	size = base64_to_bytes(input, secret_data);

	unsigned char key[16];
	for(size_t idx = 0; idx < 16; idx++) {
		key[idx] = rand() % 256;
	}
	unsigned char check_bsize_data[32];
	unsigned char last_data[32];
	size_t current_bsize = 1;
	while(1) {
		for(size_t idx = 0; idx < current_bsize; idx++) {
			check_bsize_data[idx] = 'A';
		}
		rijndael_encrypt_ecb(check_bsize_data, current_bsize, key);
		if(current_bsize > 1 && !memcmp(last_data, check_bsize_data, current_bsize - 1)) {
			break;
		}
		memcpy(last_data, check_bsize_data, current_bsize);
		current_bsize++;
	}
	printf("Determined block size to be %lu\n", current_bsize - 1);

	unsigned char current_dictionary[256][16];
	unsigned char test_data[15] = "AAAAAAAAAAAAAAA";
	unsigned char* decrypted_data = malloc(size + 1);
	for(size_t cur_byte = 0; cur_byte < size; cur_byte++) {
		for(size_t dict_idx = 0; dict_idx < 256; dict_idx++) {
			memcpy(current_dictionary[dict_idx], test_data, 15);
			current_dictionary[dict_idx][15] = dict_idx;
			rijndael_encrypt_ecb(current_dictionary[dict_idx], 16, key);
		}
		unsigned char test_block[16];
		memcpy(test_block, test_data, 15);
		test_block[15] = secret_data[cur_byte];
		rijndael_encrypt_ecb(test_block, 16, key);
		for(size_t dict_idx = 0; dict_idx < 256; dict_idx++) {
			if(!memcmp(current_dictionary[dict_idx], test_block, 16)) {
				decrypted_data[cur_byte] = dict_idx;
				decrypted_data[cur_byte + 1] = '\0';
				printf("%s\n", decrypted_data);
			}
		}
	}
}

int main() {
	srand(time(0));
	s1c1();
	s1c2();
	s1c3();
	s1c4();
	s1c5();
	s1c6();
	s1c7();
	s1c8();
	s2c9();
	s2c10();
	s2c11();
	s2c12();
	return 0;
}
