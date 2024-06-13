// WinAPIProject.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "WinAPIProject.h"
#include <list> 
#include <math.h>


using namespace std;

#define MAX_LOADSTRING 100
#define PI  3.141592;

typedef struct _tagRectangle {
    float l, t, r, b;
}RECTANGLE, *PRECTANGLE;

typedef struct _tagSphere{
    float x, y;
    float r;
}SPHERE, *PSPHERE;

typedef struct _tagMonster
{
    SPHERE tSphere;
    float fSpeed;
    float fTime;
    float fLimitTime;
    int iDir;
}MONSTER, * PMONSTER;





// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
HWND g_hWnd;
HDC g_hDC;
bool g_bLoop = true;
SPHERE   g_tPlayer = { 50.f, 50.f, 50.f };
MONSTER     g_tMonster;
POINT       g_tGunPos;
float       g_fGunLength = 70.f;
float       g_fPlayerAngle;
RECT        rc = { 0, 0, 800, 600 };

typedef struct _tagBullet
{
    SPHERE tSphere;
    float fDist;
    float fLimitDist;
    float fAngle;
}BULLET, *PBULLET;

// 몬스터 위치
list<RECTANGLE> g_MonsterPosition;


// 플레이어 총알
list<BULLET> g_PlayerBulletList;

// 몬스터 총알
list<BULLET> g_MonsterBulletList;

// 시간을 구하기 위한 변수들
LARGE_INTEGER g_tSecond;
LARGE_INTEGER g_tTime;
float g_fDeltaTime;


// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void Run();
float GetAngle(POINT r1, POINT r2);

struct _tagArea
{
    bool bStart;
    POINT ptStart;
    POINT ptEnd;
};

_tagArea g_tArea;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINAPIPROJECT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    // 화면용 DC 생성
    g_hDC = GetDC(g_hWnd);

    // 몬스터 초기화
    g_tMonster.tSphere.x = 800.f-50.f;
    g_tMonster.tSphere.y = 50.f;
    g_tMonster.tSphere.r = 50.f;
    g_tMonster.fSpeed = 500.f;
    g_tMonster.fTime = 0.f;
    g_tMonster.fLimitTime = 1.1f;
    g_tMonster.iDir = 1;
    
    // 플레이어 총구의 위치를 구해준다.
    g_tGunPos.x = g_tPlayer.x + cosf(g_fPlayerAngle) * g_fGunLength;
    g_tGunPos.y = g_tPlayer.y + sinf(g_fPlayerAngle) * g_fGunLength;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPIPROJECT));

    MSG msg;

    QueryPerformanceFrequency(&g_tSecond);
    QueryPerformanceCounter(&g_tTime);

    // 기본 메시지 루프입니다:
    while (g_bLoop)
    {
        // PeekMessage는 메세지가 메세지큐에 없어도
        // 바로 빠져나온다.
        // 메세지가 있을 경우 true, 없을 경우 false가 된다.
        // 메세지가 없는 시간이 윈도우의 데드타임이다.
        // PM_REMOVE 는 메세지가 있을 경우 가지고오면서 그 메세지를 지워버림
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // 윈도우 데드타임일 경우 (이벤트가 발생하지 않는 시간, 이 부분에 게임을 구현함)
        else {
           
            Run();
        }
    }

    ReleaseDC(g_hWnd, g_hDC);

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINAPIPROJECT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = NULL; //MAKEINTRESOURCEW(IDC_WINAPIPROJECT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hWnd = hWnd;

   // 실제 윈도우 타이틀바나 메뉴를 포함한 윈도우의 크기를 구해준다.
   
   AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
   
   // 위에서 구해준 크기로 윈도우 클라이언트 영역의 크기를 원하는 크기로 맞춰줘야 한다.
   // 윈도우 창의 위치, 크기를 정함
   // HWND_TOPMOST 윈도우를 최상단에 띄운다(다른 윈도우창이 있으면 그 위를 덮으면서 띄워짐)
   // ZORDER 창을 여러개 띄워놨을때 ZORDER라는 값을 이용해서 어떤 창이 우선순위가 있는지를 정함
   // NOZORDER => 그걸 사용하지 않음
   SetWindowPos(hWnd, HWND_TOPMOST, 100, 100, rc.right - rc.left, 
       rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            

            /*
            // 텍스트 출력
            // 유니코드 문자열은 "" 앞에 L을 붙여서 L"" 로 해줄 수 있다.
            // 아니면 TEXT 매크로를 이용한다. TEXT("text")
            TextOut(hdc, 0, 0, TEXT("text"), 5);
            TextOut(hdc, 0, 50, L"Win32", 5);
            
            // 사각형 그리기 : Left, Top, Right, Bottom 점을 잡아서 사각형을 그려준다.
            Rectangle(hdc, 100, 100, 200, 200);

            // 원 그리기
            Ellipse(hdc, 200, 200, 300, 300);

            // 선 그리기 
            // 라인 시작점의 x,y값
            MoveToEx(hdc, 300, 100, NULL);
            // 라인 종점의 x, y값
            LineTo(hdc, 400, 150);
            // 다시 LineTo를 하면 이전의 종점이 시작점이 되서 선을 연결한다.
            LineTo(hdc, 500, 100);

            TCHAR strMouse[64] = {};
            // wsprintf : 유니코드 문자열을 만들어주는 함수
            // %d 에는 정수가 대입된다
            wsprintf(strMouse, TEXT("Start = x : %d y : %d"), g_tArea.ptStart.x , g_tArea.ptStart.y);
            // lstrlen : 유니코드 문자열의 길이를 구해주는 함수
            TextOut(hdc, 600, 30, strMouse, lstrlen(strMouse));

            if (g_tArea.bStart) {
                Rectangle(hdc, g_tArea.ptStart.x, g_tArea.ptStart.y, g_tArea.ptEnd.x, g_tArea.ptEnd.y);

            }

            */

            Ellipse(hdc, 300, 300, 400, 400);
            Ellipse(hdc, 500, 300, 600, 400);
            Ellipse(hdc, 350, 350, 550, 550);

            MoveToEx(hdc, 400, 410, NULL);
            LineTo(hdc, 430, 430);
            LineTo(hdc, 400, 450);

            MoveToEx(hdc, 500, 410, NULL);
            LineTo(hdc, 470, 430);
            LineTo(hdc, 500, 450);


            EndPaint(hWnd, &ps);
        }
        break;
    
    // 마우스 왼쪽 버튼을 눌렀을 때 들어오는 메세지
    case WM_LBUTTONDOWN:
        // 마우스 위치는 lParam에 들어오게 되는데 16비트로 쪼개서 x,y값이 32비트 변수에 들어오게 된다.
        // LOWORD, HIWORD 매크로를 이용해서 하위, 상위 16비트의 값을 얻어올 수 있다.
        if (!g_tArea.bStart) {
            g_tArea.bStart = true;
            g_tArea.ptStart.x = lParam & 0x0000ffff;
            g_tArea.ptStart.y = lParam >> 16;
            g_tArea.ptEnd = g_tArea.ptStart;

            // InvalidateRect 함수는 강제로 WM_PAINT 메세지를 호출해주는 함수
            // 1번인자는 윈도우 핸들, 2번인자는 초기화할 영역이 들어가는데
            // NULL을 넣어줄 경우 전체 화면을 대상으로 갱신 한다.
            // 3번인자는 TRUE일 경우 현재 화면을 지우고 갱신하고 FALSE일 경우 현재 화면을
            // 지우지 않고 갱신한다.
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    // 마우스 왼쪽 버튼을 누르다가 뗐을 때 발생하는 메세지
    case WM_LBUTTONUP:
        if (g_tArea.bStart) {
            g_tArea.bStart = false;
            g_tArea.ptEnd.x = lParam & 0x0000ffff;
            g_tArea.ptEnd.y = lParam >> 16;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    
    // 마우스가 움직일 때 들어오는 메세지
    case WM_MOUSEMOVE:
        if (g_tArea.bStart) {
            g_tArea.ptEnd.x = lParam & 0x0000ffff;
            g_tArea.ptEnd.y = lParam >> 16;
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    // 키가 눌러졌을때 들어오는 메세지
    case WM_KEYDOWN:
        // 이 메세지가 들어올 경우 wParam에 어떤 키를 눌렀는지가 들어온다
        switch (wParam) {
        case VK_ESCAPE:
            // ESC 키를 눌렀을 때 윈도우 종료
            DestroyWindow(hWnd);
            break;
        }
        break;
    // 윈도우를 종료시킬때 들어오는 메세지
    case WM_DESTROY:
        g_bLoop = false;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void Run() {

    // Delta Time 을 구해준다.
    LARGE_INTEGER tTime;
    QueryPerformanceCounter(&tTime);

    g_fDeltaTime = (tTime.QuadPart - g_tTime.QuadPart) / (float)g_tSecond.QuadPart;

    g_tTime = tTime;

    static float fTimeScale = 1.f;

    

    if (GetAsyncKeyState(VK_F1) & 0x8000) {
        fTimeScale -= g_fDeltaTime;
        if (fTimeScale < 0.f)
            fTimeScale = 0.f;
    }
    if (GetAsyncKeyState(VK_F2) & 0x8000) {
        fTimeScale += g_fDeltaTime;
        if (fTimeScale > 1.f)
            fTimeScale = 1.f;
    }
    


    // 플레이어 초당 이동속도 : 300
    float fSpeed = 400.f * g_fDeltaTime * fTimeScale;

    RECT rcWindow;
    GetClientRect(g_hWnd, &rcWindow); 

    if (GetAsyncKeyState('A') & 0x8000) {
        g_fPlayerAngle -= g_fDeltaTime * fTimeScale * PI;
    }


    if (GetAsyncKeyState('D') & 0x8000) {
        g_fPlayerAngle += g_fDeltaTime * fTimeScale * PI;
    }

    if (GetAsyncKeyState('S') & 0x8000) {
        g_tPlayer.x -= fSpeed * cosf(g_fPlayerAngle);
        g_tPlayer.y -= fSpeed * sinf(g_fPlayerAngle);
    }

    if (GetAsyncKeyState('W') & 0x8000) {
        g_tPlayer.x += fSpeed * cosf(g_fPlayerAngle);
        g_tPlayer.y += fSpeed * sinf(g_fPlayerAngle);
    }

    // 총구 위치를 구한다.
    g_tGunPos.x = g_tPlayer.x + cosf(g_fPlayerAngle) * g_fGunLength;
    g_tGunPos.y = g_tPlayer.y + sinf(g_fPlayerAngle) * g_fGunLength;


    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        BULLET tBullet;

        tBullet.tSphere.r = 25.f;
        tBullet.tSphere.x = g_tGunPos.x + cosf(g_fPlayerAngle)* tBullet.tSphere.r;
        tBullet.tSphere.y = g_tGunPos.y + sinf(g_fPlayerAngle) * tBullet.tSphere.r;
        tBullet.fDist = 0.f;
        tBullet.fLimitDist = 500.f;
        tBullet.fAngle = g_fPlayerAngle;

        g_PlayerBulletList.push_back(tBullet);
    }


    if (GetAsyncKeyState('2') & 0x8000) {
        float fAngle =  0.f;

            for (int i = 0; i < 36; ++i) {
                BULLET tBullet;

                tBullet.tSphere.r = 25.f;
                tBullet.tSphere.x = g_tGunPos.x + cosf(fAngle) * 25.f;
                tBullet.tSphere.y = g_tGunPos.y + sinf(fAngle) * 25.f;
                tBullet.fDist = 0.f;
                tBullet.fLimitDist = 500.f;
                tBullet.fAngle = fAngle;

                g_PlayerBulletList.push_back(tBullet);

                fAngle += 3.141592 / 18.f;
        }
    }




    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        POINT ptMouse;
        // 마우스 위치를 얻어온다.
        // 마우스 위치는 스크린 좌표 기준으로 받아온다.
        GetCursorPos(&ptMouse);

        // 스크린 좌표를 클라이언트 좌표로 변환한다. 마우스좌표가 클라이언트 좌표로 변경됨
        ScreenToClient(g_hWnd, &ptMouse);

        // 플레이어와 커서의 충돌 처리
        /*if (g_tPlayerRC.l <= ptMouse.x && ptMouse.x <= g_tPlayerRC.r &&
            g_tPlayerRC.t <= ptMouse.y && ptMouse.y <= g_tPlayerRC.b) {
            MessageBox(NULL, L"플레이어 클릭", L"마우스 클릭", MB_OK);
        }*/

        // 몬스터와 커서의 충돌처리

        float fMX = g_tMonster.tSphere.x - ptMouse.x;
        float fMY = g_tMonster.tSphere.y - ptMouse.y;
        float fMDist = sqrtf(fMX * fMX + fMY * fMY);
        if(g_tMonster.tSphere.r >= fMDist)
            MessageBox(NULL, L"몬스터 클릭", L"마우스 클릭", MB_OK);
        
    }

    

    // 플레이어 총알 이동
    list<BULLET>::iterator iter;
    list<BULLET>::iterator iterEnd = g_PlayerBulletList.end();

    fSpeed = 600.f * g_fDeltaTime * fTimeScale;

    for (iter = g_PlayerBulletList.begin(); iter != iterEnd;) {
        (*iter).tSphere.x += cosf((*iter).fAngle) * fSpeed;
        (*iter).tSphere.y += sinf((*iter).fAngle) * fSpeed;

        (*iter).fDist += fSpeed;

        float fX = (*iter).tSphere.x - g_tMonster.tSphere.x;
        float fY = (*iter).tSphere.y - g_tMonster.tSphere.y;
        float fDist = sqrtf(fX * fX + fY * fY);


        // 플레이어의 총알이 몬스터와 충돌할 경우
        if (fDist <= (*iter).tSphere.r + g_tMonster.tSphere.r) {
            iter = g_PlayerBulletList.erase(iter);
            iterEnd = g_PlayerBulletList.end();
        }


        // 지정된 사정거리까지만 총알이 나감
        // fLimitDist를 넘어가면 총알 삭제됨
        else if ((*iter).fDist >= (*iter).fLimitDist) {
            iter = g_PlayerBulletList.erase(iter);
            iterEnd = g_PlayerBulletList.end();
        }

        // 총알이 화면 밖으로 나갔을 때
        else if ((*iter).tSphere.x + (*iter).tSphere.r >= rc.right) {
            iter = g_PlayerBulletList.erase(iter);
            iterEnd = g_PlayerBulletList.end();
        }

        else
            ++iter;
    }


    // 몬스터 이동
    
    g_tMonster.tSphere.y += g_tMonster.fSpeed * g_fDeltaTime * fTimeScale * g_tMonster.iDir;
    

    if (g_tMonster.tSphere.y + g_tMonster.tSphere.r >= 600) {
        g_tMonster.iDir = -1;
        g_tMonster.tSphere.y = 550;
    }
    else if (g_tMonster.tSphere.y - g_tMonster.tSphere.r <= 0) {
        g_tMonster.iDir = 1;
        g_tMonster.tSphere.y = 50;
    }


    // 몬스터 총알 발사
    list<BULLET>::iterator it;
    list<BULLET>::iterator itEnd = g_MonsterBulletList.end();

    
    g_tMonster.fTime += g_fDeltaTime * fTimeScale;

    if (g_tMonster.fTime >= g_tMonster.fLimitTime) {
        BULLET monsterBullet = {};

        monsterBullet.tSphere.r = 25.f;
        monsterBullet.tSphere.x = g_tMonster.tSphere.x - g_tMonster.tSphere.r - monsterBullet.tSphere.r;
        monsterBullet.tSphere.y = g_tMonster.tSphere.y;

        monsterBullet.fDist = 0.f;
        monsterBullet.fLimitDist = 800.f;

        g_MonsterBulletList.push_back(monsterBullet);
        g_tMonster.fTime -= g_tMonster.fLimitTime;
    }


    for (it = g_MonsterBulletList.begin(); it != itEnd;) {
        (*it).tSphere.x -= fSpeed;

        (*it).fDist += fSpeed;

        // 지정된 사정거리까지만 총알이 나감
        // fLimitDist를 넘어가면 총알 삭제됨
        if ((*it).fDist >= (*it).fLimitDist) {
            it = g_MonsterBulletList.erase(it);
            itEnd = g_MonsterBulletList.end();
        }

        // 총알이 화면 밖으로 나갔을 때
        else if ((*it).tSphere.x - (*it).tSphere.r <= rc.left) {
            it = g_MonsterBulletList.erase(it);
            itEnd = g_MonsterBulletList.end();
        }


        /*
        // 사각형 총알과 플레이어 충돌
        else if (g_tPlayerRC.l <= (*it).rc.r && (*it).rc.l <= g_tPlayerRC.r &&
            g_tPlayerRC.t <= (*it).rc.b && (*it).rc.t <= g_tPlayerRC.b) {
            it = g_MonsterBulletList.erase(it);
            itEnd = g_MonsterBulletList.end();
        }
        */

        else {
            ++it;
        }
            
    }

    


    // 출력
    //Rectangle(g_hDC, 0, 0, 800, 600);
    //Rectangle(g_hDC, g_tPlayerRC.l, g_tPlayerRC.t, g_tPlayerRC.r, g_tPlayerRC.b);
 
    Ellipse(g_hDC, g_tPlayer.x - g_tPlayer.r, g_tPlayer.y - g_tPlayer.r,
        g_tPlayer.x + g_tPlayer.r, g_tPlayer.y + g_tPlayer.r);


    MoveToEx(g_hDC, g_tPlayer.x, g_tPlayer.y, NULL);
    LineTo(g_hDC, g_tGunPos.x, g_tGunPos.y);




    Ellipse(g_hDC, g_tMonster.tSphere.x - g_tMonster.tSphere.r,
        g_tMonster.tSphere.y - g_tMonster.tSphere.r,
        g_tMonster.tSphere.x + g_tMonster.tSphere.r,
        g_tMonster.tSphere.y + g_tMonster.tSphere.r);
    
    
    for (iter = g_PlayerBulletList.begin(); iter != iterEnd; ++iter) {
        Ellipse(g_hDC, (*iter).tSphere.x - (*iter).tSphere.r,
            (*iter).tSphere.y - (*iter).tSphere.r,
            (*iter).tSphere.x + (*iter).tSphere.r,
            (*iter).tSphere.y + (*iter).tSphere.r);
    }

    for (it = g_MonsterBulletList.begin(); it != itEnd; ++it) {
        Ellipse(g_hDC, (*it).tSphere.x - (*it).tSphere.r,
            (*it).tSphere.y - (*it).tSphere.r,
            (*it).tSphere.x + (*it).tSphere.r,
            (*it).tSphere.y + (*it).tSphere.r);
    }
}

float GetAngle(POINT r1, POINT r2) {
    float x, y, r;

    y = r1.y - r2.y;
    r = sqrtf(pow(r1.x - r2.x, 2) + pow(r1.y - r2.y, 2));
    x = sqrtf(pow(r, 2) - pow(y, 2));
    
    if (r1.y < r2.y) {
        return acosf(x / r);
    }
    else if (r1.y >= r2.y) {
        return 2 * PI - acosf(x / r);
    }

}