#include <stdint.h>
#include <string.h>


#define ROUND_NR 16

#define RC_POS 0

static
void gamma_1(uint32_t *a){
	uint32_t tmp;
	
	a[1] ^= ~((a[3]) | (a[2]));
	a[0] ^=   a[2] & a[1];	
	
	tmp=a[3]; a[3]=a[0]; a[0]=tmp;
	a[2] ^= a[0] ^ a[1] ^ a[3];
	
	a[1] ^= ~((a[3]) | (a[2]));
	a[0] ^=   a[2] & a[1];	
}	

#define ROTL32(a,n) (((a)<<n)|((a)>>(32-n)))
#define ROTR32(a,n) (((a)>>n)|((a)<<(32-n)))

static
void pi1(uint32_t *a){
	a[1] = ROTL32(a[1], 1);
	a[2] = ROTL32(a[2], 5);
	a[3] = ROTL32(a[3], 2);
}

static
void pi2(uint32_t *a){
	a[1] = ROTR32(a[1], 1);
	a[2] = ROTR32(a[2], 5);
	a[3] = ROTR32(a[3], 2);
}

static
void theta(const uint32_t *k, uint32_t *a){
	uint32_t temp;

	temp = a[0] ^ a[2]; temp ^= ROTR32(temp, 8) ^ ROTL32(temp, 8);
	a[1] ^= temp;
	a[3] ^= temp;
	
	a[0] ^= k[0];
	a[1] ^= k[1];
	a[2] ^= k[2];
	a[3] ^= k[3];

	temp = a[1] ^ a[3]; temp ^= ROTR32(temp, 8) ^ ROTL32(temp, 8);
	a[0] ^= temp;
	a[2] ^= temp;	

}

static 
void noekeon_round(uint32_t *key, uint32_t *state, uint8_t const1, uint8_t const2){
	((uint8_t*)state)[RC_POS] ^= const1;
	theta(key, state);
	((uint8_t*)state)[RC_POS] ^= const2;
	pi1(state);
	gamma_1(state);
	pi2(state);
}

const uint8_t rc_tab[] = {
/*	0x80, */
	      0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A,
	0x2F, 0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35, 0x6A,
	0xD4
};
/* for more rounds
 0xD4, 0xB3, 0x7D, 0xFA, 0xEF, 0xC5, 0x91, 0x39,
 0x72, 0xE4, 0xD3, 0xBD, 0x61, 0xC2, 0x9F, 0x25,
*/

static
void changendian32(void *a){
	((uint8_t*)a)[0] ^= ((uint8_t*)a)[3];
	((uint8_t*)a)[3] ^= ((uint8_t*)a)[0];
	((uint8_t*)a)[0] ^= ((uint8_t*)a)[3];
	
	((uint8_t*)a)[1] ^= ((uint8_t*)a)[2];
	((uint8_t*)a)[2] ^= ((uint8_t*)a)[1];
	((uint8_t*)a)[1] ^= ((uint8_t*)a)[2];
}

static
void changendian(void *a){
	changendian32((uint32_t*)(&(((uint32_t*)a)[0])));
	changendian32((uint32_t*)(&(((uint32_t*)a)[1])));
	changendian32((uint32_t*)(&(((uint32_t*)a)[2])));
	changendian32((uint32_t*)(&(((uint32_t*)a)[3])));
}


void noekeon_enc(void *buffer, const void *key){
	uint8_t rc=0x80;
	uint8_t keyb[16];
	int8_t i;
	
	memcpy(keyb, key, 16);
	changendian(buffer);
	changendian(keyb);

	for(i=0; i<ROUND_NR; ++i){
		noekeon_round((uint32_t*)keyb, (uint32_t*)buffer, rc, 0);
    rc=rc_tab[i];
	}
	((uint8_t*)buffer)[RC_POS] ^= rc;
	theta((uint32_t*)keyb, (uint32_t*)buffer);

	changendian(buffer);
}




void noekeon_init(const void *key, noekeon_ctx_t *ctx){
	uint8_t nullv[16];
	
	memset(nullv, 0, 16);
	memcpy(ctx, key, 16);
	noekeon_enc(ctx, nullv);
}