LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE           := Plugin_Interface
LOCAL_SRC_FILES        := Plugin_Interface.cpp \
                          Interface.cpp \
                          Interface_Manager.cpp \
                          Interface_Thread.cpp
LOCAL_LDLIBS           := -landroid -lEGL -lGLESv1_CM -lOpenSLES
LOCAL_STATIC_LIBRARIES := MMDAgent MMDFiles Bullet_Physics JPEG libpng zlib GLFW GLee hts_engine_API Interface PortAudio
LOCAL_C_INCLUDES       := $(LOCAL_PATH)/../Library_MMDAgent/include \
                          $(LOCAL_PATH)/../Library_JPEG/include \
                          $(LOCAL_PATH)/../Library_Bullet_Physics/include \
                          $(LOCAL_PATH)/../Library_GLee/include \
                          $(LOCAL_PATH)/../Library_libpng/include \
                          $(LOCAL_PATH)/../Library_zlib/include \
                          $(LOCAL_PATH)/../Library_MMDFiles/include \
                          $(LOCAL_PATH)/../Library_hts_engine_API/include \
                          $(LOCAL_PATH)/../Library_Interface/include \
                          $(LOCAL_PATH)/../Library_GLFW/include
LOCAL_CFLAGS           += -DMMDAGENT

include $(BUILD_SHARED_LIBRARY)
