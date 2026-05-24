/************************************************************************
* Copyright (c) 2018 DonghaiYang
* File Name	: AES.h
* Abstract	:
* Comments	:
* Version	: V1.1
* Author	: DonghaiYang
* Date		: 2018-04-28
* Modified1	:
* Modified2	:
************************************************************************/



#include "AES.h"
#include "string.h"
#include "math.h"
//#include "offline_password_use.h"

//#define AES_DEBUG

#ifdef AES_DEBUG
#include "CH58x_common.h"
#endif

#define APP_LX_AES
#ifdef APP_LX_AES

void InitAES(struct AES* aes, unsigned char* key)
{

	unsigned char sBox[] =
	{ /*  0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f */ 
		0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76, /*0*/  
		0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0, /*1*/
		0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15, /*2*/ 
		0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75, /*3*/ 
		0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84, /*4*/ 
		0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf, /*5*/
		0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8, /*6*/  
		0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2, /*7*/ 
		0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73, /*8*/ 
		0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb, /*9*/ 
		0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79, /*a*/
		0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08, /*b*/
		0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a, /*c*/ 
		0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e, /*d*/
		0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf, /*e*/ 
		0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16  /*f*/
	};
	unsigned char invsBox[256] = 
	{ /*  0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f  */  
		0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb, /*0*/ 
		0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb, /*1*/
		0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e, /*2*/ 
		0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25, /*3*/ 
		0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92, /*4*/ 
		0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84, /*5*/ 
		0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06, /*6*/ 
		0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b, /*7*/
		0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73, /*8*/ 
		0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e, /*9*/
		0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b, /*a*/
		0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4, /*b*/ 
		0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f, /*c*/ 
		0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef, /*d*/ 
		0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61, /*e*/ 
		0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d  /*f*/
	}; 
	
	aes->SetKey = SetKey;
	aes->Cipher = Cipher;
	aes->InvCipher = InvCipher;
	aes->KeyExpansion = KeyExpansion;
	aes->FFmul = FFmul;
	aes->SubBytes = SubBytes;
	aes->ShiftRows = ShiftRows;
	aes->MixColumns = MixColumns;
	aes->AddRoundKey = AddRoundKey;
	aes->InvSubBytes = InvSubBytes;
	aes->InvShiftRows = InvShiftRows;
	aes->InvMixColumns = InvMixColumns;
	memcpy(aes->Sbox, sBox, 256);
	memcpy(aes->InvSbox, invsBox, 256);
	if (key != 0) 
	{
		aes->KeyExpansion(aes,key, aes->w);
	}
}

void SetKey(struct AES* aes, unsigned char *key) 
{
	aes->KeyExpansion(aes, key, aes->w);
}

unsigned char* Cipher(struct AES* aes, unsigned char* input, unsigned char *output)
{
	unsigned char state[4][4];
	int i,r,c;

	for(r=0; r<4; r++)
	{
		for(c=0; c<4 ;c++)
		{
			state[r][c] = input[c*4+r];
		}
	}

	aes->AddRoundKey(state, aes->w[0]);

	for(i=1; i<=10; i++)
	{
		aes->SubBytes(aes,state);
		aes->ShiftRows(state);
		if (i != 10)
			aes->MixColumns(state);
		aes->AddRoundKey(state, aes->w[i]);
	}

	for(r=0; r<4; r++)
	{
		for(c=0; c<4 ;c++)
		{
			output[c*4+r] = state[r][c];
		}
	}

	return output;
}

unsigned char* InvCipher(struct AES* aes, unsigned char* input, unsigned char *output)
{
	unsigned char state[4][4];
	int i,r,c;

	for(r=0; r<4; r++)
	{
		for(c=0; c<4 ;c++)
		{
			state[r][c] = input[c*4+r];
		}
	}

	aes->AddRoundKey(state, aes->w[10]);
	for(i=9; i>=0; i--)
	{
		aes->InvShiftRows(state);
		aes->InvSubBytes(aes,state);
		aes->AddRoundKey(state, aes->w[i]);
		if(i)
		{
			aes->InvMixColumns(state);
		}
	}
	
	for(r=0; r<4; r++)
	{
		for(c=0; c<4 ;c++)
		{
			output[c*4+r] = state[r][c];
		}
	}
	return output;
}

void KeyExpansion(struct AES* aes, unsigned char* key, unsigned char w[][4][4])
{
	int i,j,r,c;
	unsigned char rc[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};
	for(r=0; r<4; r++)
	{
		for(c=0; c<4; c++)
		{
			w[0][r][c] = key[r+c*4];
		}
	}
	for(i=1; i<=10; i++)
	{
		for(j=0; j<4; j++)
		{
			unsigned char t[4];
			for(r=0; r<4; r++)
			{
				t[r] = j ? w[i][r][j-1] : w[i-1][r][3];
			}
			if(j == 0)
			{
				unsigned char temp = t[0];
				for(r=0; r<3; r++)
				{
					t[r] = aes->Sbox[t[(r + 1) % 4]];
				}
				t[3] = aes->Sbox[temp];
				t[0] ^= rc[i-1];
			}
			for(r=0; r<4; r++)
			{
				w[i][r][j] = w[i-1][r][j] ^ t[r];
			}
		}
	}
}

unsigned char FFmul(unsigned char a, unsigned char b)
{
	unsigned char bw[4];
	unsigned char res=0;
	int i;
	bw[0] = b;
	for(i=1; i<4; i++)
	{
		bw[i] = bw[i-1]<<1;
		if(bw[i-1]&0x80)
		{
			bw[i]^=0x1b;
		}
	}
	for(i=0; i<4; i++)
	{
		if((a>>i)&0x01)
		{
			res ^= bw[i];
		}
	}
	return res;
}

void SubBytes(struct AES* aes, unsigned char state[][4])
{
	int r,c;
	for(r=0; r<4; r++)
	{
		for(c=0; c<4; c++)
		{
			state[r][c] = aes->Sbox[state[r][c]];
		}
	}
}

void ShiftRows(unsigned char state[][4])
{
	unsigned char t[4];
	int r,c;
	for(r=1; r<4; r++)
	{
		for(c=0; c<4; c++)
		{
			t[c] = state[r][(c+r)%4];
		}
		for(c=0; c<4; c++)
		{
			state[r][c] = t[c];
		}
	}
}

void MixColumns(unsigned char state[][4])
{
	unsigned char t[4];
	int r,c;
	for(c=0; c< 4; c++)
	{
		for(r=0; r<4; r++)
		{
			t[r] = state[r][c];
		}
		for(r=0; r<4; r++)
		{
			state[r][c] = FFmul(0x02, t[r])
						^ FFmul(0x03, t[(r+1)%4])
						^ FFmul(0x01, t[(r+2)%4])
						^ FFmul(0x01, t[(r+3)%4]);
		}
	}
}

void AddRoundKey(unsigned char state[][4], unsigned char k[][4])
{
	int r,c;
	for(c=0; c<4; c++)
	{
		for(r=0; r<4; r++)
		{
			state[r][c] ^= k[r][c];
		}
	}
}

void InvSubBytes(struct AES* aes, unsigned char state[][4])
{
	int r,c;
	for(r=0; r<4; r++)
	{
		for(c=0; c<4; c++)
		{
			state[r][c] = aes->InvSbox[state[r][c]];
		}
	}
}

void InvShiftRows(unsigned char state[][4])
{
	unsigned char t[4];
	int r,c;
	for(r=1; r<4; r++)
	{
		for(c=0; c<4; c++)
		{
			t[c] = state[r][(c-r+4)%4];
		}
		for(c=0; c<4; c++)
		{
			state[r][c] = t[c];
		}
	}
}

void InvMixColumns(unsigned char state[][4])
{
	unsigned char t[4];
	int r,c;
	for(c=0; c< 4; c++)
	{
		for(r=0; r<4; r++)
		{
			t[r] = state[r][c];
		}
		for(r=0; r<4; r++)
		{
			state[r][c] = FFmul(0x0e, t[r])
						^ FFmul(0x0b, t[(r+1)%4])
						^ FFmul(0x0d, t[(r+2)%4])
						^ FFmul(0x09, t[(r+3)%4]);
		}
	}
}

/**************************************************************/
//AESModeOfOperation

void InitAESModeOfTask(struct AESModeOfTask * aesTask) {
	aesTask->mode = MODE_CFB;
	memset(aesTask->key, 0, 16);
	memset(aesTask->iv, 0, 16);

	(aesTask->aes).InitAES(&(aesTask->aes), aesTask->key);
}

void set_mode(struct AESModeOfTask * aesTask, int _mode) {
	aesTask->mode = _mode;
}

void set_key(struct AESModeOfTask * aesTask, unsigned char *_key) {
//	assert(_key != NULL);
	memcpy(aesTask->key, _key, 16);
	(aesTask->aes).SetKey(&(aesTask->aes),aesTask->key);
}

void set_iv(struct AESModeOfTask * aesTask, unsigned char *_iv) {
//	assert(_iv != NULL);
	memcpy(aesTask->iv, _iv, 16);
}

int Encrypt(struct AESModeOfTask * aesTask, unsigned char *_in, int _length, unsigned char *_out)
{
	int first_round = 0;
	int rounds = 0;
	int start = 0;
	int end = 0;
	unsigned char input[16] = {0};
	unsigned char output[16] = {0};
	unsigned char ciphertext[16] = {0};
	unsigned char cipherout[256] = {0};
	unsigned char plaintext[16] = {0};
	int co_index = 0;
  int i,j,k;
	//
	if ( _length % 16 == 0)
	{
		rounds = _length / 16;
	}
	else
	{
		rounds = _length / 16 + 1;
	}
	//
	for (j = 0; j < rounds; ++j)
	{
		start = j*16;
		end = j*16 + 16;
		if (end > _length)
		{
			end = _length;
		}
		//
		memset(plaintext, 0, 16);
		memcpy(plaintext, _in + start, end - start);
		//
		if (aesTask->mode == MODE_CFB)
		{
			if (0 == first_round)
			{
				(aesTask->aes).Cipher(&(aesTask->aes), aesTask->iv, output);
				first_round = 1;
			} 
			else
			{
				(aesTask->aes).Cipher(&(aesTask->aes), input, output);
			}
			for (i = 0; i < 16; ++i)// ciphertext = plaintext ^ output
			{
				if ( (end - start) - 1 < i)
				{
					ciphertext[i] = 0 ^ output[i];
				} 
				else
				{
					ciphertext[i] = plaintext[i] ^ output[i];
				}
			}
			for (k = 0 ; k < end - start; ++k)
			{
				cipherout[co_index++] = ciphertext[k];
			}
			memcpy(input,ciphertext, 16);
		}
		else if (aesTask->mode == MODE_OFB)// MODE_OFB
		{
			if (0 == first_round)
			{
				(aesTask->aes).Cipher(&(aesTask->aes), aesTask->iv, output); 
				first_round = 1;
			}
			else
			{
				(aesTask->aes).Cipher(&(aesTask->aes), input, output);
			}
			for (i = 0; i < 16; ++i)// ciphertext = plaintext ^ output
			{
				if ( (end - start) - 1 < i)
				{
					ciphertext[i] = 0 ^ output[i];
				}
				else
				{
					ciphertext[i] = plaintext[i] ^ output[i];
				}
			}
			for (k = 0; k < end - start; ++k)
			{
				cipherout[co_index++] = ciphertext[k];
			}
			memcpy(input, output, 16);
		}
	}
	memcpy(_out, cipherout, co_index);

	return co_index;
}

int Decrypt(struct AESModeOfTask * aesTask, unsigned char *_in, int _length, unsigned char *_out) 
{
	int first_round = 0;
	int rounds = 0;
	unsigned char ciphertext[16] = {0};
	unsigned char input[16] = {0};
	unsigned char output[16] = {0};
	unsigned char plaintext[16] = {0};
	unsigned char plainout[256] = {0};
	int po_index = 0 ;
  int j,i,k;	
  int start = 0;
	int end = 0;
	if (_length % 16 == 0)
	{
		rounds = _length / 16;
	}
	else
	{
		rounds = _length / 16 + 1;
	}
	

	
	for (j = 0; j < rounds; j++)
	{
		start = j * 16;
		end   = start + 16;
		if ( end > _length)
		{
			end = _length;
		}
		memset(ciphertext, 0, 16);
		memcpy(ciphertext, _in + start, end - start);
		if (aesTask->mode == MODE_CFB)
		{
			if (0 == first_round)
			{
				(aesTask->aes).Cipher(&(aesTask->aes), aesTask->iv, output);
				first_round = 1;
			}
			else
			{
				(aesTask->aes).Cipher(&(aesTask->aes), input, output);
			}
			for (i = 0; i < 16; i++)
			{
				if ( end - start - 1 < i)
				{
					plaintext[i] = output[i] ^ 0;
				}
				else
				{
					plaintext[i] = output[i] ^ ciphertext[i];
				}
			}
			for (k = 0; k < end - start; ++k)
			{
				plainout[po_index++] = plaintext[k];
			}
			//memset(input, 0, 16);
			memcpy(input, ciphertext, 16);
		}
		else if (aesTask->mode == MODE_OFB)
		{
			if (0 == first_round)
			{
				(aesTask->aes).Cipher(&(aesTask->aes), aesTask->iv, output);
				first_round = 1;
			}
			else
			{
				(aesTask->aes).Cipher(&(aesTask->aes), input, output);
			}
			for (i = 0; i < 16; i++)
			{
				if ( end - start -1 < i)
				{
					plaintext[i] = 0 ^ ciphertext[i];
					first_round = 1;
				}
				else
				{
					plaintext[i] = output[i] ^ ciphertext[i];
				}
			}
			for (k = 0; k < end - start; ++k)
			{
				plainout[po_index++] = plaintext[k];
			}
			memcpy(input, output, 16);
		}
	}
    memcpy(_out, plainout, po_index);
    return po_index;
}


//���ܣ�������ʵ�ֺ���
//���ߣ�������
//�������ڣ�2018-05-08
//�汾:V1.0
void fun_checkmynyr(unsigned char k);
extern void ret_times(unsigned char *rtimes);
unsigned char  output_aes[16];//�����AES������
unsigned char  TM;//ģʽ
unsigned char  yy1,mm1,dd1;//������1
unsigned char  yy2,mm2,dd2;//������2
unsigned char  h1,m1,h2,m2;//ʱ��1&ʱ��2
unsigned int   suiji_pw;//�������
unsigned char  lock_tmp;//���ƴ���
unsigned char  cfb_len1 = 4;
unsigned char  pw_starttimes[6]={0};
unsigned long long long_pw;
unsigned long long long_pw1;
unsigned long long long_pw2;
unsigned long long_pw3;
unsigned char Hex_Bin[80];
unsigned char lx_record_buf[32];
//7����Կ,0-3�ֽ�Ϊ lock_manger
//unsigned char AES_Key_Table11[16] = { 91, 9, 93, 93, 176, 178, 97, 98, 99, 121, 101, 22, 189, 98, 115, 201 };
//unsigned char AES_Key_Table22[16] = { 89, 92, 93, 93, 178, 252, 97, 38, 89, 101, 101, 12, 176, 63, 125, 203 };
//unsigned char AES_Key_Table33[16] = { 69, 12, 05, 93, 180, 230, 38, 68, 79, 109, 101, 28, 150, 55, 135, 213 };
//unsigned char AES_Key_Table44[16] = { 62, 16, 98, 93, 198, 129, 26, 28, 69, 201, 101, 62, 133, 48, 145, 216 };
//unsigned char AES_Key_Table55[16] = { 31, 92, 06, 93, 212, 118, 98, 18, 49, 220, 101, 32, 123, 31, 148, 218 };
//unsigned char AES_Key_Table66[16] = { 19, 37, 98, 93, 233, 115, 86, 25, 99, 234, 101, 52, 108, 25, 156, 233 };
//unsigned char AES_Key_Table77[16] = { 28, 92, 18, 93, 216, 113, 83, 68, 39, 236, 101, 65, 102, 16, 168, 248 };
//unsigned char AES_IV11[16] = { 98, 49, 98, 50, 99, 51, 100, 52, 101, 53, 102, 54, 103, 55, 104, 56 };
//unsigned char AES_IV22[16] = { 19, 37, 98, 93, 233, 115, 86, 25, 99, 234, 101, 52, 108, 25, 156, 233 };
//unsigned char AES_IV33[16] = { 28, 92, 18, 93, 216, 113, 83, 68, 39, 236, 101, 65, 102, 16, 168, 248 };
//unsigned char AES_IV44[16] = { 62, 16, 98, 93, 198, 129, 26, 28, 69, 201, 101, 62, 133, 48, 145, 216 };
//unsigned char AES_IV55[16] = { 91, 9, 93, 93, 176, 178, 97, 98, 99, 121, 101, 22, 189, 98, 115, 201 };
//unsigned char AES_IV66[16] = { 31, 92, 06, 93, 212, 118, 98, 18, 49, 220, 101, 32, 123, 31, 148, 218 };
//unsigned char AES_IV77[16] = { 69, 12, 05, 93, 180, 230, 38, 68, 79, 109, 101, 28, 150, 55, 135, 213 };

unsigned char AES_Key_Table1[16] = { 201, 9, 93, 93, 176, 178, 97, 98, 99, 121, 101, 22, 189, 98, 115, 91 };
unsigned char AES_Key_Table2[16] = { 203, 92, 93, 93, 178, 252, 97, 38, 89, 101, 101, 12, 176, 63, 125, 89 };
unsigned char AES_Key_Table3[16] = { 213, 12, 05, 93, 180, 230, 38, 68, 79, 109, 101, 28, 150, 55, 135, 69 };
unsigned char AES_Key_Table4[16] = { 216, 16, 98, 93, 198, 129, 26, 28, 69, 201, 101, 62, 133, 48, 145, 62 };
unsigned char AES_Key_Table5[16] = { 218, 92, 06, 93, 212, 118, 98, 18, 49, 220, 101, 32, 123, 31, 148, 31 };
unsigned char AES_Key_Table6[16] = { 233, 37, 98, 93, 233, 115, 86, 25, 99, 234, 101, 52, 108, 25, 156, 19 };
unsigned char AES_Key_Table7[16] = { 248, 92, 18, 93, 216, 113, 83, 68, 39, 236, 101, 65, 102, 16, 168, 28 };

// unsigned char AES_IV1[16] = { 56, 49, 98, 50, 99, 51, 100, 52, 101, 53, 102, 54, 103, 55, 104, 98 };
// unsigned char AES_IV2[16] = { 233, 37, 98, 93, 233, 115, 86, 25, 99, 234, 101, 52, 108, 25, 156, 19 };
// unsigned char AES_IV3[16] = { 248, 92, 18, 93, 216, 113, 83, 68, 39, 236, 101, 65, 102, 16, 168, 28 };
// unsigned char AES_IV4[16] = { 216, 16, 98, 93, 198, 129, 26, 28, 69, 201, 101, 62, 133, 48, 145, 62 };
// unsigned char AES_IV5[16] = { 201, 9, 93, 93, 176, 178, 97, 98, 99, 121, 101, 22, 189, 98, 115, 91 };
// unsigned char AES_IV6[16] = { 218, 92, 06, 93, 212, 118, 98, 18, 49, 220, 101, 32, 123, 31, 148, 31 };
// unsigned char AES_IV7[16] = { 213, 12, 05, 93, 180, 230, 38, 68, 79, 109, 101, 28, 150, 55, 135, 69 };

//����ת��20-->0x20
unsigned char fun_dectohex(unsigned char vdata)
{
	return (((vdata/10)<<4)|(vdata%10));
}

//16����ת������
//*p���������
//snum ת���ĸ���
//��ģʽ0��fun_hextobin(output_aes,4);
unsigned char fun_hextobin(unsigned char *p,unsigned char snum)
{	
	unsigned char i,j;
	unsigned char k;
	for(j=0;j<snum;j++)
	{
		k = 0x80;
		for(i=0;i<8;i++)
		{
			if((*p)&k) Hex_Bin[j*8+i]=1;
			else Hex_Bin[j*8+i] = 0;
			k=k>>1;
		}
		p++;
  }
  return 0;
}
//2����ת10����
//*p���������
//pnumת����bit����
unsigned int fun_bintodec(unsigned char *p,unsigned char pnum)
{
	unsigned char i;
	unsigned char k;
	unsigned int  bin_int=0;

	k = pnum;
	for(i=0;i<k;i++)
	{
		bin_int += ((*p)<<(pnum-1));
		pnum--;
		p++;	    	
	}
	return bin_int;
}

/*********************************************************
;����ת��
;0x20-->20
**********************************************************/
unsigned char fun_hextodec(unsigned char y)
{
	return (y=(y/16*10)+(y%16));
}

/************************************ʱ��ת������****************************************/
void fun_timesTODateTime(unsigned char timezone,unsigned long timestamp,unsigned char *ptimes)
{
	unsigned int i,j,iDay;
	unsigned long lDay,lSec;
	unsigned char fun_timezone = 0;
	unsigned char DayOfMon[12]={31,28,31,30,31,30,31,31,30,31,30,31};
	fun_timezone = timezone;
	if((timezone&0x7f)>12)  fun_timezone = 0x08;
	lSec = timestamp;

	if(fun_timezone&0x80)   //��ʱ��������ʽ
	{
       lSec -= (fun_timezone&0x7f)*3600;
	}
	else 
	{
       lSec += (fun_timezone&0x7f)*3600;
	}
	
	lDay = lSec / 86400;
	lSec = lSec % 86400;
	i = 1970;
	while(lDay > 365)
	{
			if(((i%4==0) && (i%100!=0)) || (i%400==0))
			{
				lDay -= 366;
			}
			else
			{
				lDay -= 365;
			}
			i++;
	}
	if((lDay == 365) && !(((i%4==0)&&(i%100!=0)) || (i%400==0)))
	{
	lDay -= 365;
	i++;
	}	

	ptimes[0] = i-2000;
	for(j=0;j<12;j++)
	{
		if((j==1) && (((i%4==0)&&(i%100!=0)) || (i%400==0)))
		{
			iDay = 29;
		}else
		{
			iDay = DayOfMon[j];
		}
		if(lDay >= iDay)
		{			
			lDay -= iDay;
		}
		else
		{
		  break;
		}			
	}
  ptimes[1] = j+1;
  ptimes[2] = lDay+1;
  ptimes[3] = ((lSec / 3600))%24;
  ptimes[4] = (lSec % 3600) / 60;
  ptimes[5] = (lSec % 3600) % 60;	
}
void fun_timestampTODateTime(unsigned char timestamp[5],unsigned char *ptimes)
{
	unsigned long lSec;

	lSec = (timestamp[1]<<24)+(timestamp[2]<<16)+(timestamp[3]<<8)+timestamp[4];
	fun_timesTODateTime(timestamp[0],lSec,ptimes);
}

#define  FUN_SECOND_OF_DAY  86400  
unsigned char FunDayOfMon[12]={31,28,31,30,31,30,31,31,30,31,30,31};
//===========������ת��Ϊʱ���=============//
unsigned long fun_GetSecondTime(unsigned char timezone,unsigned char *p)
{
	unsigned int rYear,i,Cyear=0;
	unsigned char rMon,rDay,rHour,rMin,rSec;
    unsigned long CountDay = 0;
	unsigned char fun_timezone = 0;

	fun_timezone = timezone;
	if((timezone&0x7f)>12)  fun_timezone = 0x08;

	p[5] = 0;
	rSec =p[5];
	rMin =p[4];
	rHour =p[3];
	rDay =p[2];
	rMon =p[1];
	rYear= 2000+p[0];


	for(i = 1970; i < rYear; i++)
  {
   if(((i%4==0) && (i%100!=0)) || (i%400==0)) Cyear++;
  }
	CountDay = Cyear * 366 + (rYear-1970-Cyear) * 365;
	
	for(i=1; i<rMon; i++)
  {
   if((i==2) && (((rYear%4==0)&&(rYear%100!=0)) || (rYear%400==0)))
   CountDay += 29;
   else
   CountDay += FunDayOfMon[i-1];
 }
  CountDay += (rDay-1);
  CountDay = CountDay*FUN_SECOND_OF_DAY + (unsigned long)rHour*3600 + (unsigned long)rMin*60 + rSec;

  //CountDay = CountDay - 8*3600;

	if(fun_timezone&0x80)   //��ʱ��������ʽ
	{
		CountDay -= (fun_timezone&0x7f)*3600;
	}
	else 
	{
		CountDay += (fun_timezone&0x7f)*3600;
	}
  return CountDay;	
}

/*������ܼ�*/
unsigned char fun_GetWeekDayNum(unsigned int syear, unsigned char smonth, unsigned char sday)
{
  unsigned char weekday = 0U;

  if (smonth < 3U)
  	 weekday = (((23U * smonth) / 9U) + sday + 4U + syear + ((syear - 1U) / 4U) - ((syear - 1U) / 100U) + ((syear - 1U) / 400U)) % 7U;
  else
	   weekday = (((23U * smonth) / 9U) + sday + 4U + syear + (syear / 4U) - (syear / 100U) + (syear / 400U) - 2U) % 7U;

  if(weekday == 0)
  {
	  weekday = 7;
  }

  return weekday;
}
//---------BLE�ӽ���----------
//unsigned char const AES_Key_Table88[16] = { 91, 9, 93, 93, 176, 178, 97, 98, 99, 121, 101, 22, 189, 98, 115, 201 };
//unsigned char const AES_Key_Table99[16] = { 89, 92, 93, 93, 178, 252, 97, 38, 89, 101, 101, 12, 176, 63, 125, 203 };
//unsigned char const AES_Key_Table101[16] = { 69, 12, 05, 93, 180, 230, 38, 68, 79, 109, 101, 28, 150, 55, 135, 213 };
//unsigned char const AES_Key_Table111[16] = { 62, 16, 98, 93, 198, 129, 26, 28, 69, 201, 101, 62, 133, 48, 145, 216 };
//unsigned char const AES_Key_Table121[16] = { 31, 92, 06, 93, 212, 118, 98, 18, 49, 220, 101, 32, 123, 31, 148, 218 };
//unsigned char const AES_Key_Table131[16] = { 19, 37, 98, 93, 233, 115, 86, 25, 99, 234, 101, 52, 108, 25, 156, 233 };
//unsigned char const AES_Key_Table141[16] = { 28, 92, 18, 93, 216, 113, 83, 68, 39, 236, 101, 65, 102, 16, 168, 248 };
unsigned char const AES_Key_Table88[16] = { 201, 9, 93, 93, 176, 178, 97, 98, 99, 121, 101, 22, 189, 98, 115, 91 };
unsigned char const AES_Key_Table99[16] = { 203, 92, 93, 93, 178, 252, 97, 38, 89, 101, 101, 12, 176, 63, 125, 89 };
unsigned char const AES_Key_Table101[16] = { 213, 12, 05, 93, 180, 230, 38, 68, 79, 109, 101, 28, 150, 55, 135, 69 };
unsigned char const AES_Key_Table111[16] = { 216, 16, 98, 93, 198, 129, 26, 28, 69, 201, 101, 62, 133, 48, 145, 62 };
unsigned char const AES_Key_Table121[16] = { 218, 92, 06, 93, 212, 118, 98, 18, 49, 220, 101, 32, 123, 31, 148, 31 };
unsigned char const AES_Key_Table131[16] = { 233, 37, 98, 93, 233, 115, 86, 25, 99, 234, 101, 52, 108, 25, 156, 19 };
unsigned char const AES_Key_Table141[16] = { 248, 92, 18, 93, 216, 113, 83, 68, 39, 236, 101, 65, 102, 16, 168, 28 };

unsigned char AES_IV1[16] = { 56, 49, 98, 50, 99, 51, 100, 52, 101, 53, 102, 54, 103, 55, 104, 98 };
unsigned char AES_IV2[16] = { 233, 37, 98, 93, 233, 115, 86, 25, 99, 234, 101, 52, 108, 25, 156, 19 };
unsigned char AES_IV3[16] = { 248, 92, 18, 93, 216, 113, 83, 68, 39, 236, 101, 65, 102, 16, 168, 28 };
unsigned char AES_IV4[16] = { 216, 16, 98, 93, 198, 129, 26, 28, 69, 201, 101, 62, 133, 48, 145, 62 };
unsigned char AES_IV5[16] = { 201, 9, 93, 93, 176, 178, 97, 98, 99, 121, 101, 22, 189, 98, 115, 91 };
unsigned char AES_IV6[16] = { 218, 92, 06, 93, 212, 118, 98, 18, 49, 220, 101, 32, 123, 31, 148, 31 };
unsigned char AES_IV7[16] = { 213, 12, 05, 93, 180, 230, 38, 68, 79, 109, 101, 28, 150, 55, 135, 69 };

unsigned char Fun_Ble_JieMa(unsigned char *ndata,unsigned int len,unsigned char *out_Hex_Bin,Ble_JieMa_t ble_param)    
{
	unsigned char i;
	unsigned char AES_MIYAO[16];//��Կ	
	unsigned char AES_IV[16];   //��ʼ����
	unsigned char jdata[16];
	unsigned char AES_TEMP;
	unsigned int  cfb_olen;
	unsigned int decimal_values[6];
    unsigned int  cfb_len;
	
	AESModeOfTask aesTask;
	
    for (int i = 0; i < 6; i++)
    {
        decimal_values[i] = ble_param.gattway_id[i];  // 直接赋值，自动转换为10进制
    }

	AES_TEMP = ble_param.gattway_id[5]%7;

	if((ble_param.mode&0x0F)==0)   
	{
		for(unsigned int i=0;i<len;i++)
		{
			out_Hex_Bin[i] = ndata[i];
		}
		return 0;   //���ӽ���
	}
	if((ble_param.mode&0x0F)==0x01)   //��,����Կ
	{
		switch(AES_TEMP)   //ȡ��Կ
		{
			case 0:
			for(i=0;i<16;i++)	AES_MIYAO[i]=AES_Key_Table88[i];
			for(i=0;i<6;i++)
			AES_MIYAO[i+4] = decimal_values[i];
			break;
			case 1:
			for(i=0;i<16;i++)    AES_MIYAO[i]=AES_Key_Table99[i]; 
			for(i=0;i<6;i++)
			AES_MIYAO[i+4] = decimal_values[i];
			break;
			case 2:
			for(i=0;i<16;i++)    AES_MIYAO[i]=AES_Key_Table101[i];   
			for(i=0;i<6;i++)
			AES_MIYAO[i+4] = decimal_values[i];
			break;
			case 3:
			for(i=0;i<16;i++)	AES_MIYAO[i]=AES_Key_Table111[i];   
			for(i=0;i<6;i++)
			AES_MIYAO[i+4] = decimal_values[i];
			break;
			case 4:
			for(i=0;i<16;i++)	AES_MIYAO[i]=AES_Key_Table121[i];   
			for(i=0;i<6;i++)
			AES_MIYAO[i+4] = decimal_values[i];
			break;
			case 5:
			for(i=0;i<16;i++)    AES_MIYAO[i]=AES_Key_Table131[i];   
			for(i=0;i<6;i++)
			AES_MIYAO[i+4] = decimal_values[i];
			break;
			case 6:for
			(i=0;i<16;i++)	AES_MIYAO[i]=AES_Key_Table141[i];	
			for(i=0;i<6;i++)
			AES_MIYAO[i+4] = decimal_values[i];
			break;
		}
  }
	else if((ble_param.mode&0x0F)==0x02)//��������Կ,����ͨ��ʱʹ��
	{
		 for(i=0;i<16;i++)	AES_MIYAO[i]=ble_param.key[i];
	}
	switch(AES_TEMP) //����IV
	{
		case 0:for(i=0;i<16;i++)	AES_IV[i]=AES_IV1[i];
		
		break;
		case 1:for(i=0;i<16;i++)	AES_IV[i]=AES_IV2[i];
		
		break;
		case 2:for(i=0;i<16;i++)	AES_IV[i]=AES_IV3[i];break;
		case 3:for(i=0;i<16;i++)  AES_IV[i]=AES_IV4[i];break;
		case 4:for(i=0;i<16;i++)  AES_IV[i]=AES_IV5[i];break;
		case 5:for(i=0;i<16;i++)	AES_IV[i]=AES_IV6[i];break;
		case 6:for(i=0;i<16;i++)	AES_IV[i]=AES_IV7[i];break;
  }
	aesTask.aes.InitAES = InitAES;
	aesTask.InitAESModeOfTask = InitAESModeOfTask;
	aesTask.set_mode = set_mode;
	aesTask.set_key = set_key;
	aesTask.set_iv = set_iv;
	aesTask.Encrypt = Encrypt;
	aesTask.Decrypt = Decrypt;

	aesTask.InitAESModeOfTask(&aesTask);
	aesTask.set_key(&aesTask,AES_MIYAO);
	aesTask.set_mode(&aesTask, MODE_CFB);
	aesTask.set_iv(&aesTask, AES_IV);

	switch(ble_param.mode&0xF0)
	{
		case 0x10://
			cfb_len = aesTask.Encrypt(&aesTask, ndata, len, out_Hex_Bin);
		break;
		case 0x20://
			cfb_len = aesTask.Decrypt(&aesTask, ndata, len, out_Hex_Bin);
		break;
	}
	return 0x00;
}
#endif
//26#��������
// unsigned char fun_26menu()
// {
// 	  uint8_t  recode=0; 
// 	  uint8_t i;
	
// 	  flag_isnewlock = 0xff;
// 		ReadFlashData(LOCK_BASEADDR,LOCK_DEVADDR,ReadFlashBuf);
// 		for(i=0;i<LOCK_DEVADDR;i++) 
// 		{
// 			if(ReadFlashBuf[i]!=0x5a) flag_isnewlock = 0x00;
// 		}
// 		if(flag_isnewlock) return ERR_FF;
// 		ClearDisplay();		
// 	  speaker_sound(S_SetLockId);
// 		mydelay(200);
// 		recode  = fu_getuser(6);
// 		if(recode==0xff||passpc<6) return 0xff;
// 		memcpy(Lock_Manger,PassWord,Manger_Len);
// 		write_sectordata(LOCK_SUOMAIDADDR,Manger_Len,Lock_Manger);
// 		memset(PassWord,0,LOCK_LXLEN);
// 		write_sectordata(LOCK_LXBASEADDR,LOCK_LXLEN,PassWord);
// 		LX_InitSector();
// 		return RT_OK;
// }
//////////////////////////////////////////////////////////////
//�������������㷨
//uint8_t TemporPassWord(uint8_t *GroupIn)
// {	
// 	uint32_t GroupInChan=0;
// 	uint32_t PWChan=0;
// 	uint8_t keyvaltemp[6]={0};
// 	uint8_t jtimes[6];
//   uint16_t OppTime;
//   uint16_t LockNowTime;
	
// 	ReadFlashData(LOCK_SUOMAIDADDR,Manger_Len,Lock_Manger);
// 	if(Lock_Manger[0]==0xff&&Lock_Manger[1]==0xff)      return 0;
	
// 	ret_times(jtimes);    
// 	//jtimes[0]=22;jtimes[1]=8;jtimes[2]=19;jtimes[3]=18;jtimes[4]=15;
// 	GroupInChan= GroupIn[0]/16*100000+GroupIn[0]%16*10000+GroupIn[1]/16*1000+GroupIn[1]%16*100+GroupIn[2]/16*10+GroupIn[2]%16;
// 	PWChan = Lock_Manger[0]/16*100000+Lock_Manger[0]%16*10000+Lock_Manger[1]/16*1000+Lock_Manger[1]%16*100+Lock_Manger[2]/16*10+Lock_Manger[2]%16;
// 	//0-y 1-m 2-d 3-h 4-m 5-s
// 	if(jtimes[3]==0&&jtimes[4]<15&&jtimes[2]==1) //����==0&&��?<15&&��?==1
// 	{
// 		if(jtimes[1]==2)	//2��
// 		{
// 			jtimes[2]=28;	
// 			{
// 				OppTime = (GroupInChan%100000)^(PWChan%10000)^(GroupInChan/100000*1111)^(PWChan/10000*100+ jtimes[2]);
// 				LockNowTime = jtimes[3]*100+jtimes[4]+1440;
// 				if(((LockNowTime-OppTime)<15)&&((LockNowTime-OppTime)>=0))	return 1;
// 			}
// 			jtimes[2]=29;	
// 			{
// 				OppTime = (GroupInChan%100000)^(PWChan%10000)^(GroupInChan/100000*1111)^(PWChan/10000*100+ jtimes[2]);
// 				LockNowTime = jtimes[3]*100+jtimes[4]+1440;
// 				if(((LockNowTime-OppTime)<15)&&((LockNowTime-OppTime)>=0))	return 1;
// 			}
// 			return 0;
// 		}
// 	  else if(jtimes[1]==1||jtimes[1]==2||jtimes[1]==4||jtimes[1]==6||jtimes[1]==8||jtimes[1]==9||jtimes[1]==11)	jtimes[2]=31;
// 	  else                                                                                                        jtimes[2]=30;
	
// 	  OppTime = (GroupInChan%100000)^(PWChan%10000)^(GroupInChan/100000*1111)^(PWChan/10000*100+  jtimes[2]);
//     LockNowTime = jtimes[3]*100+jtimes[4]+1440;
// 		if(OppTime/100>23||OppTime%100>59)	                      return 0;
		
// 		OppTime=OppTime/100*60+OppTime%100;
// 		if(((LockNowTime-OppTime)<15)&&((LockNowTime-OppTime)>=0)) return 1;
// 	}    
// 	else if(jtimes[3]==0&&jtimes[4]<15) jtimes[2]--;
	
// 	OppTime = (GroupInChan%100000)^(PWChan%10000)^(GroupInChan/100000*1111)^(PWChan/10000*100+  jtimes[2]);
// 	LockNowTime = jtimes[3]*60+jtimes[4];
// 	if(OppTime/100>23||OppTime%100>59) return 0;		
// 	OppTime=OppTime/100*60+OppTime%100;
// 	if(((LockNowTime-OppTime)<15)&&((LockNowTime-OppTime)>=0))   return 1;
	
//   return 0;
// }
// uint8_t app_lq_lx_password(uint8_t *GroupIn)
// {
//   if(TemporPassWord(GroupIn))
// 	{
// 		sys_locktemps = 0;
// 		UserType=PASSWORD_LX;
// 		VerID = LOCK_AllUSERTMP+LOCK_TEMPORTMP;
// 		return LX_OPEN;
// 	}
// 	return ERR_FF;
// }















