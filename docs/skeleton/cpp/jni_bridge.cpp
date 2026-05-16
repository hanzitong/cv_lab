#include <jni.h>
#include <android/log.h>
#include "feature_processor.h"

#define LOG_TAG "cv_feature_lab"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)

static FeatureProcessor* gProcessor = nullptr;

// ProcessResult のクラス参照とコンストラクタ ID をキャッシュする
// FindClass / GetMethodID は JNI 呼び出しの中でも高コストなため
static jclass        gResultClass = nullptr;
static jmethodID     gResultCtor  = nullptr;

// キャッシュ初期化（init から呼ぶ）
static void cacheResultClass(JNIEnv* env) {
    jclass local = env->FindClass("com/hanzitong/cvfeaturelab/jni/ProcessResult");
    if (!local) {
        LOGE("ProcessResult class not found — check package name");
        return;
    }
    gResultClass = reinterpret_cast<jclass>(env->NewGlobalRef(local));
    env->DeleteLocalRef(local);

    // コンストラクタシグネチャ: ProcessResult(FloatArray, FloatArray, FloatArray, Int, Float)
    gResultCtor = env->GetMethodID(gResultClass, "<init>", "([F[F[FIF)V");
    if (!gResultCtor) {
        LOGE("ProcessResult constructor not found — check signature");
    }
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_hanzitong_cvfeaturelab_jni_FeatureProcessorJni_init(JNIEnv* env, jobject) {
    if (gProcessor) {
        LOGI("init called while already initialized — reinitializing");
        delete gProcessor;
    }
    gProcessor = new FeatureProcessor();
    cacheResultClass(env);
    LOGI("FeatureProcessor initialized");
}

JNIEXPORT void JNICALL
Java_com_hanzitong_cvfeaturelab_jni_FeatureProcessorJni_destroy(JNIEnv* env, jobject) {
    delete gProcessor;
    gProcessor = nullptr;

    if (gResultClass) {
        env->DeleteGlobalRef(gResultClass);
        gResultClass = nullptr;
        gResultCtor  = nullptr;
    }
    LOGI("FeatureProcessor destroyed");
}

JNIEXPORT void JNICALL
Java_com_hanzitong_cvfeaturelab_jni_FeatureProcessorJni_setAlgorithm(
        JNIEnv*, jobject, jint type) {
    if (gProcessor)
        gProcessor->setAlgorithm(static_cast<AlgorithmType>(type));
}

JNIEXPORT void JNICALL
Java_com_hanzitong_cvfeaturelab_jni_FeatureProcessorJni_setParameter(
        JNIEnv* env, jobject, jstring key, jfloat value) {
    if (!gProcessor) return;
    const char* k = env->GetStringUTFChars(key, nullptr);
    if (k) {
        gProcessor->setParameter(k, value);
        env->ReleaseStringUTFChars(key, k);
    }
}

JNIEXPORT jobject JNICALL
Java_com_hanzitong_cvfeaturelab_jni_FeatureProcessorJni_processFrame(
        JNIEnv* env, jobject,
        jbyteArray yuvBytes, jint width, jint height, jint rotationDegrees) {

    if (!gProcessor || !gResultClass || !gResultCtor) {
        LOGE("processFrame called before init or class cache is broken");
        return nullptr;
    }

    jbyte* yuv = env->GetByteArrayElements(yuvBytes, nullptr);
    if (!yuv) {
        LOGE("GetByteArrayElements failed");
        return nullptr;
    }

    ProcessResult r = gProcessor->processFrame(
        reinterpret_cast<uint8_t*>(yuv), width, height, rotationDegrees);
    env->ReleaseByteArrayElements(yuvBytes, yuv, JNI_ABORT);

    int count = static_cast<int>(r.keypoints.size());

    jfloatArray xArr = env->NewFloatArray(count);
    jfloatArray yArr = env->NewFloatArray(count);
    jfloatArray sArr = env->NewFloatArray(count);

    if (!xArr || !yArr || !sArr) {
        LOGE("Failed to allocate float arrays for %d keypoints", count);
        if (xArr) env->DeleteLocalRef(xArr);
        if (yArr) env->DeleteLocalRef(yArr);
        if (sArr) env->DeleteLocalRef(sArr);
        return nullptr;
    }

    if (count > 0) {
        std::vector<float> xs(count), ys(count), ss(count);
        for (int i = 0; i < count; i++) {
            xs[i] = r.keypoints[i].pt.x;
            ys[i] = r.keypoints[i].pt.y;
            ss[i] = r.keypoints[i].size;
        }
        env->SetFloatArrayRegion(xArr, 0, count, xs.data());
        env->SetFloatArrayRegion(yArr, 0, count, ys.data());
        env->SetFloatArrayRegion(sArr, 0, count, ss.data());
    }

    return env->NewObject(gResultClass, gResultCtor,
        xArr, yArr, sArr,
        static_cast<jint>(count),
        r.processingTimeMs);
}

} // extern "C"
