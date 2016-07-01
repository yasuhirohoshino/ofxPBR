#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {
    ofGLWindowSettings settings;
    settings.setGLVersion(4, 0);
    settings.width = 1280;
    settings.height = 720;
    ofCreateWindow(settings);
    ofRunApp(new ofApp());
}