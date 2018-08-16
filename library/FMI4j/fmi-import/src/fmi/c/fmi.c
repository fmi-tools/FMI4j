
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include "fmi2FunctionTypes.h"

#if defined(_MSC_VER) || defined(WIN32) || defined(__MINGW32__)
#include <windows.h>
#define DLL_HANDLE HANDLE
#else
#define DLL_HANDLE void*
#include <dlfcn.h>
#endif

#ifdef WIN32
#define function_ptr FARPROC
#else
typedef void* function_ptr; 
#endif

DLL_HANDLE handle;

function_ptr* load_function(const char* function_name) {
#ifdef WIN32
	return  (function_ptr) GetProcAddress(handle, function_name);
#else
	return dlsym(handle, function_name);
#endif
}

void logger(void* fmi2ComponentEnvironment, fmi2String instance_name, fmi2Status status, fmi2String category, fmi2String message, ...) {
     printf("instanceName = %s, category = %s: %s\n", instance_name, category, message);
}

fmi2CallbackFunctions callback = {
    .logger = logger,
    .allocateMemory = calloc,
    .freeMemory = free,
    .stepFinished = NULL,
    .componentEnvironment = NULL
};

jweak NONE_STATUS = NULL;
jweak OK_STATUS = NULL;
jweak WARNING_STATUS = NULL;
jweak DISCARD_STATUS = NULL;
jweak ERROR_STATUS = NULL;
jweak FATAL_STATUS = NULL;
jweak PENDING_STATUS = NULL;

jobject asJavaEnum(JNIEnv *env, fmi2Status status) {

    if (OK_STATUS == NULL) {

        jclass cls;
        jmethodID method;

        cls = (*env)->FindClass(env, "no/mechatronics/sfi/fmi4j/common/FmiStatus");
        method = (*env)->GetStaticMethodID(env, cls, "valueOf", "(I)Lno/mechatronics/sfi/fmi4j/common/FmiStatus;");

        NONE_STATUS = (*env)->NewWeakGlobalRef(env, (*env)->CallStaticObjectMethod(env, cls, method, -1));
        OK_STATUS = (*env)->NewWeakGlobalRef(env, (*env)->CallStaticObjectMethod(env, cls, method, 0));
        WARNING_STATUS = (*env)->NewWeakGlobalRef(env, (*env)->CallStaticObjectMethod(env, cls, method, 1));
        DISCARD_STATUS = (*env)->NewWeakGlobalRef(env, (*env)->CallStaticObjectMethod(env, cls, method, 2));
        ERROR_STATUS = (*env)->NewWeakGlobalRef(env, (*env)->CallStaticObjectMethod(env, cls, method, 3));
        FATAL_STATUS = (*env)->NewWeakGlobalRef(env, (*env)->CallStaticObjectMethod(env, cls, method, 4));
        PENDING_STATUS = (*env)->NewWeakGlobalRef(env, (*env)->CallStaticObjectMethod(env, cls, method, 5));

    }

    switch(status) {

        case 0: return OK_STATUS;
        case 1: return WARNING_STATUS;
        case 2: return DISCARD_STATUS;
        case 3: return ERROR_STATUS;
        case 4: return FATAL_STATUS;
        case 5: return PENDING_STATUS;
        default: return NONE_STATUS;

    }

}

JNIEXPORT jboolean JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_load(JNIEnv *env, jobject obj, jstring lib_name) {

    const char* _lib_name = (*env)->GetStringUTFChars(env, lib_name, 0);
    #ifdef WIN32
    	handle = LoadLibrary(_lib_name);
    #else
    	handle = dlopen(_lib_name, RTLD_NOW|RTLD_GLOBAL);
    #endif
    (*env)->ReleaseStringUTFChars(env, lib_name, _lib_name);

    if (!handle) {
        return JNI_FALSE;
    }

    return JNI_TRUE;

}

JNIEXPORT jstring JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_getVersion(JNIEnv *env, jobject obj) {
    const char* (*fmi2GetVersion)(void);
    fmi2GetVersion = load_function("fmi2GetVersion");
    const char* version = (*fmi2GetVersion)();
    return (*env)->NewStringUTF(env, version);
}

JNIEXPORT jstring JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_getTypesPlatform(JNIEnv *env, jobject obj) {
    const char* (*fmi2GetTypesPlatform)(void);
    fmi2GetTypesPlatform = load_function("fmi2GetTypesPlatform");
    const char* platform = (*fmi2GetTypesPlatform)();
    return (*env)->NewStringUTF(env, platform);
}

JNIEXPORT jlong JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_instantiate(JNIEnv *env, jobject obj, jstring instanceName, jint type, jstring guid,  jstring resourceLocation, jboolean visible, jboolean loggingOn) {

    const char* _instanceName = (*env)->GetStringUTFChars(env, instanceName, 0);
    const char* _guid = (*env)->GetStringUTFChars(env, guid, 0);
    const char* _resourceLocation = (*env)->GetStringUTFChars(env, resourceLocation, 0);

    void* (*fmi2Instantiate)(fmi2String, fmi2Type, fmi2String, fmi2String, const fmi2CallbackFunctions*, fmi2Boolean, fmi2Boolean);
    fmi2Instantiate = load_function("fmi2Instantiate");
    fmi2Component c = (*fmi2Instantiate)(_instanceName, type, _guid, _resourceLocation, &callback, visible == JNI_FALSE ? 0 : 1, loggingOn == JNI_FALSE ? 0 : 1);

    (*env)->ReleaseStringUTFChars(env, instanceName, _instanceName);
    (*env)->ReleaseStringUTFChars(env, guid, _guid);
    (*env)->ReleaseStringUTFChars(env, resourceLocation, _resourceLocation);

    return (jlong) c;
}


JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_setDebugLogging(JNIEnv *env, jobject obj, jlong c, jboolean loggingOn, jobjectArray categories) {

    const jsize nCategories = (*env)->GetArrayLength(env, categories);
    char* _categories = malloc(sizeof(char) * nCategories);

    for (int i = 0; i < nCategories; i++) {
        jstring str = (jstring) (*env)->GetObjectArrayElement(env, categories, i);
        _categories[i] = (*env)->GetStringUTFChars(env, str, NULL);
    }

    int (*fmi2SetDebugLogging)(void*, int, int, const char* []);
    fmi2SetDebugLogging = load_function("fmi2SetDebugLogging");
    int status = (*fmi2SetDebugLogging)((void*) c, loggingOn == JNI_FALSE ? 0 : 1, nCategories, _categories);

    free(_categories);

    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_setupExperiment(JNIEnv *env, jobject obj, jlong c, jboolean toleranceDefined, jdouble tolerance, jdouble startTime, jdouble stopTime) {

    fmi2Boolean stopTimeDefined = stopTime > startTime ? 1: 0;

    int (*fmi2SetupExperiment)(fmi2Component, fmi2Boolean, fmi2Real, fmi2Real, fmi2Boolean, fmi2Real);
    fmi2SetupExperiment = load_function("fmi2SetupExperiment");
    int status = (*fmi2SetupExperiment)((void*) c, toleranceDefined, tolerance, startTime, stopTimeDefined, stopTime);
    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_enterInitializationMode(JNIEnv *env, jobject obj, jlong c) {
    int (*fmi2EnterInitializationMode)(fmi2Component);
    fmi2EnterInitializationMode = load_function("fmi2EnterInitializationMode");
    int status = (*fmi2EnterInitializationMode)((void*) c);
    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_exitInitializationMode(JNIEnv *env, jobject obj, jlong c) {
    int (*fmi2ExitInitializationMode)(fmi2Component);
    fmi2ExitInitializationMode = load_function("fmi2ExitInitializationMode");
    int status = (*fmi2ExitInitializationMode)((void*) c);
    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_terminate(JNIEnv *env, jobject obj, jlong c) {
    int (*fmi2Terminate)(fmi2Component);
    fmi2Terminate = load_function("fmi2Terminate");
    int status = (*fmi2Terminate)((void*) c);
    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_reset(JNIEnv *env, jobject obj, jlong c) {
    int (*fmi2Reset)(fmi2Component);
    fmi2Reset = load_function("fmi2Reset");
    int status = (*fmi2Reset)((void*) c);
    return asJavaEnum(env, status);
}

JNIEXPORT void JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_freeInstance(JNIEnv *env, jobject obj, jlong c) {
    void (*fmi2FreeInstance)(fmi2Component);
    fmi2FreeInstance = load_function("fmi2FreeInstance");
    (*fmi2FreeInstance)((void*) c);
    return;
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_getInteger(JNIEnv *env, jobject obj, jlong c, jintArray vr, jintArray ref) {

    const jsize size = (*env)->GetArrayLength(env, vr);
    const jint *_vr = (*env)->GetIntArrayElements(env, vr, 0);

    fmi2Integer* _ref = malloc(sizeof(fmi2Integer) * size);

    fmi2Status (*fmi2GetInteger)(fmi2Component, const fmi2ValueReference[], size_t, fmi2Integer[]);
    fmi2GetInteger = load_function("fmi2GetInteger");
    fmi2Status status = (*fmi2GetInteger)((void*) c, _vr, size, _ref);

    (*env)->SetIntArrayRegion(env, ref, 0, size, _ref);

    free(_ref);

    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_getReal(JNIEnv *env, jobject obj, jlong c, jintArray vr, jdoubleArray ref) {

    const jsize size = (*env)->GetArrayLength(env, vr);
    const jint *_vr = (*env)->GetIntArrayElements(env, vr, 0);

    fmi2Real* _ref = malloc(sizeof(fmi2Real) * size);

    fmi2Status (*fmi2GetReal)(fmi2Component, const fmi2ValueReference[], size_t, fmi2Real[]);
    fmi2GetReal = load_function("fmi2GetReal");
    fmi2Status status = (*fmi2GetReal)((void*) c, _vr, size, _ref);

    (*env)->SetDoubleArrayRegion(env, ref, 0, size, _ref);

    free(_ref);
    (*env)->ReleaseIntArrayElements(env, vr, _vr, NULL);

    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_getString(JNIEnv *env, jobject obj, jlong c, jintArray vr, jobjectArray ref) {

    const jsize size = (*env)->GetArrayLength(env, vr);
    const jint *_vr = (*env)->GetIntArrayElements(env, vr, 0);

    fmi2Status (*fmi2GetString)(fmi2Component, const fmi2ValueReference[], size_t, fmi2String[]);
    fmi2GetString = load_function("fmi2GetString");

    char* _ref = malloc(sizeof(char) * size);
    for (int i = 0; i < size; i++) {
        jstring str = (jstring) (*env)->GetObjectArrayElement(env, ref, i);
        _ref[i] = (*env)->GetStringUTFChars(env, str, NULL);
    }

    fmi2Status status = (*fmi2GetString)((void*) c, _vr, size, _ref);

    for (int i = 0; i < size; i++) {
        jstring value = (*env)->NewStringUTF(env, _ref[i]);
        (*env)->SetObjectArrayElement(env, ref, i, value);
    }

    free(_ref);
    (*env)->ReleaseIntArrayElements(env, vr, _vr, NULL);

    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_getBoolean(JNIEnv *env, jobject obj, jlong c, jintArray vr, jbooleanArray ref) {

    const jsize size = (*env)->GetArrayLength(env, vr);
    const jint *_vr = (*env)->GetIntArrayElements(env, vr, 0);

    fmi2Boolean* _ref = malloc(sizeof(fmi2Boolean) * size);

    fmi2Status (*fmi2GetBoolean)(fmi2Component, const fmi2ValueReference[], size_t, fmi2Boolean[]);
    fmi2GetBoolean = load_function("fmi2GetBoolean");
    fmi2Status status = (*fmi2GetBoolean)((void*) c, _vr, size, _ref);

    (*env)->SetBooleanArrayRegion(env, ref, 0, size, _ref);

    free(_ref);
    (*env)->ReleaseIntArrayElements(env, vr, _vr, NULL);

    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_setInteger(JNIEnv *env, jobject obj, jlong c, jintArray vr, jintArray values) {

    const jsize size = (*env)->GetArrayLength(env, vr);
    const jint *_vr = (*env)->GetIntArrayElements(env, vr, 0);
    const jint *_values = (*env)->GetIntArrayElements(env, values, 0);

    fmi2Status (*fmi2SetInteger)(fmi2Component, const fmi2ValueReference[], size_t, fmi2Integer[]);
    fmi2SetInteger = load_function("fmi2SetInteger");

    int status = (*fmi2SetInteger)((void*) c, _vr, size, _values);

    (*env)->ReleaseIntArrayElements(env, vr, _vr, NULL);
    (*env)->ReleaseIntArrayElements(env, values, _values, NULL);

    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_setReal(JNIEnv *env, jobject obj, jlong c, jintArray vr, jdoubleArray values) {

    const jsize size = (*env)->GetArrayLength(env, vr);
    const jint *_vr = (*env)->GetIntArrayElements(env, vr, 0);
    const jdouble *_values = (*env)->GetDoubleArrayElements(env, values, 0);

    fmi2Status (*fmi2SetReal)(fmi2Component, const fmi2ValueReference[], size_t, fmi2Real[]);
    fmi2SetReal = load_function("fmi2SetReal");
    fmi2Status status = (*fmi2SetReal)((void*) c, _vr, size, _values);

    (*env)->ReleaseIntArrayElements(env, vr, _vr, NULL);
    (*env)->ReleaseDoubleArrayElements(env, values, _values, NULL);

    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_setString(JNIEnv *env, jobject obj, jlong c, jintArray vr, jobjectArray values) {

    const jsize size = (*env)->GetArrayLength(env, vr);
    const jint *_vr = (*env)->GetIntArrayElements(env, vr, 0);

    char* _values = malloc(sizeof(char) * size);
    for (int i = 0; i < size; i++) {
       jstring str = (jstring) (*env)->GetObjectArrayElement(env, values, i);
       _values[i] = (*env)->GetStringUTFChars(env, str, NULL);
    }

    fmi2Status (*fmi2SetString)(fmi2Component, const fmi2ValueReference[], size_t, fmi2String[]);
    fmi2SetString = load_function("fmi2SetString");
    fmi2Status status = (*fmi2SetString)((void*) c, _vr, size, _values);

    free(_values);
    (*env)->ReleaseIntArrayElements(env, vr, _vr, NULL);

    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_setBoolean(JNIEnv *env, jobject obj, jlong c, jintArray vr, jbooleanArray values) {

    const jsize size = (*env)->GetArrayLength(env, vr);
    const jint *_vr = (*env)->GetIntArrayElements(env, vr, 0);
    const jboolean *_values = (*env)->GetBooleanArrayElements(env, values, 0);

    fmi2Status (*fmi2SetBoolean)(fmi2Component, const fmi2ValueReference[], size_t, fmi2Boolean[]);
    fmi2SetBoolean = load_function("fmi2SetBoolean");
    fmi2Status status = (*fmi2SetBoolean)((void*) c, _vr, size, _values);

    (*env)->ReleaseIntArrayElements(env, vr, _vr, NULL);
    (*env)->ReleaseBooleanArrayElements(env, values, _values, NULL);

    return asJavaEnum(env, status);
}


JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_getFMUstate(JNIEnv *env, jobject obj, jlong c, jobject state) {

    jclass cls;
    jfieldID id;
    fmi2FMUstate _state;

    cls = (*env)->FindClass(env, "no/mechatronics/sfi/fmi4j/jni/PointerByReference");
    id = (*env)->GetFieldID(env, cls, "value", "J");

    fmi2Status (*fmi2GetFMUstate)(fmi2Component, fmi2FMUstate*);
    fmi2GetFMUstate = load_function("fmi2GetFMUstate");
    fmi2Status status = (*fmi2GetFMUstate)((void*) c, &_state);

    (*env)->SetIntField(env, state, id, (jlong) _state);

    return asJavaEnum(env, status);

}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_setFMUstate(JNIEnv *env, jobject obj, jlong c, jlong state) {

    jclass cls;
    jfieldID id;

    cls = (*env)->FindClass(env, "no/mechatronics/sfi/fmi4j/jni/PointerByReference");
    id = (*env)->GetFieldID(env, cls, "value", "J");

    fmi2Status (*fmi2setFMUstate)(fmi2Component, fmi2FMUstate);
    fmi2setFMUstate = load_function("fmi2SetFMUstate");
    fmi2Status status = (*fmi2setFMUstate)((void*) c, (void*) state);

    return asJavaEnum(env, status);

}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_getDirectionalDerivative(JNIEnv *env, jobject obj, jlong c, jintArray vUnknown_ref, jintArray vKnown_ref, jdoubleArray dvKnown_ref, jdoubleArray dvUnknown_ref) {

    const jsize nUknown = (*env)->GetArrayLength(env, vUnknown_ref);
    const jsize nKnown = (*env)->GetArrayLength(env, vUnknown_ref);

    const jint *_vUnknown_ref = (*env)->GetIntArrayElements(env, vUnknown_ref, 0);
    const jint *_vKnown_ref = (*env)->GetIntArrayElements(env, vKnown_ref, 0);
    const jdouble *_dvKnown_ref = (*env)->GetDoubleArrayElements(env, vKnown_ref, 0);
    const jdouble *_dvUnknown_ref = (*env)->GetDoubleArrayElements(env, vUnknown_ref, 0);

    fmi2Status (*fmi2GetDirectionalDerivative)(fmi2Component, const fmi2ValueReference[], size_t, const fmi2ValueReference[], size_t, const fmi2Real[], fmi2Real[]);
    fmi2GetDirectionalDerivative = load_function("fmi2GetDirectionalDerivative");
    fmi2Status status = (*fmi2GetDirectionalDerivative)((void*) c, _vUnknown_ref, nUknown, _vKnown_ref, nKnown, _dvKnown_ref, _dvUnknown_ref);

    (*env)->ReleaseIntArrayElements(env, vUnknown_ref, _vUnknown_ref, NULL);
    (*env)->ReleaseIntArrayElements(env, vKnown_ref, _vKnown_ref, NULL);

    (*env)->ReleaseDoubleArrayElements(env, dvKnown_ref, _dvKnown_ref, NULL);
    (*env)->ReleaseDoubleArrayElements(env, dvUnknown_ref, _dvUnknown_ref, NULL);

    return asJavaEnum(env, status);

}

JNIEXPORT jboolean JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_close(JNIEnv *env, jobject obj) {
    if (handle) {
        #ifdef WIN32
            return FreeLibrary(handle) == 0 ? JNI_FALSE : JNI_FALSE;
        #else
            return dlclose(handle)  == 0 ? JNI_FALSE : JNI_FALSE;
        #endif
        handle = NULL;
    }
    return JNI_FALSE;
}


/***************************************************
Functions for FMI2 for Co-Simulation
****************************************************/

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_step(JNIEnv *env, jobject obj, jlong c, jdouble currentCommunicationPoint, jdouble communicationStepSize, jboolean noSetFMUStatePriorToCurrentPoint) {
    fmi2Status (*fmi2DoStep)(fmi2Component, fmi2Real, fmi2Real, fmi2Boolean);
    fmi2DoStep = load_function("fmi2DoStep");
    fmi2Status status = (*fmi2DoStep)((void*) c, currentCommunicationPoint, communicationStepSize, noSetFMUStatePriorToCurrentPoint == JNI_FALSE ? 0 : 1);
    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_cancelStep(JNIEnv *env, jobject obj, jlong c) {
    fmi2Status (*fmi2CancelStep)(fmi2Component);
    fmi2CancelStep = load_function("fmi2CancelStep");
    fmi2Status status = (*fmi2CancelStep)((void*) c);
    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_setRealInputDerivatives(JNIEnv *env, jobject obj, jlong c, jintArray vr, jintArray order, jdoubleArray value) {

    const jsize nvr = (*env)->GetArrayLength(env, vr);

    const jint *_vr = (*env)->GetIntArrayElements(env, vr, 0);
    const jint *_order = (*env)->GetIntArrayElements(env, order, 0);
    const jdouble *_value = (*env)->GetDoubleArrayElements(env, value, 0);

    fmi2Status (*fmi2SetRealInputDerivatives)(fmi2Component, const fmi2ValueReference [], size_t, const fmi2Integer [], const fmi2Real []);
    fmi2SetRealInputDerivatives = load_function("fmi2SetRealInputDerivatives");
    fmi2Status status = (*fmi2SetRealInputDerivatives)((void*) c, _vr, nvr, _order, _value);

    (*env)->ReleaseIntArrayElements(env, vr, _vr, NULL);
    (*env)->ReleaseIntArrayElements(env, order, _order, NULL);

    (*env)->ReleaseDoubleArrayElements(env, value, _value, NULL);

    return asJavaEnum(env, status);

}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_getRealOutputDerivatives(JNIEnv *env, jobject obj, jlong c, jintArray vr, jintArray order, jdoubleArray value) {

    const jsize nvr = (*env)->GetArrayLength(env, vr);

    const jint *_vr = (*env)->GetIntArrayElements(env, vr, 0);
    const jint *_order = (*env)->GetIntArrayElements(env, order, 0);
    fmi2Real *_value = malloc(sizeof(fmi2Real) * nvr);

    fmi2Status (*fmi2GetRealOutputDerivatives)(fmi2Component, const fmi2ValueReference [], size_t, const fmi2Integer [], fmi2Real []);
    fmi2GetRealOutputDerivatives = load_function("fmi2GetRealOutputDerivatives");
    fmi2Status status = (*fmi2GetRealOutputDerivatives)((void*) c, _vr, nvr, _order, _value);

    (*env)->ReleaseIntArrayElements(env, vr, _vr, NULL);
    (*env)->ReleaseIntArrayElements(env, order, _order, NULL);

    (*env)->SetDoubleArrayRegion(env, value, 0, nvr, _value);
    free(_value);

    return asJavaEnum(env, status);

}


/***************************************************
Functions for FMI2 for Model Exchange
****************************************************/

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_enterEventMode(JNIEnv *env, jobject obj, jlong c) {
    fmi2Status (*fmi2EnterEventMode)(fmi2Component);
    fmi2EnterEventMode = load_function("fmi2EnterEventMode");
    fmi2Status status = (*fmi2EnterEventMode)((void*) c);
    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_enterContinuousTimeMode(JNIEnv *env, jobject obj, jlong c) {
    fmi2Status (*fmi2EnterContinuousTimeMode)(fmi2Component);
    fmi2EnterContinuousTimeMode = load_function("fmi2EnterContinuousTimeMode");
    fmi2Status status = (*fmi2EnterContinuousTimeMode)((void*) c);
    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_setTime(JNIEnv *env, jobject obj, jlong c, jdouble time) {
    fmi2Status (*fmi2SetTime)(fmi2Component, fmi2Real);
    fmi2SetTime = load_function("fmi2SetTime");
    fmi2Status status = (*fmi2SetTime)((void*) c, time);
    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_setContinuousStates(JNIEnv *env, jobject obj, jlong c, jdoubleArray x) {

    const jsize size = (*env)->GetArrayLength(env, x);
    const jdouble* _x = (*env)->GetDoubleArrayElements(env, x, 0);

    fmi2Status (*fmi2SetContinuousStates)(fmi2Component, const fmi2Real[], size_t);
    fmi2SetContinuousStates = load_function("fmi2SetContinuousStates");
    fmi2Status status = (*fmi2SetContinuousStates)((void*) c, _x, size);

    (*env)->ReleaseDoubleArrayElements(env, x, _x, NULL);

    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_getDerivatives(JNIEnv *env, jobject obj, jlong c, jdoubleArray derivatives) {

    const jsize size = (*env)->GetArrayLength(env, derivatives);
    fmi2Real* _derivatives = malloc(sizeof(fmi2Real) * size);

    fmi2Status (*fmi2GetDerivatives)(fmi2Component, fmi2Real[], size_t);
    fmi2GetDerivatives = load_function("fmi2GetDerivatives");
    fmi2Status status = (*fmi2GetDerivatives)((void*) c, _derivatives, size);

    (*env)->SetDoubleArrayRegion(env, derivatives, 0, size, _derivatives);
    free(_derivatives);

    return asJavaEnum(env, status);
}


JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_getEventIndicators(JNIEnv *env, jobject obj, jlong c, jdoubleArray eventIndicators) {

    const jsize size = (*env)->GetArrayLength(env, eventIndicators);
    fmi2Real* _eventIndicators = malloc(sizeof(fmi2Real) * size);

    fmi2Status (*fmi2GetEventIndicators)(fmi2Component, fmi2Real[], size_t);
    fmi2GetEventIndicators = load_function("fmi2GetEventIndicators");
    fmi2Status status = (*fmi2GetEventIndicators)((void*) c, _eventIndicators, size);

    (*env)->SetDoubleArrayRegion(env, eventIndicators, 0, size, _eventIndicators);
    free(_eventIndicators);

    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_getContinuousStates(JNIEnv *env, jobject obj, jlong c, jdoubleArray x) {

    const jsize size = (*env)->GetArrayLength(env, x);
    fmi2Real* _x = malloc(sizeof(fmi2Real) * size);

    fmi2Status (*fmi2GetContinuousStates)(fmi2Component, fmi2Real[], size_t);
    fmi2GetContinuousStates = load_function("fmi2GetContinuousStates");
    fmi2Status status = (*fmi2GetContinuousStates)((void*) c, _x, size);

    (*env)->SetDoubleArrayRegion(env, x, 0, size, _x);
    free(_x);

    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_getNominalsOfContinuousStates(JNIEnv *env, jobject obj, jlong c, jdoubleArray x_nominal) {

    const jsize size = (*env)->GetArrayLength(env, x_nominal);
    fmi2Real *_x_nominal = malloc(sizeof(fmi2Real) * size);

    fmi2Status (*fmi2GetNominalsOfContinuousStates)(fmi2Component, fmi2Real[], size_t);
    fmi2GetNominalsOfContinuousStates = load_function("fmi2GetNominalsOfContinuousStates");
    fmi2Status status = (*fmi2GetNominalsOfContinuousStates)((void*) c, _x_nominal, size);

    (*env)->SetDoubleArrayRegion(env, x_nominal, 0, size, _x_nominal);
    free(_x_nominal);

    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_completedIntegratorStep(JNIEnv *env, jobject obj, jlong c, jboolean noSetFMUStatePriorToCurrentPoint, jobject enterEventMode, jobject terminateSimulation) {

    jclass cls;
    jfieldID id;

    fmi2Boolean _enterEventMode;
    fmi2Boolean _terminateSimulation;

    cls = (*env)->FindClass(env, "no/mechatronics/sfi/fmi4j/jni/BooleanByReference");
    id = (*env)->GetFieldID(env, cls, "value", "Z");

    fmi2Status (*fmi2CompletedIntegratorStep)(fmi2Component, fmi2Boolean, fmi2Boolean*, fmi2Boolean*);
    fmi2CompletedIntegratorStep = load_function("fmi2CompletedIntegratorStep");
    fmi2Status status = (*fmi2CompletedIntegratorStep)((void*) c, noSetFMUStatePriorToCurrentPoint, &_enterEventMode, &_terminateSimulation);

    (*env)->SetBooleanField(env, enterEventMode, id, _enterEventMode);
    (*env)->SetBooleanField(env, terminateSimulation, id, _terminateSimulation);

    return asJavaEnum(env, status);
}

JNIEXPORT jobject JNICALL Java_no_mechatronics_sfi_fmi4j_jni_FmiLibrary_newDiscreteStates(JNIEnv *env, jobject obj, jlong c, jobject states) {

    jclass cls;

    jfieldID newDiscreteStatesNeeded_id;
    jfieldID terminateSimulation_id;
    jfieldID nominalsOfContinuousStatesChanged_id;
    jfieldID valuesOfContinuousStatesChanged_id;
    jfieldID nextEventTimeDefined_id;
    jfieldID nextEventTime_id;

    cls = (*env)->FindClass(env, "no/mechatronics/sfi/fmi4j/jni/EventInfo");

    newDiscreteStatesNeeded_id = (*env)->GetFieldID(env, cls, "newDiscreteStatesNeeded", "Z");
    terminateSimulation_id = (*env)->GetFieldID(env, cls, "terminateSimulation", "Z");
    nominalsOfContinuousStatesChanged_id = (*env)->GetFieldID(env, cls, "nominalsOfContinuousStatesChanged", "Z");
    valuesOfContinuousStatesChanged_id = (*env)->GetFieldID(env, cls, "valuesOfContinuousStatesChanged", "Z");
    nextEventTimeDefined_id = (*env)->GetFieldID(env, cls, "nextEventTimeDefined", "Z");
    nextEventTime_id = (*env)->GetFieldID(env, cls, "nextEventTime", "D");

    fmi2EventInfo _states = {
        .newDiscreteStatesNeeded = (*env)->GetBooleanField(env, states, newDiscreteStatesNeeded_id),
        .terminateSimulation = (*env)->GetBooleanField(env, states, terminateSimulation_id),
        .nominalsOfContinuousStatesChanged = (*env)->GetBooleanField(env, states, nominalsOfContinuousStatesChanged_id),
        .valuesOfContinuousStatesChanged = (*env)->GetBooleanField(env, states, valuesOfContinuousStatesChanged_id),
        .nextEventTimeDefined = (*env)->GetBooleanField(env, states, nextEventTimeDefined_id),
        .nextEventTime = (*env)->GetDoubleField(env, states, nextEventTime_id),
    };

    fmi2Status (*fmi2NewDiscreteStates)(fmi2Component, fmi2EventInfo*);
    fmi2NewDiscreteStates = load_function("fmi2NewDiscreteStates");
    fmi2Status status = (*fmi2NewDiscreteStates)((void*) c, &_states);

    (*env)->SetBooleanField(env, states, newDiscreteStatesNeeded_id, _states.newDiscreteStatesNeeded);
    (*env)->SetBooleanField(env, states, terminateSimulation_id, _states.terminateSimulation);
    (*env)->SetBooleanField(env, states, nominalsOfContinuousStatesChanged_id, _states.nominalsOfContinuousStatesChanged);
    (*env)->SetBooleanField(env, states, valuesOfContinuousStatesChanged_id, _states.valuesOfContinuousStatesChanged);
    (*env)->SetBooleanField(env, states, nextEventTimeDefined_id, _states.nextEventTimeDefined);
    (*env)->SetDoubleField(env, states, nextEventTime_id, _states.nextEventTime);

    return asJavaEnum(env, status);
}