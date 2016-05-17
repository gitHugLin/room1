LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

//OPENCV_CAMERA_MODULES:=on
OPENCV_INSTALL_MODULES:=on
//OPENCV_LIB_TYPE := SHARED -lm -ljnigraphics -lui
include /opt/android-opencv-sdk/OpenCV-android-sdk/sdk/native/jni/OpenCV.mk

LOCAL_MODULE := AddPictureLib
LOCAL_LDFLAGS := -Wl,--build-id
LOCAL_LDLIBS := -llog -landroid -lGLESv2 -lEGL -lcutils -lui -lutils
LOCAL_CFLAGS :=  -DEGL_EGLEXT_PROTOTYPES -DGL_GLEXT_PROTOTYPES

LOCAL_SRC_FILES := \
	myjni.cpp \
	src/format.cpp \
	src/MyThread.cpp \
	src/OrbPatch.cpp \
	src/MutGetHomography.cpp \
	src/GetHomography.cpp \
	src/PerspectiveAdd.cpp \
	Application.mk \

LOCAL_C_INCLUDE := /opt/android-opencv-sdk/OpenCV-android-sdk/sdk/native/jni/include
LOCAL_C_INCLUDES += include
LOCAL_C_INCLUDES += ./

include $(BUILD_SHARED_LIBRARY)
