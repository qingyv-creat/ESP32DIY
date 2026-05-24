
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

#ifndef __AES_V_H__
#define __AES_V_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include "aes_use.h"
	
#define AES_JIAMI
#define MODE_OFB  0x00000001
#define MODE_CFB  0x00000002

#define JIEMIV20 0x20
#define JIAMIV20 0x10

typedef struct AES
{
	//public
	void(*InitAES)(struct AES* aes, unsigned char* key);
	void (*SetKey)(struct AES* aes, unsigned char *key);
	unsigned char* (*Cipher)(struct AES* aes, unsigned char* input, unsigned char* output);
	unsigned char* (*InvCipher)(struct AES* aes, unsigned char* input, unsigned char* output);

	//private
	unsigned char Sbox[256];
	unsigned char InvSbox[256];
	unsigned char w[11][4][4];

	void (*KeyExpansion)(struct AES* aes, unsigned char* key, unsigned char w[][4][4]);
	unsigned char (*FFmul)(unsigned char a, unsigned char b);

	void (*SubBytes)(struct AES* aes, unsigned char state[][4]);
	void (*ShiftRows)(unsigned char state[][4]);
	void (*MixColumns)(unsigned char state[][4]);
	void (*AddRoundKey)(unsigned char state[][4], unsigned char k[][4]);

	void (*InvSubBytes)(struct AES* aes, unsigned char state[][4]);
	void (*InvShiftRows)(unsigned char state[][4]);
	void (*InvMixColumns)(unsigned char state[][4]);
} AES;

typedef struct AESModeOfTask
{
    //public
	void (*InitAESModeOfTask)(struct AESModeOfTask * aesTask);
	void(*set_mode)(struct AESModeOfTask * aesTask, int _mode);
	void(*set_key)(struct AESModeOfTask * aesTask, unsigned char *key);
	void(*set_iv)(struct AESModeOfTask * aesTask, unsigned char *iv);
	int(*Encrypt)(struct AESModeOfTask * aesTask, unsigned char *input, int length, unsigned char *output);
	int(*Decrypt)(struct AESModeOfTask * aesTask, unsigned char *input, int length, unsigned char *output);

	//private
	AES aes;
	int	  mode;
	unsigned char key[16];
	unsigned char iv[16];
} AESModeOfTask;

typedef struct {
	unsigned char mode;          //模式
	unsigned char gattway_id[6];    //锁id  
	unsigned char key[16];       //密钥  
}Ble_JieMa_t;

typedef struct {
    unsigned char timestamp[5];               //时区信息与基准时间戳(timestamp[0]代表时区信息：)
                                              //时区范围为GMT-12:00至GMT+12:00;
                                              //0x00代表世界时间GMT+0:00
                                              //0x0C 代表世界世界GMT+12:00
    unsigned char frist_master_password[6];   //第一个管理员密码格式（密码：123456-->0x12、0x34、0x56）
    unsigned char Lock_Manger[4];             //锁码先预留 
    
}offline_password_param_t; 


extern unsigned char Fun_Ble_JieMa(unsigned char *ndata,unsigned int len,unsigned char *out_Hex_Bin,Ble_JieMa_t ble_param);	



void InitAES(struct AES* aes, unsigned char* key);
void SetKey(struct AES* aes, unsigned char *key);
unsigned char* Cipher(struct AES* aes, unsigned char* input, unsigned char *output);
unsigned char* InvCipher(struct AES* aes, unsigned char* input, unsigned char *output);
void KeyExpansion(struct AES* aes, unsigned char* key, unsigned char w[][4][4]);
unsigned char FFmul(unsigned char a, unsigned char b);
void SubBytes(struct AES* aes, unsigned char state[][4]);
void ShiftRows(unsigned char state[][4]);
void MixColumns(unsigned char state[][4]);
void AddRoundKey(unsigned char state[][4], unsigned char k[][4]);
void InvSubBytes(struct AES* aes, unsigned char state[][4]);
void InvShiftRows(unsigned char state[][4]);
void InvMixColumns(unsigned char state[][4]);

void InitAESModeOfTask(struct AESModeOfTask * aesTask);
void set_mode(struct AESModeOfTask * aesTask, int _mode);
void set_key(struct AESModeOfTask * aesTask, unsigned char *_key);
void set_iv(struct AESModeOfTask * aesTask, unsigned char *_iv);
int Encrypt(struct AESModeOfTask * aesTask, unsigned char *_in, int _length, unsigned char *_out);
int Decrypt(struct AESModeOfTask * aesTask, unsigned char *_in, int _length, unsigned char *_out);
extern unsigned char fun_searchpwv20(unsigned char *p,unsigned char timestamp[5]); //查找正确密码
extern unsigned char fun_inputpassword(unsigned char *p,unsigned char len);
//extern void NaiveEncrypt(void *pData, size_t byteDataLen, const void *pKey, const size_t byteKeyLen);
//extern void NaiveDecrypt(void *pData, size_t byteDataLen, const void *pKey, const size_t byteKeyLen);
#ifdef __cplusplus
}
#endif


#endif 
