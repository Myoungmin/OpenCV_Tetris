#include <opencv2/opencv.hpp> 
#include <iostream>
#include <time.h>

using namespace std;
using namespace cv;
using namespace ml;	//머신러닝 네임스페이스 사용

Mat img;

const unsigned int tetriminos[7][4] = {
	{ 0x6600, 0x6600, 0x6600, 0x6600 },       // O
	{ 0xF000, 0x4444, 0xF000, 0x4444 },        // I
	{ 0x6C00, 0x8C40, 0x6C00, 0x8C40 },    // S
	{ 0xC600, 0x4C80, 0xC600, 0x4C80 },    // Z
	{ 0x4E00, 0x8C80, 0xE400, 0x4C40 },    // T
	{ 0x2E00, 0xC440, 0xE800, 0x88C0 },    // L
	{ 0x8E00, 0x44C0, 0xE200, 0xC880 },    // J
};


unsigned char shape;                  // 테트리미노스의 7가지 모양
unsigned char pattern;                 // 테트리미노스의 4가지 패턴
unsigned char cur_line;               // 테트리니노스의 현재 라인
unsigned char cur_col;                // 테트리니노스의 현재 칸
unsigned long int temp_line[4];          // 테트리미노스 라인 임시 저장소
unsigned long int main_board[32] = { 0 };	//테르리미노스가 굳어진 후 저장된 게임보드
unsigned long int game_board[32];	//테트리미노스가 움직이면서 변화하는 게임보드
unsigned char crush = 0;	//부딪힘을 나타내는 플레그
unsigned char new_block = 0;	//새로운 블록이 생성되야함을 나타내는 플레그
unsigned char game_over = 0;	//게임이 종료되었음을 나타내는 플레그
unsigned char next_block = 0;
unsigned char next_board[8] = { 0 };
unsigned char red;
unsigned char blue;
unsigned char green;
int b;
int g;
int r;


unsigned char Collision()	//충돌 여부 확인
{
	if (((main_board[cur_line] & temp_line[0]) != 0) | ((main_board[cur_line + 1] & temp_line[1]) != 0) |
		((main_board[cur_line + 2] & temp_line[2]) != 0) | ((main_board[cur_line + 3] & temp_line[3]) != 0))
		return 1;         // 충돌 1 리턴
	else
		return 0;  // 충돌 없음 0 리턴
}


void tetriminos_to_temp_line()
{
	for (int i = 0; i < 4; i++)	//테트리미노스 라인 임시 저장소 초기화
	{
		temp_line[i] = 0;
	}

	if (cur_col == 2)
	{
		temp_line[0] = ((unsigned long int)(tetriminos[shape][pattern] & 0xF000) >> (15 - cur_col));
		temp_line[1] = ((unsigned long int)(tetriminos[shape][pattern] & 0x0F00) >> (11 - cur_col));
		temp_line[2] = ((unsigned long int)(tetriminos[shape][pattern] & 0x00F0) >> (7 - cur_col));
		temp_line[3] = ((unsigned long int)(tetriminos[shape][pattern] & 0x000F) >> (3 - cur_col));
	}
	else if (cur_col >= 3)
	{
		temp_line[0] = ((unsigned long int)(tetriminos[shape][pattern] & 0xF000) >> 12) << (cur_col - 3);	//이 과정이 필요 없고 뒤에서 한번에 충돌감지 이후 넣어줘도 상관없어 보이지만 충돌 여부 판별하는 함수가 temp_line와 메인보드가 겹치는지 파악하기때문에 temp_line에 변화된 사항을 반영해야 한다.
		temp_line[1] = ((unsigned long int)(tetriminos[shape][pattern] & 0x0F00) >> 8) << (cur_col - 3);
		temp_line[2] = ((unsigned long int)(tetriminos[shape][pattern] & 0x00F0) >> 4) << (cur_col - 3);
		temp_line[3] = ((unsigned long int)(tetriminos[shape][pattern] & 0x000F)) << (cur_col - 3);
	}
}


void draw_map()
{
	for (int i = 0; i < 12; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			if ((game_board[j] & ((unsigned long int)1 << i)) != 0)
			{
				rectangle(img, Point(10 * i, 10 * j), Point(10 * i + 9, 10 * j + 9), Scalar(blue, green, red), -1);
			}
		}
	}

	


	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if ((next_board[j] & (1 << i)) != 0)
			{
				rectangle(img, Point(130 + 10 * i, 10 * j), Point(130 + 10 * i + 9, 10 * j + 9), Scalar(b, g, r), -1);
			}
		}
	}
}

void NewTetriminos()
{
	new_block = 0;	//새로운 블록 플레그 끄기
	shape = next_block;
	next_block = rand() % 7;					//다음에 올 테트리미노스 랜덤 출력
	pattern = 0;	//기본 회전모향 설정
	cur_line = 0;                 // 테트리미노스 현재 라인 (최상위 라인)
	cur_col = 6;                // 테트리미노스의 현재 칸

	tetriminos_to_temp_line();


	game_over |= game_board[cur_line] & temp_line[0];	//게임보드에 있는 테트리미노스와 임시저장소에 생긴 테트리미노스가 겹치는지 확인하고 겹치면 게임오버 플레그 켜짐
	game_over |= game_board[cur_line + 1] & temp_line[1];	//or 연산으로 어디든 겹치면 플레그 켜진다
	game_over |= game_board[cur_line + 2] & temp_line[2];
	game_over |= game_board[cur_line + 3] & temp_line[3];


	game_board[cur_line] |= temp_line[0];	//현재라인 아래로 이동 후 변화하는 보드에 반영
	game_board[cur_line + 1] |= temp_line[1];
	game_board[cur_line + 2] |= temp_line[2];
	game_board[cur_line + 3] |= temp_line[3];

}


void NextTetriminos()
{
	for (int i = 0; i < 8; i++)	//넥스트 보드 초기화
	{
		next_board[i] = 0;
	}


	next_board[2] |= (unsigned char)((tetriminos[next_block][0] & 0xF000) >> 10);
	next_board[3] |= (unsigned char)((tetriminos[next_block][0] & 0x0F00) >> 6);
	next_board[4] |= (unsigned char)((tetriminos[next_block][0] & 0x00F0) >> 2);
	next_board[5] |= (unsigned char)((tetriminos[next_block][0] & 0x000F) << 2);


	

}


int main() {
	srand((unsigned int)time(NULL));

	VideoCapture cap(0);	//비디오 캡쳐

	while (1)
	{
		game_over = 0;	//게임종료 플레그 끄기

		blue = rand() % 256;
		green = rand() % 256;
		red = rand() % 256;

		for (int i = 0; i < 31; i++) main_board[i] = 0x801;	//메인보드 초기화
		main_board[31] = 0xFFF;
		new_block = 1;


		while (game_over == 0)	//게임종료 플레그가 꺼저있을동안 반복
		{
			cap >> img;	//비디오 캡쳐 영상으로

			flip(img, img, 1);	//비디오 좌우 반전

			if (new_block == 1)
			{
				b = rand() % 256;
				g = rand() % 256;
				r = rand() % 256;
				for (int i = 0; i < 31; i++)
				{
					if (main_board[i] == 0xFFF)
						//1줄이 모두 완성되어서 깨질 줄이 있는지 확인
					{
						main_board[i] = 0x801;
						for (int k = i; k > 0; k--)
						{
							main_board[k] = main_board[k - 1];	//깨진 줄의 위에 줄들 아래로 이동(행렬상 열 증가)
						}
					}
				}

				for (int i = 0; i < 32; i++)
				{
					game_board[i] = main_board[i];	//굳어진후 저장된 보드를 변화하는 보드로 복사
				}

				NewTetriminos();	//새로운 테트리미노스 생성
				NextTetriminos();
			}

			if (new_block == 0)
			{
				draw_map();	//반영된 변화하는 보드 화면으로 출력
				for (int i = 0; i < 32; i++)
				{
					game_board[i] = main_board[i];	//굳어진후 저장된 보드를 변화하는 보드로 복사
				}

				cur_line++;	//현재라인 아래로 이동

				if (Collision() == 1)	//이동후 충돌 발생시
				{
					cur_line--;	//원래 라인으로 복귀
					main_board[cur_line] |= temp_line[0];	//현재라인 아래로 이동 후 변화하는 보드에 반영
					main_board[cur_line + 1] |= temp_line[1];
					main_board[cur_line + 2] |= temp_line[2];
					main_board[cur_line + 3] |= temp_line[3];

					new_block = 1;	//새로운 블록 플레그 켜짐
				}

				for (int i = 0; i < 32; i++)
				{
					game_board[i] = main_board[i];	//굳어진후 저장된 보드를 변화하는 보드로 복사
				}

				game_board[cur_line] |= temp_line[0];	//현재라인 아래로 이동 후 변화하는 보드에 반영
				game_board[cur_line + 1] |= temp_line[1];
				game_board[cur_line + 2] |= temp_line[2];
				game_board[cur_line + 3] |= temp_line[3];

			}

			imshow("src", img);
			switch (waitKeyEx(100))
			{
			case 27:
				return 0;
				break;
			case 0x250000:
				for (int i = 0; i < 32; i++)
				{
					game_board[i] = main_board[i];	//굳어진후 저장된 보드를 변화하는 보드로 복사
				}

				if (cur_col >= 3) cur_col--;
				else cur_col = 2;

				tetriminos_to_temp_line();

				if (Collision() == 1) cur_col++;

				tetriminos_to_temp_line();

				game_board[cur_line] |= temp_line[0];	//현재라인 아래로 이동 후 변화하는 보드에 반영
				game_board[cur_line + 1] |= temp_line[1];
				game_board[cur_line + 2] |= temp_line[2];
				game_board[cur_line + 3] |= temp_line[3];
				break;

			case 0x260000:
				for (int i = 0; i < 32; i++)
				{
					game_board[i] = main_board[i];	//굳어진후 저장된 보드를 변화하는 보드로 복사
				}

				pattern++;	//회전으로 상태 변화
				if (pattern == 4) pattern = 0; //마지막에서 처음으로

				tetriminos_to_temp_line();

				if (Collision() == 1) pattern--;


				tetriminos_to_temp_line();

				game_board[cur_line] |= temp_line[0];	//현재라인 아래로 이동 후 변화하는 보드에 반영
				game_board[cur_line + 1] |= temp_line[1];
				game_board[cur_line + 2] |= temp_line[2];
				game_board[cur_line + 3] |= temp_line[3];
				break;

			case 0x280000:
				for (int i = 0; i < 32; i++)
				{
					game_board[i] = main_board[i];	//굳어진후 저장된 보드를 변화하는 보드로 복사
				}

				tetriminos_to_temp_line();

				while (Collision() == 0) cur_line++;

				cur_line--;

				tetriminos_to_temp_line();

				game_board[cur_line] |= temp_line[0];	//현재라인 아래로 이동 후 변화하는 보드에 반영
				game_board[cur_line + 1] |= temp_line[1];
				game_board[cur_line + 2] |= temp_line[2];
				game_board[cur_line + 3] |= temp_line[3];
				break;

			case 0x270000:
				for (int i = 0; i < 32; i++)
				{
					game_board[i] = main_board[i];	//굳어진후 저장된 보드를 변화하는 보드로 복사
				}

				cur_col++;

				tetriminos_to_temp_line();

				if (Collision() == 1) cur_col--;

				tetriminos_to_temp_line();


				game_board[cur_line] |= temp_line[0];	//현재라인 아래로 이동 후 변화하는 보드에 반영
				game_board[cur_line + 1] |= temp_line[1];
				game_board[cur_line + 2] |= temp_line[2];
				game_board[cur_line + 3] |= temp_line[3];
				break;
			default :
				break;
			}
		}
	}	
}

