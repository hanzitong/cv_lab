#include <jni.h>
#include <android/log.h>
#include "processor_registry.h"

#include <cstring>
#include <string>

#define LOG_TAG "cv_feature_lab"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)

static ProcessorRegistry* gRegistry = nullptr;

// ProcessOutput のクラス参照とコンストラクタ ID をキャッシュ（DD-13参照）
static jclass    gOutputClass = nullptr;
static jmethodID gOutputCtor  = nullptr;

// "(I[F[F[FI[BII[F[F[F[FF)V"
// = outputType, kpX, kpY, kpSz, kpCount, ovPixels, ovW, ovH,
//   asX, asY, aeX, aeY, processingTimeMs
static constexpr char kOutputCtorSig[] =
    "(I[F[F[FI[BII[F[F[F[FF)V";

static void cacheOutputClass(JNIEnv* env) {
    jclass local = env->FindClass(
        "com/hanzitong/cvfeaturelab/jni/ProcessOutput");
    if (!local) {
        LOGE("ProcessOutput class not found — check package name");
        return;
    }
    gOutputClass = reinterpret_cast<jclass>(env->NewGlobalRef(local));
    env->DeleteLocalRef(local);

    gOutputCtor = env->GetMethodID(gOutputClass, "<init>", kOutputCtorSig);
    if (!gOutputCtor) {
        LOGE("ProcessOutput constructor not found — check signature");
    }
}

// floatArray にデータを書き込むユーティリティ
static void fillFloatArray(JNIEnv* env, jfloatArray arr, const float* data, int n) {
    if (n > 0) env->SetFloatArrayRegion(arr, 0, n, data);
}

extern "C" {

JNIEXPORT void JNICALL
Java_com_hanzitong_cvfeaturelab_jni_ProcessorJni_init(JNIEnv* env, jobject) {
    if (gRegistry) {
        LOGI("init called while already initialized — reinitializing");
        delete gRegistry;
    }
    gRegistry = new ProcessorRegistry();
    cacheOutputClass(env);
    LOGI("ProcessorRegistry initialized");
}

JNIEXPORT void JNICALL
Java_com_hanzitong_cvfeaturelab_jni_ProcessorJni_destroy(JNIEnv* env, jobject) {
    delete gRegistry;
    gRegistry = nullptr;

    if (gOutputClass) {
        env->DeleteGlobalRef(gOutputClass);
        gOutputClass = nullptr;
        gOutputCtor  = nullptr;
    }
    LOGI("ProcessorRegistry destroyed");
}

// プラグイン一覧を String[] として返す
JNIEXPORT jobjectArray JNICALL
Java_com_hanzitong_cvfeaturelab_jni_ProcessorJni_listPlugins(JNIEnv* env, jobject) {
    if (!gRegistry) return env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);

    auto names = gRegistry->listPlugins();
    jclass strClass = env->FindClass("java/lang/String");
    jobjectArray arr = env->NewObjectArray(static_cast<jsize>(names.size()), strClass, nullptr);

    for (int i = 0; i < static_cast<int>(names.size()); ++i) {
        jstring s = env->NewStringUTF(names[i].c_str());
        env->SetObjectArrayElement(arr, i, s);
        env->DeleteLocalRef(s);
    }
    env->DeleteLocalRef(strClass);
    return arr;
}

// プラグインのパラメーター定義を "key|displayName|min|max|default|isInt" 形式で返す
JNIEXPORT jobjectArray JNICALL
Java_com_hanzitong_cvfeaturelab_jni_ProcessorJni_getPluginDefs(
        JNIEnv* env, jobject, jstring jname) {
    const char* cname = env->GetStringUTFChars(jname, nullptr);
    auto defs = gRegistry ? gRegistry->getParamDefs(cname ? cname : "") : std::vector<ParamDef>{};
    if (cname) env->ReleaseStringUTFChars(jname, cname);

    jclass strClass = env->FindClass("java/lang/String");
    jobjectArray arr = env->NewObjectArray(static_cast<jsize>(defs.size()), strClass, nullptr);

    for (int i = 0; i < static_cast<int>(defs.size()); ++i) {
        const auto& d = defs[i];
        // %g 形式で不要なゼロを除去し、Kotlin の toFloat() で読める文字列を生成
        char buf[256];
        std::snprintf(buf, sizeof(buf), "%s|%s|%g|%g|%g|%d",
            d.key.c_str(), d.displayName.c_str(),
            d.min, d.max, d.defaultValue, d.isInt ? 1 : 0);
        jstring s = env->NewStringUTF(buf);
        env->SetObjectArrayElement(arr, i, s);
        env->DeleteLocalRef(s);
    }
    env->DeleteLocalRef(strClass);
    return arr;
}

JNIEXPORT void JNICALL
Java_com_hanzitong_cvfeaturelab_jni_ProcessorJni_selectPlugin(
        JNIEnv* env, jobject, jstring jname) {
    if (!gRegistry) return;
    const char* name = env->GetStringUTFChars(jname, nullptr);
    if (name) {
        gRegistry->selectPlugin(name);
        env->ReleaseStringUTFChars(jname, name);
    }
}

JNIEXPORT void JNICALL
Java_com_hanzitong_cvfeaturelab_jni_ProcessorJni_setParameter(
        JNIEnv* env, jobject, jstring jkey, jfloat value) {
    if (!gRegistry) return;
    const char* key = env->GetStringUTFChars(jkey, nullptr);
    if (key) {
        gRegistry->setParameter(key, value);
        env->ReleaseStringUTFChars(jkey, key);
    }
}

JNIEXPORT jobject JNICALL
Java_com_hanzitong_cvfeaturelab_jni_ProcessorJni_processFrame(
        JNIEnv* env, jobject,
        jbyteArray yuvBytes, jint width, jint height, jint rotationDegrees) {

    if (!gRegistry || !gOutputClass || !gOutputCtor) {
        LOGE("processFrame called before init or class cache is broken");
        return nullptr;
    }

    jbyte* yuv = env->GetByteArrayElements(yuvBytes, nullptr);
    if (!yuv) {
        LOGE("GetByteArrayElements failed");
        return nullptr;
    }

    IProcessor::Output r = gRegistry->processFrame(
        reinterpret_cast<uint8_t*>(yuv), width, height, rotationDegrees);
    env->ReleaseByteArrayElements(yuvBytes, yuv, JNI_ABORT);

    // 各フィールドを初期化（使わない型は空配列）
    jfloatArray kpX  = env->NewFloatArray(0);
    jfloatArray kpY  = env->NewFloatArray(0);
    jfloatArray kpSz = env->NewFloatArray(0);
    jbyteArray  ovPx = env->NewByteArray(0);
    jfloatArray asX  = env->NewFloatArray(0);
    jfloatArray asY  = env->NewFloatArray(0);
    jfloatArray aeX  = env->NewFloatArray(0);
    jfloatArray aeY  = env->NewFloatArray(0);
    jint kpCount = 0, ovW = 0, ovH = 0;
    jint outType = static_cast<jint>(r.outputType);

    switch (r.outputType) {
        case OutputType::KEYPOINTS: {
            int n = static_cast<int>(r.keypoints.size());
            kpCount = n;
            if (n > 0) {
                std::vector<float> xs(n), ys(n), ss(n);
                for (int i = 0; i < n; ++i) {
                    xs[i] = r.keypoints[i].pt.x;
                    ys[i] = r.keypoints[i].pt.y;
                    ss[i] = r.keypoints[i].size;
                }
                env->DeleteLocalRef(kpX);  kpX = env->NewFloatArray(n);
                env->DeleteLocalRef(kpY);  kpY = env->NewFloatArray(n);
                env->DeleteLocalRef(kpSz); kpSz = env->NewFloatArray(n);
                fillFloatArray(env, kpX,  xs.data(), n);
                fillFloatArray(env, kpY,  ys.data(), n);
                fillFloatArray(env, kpSz, ss.data(), n);
            }
            break;
        }
        case OutputType::OVERLAY_IMAGE: {
            if (!r.overlayImage.empty() && r.overlayImage.isContinuous()) {
                ovW = r.overlayImage.cols;
                ovH = r.overlayImage.rows;
                int len = ovW * ovH;
                env->DeleteLocalRef(ovPx);
                ovPx = env->NewByteArray(len);
                env->SetByteArrayRegion(ovPx, 0, len,
                    reinterpret_cast<const jbyte*>(r.overlayImage.data));
            }
            break;
        }
        case OutputType::POINT_VECTORS: {
            int n = static_cast<int>(r.arrowStarts.size());
            if (n > 0 && n == static_cast<int>(r.arrowEnds.size())) {
                std::vector<float> sxv(n), syv(n), exv(n), eyv(n);
                for (int i = 0; i < n; ++i) {
                    sxv[i] = r.arrowStarts[i].x; syv[i] = r.arrowStarts[i].y;
                    exv[i] = r.arrowEnds[i].x;   eyv[i] = r.arrowEnds[i].y;
                }
                env->DeleteLocalRef(asX); asX = env->NewFloatArray(n);
                env->DeleteLocalRef(asY); asY = env->NewFloatArray(n);
                env->DeleteLocalRef(aeX); aeX = env->NewFloatArray(n);
                env->DeleteLocalRef(aeY); aeY = env->NewFloatArray(n);
                fillFloatArray(env, asX, sxv.data(), n);
                fillFloatArray(env, asY, syv.data(), n);
                fillFloatArray(env, aeX, exv.data(), n);
                fillFloatArray(env, aeY, eyv.data(), n);
            }
            break;
        }
    }

    jobject result = env->NewObject(gOutputClass, gOutputCtor,
        outType, kpX, kpY, kpSz, kpCount,
        ovPx, ovW, ovH,
        asX, asY, aeX, aeY,
        r.processingTimeMs);

    env->DeleteLocalRef(kpX);
    env->DeleteLocalRef(kpY);
    env->DeleteLocalRef(kpSz);
    env->DeleteLocalRef(ovPx);
    env->DeleteLocalRef(asX);
    env->DeleteLocalRef(asY);
    env->DeleteLocalRef(aeX);
    env->DeleteLocalRef(aeY);

    return result;
}

} // extern "C"
