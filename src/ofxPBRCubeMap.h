#pragma once
#include "ofMain.h"
#include "shaders/importanceSampling.h"

class ofxPBRCubeMap{
private:
    ofCamera envCam[6];
    ofFbo envFbo[6];
    ofFbo cacheEnvFbo;
    ofTexture envTexture;
    
    template<typename T>
    struct Texture{
        T rawTexture;
        T cubeMapFaces[6];
        vector<T> filteredCubeMapFaces[6];
        T cacheTexture;
    };
    
    Texture<ofImage> iTex;
    Texture<ofFloatImage> fTex;
    
    ofShader shader;
    ImportanceSampling importanceSampling;
    ofMesh sphereMesh, envSphereMesh;
    
    unsigned int cubeMapID;
    unsigned int filteredCubeMapID;
    int textureUnit;
    int baseSize;
    int textureFormat;
    int maxMipLevel;
    
    int cacheWidth, cacheHeight;
    ofFbo cacheFbo;
    
	bool bIsAllocated = false;
    
    float rotation = 0.0;
    float exposure = 1.0;
    float envLevel = 0.0;

	void loadShaders();
    void loadImage(string imagePath);
    void generate();
    void makeRawCubeMap();
    void makeFilteredCubeMap();
    void makeCache(string cachePath);
    void makeCubeMapFaces(int width, int height,
                          ofPixels& px, ofPixels& py, ofPixels& pz,
                          ofPixels& nx, ofPixels& ny, ofPixels& nz,
                          int index = 0);
    void makeCubeMapFaces(int width, int height,
                          ofFloatPixels& px, ofFloatPixels& py, ofFloatPixels& pz,
                          ofFloatPixels& nx, ofFloatPixels& ny, ofFloatPixels& nz,
                          int index = 0);
    bool isHDRImagePath(string path);
    
public:
    ofxPBRCubeMap();
    
    void load(ofImage * sphereMapImage, int baseSize = 512);
    void load(ofFloatImage * sphereMapImage, int baseSize = 512);
    void load(string imagePath, int baseSize = 512, bool useCache = false, string cacheDirectry = "");
    void loadFromCache(string cachePath);
    void bind(int pos);
    void unbind();
    bool isHDR();
	bool isAllocated();
    int getNumMips();
	ofTexture* getPanoramaTexture();
	ofFloatColor getColor(int x, int y);
    template<typename T>
    T* getPanorama();

	void setRotation(float rotation) { this->rotation = rotation; };
	float getRotation() { return rotation; };
	void setExposure(float exposure) { this->exposure = exposure; };
	float getExposure() { return exposure; };
	void setEnvLevel(float level) { this->envLevel = level; };
	float getEnvLevel() { return envLevel; };
};