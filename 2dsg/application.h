#ifndef APPLICATION_H
#define APPLICATION_H

#include "touch.h"
#include <set>
#include "orientation.h"
#include "texturemanager.h"
#include <math.h>
#include "ticker.h"
#include <vector>
#include <string>
#include <ginput.h>
#include <timercontainer.h>
#include <refptr.h>
#include "Matrices.h"
#include <functional>


class Font;
class Stage;
class Event;

class Application
{
public:
	Application();
	~Application();

	Stage* stage() const;
	
	void setOrientation(Orientation orientation);
	Orientation orientation() const;

	void setHardwareOrientation(Orientation orientation);
	Orientation hardwareOrientation() const;

    void setDeviceOrientation(Orientation orientation);
    Orientation getDeviceOrientation() const;
	
	void setResolution(int width, int height,bool keepBuffers=false);
	void resolution(int* width, int* height);

	void initView();
	void releaseView();

	void enterFrame();
	void clearBuffers();
	void renderScene(int deltaFrameCount = -1,float *vmat=NULL,float *pmat=NULL,const std::function<void(ShaderEngine *,Matrix4 &)> &preStage=nullptr);
    bool onDemandDraw(bool &now);

    void mouseDown(int x, int y, int button, int modifiers, int type);
    void mouseUp(int x, int y, int button, int modifiers, int type);
    void mouseMove(int x, int y, int button, int modifiers, int type);
    void mouseHover(int x, int y, int button, int modifiers, int type);
	void mouseWheel(int x, int y, int wheel, int modifiers);
    void mouseEnter(int x, int y, int buttons, int modifiers);
    void mouseLeave(int x, int y, int modifiers);

    void touchesBegin(ginput_TouchEvent *event);
    void touchesMove(ginput_TouchEvent *event);
    void touchesEnd(ginput_TouchEvent *event);
    void touchesCancel(ginput_TouchEvent *event);

    void keyDown(int keyCode, int realCode, int modifiers);
    void keyUp(int keyCode, int realCode, int modifiers);
    void keyChar(const char *code);

	void setClearColorBuffer(bool b)
	{
		clearColorBuffer_ = b;
	}

	bool getClearColorBuffer() const
	{
		return clearColorBuffer_;
	}

	void broadcastEvent(Event* event);

	Font* getDefaultFont();

    void setScale(float scale)
	{
		scale_ = scale;
	}

    float getScale()
	{
		return scale_?scale_:1;;
	}

	void setLogicalDimensions(int width, int height);

    void setLogicalScaleMode(LogicalScaleMode mode);
    LogicalScaleMode getLogicalScaleMode() const;

	int getLogicalWidth() const;
	int getLogicalHeight() const;
	int getHardwareWidth() const;
	int getHardwareHeight() const;

    void enableOnDemandDraw(bool onDemand)
    {
        onDemandDraw_ = onDemand;
    }

	void setImageScales(const std::vector<std::pair<std::string, float> >& imageScales);
	const std::vector<std::pair<std::string, float> >& getImageScales() const;

    const char *getImageSuffix(float *scale) const;

    const char *getImageSuffix(const char *file, float *scale) const;

	void addTicker(Ticker* ticker);
	void removeTicker(Ticker* ticker);

	void setBackgroundColor(float r, float g, float b, float a=1.f);
	void getBackgroundColor(float* r, float* g, float* b, float* a=NULL);

	float getLogicalTranslateX() const;
	float getLogicalTranslateY() const;
	float getLogicalScaleX() const;
	float getLogicalScaleY() const;

	TextureManager* getTextureManager()
	{
		return &textureManager_;
	}

    TimerContainer *getTimerContainer()
    {
        return &timerContainer_;
    }

    void *createAutounrefPool();
    void deleteAutounrefPool(void *);
    void autounref(GReferenced *referenced);

    void configureFrustum(float fov,float farplane,float nearplane);

private:
	TextureManager textureManager_;
	Stage* stage_;
	Orientation orientation_;
	Orientation hardwareOrientation_;
	Orientation deviceOrientation_;
	int width_, height_;

	int logicalWidth_, logicalHeight_;
	LogicalScaleMode scaleMode_;
	float logicalScaleX_, logicalScaleY_;
	float logicalTranslateX_, logicalTranslateY_;
	float fov_;
	float farplane_;
	float nearplane_;
	bool projectionDirty_;
	Matrix4 projectionMatrix_,vpProjectionMatrix_;

	void calculateLogicalTransformation();
	void correctTouchPositionLogical(int* x, int* y);

	void correctTouchPosition(int* x, int* y);
	void correctTouchPositionHardware(int* x, int *y);
    void correctTouchPositions(ginput_TouchEvent *event);

	int nframe_;
	double time_;
	double lastFrameRenderTime_;
    bool onDemandDraw_;

	bool clearColorBuffer_;

	Font* defaultFont_;

    float scale_;

    std::vector<std::pair<std::string, float> > imageScales_;

    struct ImageScale
    {
        ImageScale(const char *suffix, float scale) :
            suffix(suffix),
            scale(scale),
            midscale(0)
        {
        }

        const char *suffix;
        float scale;
        float midscale;

        bool operator < (const ImageScale &other) const
        {
            return scale > other.scale;
        }
    };

    std::vector<ImageScale> imageScales2_;

	std::set<Ticker*> tickers_;
	bool tickersIteratorInvalid_;

	float backr_, backg_, backb_, backa_;

    TimerContainer timerContainer_;

    std::vector<std::vector<GReferenced*>*> unrefPool_;
    std::vector<std::vector<GReferenced*>*> unrefPoolTrash_;
};


#endif
