/*
 
 Written by Jared Bruni - http://github.com/lostjared
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 
 */

#ifndef __HEADER__H__
#define __HEADER__H__


#include<GL/glew.h>

namespace MX_i {
    extern int FPS;
    extern float dt;
    void Init(int *argc, char **argv, int w, int h);
    void SetCallbacks(void (*update)(), void (*idle)(), void (*resize)(int w, int h));
    void Loop();
    void Quit();
    void SwapBuffers();
    void Redisplay();
    void CreateTimer(unsigned int (*timer)(unsigned int i), int timeout);
    GLubyte *LoadBMP(const char *src, int *w, int *h);
    unsigned int GetTicks();
    enum Buttons { B_UP=0, B_DOWN, B_LEFT, B_RIGHT, B_1,B_2, B_3, B_4, B_5, B_6};
    bool PollController(enum Buttons b);
    void SetWindowTitle(const char *src);
}

#endif


