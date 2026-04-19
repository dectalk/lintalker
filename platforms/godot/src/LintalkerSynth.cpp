#include "LintalkerSynth.h"

extern "C" {
#include "../../../include/SpeechEqu.h"
#include "../../../include/MT4.h"
#include "../../../include/Fsynth.h"
#include "../../../include/Versions.h"
#include "../../../include/Macintosh.h"
}

#include <string.h>
#include <godot_cpp/core/class_db.hpp>

// C engine API (implemented in GodotPlatform.c)
extern "C" {
    long        _OpenSpeech(void);
    long        _CloseSpeech(shellVarPtr svv);
    long        _SpeakBuffer(shellVarPtr svv, Ptr textBuf, long byteLen, long controlFlags);
    long        _UseVoice(shellVarPtr svv, short index);
    long        _SetCustomVoice(shellVarPtr svv, voiceData *vd, unsigned char *pcmwave);
    void        _InstallLipsync(shellVarPtr svv);
    const float *_GetLipsyncFrames(int *out_count);
    void        _ClearLipsync(shellVarPtr svv);
}

// Engine globals (defined in GodotPlatform.c)
extern "C" {
    extern shellVarPtr  gInstanceStorage;
    extern short        gSpeechIsDone;
}

namespace godot {

// Built-in voice names in the same order as _UseVoice indices
static const char *kBuiltinVoiceNames[] = {
    "Fred", "Kathy", "Princess", "Junior", "Ralph",
    "Whisper", "Zarvox", "Trinoids", "Bubbles", "Boing",
    "Bells", "Hysterical", "Deranged", "GoodNews", "BadNews",
    "PipeOrgan", "Cellos"
};
static const int kNumBuiltinVoices = 17;

LintalkerSynth::LintalkerSynth() {}

LintalkerSynth::~LintalkerSynth()
{
    shutdown();
}

bool LintalkerSynth::initialize()
{
    if (engine_open) return true;
    long err = _OpenSpeech();
    if (err != 0) return false;
    svv = gInstanceStorage;
    engine_open = true;
    // Default to Fred
    _UseVoice(svv, 0);
    return true;
}

void LintalkerSynth::shutdown()
{
    if (!engine_open) return;
    _CloseSpeech(svv);
    svv = nullptr;
    engine_open = false;
}

bool LintalkerSynth::set_builtin_voice(int index)
{
    if (!engine_open) return false;
    if (index < 0 || index >= kNumBuiltinVoices) return false;
    long err = _UseVoice(svv, (short)index);
    if (err != 0) return false;
    current_voice_index = index;
    return true;
}

bool LintalkerSynth::set_voice(Ref<LintalkerVoice> voice)
{
    if (!engine_open || voice.is_null()) return false;
    return _apply_voice(voice);
}

PackedStringArray LintalkerSynth::get_voice_names() const
{
    PackedStringArray arr;
    for (int i = 0; i < kNumBuiltinVoices; i++)
        arr.push_back(String(kBuiltinVoiceNames[i]));
    return arr;
}

Ref<AudioStreamWAV> LintalkerSynth::speak(const String &text)
{
    PackedByteArray pcm = speak_pcm(text);
    if (pcm.is_empty()) return Ref<AudioStreamWAV>();

    Ref<AudioStreamWAV> stream;
    stream.instantiate();
    stream->set_data(pcm);
    stream->set_format(AudioStreamWAV::FORMAT_16_BITS);
    stream->set_mix_rate(22050);
    stream->set_stereo(false);
    return stream;
}

PackedByteArray LintalkerSynth::speak_pcm(const String &text)
{
    PackedByteArray result;
    if (!engine_open) return result;

    CharString utf8 = text.utf8();
    const char *buf = utf8.get_data();
    long len = (long) utf8.length();

    // Arm lipsync accumulator before synthesis
    _InstallLipsync(svv);

    long err = _SpeakBuffer(svv, (Ptr)buf, len, 0);
    if (err != 0) return result;

    short *samples  = svv->outputBuf;
    int    num_samp = (int) svv->outputLen;
    if (!samples || num_samp <= 0) return result;

    result.resize(num_samp * 2);
    memcpy(result.ptrw(), samples, (size_t)(num_samp * 2));
    return result;
}

PackedFloat32Array LintalkerSynth::get_lipsync_frames() const
{
    PackedFloat32Array result;
    int frame_count = 0;
    const float *frames = _GetLipsyncFrames(&frame_count);
    if (!frames || frame_count <= 0) return result;

    result.resize(frame_count * 4);
    memcpy(result.ptrw(), frames, (size_t)(frame_count * 4) * sizeof(float));
    return result;
}

String LintalkerSynth::get_phoneme_name(int id) const
{
    // Matches the enum order in mt4.h
    static const char *names[] = {
        "IY","IH","EH","AE","AA","AH","AO","UH","AX","ER",
        "EY","AY","OY","AW","OW","UW","YU","IR","XR","AR",
        "OR","UR","IX","SIL","RX","LX","EL","EN","W","Y",
        "R","L","H","M","N","NG","F","V","TH","DH",
        "S","Z","SH","ZH","P","B","T","D","K","G",
        "CH","JH","TX","DX","QX","DD",
        "STRESS1","STRESS2","EMPHSTRESS",
        "PRISE","PFALL","DINC","DDEC","SYLL","WORD","PREP","VERB"
    };
    static const int name_count = (int)(sizeof(names) / sizeof(names[0]));
    if (id < 0 || id >= name_count) return String("?");
    return String(names[id]);
}

bool LintalkerSynth::_apply_voice(Ref<LintalkerVoice> voice)
{
    voiceData vd;
    if (!voice->fill_voice_data(&vd)) return false;
    long err = _SetCustomVoice(svv, &vd, nullptr);
    return (err == 0);
}

void LintalkerSynth::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("initialize"),         &LintalkerSynth::initialize);
    ClassDB::bind_method(D_METHOD("shutdown"),           &LintalkerSynth::shutdown);

    ClassDB::bind_method(D_METHOD("set_builtin_voice", "index"), &LintalkerSynth::set_builtin_voice);
    ClassDB::bind_method(D_METHOD("get_builtin_voice"),          &LintalkerSynth::get_builtin_voice);
    ClassDB::bind_method(D_METHOD("set_voice", "voice"),         &LintalkerSynth::set_voice);
    ClassDB::bind_method(D_METHOD("get_voice_names"),            &LintalkerSynth::get_voice_names);

    ClassDB::bind_method(D_METHOD("speak",     "text"), &LintalkerSynth::speak);
    ClassDB::bind_method(D_METHOD("speak_pcm", "text"), &LintalkerSynth::speak_pcm);

    // Lipsync
    ClassDB::bind_method(D_METHOD("get_lipsync_frames"),       &LintalkerSynth::get_lipsync_frames);
    ClassDB::bind_method(D_METHOD("get_phoneme_name", "id"),   &LintalkerSynth::get_phoneme_name);

    // Constants — lipsync frame layout
    BIND_CONSTANT(LIPSYNC_FLOATS_PER_FRAME);
    BIND_CONSTANT(LIPSYNC_FPS);

    // Phoneme opcode constants (match mt4.h enum order)
    BIND_CONSTANT(PHONEME_IY);  BIND_CONSTANT(PHONEME_IH);  BIND_CONSTANT(PHONEME_EH);
    BIND_CONSTANT(PHONEME_AE);  BIND_CONSTANT(PHONEME_AA);  BIND_CONSTANT(PHONEME_AH);
    BIND_CONSTANT(PHONEME_AO);  BIND_CONSTANT(PHONEME_UH);  BIND_CONSTANT(PHONEME_AX);
    BIND_CONSTANT(PHONEME_ER);  BIND_CONSTANT(PHONEME_EY);  BIND_CONSTANT(PHONEME_AY);
    BIND_CONSTANT(PHONEME_OY);  BIND_CONSTANT(PHONEME_AW);  BIND_CONSTANT(PHONEME_OW);
    BIND_CONSTANT(PHONEME_UW);  BIND_CONSTANT(PHONEME_YU);
    BIND_CONSTANT(PHONEME_SIL);
    BIND_CONSTANT(PHONEME_M);   BIND_CONSTANT(PHONEME_N);   BIND_CONSTANT(PHONEME_NG);
    BIND_CONSTANT(PHONEME_P);   BIND_CONSTANT(PHONEME_B);   BIND_CONSTANT(PHONEME_T);
    BIND_CONSTANT(PHONEME_D);   BIND_CONSTANT(PHONEME_K);   BIND_CONSTANT(PHONEME_G);
    BIND_CONSTANT(PHONEME_F);   BIND_CONSTANT(PHONEME_V);   BIND_CONSTANT(PHONEME_TH);
    BIND_CONSTANT(PHONEME_DH);  BIND_CONSTANT(PHONEME_S);   BIND_CONSTANT(PHONEME_Z);
    BIND_CONSTANT(PHONEME_SH);  BIND_CONSTANT(PHONEME_ZH);
    BIND_CONSTANT(PHONEME_CH);  BIND_CONSTANT(PHONEME_JH);
    BIND_CONSTANT(PHONEME_R);   BIND_CONSTANT(PHONEME_L);
    BIND_CONSTANT(PHONEME_W);   BIND_CONSTANT(PHONEME_Y);   BIND_CONSTANT(PHONEME_H);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "builtin_voice"), "set_builtin_voice", "get_builtin_voice");
}

} // namespace godot
