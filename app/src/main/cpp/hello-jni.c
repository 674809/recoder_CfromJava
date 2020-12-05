//
// Created by ybf on 2020/12/1.
//

#include <malloc.h>
#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <stdio.h>

static JNIEnv* (*jni_env);
static jbyteArray buffer;
static jobject audio_track;
static jint buffer_size;
static jmethodID method_write;

#define AUDIO_SOURCE_VOICE_COMMUNICATION (7)
#define AUDIO_SOURCE_MIC (1)
#define SAMPLE_RATE_IN_HZ (11025)
#define CHANNEL_CONFIGURATION_MONO (16)
#define ENCODING_PCM_16BIT (2)

#define LOG_TAG "test"
#define LOGI(f,v)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,f,v)
#define LOGI2(a)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,a)
/* com_test_recoder_MainActivity */

static int run= 1;
void  Java_com_test_recoder_MainActivity_stopRecord(JNIEnv* jni_env,
                                                    jobject thiz){
    run = 0;
}
void  Java_com_test_recoder_MainActivity_startRecord(JNIEnv* jni_env,
                                                     jobject thiz){

    jclass audio_record_class =(*jni_env)->FindClass(jni_env,"android/media/AudioRecord");

    jmethodID constructor_id = (*jni_env)->GetMethodID(jni_env,audio_record_class, "<init>",
                                                       "(IIIII)V");

    jmethodID min_buff_size_id = (*jni_env)->GetStaticMethodID(jni_env,audio_record_class,
                                                               "getMinBufferSize", "(III)I");

    jint buff_size
            = (*jni_env)->CallStaticIntMethod(jni_env,audio_record_class,
                                              min_buff_size_id,
                                              SAMPLE_RATE_IN_HZ,
                                              CHANNEL_CONFIGURATION_MONO,
                                              ENCODING_PCM_16BIT);

    jobject audioRecord = (*jni_env)->NewObject(jni_env,audio_record_class, constructor_id,
            //          AUDIO_SOURCE_MIC,
                                                AUDIO_SOURCE_VOICE_COMMUNICATION,
                                                SAMPLE_RATE_IN_HZ,
                                                CHANNEL_CONFIGURATION_MONO,
                                                ENCODING_PCM_16BIT,
                                                buff_size);

    LOGI2("startRecording");
    jmethodID record_id = (*jni_env)->GetMethodID(jni_env,audio_record_class, "startRecording",
                                                  "()V");

    //start recording
    (*jni_env)->CallVoidMethod(jni_env,audioRecord, record_id);
    LOGI2("after call startRecording");
    jmethodID read_id = (*jni_env)->GetMethodID(jni_env,audio_record_class, "read", "([BII)I");


    int nread = 0;
    int blockSize = 100;
    jbyteArray read_buff = (*jni_env)->NewByteArray(jni_env,blockSize);
    jbyteArray aes_bytes = (*jni_env)->NewByteArray(jni_env,blockSize);

    jbyte* audio_bytes;

    FILE* fp = fopen("/storage/emulated/0/RecoderTest/temp.pcm","ab");
    LOGI2("after fopen");

    //
    jclass HelloJniCls =(*jni_env)->FindClass(jni_env,"com/test/recoder/MainActivity");
    jmethodID receiveAudioData = (*jni_env)->GetStaticMethodID(jni_env,HelloJniCls,"receiveAudioData",
                                                               "([BI)V");

    while (run) {

        nread = (*jni_env)->CallIntMethod(jni_env,audioRecord,read_id, read_buff, 0, blockSize);
        if(nread<=0){
            break;
        }

        audio_bytes = (jbyte*)calloc(nread,1);

        (*jni_env)->GetByteArrayRegion(jni_env,read_buff, 0, nread,audio_bytes);
//          fwrite(audio_bytes, 1, nread, fp);

        (*jni_env)->CallStaticVoidMethod(jni_env,HelloJniCls, receiveAudioData, read_buff,nread);
        usleep(50);
    }
}

void  Java_com_test_recoder_MainActivity_play(JNIEnv* jni_env,
                                              jobject thiz){
    LOGI2("after Java_com_example_hellojni_HelloJni_play");

//  (*jni_env) = jni_env;
    jclass audio_track_cls = (*jni_env)->FindClass(jni_env,"android/media/AudioTrack");
    jmethodID min_buff_size_id = (*jni_env)->GetStaticMethodID(
            jni_env,
            audio_track_cls,
            "getMinBufferSize",
            "(III)I");
    buffer_size = (*jni_env)->CallStaticIntMethod(jni_env,audio_track_cls,min_buff_size_id,
                                                  11025,
                                                  2,          /*CHANNEL_CONFIGURATION_MONO*/
                                                  2);         /*ENCODING_PCM_16BIT*/
    LOGI("buffer_size=%i",buffer_size);
    buffer = (*jni_env)->NewByteArray(jni_env,buffer_size/4);

    char buf[buffer_size/4];

    jmethodID constructor_id = (*jni_env)->GetMethodID(jni_env,audio_track_cls, "<init>",
                                                       "(IIIIII)V");
    audio_track = (*jni_env)->NewObject(jni_env,audio_track_cls,
                                        constructor_id,
                                        3,            /*AudioManager.STREAM_MUSIC*/
                                        11025,        /*sampleRateInHz*/
                                        2,            /*CHANNEL_CONFIGURATION_MONO*/
                                        2,            /*ENCODING_PCM_16BIT*/
                                        buffer_size,  /*bufferSizeInBytes*/
                                        1             /*AudioTrack.MODE_STREAM*/
    );

    //setvolume
    LOGI2("setStereoVolume 1");
    jmethodID setStereoVolume = (*jni_env)->GetMethodID(jni_env,audio_track_cls,"setStereoVolume","(FF)I");
    (*jni_env)->CallIntMethod(jni_env,audio_track,setStereoVolume,1.0,1.0);
    LOGI2("setStereoVolume 2");
    //play
    jmethodID method_play = (*jni_env)->GetMethodID(jni_env,audio_track_cls, "play",
                                                    "()V");
    (*jni_env)->CallVoidMethod(jni_env,audio_track, method_play);

    //write
    method_write = (*jni_env)->GetMethodID(jni_env,audio_track_cls,"write","([BII)I");

    FILE* fp = fopen("/storage/emulated/0/RecoderTest/temp.pcm","rb");
    LOGI2("after open");
    int i=0;
    while(!feof(fp)){
        jint read= fread(buf,sizeof(char),200,fp);
        (*jni_env)->SetByteArrayRegion(jni_env,buffer, 0,read, (jbyte *)buf);

        (*jni_env)->CallIntMethod(jni_env,audio_track,method_write,buffer,0,read);
    }

}