#include "LintalkerVoice.h"

// Engine headers (C) — MacTypes.h must come before Fsynth.h for the Fixed typedef
extern "C" {
#include "../../../include/MacTypes.h"
#include "../../../include/Fsynth.h"
}

#include <string.h>

// Convenience macro: bind getter+setter and register an INT property
#define BIND_INT_PROP(name) \
    ClassDB::bind_method(D_METHOD("set_" #name, "v"), &LintalkerVoice::set_##name); \
    ClassDB::bind_method(D_METHOD("get_" #name),      &LintalkerVoice::get_##name); \
    ADD_PROPERTY(PropertyInfo(Variant::INT, #name), "set_" #name, "get_" #name)

namespace godot {

LintalkerVoice::LintalkerVoice()
{
    // Pre-size the waveform arrays to 48 elements (all zero = harmonic default)
    vwave.resize(48);
    vwave1.resize(48);
}

bool LintalkerVoice::fill_voice_data(voiceData *vd) const
{
    if (!vd) return false;
    memset(vd, 0, sizeof(voiceData));

    // Prosody
    vd->pitch          = (short) pitch;
    vd->pitchRange     = (short) pitch_range;
    vd->stressGain     = (short) stress_gain;
    vd->rate           = (short) rate;
    vd->intonation     = (short) intonation;
    vd->assertiveness  = (int32_t) assertiveness;
    vd->baselineFall   = (short) baseline_fall;
    vd->quickness      = (short) quickness;
    vd->pitchCmdStep   = (short) pitch_cmd_step;
    vd->durCmdStep     = (short) dur_cmd_step;
    vd->riseAmt        = (short) rise_amt;
    vd->fallAmt        = (short) fall_amt;
    vd->riseAmt1       = (short) rise_amt1;
    vd->fallAmt1       = (short) fall_amt1;
    vd->portamento     = (short) portamento;
    vd->tempo          = (short) tempo;
    vd->down_Ramp_Step = 15360;  // kDownRampStep default

    // Timbre
    vd->voice       = (short) voice_gender;
    vd->vGain       = (short) voiced_gain;
    vd->aGain       = (short) aspiration_gain;
    vd->aCycle      = (short) aspiration_cycle;
    vd->nGain       = (short) nasal_gain;
    vd->nasal_Base  = (short) nasality_base;
    vd->nasal_targ  = (short) nasality_target;
    vd->nasal_BW    = (short) nasality_bw;
    vd->locus       = (short) locus;
    vd->bwGain1     = (short) bw_gain1;
    vd->bwGain2     = (short) bw_gain2;
    vd->bwGain3     = (short) bw_gain3;
    vd->f1_Offset   = (short) f1_offset;
    vd->f2_Offset   = (short) f2_offset;
    vd->f3_Offset   = (short) f3_offset;
    vd->f4_Freq     = (short) f4_freq;
    vd->f4_BW       = (short) f4_bw;
    vd->f4p_Freq    = (short) f4p_freq;
    vd->f4p_BW      = (short) f4p_bw;
    vd->f5p_Freq    = (short) f5p_freq;
    vd->f5p_BW      = (short) f5p_bw;
    vd->f6p_Freq    = (short) f6p_freq;
    vd->f6p_BW      = (short) f6p_bw;
    vd->chorus      = (short) chorus;
    vd->nasalAmt    = (short) nasality_base;

    // Source
    vd->waveType    = (short) wave_type;
    vd->sPitch      = (short) spitch;
    vd->sGain       = (short) sgain;
    vd->AsperW      = (short) asper_width;
    vd->sndID       = (short) snd_id;
    vd->vowelSync   = (short) vowel_sync;
    vd->loopPoint   = (int32_t) loop_point;

    // Waveform tables (clamped to 48 entries)
    for (int i = 0; i < 48; i++) {
        vd->vWave[i]  = (i < vwave.size())  ? (short)(int)vwave[i]  : 0;
        vd->vWave1[i] = (i < vwave1.size()) ? (short)(int)vwave1[i] : 0;
    }

    // Vibrato
    vd->vibratoDepth1 = (short) vibrato_depth;
    vd->vibratoDepth2 = (short) vibrato_depth2;
    vd->vibratoFreq   = (short) vibrato_freq;

    // Reverb
    vd->rvbDelay  = (short) reverb_delay;
    vd->rvbDepth  = (short) reverb_depth;

    // stressDurTime default: 250ms / 5ms per frame = 50 frames
    vd->stressDurTime = 50;

    return true;
}

void LintalkerVoice::_bind_methods()
{
    // --- Prosody -------------------------------------------------------
    ADD_GROUP("Prosody", "");
    BIND_INT_PROP(pitch);
    BIND_INT_PROP(pitch_range);
    BIND_INT_PROP(stress_gain);
    BIND_INT_PROP(rate);
    BIND_INT_PROP(intonation);
    BIND_INT_PROP(assertiveness);
    BIND_INT_PROP(baseline_fall);
    BIND_INT_PROP(quickness);
    BIND_INT_PROP(pitch_cmd_step);
    BIND_INT_PROP(dur_cmd_step);
    BIND_INT_PROP(rise_amt);
    BIND_INT_PROP(fall_amt);
    BIND_INT_PROP(rise_amt1);
    BIND_INT_PROP(fall_amt1);
    BIND_INT_PROP(portamento);
    BIND_INT_PROP(tempo);

    // --- Timbre --------------------------------------------------------
    ADD_GROUP("Timbre", "");
    BIND_INT_PROP(voice_gender);
    BIND_INT_PROP(voiced_gain);
    BIND_INT_PROP(aspiration_gain);
    BIND_INT_PROP(aspiration_cycle);
    BIND_INT_PROP(nasal_gain);
    BIND_INT_PROP(nasality_base);
    BIND_INT_PROP(nasality_target);
    BIND_INT_PROP(nasality_bw);
    BIND_INT_PROP(locus);
    BIND_INT_PROP(bw_gain1);
    BIND_INT_PROP(bw_gain2);
    BIND_INT_PROP(bw_gain3);
    BIND_INT_PROP(f1_offset);
    BIND_INT_PROP(f2_offset);
    BIND_INT_PROP(f3_offset);
    BIND_INT_PROP(f4_freq);
    BIND_INT_PROP(f4_bw);
    BIND_INT_PROP(f4p_freq);
    BIND_INT_PROP(f4p_bw);
    BIND_INT_PROP(f5p_freq);
    BIND_INT_PROP(f5p_bw);
    BIND_INT_PROP(f6p_freq);
    BIND_INT_PROP(f6p_bw);
    BIND_INT_PROP(chorus);

    // --- Source --------------------------------------------------------
    ADD_GROUP("Source", "");
    BIND_INT_PROP(wave_type);
    BIND_INT_PROP(spitch);
    BIND_INT_PROP(sgain);
    BIND_INT_PROP(asper_width);
    BIND_INT_PROP(snd_id);
    BIND_INT_PROP(vowel_sync);
    BIND_INT_PROP(loop_point);

    ClassDB::bind_method(D_METHOD("set_vwave",  "v"), &LintalkerVoice::set_vwave);
    ClassDB::bind_method(D_METHOD("get_vwave"),       &LintalkerVoice::get_vwave);
    ClassDB::bind_method(D_METHOD("set_vwave1", "v"), &LintalkerVoice::set_vwave1);
    ClassDB::bind_method(D_METHOD("get_vwave1"),      &LintalkerVoice::get_vwave1);
    ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "vwave"),  "set_vwave",  "get_vwave");
    ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "vwave1"), "set_vwave1", "get_vwave1");

    // --- Vibrato -------------------------------------------------------
    ADD_GROUP("Vibrato", "vibrato_");
    BIND_INT_PROP(vibrato_depth);
    BIND_INT_PROP(vibrato_depth2);
    BIND_INT_PROP(vibrato_freq);

    // --- Reverb --------------------------------------------------------
    ADD_GROUP("Reverb", "reverb_");
    BIND_INT_PROP(reverb_delay);
    BIND_INT_PROP(reverb_depth);
}

} // namespace godot
