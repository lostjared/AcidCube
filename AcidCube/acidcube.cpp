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

#define ACID_CUBE_VERSION "v1.0"
#include<cstdio>
#include<cstdlib>
#include<ctime>
#include<unistd.h>
#include "MX_interface.h"
#include<acidcam/ac.h>
#include<opencv2/opencv.hpp>

void updateTexture(cv::Mat &frame, GLuint &tex);
void genTextureFromFrame(cv::Mat &frame, GLuint &tex);

// variables
GLuint logo_texture;
float spin_x = 0, spin_y = 0, spin_z = 0;
float dist = -3.0;
int axis = 0;
bool start = true, going = false;
bool start_wait = false;
int filter_mode = 0;
enum class ProgramMode { CAMERA, VIDEO };
ProgramMode prog_mode = ProgramMode::VIDEO;
float xRot = 1.0;
cv::VideoCapture cap;
cv::VideoWriter writer;
static int filter_index = 0;
std::string filename;
namespace MX_i {
    int FPS = 30;
    float dt = 0;
}
using MX_i::FPS;
using MX_i::dt;
int old_t = 0;
float intro_zPos = 0, intro_yRot = 0;
float movement_speed = 0.9f;
int cx = 0, cy = 0;
bool background = false;

GLfloat frontFace[] = {
    -1.0f, -1.0f, 1.0f, // front face
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f
};

GLfloat frontTexture[] = {
    0, 0, // front
    1, 1,
    0, 1,
    
    0, 0,
    1, 0,
    1, 1,
};

GLfloat vertices[] = { -1.0f, -1.0f, 1.0f, // front face
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    
    -1.0f, -1.0f, -1.0f, // left side
    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,
    
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    
    -1.0f, 1.0f, -1.0f, // top
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    
    -1.0f, -1.0f, -1.0f, // bottom
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    
    1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    
    1.0f, -1.0f, -1.0f, // right
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    
    -1.0f, -1.0f, -1.0f, // back face
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    
};

GLfloat texCoords[] = {
    0, 0, // front
    1, 1,
    0, 1,
    
    0, 0,
    1, 0,
    1, 1,
    
    0, 0, // left
    1, 0,
    0, 1,
    
    0, 1,
    1, 0,
    1, 1,
    
    0,0, // top
    0,1,
    1,1,
    
    1, 1,
    1, 0,
    0, 0,
    
    0, 0,// bottom
    0, 1,
    1, 1,
    
    1,1,
    1,0,
    0,0,
    
    0,0,// right
    1,0,
    0,1,
    
    0,1,
    1,0,
    1,1,
    
    0,0, // back
    1,1,
    0,1,
    
    0,0,
    1,0,
    1,1
};

unsigned int timer_callback(unsigned int t) {
    
    if(MX_i::PollController(MX_i::B_LEFT)) {
        if(filter_index > 0)
            --filter_index;
    }
    else if(MX_i::PollController(MX_i::B_RIGHT)) {
        if(filter_index < ac::draw_max-6)
            ++filter_index;
    }
    
    if(MX_i::PollController(MX_i::B_UP)) {
        if(axis > 0)
            --axis;
    }
    else if(MX_i::PollController(MX_i::B_DOWN)) {
        if(axis < 3)
            ++axis;
    }
    
    if(MX_i::PollController(MX_i::B_2)) {
        filter_mode = (filter_mode == 0) ? 1 : 0;
    }
    return t;
}

void idle() {
    
    if(start_wait == false && MX_i::PollController(MX_i::B_1)) {
        start_wait = true;
        MX_i::SetWindowTitle("Acid Cube");
    }
    
    if(start_wait == true && start == true) {
        
        spin_x += movement_speed * dt;
        dist -= 0.01f * dt;
        if(dist <= -20.0f) {
            going = true;
            start = false;
        }
    }
    else if(going == true) {
        dist += 0.0009 * dt;
        spin_x -= movement_speed * dt;
        if(dist > 1.0f) {
            going = false;
            start = true;
        }
    }
    
    int t = MX_i::GetTicks();
    int passed = t - old_t;
    old_t = t ;
    dt = passed;
    MX_i::Redisplay();
}

void renderIntro() {
    
    if(start_wait == false) return;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    cv::Mat frame;
    bool frame_read = cap.read(frame);
    if(frame_read == false && prog_mode == ProgramMode::VIDEO) {
        cap.set(CV_CAP_PROP_POS_FRAMES,1);
        frame_read = cap.read(frame);
    }
    if(frame_read == false && prog_mode == ProgramMode::CAMERA) {
        std::cerr << "Could not read frame quitting...\n";
        exit(EXIT_FAILURE);
    }
    
    if(frame_read) {
        cv::Mat outframe;
        cv::flip(frame,outframe, 0);
        frame = outframe.clone();
        ac::DrawFilter(filter_index, frame);
        updateTexture(frame, logo_texture);
        if(background == true) {
            glBindTexture(GL_TEXTURE_2D, logo_texture);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, frontFace);
            glTexCoordPointer(2, GL_FLOAT, 0, frontTexture);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0f, (float)(width/height), 0.1f, 300.0f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        
        if(filter_mode == 0) {
            glTranslatef(0.0f, 0.0f, dist);
            switch(axis) {
                case 0:
                    glRotatef(spin_x, 0.0f, 0.0f, 1.0f);
                    break;
                case 1:
                    glRotatef(spin_x, 1.0f, 0.0f, 0.0f);
                    break;
                case 2:
                    glRotatef(spin_x, 1.0f, 1.0f, 0.0f);
                    break;
                case 3:
                    glRotatef(spin_x, 1.0f, 1.0f, 1.0f);
                    break;
            }
        }
        else if(filter_mode == 1) {
            glTranslatef(0, 0, -4.0);
            glRotatef(intro_yRot, 0, 1, 0);
            intro_yRot += 0.1f * dt;
            if(intro_yRot > 360) {
                intro_yRot = 1;
            } else if(intro_yRot > 180) {
                static float xRot = 0.0f;
                xRot += 0.1f * dt;
                glRotatef(xRot, 1, 0, 0);
            }
        }
        glVertexPointer(3, GL_FLOAT, 0, vertices);
        glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
        glBindTexture(GL_TEXTURE_2D, logo_texture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //	glLoadIdentity();
    glPushMatrix();
    renderIntro();
    glPopMatrix();
    MX_i::SwapBuffers();
    
    if(start_wait == true && writer.isOpened()) {
        cv::Mat img;
        img.create(cy,cx,CV_8UC3);
        glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3) ? 1 : 4);
        glPixelStorei(GL_PACK_ROW_LENGTH, img.step/img.elemSize());
        glReadPixels(0, 0, img.cols, img.rows, GL_BGR, GL_UNSIGNED_BYTE, img.data);
        cv::Mat flipped;
        cv::flip(img, flipped, 0);
        writer.write(flipped);
    }
}

void resize(int w, int h) {
    if(w <= 0 || h <= 0) w = 1, h = 1;
    width = w, height = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, float(w / h), 0.1f, 300.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void genTextureFromFrame(cv::Mat &frame, GLuint &tex) {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.cols, frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, frame.ptr());
}

void updateTexture(cv::Mat &frame, GLuint &tex) {
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.cols, frame.rows, GL_BGR, GL_UNSIGNED_BYTE, frame.ptr());
}

void  init() {
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    resize(width, height);
    srand((unsigned int)time(0));
    glEnable(GL_TEXTURE_2D);
    cv::Mat frame;
    if(cap.read(frame)) {
        genTextureFromFrame(frame, logo_texture);
    }
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    MX_i::CreateTimer(timer_callback, 150);
}

void clean() {
    glDeleteTextures(1, &logo_texture);
}

void output_info(std::string name) {
    std::cout << name << " " << ACID_CUBE_VERSION << " arguments:\n-l list files\n-w window width\n-h window height\n-W resized video width\n-H resized video height\n-f starting filter index\n-i filename\n-c web camera mode\n-v video file mode\n-S rotation mode spin\n-C rotation mode cube\n-s rotation speed\n-u subfilter\n-b draw background\n";
}

int main(int argc, char **argv) {
    int opt = 0;
    int v_w = 640;
    int v_h = 480;
    ac::fill_filter_map();
    std::string fval;
    if(argc > 1) {
        while((opt = getopt(argc, argv, "lW:H:w:h:f:i:vcCSs:u:o:b")) != -1) {
            switch(opt) {
                case 'o':
                    fval = optarg;
                    if(fval.find(".mov") == std::string::npos) {
                        fval += ".mov";
                    }
                    break;
                case 'b':
                    background = true;
                    break;
                case 'l':
                    for(int i = 0; i < ac::draw_max-5; ++i)
                        std::cout << std::setw(3) << i << "\t " << ac::draw_strings[i] << "\n";
                    exit(EXIT_SUCCESS);
                    break;
                case 'w':
                    cx = atoi(optarg);
                    if(cx < 128) {
                        std::cerr << "Error invalid width..\n";
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'h':
                    cy = atoi(optarg);
                    if(cy < 128) {
                        std::cerr << "Error invalid width..\n";
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'f':
                    filter_index = atoi(optarg);
                    if(filter_index < 0 || filter_index > ac::draw_max-6) {
                        std::cerr << "Error invalid filter number...\n";
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 's':
                    movement_speed = atof(optarg);
                    if(movement_speed == 0) {
                        std::cerr << "movement speed cannot be zero..\n";
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'i':
                    filename = optarg;
                    break;
                case 'c':
                    prog_mode = ProgramMode::CAMERA;
                    break;
                case 'v':
                    prog_mode = ProgramMode::VIDEO;
                    break;
                case 'S':
                    filter_mode = 0;
                    break;
                case 'C':
                    filter_mode = 1;
                    break;
                case 'W':
                    v_w = atoi(optarg);
                    break;
                case 'H':
                    v_h = atoi(optarg);
                    break;
                case 'u': {
                    int value = atoi(optarg);
                    if(value >= 0 && value < ac::draw_max-5)
                        ac::setSubFilter(value);
                    else {
                        std::cerr << "Error subfilter out of range.\n";
                        exit(EXIT_FAILURE);
                    }
                }
                    break;
            }
        }
        if(prog_mode == ProgramMode::CAMERA) {
            cap.open(0);
            if(!cap.isOpened()) {
                std::cerr << "Could not open webcam 0\n";
                exit(EXIT_FAILURE);
            }
        } else if(prog_mode == ProgramMode::VIDEO) {
            cap.open(filename);
            if(!cap.isOpened()) {
                std::cerr << "Error could not load video: " << filename << " ...\n";
                exit(EXIT_FAILURE);
            }
        }
    }
    else {
        output_info(argv[0]);
        exit(EXIT_SUCCESS);
    }
    cap.set(CV_CAP_PROP_FRAME_WIDTH, v_w);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, v_h);
    if(fval.length()>0) {
        writer.open(fval, CV_FOURCC('m', 'p', '4', 'v'), 60, cv::Size(cx, cy), true);
        if(!writer.isOpened()) {
            std::cerr << "Could not open file: " << fval << "\n";
            exit(EXIT_FAILURE);
        }
    }
    
    if(cx <= 0 || cy <= 0 ) {
        std::cerr << "Error must pass valid resolution\n";
        return -1;
    }
    MX_i::Init(&argc, argv, cx, cy);
    MX_i::SetCallbacks(render, idle, resize);
    MX_i::SetWindowTitle("Acid Cube - Press the A key to Start");
    init();
    MX_i::Loop();
    clean();
    MX_i::Quit();
    return 0;
}
