#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "raylib.h"

struct Point {
    double x, y;
    Color color;
};

Color colors[7] = {
    (Color){0, 0, 139, 255},      // 深蓝色
    (Color){0, 0, 255, 255},      // 蓝色
    (Color){65, 105, 225, 255},   // 皇家蓝
    (Color){135, 206, 235, 255},  // 天蓝色
    (Color){173, 216, 230, 255},  // 浅蓝色
    (Color){176, 224, 230, 255},  // 粉蓝色
    (Color){95, 158, 160, 255}    // 青灰色
};
#define xScreen 1200
#define yScreen 800
#define pi 3.1415927
#define e 2.71828
#define avg_d 0.16
#define basic_num 500
#define circle_num 222
#define frames 25

struct Point ori_pts[basic_num];
struct Point pts[basic_num * circle_num];

Texture2D textures[frames];
RenderTexture2D renderTextures[frames];

double screen_x(double x) {
    return x + xScreen / 2;
}

double screen_y(double y) {
    return yScreen / 2 - y;
}

int create_random(int x1, int x2) {
    if (x1 > x2) {
        return rand() % (x1 - x2 + 1) + x2;
    } else {
        return rand() % (x2 - x1 + 1) + x1;
    }
}

void create_data() {
    int idx = 0;
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;

    for (double r = 0.1; r <= 2 * pi; r += 0.005) {
        x2 = 16 * pow(sin(r), 3);
        y2 = 13 * cos(r) - 5 * cos(2 * r) - 2 * cos(3 * r) - cos(4 * r);

        double d = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
        if (d > avg_d) {
            x1 = x2;
            y1 = y2;
            ori_pts[idx].x = x2;
            ori_pts[idx++].y = y2;
        }
    }

    idx = 0;
    double lightness = 1.6;
    for (double size = 0.1; size <= 20; size += 0.1) {
        double suc_p = 1 / (1 + pow(e, 8 - size / 2));
        if (lightness > 1) lightness -= 0.005;

        for (int i = 0; i < basic_num; ++i) {
            if (suc_p > (create_random(100, 0) / 100.0)) {
                Color color = colors[create_random(0, 6)];
                pts[idx].color = (Color){
                    (unsigned char)(color.r / lightness),
                    (unsigned char)(color.g / lightness),
                    (unsigned char)(color.b / lightness),
                    255
                };
                pts[idx].x = size * ori_pts[i].x + create_random(-4, 4);
                pts[idx++].y = size * ori_pts[i].y + create_random(-4, 4);
            }
        }
    }
    
    int pts_size = idx;

    for (int j = 0; j < frames; ++j) {

        renderTextures[j] = LoadRenderTexture(xScreen, yScreen);

        BeginTextureMode(renderTextures[j]);
        ClearBackground(BLACK);  
        
        for (idx = 0; idx < pts_size; ++idx) {
            double x = pts[idx].x, y = pts[idx].y;
            double d = sqrt(x * x + y * y);
            double d_increase = -0.0009 * d * d + 0.35714 * d + 5;
            double x_increase = d_increase * x / d / frames;
            double y_increase = d_increase * y / d / frames;
            pts[idx].x += x_increase;
            pts[idx].y += y_increase;

            DrawCircle((int)screen_x(pts[idx].x), (int)screen_y(pts[idx].y), 1, pts[idx].color);
        }

        for (double size = 17; size < 23; size += 0.3) {
            for (idx = 0; idx < basic_num; ++idx) {
                if ((create_random(0, 100) / 100.0 > 0.6 && size >= 20) || 
                    (size < 20 && create_random(0, 100) / 100.0 > 0.95)) {
                    double x, y;
                    if (size >= 20) {
                        x = ori_pts[idx].x * size + create_random(-j * j / 5 - 15, j * j / 5);
                        y = ori_pts[idx].y * size + create_random(-j * j / 5 - 15, j * j / 5);
                    } else {
                        x = ori_pts[idx].x * size + create_random(-5, 5);
                        y = ori_pts[idx].y * size + create_random(-5, 5);
                    }
                    DrawCircle((int)screen_x(x), (int)screen_y(y), 1, colors[create_random(0, 6)]);
                }
            }
        }
        
        EndTextureMode();
        
        textures[j] = renderTextures[j].texture;
    }
}

int main() {
    InitWindow(xScreen, yScreen, "Heart Animation");
    SetTargetFPS(60);
    
    srand((unsigned int)time(0));
    create_data();
    
    bool extend = true, shrink = false;
    int f = 0;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
      
        DrawTexture(textures[f], 0, 0, WHITE);
        
        EndDrawing();
    
        if (extend) {
            if (f == frames - 1) {
                shrink = true;
                extend = false;
            } else {
                ++f;
            }
        } else {
            if (f == 0) {
                shrink = false;
                extend = true;
            } else {
                --f;
            }
        }

        WaitTime(0.02);
    }

    for (int i = 0; i < frames; i++) {
        UnloadRenderTexture(renderTextures[i]);
    }
    
    CloseWindow();
    return 0;
}