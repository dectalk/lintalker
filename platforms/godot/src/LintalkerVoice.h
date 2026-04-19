#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>

// Forward-declare the C voiceData struct to avoid pulling all engine headers
// into C++ compilation units.  The actual struct is defined in Fsynth.h.
struct voiceData;

namespace godot {

/*
 * LintalkerVoice – GDScript-accessible Resource that exposes every
 * voiceData field as a named property.
 *
 * Conceptual groupings (matching MacinTalk 3 VoiceDescription fields):
 *
 *   PROSODY  pitch, pitch_range, stress_gain, rate, intonation, assertiveness,
 *            baseline_fall, quickness, pitch_cmd_step, dur_cmd_step,
 *            rise_amt, fall_amt, rise_amt1, fall_amt1, portamento, tempo
 *
 *   TIMBRE   voice_gender, voiced_gain, aspiration_gain, aspiration_cycle,
 *            nasal_gain, nasality_base, nasality_target, nasality_bw, locus,
 *            bw_gain1/2/3, f1_offset, f2_offset, f3_offset,
 *            f4_freq, f4_bw, f4p_freq, f4p_bw,
 *            f5p_freq, f5p_bw, f6p_freq, f6p_bw, chorus
 *
 *   SOURCE   wave_type, spitch, sgain, asper_width,
 *            vwave, vwave1 (PackedFloat32Array of 48 values each)
 *            snd_id, vowel_sync, loop_point
 *
 *   VIBRATO  vibrato_depth, vibrato_depth2, vibrato_freq
 *
 *   REVERB   reverb_delay, reverb_depth
 */
class LintalkerVoice : public Resource {
    GDCLASS(LintalkerVoice, Resource)

public:
    // --- Prosody -------------------------------------------------------
    int   pitch          = 65;
    int   pitch_range    = 40;
    int   stress_gain    = 50;
    int   rate           = 200;
    int   intonation     = 1;
    int   assertiveness  = 70;
    int   baseline_fall  = 18;
    int   quickness      = 45;
    int   pitch_cmd_step = 24;
    int   dur_cmd_step   = 24;
    int   rise_amt       = 200;
    int   fall_amt       = 130;
    int   rise_amt1      = 140;
    int   fall_amt1      = 80;
    int   portamento     = 0;
    int   tempo          = 85;

    // --- Timbre --------------------------------------------------------
    int   voice_gender       = 0;   // 0 = male tables, 1 = female
    int   voiced_gain        = 58;
    int   aspiration_gain    = 0;
    int   aspiration_cycle   = 0;
    int   nasal_gain         = 0;
    int   nasality_base      = 0;
    int   nasality_target    = 0;
    int   nasality_bw        = 0;
    int   locus              = 0;
    int   bw_gain1           = 0;
    int   bw_gain2           = 0;
    int   bw_gain3           = 0;
    int   f1_offset          = 0;
    int   f2_offset          = 0;
    int   f3_offset          = 0;
    int   f4_freq            = 3300;
    int   f4_bw              = 580;
    int   f4p_freq           = 3300;
    int   f4p_bw             = 580;
    int   f5p_freq           = 3750;
    int   f5p_bw             = 1400;
    int   f6p_freq           = 4900;
    int   f6p_bw             = 1000;
    int   chorus             = 0;

    // --- Source --------------------------------------------------------
    int   wave_type      = 0;   // 0=kUseHarm, 2=kUseSnd, 3=kUseSyncSnd
    int   spitch         = 60;  // MIDI note for sample root
    int   sgain          = 80;
    int   asper_width    = 0;
    int   snd_id         = 0;
    int   vowel_sync     = 0;
    int   loop_point     = 0;
    PackedFloat32Array vwave;   // 48 values: glottal waveform table
    PackedFloat32Array vwave1;  // 48 values: alternate waveform table

    // --- Vibrato -------------------------------------------------------
    int   vibrato_depth  = 0;
    int   vibrato_depth2 = 0;
    int   vibrato_freq   = 0;

    // --- Reverb --------------------------------------------------------
    int   reverb_delay   = 0;
    int   reverb_depth   = 0;

    LintalkerVoice();
    ~LintalkerVoice() {}

    // Fills a C voiceData struct from this resource's properties.
    // Returns true on success.
    bool fill_voice_data(voiceData *vd) const;

    // Setters / getters (GDScript properties)
    void set_pitch(int v)          { pitch = v; }
    int  get_pitch() const         { return pitch; }
    void set_pitch_range(int v)    { pitch_range = v; }
    int  get_pitch_range() const   { return pitch_range; }
    void set_stress_gain(int v)    { stress_gain = v; }
    int  get_stress_gain() const   { return stress_gain; }
    void set_rate(int v)           { rate = v; }
    int  get_rate() const          { return rate; }
    void set_intonation(int v)     { intonation = v; }
    int  get_intonation() const    { return intonation; }
    void set_assertiveness(int v)  { assertiveness = v; }
    int  get_assertiveness() const { return assertiveness; }
    void set_baseline_fall(int v)  { baseline_fall = v; }
    int  get_baseline_fall() const { return baseline_fall; }
    void set_quickness(int v)      { quickness = v; }
    int  get_quickness() const     { return quickness; }
    void set_pitch_cmd_step(int v) { pitch_cmd_step = v; }
    int  get_pitch_cmd_step() const{ return pitch_cmd_step; }
    void set_dur_cmd_step(int v)   { dur_cmd_step = v; }
    int  get_dur_cmd_step() const  { return dur_cmd_step; }
    void set_rise_amt(int v)       { rise_amt = v; }
    int  get_rise_amt() const      { return rise_amt; }
    void set_fall_amt(int v)       { fall_amt = v; }
    int  get_fall_amt() const      { return fall_amt; }
    void set_rise_amt1(int v)      { rise_amt1 = v; }
    int  get_rise_amt1() const     { return rise_amt1; }
    void set_fall_amt1(int v)      { fall_amt1 = v; }
    int  get_fall_amt1() const     { return fall_amt1; }
    void set_portamento(int v)     { portamento = v; }
    int  get_portamento() const    { return portamento; }
    void set_tempo(int v)          { tempo = v; }
    int  get_tempo() const         { return tempo; }

    void set_voice_gender(int v)       { voice_gender = v; }
    int  get_voice_gender() const      { return voice_gender; }
    void set_voiced_gain(int v)        { voiced_gain = v; }
    int  get_voiced_gain() const       { return voiced_gain; }
    void set_aspiration_gain(int v)    { aspiration_gain = v; }
    int  get_aspiration_gain() const   { return aspiration_gain; }
    void set_aspiration_cycle(int v)   { aspiration_cycle = v; }
    int  get_aspiration_cycle() const  { return aspiration_cycle; }
    void set_nasal_gain(int v)         { nasal_gain = v; }
    int  get_nasal_gain() const        { return nasal_gain; }
    void set_nasality_base(int v)      { nasality_base = v; }
    int  get_nasality_base() const     { return nasality_base; }
    void set_nasality_target(int v)    { nasality_target = v; }
    int  get_nasality_target() const   { return nasality_target; }
    void set_nasality_bw(int v)        { nasality_bw = v; }
    int  get_nasality_bw() const       { return nasality_bw; }
    void set_locus(int v)              { locus = v; }
    int  get_locus() const             { return locus; }
    void set_bw_gain1(int v)           { bw_gain1 = v; }
    int  get_bw_gain1() const          { return bw_gain1; }
    void set_bw_gain2(int v)           { bw_gain2 = v; }
    int  get_bw_gain2() const          { return bw_gain2; }
    void set_bw_gain3(int v)           { bw_gain3 = v; }
    int  get_bw_gain3() const          { return bw_gain3; }
    void set_f1_offset(int v)          { f1_offset = v; }
    int  get_f1_offset() const         { return f1_offset; }
    void set_f2_offset(int v)          { f2_offset = v; }
    int  get_f2_offset() const         { return f2_offset; }
    void set_f3_offset(int v)          { f3_offset = v; }
    int  get_f3_offset() const         { return f3_offset; }
    void set_f4_freq(int v)            { f4_freq = v; }
    int  get_f4_freq() const           { return f4_freq; }
    void set_f4_bw(int v)              { f4_bw = v; }
    int  get_f4_bw() const             { return f4_bw; }
    void set_f4p_freq(int v)           { f4p_freq = v; }
    int  get_f4p_freq() const          { return f4p_freq; }
    void set_f4p_bw(int v)             { f4p_bw = v; }
    int  get_f4p_bw() const            { return f4p_bw; }
    void set_f5p_freq(int v)           { f5p_freq = v; }
    int  get_f5p_freq() const          { return f5p_freq; }
    void set_f5p_bw(int v)             { f5p_bw = v; }
    int  get_f5p_bw() const            { return f5p_bw; }
    void set_f6p_freq(int v)           { f6p_freq = v; }
    int  get_f6p_freq() const          { return f6p_freq; }
    void set_f6p_bw(int v)             { f6p_bw = v; }
    int  get_f6p_bw() const            { return f6p_bw; }
    void set_chorus(int v)             { chorus = v; }
    int  get_chorus() const            { return chorus; }

    void set_wave_type(int v)          { wave_type = v; }
    int  get_wave_type() const         { return wave_type; }
    void set_spitch(int v)             { spitch = v; }
    int  get_spitch() const            { return spitch; }
    void set_sgain(int v)              { sgain = v; }
    int  get_sgain() const             { return sgain; }
    void set_asper_width(int v)        { asper_width = v; }
    int  get_asper_width() const       { return asper_width; }
    void set_snd_id(int v)             { snd_id = v; }
    int  get_snd_id() const            { return snd_id; }
    void set_vowel_sync(int v)         { vowel_sync = v; }
    int  get_vowel_sync() const        { return vowel_sync; }
    void set_loop_point(int v)         { loop_point = v; }
    int  get_loop_point() const        { return loop_point; }
    void set_vwave(const PackedFloat32Array &v)  { vwave = v; }
    PackedFloat32Array get_vwave() const         { return vwave; }
    void set_vwave1(const PackedFloat32Array &v) { vwave1 = v; }
    PackedFloat32Array get_vwave1() const        { return vwave1; }

    void set_vibrato_depth(int v)      { vibrato_depth = v; }
    int  get_vibrato_depth() const     { return vibrato_depth; }
    void set_vibrato_depth2(int v)     { vibrato_depth2 = v; }
    int  get_vibrato_depth2() const    { return vibrato_depth2; }
    void set_vibrato_freq(int v)       { vibrato_freq = v; }
    int  get_vibrato_freq() const      { return vibrato_freq; }

    void set_reverb_delay(int v)       { reverb_delay = v; }
    int  get_reverb_delay() const      { return reverb_delay; }
    void set_reverb_depth(int v)       { reverb_depth = v; }
    int  get_reverb_depth() const      { return reverb_depth; }

protected:
    static void _bind_methods();
};

} // namespace godot
