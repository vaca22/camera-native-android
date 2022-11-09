#include <jni.h>
#include <cstring>
#include <utils/native_debug.h>
#include "camera_manager.h"
#include "camera_engine.h"


CameraAppEngine *pEngineObj = nullptr;

extern "C" JNIEXPORT jlong JNICALL
Java_com_sample_textureview_ViewActivity_createCamera(JNIEnv *env,
                                                      jobject instance,
                                                      jint width, jint height) {
  pEngineObj = new CameraAppEngine(env, instance, width, height);
  return reinterpret_cast<jlong>(pEngineObj);
}


extern "C" JNIEXPORT void JNICALL
Java_com_sample_textureview_ViewActivity_deleteCamera(JNIEnv *env,
                                                      jobject instance,
                                                      jlong ndkCameraObj) {
  if (!pEngineObj || !ndkCameraObj) {
    return;
  }
  CameraAppEngine *pApp = reinterpret_cast<CameraAppEngine *>(ndkCameraObj);
  ASSERT(pApp == pEngineObj, "NdkCamera Obj mismatch");

  delete pApp;

  // also reset the private global object
  pEngineObj = nullptr;
}


extern "C" JNIEXPORT jobject JNICALL
Java_com_sample_textureview_ViewActivity_getMinimumCompatiblePreviewSize(
    JNIEnv *env, jobject instance, jlong ndkCameraObj) {
  if (!ndkCameraObj) {
    return nullptr;
  }
  CameraAppEngine *pApp = reinterpret_cast<CameraAppEngine *>(ndkCameraObj);
  jclass cls = env->FindClass("android/util/Size");
  jobject previewSize =
      env->NewObject(cls, env->GetMethodID(cls, "<init>", "(II)V"),
                     pApp->GetCompatibleCameraRes().width,
                     pApp->GetCompatibleCameraRes().height);
  return previewSize;
}


extern "C" JNIEXPORT jint JNICALL
Java_com_sample_textureview_ViewActivity_getCameraSensorOrientation(
    JNIEnv *env, jobject instance, jlong ndkCameraObj) {
  ASSERT(ndkCameraObj, "NativeObject should not be null Pointer");
  CameraAppEngine *pApp = reinterpret_cast<CameraAppEngine *>(ndkCameraObj);
  return pApp->GetCameraSensorOrientation(ACAMERA_LENS_FACING_BACK);
}


extern "C" JNIEXPORT void JNICALL
Java_com_sample_textureview_ViewActivity_onPreviewSurfaceCreated(
    JNIEnv *env, jobject instance, jlong ndkCameraObj, jobject surface) {
  ASSERT(ndkCameraObj && (jlong)pEngineObj == ndkCameraObj,
         "NativeObject should not be null Pointer");
  CameraAppEngine *pApp = reinterpret_cast<CameraAppEngine *>(ndkCameraObj);
  pApp->CreateCameraSession(surface);
  pApp->StartPreview(true);
}


extern "C" JNIEXPORT void JNICALL
Java_com_sample_textureview_ViewActivity_onPreviewSurfaceDestroyed(
    JNIEnv *env, jobject instance, jlong ndkCameraObj, jobject surface) {
  CameraAppEngine *pApp = reinterpret_cast<CameraAppEngine *>(ndkCameraObj);
  ASSERT(ndkCameraObj && pEngineObj == pApp,
         "NativeObject should not be null Pointer");
  jclass cls = env->FindClass("android/view/Surface");
  jmethodID toString =
      env->GetMethodID(cls, "toString", "()Ljava/lang/String;");

  jstring destroyObjStr =
      reinterpret_cast<jstring>(env->CallObjectMethod(surface, toString));
  const char *destroyObjName = env->GetStringUTFChars(destroyObjStr, nullptr);

  jstring appObjStr = reinterpret_cast<jstring>(
      env->CallObjectMethod(pApp->GetSurfaceObject(), toString));
  const char *appObjName = env->GetStringUTFChars(appObjStr, nullptr);

  ASSERT(!strcmp(destroyObjName, appObjName), "object Name MisMatch");

  env->ReleaseStringUTFChars(destroyObjStr, destroyObjName);
  env->ReleaseStringUTFChars(appObjStr, appObjName);

  pApp->StartPreview(false);
}
