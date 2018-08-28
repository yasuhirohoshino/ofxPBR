#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {
    ofGLWindowSettings settings;
    settings.setGLVersion(4, 1);
	settings.setSize(1280, 720);
    ofCreateWindow(settings);
    ofRunApp(new ofApp());
}