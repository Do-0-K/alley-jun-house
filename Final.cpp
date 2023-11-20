#pragma comment (lib, "msimg32.lib")
#include<windows.h>
#include<ctime>
#include "fmod.hpp"
#include "fmod_errors.h"
#include "resource.h"
FMOD::System* ssystem;
FMOD::Sound* s_bad_end, * correct, * incorrect, * s_laddle, * hit_sound, * stage1, * stage3, * scene_sound, * nurvous
, * fried, * grab_pan, * secret_hit, * stage2, * s_title, * ending;
FMOD::Channel* channel = 0;	//배경음악
FMOD::Channel* channel1 = 0;	//효과음
FMOD::Channel* channel2 = 0;	//국자 소리
FMOD::Channel* channel3 = 0;	//환경음
FMOD_RESULT result;
void* extradriverdata = 0;

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"골목 전집";

LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

struct LADLE {	//국자 구조체
	int x, y;	//국자의 위치
	HBITMAP ladle;	//국자 비트맵
};
LADLE ladle_03;

struct FIREBOARD {
	bool used;	//시용중인가?
	int vari;	//전의 종류 = P_select
	HBITMAP pancake; //전 비트맵
	int count;	//얼마동안 조리했나
	RECT range;	//어디를 클릭해야 선택되나
	int x, y;	//전의 위치
	int width, height;	//전 크기
};

struct FALLINGPOT {
	int x, y;	//냄비 위치
	bool falling;	//떨어지고 있나?
	bool crash;		//충돌했나?
	HBITMAP pot;
};
FIREBOARD f_board_03[8];
FIREBOARD Stack_03[10];
FALLINGPOT falling_pot_03;

struct ORDER {
	int vari;	//손님의 종류1. 일반인, 2. 방해꾼, 3. 유명인
	bool place; //자리에 정착했나?(위에 올라왔나)
	int x, y;	//손님의 위치
	HBITMAP face;	//손님 비트맵
	double face_count;	//애니메이션 카운트
	int en_count;	//버틴 시간	15초 후 변환
	int or_top;		//몇층까지 주문했나?
	FIREBOARD stack[10];	//뭘 주문했나?
	int complete = 0; //배달 완료했나?
	int complete_face = 0; //배달 완료했을 때 표정
	HBITMAP score; //점수 비트맵
	int score_x = 0; //점수 위치
	int score_y = 0; //점수 위치
	int c_06 = 0;
	bool hit_pot;	//냄비에 맞았나?
	bool get_dish; //음식을 받았나?
	bool correct; //맞았나?
	bool youangry; //전에 화났나?
};
ORDER order[3];	//0왼쪽 손님, 1, 중앙손님, 2. 오른쪽 손님


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
	static HDC hDC_03, mDC1_03, mDC2_03;	//DC들
	static HBITMAP hBit_03, background_03[7], oldBit_03[2];	//hBit, 배경, oldBit
	static HBITMAP fire_03[4], c_board_03, dish_03, pot_03;	//불판, 도마, 접시, 냄비
	static HBITMAP score_board_03, score_text_03, score_num_03[10];	//점수판, 점수 텍스트, 0~9
	static HBITMAP life_board_03, heart_03[2];	//체력 판, 하트
	static HBITMAP talkBox_03, pancake_03[12], hit_pot_03, minus_heart_03;		//말풍선, 전의 종류, 부딪힌 냄비, 하트 감소
	static HBITMAP order_face_03[9];	//손님 얼굴
	static HBITMAP order_check_06[3]; //손님 응대에 따른 점수
	static HBITMAP villain_03[5];	//빌런의 얼굴 0. 보통 1~2. 고통 3~4. 웃음
	static HBITMAP master_03[8];	//유명인 대기부터 떠나가기까지 0~2 대기 3~7떠나감
	static HBITMAP camera_03, light_03;	//카메라와 조명

	static int stage_03, fire_count_03;	//몇번째 스테이지, 불판 애니메이션 카운트
	static int P_select_03, P_top_03, score_03;	//무슨 전을 골랐나, 접시에 전을 얼마나 쌓았나? 점수
	static int mx_03, my_03, cal_score_03, sub_i_03;	//마우스 좌표, 점수 계산용 변수, 점수계산용
	static int life_03;	//남은 체력
	static bool grab_pot_03;	//냄비를 잡았는가?
	static int count_06 = 0; //손님 주문과 내가 만든 요리가 서로 맞는지 확인용
	static bool exist_v_03, isTimer5003;	//방해꾼이 존재하나?, 5003타이머가 작동중인가
	static double v_percent_03, v_check_03;	//방해꾼 등장 확률, 방해꾼 확률 체크
	static int success_03;	//주문 성공 횟수

	switch (Message) {
	case WM_CREATE:
		stage_03 = 0;	//시작은 1스테이지
		P_top_03 = 0;
		{
			heart_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP36));	//정상하트
			heart_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP35));	//부서진 하트
		}
		{//배경 비트맵 지정
			background_03[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP10));	//타이틀
			background_03[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));	//시장배경
			background_03[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP70));	//시장배경
			background_03[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP71));	//시장배경
			background_03[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP72));	//해피 엔딩
			background_03[5] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP51));	//배드 엔딩
			background_03[6] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP69));	//배드 엔딩2
		}
		{//점수 텍스트
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
		}//불판 비트맵
		{
			c_board_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));	//도마
			dish_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP7));	//접시
			ladle_03.ladle = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP8));	//국자
			pot_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP9));	//냄비
			score_board_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP23));	//점수판
			score_text_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP24));	//점수 텍스트
			life_board_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP37));	//하트 판
			talkBox_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP39));	//말풍선
			hit_pot_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP47));	//부서진 냄비
			minus_heart_03 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP50));	//하트 감소
		}
		{//전의 종류 0~2 김치 3~5 파전 6~8 감자 9~11 메밀 
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
		{//손님 얼굴 0평시 1화남 2분노 3 먹는중1, 4 먹는중 2, 5고통1, 6. 고통2
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
		for (int i = 0; i < 8; ++i) {//불판 구역 지정
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
		for (int i = 0; i < 3; ++i) {//손님 초기 지정
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
		P_select_03 = 1;	//김치전 선택(키입력으로 바꿈)
		ladle_03.x = 670; ladle_03.y = 425;	//초기 국자 위치
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
			FMOD_LOOP_NORMAL, 0, &s_bad_end);	//배드엔딩 사운드
		ssystem->createSound("sound\\correct.mp3",
			FMOD_DEFAULT, 0, &correct);		//맞췃을때 사운드
		ssystem->createSound("sound\\incorrect.mp3",
			FMOD_DEFAULT, 0, &incorrect);	//틀렸을때 사운드
		ssystem->createSound("sound\\laddle.wav",
			FMOD_DEFAULT, 0, &s_laddle);	//국자 사운드
		ssystem->createSound("sound\\pot_hit.mp3",
			FMOD_DEFAULT, 0, &hit_sound);	//냄비 사운드
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
		channel->stop();	//배경음 채널
		channel1->stop();	//효과음 채널
		channel2->stop();	//국자 이동소리 채널
		channel3->stop();
		ssystem->playSound(s_title, 0, false, &channel);
		channel->setVolume(0.3);	//배경음 채널
		break;
	case WM_PAINT:
		GetClientRect(hWnd, &rt_03);
		hDC_03 = BeginPaint(hWnd, &ps_03);
		hBit_03 = CreateCompatibleBitmap(hDC_03, rt_03.right, rt_03.bottom);
		mDC1_03 = CreateCompatibleDC(hDC_03);
		mDC2_03 = CreateCompatibleDC(mDC1_03);
		oldBit_03[0] = (HBITMAP)SelectObject(mDC1_03, hBit_03);
		//스테이지에 따른 배경 설정 0 시작전, 1~3 게임중, 4~엔딩, 5배드엔딩, 6. 배드2
		{
			if (stage_03 == 0) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[0]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1024, 768, SRCCOPY);//배경 넣기
			}
			else if (stage_03 == 1) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[1]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1280, 853, SRCCOPY);//배경 넣기
			}
			else if (stage_03 == 2) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[2]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1280, 853, SRCCOPY);//배경 넣기
			}
			else if (stage_03 == 3) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[3]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1280, 853, SRCCOPY);//배경 넣기
			}
			else if (stage_03 == 4) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[4]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1024, 768, SRCCOPY);//배경 넣기

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_text_03);	//
				TransparentBlt(mDC1_03, 373, 580, 90, 30, mDC2_03, 0, 0, 90, 30, RGB(34, 177, 76)); //점수 텍스트 넣기
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
					TransparentBlt(mDC1_03, (sub_i_03 * 30) + 488, 580, 30, 30, mDC2_03, 0, 0, 30, 30, RGB(34, 177, 76)); //점수 텍스트 넣기
					cal_score_03 -= (cal_score_03 / i) * i;
					sub_i_03++;
				}
			}
			else if (stage_03 == 5) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[5]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1024, 768, SRCCOPY);//배경 넣기

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_text_03);	//
				TransparentBlt(mDC1_03, 360, 400, 90, 30, mDC2_03, 0, 0, 90, 30, RGB(34, 177, 76)); //점수 텍스트 넣기
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
					TransparentBlt(mDC1_03, (sub_i_03 * 30) + 475, 400, 30, 30, mDC2_03, 0, 0, 30, 30, RGB(34, 177, 76)); //점수 텍스트 넣기
					cal_score_03 -= (cal_score_03 / i) * i;
					sub_i_03++;
				}
			}
			else if (stage_03 == 6) {
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, background_03[6]);
				StretchBlt(mDC1_03, 0, 0, rt_03.right, rt_03.bottom, mDC2_03, 0, 0, 1024, 768, SRCCOPY);//배경 넣기

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_text_03);	//
				TransparentBlt(mDC1_03, 360, 50, 90, 30, mDC2_03, 0, 0, 90, 30, RGB(34, 177, 76)); //점수 텍스트 넣기
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
					TransparentBlt(mDC1_03, (sub_i_03 * 30) + 475, 50, 30, 30, mDC2_03, 0, 0, 30, 30, RGB(34, 177, 76)); //점수 텍스트 넣기
					cal_score_03 -= (cal_score_03 / i) * i;
					sub_i_03++;
				}
			}

			if (stage_03 >= 1 && stage_03 <=3) {
				{//손님 출력
					for (int i = 0; i < 3; ++i) {
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, order[i].face);
						TransparentBlt(mDC1_03, order[i].x, order[i].y, 167, 260, mDC2_03, 0, 0, 167, 260, RGB(34, 177, 76));//손님 출력
						if ((order[i].vari == 1 || order[i].vari == 3) && order[i].y == 180) {
							if (order[i].place && order[i].y == 180 && !order[i].hit_pot) {
								oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, talkBox_03);
								TransparentBlt(mDC1_03, (i * 320) + 205, 130, 155, 150,
									mDC2_03, 0, 0, 155, 150, RGB(34, 177, 76));//말풍선 출력

								oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, dish_03);
								TransparentBlt(mDC1_03, (i * 320) + 220, 210, 127, 50,
									mDC2_03, 0, 0, 127, 54, RGB(34, 177, 76));//접시 출력
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
				TransparentBlt(mDC1_03, falling_pot_03.x, falling_pot_03.y, 167, 50, mDC2_03, 0, 0, 241, 114,RGB(34, 177, 76));	//떨어지는 냄비 넣기
				
				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, fire_03[fire_count_03]);
				StretchBlt(mDC1_03, 0, 440, 740, 289, mDC2_03, 0, 0, 740, 289, SRCCOPY);	//불판 넣기

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, c_board_03);
				StretchBlt(mDC1_03, 740, 440, 268, 289, mDC2_03, 0, 0, 268, 289, SRCCOPY);	//도마 넣기

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, pot_03);
				TransparentBlt(mDC1_03, 740, 440, 268, 145, mDC2_03, 0, 0, 241, 114, RGB(34, 177, 76)); //냄비 넣기

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, dish_03);
				TransparentBlt(mDC1_03, 740, 584, 268, 145, mDC2_03, 0, 0, 127, 54, RGB(34, 177, 76)); //접비 넣기

				{//점수 표시
					oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_board_03);
					TransparentBlt(mDC1_03, 0, 0, 300, 60, mDC2_03, 0, 0, 300, 60, RGB(34, 177, 76)); //점수판 넣기

					oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, score_text_03);
					TransparentBlt(mDC1_03, 10, 15, 90, 30, mDC2_03, 0, 0, 90, 30, RGB(34, 177, 76)); //점수 텍스트 넣기
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
						TransparentBlt(mDC1_03, (sub_i_03 * 30) + 125, 15, 30, 30, mDC2_03, 0, 0, 30, 30, RGB(34, 177, 76)); //점수 텍스트 넣기
						cal_score_03 -= (cal_score_03 / i) * i;
						sub_i_03++;
					}
				}
				{//하트 표기
					oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, life_board_03);
					TransparentBlt(mDC1_03, 838, 0, 170, 60, mDC2_03, 0, 0, 170, 60, RGB(34, 177, 76)); //체력 판 넣기
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
					TransparentBlt(mDC1_03, Stack_03[i].x, Stack_03[i].y, Stack_03[i].width, Stack_03[i].height, mDC2_03, 0, 0, 380, 209, RGB(34, 177, 76)); //접비 넣기
				}

				oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, ladle_03.ladle);
				TransparentBlt(mDC1_03, ladle_03.x, ladle_03.y, 70, 100, mDC2_03, 0, 0, 104, 222, RGB(34, 177, 76));	//국자 넣기

				for (int i = 0; i < 8; ++i) {
					if (f_board_03[i].used) {
						oldBit_03[1] = (HBITMAP)SelectObject(mDC2_03, f_board_03[i].pancake);
						TransparentBlt(mDC1_03, f_board_03[i].x, f_board_03[i].y, f_board_03[i].width, f_board_03[i].height,
							mDC2_03, 0, 0, 380, 209, RGB(34, 177, 76));	//불판에 전 넣기
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
		case 1003: {//불판 애니메이션
			fire_count_03++;
			fire_count_03 %= 4;
			break;
		}
		case 2003: {//불판 별 전의 카운트 증가
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
		case 3003: {//냄비히트
			if (falling_pot_03.crash) {
				falling_pot_03.crash = false;
				falling_pot_03.x = falling_pot_03.y = -1000;
			}
			KillTimer(hWnd, 3003);
			break;
		}
		case 4003: {//1번 손님 개별 카운트
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
			else if (order[0].get_dish) { //주문한 메뉴를 받았을 때
				if (order[0].correct == true) { //맞았을 때
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
					order[0].score_y -= 10; //얼굴에 따른 점수표 상승
					if (order[0].c_06 == 4) { //4번 아그작아그작하고 종료
						if (order[0].vari == 1) {
							order[0].c_06 = 0;
							order[0].face = order_face_03[0]; //처음 표정으로 내려가도록 설정
							order[0].face_count = 21; //바로 하강하도록
							order[0].complete = 0; //완료 처리 안한걸로 수정
							order[0].score_y = 300; //점수 원래 좌표에서 대기
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
							order[0].face = villain_03[0]; //처음 표정
							order[0].get_dish = false;
							order[0].correct = false;
							order[0].complete = 0;
							order[0].score_y = 300; //점수 원래 좌표에서 대기
							KillTimer(hWnd, 4003);
						}
					}
					order[0].c_06++;
				}
				else { //잘못 줬을 때
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
					order[0].score_y -= 10; //얼굴에 따른 점수표 상승
					if (order[0].c_06 >= 4) {
						if (order[0].vari == 1) {
							order[0].c_06 = 0;
							order[0].face = order_face_03[2]; //화난 표정으로 내려가도록 설정
							order[0].face_count = 21; //바로 하강하도록
							order[0].complete = 0; //완료 처리 안한걸로 수정
							//order[0].get_dish = false;
							order[0].correct = false;
							order[0].score_y = 300; //점수 원래 좌표에서 대기
							if (order[0].vari == 1) {
								life_03--;
							}
							KillTimer(hWnd, 4003);
						}
						else if (order[0].vari == 2) {
							order[0].face = villain_03[0]; //처음 표정
							order[0].get_dish = false;
							order[0].correct = false;
							order[0].score_y = 300; //점수 원래 좌표에서 대기
							order[0].complete = 0;
							KillTimer(hWnd, 4003);
						}
					}
				}
			}
			break;
		}
		case 5003: {//2번 손님 개별 카운트
			if (order[1].vari == 3) {	//유명인일때
				order[1].face = master_03[order[1].c_06 + 3];
				order[1].c_06++;
				if (order[1].c_06 >= 6) {
					if (order[1].correct) {//맞으면 스테이지 up
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
						P_top_03 = 0;	//접시 초기화
						for (int i = 0; i < 8; ++i) {//불판 초기화
							f_board_03[i].used = false;
						}
						for (int i = 0; i < 3; ++i) {//손님 초기 지정
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
						P_select_03 = 1;	//김치전 선택(키입력으로 바꿈)
						ladle_03.x = 670; ladle_03.y = 425;	//초기 국자 위치
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
							SetTimer(hWnd, 1003, 200, NULL);//불판 타이머
							SetTimer(hWnd, 2003, 1000, NULL);	//전 조리 카운트
							SetTimer(hWnd, 1006, 40, NULL);	//손님 이동
							SetTimer(hWnd, 2006, 1000, NULL);	//손님의 대기시간
						}
					}
					else {	//틀리면 엔딩
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
			else if (order[1].get_dish) { //주문한 메뉴를 받았을 때
				if (order[1].correct == true) { //맞았을 때
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
					order[1].score_y -= 10; //얼굴에 따른 점수표 상승
					if (order[1].c_06 == 4) { //4번 아그작아그작하고 종료
						if (order[1].vari == 1) {
							order[1].c_06 = 0;
							order[1].face = order_face_03[0]; //처음 표정으로 내려가도록 설정
							order[1].face_count = 21; //바로 하강하도록
							order[1].complete = 0; //완료 처리 안한걸로 수정
							order[1].score_y = 300; //점수 원래 좌표에서 대기
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
							order[1].face = villain_03[0]; //처음 표정
							order[1].get_dish = false;
							order[1].correct = false;
							order[1].complete = 0;
							order[1].score_y = 300; //점수 원래 좌표에서 대기
							KillTimer(hWnd, 5003);
						}
					}
					order[1].c_06++;
				}
				else { //잘못 줬을 때
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
					order[1].score_y -= 10; //얼굴에 따른 점수표 상승
					if (order[1].c_06 >= 4) {
						if (order[1].vari == 1) {
							order[1].c_06 = 0;
							order[1].face = order_face_03[2]; //화난 표정으로 내려가도록 설정
							order[1].face_count = 21; //바로 하강하도록
							order[1].complete = 0; //완료 처리 안한걸로 수정
							//order[1].get_dish = false;
							order[1].correct = false;
							order[1].score_y = 300; //점수 원래 좌표에서 대기
							life_03--;
							KillTimer(hWnd, 5003);
						}
						else if (order[1].vari == 2) {
							order[1].face = villain_03[0]; //처음 표정
							order[1].get_dish = false;
							order[1].correct = false;
							order[1].complete = 0;
							order[1].score_y = 300; //점수 원래 좌표에서 대기
							KillTimer(hWnd, 5003);
						}
					}
				}
			}
			break;
		}
		case 6003: {//3번 손님 개별 카운트
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
			else if (order[2].get_dish) { //주문한 메뉴를 받았을 때
				if (order[2].correct == true) { //맞았을 때
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
					order[2].score_y -= 10; //얼굴에 따른 점수표 상승
					if (order[2].c_06 == 4) { //4번 아그작아그작하고 종료
						if (order[2].vari == 1) {
							order[2].c_06 = 0;
							order[2].face = order_face_03[0]; //처음 표정으로 내려가도록 설정
							order[2].face_count = 21; //바로 하강하도록
							order[2].complete = 0; //완료 처리 안한걸로 수정
							//order[2].get_dish = false;
							order[2].correct = false;
							order[2].score_y = 300; //점수 원래 좌표에서 대기
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
							order[2].face = villain_03[0]; //처음 표정
							order[2].get_dish = false;
							order[2].correct = false;
							order[2].complete = 0;
							order[2].score_y = 300; //점수 원래 좌표에서 대기
							KillTimer(hWnd, 6003);
						}
					}
					order[2].c_06++;
				}
				else { //잘못 줬을 때
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
					order[2].score_y -= 10; //얼굴에 따른 점수표 상승
					if (order[2].c_06 >= 4) {
						if (order[2].vari == 1) {
							order[2].c_06 = 0;
							order[2].face = order_face_03[2]; //화난 표정으로 내려가도록 설정
							order[2].face_count = 21; //바로 하강하도록
							order[2].complete = 0; //완료 처리 안한걸로 수정
							order[2].score_y = 300; //점수 원래 좌표에서 대기
							//order[2].get_dish = false;
							order[2].correct = false;
							life_03--;
							KillTimer(hWnd, 6003);
						}
						else if (order[2].vari == 2) {
							order[2].face = villain_03[0]; //처음 표정
							order[2].get_dish = false;
							order[2].correct = false;
							order[2].complete = 0;
							order[2].score_y = 300; //점수 원래 좌표에서 대기
							KillTimer(hWnd, 6003);
						}
					}
				}
			}
			break;
		}

		case 1006: {//손님의 상승 및 하강
			for (int i = 0; i < 3; ++i) {
				if (!order[i].place) {
					order[i].y -= 40;
					if (order[i].y <= 180) {//손님이 도착하면 주문을 지정받는다
						if (order[i].y < 180)
							order[i].y = 180;
						order[i].face_count = 0;
						order[i].place = true;
						if (stage_03 == 1) {
							if (order[i].vari == 1) {
								order[i].or_top = rand() % 2 + 1;
								for (int j = 0; j < order[i].or_top; ++j) {
									order[i].stack[j].vari = rand() % 4 + 1;
									switch (order[i].stack[j].vari) {//주문 비트맵끼리 비교 가능?//가능하면 비트맵만으로 체크 가능(연산 감소)
									case 1://김치
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://파전
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://감자
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://메밀
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
									switch (order[i].stack[j].vari) {//주문 비트맵끼리 비교 가능?//가능하면 비트맵만으로 체크 가능(연산 감소)
									case 1://김치
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://파전
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://감자
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://메밀
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
									switch (order[i].stack[j].vari) {//주문 비트맵끼리 비교 가능?//가능하면 비트맵만으로 체크 가능(연산 감소)
									case 1://김치
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://파전
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://감자
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://메밀
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
									switch (order[i].stack[j].vari) {//주문 비트맵끼리 비교 가능?//가능하면 비트맵만으로 체크 가능(연산 감소)
									case 1://김치
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://파전
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://감자
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://메밀
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
									switch (order[i].stack[j].vari) {//주문 비트맵끼리 비교 가능?//가능하면 비트맵만으로 체크 가능(연산 감소)
									case 1://김치
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://파전
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://감자
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://메밀
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
									switch (order[i].stack[j].vari) {//주문 비트맵끼리 비교 가능?//가능하면 비트맵만으로 체크 가능(연산 감소)
									case 1://김치
										order[i].stack[j].pancake = pancake_03[1];
										break;
									case 2://파전
										order[i].stack[j].pancake = pancake_03[4];
										break;
									case 3://감자
										order[i].stack[j].pancake = pancake_03[7];
										break;
									case 4://메밀
										order[i].stack[j].pancake = pancake_03[10];
										break;
									}
									order[i].stack[j].count = 10;
								}
							}
						}
					}
				}
				else if (order[i].place && order[i].face_count >= 20) {//손님이 올라오고 20초가 지났을때
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
								channel1->setVolume(0.5);	//효과음 채널
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
						order[i].score_y = 300; //점수 원래 좌표에서 대기
						v_check_03 = rand() % 100 + 1;
						if (success_03 >= 5) {//일정 횟수 주문 성공 시 유명인	스테이지별로 다르게 만들어라
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
							order[i].vari = 2;	//방해꾼
							v_percent_03 = 0;
						}
						else {
							order[i].vari = 1;	//손님의 종류 지정
						}
						switch (order[i].vari) {
						case 1://손님
							order[i].face = order_face_03[0];
							break;
						case 2://방해꾼
							order[i].face = villain_03[0];
							break;
						}
					}
				}
				if (falling_pot_03.falling && !falling_pot_03.crash) {//냄비의 하강
					falling_pot_03.y += 15;
					for (int i = 0; i < 3; ++i) {
						if (falling_pot_03.x == order[i].x && falling_pot_03.y + 50 >= order[i].y && order[i].place
							&& !order[i].hit_pot) {
							if (rand() % 100 + 1 <= 10) {	//10% 확률로 뚝배기 사운드
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
								order[i].face = order_face_03[6];	//손님 아닌 다른사람일땐 바꿔주자※
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
		case 2006: {//손님의 분노 카운트 증가
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
					P_select_03 = 1;	//김치전 선택
					ladle_03.x = 670; ladle_03.y = 425;
					break;
				case 'W':
				case 'w':
					channel2->stop();
					ssystem->playSound(s_laddle, 0, false, &channel2);
					channel2->setVolume(0.5);
					P_select_03 = 2;	//파전 선택
					ladle_03.x = 670; ladle_03.y = 495;
					break;
				case 'E':
				case 'e':
					channel2->stop();
					ssystem->playSound(s_laddle, 0, false, &channel2);
					channel2->setVolume(0.5);
					P_select_03 = 3;	//감자전 선택
					ladle_03.x = 670; ladle_03.y = 560;
					break;
				case 'R':
				case 'r':
					channel2->stop();
					ssystem->playSound(s_laddle, 0, false, &channel2);
					channel2->setVolume(0.5);
					P_select_03 = 4;	//메밀전 선택
					ladle_03.x = 670; ladle_03.y = 625;
					break;
				case 'o':	//점수 테스트용
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
			SetTimer(hWnd, 1003, 200, NULL);//불판 타이머
			SetTimer(hWnd, 2003, 1000, NULL);	//전 조리 카운트
			SetTimer(hWnd, 1006, 40, NULL);	//손님 이동
			SetTimer(hWnd, 2006, 1000, NULL);	//손님의 대기시간
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
			P_top_03 = 0;	//접시 초기화
			for (int i = 0; i < 8; ++i) {//불판 초기화
				f_board_03[i].used = false;
			}
			for (int i = 0; i < 3; ++i) {//손님 초기 지정
				order[i].vari = 1;
				order[i].y = (i * 2000) + 500;
				order[i].x = (i * 322) + 38;
				order[i].score_x = (i * 322) + 45;
				order[i].score_y = 300;
				order[i].place = false;
				order[i].hit_pot = false;
				order[i].face = order_face_03[0];
			}
			P_select_03 = 1;	//김치전 선택(키입력으로 바꿈)
			ladle_03.x = 670; ladle_03.y = 425;	//초기 국자 위치
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
			if (!grab_pot_03) {//냄비를 잡지 않은 상태
				for (int i = 0; i < 3; ++i) {//올라온 손님 클릭
					if (mx_03 >= order[i].x && mx_03 <= order[i].x + 167
						&& my_03 >= order[i].y && my_03 <= order[i].y + 260) {
						if (order[i].place == true && order[i].y == 180) {
							if (P_top_03 > 0) {
								if (order[i].vari == 1 || order[i].vari == 3) {
									if (P_top_03 == order[i].or_top) { //쌓은 전의 층와 손님이 원하는 전의 층 비교
										for (int j = 0; j < P_top_03; j++) {
											if (order[i].stack[j].pancake == Stack_03[j].pancake) {
												count_06++; //전의 종류가 총 몇개 맞는지 확인
											}
										}
										if (count_06 == P_top_03) { //전의 종류가 전부 다 같고 층 수와 일치 할때
											order[i].correct = true;
											order[i].get_dish = true;
											P_top_03 = 0; //접시에 쌓은 전 처리
											if (order[i].vari == 1) {
												order[i].complete = 1; //완료했다고 처리
											}
											order[i].c_06 = 0;
											if (order[i].vari == 1) {
												ssystem->playSound(correct, 0, false, &channel1);
												channel1->setVolume(0.5);	//효과음 채널
												if (0 < order[i].face_count && order[i].face_count <= 10) {//0~10초 사이에는 100점
													order[i].score = order_check_06[0];
													score_03 += 100;
												}
												else if (10 < order[i].face_count && order[i].face_count <= 15) {//10~15초 사이에는 80점
													order[i].score = order_check_06[1];
													score_03 += 80;
												}
												else {//15~20초 사이에는 50점
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
											P_top_03 = 0; //접시에 쌓은 전 처리
											if (order[i].vari == 1) {
												ssystem->playSound(incorrect, 0, false, &channel1);
												channel1->setVolume(0.5);	//효과음 채널
												order[i].complete = 1; //완료했다고 처리
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
										P_top_03 = 0; //접시에 쌓은 전 처리
										if (order[i].vari == 1) {
											ssystem->playSound(incorrect, 0, false, &channel1);
											channel1->setVolume(0.5);	//효과음 채널
											order[i].complete = 1; //완료했다고 처리
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
									order[i].complete = 0; //완료했다고 처리
									P_top_03 = 0; //접시에 쌓은 전 처리
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
							case 1://김치
								f_board_03[i].pancake = pancake_03[0];
								break;
							case 2://파전
								f_board_03[i].pancake = pancake_03[3];
								break;
							case 3://감자전
								f_board_03[i].pancake = pancake_03[6];
								break;
							case 4://메밀전
								f_board_03[i].pancake = pancake_03[9];
								break;
							}
						}
					}
				}
			}
			else if (grab_pot_03) {//냄비를 잡고있을때 클릭
				grab_pot_03 = false;
				falling_pot_03.falling = true;
			}
			if (!grab_pot_03 && mx_03 > 740 && mx_03 < rt_03.right && my_03 > 440 && my_03 < 585 && !falling_pot_03.falling) {//냄비를 클릭
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