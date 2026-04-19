#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include "LintalkerVoice.h"

// C engine opaque types
struct shellVar;
typedef struct shellVar *shellVarPtr;

namespace godot {

/*
 * LintalkerSynth – main GDExtension node.
 *
 * Lifecycle:
 *   var synth = LintalkerSynth.new()   # or add as Node
 *   synth.initialize()                  # start the engine
 *   synth.set_builtin_voice(0)          # "Fred"
 *   var wav = synth.speak("Hello")      # returns AudioStreamWAV
 *   $AudioStreamPlayer.stream = wav
 *   $AudioStreamPlayer.play()
 *   synth.shutdown()                    # optional; done in destructor too
 *
 * Custom voices:
 *   var voice = LintalkerVoice.new()
 *   voice.pitch = 120
 *   voice.vibrato_depth = 400
 *   synth.set_voice(voice)
 *   synth.speak("Hello")
 */
class LintalkerSynth : public Node {
    GDCLASS(LintalkerSynth, Node)

public:
    // Lipsync layout constants
    enum {
        LIPSYNC_FLOATS_PER_FRAME = 4,   // floats per frame in get_lipsync_frames()
        LIPSYNC_FPS              = 200, // frames per second (kFrameTime = 5ms)
    };

    // Phoneme opcode constants (match mt4.h enum)
    enum {
        PHONEME_IY=0, PHONEME_IH,  PHONEME_EH,  PHONEME_AE, PHONEME_AA,
        PHONEME_AH,   PHONEME_AO,  PHONEME_UH,  PHONEME_AX, PHONEME_ER,
        PHONEME_EY,   PHONEME_AY,  PHONEME_OY,  PHONEME_AW, PHONEME_OW,
        PHONEME_UW,   PHONEME_YU,  PHONEME_IR,  PHONEME_XR, PHONEME_AR,
        PHONEME_OR,   PHONEME_UR,  PHONEME_IX,  PHONEME_SIL,
        PHONEME_RX,   PHONEME_LX,  PHONEME_EL,  PHONEME_EN,
        PHONEME_W,    PHONEME_Y,   PHONEME_R,   PHONEME_L,  PHONEME_H,
        PHONEME_M,    PHONEME_N,   PHONEME_NG,
        PHONEME_F,    PHONEME_V,   PHONEME_TH,  PHONEME_DH,
        PHONEME_S,    PHONEME_Z,   PHONEME_SH,  PHONEME_ZH,
        PHONEME_P,    PHONEME_B,   PHONEME_T,   PHONEME_D,
        PHONEME_K,    PHONEME_G,   PHONEME_CH,  PHONEME_JH,
    };

    LintalkerSynth();
    ~LintalkerSynth();

    // Engine lifecycle
    bool initialize();
    void shutdown();

    // Voice selection
    bool set_builtin_voice(int index);         // 0-16 built-in voices
    bool set_voice(Ref<LintalkerVoice> voice); // custom voice resource
    int  get_builtin_voice() const { return current_voice_index; }

    // Query built-in voice names
    PackedStringArray get_voice_names() const;

    // Synthesis: returns an AudioStreamWAV (22050 Hz, mono, 16-bit)
    // ready to assign to an AudioStreamPlayer.
    Ref<AudioStreamWAV> speak(const String &text);

    // Returns raw PCM as PackedByteArray (signed 16-bit LE, 22050 Hz mono)
    // for when you want to handle audio yourself.
    PackedByteArray speak_pcm(const String &text);

    // Lipsync -----------------------------------------------------------
    //
    // After speak() / speak_pcm(), returns a flat PackedFloat32Array with
    // LIPSYNC_FLOATS_PER_FRAME (4) values per frame at LIPSYNC_FPS (200 Hz):
    //   [phoneme_id, prev_phoneme_id, next_phoneme_id, blend_0_to_1, ...]
    //
    // Index by frame = int(player.get_playback_position() * LIPSYNC_FPS)
    // clamped to [0, frame_count - 1].
    //
    // phoneme_id values are the engine's internal opcode constants exposed
    // as PHONEME_* integer constants on this class.
    PackedFloat32Array get_lipsync_frames() const;

    // Human-readable name for a phoneme opcode (e.g. 0 -> "IY", 23 -> "SIL")
    String get_phoneme_name(int id) const;

protected:
    static void _bind_methods();

private:
    shellVarPtr svv = nullptr;
    int current_voice_index = 0;
    bool engine_open = false;

    // Apply a LintalkerVoice resource to the open engine channel
    bool _apply_voice(Ref<LintalkerVoice> voice);
};

} // namespace godot
