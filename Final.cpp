#pragma comment (lib, "msimg32.lib")
#include<windows.h>
#include<ctime>
#include "fmod.hpp"
#include "fmod_errors.h"
#include "resource.h"
FMOD::System* ssystem;
FMOD::Sound* s_bad_end, * correct, * incorrect, * s_laddle, * hit_sound, * stage1, * stage3, * scene_sound, * nurvous
, * fried, * grab_pan, * secret_hit, * stage2, * s_title, * ending;
FMOD::Channel* channel = 0;	//¹è°æÀ½¾Ç
FMOD::Channel* channel1 = 0;	//È¿°úÀ½
FMOD::Channel* channel2 = 0;	//±¹ÀÚ ¼Ò¸®
FMOD::Channel* channel3 = 0;	//È¯°æÀ½
FMOD_RESULT result;
void* extradriverdata = 0;

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"°ñ¸ñ ÀüÁý";

LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

struct LADLE {	//±¹ÀÚ ±¸Á¶Ã¼
	int x, y;	//±¹ÀÚÀÇ À§Ä¡
	HBITMAP ladle;	//±¹ÀÚ ºñÆ®¸Ê
};
LADLE ladle_03;

struct FIREBOARD {
	bool used;	//½Ã¿ëÁßÀÎ°¡?
	int vari;	//ÀüÀÇ Á¾·ù = P_select
	HBITMAP pancake; //Àü ºñÆ®¸Ê
	int count;	//¾ó¸¶µ¿¾È Á¶¸®Çß³ª
	RECT range;	//¾îµð¸¦ Å¬¸¯ÇØ¾ß ¼±ÅÃµÇ³ª
	int x, y;	//ÀüÀÇ À§Ä¡
	int width, height;	//Àü Å©±â
};

struct FALLINGPOT {
	int x, y;	//³¿ºñ À§Ä¡
	bool falling;	//¶³¾îÁö°í ÀÖ³ª?
	bool crash;		//Ãæµ¹Çß³ª?
	HBITMAP pot;
};
FIREBOARD f_board_03[8];
FIREBOARD Stack_03[10];
FALLINGPOT falling_pot_03;

struct ORDER {
	int vari;	//¼Õ´ÔÀÇ Á¾·ù1. ÀÏ¹ÝÀÎ, 2. ¹æÇØ²Û, 3. À¯¸íÀÎ
	bool place; //ÀÚ¸®¿¡ Á¤ÂøÇß³ª?(À§¿¡ ¿Ã¶ó¿Ô³ª)
	int x, y;	//¼Õ´ÔÀÇ À§Ä¡
	HBITMAP face;	//¼Õ´Ô ºñÆ®¸Ê
	double face_count;	//¾Ö´Ï¸ÞÀÌ¼Ç Ä«¿îÆ®
	int en_count;	//¹öÆ¾ ½Ã°£	15ÃÊ ÈÄ º¯È¯
	int or_top;		//¸îÃþ±îÁö ÁÖ¹®Çß³ª?
	FIREBOARD stack[10];	//¹» ÁÖ¹®Çß³ª?
	int complete = 0; //¹è´Þ ¿Ï·áÇß³ª?
	int complete_face = 0; //¹è´Þ ¿Ï·áÇßÀ» ¶§ Ç¥Á¤
	HBITMAP score; //Á¡¼ö ºñÆ®¸Ê
	int score_x = 0; //Á¡¼ö À§Ä¡
	int score_y = 0; //Á¡¼ö À§Ä¡
	int c_06 = 0;
	bool hit_pot;	//³¿ºñ¿¡ ¸Â¾Ò³ª?
	bool get_dish; //À½½ÄÀ» ¹Þ¾Ò³ª?
	bool correct; //¸Â¾Ò³ª?
	bool youangry; //Àü¿¡ È­³µ³ª?
};
ORDER order[3];	//0¿ÞÂÊ ¼Õ´Ô, 1, Áß¾Ó¼Õ´Ô, 2. ¿À¸¥ÂÊ ¼Õ´Ô


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE PrevhInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.hInstance = hInstance;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 1024, 768, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	srand(time(NULL));
	PAINTSTRUCT ps_03;
	static RECT rt_03;
	static HDC hDC_03, mDC1_03, mDC2_03;	//DCµé
	static HBITMAP hBit_03, background_03[7], oldBit_03[2];	//hBit, ¹è°æ, oldBit
	static HBITMAP fire_03[4], c_board_03, dish_03, pot_03;	//ºÒÆÇ, µµ¸¶, Á¢½Ã, ³¿ºñ
	static HBITMAP score_board_03, score_text_03, score_num_03[10];	//Á¡¼öÆÇ, Á¡¼ö ÅØ½ºÆ®, 0~9
	static HBITMAP life_board_03, heart_03[2];	//Ã¼·Â ÆÇ, ÇÏÆ®
	static HBITMAP talkBox_03, pancake_03[12], hit_pot_03, minus_heart_03;		//¸»Ç³¼±, ÀüÀÇ Á¾·ù, ºÎµúÈù ³¿ºñ, ÇÏÆ® °¨¼Ò
	static HBITMAP order_face_03[9];	//¼Õ´Ô ¾ó±¼
	static HBITMAP order_check_06[3]; //¼Õ´Ô ÀÀ´ë¿¡ µû¸¥ Á¡¼ö
	static HBITMAP villain_03[5];	//ºô·±ÀÇ ¾ó±¼ 0. º¸Åë 1~2. °íÅë 3~4. ¿ôÀ½
	static HBITMAP master_03[8];	//À¯¸íÀÎ ´ë±âºÎÅÍ ¶°³ª°¡±â±îÁö 0~2 ´ë±â 3~7¶°³ª°¨
	static HBITMAP camera_03, light_03;	//Ä«¸Þ¶ó¿Í Á¶¸í

	static int stage_03, fire_count_03;	//¸î¹øÂ° ½ºÅ×ÀÌÁö, ºÒÆÇ ¾Ö´Ï¸ÞÀÌ¼Ç Ä«¿îÆ®
	static int P_select_03, P_top_03, score_03;	//¹«½¼ ÀüÀ» °ñ¶ú³ª, Á¢½Ã¿¡ ÀüÀ» ¾ó¸¶³ª ½×¾Ò³ª? Á¡¼ö
	static int mx_03, my_03, cal_score_03, sub_i_03;	//¸¶¿ì½º ÁÂÇ¥, Á¡¼ö °è»ê¿ë º¯¼ö, Á¡¼ö°è»ê¿ë
	static int life_03;	//³²Àº Ã¼·Â
	static bool grab_pot_03;	//³¿ºñ¸¦ Àâ¾Ò´Â°¡?
	static int count_06 = 0; //¼Õ´Ô ÁÖ¹®°ú ³»°¡ ¸¸µç ¿ä¸®°¡ ¼­·Î ¸Â´ÂÁö È®ÀÎ¿ë
	static bool exist_v_03, isTimer5003;	//¹æÇØ²ÛÀÌ Á¸ÀçÇÏ³ª?, 5003Å¸ÀÌ¸Ó°¡ ÀÛµ¿ÁßÀÎ°¡
	static double v_percent_03, v_check_03;	//¹æÇØ²Û µîÀå È®·ü, ¹æÇØ²Û È®·ü Ã¼Å©
	static int success_03;	//ÁÖ¹® ¼º°ø È½¼ö

	switch (Message) {
	case WM_CREATE:
		stage_03 = 0;	//½ÃÀÛÀº 1½ºÅ×ÀÌÁö
		P_top_03 = 0;
		{
			heart_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP36));	//Á¤»óÇÏÆ®
			heart_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP35));	//ºÎ¼­Áø ÇÏÆ®
		}
		{//¹è°æ ºñÆ®¸Ê ÁöÁ¤
			background_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP10));	//Å¸ÀÌÆ²
			background_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));	//½ÃÀå¹è°æ
			background_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP70));	//½ÃÀå¹è°æ
			background_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP71));	//½ÃÀå¹è°æ
			background_03[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP72));	//ÇØÇÇ ¿£µù
			background_03[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP51));	//¹èµå ¿£µù
			background_03[6] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP69));	//¹èµå ¿£µù2
		}
		{//Á¡¼ö ÅØ½ºÆ®
			score_num_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP25));
			score_num_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP26));
			score_num_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP27));
			score_num_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP28));
			score_num_03[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP29));
			score_num_03[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP30));
			score_num_03[6] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP31));
			score_num_03[7] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP32));
			score_num_03[8] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP33));
			score_num_03[9] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP34));
		}
		{
			fire_count_03 = 0;
			fire_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
			fire_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));
			fire_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP4));
			fire_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP5));
		}//ºÒÆÇ ºñÆ®¸Ê
		{
			c_board_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));	//µµ¸¶
			dish_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP7));	//Á¢½Ã
			ladle_03.ladle = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP8));	//±¹ÀÚ
			pot_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP9));	//³¿ºñ
			score_board_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP23));	//Á¡¼öÆÇ
			score_text_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP24));	//Á¡¼ö ÅØ½ºÆ®
			life_board_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP37));	//ÇÏÆ® ÆÇ
			talkBox_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP39));	//¸»Ç³¼±
			hit_pot_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP47));	//ºÎ¼­Áø ³¿ºñ
			minus_heart_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP50));	//ÇÏÆ® °¨¼Ò
		}
		{//ÀüÀÇ Á¾·ù 0~2 ±èÄ¡ 3~5 ÆÄÀü 6~8 °¨ÀÚ 9~11 ¸Þ¹Ð 
			pancake_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP14));
			pancake_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP15));
			pancake_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP16));
			pancake_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP17));
			pancake_03[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP18));
			pancake_03[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP19));
			pancake_03[6] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP20));
			pancake_03[7] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP21));
			pancake_03[8] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP22));
			pancake_03[9] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP11));
			pancake_03[10] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP12));
			pancake_03[11] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP13));
		}
		{//¼Õ´Ô ¾ó±¼ 0Æò½Ã 1È­³² 2ºÐ³ë 3 ¸Ô´ÂÁß1, 4 ¸Ô´ÂÁß 2, 5°íÅë1, 6. °íÅë2
			order_face_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP41));
			order_face_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP40));
			order_face_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP38));
			order_face_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP42));
			order_face_03[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP43));
			order_face_03[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP48));
			order_face_03[6] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP49));
			order_face_03[7] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP67));
			order_face_03[8] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP68));
		}
		{
			order_check_06[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP46));
			order_check_06[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP45));
			order_check_06[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP44));
		}
		{
			villain_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP52));
			villain_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP53));
			villain_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP54));
			villain_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP55));
			villain_03[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP56));
		}
		{
			camera_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP58));
			light_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP57));
		}
		{
			master_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP59));
			master_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP60));
			master_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP61));
			master_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP62));
			master_03[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP63));
			master_03[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP64));
			master_03[6] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP65));
			master_03[7] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP66));
		}
		for (int i = 0; i < 8; ++i) {//ºÒÆÇ ±¸¿ª ÁöÁ¤
			f_board_03[i].used = false;
			if (i < 4) {
				f_board_03[i].range.left = f_board_03[i].x = 66 + (i * 119);
				f_board_03[i].range.right = 66 + ((i + 1) * 119);
				f_board_03[i].range.top = 465; f_board_03[i].range.bottom = 582;
				f_board_03[i].y = 495;
			}
			else {
				f_board_03[i].range.left = f_board_03[i].x = 66 + ((i - 4) * 119);
				f_board_03[i].range.right = 66 + ((i - 3) * 119);
				f_board_03[i].range.top = 582; f_board_03[i].range.bottom = 699;
				f_board_03[i].y = 612;
			}
			f_board_03[i].width = 117; f_board_03[i].height = 60;
		}
		for (int i = 0; i < 3; ++i) {//¼Õ´Ô ÃÊ±â ÁöÁ¤
			order[i].vari = 1;
			order[i].y = (i * 2000) + 500;
			order[i].x = (i * 322) + 38;
			order[i].score_x = (i * 322) + 45;
			order[i].score_y = 300;
			order[i].place = false;
			order[i].hit_pot = false;
			order[i].face = order_face_03[0];
			order[i].youangry = false;
			for (int j = 0; j < 10; ++j) {
				order[i].stack[j].x = (i * 320) + 225;
				order[i].stack[j].y = 210 - (j * 15);
				order[i].stack[j].width = 117;
				order[i].stack[j].height = 40;
			}
		}
		score_03 = 0;
		life_03 = 3;
		P_select_03 = 1;	//±èÄ¡Àü ¼±ÅÃ(Å°ÀÔ·ÂÀ¸·Î ¹Ù²Þ)
		ladle_03.x = 670; ladle_03.y = 425;	//ÃÊ±â ±¹ÀÚ À§Ä¡
		grab_pot_03 = false;
		v_percent_03 = 0;
		success_03 = 0;
		isTimer5003 = false;
		{
			falling_pot_03.crash = false;
			falling_pot_03.falling = false;
			falling_pot_03.x = falling_pot_03.y = -1000;
			falling_pot_03.pot = pot_03;
		}
		result = FMOD::System_Create(&ssystem);
		if (result != FMOD_OK)
			exit(0);

		ssystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
		ssystem->createSound("sound\\1997 spring.mp3",
			FMOD_LOOP_NORMAL, 0, &s_bad_end);	//¹èµå¿£µù »ç¿îµå
		ssystem->createSound("sound\\correct.mp3",
			FMOD_DEFAULT, 0, &correct);		//¸Â­ŸÀ»¶§ »ç¿îµå
		ssystem->createSound("sound\\incorrect.mp3",
			FMOD_DEFAULT, 0, &incorrect);	//Æ²·ÈÀ»¶§ »ç¿îµå
		ssystem->createSound("sound\\laddle.wav",
			FMOD_DEFAULT, 0, &s_laddle);	//±¹ÀÚ »ç¿îµå
		ssystem->createSound("sound\\pot_hit.mp3",
			FMOD_DEFAULT, 0, &hit_sound);	//³¿ºñ »ç¿îµå
		ssystem->createSound("sound\\stage1.mp3", FMOD_LOOP_NORMAL, 0, &stage1);
		ssystem->createSound("sound\\stage3.mp3", FMOD_LOOP_NORMAL, 0, &stage3);
		ssystem->createSound("sound\\scene sound.mp3", FMOD_LOOP_NORMAL, 0, &scene_sound);
		ssystem->createSound("sound\\nurvous.mp3", FMOD_LOOP_NORMAL, 0, &nurvous);
		ssystem->createSound("sound\\fried.mp3", FMOD_DEFAULT, 0, &fried);
		ssystem->createSound("sound\\grab_pan.mp3", FMOD_DEFAULT, 0, &grab_pan);
		ssystem->createSound("sound\\secret_hit.mp3", FMOD_DEFAULT, 0, &secret_hit);
		ssystem->createSound("sound\\stage2.mp3", FMOD_LOOP_NORMAL, 0, &stage2);
		ssystem->createSound("sound\\title.mp3", FMOD_LOOP_NORMAL, 0, &s_title);
		ssystem->createSound("sound\\ending.mp3", FMOD_LOOP_NORMAL, 0, &ending);
		channel->stop();	//¹è°æÀ½ Ã¤³Î
		channel1->stop();	//È¿°úÀ½ Ã¤³Î
		channel2->stop();	//±¹ÀÚ ÀÌµ¿¼Ò¸® Ã¤³Î
		channel3->stop();
		ssystem->playSound(s_title, 0, false, &channel);
		channel->setVolume(0.3);	//¹è°æÀ½ Ã¤³Î
		break;
	case WM_PAINT:
		GetClientRect(hWnd, &rt_03);
		hDC_03 = BeginPaint(hWnd, &ps_03);
		hBit_03 = CreateCompatibleBitmap(hDC_03, rt_03.right, rt_03.bottom);
		mDC1_03 = CreateCompatibleDC(hDC_03);
		mDC2_03 = CreateCompatibleDC(mDC1_03);
		oldBit_03[0] = (HBITMAP)SelectObject(mDC1_03, hBit_03);
		//½ºÅ×ÀÌÁö¿¡ µû¸¥ ¹è°æ ¼³Á¤ 0 ½ÃÀÛÀü, 1~3 °ÔÀÓÁß, 4~¿£µù, 5¹èµå¿£µù, 6. ¹èµå2
		{
			if (stage_03 == 0) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[0]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1024, 768, SRCCOPY);//¹è°æ ³Ö±â
			}
			else if (stage_03 == 1) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[1]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1280, 853, SRCCOPY);//¹è°æ ³Ö±â
			}
			else if (stage_03 == 2) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[2]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1280, 853, SRCCOPY);//¹è°æ ³Ö±â
			}
			else if (stage_03 == 3) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[3]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1280, 853, SRCCOPY);//¹è°æ ³Ö±â
			}
			else if (stage_03 == 4) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[4]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1024, 768, SRCCOPY);//¹è°æ ³Ö±â

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_text_03);	//
				TransparentBlt(mDC1_03, 373, 580, 90, 30, mDC2_03, 0, 0, 90, 30, RGB(34, 177, 76)); //Á¡¼ö ÅØ½ºÆ® ³Ö±â
				if (score_03 >= 99999) {
					score_03 = 99999;
				}
				cal_score_03 = score_03;
				sub_i_03 = 0;
				for (int i = 10000; i >= 1; i /= 10) {
					switch (cal_score_03 / i) {
					case 0:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[0]);
						break;
					case 1:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[1]);
						break;
					case 2:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[2]);
						break;
					case 3:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[3]);
						break;
					case 4:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[4]);
						break;
					case 5:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[5]);
						break;
					case 6:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[6]);
						break;
					case 7:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[7]);
						break;
					case 8:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[8]);
						break;
					case 9:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[9]);
						break;
					}
					TransparentBlt(mDC1_03, (sub_i_03 * 30) + 488, 580, 30, 30, mDC2_03, 0, 0, 30, 30, RGB(34, 177, 76)); //Á¡¼ö ÅØ½ºÆ® ³Ö±â
					cal_score_03 -= (cal_score_03 / i) * i;
					sub_i_03++;
				}
			}
			else if (stage_03 == 5) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[5]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1024, 768, SRCCOPY);//¹è°æ ³Ö±â

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_text_03);	//
				TransparentBlt(mDC1_03, 360, 400, 90, 30, mDC2_03, 0, 0, 90, 30, RGB(34, 177, 76)); //Á¡¼ö ÅØ½ºÆ® ³Ö±â
				if (score_03 >= 99999) {
					score_03 = 99999;
				}
				cal_score_03 = score_03;
				sub_i_03 = 0;
				for (int i = 10000; i >= 1; i /= 10) {
					switch (cal_score_03 / i) {
					case 0:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[0]);
						break;
					case 1:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[1]);
						break;
					case 2:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[2]);
						break;
					case 3:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[3]);
						break;
					case 4:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[4]);
						break;
					case 5:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[5]);
						break;
					case 6:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[6]);
						break;
					case 7:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[7]);
						break;
					case 8:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[8]);
						break;
					case 9:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[9]);
						break;
					}
					TransparentBlt(mDC1_03, (sub_i_03 * 30) + 475, 400, 30, 30, mDC2_03, 0, 0, 30, 30, RGB(34, 177, 76)); //Á¡¼ö ÅØ½ºÆ® ³Ö±â
					cal_score_03 -= (cal_score_03 / i) * i;
					sub_i_03++;
				}
			}
			else if (stage_03 == 6) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[6]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1024, 768, SRCCOPY);//¹è°æ ³Ö±â

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_text_03);	//
				TransparentBlt(mDC1_03, 360, 50, 90, 30, mDC2_03, 0, 0, 90, 30, RGB(34, 177, 76)); //Á¡¼ö ÅØ½ºÆ® ³Ö±â
				if (score_03 >= 99999) {
					score_03 = 99999;
				}
				cal_score_03 = score_03;
				sub_i_03 = 0;
				for (int i = 10000; i >= 1; i /= 10) {
					switch (cal_score_03 / i) {
					case 0:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[0]);
						break;
					case 1:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[1]);
						break;
					case 2:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[2]);
						break;
					case 3:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[3]);
						break;
					case 4:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[4]);
						break;
					case 5:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[5]);
						break;
					case 6:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[6]);
						break;
					case 7:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[7]);
						break;
					case 8:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[8]);
						break;
					case 9:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[9]);
						break;
					}
					TransparentBlt(mDC1_03, (sub_i_03 * 30) + 475, 50, 30, 30, mDC2_03, 0, 0, 30, 30, RGB(34, 177, 76)); //Á¡¼ö ÅØ½ºÆ® ³Ö±â
					cal_score_03 -= (cal_score_03 / i) * i;
					sub_i_03++;
				}
			}

			if (stage_03 >= 1 && stage_03 <=3) {
				{//¼Õ´Ô Ãâ·Â
					for (int i = 0; i < 3; ++i) {
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, order[i].face);
						TransparentBlt(mDC1_03, order[i].x, order[i].y, 167, 260, mDC2_03, 0, 0, 167, 260, RGB(34, 177, 76));//¼Õ´Ô Ãâ·Â
						if ((order[i].vari == 1 || order[i].vari == 3) && order[i].y == 180) {
							if (order[i].place && order[i].y == 180 && !order[i].hit_pot) {
								oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, talkBox_03);
								TransparentBlt(mDC1_03, (i * 320) + 205, 130, 155, 150,
									mDC2_03, 0, 0, 155, 150, RGB(34, 177, 76));//¸»Ç³¼± Ãâ·Â

								oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, dish_03);
								TransparentBlt(mDC1_03, (i * 320) + 220, 210, 127, 50,
									mDC2_03, 0, 0, 127, 54, RGB(34, 177, 76));//Á¢½Ã Ãâ·Â
								for (int j = 0; j < order[i].or_top; ++j) {
									oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, order[i].stack[j].pancake);
									TransparentBlt(mDC1_03, order[i].stack[j].x, order[i].stack[j].y, order[i].stack[j].width, order[i].stack[j].height,
										mDC2_03, 0, 0, 380, 209, RGB(34, 177, 76));
								}
							}
						}
						if (order[i].complete == 1) {
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, order[i].score);
							TransparentBlt(mDC1_03, order[i].score_x, order[i].score_y, 140, 50,
								mDC2_03, 0, 0, 140, 50, RGB(34, 177, 76));
						}
					}
				}
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, falling_pot_03.pot);
				TransparentBlt(mDC1_03, falling_pot_03.x, falling_pot_03.y, 167, 50, mDC2_03, 0, 0, 241, 114,RGB(34, 177, 76));	//¶³¾îÁö´Â ³¿ºñ ³Ö±â
				
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, fire_03[fire_count_03]);
				StretchBlt(mDC1_03, 0, 440, 740, 289, mDC2_03, 0, 0, 740, 289, SRCCOPY);	//ºÒÆÇ ³Ö±â

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, c_board_03);
				StretchBlt(mDC1_03, 740, 440, 268, 289, mDC2_03, 0, 0, 268, 289, SRCCOPY);	//µµ¸¶ ³Ö±â

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, pot_03);
				TransparentBlt(mDC1_03, 740, 440, 268, 145, mDC2_03, 0, 0, 241, 114, RGB(34, 177, 76)); //³¿ºñ ³Ö±â

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, dish_03);
				TransparentBlt(mDC1_03, 740, 584, 268, 145, mDC2_03, 0, 0, 127, 54, RGB(34, 177, 76)); //Á¢ºñ ³Ö±â

				{//Á¡¼ö Ç¥½Ã
					oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_board_03);
					TransparentBlt(mDC1_03, 0, 0, 300, 60, mDC2_03, 0, 0, 300, 60, RGB(34, 177, 76)); //Á¡¼öÆÇ ³Ö±â

					oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_text_03);
					TransparentBlt(mDC1_03, 10, 15, 90, 30, mDC2_03, 0, 0, 90, 30, RGB(34, 177, 76)); //Á¡¼ö ÅØ½ºÆ® ³Ö±â
					if (score_03 >= 99999) {
						score_03 = 99999;
					}
					cal_score_03 = score_03;
					sub_i_03 = 0;
					for (int i = 10000; i >= 1; i /= 10) {
						switch (cal_score_03 / i) {
						case 0:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[0]);
							break;
						case 1:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[1]);
							break;
						case 2:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[2]);
							break;
						case 3:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[3]);
							break;
						case 4:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[4]);
							break;
						case 5:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[5]);
							break;
						case 6:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[6]);
							break;
						case 7:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[7]);
							break;
						case 8:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[8]);
							break;
						case 9:
							oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_num_03[9]);
							break;
						}
						TransparentBlt(mDC1_03, (sub_i_03 * 30) + 125, 15, 30, 30, mDC2_03, 0, 0, 30, 30, RGB(34, 177, 76)); //Á¡¼ö ÅØ½ºÆ® ³Ö±â
						cal_score_03 -= (cal_score_03 / i) * i;
						sub_i_03++;
					}
				}
				{//ÇÏÆ® Ç¥±â
					oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, life_board_03);
					TransparentBlt(mDC1_03, 838, 0, 170, 60, mDC2_03, 0, 0, 170, 60, RGB(34, 177, 76)); //Ã¼·Â ÆÇ ³Ö±â
					switch (life_03) {
					case 0:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, heart_03[1]);
						TransparentBlt(mDC1_03, 843, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						TransparentBlt(mDC1_03, 898, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						TransparentBlt(mDC1_03, 953, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						break;
					case 1:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, heart_03[1]);
						TransparentBlt(mDC1_03, 843, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						TransparentBlt(mDC1_03, 898, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, heart_03[0]);
						TransparentBlt(mDC1_03, 953, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						break;
					case 2:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, heart_03[1]);
						TransparentBlt(mDC1_03, 843, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, heart_03[0]);
						TransparentBlt(mDC1_03, 898, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						TransparentBlt(mDC1_03, 953, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						break;
					case 3:
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, heart_03[0]);
						TransparentBlt(mDC1_03, 843, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						TransparentBlt(mDC1_03, 898, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						TransparentBlt(mDC1_03, 953, 5, 50, 50, mDC2_03, 0, 0, 50, 50, RGB(34, 177, 76));
						break;
					}
				}

				for (int i = 0; i < P_top_03; ++i) {
					oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, Stack_03[i].pancake);
					TransparentBlt(mDC1_03, Stack_03[i].x, Stack_03[i].y, Stack_03[i].width, Stack_03[i].height, mDC2_03, 0, 0, 380, 209, RGB(34, 177, 76)); //Á¢ºñ ³Ö±â
				}

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, ladle_03.ladle);
				TransparentBlt(mDC1_03, ladle_03.x, ladle_03.y, 70, 100, mDC2_03, 0, 0, 104, 222, RGB(34, 177, 76));	//±¹ÀÚ ³Ö±â

				for (int i = 0; i < 8; ++i) {
					if (f_board_03[i].used) {
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, f_board_03[i].pancake);
						TransparentBlt(mDC1_03, f_board_03[i].x, f_board_03[i].y, f_board_03[i].width, f_board_03[i].height,
							mDC2_03, 0, 0, 380, 209, RGB(34, 177, 76));	//ºÒÆÇ¿¡ Àü ³Ö±â
					}
				}
			}
		}
		BitBlt(hDC_03, 0, 0, rt_03.right, rt_03.bottom, mDC1_03, 0, 0, SRCCOPY);
		SelectObject(mDC1_03, oldBit_03[0]);
		DeleteObject(hBit_03);
		DeleteDC(mDC2_03);
		DeleteDC(mDC1_03);
		EndPaint(hWnd, &ps_03);
		break;
	case WM_TIMER:
		switch (wParam) {
		case 1003: {//ºÒÆÇ ¾Ö´Ï¸ÞÀÌ¼Ç
			fire_count_03++;
			fire_count_03 %= 4;
			break;
		}
		case 2003: {//ºÒÆÇ º° ÀüÀÇ Ä«¿îÆ® Áõ°¡
			for (int i = 0; i < 8; ++i) {
				if (f_board_03[i].used) {
					f_board_03[i].count++;
					if (f_board_03[i].count == 7) {
						switch (f_board_03[i].vari) {
						case 1:
							f_board_03[i].pancake = pancake_03[1];
							break;
						case 2:
							f_board_03[i].pancake = pancake_03[4];
							break;
						case 3:
							f_board_03[i].pancake = pancake_03[7];
							break;
						case 4:
							f_board_03[i].pancake = pancake_03[10];
							break;
						}
					}
					else if (f_board_03[i].count == 17) {
						switch (f_board_03[i].vari) {
						case 1:
							f_board_03[i].pancake = pancake_03[2];
							break;
						case 2:
							f_board_03[i].pancake = pancake_03[5];
							break;
						case 3:
							f_board_03[i].pancake = pancake_03[8];
							break;
						case 4:
							f_board_03[i].pancake = pancake_03[11];
							break;
						}
					}
				}
			}
			break;
		}
		case 3003: {//³¿ºñÈ÷Æ®
			if (falling_pot_03.crash) {
				falling_pot_03.crash = false;
				falling_pot_03.x = falling_pot_03.y = -1000;
			}
			KillTimer(hWnd, 3003);
			break;
		}
		case 4003: {//1¹ø ¼Õ´Ô °³º° Ä«¿îÆ®
			if (order[0].hit_pot) {
				order[0].score_y -= 10;
				if (order[0].vari == 1) {
					if (order[0].c_06 % 2 == 0)
						order[0].face = order_face_03[5];
					else if (order[0].c_06 % 2 == 1) {
						order[0].face = order_face_03[6];
					}
				}
				else if (order[0].vari == 2) {
					if (order[0].c_06 % 2 == 0)
						order[0].face = villain_03[1];
					else if (order[0].c_06 % 2 == 1) {
						order[0].face = villain_03[2];
					}
				}
				order[0].c_06++;
				if (order[0].c_06 >= 4) {
					order[0].hit_pot = false;
					order[0].c_06 = 0;
					if (order[0].vari != 4) {
						order[0].y += 40;
						order[0].face_count = 100;
						order[0].complete = 0;
						order[0].score_y = 300;
						order[0].youangry = true;
						if (order[0].vari == 1)
							life_03--;
					}
					KillTimer(hWnd, 4003);
				}
			}
			else if (order[0].get_dish) { //ÁÖ¹®ÇÑ ¸Þ´º¸¦ ¹Þ¾ÒÀ» ¶§
				if (order[0].correct == true) { //¸Â¾ÒÀ» ¶§
					if (order[0].vari == 1) {
						if (order[0].c_06 % 2 == 0)
							order[0].face = order_face_03[3];
						else if (order[0].c_06 % 2 == 1) {
							order[0].face = order_face_03[4];
						}
					}
					else if (order[0].vari == 2) {
						if (order[0].c_06 % 2 == 0)
							order[0].face = villain_03[3];
						else if (order[0].c_06 % 2 == 1) {
							order[0].face = villain_03[4];
						}
					}
					order[0].score_y -= 10; //¾ó±¼¿¡ µû¸¥ Á¡¼öÇ¥ »ó½Â
					if (order[0].c_06 == 4) { //4¹ø ¾Æ±×ÀÛ¾Æ±×ÀÛÇÏ°í Á¾·á
						if (order[0].vari == 1) {
							order[0].c_06 = 0;
							order[0].face = order_face_03[0]; //Ã³À½ Ç¥Á¤À¸·Î ³»·Á°¡µµ·Ï ¼³Á¤
							order[0].face_count = 21; //¹Ù·Î ÇÏ°­ÇÏµµ·Ï
							order[0].complete = 0; //¿Ï·á Ã³¸® ¾ÈÇÑ°É·Î ¼öÁ¤
							order[0].score_y = 300; //Á¡¼ö ¿ø·¡ ÁÂÇ¥¿¡¼­ ´ë±â
							//order[0].get_dish = false;
							order[0].correct = false;
							success_03++;
							if (success_03 >= 5) {
								for (int i = 0; i < 3; ++i) {
									order[i].youangry = true;
									order[i].face_count = 100;
									order[i].complete = 0;
								}
							}
							KillTimer(hWnd, 4003);
						}
						else if (order[0].vari == 2) {
							order[0].face = villain_03[0]; //Ã³À½ Ç¥Á¤
							order[0].get_dish = false;
							order[0].correct = false;
							order[0].complete = 0;
							order[0].score_y = 300; //Á¡¼ö ¿ø·¡ ÁÂÇ¥¿¡¼­ ´ë±â
							KillTimer(hWnd, 4003);
						}
					}
					order[0].c_06++;
				}
				else { //Àß¸ø ÁáÀ» ¶§
					if (order[0].vari == 1) {
						if (order[0].c_06 % 2 == 0)
							order[0].face = order_face_03[7];
						else if (order[0].c_06 % 2 == 1) {
							order[0].face = order_face_03[8];
						}
					}
					else if (order[0].vari == 2) {
						if (order[0].c_06 % 2 == 0)
							order[0].face = villain_03[3];
						else if (order[0].c_06 % 2 == 1) {
							order[0].face = villain_03[4];
						}
					}
					order[0].c_06++;
					order[0].score_y -= 10; //¾ó±¼¿¡ µû¸¥ Á¡¼öÇ¥ »ó½Â
					if (order[0].c_06 >= 4) {
						if (order[0].vari == 1) {
							order[0].c_06 = 0;
							order[0].face = order_face_03[2]; //È­³­ Ç¥Á¤À¸·Î ³»·Á°¡µµ·Ï ¼³Á¤
							order[0].face_count = 21; //¹Ù·Î ÇÏ°­ÇÏµµ·Ï
							order[0].complete = 0; //¿Ï·á Ã³¸® ¾ÈÇÑ°É·Î ¼öÁ¤
							//order[0].get_dish = false;
							order[0].correct = false;
							order[0].score_y = 300; //Á¡¼ö ¿ø·¡ ÁÂÇ¥¿¡¼­ ´ë±â
							if (order[0].vari == 1) {
								life_03--;
							}
							KillTimer(hWnd, 4003);
						}
						else if (order[0].vari == 2) {
							order[0].face = villain_03[0]; //Ã³À½ Ç¥Á¤
							order[0].get_dish = false;
							order[0].correct = false;
							order[0].score_y = 300; //Á¡¼ö ¿ø·¡ ÁÂÇ¥¿¡¼­ ´ë±â
							order[0].complete = 0;
							KillTimer(hWnd, 4003);
						}
					}
				}
			}
			break;
		}
		case 5003: {//2¹ø ¼Õ´Ô °³º° Ä«¿îÆ®
			if (order[1].vari == 3) {	//À¯¸íÀÎÀÏ¶§
				order[1].face = master_03[order[1].c_06 + 3];
				order[1].c_06++;
				if (order[1].c_06 >= 6) {
					if (order[1].correct) {//¸ÂÀ¸¸é ½ºÅ×ÀÌÁö up
						stage_03++;
						if (stage_03 == 2) {
							channel->stop();
							channel3->stop();
							ssystem->playSound(stage2, 0, false, &channel);
							ssystem->playSound(scene_sound, 0, false, &channel3);
							channel->setVolume(0.5);
							channel3->setVolume(0.35);
						}
						else if (stage_03 == 3) {
							channel->stop();
							channel3->stop();
							ssystem->playSound(stage3, 0, false, &channel);
							channel->setVolume(0.5);
							ssystem->playSound(scene_sound, 0, false, &channel3);
							channel3->setVolume(0.35);
						}
						else if (stage_03 == 4) {
							channel->stop();
							channel3->stop();
							ssystem->playSound(ending, 0, false, &channel);
							channel->setVolume(0.3);

						}
						if (life_03 < 3)
							life_03++;
						P_top_03 = 0;	//Á¢½Ã ÃÊ±âÈ­
						for (int i = 0; i < 8; ++i) {//ºÒÆÇ ÃÊ±âÈ­
							f_board_03[i].used = false;
						}
						for (int i = 0; i < 3; ++i) {//¼Õ´Ô ÃÊ±â ÁöÁ¤
							order[i].vari = 1;
							order[i].y = (i * 2000) + 500;
							order[i].x = (i * 322) + 38;
							order[i].score_x = (i * 322) + 45;
							order[i].score_y = 300;
							order[i].place = false;
							order[i].hit_pot = false;
							order[i].youangry = false;
							order[i].face = order_face_03[0];
							order[i].get_dish = false;
						}
						P_select_03 = 1;	//±èÄ¡Àü ¼±ÅÃ(Å°ÀÔ·ÂÀ¸·Î ¹Ù²Þ)
						ladle_03.x = 670; ladle_03.y = 425;	//ÃÊ±â ±¹ÀÚ À§Ä¡
						grab_pot_03 = false;
						v_percent_03 = 0;
						success_03 = 0;
						isTimer5003 = false;
						{
							falling_pot_03.crash = false;
							falling_pot_03.falling = false;
							falling_pot_03.x = falling_pot_03.y = -1000;
							falling_pot_03.pot = pot_03;
						}
						KillTimer(hWnd, 5003);
						if (stage_03 != 4) {
							SetTimer(hWnd, 1003, 200, NULL);//ºÒÆÇ Å¸ÀÌ¸Ó
							SetTimer(hWnd, 2003, 1000, NULL);	//Àü Á¶¸® Ä«¿îÆ®
							SetTimer(hWnd, 1006, 40, NULL);	//¼Õ´Ô ÀÌµ¿
							SetTimer(hWnd, 2006, 1000, NULL);	//¼Õ´ÔÀÇ ´ë±â½Ã°£
						}
					}
					else {	//Æ²¸®¸é ¿£µù
						stage_03 = 6;
						channel->stop();
						ssystem->playSound(s_bad_end, 0, false, &channel);
						channel->setVolume(0.5);
						KillTimer(hWnd, 1003);
						KillTimer(hWnd, 2003);
						KillTimer(hWnd, 3003);
						KillTimer(hWnd, 4003);
						KillTimer(hWnd, 5003);
						KillTimer(hWnd, 6003);

						KillTimer(hWnd, 1006);
						KillTimer(hWnd, 2006);
						KillTimer(hWnd, 3006);
					}
				}
			}
			else if (order[1].hit_pot) {
				order[1].score_y -= 10;
				if (order[1].vari == 1) {
					if (order[1].c_06 % 2 == 0)
						order[1].face = order_face_03[5];
					else if (order[1].c_06 % 2 == 1) {
						order[1].face = order_face_03[6];
					}
				}
				else if (order[1].vari == 2) {
					if (order[1].c_06 % 2 == 0)
						order[1].face = villain_03[1];
					else if (order[1].c_06 % 2 == 1) {
						order[1].face = villain_03[2];
					}
				}
				order[1].c_06++;
				if (order[1].vari != 3) {
					if (order[1].c_06 >= 4) {
						order[1].y += 40;
						order[1].face_count = 100;
						order[1].hit_pot = false;
						order[1].complete = 0;
						order[1].score_y = 300;
						order[1].youangry = true;
						if (order[1].vari == 1)
							life_03--;
						KillTimer(hWnd, 5003);
					}
				}
			}
			else if (order[1].get_dish) { //ÁÖ¹®ÇÑ ¸Þ´º¸¦ ¹Þ¾ÒÀ» ¶§
				if (order[1].correct == true) { //¸Â¾ÒÀ» ¶§
					if (order[1].vari == 1) {
						if (order[1].c_06 % 2 == 0)
							order[1].face = order_face_03[3];
						else if (order[1].c_06 % 2 == 1) {
							order[1].face = order_face_03[4];
						}
					}
					else if (order[1].vari == 2) {
						if (order[1].c_06 % 2 == 0)
							order[1].face = villain_03[3];
						else if (order[1].c_06 % 2 == 1) {
							order[1].face = villain_03[4];
						}
					}
					order[1].score_y -= 10; //¾ó±¼¿¡ µû¸¥ Á¡¼öÇ¥ »ó½Â
					if (order[1].c_06 == 4) { //4¹ø ¾Æ±×ÀÛ¾Æ±×ÀÛÇÏ°í Á¾·á
						if (order[1].vari == 1) {
							order[1].c_06 = 0;
							order[1].face = order_face_03[0]; //Ã³À½ Ç¥Á¤À¸·Î ³»·Á°¡µµ·Ï ¼³Á¤
							order[1].face_count = 21; //¹Ù·Î ÇÏ°­ÇÏµµ·Ï
							order[1].complete = 0; //¿Ï·á Ã³¸® ¾ÈÇÑ°É·Î ¼öÁ¤
							order[1].score_y = 300; //Á¡¼ö ¿ø·¡ ÁÂÇ¥¿¡¼­ ´ë±â
							//order[1].get_dish = false;
							order[1].correct = false;
							success_03++;
							if (success_03 >= 5) {
								for (int i = 0; i < 3; ++i) {
									order[i].youangry = true;
									order[i].face_count = 100;
									order[i].complete = 0;
								}
							}
							KillTimer(hWnd, 5003);
						}
						else if (order[1].vari == 2) {
							order[1].face = villain_03[0]; //Ã³À½ Ç¥Á¤
							order[1].get_dish = false;
							order[1].correct = false;
							order[1].complete = 0;
							order[1].score_y = 300; //Á¡¼ö ¿ø·¡ ÁÂÇ¥¿¡¼­ ´ë±â
							KillTimer(hWnd, 5003);
						}
					}
					order[1].c_06++;
				}
				else { //Àß¸ø ÁáÀ» ¶§
					if (order[1].vari == 1) {
						if (order[1].c_06 % 2 == 0)
							order[1].face = order_face_03[7];
						else if (order[1].c_06 % 2 == 1) {
							order[1].face = order_face_03[8];
						}
					}
					else if (order[1].vari == 2) {
						if (order[1].c_06 % 2 == 0)
							order[1].face = villain_03[3];
						else if (order[1].c_06 % 2 == 1) {
							order[1].face = villain_03[4];
						}
					}
					order[1].c_06++;
					order[1].score_y -= 10; //¾ó±¼¿¡ µû¸¥ Á¡¼öÇ¥ »ó½Â
					if (order[1].c_06 >= 4) {
						if (order[1].vari == 1) {
							order[1].c_06 = 0;
							order[1].face = order_face_03[2]; //È­³­ Ç¥Á¤À¸·Î ³»·Á°¡µµ·Ï ¼³Á¤
							order[1].face_count = 21; //¹Ù·Î ÇÏ°­ÇÏµµ·Ï
							order[1].complete = 0; //¿Ï·á Ã³¸® ¾ÈÇÑ°É·Î ¼öÁ¤
							//order[1].get_dish = false;
							order[1].correct = false;
							order[1].score_y = 300; //Á¡¼ö ¿ø·¡ ÁÂÇ¥¿¡¼­ ´ë±â
							life_03--;
							KillTimer(hWnd, 5003);
						}
						else if (order[1].vari == 2) {
							order[1].face = villain_03[0]; //Ã³À½ Ç¥Á¤
							order[1].get_dish = false;
							order[1].correct = false;
							order[1].complete = 0;
							order[1].score_y = 300; //Á¡¼ö ¿ø·¡ ÁÂÇ¥¿¡¼­ ´ë±â
							KillTimer(hWnd, 5003);
						}
					}
				}
			}
			break;
		}
		case 6003: {//3¹ø ¼Õ´Ô °³º° Ä«¿îÆ®
			if (order[2].hit_pot) {
				order[2].score_y -= 10;
				if (order[2].vari == 1) {
					if (order[2].c_06 % 2 == 0)
						order[2].face = order_face_03[5];
					else if (order[2].c_06 % 2 == 1) {
						order[2].face = order_face_03[6];
					}
				}
				else if (order[2].vari == 2) {
					if (order[2].c_06 % 2 == 0)
						order[2].face = villain_03[1];
					else if (order[2].c_06 % 2 == 1) {
						order[2].face = villain_03[2];
					}
				}
				order[2].c_06++;
				if (order[2].c_06 >= 4) {
					order[2].hit_pot = false;
					order[2].c_06 = 0;
					if (order[2].vari != 5) {
						order[2].y += 40;
						order[2].face_count = 100;
						order[2].complete = 0;
						order[2].score_y = 300;
						order[2].youangry = true;
						if (order[2].vari == 1)
							life_03--;
					}
					KillTimer(hWnd, 6003);
				}
			}
			else if (order[2].get_dish) { //ÁÖ¹®ÇÑ ¸Þ´º¸¦ ¹Þ¾ÒÀ» ¶§
				if (order[2].correct == true) { //¸Â¾ÒÀ» ¶§
					if (order[2].vari == 1) {
						if (order[2].c_06 % 2 == 0)
							order[2].face = order_face_03[3];
						else if (order[2].c_06 % 2 == 1) {
							order[2].face = order_face_03[4];
						}
					}
					else if (order[2].vari == 2) {
						if (order[2].c_06 % 2 == 0)
							order[2].face = villain_03[3];
						else if (order[2].c_06 % 2 == 1) {
							order[2].face = villain_03[4];
						}
					}
					order[2].score_y -= 10; //¾ó±¼¿¡ µû¸¥ Á¡¼öÇ¥ »ó½Â
					if (order[2].c_06 == 4) { //4¹ø ¾Æ±×ÀÛ¾Æ±×ÀÛÇÏ°í Á¾·á
						if (order[2].vari == 1) {
							order[2].c_06 = 0;
							order[2].face = order_face_03[0]; //Ã³À½ Ç¥Á¤À¸·Î ³»·Á°¡µµ·Ï ¼³Á¤
							order[2].face_count = 21; //¹Ù·Î ÇÏ°­ÇÏµµ·Ï
							order[2].complete = 0; //¿Ï·á Ã³¸® ¾ÈÇÑ°É·Î ¼öÁ¤
							//order[2].get_dish = false;
							order[2].correct = false;
							order[2].score_y = 300; //Á¡¼ö ¿ø·¡ ÁÂÇ¥¿¡¼­ ´ë±â
							success_03++;
							if (success_03 >= 5) {
								for (int i = 0; i < 3; ++i) {
									order[i].youangry = true;
									order[i].face_count = 100;
									order[i].complete = 0;
								}
							}
							KillTimer(hWnd, 6003);
						}
						else if (order[2].vari == 2) {
							order[2].face = villain_03[0]; //Ã³À½ Ç¥Á¤
							order[2].get_dish = false;
							order[2].correct = false;
							order[2].complete = 0;
							order[2].score_y = 300; //Á¡¼ö ¿ø·¡ ÁÂÇ¥¿¡¼­ ´ë±â
							KillTimer(hWnd, 6003);
						}
					}
					order[2].c_06++;
				}
				else { //Àß¸ø ÁáÀ» ¶§
					if (order[2].vari == 1) {
						if (order[2].c_06 % 2 == 0)
							order[2].face = order_face_03[7];
						else if (order[2].c_06 % 2 == 1) {
							order[2].face = order_face_03[8];
						}
					}
					else if (order[2].vari == 2) {
						if (order[2].c_06 % 2 == 0)
							order[2].face = villain_03[3];
						else if (order[2].c_06 % 2 == 1) {
							order[2].face = villain_03[4];
						}
					}
					order[2].c_06++;
					order[2].score_y -= 10; //¾ó±¼¿¡ µû¸¥ Á¡¼öÇ¥ »ó½Â
					if (order[2].c_06 >= 4) {
						if (order[2].vari == 1) {
							order[2].c_06 = 0;
							order[2].face = order_face_03[2]; //È­³­ Ç¥Á¤À¸·Î ³»·Á°¡µµ·Ï ¼³Á¤
							order[2].face_count = 21; //¹Ù·Î ÇÏ°­ÇÏµµ·Ï
							order[2].complete = 0; //¿Ï·á Ã³¸® ¾ÈÇÑ°É·Î ¼öÁ¤
							order[2].score_y = 300; //Á¡¼ö ¿ø·¡ ÁÂÇ¥¿¡¼­ ´ë±â
							//order[2].get_dish = false;
							order[2].correct = false;
							life_03--;
							KillTimer(hWnd, 6003);
						}
						else if (order[2].vari == 2) {
							order[2].face = villain_03[0]; //Ã³À½ Ç¥Á¤
							order[2].get_dish = false;
							order[2].correct = false;
							order[2].complete = 0;
							order[2].score_y = 300; //Á¡¼ö ¿ø·¡ ÁÂÇ¥¿¡¼­ ´ë±â
							KillTimer(hWnd, 6003);
						}
					}
				}
			}
			break;
		}

		case 1006: {//¼Õ´ÔÀÇ »ó½Â ¹× ÇÏ°­
			for (int i = 0; i < 3; ++i) {
				if (!order[i].place) {
					order[i].y -= 40;
					if (order[i].y <= 180) {//¼Õ´ÔÀÌ µµÂøÇÏ¸é ÁÖ¹®À» ÁöÁ¤¹Þ´Â´Ù
						if (order[i].y < 180)
							order[i].y = 180;
						order[i].face_count = 0;
						order[i].place = true;
						if (stage_03 == 1) {
							if (order[i].vari == 1) {
								order[i].or_top = rand() % 2 + 1;
								for (int j = 0; j < order[i].or_top; ++j) {
									order[i].stack[j].vari = rand() % 4 + 1;
									switch (order[i].stack[j].vari) {//ÁÖ¹® ºñÆ®¸Ê³¢¸® ºñ±³ °¡´É?//°¡´ÉÇÏ¸é ºñÆ®¸Ê¸¸À¸·Î Ã¼Å© °¡´É(¿¬»ê °¨¼Ò)
									case 1://±èÄ¡
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://ÆÄÀü
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://°¨ÀÚ
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://¸Þ¹Ð
										order[i].stack[j].pancake = pancake_03[10];
										break;
									}
									order[i].stack[j].count = 10;
								}
							}
							else if (order[i].vari == 3) {
								order[i].or_top = 4;
								for (int j = 0; j < order[i].or_top; ++j) {
									order[i].stack[j].vari = rand() % 4 + 1;
									switch (order[i].stack[j].vari) {//ÁÖ¹® ºñÆ®¸Ê³¢¸® ºñ±³ °¡´É?//°¡´ÉÇÏ¸é ºñÆ®¸Ê¸¸À¸·Î Ã¼Å© °¡´É(¿¬»ê °¨¼Ò)
									case 1://±èÄ¡
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://ÆÄÀü
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://°¨ÀÚ
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://¸Þ¹Ð
										order[i].stack[j].pancake = pancake_03[10];
										break;
									}
									order[i].stack[j].count = 10;
								}
							}
						}
						else if (stage_03 == 2) {
							if (order[i].vari == 1) {
								order[i].or_top = rand() % 2 + 2;
								for (int j = 0; j < order[i].or_top; ++j) {
									order[i].stack[j].vari = rand() % 4 + 1;
									switch (order[i].stack[j].vari) {//ÁÖ¹® ºñÆ®¸Ê³¢¸® ºñ±³ °¡´É?//°¡´ÉÇÏ¸é ºñÆ®¸Ê¸¸À¸·Î Ã¼Å© °¡´É(¿¬»ê °¨¼Ò)
									case 1://±èÄ¡
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://ÆÄÀü
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://°¨ÀÚ
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://¸Þ¹Ð
										order[i].stack[j].pancake = pancake_03[10];
										break;
									}
									order[i].stack[j].count = 10;
								}
							}
							else if (order[i].vari == 3) {
								order[i].or_top = 5;
								for (int j = 0; j < order[i].or_top; ++j) {
									order[i].stack[j].vari = rand() % 4 + 1;
									switch (order[i].stack[j].vari) {//ÁÖ¹® ºñÆ®¸Ê³¢¸® ºñ±³ °¡´É?//°¡´ÉÇÏ¸é ºñÆ®¸Ê¸¸À¸·Î Ã¼Å© °¡´É(¿¬»ê °¨¼Ò)
									case 1://±èÄ¡
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://ÆÄÀü
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://°¨ÀÚ
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://¸Þ¹Ð
										order[i].stack[j].pancake = pancake_03[10];
										break;
									}
									order[i].stack[j].count = 10;
								}
							}
						}
						else if (stage_03 == 3) {
							if (order[i].vari == 1) {
								order[i].or_top = rand() % 3 + 2;
								for (int j = 0; j < order[i].or_top; ++j) {
									order[i].stack[j].vari = rand() % 4 + 1;
									switch (order[i].stack[j].vari) {//ÁÖ¹® ºñÆ®¸Ê³¢¸® ºñ±³ °¡´É?//°¡´ÉÇÏ¸é ºñÆ®¸Ê¸¸À¸·Î Ã¼Å© °¡´É(¿¬»ê °¨¼Ò)
									case 1://±èÄ¡
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://ÆÄÀü
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://°¨ÀÚ
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://¸Þ¹Ð
										order[i].stack[j].pancake = pancake_03[10];
										break;
									}
									order[i].stack[j].count = 10;
								}
							}
							else if (order[i].vari == 3) {
								order[i].or_top = 6;
								for (int j = 0; j < order[i].or_top; ++j) {
									order[i].stack[j].vari = rand() % 4 + 1;
									switch (order[i].stack[j].vari) {//ÁÖ¹® ºñÆ®¸Ê³¢¸® ºñ±³ °¡´É?//°¡´ÉÇÏ¸é ºñÆ®¸Ê¸¸À¸·Î Ã¼Å© °¡´É(¿¬»ê °¨¼Ò)
									case 1://±èÄ¡
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://ÆÄÀü
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://°¨ÀÚ
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://¸Þ¹Ð
										order[i].stack[j].pancake = pancake_03[10];
										break;
									}
									order[i].stack[j].count = 10;
								}
							}
						}
					}
				}
				else if (order[i].place && order[i].face_count >= 20) {//¼Õ´ÔÀÌ ¿Ã¶ó¿À°í 20ÃÊ°¡ Áö³µÀ»¶§
					if (order[i].vari == 3) {
						order[i].y++;
						order[i].c_06 = 0;
						order[i].correct = false;
						channel->stop();
						channel3->stop();
						SetTimer(hWnd, 5003, 800, NULL);
						KillTimer(hWnd, 1003);
						KillTimer(hWnd, 2003);
						KillTimer(hWnd, 3003);
						KillTimer(hWnd, 4003);
						KillTimer(hWnd, 6003);

						KillTimer(hWnd, 1006);
						KillTimer(hWnd, 2006);
					}
					else {
						if (order[i].get_dish == false && success_03 < 15 && order[i].youangry == false) {
							order[i].face_count = 0;
							order[i].correct = false;
							order[i].get_dish = true;
							order[i].complete = 1;
							order[i].score = minus_heart_03;
							order[i].y++;
							if (order[i].vari == 1) {
								ssystem->playSound(incorrect, 0, false, &channel1);
								channel1->setVolume(0.5);	//È¿°úÀ½ Ã¤³Î
							}
							if (i == 0) {
								SetTimer(hWnd, 4003, 200, NULL);
							}
							else if (i == 1) {
								SetTimer(hWnd, 5003, 200, NULL);
							}
							else {
								SetTimer(hWnd, 6003, 200, NULL);
							}
						}
						if (order[i].face_count >= 20) {
							order[i].y += 40;
						}
					}
					if (order[i].y >= 500) {
						order[i].place = false;
						order[i].get_dish = false;
						order[i].youangry = false;
						order[i].c_06 = 0;
						order[i].score_y = 300; //Á¡¼ö ¿ø·¡ ÁÂÇ¥¿¡¼­ ´ë±â
						v_check_03 = rand() % 100 + 1;
						if (success_03 >= 5) {//ÀÏÁ¤ È½¼ö ÁÖ¹® ¼º°ø ½Ã À¯¸íÀÎ	½ºÅ×ÀÌÁöº°·Î ´Ù¸£°Ô ¸¸µé¾î¶ó
							if (i == 0) {
								order[i].vari = 4;
								order[i].face = light_03;
							}
							if (i == 1) {
								order[i].vari = 3;
								order[i].face = master_03[0];
							}
							if (i == 2) {
								order[i].vari = 5;
								order[i].face = camera_03;
							}
						}
						else if (v_check_03 <= v_percent_03) {
							order[i].vari = 2;	//¹æÇØ²Û
							v_percent_03 = 0;
						}
						else {
							order[i].vari = 1;	//¼Õ´ÔÀÇ Á¾·ù ÁöÁ¤
						}
						switch (order[i].vari) {
						case 1://¼Õ´Ô
							order[i].face = order_face_03[0];
							break;
						case 2://¹æÇØ²Û
							order[i].face = villain_03[0];
							break;
						}
					}
				}
				if (falling_pot_03.falling && !falling_pot_03.crash) {//³¿ºñÀÇ ÇÏ°­
					falling_pot_03.y += 15;
					for (int i = 0; i < 3; ++i) {
						if (falling_pot_03.x == order[i].x && falling_pot_03.y + 50 >= order[i].y && order[i].place
							&& !order[i].hit_pot) {
							if (rand() % 100 + 1 <= 10) {	//10% È®·ü·Î ¶Ò¹è±â »ç¿îµå
								ssystem->playSound(secret_hit, 0, false, &channel1);
							}
							else {
								ssystem->playSound(hit_sound, 0, false, &channel1);
								channel1->setVolume(0.5);
							}
							falling_pot_03.y += 50;
							falling_pot_03.falling = false;
							falling_pot_03.crash = true;
							falling_pot_03.pot = hit_pot_03;
							order[i].hit_pot = true;
							order[i].c_06 = 0;
							order[i].face_count = 0;
							if (order[i].vari == 1) {
								order[i].face = order_face_03[6];	//¼Õ´Ô ¾Æ´Ñ ´Ù¸¥»ç¶÷ÀÏ¶© ¹Ù²ãÁÖÀÚ¡Ø
							}
							else if (order[i].vari == 2) {
								order[i].face = villain_03[2];
							}
							if (order[i].vari == 1) {
								order[i].complete = 1;
								order[i].score = minus_heart_03;
							}
							else if (order[i].vari == 2) {
								order[i].complete = 1;
								order[i].score = order_check_06[2];
								score_03 += 50;
							}
							SetTimer(hWnd, 3003, 200, NULL);
							if (i == 0) {
								SetTimer(hWnd, 4003, 200, NULL);
							}
							else if (i == 1) {
								if (order[i].vari == 3) {
									channel->stop();
									channel3->stop();
									order[i].y++;
									order[i].c_06 = 0;
									order[i].correct = false;
									isTimer5003 = true;
									KillTimer(hWnd, 1003);
									KillTimer(hWnd, 2003);
									KillTimer(hWnd, 4003);
									KillTimer(hWnd, 6003);

									KillTimer(hWnd, 1006);
									KillTimer(hWnd, 2006);
									SetTimer(hWnd, 5003, 800, NULL);
								}
								else {
									SetTimer(hWnd, 5003, 200, NULL);
								}
							}
							else if (i == 2) {
								SetTimer(hWnd, 6003, 200, NULL);
							}
							break;
						}
						else {
							if (falling_pot_03.y >= 440) {
								falling_pot_03.falling = false;
								falling_pot_03.x = falling_pot_03.y = -1000;
							}
						}
					}
				}
			}
			break;
		}
		case 2006: {//¼Õ´ÔÀÇ ºÐ³ë Ä«¿îÆ® Áõ°¡
			for (int i = 0; i < 3; ++i) {
				if (order[i].place && order[i].y <= 180) {
					if (order[i].vari == 1 && !order[i].get_dish) {
						order[i].face_count++;
						if (order[i].face_count == 10)
							order[i].face = order_face_03[1];
						else if (order[i].face_count == 15)
							order[i].face = order_face_03[2];
					}
					if (order[i].vari == 3 && !order[i].get_dish) {
						order[i].face_count++;
						if (order[i].face_count == 10)
							order[i].face = master_03[1];
						else if (order[i].face_count == 15)
							order[i].face = master_03[2];
					}
				}
			}
			if (stage_03 == 1) {
				v_percent_03 += 0.5;
			}
			else if (stage_03 == 2) {
				v_percent_03 += 1;
			}
			else if (stage_03 == 3) {
				v_percent_03 += 1.5;
			}
			break;
		}
		}
		if (life_03 == 0) {
			stage_03 = 5;
			channel->stop();
			channel3->stop();
			ssystem->playSound(s_bad_end, 0, false, &channel);
			channel->setVolume(0.5);
			KillTimer(hWnd, 1003);
			KillTimer(hWnd, 2003);
			KillTimer(hWnd, 3003);
			KillTimer(hWnd, 4003);
			KillTimer(hWnd, 5003);
			KillTimer(hWnd, 6003);

			KillTimer(hWnd, 1006);
			KillTimer(hWnd, 2006);
		}
		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_CHAR:
		if (stage_03 >= 1 && stage_03<=3) {
			exist_v_03 = false;
			for (int i = 0; i < 3; ++i) {
				if (order[i].vari == 2) {
					exist_v_03 = true;
					break;
				}
			}
			if (!exist_v_03 && !isTimer5003) {
				switch (wParam) {
				case 'Q':
				case 'q':
					channel2->stop();
					ssystem->playSound(s_laddle, 0, false, &channel2);
					channel2->setVolume(0.5);
					P_select_03 = 1;	//±èÄ¡Àü ¼±ÅÃ
					ladle_03.x = 670; ladle_03.y = 425;
					break;
				case 'W':
				case 'w':
					channel2->stop();
					ssystem->playSound(s_laddle, 0, false, &channel2);
					channel2->setVolume(0.5);
					P_select_03 = 2;	//ÆÄÀü ¼±ÅÃ
					ladle_03.x = 670; ladle_03.y = 495;
					break;
				case 'E':
				case 'e':
					channel2->stop();
					ssystem->playSound(s_laddle, 0, false, &channel2);
					channel2->setVolume(0.5);
					P_select_03 = 3;	//°¨ÀÚÀü ¼±ÅÃ
					ladle_03.x = 670; ladle_03.y = 560;
					break;
				case 'R':
				case 'r':
					channel2->stop();
					ssystem->playSound(s_laddle, 0, false, &channel2);
					channel2->setVolume(0.5);
					P_select_03 = 4;	//¸Þ¹ÐÀü ¼±ÅÃ
					ladle_03.x = 670; ladle_03.y = 625;
					break;
				case 'o':	//Á¡¼ö Å×½ºÆ®¿ë
					score_03 += 1000;
					break;
				case 'p':
					score_03 += 100;
					break;
				case 'l':
					score_03 += 10;
					break;
				case ';':
					score_03 += 1;
					break;
				case '+':
					life_03++;
					break;
				case '-':
					life_03--;
					break;
				case '9':
					success_03++;
					break;
				}
			}
		}
		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_LBUTTONDOWN:
		mx_03 = LOWORD(lParam);
		my_03 = HIWORD(lParam);
		count_06 = 0;
		if (stage_03 == 0) {
			stage_03++;
			SetTimer(hWnd, 1003, 200, NULL);//ºÒÆÇ Å¸ÀÌ¸Ó
			SetTimer(hWnd, 2003, 1000, NULL);	//Àü Á¶¸® Ä«¿îÆ®
			SetTimer(hWnd, 1006, 40, NULL);	//¼Õ´Ô ÀÌµ¿
			SetTimer(hWnd, 2006, 1000, NULL);	//¼Õ´ÔÀÇ ´ë±â½Ã°£
			for (int i = 0; i < 3; ++i) {
				order[i].youangry = false;
			}
			channel->stop();
			channel3->stop();
			ssystem->playSound(stage1, 0, false, &channel);
			channel->setVolume(0.5);
			ssystem->playSound(scene_sound, 0, false, &channel3);
			channel3->setVolume(0.35);
		}
		else if (stage_03 == 4 || stage_03 == 5 || stage_03 == 6) {
			stage_03 = 0;
			score_03 = 0;
			life_03 = 3;
			P_top_03 = 0;	//Á¢½Ã ÃÊ±âÈ­
			for (int i = 0; i < 8; ++i) {//ºÒÆÇ ÃÊ±âÈ­
				f_board_03[i].used = false;
			}
			for (int i = 0; i < 3; ++i) {//¼Õ´Ô ÃÊ±â ÁöÁ¤
				order[i].vari = 1;
				order[i].y = (i * 2000) + 500;
				order[i].x = (i * 322) + 38;
				order[i].score_x = (i * 322) + 45;
				order[i].score_y = 300;
				order[i].place = false;
				order[i].hit_pot = false;
				order[i].face = order_face_03[0];
			}
			P_select_03 = 1;	//±èÄ¡Àü ¼±ÅÃ(Å°ÀÔ·ÂÀ¸·Î ¹Ù²Þ)
			ladle_03.x = 670; ladle_03.y = 425;	//ÃÊ±â ±¹ÀÚ À§Ä¡
			grab_pot_03 = false;
			v_percent_03 = 0;
			success_03 = 0;
			isTimer5003 = false;
			{
				falling_pot_03.crash = false;
				falling_pot_03.falling = false;
				falling_pot_03.x = falling_pot_03.y = -1000;
				falling_pot_03.pot = pot_03;
			}
			channel->stop();
			channel3->stop();
			ssystem->playSound(s_title, 0, false, &channel);
			channel->setVolume(0.3);
		}
		else if(!isTimer5003) {
			if (!grab_pot_03) {//³¿ºñ¸¦ ÀâÁö ¾ÊÀº »óÅÂ
				for (int i = 0; i < 3; ++i) {//¿Ã¶ó¿Â ¼Õ´Ô Å¬¸¯
					if (mx_03 >= order[i].x && mx_03 <= order[i].x + 167
						&& my_03 >= order[i].y && my_03 <= order[i].y + 260) {
						if (order[i].place == true && order[i].y == 180) {
							if (P_top_03 > 0) {
								if (order[i].vari == 1 || order[i].vari == 3) {
									if (P_top_03 == order[i].or_top) { //½×Àº ÀüÀÇ Ãþ¿Í ¼Õ´ÔÀÌ ¿øÇÏ´Â ÀüÀÇ Ãþ ºñ±³
										for (int j = 0; j < P_top_03; j++) {
											if (order[i].stack[j].pancake == Stack_03[j].pancake) {
												count_06++; //ÀüÀÇ Á¾·ù°¡ ÃÑ ¸î°³ ¸Â´ÂÁö È®ÀÎ
											}
										}
										if (count_06 == P_top_03) { //ÀüÀÇ Á¾·ù°¡ ÀüºÎ ´Ù °°°í Ãþ ¼ö¿Í ÀÏÄ¡ ÇÒ¶§
											order[i].correct = true;
											order[i].get_dish = true;
											P_top_03 = 0; //Á¢½Ã¿¡ ½×Àº Àü Ã³¸®
											if (order[i].vari == 1) {
												order[i].complete = 1; //¿Ï·áÇß´Ù°í Ã³¸®
											}
											order[i].c_06 = 0;
											if (order[i].vari == 1) {
												ssystem->playSound(correct, 0, false, &channel1);
												channel1->setVolume(0.5);	//È¿°úÀ½ Ã¤³Î
												if (0 < order[i].face_count && order[i].face_count <= 10) {//0~10ÃÊ »çÀÌ¿¡´Â 100Á¡
													order[i].score = order_check_06[0];
													score_03 += 100;
												}
												else if (10 < order[i].face_count && order[i].face_count <= 15) {//10~15ÃÊ »çÀÌ¿¡´Â 80Á¡
													order[i].score = order_check_06[1];
													score_03 += 80;
												}
												else {//15~20ÃÊ »çÀÌ¿¡´Â 50Á¡
													order[i].score = order_check_06[2];
													score_03 += 50;
												}
											}
											if (i == 0) {
												SetTimer(hWnd, 4003, 200, NULL);
											}
											else if (i == 1) {
												if (order[i].vari == 3) {
													channel->stop();
													channel3->stop();
													ssystem->playSound(nurvous, 0, false, &channel);
													channel->setVolume(0.5);
													order[i].y++;
													isTimer5003 = true;
													SetTimer(hWnd, 5003, 800, NULL);
													KillTimer(hWnd, 1003);
													KillTimer(hWnd, 2003);
													KillTimer(hWnd, 3003);
													KillTimer(hWnd, 4003);
													KillTimer(hWnd, 6003);

													KillTimer(hWnd, 1006);
													KillTimer(hWnd, 2006);
												}
												else {
													SetTimer(hWnd, 5003, 200, NULL);
												}
											}
											else {
												SetTimer(hWnd, 6003, 200, NULL);
											}
										}
										else {
											order[i].correct = false;
											order[i].get_dish = true;
											P_top_03 = 0; //Á¢½Ã¿¡ ½×Àº Àü Ã³¸®
											if (order[i].vari == 1) {
												ssystem->playSound(incorrect, 0, false, &channel1);
												channel1->setVolume(0.5);	//È¿°úÀ½ Ã¤³Î
												order[i].complete = 1; //¿Ï·áÇß´Ù°í Ã³¸®
												order[i].score = minus_heart_03;
											}
											order[i].c_06 = 0;
											if (i == 0) {
												SetTimer(hWnd, 4003, 200, NULL);
											}
											else if (i == 1) {
												if (order[i].vari == 3) {
													channel->stop();
													channel3->stop();
													ssystem->playSound(nurvous, 0, false, &channel);
													channel->setVolume(0.5);
													order[i].y++;
													isTimer5003 = true;
													SetTimer(hWnd, 5003, 800, NULL);
													KillTimer(hWnd, 1003);
													KillTimer(hWnd, 2003);
													KillTimer(hWnd, 3003);
													KillTimer(hWnd, 4003);
													KillTimer(hWnd, 6003);

													KillTimer(hWnd, 1006);
													KillTimer(hWnd, 2006);
												}
												else
													SetTimer(hWnd, 5003, 200, NULL);
											}
											else {
												SetTimer(hWnd, 6003, 200, NULL);
											}
										}
									}
									else {
										order[i].correct = false;
										order[i].get_dish = true;
										P_top_03 = 0; //Á¢½Ã¿¡ ½×Àº Àü Ã³¸®
										if (order[i].vari == 1) {
											ssystem->playSound(incorrect, 0, false, &channel1);
											channel1->setVolume(0.5);	//È¿°úÀ½ Ã¤³Î
											order[i].complete = 1; //¿Ï·áÇß´Ù°í Ã³¸®
										}
										order[i].score = minus_heart_03;
										order[i].c_06 = 0;
										if (i == 0) {
											SetTimer(hWnd, 4003, 200, NULL);
										}
										else if (i == 1) {
											if (order[i].vari == 3) {
												channel->stop();
												channel3->stop();
												ssystem->playSound(nurvous, 0, false, &channel);
												channel->setVolume(0.5);
												order[i].y++;
												isTimer5003 = true;
												SetTimer(hWnd, 5003, 800, NULL);
												KillTimer(hWnd, 1003);
												KillTimer(hWnd, 2003);
												KillTimer(hWnd, 3003);
												KillTimer(hWnd, 4003);
												KillTimer(hWnd, 6003);

												KillTimer(hWnd, 1006);
												KillTimer(hWnd, 2006);
											}
											else
												SetTimer(hWnd, 5003, 200, NULL);
										}
										else {
											SetTimer(hWnd, 6003, 200, NULL);
										}
									}
								}
								else if (order[i].vari == 2) {
									order[i].correct = false;
									order[i].get_dish = true;
									order[i].complete = 0; //¿Ï·áÇß´Ù°í Ã³¸®
									P_top_03 = 0; //Á¢½Ã¿¡ ½×Àº Àü Ã³¸®
									order[i].c_06 = 0;
									if (i == 0) {
										SetTimer(hWnd, 4003, 200, NULL);
									}
									else if (i == 1) {
										SetTimer(hWnd, 5003, 200, NULL);
									}
									else {
										SetTimer(hWnd, 6003, 200, NULL);
									}
								}
							}
						}
					}
				}
				for (int i = 0; i < 8; ++i) {
					if (mx_03 > f_board_03[i].range.left && mx_03 < f_board_03[i].range.right
						&& my_03 > f_board_03[i].range.top && my_03 < f_board_03[i].range.bottom) {
						if (f_board_03[i].used) {
							if (P_top_03 < 10) {
								ssystem->playSound(grab_pan, 0, false, &channel1);
								channel1->setVolume(0.5);
								Stack_03[P_top_03] = f_board_03[i];
								Stack_03[P_top_03].x = 760;
								Stack_03[P_top_03].y = 594 - (P_top_03 * 15);
								Stack_03[P_top_03].width = 220;
								Stack_03[P_top_03].height = 110;
								P_top_03++;
								f_board_03[i].used = false;
							}
						}
						else {
							ssystem->playSound(fried, 0, false, &channel1);
							channel1->setVolume(0.5);
							f_board_03[i].used = true;
							f_board_03[i].count = 0;
							f_board_03[i].vari = P_select_03;
							switch (f_board_03[i].vari) {
							case 1://±èÄ¡
								f_board_03[i].pancake = pancake_03[0];
								break;
							case 2://ÆÄÀü
								f_board_03[i].pancake = pancake_03[3];
								break;
							case 3://°¨ÀÚÀü
								f_board_03[i].pancake = pancake_03[6];
								break;
							case 4://¸Þ¹ÐÀü
								f_board_03[i].pancake = pancake_03[9];
								break;
							}
						}
					}
				}
			}
			else if (grab_pot_03) {//³¿ºñ¸¦ Àâ°íÀÖÀ»¶§ Å¬¸¯
				grab_pot_03 = false;
				falling_pot_03.falling = true;
			}
			if (!grab_pot_03 && mx_03 > 740 && mx_03 < rt_03.right && my_03 > 440 && my_03 < 585 && !falling_pot_03.falling) {//³¿ºñ¸¦ Å¬¸¯
				grab_pot_03 = true;
				falling_pot_03.x = order[2].x;
				falling_pot_03.y = 50;
				falling_pot_03.pot = pot_03;
			}
		}
		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_MOUSEMOVE:
		if (grab_pot_03) {
			mx_03 = LOWORD(lParam);
			my_03 = HIWORD(lParam);
			for (int i = 0; i < 3; ++i) {
				if (mx_03 > order[i].x && mx_03 < order[i].x + 167) {
					falling_pot_03.x = order[i].x;
					falling_pot_03.y = 50;
					break;
				}
			}
		}
		break;
	case WM_RBUTTONDOWN:
		if (grab_pot_03) {
			falling_pot_03.x = falling_pot_03.y = -1000;
			grab_pot_03 = false;
		}
		else {
			P_top_03 = 0;
		}
		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_DESTROY:
		ssystem->close();
		ssystem->release();
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hWnd, Message, wParam, lParam));
}