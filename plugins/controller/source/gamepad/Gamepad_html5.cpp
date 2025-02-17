#include <gamepad/Gamepad.h>
#include <gamepad/Gamepad_private.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <emscripten.h>
#include "glog.h"

#include "cJSON.h"
extern "C" cJSON *JSCall(const char *mtd, cJSON *args);
extern "C" void JSCallV(const char *mtd, cJSON *args);

struct Gamepad_devicePrivate {
    int joystickID;
    bool connected;
    std::string name;
    Gamepad_devicePrivate() : name("Unknown"),joystickID(0),connected(false) { };
};

static struct Gamepad_device ** devices = NULL;
static unsigned int numDevices = 0;
static unsigned int nextDeviceID = 0;

static bool inited = false;

void Gamepad_init() {
    if (!inited) {
        inited = true;
    }
    Gamepad_processEvents();
}

static void disposeDevice(struct Gamepad_device * deviceRecord) {
    delete (struct Gamepad_device *)(deviceRecord->privateData);
    free(deviceRecord->axisStates);
    free(deviceRecord->buttonStates);

    free(deviceRecord);
}

void Gamepad_shutdown() {
    unsigned int deviceIndex;

    if (inited) {
        for (deviceIndex = 0; deviceIndex < numDevices; deviceIndex++) {
            disposeDevice(devices[deviceIndex]);
        }
        free(devices);
        devices = NULL;
        numDevices = 0;
        inited = false;
    }
}

unsigned int Gamepad_numDevices() {
    return numDevices;
}

struct Gamepad_device * Gamepad_deviceAtIndex(unsigned int deviceIndex) {
    if (deviceIndex >= numDevices) {
        return NULL;
    }
    return devices[deviceIndex];
}


void Gamepad_detectDevices() {
}

static void handleAxisChange(struct Gamepad_device * device, int axisIndex, float value,double time) {
    float lastValue;

    if (axisIndex < 0 || axisIndex >= (int) device->numAxes) {
        return;
    }

    lastValue = device->axisStates[axisIndex];
    device->axisStates[axisIndex] = value;
    if ((Gamepad_axisMoveCallback != NULL)&&(lastValue!=value)) {
        Gamepad_axisMoveCallback(device, axisIndex, value, lastValue, time, Gamepad_axisMoveContext);
    }
}

static void handleButtonChange(struct Gamepad_device * device, unsigned int buttonIndex, bool down,double time) {
    if (device->buttonStates[buttonIndex] != down) {
        device->buttonStates[buttonIndex] = down;
        if (down && Gamepad_buttonDownCallback != NULL) {
            Gamepad_buttonDownCallback(device, buttonIndex, time, Gamepad_buttonDownContext);
        }
        else if (!down && Gamepad_buttonUpCallback != NULL) {
            Gamepad_buttonUpCallback(device, buttonIndex, time, Gamepad_buttonUpContext);
        }
    }
}


void Gamepad_processEvents() {
    static int inProcessEvents=0;

    unsigned int numPadsSupported;
    unsigned int deviceIndex, deviceIndex2;
    int duplicate;
    struct Gamepad_device * deviceRecord;
    struct Gamepad_devicePrivate * deviceRecordPrivate;
    int axisIndex;

    if (!inited || inProcessEvents) {
        return;
    }

    inProcessEvents = 1;

    cJSON *res=JSCall("GidController_getGamepads",NULL);
    if (res!=NULL) {
		numPadsSupported = cJSON_GetArraySize(res);
		for (deviceIndex = 0; deviceIndex < numPadsSupported; deviceIndex++) {
			cJSON *item=cJSON_GetArrayItem(res,deviceIndex);
			if (item) {
				cJSON *indexp=cJSON_GetObjectItem(item,"index");
				int index=0;
				if (indexp)
					index=(int)(indexp->valuedouble);
				else
					continue;
				duplicate = -1;
				for (deviceIndex2 = 0; deviceIndex2 < numDevices; deviceIndex2++) {
					if (((struct Gamepad_devicePrivate *) devices[deviceIndex2]->privateData)->joystickID == index) {
						duplicate = deviceIndex2;
						break;
					}
				}
				cJSON *axesp=cJSON_GetObjectItem(item,"axes");
				cJSON *buttonsp=cJSON_GetObjectItem(item,"buttons");
				cJSON *idp=cJSON_GetObjectItem(item,"id");
				cJSON *connectedp=cJSON_GetObjectItem(item,"connected");
				cJSON *timestampp=cJSON_GetObjectItem(item,"timestamp");
				if ((!axesp)||(!buttonsp)||(!timestampp)) continue;
				bool connected=connectedp?connectedp->valueint:0;
				if (duplicate==-1) {
					deviceRecordPrivate = new Gamepad_devicePrivate();
					deviceRecordPrivate->joystickID = index;
					const char *name=idp?cJSON_GetStringValue(idp):NULL;
					if (name)
						deviceRecordPrivate->name = std::string(name);
					deviceRecordPrivate->connected=false;

					deviceRecord = (struct Gamepad_device *)malloc(sizeof(struct Gamepad_device));
					deviceRecord->deviceID = nextDeviceID++;
					deviceRecord->description = deviceRecordPrivate->name.c_str();
					deviceRecord->vendorID = 0;
					deviceRecord->productID = 0;
					deviceRecord->numAxes = cJSON_GetArraySize(axesp)+2; //Include Trigger
					deviceRecord->numButtons = cJSON_GetArraySize(buttonsp);
					deviceRecord->axisStates = (float *)calloc(sizeof(float), deviceRecord->numAxes);
					deviceRecord->buttonStates = (bool *) calloc(sizeof(bool), deviceRecord->numButtons);
					devices = (struct Gamepad_device **)realloc(devices, sizeof(struct Gamepad_device *) * (numDevices + 1));
					devices[numDevices++] = deviceRecord;


					deviceRecord->privateData = deviceRecordPrivate;
				}
				else {
					deviceRecord=devices[duplicate];
					deviceRecordPrivate = (struct Gamepad_devicePrivate *)deviceRecord->privateData;
				}


				if ((Gamepad_deviceAttachCallback != NULL)&&connected&&(!deviceRecordPrivate->connected)) {
					deviceRecordPrivate->connected=true;
					Gamepad_deviceAttachCallback(deviceRecord, Gamepad_deviceAttachContext);
				}

				double time=timestampp->valuedouble;
				//Process axes
		        for (unsigned int i = 0; i < cJSON_GetArraySize(axesp); i++) {
					cJSON *val=cJSON_GetArrayItem(axesp,i);
					if (val)
						handleAxisChange(deviceRecord, i, val->valuedouble,time);
		        }
				//Process buttons
		        for (unsigned int i = 0; i < cJSON_GetArraySize(buttonsp); i++) {
					cJSON *val=cJSON_GetArrayItem(buttonsp,i);
					cJSON *val2=val?cJSON_GetObjectItem(val,"pressed"):NULL;
					if (val2)
						handleButtonChange(deviceRecord, i, val2->valueint,time);
					if ((i==6)||(i==7)) { //Triggers
						cJSON *val2=val?cJSON_GetObjectItem(val,"value"):NULL;
						if (val2)
							handleAxisChange(deviceRecord, i-2, val2->valuedouble*2-1,time);
					}
		        }

				if ((Gamepad_deviceRemoveCallback != NULL)&&(!connected)&&(deviceRecordPrivate->connected)) {
					deviceRecordPrivate->connected=false;
					Gamepad_deviceRemoveCallback(deviceRecord, Gamepad_deviceRemoveContext);
				}

			}
		}
		cJSON_Delete(res);
    }

    inProcessEvents = 0;
}
