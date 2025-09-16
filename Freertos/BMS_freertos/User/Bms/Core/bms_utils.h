#ifndef __BMS_UTILS_H__
#define __BMS_UTILS_H__



#include <stdint.h>



void BubbleFloat(float a[], uint32_t n);


int binarySearch(uint16_t *nums, uint8_t left, uint8_t right, uint16_t target);
int right_bound(uint16_t *nums, uint8_t start_pos, uint8_t end_pos, uint16_t target);
int left_bound(uint16_t *nums, uint16_t start_pos, uint16_t end_pos, uint16_t target);

int cmp_int8_t(const void *e1, const void *e2);
int cmp_uint8_t(const void *e1, const void *e2);
int cmp_int16_t(const void *e1, const void *e2);
int cmp_uint16_t(const void *e1, const void *e2);
int cmp_float(const void *e1, const void *e2);
int cmp_double(const void *e1, const void *e2);
void swap(uint8_t *buf1, uint8_t *buf2, uint32_t width);
void BubbleSort(void *base, uint32_t sz, uint32_t width, int (*cmp)(void *e1, void *e2));




#endif



