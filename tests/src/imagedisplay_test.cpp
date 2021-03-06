#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#include <rtac_display/Color.h>
#include <rtac_display/samples/ImageDisplay.h>
using namespace rtac::display;

std::vector<uint8_t> image_data(int width, int height)
{
    std::vector<uint8_t> res(3*width*height);
    for(int h = 0; h < height; h++) {
        for(int w = 0; w < width; w++) {
            unsigned int bw = (w + h) & 0x1;
            if(bw) {
                res[3*(width*h + w)]     = 0;
                res[3*(width*h + w) + 1] = 255;
                res[3*(width*h + w) + 2] = 255;
            }
            else {
                res[3*(width*h + w)]     = 255;
                res[3*(width*h + w) + 1] = 124;
                res[3*(width*h + w) + 2] = 0;
            }
        }
    }
    return res;
}

std::vector<Color::RGB8> image_data_rgb(int width, int height)
{
    std::vector<Color::RGB8> res(3*width*height);
    for(int h = 0; h < height; h++) {
        for(int w = 0; w < width; w++) {
            unsigned int bw = (w + h) & 0x1;
            if(bw) {
                res[width*h + w] = Color::RGB8({0, 255, 255});
            }
            else {
                res[width*h + w] = Color::RGB8({255, 124, 0});
            }
        }
    }
    return res;
}

std::vector<Color::RGBf> image_data_rgbf(int width, int height)
{
    std::vector<Color::RGBf> res(3*width*height);
    for(int h = 0; h < height; h++) {
        for(int w = 0; w < width; w++) {
            unsigned int bw = (w + h) & 0x1;
            if(bw) {
                res[width*h + w] = Color::RGBf({0, 1, 1});
            }
            else {
                res[width*h + w] = Color::RGBf({1, 0.5, 0});
            }
        }
    }
    return res;
}

int main()
{
    samples::ImageDisplay display;
    display.remove_display_flags(DrawingSurface::GAMMA_CORRECTION);
    
    unsigned int W = 4, H = 4;
    auto data0 = image_data(W,H);
    auto data1 = image_data_rgb(W,H);
    auto data2 = image_data_rgbf(W,H);

    display.set_image({W,H}, data0.data());
    display.set_image({W,H}, data1.data());
    display.set_image({W,H}, data2.data());
    GLVector<Color::RGBf> data3(data2);
    display.set_image({W,H}, data3);

    while(!display.should_close()) {
        display.draw();
        this_thread::sleep_for(10ms);
    }
    return 0;
}
