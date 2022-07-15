#include <stdio.h>
#include <windows.h>
#include <math.h>

#define MATRIX_Y_SIZE 80
#define MATRIX_X_SIZE 80
#define FRAME(px, py, sx, sy)                       \
    for (vector pos = {0, py}; pos.y < sy; pos.y++) \
        for (pos.x = px; pos.x < sx; pos.x++)
#define FRAGMENT FRAME(0, 0, MATRIX_X_SIZE, MATRIX_Y_SIZE)

typedef unsigned char byte;

enum color
{
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    PURPLE = 5,
    YELLOW = 6,
    WHITE = 7,
    GRAY = 8,
    LIGHT_BLUE = 9,
    LIGHT_GREEN = 10,
    LIGHT_CYAN = 11,
    LIGHT_RED = 12,
    LIGHT_PURPLE = 13,
    LIGHT_YELLOW = 14,
    LIGHT_WHITE = 15
};

typedef struct
{
    byte x;
    byte y;
} vector;

typedef struct
{
    byte foreground;
    byte background;
    byte texture;
} pixel;

pixel CLEAR_COLOR = {
    .foreground = WHITE,
    .background = BLACK,
    .texture = ' '};

pixel *matrix[MATRIX_Y_SIZE][MATRIX_X_SIZE];
pixel *frame_buffer[MATRIX_Y_SIZE][MATRIX_X_SIZE];
HANDLE console_handler;

void gotoxy(int x, int y)
{
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void draw(pixel *pix, vector pos)
{

    if (pos.x >= MATRIX_X_SIZE || pos.y >= MATRIX_Y_SIZE)
    {
        return;
    }

    if (pos.x < 0 || pos.y < 0)
    {
        return;
    }

    matrix[pos.y][pos.x] = pix;
}

pixel *get_pixel(vector pos, pixel *buffer[MATRIX_Y_SIZE][MATRIX_X_SIZE])
{

    if (pos.x >= MATRIX_X_SIZE || pos.y >= MATRIX_Y_SIZE)
    {
        return NULL;
    }

    if (pos.x < 0 || pos.y < 0)
    {
        return NULL;
    }

    return buffer[pos.y][pos.x];
}

void g_color(byte fg, byte bg)
{
    SetConsoleTextAttribute(console_handler, fg | (bg << 4));
}

void set_color(pixel *pixel)
{
    g_color(pixel->foreground, pixel->background);
}

void fill(pixel *pix)
{
    FRAGMENT
    {
        draw(pix, pos);
    }
}

void draw_rect(pixel *pix, vector pos_p, vector size)
{
    FRAME(pos_p.x, pos_p.y, pos_p.x + size.x, pos_p.y + size.y)
    {
        draw(pix, pos);
    }
}

void render()
{
    FRAGMENT
    {
        pixel *pix = get_pixel(pos, matrix);
        if (get_pixel(pos, frame_buffer) != pix)
        {
            gotoxy(pos.x, pos.y);
            set_color(pix);
            printf("%c", pix->texture);
            frame_buffer[pos.y][pos.x] = matrix[pos.y][pos.x];
        }
    }

    gotoxy(0, 0);
    set_color(&CLEAR_COLOR);
}

void draw_border()
{
    for (int bx = 0; bx <= MATRIX_X_SIZE + 1; bx++)
    {
        gotoxy(bx, MATRIX_Y_SIZE);
        printf("%c", '-');
    }
    for (int by = 0; by <= MATRIX_Y_SIZE + 1; by++)
    {
        gotoxy(MATRIX_X_SIZE, by);
        printf("%c", '|');
    }
}

void init()
{
    draw_border();
    console_handler = GetStdHandle(STD_OUTPUT_HANDLE);
    fill(&CLEAR_COLOR);
    render();
}

struct bounce
{
    vector pos;
    vector size;
    vector speed;
    pixel *pix;
} ball;

void init_bounce()
{
    ball.pos = (vector){
        .x = 20,
        .y = 5};
    ball.size = (vector){
        .x = 30,
        .y = 30};
    ball.speed = (vector){
        .x = 1,
        .y = 1};
    ball.pix = (pixel *)malloc(sizeof(pixel));
    ball.pix->foreground = BLACK;
    ball.pix->background = BLACK;
    ball.pix->texture = ' ';
}

void update_bounce()
{
    ball.pos.x += ball.speed.x;
    ball.pos.y += ball.speed.y;
    if (ball.pos.x < 0 || ball.pos.x + ball.size.x > MATRIX_X_SIZE)
    {
        ball.pos.x -= ball.speed.x;
        ball.speed.x *= -1;
    }
    if (ball.pos.y < 0 || ball.pos.y + ball.size.y > MATRIX_Y_SIZE)
    {
        ball.pos.y -= ball.speed.y;
        ball.speed.y *= -1;
    }
}

pixel BOUNCE_SHADER = {
    .foreground = BLACK,
    .background = PURPLE,
    .texture = ' '};

pixel BOUNCE_SHADER2 = {
    .foreground = BLACK,
    .background = BLUE,
    .texture = ' '};

void bounce_shader(pixel *pix, vector pos_p, vector size, int time)
{
    FRAME(pos_p.x, pos_p.y, pos_p.x + size.x, pos_p.y + size.y)
    {

        float relative_x = ((float)pos.x - (float)pos_p.x) / (float)size.x;
        float relative_y = ((float)pos.y - (float)pos_p.y) / (float)size.y;

        float center_x = relative_x - 0.5;
        float center_y = relative_y - 0.5;
        float distance = sqrt(center_x * center_x + center_y * center_y);

        float teta = (float)time * 0.1 - 6.0 * distance;
        float rx = cos(teta) * center_x - sin(teta) * center_y;
        float ry = sin(teta) * center_x + cos(teta) * center_y;

        if (cos(atan2(rx, ry) * 3.0) < distance * 2.0)
        {
            draw(pix, pos);
        }
        else
        {
            if (cos(time * 0.2 + distance * 10.0) > 0.0)
                draw(&BOUNCE_SHADER, pos);
            else
                draw(&BOUNCE_SHADER2, pos);
        }
    }
}

void render_bounce(int time)
{
    bounce_shader(ball.pix, ball.pos, ball.size, time);
}

void game_loop()
{
    int time = 0;
    while (1)
    {
        fill(&CLEAR_COLOR);
        update_bounce();
        render_bounce(time);
        render();
        time += 1;
        Sleep(50);
    }
}

int main()
{
    init_bounce();
    init();
    game_loop();
    return 0;
}