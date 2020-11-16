#include <iostream>
#include <math.h>
#include <windows.h>

#pragma warning(disable : 4996)


void MemoryClear(UCHAR** buf);
UCHAR** memory_alloc2D(int width, int height);
float inverse_normal_cdf(float p, float cdf[256], float tolerance = 0.1);
void DrawCDF(float cdf[256], int x_origin, int y_origin);
void DrawHistogram(float histogram[256], int x_origin, int y_origin);


HWND hwnd;
HDC hdc;



int main(void)
{
	system("color F0");
	hwnd = GetForegroundWindow();
	hdc = GetWindowDC(hwnd);

	int width = 512;	// �̹��� ������ ���� ����
	int height = 512;	// �̹��� ������ ���� ����

	float Image1_Histogram[256] = { 0, };  // �̹���1�� ������׷�
	float Image2_Histogram[256] = { 0, };  // �̹���2�� ������׷�
	float Image_mapping_Histogram[256] = { 0, }; // histogram mapping�� �� ������׷�

	float Image1_CDF[256] = { 0, };		   // �̹���1�� CDF
	float Image2_CDF[256] = { 0, };		   // �̹���2�� CDF
	float Image_mapping_CDF[256] = { 0, }; // histogram mapping�� �� CDF

	FILE* Input1_file = fopen("barbara.raw", "rb");	// Input �̹���1
	FILE* Input2_file = fopen("Couple.raw", "rb");	// Input �̹���2
	FILE* Output_file = fopen("output.raw", "wb");		  // histogram mapping�� ��� �̹���

	UCHAR** Input1_data = memory_alloc2D(width, height);  // Input �̹���1�� �ȼ� �� ����
	UCHAR** Input2_data = memory_alloc2D(width, height);  // Input �̹���2�� �ȼ� �� ����
	UCHAR** Output_data = memory_alloc2D(width, height);  // histogram mapping�� �ȼ� ���� ����
	


	int sum = 0;

	if (!Input1_file || !Input2_file) {	// ������ �ش� ��ο� ���� ���
		printf("Can not open file.");
		return -1;
	}

	// �̹����� �о�� ��� �� �ϳ��ϳ��� ����
	fread(&Input1_data[0][0], sizeof(UCHAR), width * height, Input1_file);
	fread(&Input2_data[0][0], sizeof(UCHAR), width * height, Input2_file);


	// �̹���1 ������׷� ���ϱ�
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int data = Input1_data[i][j];
			Image1_Histogram[data] += 1;	// �ش� ��� ���� �� �� ī��Ʈ
		}
	}
	DrawHistogram(Image1_Histogram, 30, 400); // �̹���1�� ������׷� ���
	// �̹���1 CDF ���ϱ� = Histogram Equalization
	sum = 0;
	for (int i = 0; i < 256; i++) {
		sum += (int)Image1_Histogram[i];
		Image1_CDF[i] = (float)sum / (width * height);	// ��ü ũ��� ������ Ȯ�� ���ϱ�
	}
	DrawCDF(Image1_CDF, 30, 400);	// �̹���1�� CDF ���


	// �̹���2 ������׷� ���ϱ�
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int data = Input2_data[i][j];
			Image2_Histogram[data] += 1;	// �ش� ��� ���� �� �� ī��Ʈ
		}
	}
	DrawHistogram(Image2_Histogram, 400, 400); // �̹���2�� ������׷� ���
	// �̹���2 CDF ���ϱ� = Histogram Equalization
	sum = 0;
	for (int i = 0; i < 256; i++) {
		sum += (int)Image2_Histogram[i];
		Image2_CDF[i] = (float)sum / (width * height);	// ��ü ũ��� ������ Ȯ�� ���ϱ�
	}
	DrawCDF(Image2_CDF, 400, 400);	// �̹���2�� CDF ���



	// Histogram mapping
	for (int j = 0; j < 256; j++) {
		float p = Image1_CDF[j];  // �̹���1�� CDF
		// �̹���1�� CDF�� �̹���2�� CDF�� ���Լ��� �������ν� Histogram mapping
		int inverse_value = inverse_normal_cdf(p, Image2_CDF, 0.00001); // 0~255

		for (int k = 0; k < Image1_Histogram[j]; k++) {
			Image_mapping_Histogram[inverse_value] += 1;  // �ش� ���� �� �� ī����
		}
	}
	DrawHistogram(Image_mapping_Histogram, 800, 400);	// histogram mapping�� ������׷� ���
	//for (int i = 0; i < 256; i++) {
	//	printf("[%d]: %f\n", i, Image_mapping_Histogram[i]);
	//}

	// histogram mapping�� CDF ���ϱ�
	sum = 0;
	for (int i = 0; i < 256; i++) {
		sum += (int)Image_mapping_Histogram[i];	// ���� ����
		Image_mapping_CDF[i] = (float)sum / (width * height); // ��ü ũ��� ������ Ȯ�� ���ϱ�
		//printf("[%d]: %f\n", i, Image_mapping_CDF[i]);
	}
	DrawCDF(Image_mapping_CDF, 800, 400); // histogram mapping�� CDF ���



	// histogram mapping�� �̹��� ������� �ȼ� �� ����
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int brightness = Input1_data[i][j];	// �̹���1�� �� �ȼ��� ��� ��(r)
			float cdf = Image1_CDF[brightness]; // r�� cdf�� = T(r)
			Output_data[i][j] = inverse_normal_cdf(cdf, Image2_CDF, 0.00001); // histogram mapping�� z (0~255)
		}
	}

	// Output_data�� �����ͷ� Output_file�� ����
	fwrite(&Output_data[0][0], sizeof(UCHAR), width * height, Output_file);



	MemoryClear(Input1_data); // �޸� ����
	MemoryClear(Input2_data);
	MemoryClear(Output_data);
	fclose(Input1_file);	// ���� close
	fclose(Input2_file);
	fclose(Output_file);

	return 0;
}



// �޸𸮸� �����ϴ� �Լ�
void MemoryClear(UCHAR** buf) {
	if (buf) {
		free(buf[0]);
		free(buf);
		buf = NULL;
	}
}

// 2���� �迭�� ���� �޸𸮸� �Ҵ��ϴ� �Լ�
UCHAR** memory_alloc2D(int width, int height)
{
	UCHAR** ppMem2D = 0;
	int	i;

	//arrary of pointer
	ppMem2D = (UCHAR**)calloc(sizeof(UCHAR*), height);
	if (ppMem2D == 0) {
		return 0;
	}

	*ppMem2D = (UCHAR*)calloc(sizeof(UCHAR), height * width);
	if ((*ppMem2D) == 0) {//free the memory of array of pointer        
		free(ppMem2D);
		return 0;
	}

	for (i = 1; i < height; i++) {
		ppMem2D[i] = ppMem2D[i - 1] + width;
	}

	return ppMem2D;
}

// ���Լ�
float inverse_normal_cdf(float p, float cdf[256], float tolerance = 0.1)
{
	//// ǥ�����Ժ����� �ƴ� ���
	//if (mu != 0 || sigma != 1) {
	//	return mu + sigma * inverse_normal_cdf(p, 0, 1, tolerance = tolerance);
	//}

	float low_x = 0.0, low_p = 0.0;   // cdf(0)�� 0�� ����
	float hi_x = 255.0, hi_p = 1.0;	  // cdf(255)�� 1�� ����
	float mid_x = 0.0, mid_p = 0.0;
	float before_mid_p = 0.0;

	while (hi_x - low_x > tolerance)	// �ش� p�� ã�� ������ �ݺ�
	{
		mid_x = (low_x + hi_x) / 2; // �߰� �� ���
		//mid_p = normal_cdf(mid_x, mu, sigma); // �߰� ���� cdf ���� ���
		mid_p = cdf[(int)mid_x];
		if (mid_p < p) {  // mid_p�� ã�� ������ ������ �� ū ������ Ž��
			low_x = mid_x;
			low_p = mid_p;
		}
		else if (mid_p > p) { // mid_p�� ã�� ������ ũ�� �� ���� ������ Ž��
			hi_x = mid_x;
			hi_p = mid_p;
		}
		else {	// p ���� ã���� break
			break;
		}

		if (mid_p == before_mid_p)
			break;
		before_mid_p = mid_p;
	}
	return (int)mid_x;	// normal_cdf ���� p�� x ��ȯ
}

// CDF�� �׸��� �Լ�
void DrawCDF(float cdf[256], int x_origin, int y_origin) {
	for (int CurX = 0; CurX < 256; CurX++) {
		for (int CurY = 0; CurY < cdf[CurX]; CurY++) {
			MoveToEx(hdc, x_origin + CurX, y_origin, 0);
			SetPixel(hdc, x_origin + CurX, y_origin - cdf[CurX] * 100, RGB(0, 0, 255));
		}
	}
}

// Histogram�� ����ϴ� �Լ�
void DrawHistogram(float histogram[256], int x_origin, int y_origin) {
	MoveToEx(hdc, x_origin, y_origin, 0);
	LineTo(hdc, x_origin + 255, y_origin);

	MoveToEx(hdc, x_origin, 100, 0);
	LineTo(hdc, x_origin, y_origin);

	for (int CurX = 0; CurX < 256; CurX++) {
		for (int CurY = 0; CurY < histogram[CurX]; CurY++) {
			MoveToEx(hdc, x_origin + CurX, y_origin, 0);
			LineTo(hdc, x_origin + CurX, y_origin - histogram[CurX] / 50);
		}
	}
}
