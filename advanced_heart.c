#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "raylib.h"

struct Point {
    double x, y;
    Color color;
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

double timer = 0.0;
int color_mode = 0; // 0:彩虹, 1:火焰, 2:海洋

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

// 1. 彩虹渐变效果
Color get_rainbow_color(double time, double x, double y) {
    double hue = fmod(time * 0.5 + atan2(y, x) / (2 * pi), 1.0);
    
    double r, g, b;
    double h = hue * 6.0;
    int i = (int)h;
    double f = h - i;
    double p = 0.0;
    double q = 1.0 - f;
    double t = f;
    
    switch (i % 6) {
        case 0: r = 1.0; g = t; b = p; break;
        case 1: r = q; g = 1.0; b = p; break;
        case 2: r = p; g = 1.0; b = t; break;
        case 3: r = p; g = q; b = 1.0; break;
        case 4: r = t; g = p; b = 1.0; break;
        case 5: r = 1.0; g = p; b = q; break;
    }
    
    return (Color){
        (unsigned char)(r * 255),
        (unsigned char)(g * 255),
        (unsigned char)(b * 255),
        255
    };
}

// 2. 火焰效果
Color get_fire_color(double time, double x, double y) {
    double intensity = 0.5 + 0.5 * sin(time * 3 + sqrt(x*x + y*y) * 0.1);
    double flicker = 0.8 + 0.2 * sin(time * 8 + x * 0.5);
    
    return (Color){
        (unsigned char)(255 * intensity * flicker),
        (unsigned char)(100 * intensity * flicker),
        (unsigned char)(50 * (1.0 - intensity) * flicker),
        255
    };
}

// 3. 海洋波浪效果
Color get_ocean_color(double time, double x, double y) {
    double wave1 = sin(time * 2 + x * 0.05) * 0.3 + 0.7;
    double wave2 = cos(time * 1.5 + y * 0.03) * 0.2 + 0.8;
    double wave = (wave1 + wave2) * 0.5;
    
    return (Color){
        (unsigned char)(30 + 40 * wave),
        (unsigned char)(80 + 100 * wave),
        (unsigned char)(150 + 105 * wave),
        255
    };
}

// 根据当前模式获取颜色
Color get_dynamic_color(double time, double x, double y) {
    switch (color_mode) {
        case 0: return get_rainbow_color(time, x, y);
        case 1: return get_fire_color(time, x, y);
        case 2: return get_ocean_color(time, x, y);
        default: return get_rainbow_color(time, x, y);
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
                Color color = get_dynamic_color(size * 0.1, ori_pts[i].x, ori_pts[i].y);
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

            // 更新粒子颜色
            double time_factor = timer + j * 0.04;
            pts[idx].color = get_dynamic_color(time_factor, pts[idx].x, pts[idx].y);
            
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
                    Color dynamic_color = get_dynamic_color(timer + size * 0.1, x, y);
                    DrawCircle((int)screen_x(x), (int)screen_y(y), 1, dynamic_color);
                }
            }
        }
        
        EndTextureMode();
        textures[j] = renderTextures[j].texture;
    }
}

int main() {
    InitWindow(xScreen, yScreen, "Dynamic Color Heart - Press 1,2,3 to change colors");
    SetTargetFPS(60);
    
    srand((unsigned int)time(0));
    create_data();
    
    bool extend = true, shrink = false;
    int f = 0;
    bool need_recreate = false;

    while (!WindowShouldClose()) {
        timer += GetFrameTime();
        
        // 检测按键切换颜色模式
        if (IsKeyPressed(KEY_ONE)) {
            color_mode = 0;
            need_recreate = true;
        } else if (IsKeyPressed(KEY_TWO)) {
            color_mode = 1;
            need_recreate = true;
        } else if (IsKeyPressed(KEY_THREE)) {
            color_mode = 2;
            need_recreate = true;
        }
        
        // 如果需要重新生成数据（颜色模式改变）
        if (need_recreate) {
            create_data();
            need_recreate = false;
            f = 0; // 重置动画
            extend = true;
            shrink = false;
        }
        
        BeginDrawing();
        ClearBackground(BLACK);
      
        DrawTexture(textures[f], 0, 0, WHITE);
        
        // 显示当前颜色模式和操作提示
        const char* mode_names[] = {"RAINBOW", "FIRE", "OCEAN"};
        DrawText(TextFormat("Color Mode: %s", mode_names[color_mode]), 10, 10, 30, WHITE);
        DrawText("Press 1, 2, 3 to change colors", 10, 50, 20, WHITE);
        DrawText("Press ESC to exit", 10, yScreen - 30, 20, WHITE);
        
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