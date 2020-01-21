#include <opencv2/opencv.hpp> 
#include <iostream>
#include <time.h>

using namespace std;
using namespace cv;
using namespace ml;	//�ӽŷ��� ���ӽ����̽� ���

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


unsigned char shape;                  // ��Ʈ���̳뽺�� 7���� ���
unsigned char pattern;                 // ��Ʈ���̳뽺�� 4���� ����
unsigned char cur_line;               // ��Ʈ���ϳ뽺�� ���� ����
unsigned char cur_col;                // ��Ʈ���ϳ뽺�� ���� ĭ
unsigned long int temp_line[4];          // ��Ʈ���̳뽺 ���� �ӽ� �����
unsigned long int main_board[32] = { 0 };	//�׸����̳뽺�� ������ �� ����� ���Ӻ���
unsigned long int game_board[32];	//��Ʈ���̳뽺�� �����̸鼭 ��ȭ�ϴ� ���Ӻ���
unsigned char crush = 0;	//�ε����� ��Ÿ���� �÷���
unsigned char new_block = 0;	//���ο� ����� �����Ǿ����� ��Ÿ���� �÷���
unsigned char game_over = 0;	//������ ����Ǿ����� ��Ÿ���� �÷���
unsigned char next_block = 0;
unsigned char next_board[8] = { 0 };
unsigned char red;
unsigned char blue;
unsigned char green;


unsigned char Collision()	//�浹 ���� Ȯ��
{
	if (((main_board[cur_line] & temp_line[0]) != 0) | ((main_board[cur_line + 1] & temp_line[1]) != 0) |
		((main_board[cur_line + 2] & temp_line[2]) != 0) | ((main_board[cur_line + 3] & temp_line[3]) != 0))
		return 1;         // �浹 1 ����
	else
		return 0;  // �浹 ���� 0 ����
}


void tetriminos_to_temp_line()
{
	for (int i = 0; i < 4; i++)	//��Ʈ���̳뽺 ���� �ӽ� ����� �ʱ�ȭ
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
		temp_line[0] = ((unsigned long int)(tetriminos[shape][pattern] & 0xF000) >> 12) << (cur_col - 3);	//�� ������ �ʿ� ���� �ڿ��� �ѹ��� �浹���� ���� �־��൵ ������� �������� �浹 ���� �Ǻ��ϴ� �Լ��� temp_line�� ���κ��尡 ��ġ���� �ľ��ϱ⶧���� temp_line�� ��ȭ�� ������ �ݿ��ؾ� �Ѵ�.
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
				rectangle(img, Point(130 + 3 * i, 3 * j), Point(3 * i + 2, 3 * j + 2), Scalar(blue, green, red), -1);
			}
		}
	}
}

void NewTetriminos()
{
	new_block = 0;	//���ο� ��� �÷��� ����
	shape = next_block;
	next_block = rand() % 7;					//������ �� ��Ʈ���̳뽺 ���� ���
	pattern = 0;	//�⺻ ȸ������ ����
	cur_line = 0;                 // ��Ʈ���̳뽺 ���� ���� (�ֻ��� ����)
	cur_col = 6;                // ��Ʈ���̳뽺�� ���� ĭ

	tetriminos_to_temp_line();


	game_over |= game_board[cur_line] & temp_line[0];	//���Ӻ��忡 �ִ� ��Ʈ���̳뽺�� �ӽ�����ҿ� ���� ��Ʈ���̳뽺�� ��ġ���� Ȯ���ϰ� ��ġ�� ���ӿ��� �÷��� ����
	game_over |= game_board[cur_line + 1] & temp_line[1];	//or �������� ���� ��ġ�� �÷��� ������
	game_over |= game_board[cur_line + 2] & temp_line[2];
	game_over |= game_board[cur_line + 3] & temp_line[3];


	game_board[cur_line] |= temp_line[0];	//������� �Ʒ��� �̵� �� ��ȭ�ϴ� ���忡 �ݿ�
	game_board[cur_line + 1] |= temp_line[1];
	game_board[cur_line + 2] |= temp_line[2];
	game_board[cur_line + 3] |= temp_line[3];

}


void NextTetriminos()
{
	for (int i = 0; i < 8; i++)	//�ؽ�Ʈ ���� �ʱ�ȭ
	{
		next_board[i] = 0;
	}


	next_board[2] |= (unsigned char)((tetriminos[next_block][0] & 0xF000) >> 10);
	next_board[3] |= (unsigned char)((tetriminos[next_block][0] & 0x0F00) >> 6);
	next_board[4] |= (unsigned char)((tetriminos[next_block][0] & 0x00F0) >> 2);
	next_board[5] |= (unsigned char)((tetriminos[next_block][0] & 0x000F) << 2);


	unsigned char MAP[2][128] = { 0 };


	MAP[0][2] |= 0xFF;	//���� ���� 2ĭ ���� �ʹ� �β����� ���ٷ� �������
	MAP[1][2] |= 0xFF;
	MAP[0][17] |= 0xFF;
	MAP[1][17] |= 0xFF;

	for (int i = 2; i < 18; i++)
	{
		MAP[0][i] |= 0x01;
		MAP[1][i] |= 0x80;
	}


	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if ((next_board[i] & (1 << j)) != 0)
			{
				if (j < 4)
				{
					MAP[1][2 + (2 * i)] |= (0x03 << 2 * (3 - j));
					MAP[1][2 + (2 * i + 1)] |= (0x03 << 2 * (3 - j));
				}
				else
				{
					MAP[0][2 + (2 * i)] |= (0x03 << 2 * (7 - j));
					MAP[0][2 + (2 * i + 1)] |= (0x03 << 2 * (7 - j));
				}
			}
		}
	}

}


int main() {
	srand((unsigned int)time(NULL));

	VideoCapture cap(0);	//���� ĸ��

	while (1)
	{
		game_over = 0;	//�������� �÷��� ����


		for (int i = 0; i < 31; i++) main_board[i] = 0x801;	//���κ��� �ʱ�ȭ
		main_board[31] = 0xFFF;
		new_block = 1;


		while (game_over == 0)	//�������� �÷��װ� ������������ �ݺ�
		{
			cap >> img;	//���� ĸ�� ��������

			flip(img, img, 1);	//���� �¿� ����

			if (new_block == 1)
			{
				blue = rand() % 256;
				green = rand() % 256;
				red = rand() % 256;
				for (int i = 0; i < 31; i++)
				{
					if (main_board[i] == 0xFFF)
						//1���� ��� �ϼ��Ǿ ���� ���� �ִ��� Ȯ��
					{
						main_board[i] = 0x801;
						for (int k = i; k > 0; k--)
						{
							main_board[k] = main_board[k - 1];	//���� ���� ���� �ٵ� �Ʒ��� �̵�(��Ļ� �� ����)
						}
					}
				}

				for (int i = 0; i < 32; i++)
				{
					game_board[i] = main_board[i];	//�������� ����� ���带 ��ȭ�ϴ� ����� ����
				}

				NewTetriminos();	//���ο� ��Ʈ���̳뽺 ����
				//NextTetriminos();
			}

			if (new_block == 0)
			{
				draw_map();	//�ݿ��� ��ȭ�ϴ� ���� ȭ������ ���
				for (int i = 0; i < 32; i++)
				{
					game_board[i] = main_board[i];	//�������� ����� ���带 ��ȭ�ϴ� ����� ����
				}

				cur_line++;	//������� �Ʒ��� �̵�

				if (Collision() == 1)	//�̵��� �浹 �߻���
				{
					cur_line--;	//���� �������� ����
					main_board[cur_line] |= temp_line[0];	//������� �Ʒ��� �̵� �� ��ȭ�ϴ� ���忡 �ݿ�
					main_board[cur_line + 1] |= temp_line[1];
					main_board[cur_line + 2] |= temp_line[2];
					main_board[cur_line + 3] |= temp_line[3];

					new_block = 1;	//���ο� ��� �÷��� ����
				}

				for (int i = 0; i < 32; i++)
				{
					game_board[i] = main_board[i];	//�������� ����� ���带 ��ȭ�ϴ� ����� ����
				}

				game_board[cur_line] |= temp_line[0];	//������� �Ʒ��� �̵� �� ��ȭ�ϴ� ���忡 �ݿ�
				game_board[cur_line + 1] |= temp_line[1];
				game_board[cur_line + 2] |= temp_line[2];
				game_board[cur_line + 3] |= temp_line[3];

			}

			imshow("src", img);
			if (waitKey(1) == 27)
				return 0;
		}
	}	
}

