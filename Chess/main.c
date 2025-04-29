#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <gl/gl.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "stb_easy_font.h"

#include <mmsystem.h>

#include "back.h"

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);


typedef struct {
    int x, y;
} Coord;


unsigned int textures[42];

Pos marked_cells[N];

Coord click1, click2;

char digits[8][2] = {"1", "2", "3", "4", "5", "6", "7", "8"};
char letters[8][2] = {"A", "B", "C", "D", "E", "F", "G", "H"};


void print_string(float x, float y, char *text, float r, float g, float b)
{
  static char buffer[99999]; // ~500 chars
  int num_quads;

  num_quads = stb_easy_font_print(x, y, text, NULL, buffer, sizeof(buffer));

  glColor3f(r,g,b);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 16, buffer);
  glDrawArrays(GL_QUADS, 0, num_quads*4);
  glDisableClientState(GL_VERTEX_ARRAY);
}

BOOL is_cell_in_map(int x, int y) {
    return (x >= 0) && (y >= 0) && (x < mapH) && (y < mapH);
}

void screen_to_opengl(HWND hwnd, int x, int y, float *ox, float *oy) {
    RECT rct;
    GetClientRect(hwnd, &rct);
    *ox = x / (float)rct.right * mapW;
    *oy = y / (float)rct.bottom * mapH;
}

void clear_clicks() {
    click1.x = -1; click1.y = -1;
    click2.x = -1; click2.y = -1;
}

float vertex[] = {0,0, 1,0, 1,-1, 0,-1};
float texCoord[] = {0,0, 1,0, 1,1, 0,1};

void display_texture(int idx) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[idx]);

    glColor3f(1, 1, 1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, vertex);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoord);
    glDrawArrays(GL_POLYGON, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}

void show_cell(int i, int j) {
    if (map[i][j].type == PAWN) {
        if (map[i][j].col == 0) display_texture(0+(i+j)%2);
        if (map[i][j].col == 1) display_texture(12+(i+j)%2);
    }
    if (map[i][j].type == ROOK) {
        if (map[i][j].col == 0) display_texture(2+(i+j)%2);
        if (map[i][j].col == 1) display_texture(14+(i+j)%2);
    }
    if (map[i][j].type == HORSE) {
        if (map[i][j].col == 0) display_texture(4+(i+j)%2);
        if (map[i][j].col == 1) display_texture(16+(i+j)%2);
    }
    if (map[i][j].type == BISHOP) {
        if (map[i][j].col == 0) display_texture(6+(i+j)%2);
        if (map[i][j].col == 1) display_texture(18+(i+j)%2);
    }
    if (map[i][j].type == QUEEN) {
        if (map[i][j].col == 0) display_texture(8+(i+j)%2);
        if (map[i][j].col == 1) display_texture(20+(i+j)%2);
    }
    if (map[i][j].type == KING) {
        if (map[i][j].col == 0) display_texture(10+(i+j)%2);
        if (map[i][j].col == 1) display_texture(22+(i+j)%2);
    }
    if (map[i][j].type == EMPTY) {
        display_texture(24+(i+j)%2);
    }
}

void show_red_frame() {
    glEnable(GL_BLEND);

    glBegin(GL_QUADS);
        glColor4f(1, 0, 0, 0.6);
        glVertex2f(0, 0);
        glVertex2f(1, 0);
        glVertex2f(1, -0.1);
        glVertex2f(0, -0.1);
    glEnd();
    glBegin(GL_QUADS);
        glColor4f(1, 0, 0, 0.6);
        glVertex2f(0, -0.9);
        glVertex2f(1, -0.9);
        glVertex2f(1, -1);
        glVertex2f(0, -1);
    glEnd();

    glBegin(GL_QUADS);
        glColor4f(1, 0, 0, 0.6);
        glVertex2f(0.9, -0.1);
        glVertex2f(1, -0.1);
        glVertex2f(1, -0.9);
        glVertex2f(0.9, -0.9);
    glEnd();
    glBegin(GL_QUADS);
        glColor4f(1, 0, 0, 0.6);
        glVertex2f(0, -0.1);
        glVertex2f(0.1, -0.1);
        glVertex2f(0.1, -0.9);
        glVertex2f(0, -0.9);
    glEnd();

    glDisable(GL_BLEND);
}

void show_blue_frame() {
    glEnable(GL_BLEND);

    glBegin(GL_TRIANGLES);
        glColor4f(0, 0, 1, 0.6);
        glVertex2f(0, 0);
        glVertex2f(0.3, 0);
        glVertex2f(0, -0.3);
    glEnd();

    glBegin(GL_TRIANGLES);
        glColor4f(0, 0, 1, 0.6);
        glVertex2f(1, 0);
        glVertex2f(0.7, 0);
        glVertex2f(1, -0.3);
    glEnd();

    glBegin(GL_TRIANGLES);
        glColor4f(0, 0, 1, 0.6);
        glVertex2f(1, -1);
        glVertex2f(1, -0.7);
        glVertex2f(0.7, -1);
    glEnd();

    glBegin(GL_TRIANGLES);
        glColor4f(0, 0, 1, 0.6);
        glVertex2f(0, -1);
        glVertex2f(0.3, -1);
        glVertex2f(0, -0.7);
    glEnd();


    glDisable(GL_BLEND);
}

void show_gray_frame() {
    glEnable(GL_BLEND);

    glBegin(GL_QUADS);
        glColor4f(0.5, 0.5, 0.5, 0.6);
        glVertex2f(0, 0);
        glVertex2f(1, 0);
        glVertex2f(1, -1);
        glVertex2f(0, -1);
    glEnd();

    glDisable(GL_BLEND);
}

void show_marked_cell(int style) {
    glEnable(GL_BLEND);
    if (style == 0) {
        glBegin(GL_QUADS);
            glColor4f(0, 0, 1, 0.6);
            glVertex2f(0.4, -0.4);
            glVertex2f(0.6, -0.4);
            glVertex2f(0.6, -0.6);
            glVertex2f(0.4, -0.6);
        glEnd();
    }

    if (style == 1) {
        glBegin(GL_QUADS);
            glColor4f(1, 0, 0, 0.6);
            glVertex2f(0.4, -0.4);
            glVertex2f(0.6, -0.4);
            glVertex2f(0.6, -0.6);
            glVertex2f(0.4, -0.6);
        glEnd();
    }
    glDisable(GL_BLEND);
}

void clear_marked_cells() {
    for (int q=0; q<N; ++q) {
        marked_cells[q].x = -1;
        marked_cells[q].y = -1;
        marked_cells[q].col = -1;
        marked_cells[q].type = -1;
    }
}

void mark_cells(int x, int y) {
    if (turn != map[x][y].col) return;
    for (int q=0; q<N; ++q) {
        marked_cells[q] = map[x][y].pos[q];
    }
}

void display_map_coords() {
    for (int i=0; i<mapH; ++i) {
        glLoadIdentity();
        glScalef(2.0/mapH, 2.0/mapW, 1);
        glTranslatef(-mapH*0.5, mapW*0.5, 0);
        glTranslatef(0.02, -i-0.04, 0);

        glPushMatrix();

        glScalef(0.03, -0.03, 1);
        print_string(0,0, digits[mapH-i-1], i%2, i%2, i%2);

        glPopMatrix();
    }

    for (int j=0; j<mapW; ++j) {
        glLoadIdentity();
        glScalef(2.0/mapH, 2.0/mapW, 1);
        glTranslatef(-mapH*0.5, mapW*0.5, 0);
        glTranslatef(j+0.85, -7.78, 0);

        glPushMatrix();

        glScalef(0.03, -0.03, 1);
        print_string(0,0, letters[j], (j+1)%2, (j+1)%2, (j+1)%2);

        glPopMatrix();
    }
}

void display_marked_cells() {
    glLoadIdentity();
    glScalef(2.0/mapH, 2.0/mapW, 1);
    glTranslatef(-mapH*0.5, mapW*0.5, 0);
    for (int q=0; q<N; ++q) {
        if (marked_cells[q].col != -1) {
            glPushMatrix();
            glTranslatef(marked_cells[q].y, -marked_cells[q].x, 0);
            if (marked_cells[q].col == 2) {
                show_marked_cell(0);
            } else {
                show_marked_cell(1);
            }
            glPopMatrix();
        }
    }
}

void display_turning_set(int i, int j) {
    glLoadIdentity();
    glScalef(2.0/mapH, 2.0/mapW, 1);
    glTranslatef(-mapH*0.5, mapW*0.5, 0);
    for (int i=0; i<mapH; ++i) {
        for (int j=0; j<mapW; ++j) {
            glPushMatrix();

            glTranslatef(j, -i, 0);
            show_gray_frame();

            glPopMatrix();
        }
    }

    glLoadIdentity();
    glScalef(2.0/mapH, 2.0/mapW, 1);
    glTranslatef(-mapH*0.5, mapW*0.5, 0);

    int k, p;
    if (i == 0) {
        k = -1;
        p = 0;
    }
    if (i == 7) {
        k = 1;
        p = 8;
    }

    glPushMatrix();
    glTranslatef(j, -i, 0);
    display_texture(32+p+(i+j)%2);
    glPopMatrix();


    glPushMatrix();
    glTranslatef(j, -i+k, 0);
    display_texture(28+p+(i-k+j)%2);
    glPopMatrix();


    glPushMatrix();
    glTranslatef(j, -i+2*k, 0);
    display_texture(26+p+(i-2*k+j)%2);
    glPopMatrix();


    glPushMatrix();
    glTranslatef(j, -i+3*k, 0);
    display_texture(30+p+(i-3*k+j)%2);
    glPopMatrix();

}

void show_map() {
    glLoadIdentity();
    glScalef(2.0/mapH, 2.0/mapW, 1);
    glTranslatef(-mapH*0.5, mapW*0.5, 0);
    for (int i=0; i<mapH; ++i) {
        for (int j=0; j<mapW; ++j) {
            glPushMatrix();

            glTranslatef(j, -i, 0);
            show_cell(i, j);
            if (white_king_under_check && map[i][j].type == KING && map[i][j].col == 0) show_red_frame();
            if (black_king_under_check && map[i][j].type == KING && map[i][j].col == 1) show_red_frame();

            glPopMatrix();
        }
    }

    display_map_coords();

    if (pawn_turning) {
        display_turning_set(turning_i, turning_j);
    }

    display_marked_cells();

    if (click1.x != -1 && click1.y != -1) {
        glLoadIdentity();
        glScalef(2.0/mapH, 2.0/mapW, 1);
        glTranslatef(-mapH*0.5, mapW*0.5, 0);
        glTranslatef(click1.y, -click1.x, 0);

        glPushMatrix();

        show_blue_frame();

        glPopMatrix();
    }
}

void init_texture(int idx, char fname[]) {
    int width, height, cnt;
    unsigned char *data = stbi_load(fname, &width, &height, &cnt, 0);
    glGenTextures(1, &textures[idx]);
    glBindTexture(GL_TEXTURE_2D, textures[idx]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                                    0, cnt == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
}

void load_textures() {
    init_texture(0, "images/white_pawn_W.png");
    init_texture(1, "images/white_pawn_B.png");
    init_texture(2, "images/white_rook_W.png");
    init_texture(3, "images/white_rook_B.png");
    init_texture(4, "images/white_horse_W.png");
    init_texture(5, "images/white_horse_B.png");
    init_texture(6, "images/white_bishop_W.png");
    init_texture(7, "images/white_bishop_B.png");
    init_texture(8, "images/white_queen_W.png");
    init_texture(9, "images/white_queen_B.png");
    init_texture(10, "images/white_king_W.png");
    init_texture(11, "images/white_king_B.png");
    init_texture(12, "images/black_pawn_W.png");
    init_texture(13, "images/black_pawn_B.png");
    init_texture(14, "images/black_rook_W.png");
    init_texture(15, "images/black_rook_B.png");
    init_texture(16, "images/black_horse_W.png");
    init_texture(17, "images/black_horse_B.png");
    init_texture(18, "images/black_bishop_W.png");
    init_texture(19, "images/black_bishop_B.png");
    init_texture(20, "images/black_queen_W.png");
    init_texture(21, "images/black_queen_B.png");
    init_texture(22, "images/black_king_W.png");
    init_texture(23, "images/black_king_B.png");
    init_texture(24, "images/empty_W.png");
    init_texture(25, "images/empty_B.png");

    init_texture(26, "images/turning_white_rook_W.png");
    init_texture(27, "images/turning_white_rook_B.png");
    init_texture(28, "images/turning_white_horse_W.png");
    init_texture(29, "images/turning_white_horse_B.png");
    init_texture(30, "images/turning_white_bishop_W.png");
    init_texture(31, "images/turning_white_bishop_B.png");
    init_texture(32, "images/turning_white_queen_W.png");
    init_texture(33, "images/turning_white_queen_B.png");
    init_texture(34, "images/turning_black_rook_W.png");
    init_texture(35, "images/turning_black_rook_B.png");
    init_texture(36, "images/turning_black_horse_W.png");
    init_texture(37, "images/turning_black_horse_B.png");
    init_texture(38, "images/turning_black_bishop_W.png");
    init_texture(39, "images/turning_black_bishop_B.png");
    init_texture(40, "images/turning_black_queen_W.png");
    init_texture(41, "images/turning_black_queen_B.png");
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          600,
                          600,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    load_textures();
    clear_clicks();
    clear_marked_cells();
    game_start();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            show_map();

            SwapBuffers(hDC);
            Sleep (1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_SIZE:
        {
            glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
            glLoadIdentity();
            float k = LOWORD(lParam) / (float)HIWORD(lParam);
            glOrtho(-k, k, -1,1, -1,1);
        }

        case WM_LBUTTONDOWN:
        {
            POINTFLOAT pf;
            screen_to_opengl(hwnd, LOWORD(lParam), HIWORD(lParam), &pf.x, &pf.y);
            int x = (int)pf.y;
            int y = (int)pf.x;

            if (is_cell_in_map(x, y)) {
                if (pawn_turning) {
                    if (init_pawn_turning(x, y)) {
                        generate_positions();
                        check_move_result();
                        turn = 1 - turn; --move_number;
                        show(-1, -1, -1, -1);
                        turn = 1 - turn; ++move_number;
                    }
                } else {
                    if (map[x][y].col == turn) {
                        click1.x = x; click1.y = y;
                        mark_cells(x, y);
                    } else {
                        if (click1.x != -1 && click1.y != -1) {
                            click2.x = x; click2.y = y;
                            if (move_to(click1.x, click1.y, click2.x, click2.y)) {
                                show(click1.x, click1.y, click2.x, click2.y);
                                ++move_number;
                                generate_positions();
                                turn = 1 - turn;
                                if (check_move_result()) {
                                    PlaySound("sound_effects/game_end.wav", NULL, SND_ASYNC);
                                }
                                else if (figure_taken) {
                                    PlaySound("sound_effects/taking_figure.wav", NULL, SND_ASYNC);
                                    figure_taken = 0;
                                } else {
                                    PlaySound("sound_effects/figure_move.wav", NULL, SND_ASYNC);
                                }
                            }
                        }
                        clear_marked_cells();
                        clear_clicks();
                    }
                }
            }
        }

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}
