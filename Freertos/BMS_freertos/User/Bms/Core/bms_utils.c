#include "bms_utils.h"


// 查找一个数是否存在数组中
int binarySearch(uint16_t *nums, uint8_t left, uint8_t right, uint16_t target)  
{
    while (left <= right) 
	{ 
		// 注意
        int mid = (right + left) / 2;
			
        if(nums[mid] == target)
		{
            return mid; 
		}
        else if (nums[mid] < target)
		{
            left = mid + 1; // 注意
		}
        else if (nums[mid] > target)
		{
            right = mid - 1; // 注意
        }
	}
    return -1;
}

// 查找一个数在数组中的右侧边界(二分法)
// start_pos：起始位置
// end_pos：结束位置
// 返回-1：表示不存在这个数
int right_bound(uint16_t *nums, uint8_t start_pos, uint8_t end_pos, uint16_t target) 
{
	uint16_t left = start_pos;
	uint16_t right = end_pos;
		
    while (left < right)
	{
        int mid = (left + right) / 2;
		
        if (nums[mid] == target) 
		{
            left = mid + 1; // 注意
        } 
		else if (nums[mid] < target) 
		{
            left = mid + 1;
        }
		else if (nums[mid] > target) 
		{
            right = mid;
        }
    }
	if ((left - 1) < start_pos)
	{
		return -1;
	}
    return left - 1; // 注意
}

// 查找一个数在数组中的左侧边界(二分法)
// start_pos：起始位置
// end_pos：结束位置
// 返回-1：表示不存在这个数
int left_bound(uint16_t *nums, uint16_t start_pos, uint16_t end_pos, uint16_t target) 
{
	uint16_t left = start_pos;
	uint16_t right = end_pos;
	
    while (left < right) 
	{ 
		// 注意
        int mid = (left + right) / 2;
		
        if (nums[mid] == target) 
		{
            right = mid;
        }
		else if (nums[mid] < target) 
		{
            left = mid + 1;
        } 
		else if (nums[mid] > target) 
		{
            right = mid; // 注意
        }
    }
	
	if (left > end_pos)
	{
		return -1;
	}
    return left;
}



// 冒泡排序float类型
void BubbleFloat(float a[], uint32_t n)
{
	float t;
    uint32_t i, j;
      
    for (i = 1; i < n; i++)
    {
        for (j = 0; j < n-i; j++)
        {
            if (a[j] > a[j+1])
            {
                t = a[j];
                a[j] = a[j+1];
                a[j+1] = t;
            }
        }
    }
}

/*
void bubble(int a[],int n)
{
    int i,j,t;
    
    for (i = 0; i < n-1; i++)
    {
        for (j = 0; j < n-i-1; j++)
        {
            if (a[j] > a[j+1])
            {
                t = a[j];
                a[j] = a[j+1];
                a[j+1] = t;
            }
        }
    }
}
*/







int cmp_int8_t(const void *e1, const void *e2)
{
    return *(int8_t *)e1 - *(int8_t *)e2;
}

int cmp_uint8_t(const void *e1, const void *e2)
{
    return *(uint8_t *)e1 - *(uint8_t *)e2;
}

int cmp_int16_t(const void *e1, const void *e2)
{
    return *(int16_t *)e1 - *(int16_t *)e2;
}

int cmp_uint16_t(const void *e1, const void *e2)
{
    return *(uint16_t *)e1 - *(uint16_t *)e2;
}

int cmp_float(const void *e1, const void *e2)
{
	if (*(float *)e1 > *(float *)e2)
	{
		return 1;
	}

    return 0;    
}

int cmp_double(const void *e1, const void *e2)
{
	if (*(double *)e1 > *(double *)e2)
	{
		return 1;
	}

    return 0;
}


// 交换元素,任意类型
void swap(uint8_t *buf1, uint8_t *buf2, uint32_t width)
{
	uint8_t temp;
    uint32_t i;
    
    for (i = 0; i < width; i++)
    {
        temp = *buf1;
        *buf1 = *buf2;
        *buf2 = temp;
        buf1++;
        buf2++;
    }
}


// 冒泡排序,任意类型
// base：	基地址
// sz:		要排序元素个数
// width:	单个元素的宽度
// cmp:		不明确类型的情况下,两个数据的对比结果必须由用户完成
//			如果e1比e2大则cmp应返回大于0的数,反之则返回小于等于0的数
void BubbleSort(void *base, uint32_t sz, uint32_t width, int (*cmp)(void *e1, void *e2))
{
    uint32_t i = 0, j = 0;

    for (i = 1; i < sz; i++)
    {
        for (j = 0; j < sz - i; j++)
        {
            if (cmp((uint8_t *)base + j * width, (uint8_t *)base + (j + 1) * width) > 0)
            {
                swap((uint8_t *)base + j * width, (uint8_t *)base + (j + 1) * width, width);
            }
        }
    }
}





/*

下面是任意类型数据冒排序的测试代码



#define BMS_CELL_MAX	5
#define BMS_TEMP_MAX	2


typedef struct
{
	float CellVoltage; 	// 电芯电压
	uint8_t CellNumber;	// 电芯的编号
}BMS_CellDataTypedef;

typedef struct
{
	float CellTemp[BMS_TEMP_MAX];					// 采样温度,温度数据会从小到大排序
	float BatteryVoltage;							// 电池总电压
	float BatteryCurrent;							// 电池组电流
	BMS_CellDataTypedef CellData[BMS_CELL_MAX]; 	// 电芯电压,电压数据会从小到大排序
}BMS_SampleDataTypedef;


int cmp1(void *e1, void *e2)
{
	float temp1, temp2;
	
	temp1 = (*(BMS_CellDataTypedef *)e1).CellVoltage;
	temp2 = (*(BMS_CellDataTypedef *)e2).CellVoltage;

	if (temp1 > temp2)
	{
		return 1;
	}

    return 0;
}

 
int main()
{
	uint8_t index;
	
	BMS_SampleDataTypedef SampleData;
	
	SampleData.CellData[0].CellNumber = 0;
	SampleData.CellData[1].CellNumber = 1;
	SampleData.CellData[2].CellNumber = 2;
	SampleData.CellData[3].CellNumber = 3;
	SampleData.CellData[4].CellNumber = 4;
	
	
	SampleData.CellData[0].CellVoltage = 3.32;
	SampleData.CellData[1].CellVoltage = 3.12;
	SampleData.CellData[2].CellVoltage = 3.59;
	SampleData.CellData[3].CellVoltage = 3.57;
	SampleData.CellData[4].CellVoltage = 4.20;
	
	
	BubbleSort(SampleData.CellData, BMS_CELL_MAX, sizeof(BMS_CellDataTypedef), cmp1);
	
	for (index = 0; index < BMS_CELL_MAX; index++)
	{
		printf("%f\r\n", SampleData.CellData[index].CellVoltage);
	}
	printf("\r\n");
	
    return 0;
}

*/
