//
// Created by root on 18-6-12.
//
#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <sys/system_properties.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>
#include "property.h"
#include <sched.h>



#define LOG_TAG "lianghuan"


#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,__VA_ARGS__);
#ifdef __cplusplus
extern "C" {
#endif

#include <libavutil/log.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libavcodec/jni.h>
#include <sys/time.h>

static int init_flag = 0;
const char* app_sign = "636E732DDB59367033DA52F1103FE04AA3E7FD05";
const char hexcode[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

static JavaVM* javaVM;
static pthread_mutex_t mMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t mCond = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t mPauseMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t mPauseCond = PTHREAD_MUTEX_INITIALIZER;

static jobject yuv_callback;
static jmethodID callback_method;
static char* flv_path;
static volatile int run_flag;
static volatile int pause_flag;
pthread_t pthread[2];

static timeval time_pre_decode;
static timeval time_after_decode;

void flv2yuv(char*);
void *decode_thread(void*);
void *notify_thread(void*);
unsigned char yuv_data[460800];
int32_t effort_timestamp;
static unsigned char* extradata;
static unsigned int extradata_size;

void set_flv(JNIEnv* env, jclass clazz, jstring jflv_path){

//    char* key_c = "ro.build.id";
//
//    char value[PROPERTY_VALUE_MAX];
//    __system_property_get(key_c, value);
//    if (strcmp(value,"E077")){
////        return;
//    } else{
//        LOGE("lianghuan my product, pass");
//    }

    if (flv_path != NULL){
        delete flv_path;
    }

    if (jflv_path == NULL){
        LOGE("lianghuan","jflv_path = null");
        return;
    }

    const jsize len = env->GetStringUTFLength(jflv_path);
    char* path_c = static_cast<char *>(malloc(len + 1));
    env->GetStringUTFRegion(jflv_path,0,len,path_c);
    path_c[len] = '\0';
    flv_path = path_c;
    //LOGE("lianghuan path_c = %s",path_c);
}

void set_yuv_callback(JNIEnv* env, jclass clazz, jobject obj){
    if (init_flag == 0){
        return;
    }

    jobject callback = env->NewGlobalRef(obj);
    if (env->IsSameObject(callback,yuv_callback)){
        return;
    }

    yuv_callback = callback;

    if (yuv_callback){
        jclass clazz = env->GetObjectClass(yuv_callback);

        if (clazz){
            callback_method = env->GetMethodID(clazz,"onYuvFrame","(Ljava/nio/ByteBuffer;I)V");
        }
        if (callback_method){
            LOGE("lianghuan set native callback success");
        } else{
            env->DeleteGlobalRef(yuv_callback);
            yuv_callback = NULL;
            callback_method = NULL;
        }

    }
}

inline void notifyDecodeThread(){
    pthread_mutex_lock(&mPauseMutex);
    pthread_cond_signal(&mPauseCond);
    pthread_mutex_unlock(&mPauseMutex);
}

void resume(JNIEnv* env, jclass clazz){
    pause_flag = 0;
    notifyDecodeThread();
}

inline void notifyNotifyThread(){
    pthread_mutex_lock(&mMutex);
    pthread_cond_signal(&mCond);
    pthread_mutex_unlock(&mMutex);
}

void start_flv2yuv(JNIEnv* env, jclass clazz){
    if (init_flag == 0){
        return;
    }
    LOGE("lianghuan wait work thread destory");
    run_flag = 0;
    notifyNotifyThread();
    if (pause_flag){
        pause_flag = 0;
        notifyDecodeThread();
    }
    pthread_join(pthread[0],NULL);
    pthread_join(pthread[1],NULL);
    LOGE("lianghuan old work thread had been destory, now do new work");
    memset(&pthread,0, sizeof(pthread));

    run_flag = 1;
    /*int res = pthread_create(&pthread[0],NULL,notify_thread,NULL);
    if (res != 0){
        run_flag = 0;
        return;
    }
*/
    int res = pthread_create(&pthread[1],NULL,decode_thread,NULL);

    if (res != 0){
        run_flag = 0;
        return;
    }
    LOGE("lianghuan start flv2yuv success");
}

void stop(JNIEnv* env, jclass clazz){
    run_flag = 0;
}

void pause(JNIEnv* env,jclass clazz){
    pause_flag = 1;
}



void *decode_thread(void* args){
    while (run_flag){
        flv2yuv(flv_path);
    }
    LOGE("lianghuan stop decode success");
    pthread_exit(NULL);
}

void *notify_thread(void* args){
    JNIEnv* env;
    javaVM->AttachCurrentThread(&env,NULL);
    while (run_flag){
        pthread_mutex_lock(&mMutex);
        pthread_cond_wait(&mCond,&mMutex);
        pthread_mutex_unlock(&mMutex);

        jobject buf = env->NewDirectByteBuffer(yuv_data,460800);
        env->CallVoidMethod(yuv_callback,callback_method,buf,effort_timestamp);
        env->ExceptionClear();
        env->DeleteLocalRef(buf);
    }
    javaVM->DetachCurrentThread();

    pthread_exit(NULL);
}

jbyteArray get_extradata(JNIEnv* env, jclass clazz){
    if (extradata == NULL){
        return 0;
    }

    if (extradata_size == 0){
        return 0;
    }
    LOGE("lianghuan get extradata");


    jbyteArray data = env->NewByteArray(extradata_size);
    jbyte* data_ptr = env->GetByteArrayElements(data,0);

    unsigned char* data_c = (unsigned char*)data_ptr;
    memcpy(data_c,extradata,extradata_size);
    env->ReleaseByteArrayElements(data,data_ptr,0);
    return data;
}

void flv2yuv(char* path){
    LOGE("lianghuan flv2yuv");
    JNIEnv* env;
    LOGE("lianghuan flv2yuv 2");
    javaVM->AttachCurrentThread(&env,NULL);
    LOGE("lianghuan flv2yuv 3");
    av_register_all();
    avformat_network_init();
    AVFormatContext* pFormatContext = avformat_alloc_context();
    if (avformat_open_input(&pFormatContext, path, NULL, NULL) != 0){
        LOGE("lianghuan avformat_open_input failed");
        return ;
    }



    if (avformat_find_stream_info(pFormatContext, NULL) < 0){
        LOGE("lianghuan avformat_find_stream_info failed");
        return;
    }

    extradata_size = pFormatContext->streams[0]->codec->extradata_size;
    LOGE("lianghuan extradata_size = %d",extradata_size);
    if (extradata != NULL){
        delete extradata;
    }
    extradata = (unsigned char*)malloc(extradata_size);
    memcpy(extradata,pFormatContext->streams[0]->codec->extradata,extradata_size);

    int32_t dwVideoType = -1;
    for (int i = 0; i < pFormatContext->nb_streams; i++){
        if (pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            dwVideoType = i;
            break;
        }
    }
    //LOGE("lianghuan dwVideoType = %x",dwVideoType);
    AVCodecContext *pCodecContext = pFormatContext->streams[dwVideoType]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodecContext->codec_id);
//    AVCodec *pCodec = avcodec_find_decoder_by_name("h264_mediacodec");
    if (avcodec_open2(pCodecContext, pCodec, NULL) != 0){
        avformat_close_input(&pFormatContext);
        LOGE("lianghuan avcodec_open2 failed");
        return;
    }
    int nGot = 0;
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pYuv = av_frame_alloc();
    AVPacket *pPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    int nPicSize = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecContext->width, pCodecContext->height);
    uint8_t *buf = (uint8_t*)av_malloc(nPicSize);
    //LOGE("lianghuan decode width = %d, height = %d, pix_fmt = %x",pCodecContext->width,pCodecContext->height,pCodecContext->pix_fmt);
    //LOGE("lianghuan nPicSize = %d",nPicSize);
    if (buf == NULL){
        printf("av malloc failed!\n");
        exit(1);
    }
    avpicture_fill((AVPicture *)pYuv, buf, AV_PIX_FMT_YUV420P, pCodecContext->width, pCodecContext->height);
    int y_size = pCodecContext->width * pCodecContext->height;
    int u_size,v_size;
    u_size = v_size = y_size / 4;

    SwsContext* pSws = sws_getContext(pCodecContext->width, pCodecContext->height, pCodecContext->pix_fmt, pCodecContext->width, pCodecContext->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    int64_t pre_frame_timestamp = 0;
    gettimeofday(&time_pre_decode,NULL);
    while (run_flag){

        if (av_read_frame(pFormatContext, pPacket) >= 0){
            //LOGE("lianghuan duration = %d, convorgonce_duration = %d, dts = %d,stream_index = %d, pos = %d, pts = %d",pPacket->duration,pPacket->convergence_duration,pPacket->dts,pPacket->stream_index,pPacket->pos,pPacket->pts);
            //LOGE("lianghuan av_read_frame stread_index = %x",pPacket->stream_index);

            pPacket->pts = 0;
            pPacket->pos = 0;

            if (pPacket->stream_index == dwVideoType){
                long dts = pPacket->dts;
                LOGE("lianghuan dts = %ld, size = %d, flags = %d",dts,pPacket->size,pPacket->flags);
                memcpy(yuv_data,(pPacket->data + 4),(pPacket->size -4));
                jobject buf = env->NewDirectByteBuffer(yuv_data,(pPacket->size - 4));
                env->CallVoidMethod(yuv_callback,callback_method,buf,pPacket->dts);
                env->ExceptionClear();
                env->DeleteLocalRef(buf);

                av_usleep(30000);

//                if (avcodec_decode_video2(pCodecContext, pFrame, &nGot, pPacket) < 0){
//                    break;
//                }

               // LOGE("lianghuan decode video frame finish %d, best_timestamp = %d",nGot,pFrame->best_effort_timestamp);
                if(false){
                    int nRet = sws_scale(pSws,
                                         reinterpret_cast<const uint8_t *const *>(pFrame->data), pFrame->linesize, 0, pCodecContext->height, pYuv->data, pYuv->linesize);

                    memcpy(yuv_data,pYuv->data[0],y_size);
                    memcpy(yuv_data + y_size,pYuv->data[1],u_size);
                    memcpy(yuv_data + y_size + u_size,pYuv->data[2],v_size);
                    effort_timestamp = pFrame->best_effort_timestamp;
//                    time(&time_after_decode);
                    gettimeofday(&time_after_decode,NULL);
                    int32_t timestamp = pFrame->best_effort_timestamp - pre_frame_timestamp;
                    pre_frame_timestamp = pFrame->best_effort_timestamp;

                    int32_t pre_decode_time = time_pre_decode.tv_usec;
                    int32_t after_devode_time = time_after_decode.tv_usec;
                    int32_t sleeptime = after_devode_time - pre_decode_time;

                    int32_t sec = time_after_decode.tv_sec - time_pre_decode.tv_sec;
                    if (sec > 0){
                        sleeptime += sec * 1000 * 1000;
                    }
                    //LOGE("lianghuan sleeptime = %d, timestamp = %d",sleeptime,timestamp);
                    sleeptime = timestamp * 1000 - sleeptime;

                    //LOGE("lianghuan time pre decode = %d, time after decode = %d, sleeptime = %d",pre_decode_time,after_devode_time,sleeptime);
                    if (sleeptime > 0){
                        av_usleep(sleeptime);
                    }
                    gettimeofday(&time_pre_decode,NULL);

                    notifyNotifyThread();

                    if (pause_flag){
                        pthread_mutex_lock(&mPauseMutex);
                        pthread_cond_wait(&mPauseCond,&mPauseMutex);
                        pthread_mutex_unlock(&mPauseMutex);
                    }
                }
            }
        }
        else{
            break;
        }
    }
    javaVM->DetachCurrentThread();

    sws_freeContext(pSws);

    av_free(pFrame);
    av_free(pYuv);
    av_free(buf);
    av_free(pPacket);
    avcodec_close(pCodecContext);
    avformat_close_input(&pFormatContext);
}


void init(JNIEnv* env, jclass clazz, jobject obj){
    //LOGE("system properties init start");
    if (init_flag == 1){
        //LOGE("system properties init success");
        return;
    }
    //上下文对象
    jclass context_class = env->GetObjectClass(obj);

    jmethodID methodId = env->GetMethodID(context_class, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    jobject package_manager = env->CallObjectMethod(obj, methodId);
    if (package_manager == NULL) {
        //LOGE("package_manager is NULL!!!");
        return;
    }

    //反射获取包名
    methodId = env->GetMethodID(context_class, "getPackageName", "()Ljava/lang/String;");
    jstring package_name = (jstring)env->CallObjectMethod(obj, methodId);
    if (package_name == NULL) {
        //LOGE("package_name is NULL!!!");
        return;
    }
    env->DeleteLocalRef(context_class);

    //获取PackageInfo对象
    jclass pack_manager_class = env->GetObjectClass(package_manager);
    methodId = env->GetMethodID(pack_manager_class, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
    env->DeleteLocalRef(pack_manager_class);
    jobject package_info = env->CallObjectMethod(package_manager, methodId, package_name, 0x40);
    if (package_info == NULL) {
        //LOGE("getPackageInfo() is NULL!!!");
        return;
    }
    env->DeleteLocalRef(package_manager);


    //获取签名信息
    jclass package_info_class = env->GetObjectClass(package_info);
    jfieldID fieldId = env->GetFieldID(package_info_class, "signatures", "[Landroid/content/pm/Signature;");
    env->DeleteLocalRef(package_info_class);
    jobjectArray signature_object_array = (jobjectArray)env->GetObjectField(package_info, fieldId);
    if (signature_object_array == NULL) {
        //LOGE("signature is NULL!!!");
        return;
    }
    jobject signature_object = env->GetObjectArrayElement(signature_object_array, 0);
    env->DeleteLocalRef(package_info);


    //签名信息转换成sha1值
    jclass signature_class = env->GetObjectClass(signature_object);
    methodId = env->GetMethodID(signature_class, "toByteArray", "()[B");
    env->DeleteLocalRef(signature_class);
    jbyteArray signature_byte = (jbyteArray) env->CallObjectMethod(signature_object, methodId);
    jclass byte_array_input_class=env->FindClass("java/io/ByteArrayInputStream");
    methodId=env->GetMethodID(byte_array_input_class,"<init>","([B)V");
    jobject byte_array_input=env->NewObject(byte_array_input_class,methodId,signature_byte);
    jclass certificate_factory_class=env->FindClass("java/security/cert/CertificateFactory");
    methodId=env->GetStaticMethodID(certificate_factory_class,"getInstance","(Ljava/lang/String;)Ljava/security/cert/CertificateFactory;");
    jstring x_509_jstring=env->NewStringUTF("X.509");
    jobject cert_factory=env->CallStaticObjectMethod(certificate_factory_class,methodId,x_509_jstring);
    methodId=env->GetMethodID(certificate_factory_class,"generateCertificate",("(Ljava/io/InputStream;)Ljava/security/cert/Certificate;"));
    jobject x509_cert=env->CallObjectMethod(cert_factory,methodId,byte_array_input);
    env->DeleteLocalRef(certificate_factory_class);
    jclass x509_cert_class=env->GetObjectClass(x509_cert);
    methodId=env->GetMethodID(x509_cert_class,"getEncoded","()[B");
    jbyteArray cert_byte=(jbyteArray)env->CallObjectMethod(x509_cert,methodId);
    env->DeleteLocalRef(x509_cert_class);
    jclass message_digest_class=env->FindClass("java/security/MessageDigest");
    methodId=env->GetStaticMethodID(message_digest_class,"getInstance","(Ljava/lang/String;)Ljava/security/MessageDigest;");
    jstring sha1_jstring=env->NewStringUTF("SHA1");
    jobject sha1_digest=env->CallStaticObjectMethod(message_digest_class,methodId,sha1_jstring);
    methodId=env->GetMethodID(message_digest_class,"digest","([B)[B");
    jbyteArray sha1_byte=(jbyteArray)env->CallObjectMethod(sha1_digest,methodId,cert_byte);
    env->DeleteLocalRef(message_digest_class);

    //转换成char
    jsize array_size=env->GetArrayLength(sha1_byte);
    jbyte* sha1 =env->GetByteArrayElements(sha1_byte,NULL);
    char *hex_sha=new char[array_size*2+1];
    for (int i = 0; i <array_size ; ++i) {
        hex_sha[2*i]=hexcode[((unsigned char)sha1[i])/16];
        hex_sha[2*i+1]=hexcode[((unsigned char)sha1[i])%16];
    }
    hex_sha[array_size*2]='\0';
    //LOGE("hex_sha = %s",hex_sha);

    if (strcmp(app_sign,hex_sha)){
        init_flag = 0;
        //LOGE("system properties failure");
    } else{
        init_flag = 1;
        //LOGE("system properties success");
    }

    return;
}


static const char* className = "com/example/bamboofly/wifiserver/Flv2Yuv";

static JNINativeMethod gMethod[] = {
        {"setFlvPath","(Ljava/lang/String;)V",(void *)set_flv},
        {"setCallback","(Lcom/example/bamboofly/wifiserver/YuvCallback;)V",(void *)set_yuv_callback},
        {"startDecode","()V",(void *)start_flv2yuv},
        {"stopDecode","()V",(void *)stop},
        {"pauseTest","()V",(void *)pause},
        {"init","(Ljava/lang/Object;)V",(void *)init},
        {"resumeTest","()V",(void *)resume},
        {"getExtraData","()[B",(jbyteArray *)get_extradata}
};

static int registerNativeMethod(JNIEnv* env){

    jclass clazz;
    clazz = env->FindClass(className);
    if (clazz == NULL){
        return -1;
    }

    if (env->RegisterNatives(clazz,gMethod, sizeof(gMethod) / sizeof(gMethod[0])) != JNI_OK){
        return -1;
    }
    return 0;
}

jint JNI_OnLoad(JavaVM* vm, void *reserved){
    JNIEnv* env = NULL;
    jint result = -1;
    LOGE("jni_onload");

    if ((*vm).GetEnv((void **)&env,JNI_VERSION_1_4) != JNI_OK){
        return result;
    }

    if (registerNativeMethod(env) < 0){
        return result;
    }

//    av_jni_set_java_vm(vm,NULL);
    init_flag = 1;
    javaVM = vm;
    return JNI_VERSION_1_4;
}

jint JNI_OnUnLoad(JavaVM* vm,void* resered){
    LOGE("jni onunload");
}

#ifdef __cplusplus
}
#endif
