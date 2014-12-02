#include <stdio.h>

void hex_to_bytes(unsigned char* str) {
	size_t idx;
	for(idx = 0; str[idx] != '\0'; idx++) {
		if(str[idx] <= '9') {
			str[idx] -= 48;
		} else {
			str[idx] -= 87;
		}
	}

	for(idx = 0; str[idx * 2] != '\0'; idx++) {
		str[idx] = str[idx * 2] << 4;
		str[idx] += str[idx * 2 + 1];
	}
	str[idx] = '\0';
}

int main() {
	unsigned char input[256];
	scanf("%255s\n", input);
	hex_to_bytes(input);
	unsigned char output[256];

	size_t i_idx = 0;
	size_t o_idx = 0;
	while(input[i_idx]     != '\0' && input[i_idx + 1] != '\0' &&
	      input[i_idx + 2] != '\0' ) {
		output[o_idx] = input[i_idx] >> 2;
		output[o_idx + 1] = (input[i_idx] & 3) << 4;
		output[o_idx + 1] += input[i_idx + 1] >> 4;
		output[o_idx + 2] = (input[i_idx + 1] & 15) << 2;
		output[o_idx + 2] += input[i_idx + 2] >> 6;
		output[o_idx + 3] = input[i_idx + 2] & 63;
		i_idx += 3;
		o_idx += 4;
	}
	if(input[i_idx] != '\0') {
		output[o_idx] = input[i_idx] >> 2;
		output[o_idx + 1] = (input[i_idx] & 3) << 4;
		i_idx++;
		o_idx++;
		if(input[i_idx] != '\0') {
			output[o_idx] += input[i_idx] >> 4;
			output[o_idx + 1] = (input[i_idx] & 15) << 2;
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
	return 0;
}
