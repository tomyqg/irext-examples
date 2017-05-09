#ifndef     _CCM_H_
#define     _CCM_H_

uint8_t SSP_CCM_Auth_Sw (uint8_t, uint8_t *, uint8_t *, uint16_t, uint8_t *, uint16_t, uint8_t *, uint8_t *, uint8_t);
uint8_t SSP_CCM_Encrypt_Sw (uint8_t, uint8_t *, uint8_t *, uint16_t, uint8_t *, uint8_t *, uint8_t);
uint8_t SSP_CCM_Decrypt_Sw (uint8_t, uint8_t *, uint8_t *, uint16_t, uint8_t *, uint8_t *, uint8_t);
uint8_t SSP_CCM_InvAuth_Sw (uint8_t, uint8_t *, uint8_t *, uint16_t, uint8_t *, uint16_t, uint8_t *, uint8_t *, uint8_t);

uint8_t SSP_CTR_Decrypt_Sw (uint8_t* pCipherTxt, uint16_t cipherTxtLen, uint8_t *AesKey, uint8_t* Nonce, uint8_t* IV);
uint8_t SSP_CTR_Encrypt_Sw (uint8_t *M, uint16_t len_m, uint8_t *AesKey, uint8_t *Nonce, uint8_t* IV);

uint8_t SSP_CCM_Auth_Encrypt_Sw (uint8_t, uint8_t, uint8_t *, uint8_t *, uint16_t, uint8_t *, uint16_t, uint8_t *, uint8_t *, uint8_t);
uint8_t SSP_CCM_InvAuth_Decrypt_Sw (uint8_t, uint8_t, uint8_t *, uint8_t *, uint16_t, uint8_t *, uint16_t, uint8_t *, uint8_t *, uint8_t);

#endif  // _CCM_H_

