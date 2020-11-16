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

	int width = 512;	// 이미지 파일의 가로 길이
	int height = 512;	// 이미지 파일의 세로 길이

	float Image1_Histogram[256] = { 0, };  // 이미지1의 히스토그램
	float Image2_Histogram[256] = { 0, };  // 이미지2의 히스토그램
	float Image_mapping_Histogram[256] = { 0, }; // histogram mapping을 한 히스토그램

	float Image1_CDF[256] = { 0, };		   // 이미지1의 CDF
	float Image2_CDF[256] = { 0, };		   // 이미지2의 CDF
	float Image_mapping_CDF[256] = { 0, }; // histogram mapping을 한 CDF

	FILE* Input1_file = fopen("input1.raw", "rb");	// Input 이미지1
	FILE* Input2_file = fopen("input2.raw", "rb");	// Input 이미지2
	FILE* Output_file = fopen("output.raw", "wb");		  // histogram mapping한 출력 이미지

	UCHAR** Input1_data = memory_alloc2D(width, height);  // Input 이미지1의 픽셀 값 저장
	UCHAR** Input2_data = memory_alloc2D(width, height);  // Input 이미지2의 픽셀 값 저장
	UCHAR** Output_data = memory_alloc2D(width, height);  // histogram mapping한 픽셀 값을 저장
	


	int sum = 0;

	if (!Input1_file || !Input2_file) {	// 파일이 해당 경로에 없는 경우
		printf("Can not open file.");
		return -1;
	}

	// 이미지를 읽어와 밝기 값 하나하나를 저장
	fread(&Input1_data[0][0], sizeof(UCHAR), width * height, Input1_file);
	fread(&Input2_data[0][0], sizeof(UCHAR), width * height, Input2_file);


	// 이미지1 히스토그램 구하기
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int data = Input1_data[i][j];
			Image1_Histogram[data] += 1;	// 해당 밝기 값의 빈도 수 카운트
		}
	}
	DrawHistogram(Image1_Histogram, 30, 400); // 이미지1의 히스토그램 출력
	// 이미지1 CDF 구하기 = Histogram Equalization
	sum = 0;
	for (int i = 0; i < 256; i++) {
		sum += (int)Image1_Histogram[i];
		Image1_CDF[i] = (float)sum / (width * height);	// 전체 크기로 나눠서 확률 구하기
	}
	DrawCDF(Image1_CDF, 30, 400);	// 이미지1의 CDF 출력


	// 이미지2 히스토그램 구하기
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int data = Input2_data[i][j];
			Image2_Histogram[data] += 1;	// 해당 밝기 값의 빈도 수 카운트
		}
	}
	DrawHistogram(Image2_Histogram, 400, 400); // 이미지2의 히스토그램 출력
	// 이미지2 CDF 구하기 = Histogram Equalization
	sum = 0;
	for (int i = 0; i < 256; i++) {
		sum += (int)Image2_Histogram[i];
		Image2_CDF[i] = (float)sum / (width * height);	// 전체 크기로 나눠서 확률 구하기
	}
	DrawCDF(Image2_CDF, 400, 400);	// 이미지2의 CDF 출력



	// Histogram mapping
	for (int j = 0; j < 256; j++) {
		float p = Image1_CDF[j];  // 이미지1의 CDF
		// 이미지1의 CDF를 이미지2의 CDF의 역함수에 넣음으로써 Histogram mapping
		int inverse_value = inverse_normal_cdf(p, Image2_CDF, 0.00001); // 0~255

		for (int k = 0; k < Image1_Histogram[j]; k++) {
			Image_mapping_Histogram[inverse_value] += 1;  // 해당 값의 빈도 수 카운팅
		}
	}
	DrawHistogram(Image_mapping_Histogram, 800, 400);	// histogram mapping한 히스토그램 출력
	//for (int i = 0; i < 256; i++) {
	//	printf("[%d]: %f\n", i, Image_mapping_Histogram[i]);
	//}

	// histogram mapping한 CDF 구하기
	sum = 0;
	for (int i = 0; i < 256; i++) {
		sum += (int)Image_mapping_Histogram[i];	// 값을 누적
		Image_mapping_CDF[i] = (float)sum / (width * height); // 전체 크기로 나눠서 확률 구하기
		//printf("[%d]: %f\n", i, Image_mapping_CDF[i]);
	}
	DrawCDF(Image_mapping_CDF, 800, 400); // histogram mapping한 CDF 출력



	// histogram mapping한 이미지 출력파일 픽셀 값 설정
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int brightness = Input1_data[i][j];	// 이미지1의 한 픽셀의 밝기 값(r)
			float cdf = Image1_CDF[brightness]; // r의 cdf값 = T(r)
			Output_data[i][j] = inverse_normal_cdf(cdf, Image2_CDF, 0.00001); // histogram mapping한 z (0~255)
		}
	}

	// Output_data의 데이터로 Output_file을 생성
	fwrite(&Output_data[0][0], sizeof(UCHAR), width * height, Output_file);



	MemoryClear(Input1_data); // 메모리 해제
	MemoryClear(Input2_data);
	MemoryClear(Output_data);
	fclose(Input1_file);	// 파일 close
	fclose(Input2_file);
	fclose(Output_file);

	return 0;
}



// 메모리를 해제하는 함수
void MemoryClear(UCHAR** buf) {
	if (buf) {
		free(buf[0]);
		free(buf);
		buf = NULL;
	}
}

// 2차원 배열에 동적 메모리를 할당하는 함수
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

// 역함수
float inverse_normal_cdf(float p, float cdf[256], float tolerance = 0.1)
{
	//// 표준정규분포가 아닌 경우
	//if (mu != 0 || sigma != 1) {
	//	return mu + sigma * inverse_normal_cdf(p, 0, 1, tolerance = tolerance);
	//}

	float low_x = 0.0, low_p = 0.0;   // cdf(0)는 0에 근접
	float hi_x = 255.0, hi_p = 1.0;	  // cdf(255)는 1에 근접
	float mid_x = 0.0, mid_p = 0.0;
	float before_mid_p = 0.0;

	while (hi_x - low_x > tolerance)	// 해당 p를 찾을 때까지 반복
	{
		mid_x = (low_x + hi_x) / 2; // 중간 값 계산
		//mid_p = normal_cdf(mid_x, mu, sigma); // 중간 값의 cdf 값을 계산
		mid_p = cdf[(int)mid_x];
		if (mid_p < p) {  // mid_p가 찾는 값보다 작으면 더 큰 값에서 탐색
			low_x = mid_x;
			low_p = mid_p;
		}
		else if (mid_p > p) { // mid_p가 찾는 값보다 크면 더 작은 값에서 탐색
			hi_x = mid_x;
			hi_p = mid_p;
		}
		else {	// p 값을 찾으면 break
			break;
		}

		if (mid_p == before_mid_p)
			break;
		before_mid_p = mid_p;
	}
	return (int)mid_x;	// normal_cdf 값이 p인 x 반환
}

// CDF를 그리는 함수
void DrawCDF(float cdf[256], int x_origin, int y_origin) {
	for (int CurX = 0; CurX < 256; CurX++) {
		for (int CurY = 0; CurY < cdf[CurX]; CurY++) {
			MoveToEx(hdc, x_origin + CurX, y_origin, 0);
			SetPixel(hdc, x_origin + CurX, y_origin - cdf[CurX] * 100, RGB(0, 0, 255));
		}
	}
}

// Histogram을 출력하는 함수
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
