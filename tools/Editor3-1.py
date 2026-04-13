import wx
import wx.lib.scrolledpanel
import struct
import os
import re
import sys 
import tempfile
import subprocess
import numpy as np
import sounddevice as sd
from functools import partial

# --- Data Model: Field Definitions and Descriptions ---
VOICE_DESC_FIELDS = [
    ("length", "l", 4, False, False), ("creator", "4s", 4, False, False),
    ("id", "L", 4, False, False), ("version", "l", 4, False, False),
    ("name", "64s", 64, True, False), ("comment", "256s", 256, True, False),
    ("gender", "h", 2, False, False), ("age", "h", 2, False, False),
    ("script", "h", 2, False, False), ("language", "h", 2, False, False),
    ("region", "h", 2, False, False), ("reserved1", "l", 4, False, False),
    ("reserved2", "l", 4, False, False), ("reserved3", "l", 4, False, False),
    ("reserved4", "l", 4, False, False),
]

VOICE_DATA_FIELDS = [
    ("pitch", "h", 2, False, False), ("pitchRange", "h", 2, False, False),
    ("stressGain", "h", 2, False, False), ("rate", "h", 2, False, False),
    ("voice_type_sel", "h", 2, False, False), ("vGain", "h", 2, False, False),
    ("aGain", "h", 2, False, False), ("aCycle", "h", 2, False, False),
    ("f4_Freq", "h", 2, False, False), ("f4_BW", "h", 2, False, False),
    ("f4p_Freq", "h", 2, False, False), ("f4p_BW", "h", 2, False, False),
    ("f5p_Freq", "h", 2, False, False), ("f5p_BW", "h", 2, False, False),
    ("f6p_Freq", "h", 2, False, False), ("f6p_BW", "h", 2, False, False),
    ("nasal_Base", "h", 2, False, False), ("nasal_targ", "h", 2, False, False),
    ("nasal_BW", "h", 2, False, False), ("locus", "h", 2, False, False),
    ("bwGain1", "h", 2, False, False), ("bwGain2", "h", 2, False, False),
    ("bwGain3", "h", 2, False, False), ("f1_Offset", "h", 2, False, False),
    ("f2_Offset", "h", 2, False, False), ("f3_Offset", "h", 2, False, False),
    ("chorus", "h", 2, False, False), ("nGain", "h", 2, False, False),
    ("sPitch", "h", 2, False, False), ("sGain", "h", 2, False, False),
    ("AsperW", "h", 2, False, False), ("voiceVers", "h", 2, False, False),
    ("riseAmt", "h", 2, False, False), ("fallAmt", "h", 2, False, False),
    ("riseAmt1", "h", 2, False, False), ("fallAmt1", "h", 2, False, False),
    ("assertiveness", "l", 4, False, False), ("baselineFall", "h", 2, False, False),
    ("quickness", "h", 2, False, False), ("pitchCmdStep", "h", 2, False, False),
    ("durCmdStep", "h", 2, False, False), ("down_Ramp_Step", "l", 4, False, False),
    ("stressDurTime", "h", 2, False, False), ("tempo", "h", 2, False, False),
    ("waveType", "h", 2, False, False),
    ("vWave", "96s", 96, False, True), ("vWave1", "96s", 96, False, True),
    ("sndID", "h", 2, False, False), ("vowelSync", "h", 2, False, False),
    ("loopPoint", "l", 4, False, False), ("customForm", "h", 2, False, False),
    ("nasalAmt", "h", 2, False, False), ("vibratoDepth1", "h", 2, False, False),
    ("vibratoDepth2", "h", 2, False, False), ("vibratoFreq", "h", 2, False, False),
    ("intonation", "h", 2, False, False), ("portamento", "h", 2, False, False),
    ("emphVoice", "h", 2, False, False), ("rvbDelay", "h", 2, False, False),
    ("rvbDepth", "h", 2, False, False), ("rvbWetDry", "h", 2, False, False),
    ("free1", "l", 4, False, False), ("free2", "l", 4, False, False),
    ("free3", "l", 4, False, False), ("free4", "l", 4, False, False),
    ("free5", "l", 4, False, False), ("free6", "l", 4, False, False),
    ("free7", "l", 4, False, False), ("free8", "l", 4, False, False),
    ("notes_count", "h", 2, False, False),
    ("notes_data", "78s", 78, False, True)
]

FIELD_DESCRIPTIONS = { # Using your full list
    "length": "Total length of the VoiceDescription structure in bytes (typically 362).","creator": "4-character creator code of the synthesizer engine (e.g., 'mtk3' for MacinTalk 3, 'gala' for others). This indicates which engine this voice is designed for.","id": "Voice ID number. This should be unique for a given synthesizer creator. (Unsigned Long)","version": "Version of the voice data format (e.g., 0x0104 for MacinTalk version 1.04). Ensures compatibility.","name": "Display name of the voice (e.g., \"Fred\", \"Kathy\"). Pascal-style string, max 63 chars.","comment": "A descriptive comment or a demo phrase for the voice. Pascal-style string, max 255 chars.","gender": "Gender of the voice: 0 for Neuter, 1 for Male, 2 for Female.","age": "Approximate age of the voice in years.","script": "Script code (e.g., MacRoman) that the voice text input is expected to be in.","language": "Language code of the voice's output speech (e.g., for English).","region": "Region code for the voice's output speech (e.g., US, UK).","reserved1": "Reserved for future use by Apple. Should be 0.","reserved2": "Reserved for future use by Apple. Should be 0.","reserved3": "Reserved for future use by Apple. Should be 0.","reserved4": "Reserved for future use by Apple. Should be 0.","pitch": "Base pitch of the voice in Hertz (e.g., 97 for Fred, 200 for Kathy). Typical range 50-500Hz.","pitchRange": "Pitch modulation range (0-100%). Determines how much overall pitch varies for intonation. 0 makes the voice monotonic.","stressGain": "Gain applied to stressed syllables (0-100%). Affects how much pitch and/or amplitude changes for emphasis.","rate": "Speaking rate in nominal words per minute (e.g., 160). Typical range 50-1000.","voice_type_sel": "Selects which set of formant and acoustic tables to use. 0 for Male tables (kMaleTbls), 1 for Female tables (kFemaleTbls).","vGain": "Voiced (glottal) source gain (0-100%). Strength of the harmonic (buzz) component of the voice.","aGain": "Aspiration noise gain (0-100%). Amount of 'h'-like breathy noise mixed into the source.","aCycle": "Aspiration duty cycle (0-255). Affects the character/periodicity of the aspiration noise.","f4_Freq": "Center frequency of the fixed cascade F4 formant (in internal pitch units or Hz, depending on context/initialization).","f4_BW": "Bandwidth of the fixed cascade F4 formant (Hz).","f4p_Freq": "Center frequency of the fixed parallel F4p formant (Hz).","f4p_BW": "Bandwidth of the fixed parallel F4p formant (Hz).","f5p_Freq": "Center frequency of the fixed parallel F5p formant (Hz).","f5p_BW": "Bandwidth of the fixed parallel F5p formant (Hz).","f6p_Freq": "Center frequency of the fixed parallel F6p formant (Hz).","f6p_BW": "Bandwidth of the fixed parallel F6p formant (Hz).","nasal_Base": "Frequency of the nasal pole/zero when not producing a nasal sound (e.g., 330Hz for kNbase1).","nasal_targ": "Target frequency of the nasal pole/zero during nasal sounds (e.g., 400Hz for kNtarg1).","nasal_BW": "Bandwidth of the nasal pole/zero filter (Hz).","locus": "Parameter affecting formant transitions (coarticulation), likely a percentage or scaling factor (0-100+).","bwGain1": "Scaling factor for F1 bandwidth (e.g., 100 for 100%, 150 for 150% of base).","bwGain2": "Scaling factor for F2 bandwidth.","bwGain3": "Scaling factor for F3 bandwidth.","f1_Offset": "Offset in Hz to be added to (or subtracted from) the base F1 formant frequency.","f2_Offset": "Offset in Hz for F2.","f3_Offset": "Offset in Hz for F3.","chorus": "Chorus effect depth. 0 for no chorus. Other signed values create a detuned, thicker sound.","nGain": "Overall noise gain for the parallel formant bank excitation.","sPitch": "MIDI note number for the pitch of a pre-recorded sound sample if waveType is kUseSnd/kUseSyncSnd.","sGain": "Gain for a pre-recorded sound sample (0-100+).","AsperW": "Aspiration waveform selection: 0 for BandNoise, 1 for NoiseWave (white), 2 for HPNoise.","voiceVers": "Internal version of this voiceData structure (e.g., 0x0104 for kVoiceVer).","riseAmt": "Default amount of pitch rise for intonation contours (e.g., kHZ_10, an offset from normal pitch).","fallAmt": "Default amount of pitch fall for intonation contours (e.g., -kHZ_10).","riseAmt1": "Secondary pitch rise amount for more complex intonation.","fallAmt1": "Secondary pitch fall amount.","assertiveness": "Scales the magnitude of pitch falls (0x10000 = 100% of specified fallAmt).","baselineFall": "Amount the baseline pitch declines over a sentence (e.g., kHZ_18).","quickness": "Controls the speed/responsiveness of the pitch filter to target changes (e.g., (50*48)+4800).","pitchCmdStep": "Step size for relative pitch adjustments via embedded commands (e.g., kPitch_Step = 42 units).","durCmdStep": "Multiplier for relative duration adjustments via embedded commands (e.g., kDur_Step = 0x155 for 133%).","down_Ramp_Step": "Rate of sentence declination (pitch fall) per frame (e.g., kDownRampStep = 15360 units).","stressDurTime": "Base duration of the acoustic effect of stress (e.g., 250ms / kFrameTime).","tempo": "Tempo for singing in beats per minute (e.g., 85, 120).","waveType": "Glottal source type: 0=kUseHarm (harmonic synthesis), 1=kUseSnd (pre-recorded sample), 2=kUseSyncSnd (sample with synchronization markers).","vWave": "Array of 48 shorts: Defines amplitudes of the first 48 harmonics for the primary glottal waveform when waveType is kUseHarm. Used by InvDFT to create the buzzing source.","vWave1": "Array of 48 shorts: Harmonic amplitudes for the secondary (chorus) glottal waveform. Used if chorus is active and waveType is kUseHarm.","sndID": "Resource ID of a pre-recorded sound sample to be used if waveType is kUseSnd or kUseSyncSnd.","vowelSync": "Flag (0 or 1): If true and using kUseSnd, re-triggers the sound sample at the beginning of each vowel.","loopPoint": "Byte offset for the loop point within a pre-recorded sound sample. 0 usually means no specific internal loop or loop entire sample.","customForm": "Resource ID for custom formant tables. If 0, uses default internal tables (Male/Female).","nasalAmt": "Offset applied to the nasal zero frequency during nasalization, affecting the spectral notch.","vibratoDepth1": "Primary vibrato depth (e.g., kVib1 = 31, representing ~3.1% pitch variation).","vibratoDepth2": "Secondary vibrato depth, used for less prominent vibrato (e.g., kVib2 = 16).","vibratoFreq": "Vibrato frequency (e.g., kVibFreq = 47, representing ~4.7Hz scaled for internal timing).","intonation": "Overall scaling factor for intonation effects (0-100%).","portamento": "Portamento time in milliseconds for pitch glides between notes in singing mode (e.g., kPort = 15ms).","emphVoice": "Flag (0 or 1): If true, enables a high-frequency emphasis filter for a brighter, crisper sound.","rvbDelay": "Reverb delay time factor. This value (0-100) scales internal tap delay values.","rvbDepth": "Reverb depth or mix level (0-100+). 0 means no reverb.","rvbWetDry": "Reverb wet/dry mix (0=Wet, 1=Dry). Seems to be kDry (1) for all provided voices.","free1": "Reserved for future use by the speech engine.","free2": "Reserved for future use.","free3": "Reserved for future use.","free4": "Reserved for future use.","free5": "Reserved for future use.","free6": "Reserved for future use.","free7": "Reserved for future use.","free8": "Reserved for future use.","notes_count": "Number of musical notes defined in the 'notes_data' array for singing. This is the first element of the C 'notes[40]' array.","notes_data": "Array of up to 39 shorts for musical notes (following notes_count). Each short typically encodes pitch in the lower byte and duration in the higher nibble of the higher byte."
}
HIDDEN_FIELDS = {
    "length", "creator", "id", "version", "gender", "age", "script", "language", "region",
    "reserved1", "reserved2", "reserved3", "reserved4",
    "voiceVers", "riseAmt", "fallAmt", "riseAmt1", "fallAmt1", "assertiveness",
    "baselineFall", "quickness", "pitchCmdStep", "durCmdStep", "down_Ramp_Step",
    "stressDurTime", "sndID", "customForm", "rvbWetDry",
    "free1", "free2", "free3", "free4", "free5", "free6", "free7", "free8"
}

# --- Helper Functions for Note and Pitch Conversion ---
NOTE_NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
DUR_CODE_MAP = {0: 0.25, 1: 0.25, 2: 0.25, 3: 0.5, 4: 0.75, 5: 1.0, 6: 1.5, 7: 2.0}
DUR_VALUE_MAP = {v: k for k, v in DUR_CODE_MAP.items()}

def freq_to_midi(freq): return 69 + 12 * np.log2(freq / 440.0) if freq > 0 else 0
def midi_to_note_name(midi_val):
    if not (0 <= midi_val <= 127): return "???"
    midi_val = int(round(midi_val))
    octave = midi_val // 12 - 1
    note = NOTE_NAMES[midi_val % 12]
    return f"{note}{octave}"
def note_name_to_midi(name_str):
    match = re.match(r"([A-Ga-g])([#b]?)(-?\d+)", name_str.strip())
    if not match: return None
    note_char, acc, oct_str = match.groups()
    base_note = NOTE_NAMES.index(note_char.upper())
    if acc == '#': base_note += 1
    elif acc == 'b': base_note -= 1
    return (int(oct_str) + 1) * 12 + (base_note % 12)

# --- Advanced Editor Dialogs ---
class NoteEditorDialog(wx.Dialog):
    def __init__(self, parent, notes_count, notes_data_bytes, base_pitch_hz):
        super().__init__(parent, title="Note Sequence Editor", size=(500, 400))
        self.notes_count = notes_count
        self.notes_data_bytes = notes_data_bytes
        base_midi = freq_to_midi(base_pitch_hz)

        panel = wx.Panel(self); vbox = wx.BoxSizer(wx.VERTICAL)
        note_strs = []
        for i in range(min(self.notes_count, 39)):
            byte_chunk = self.notes_data_bytes[i*2 : i*2+2]
            if len(byte_chunk) < 2: break
            dur_code, pitch_offset = struct.unpack('>Bb', byte_chunk)
            
            duration = DUR_CODE_MAP.get(dur_code, 0.0)
            note_midi = base_midi + pitch_offset
            note_name = midi_to_note_name(note_midi)
            
            if duration.is_integer():
                note_strs.append(f"{note_name}:{int(duration)}")
            else:
                note_strs.append(f"{note_name}:{duration:.2f}")

        initial_text = ", ".join(note_strs)
        self.text_ctrl = wx.TextCtrl(panel, value=initial_text, style=wx.TE_MULTILINE)
        vbox.Add(wx.StaticText(panel, label=f"Format: Note:Duration (Base: {midi_to_note_name(base_midi)})"), flag=wx.ALL, border=5)
        vbox.Add(self.text_ctrl, 1, wx.EXPAND | wx.ALL, 5)

        btnsizer = wx.StdDialogButtonSizer(); ok_btn = wx.Button(panel, wx.ID_OK); cancel_btn = wx.Button(panel, wx.ID_CANCEL)
        btnsizer.AddButton(ok_btn); btnsizer.AddButton(cancel_btn); btnsizer.Realize()
        vbox.Add(btnsizer, 0, wx.ALL, 10)
        panel.SetSizer(vbox); ok_btn.Bind(wx.EVT_BUTTON, self.on_ok); self.Centre()

    def on_ok(self, event):
        raw_text = self.text_ctrl.GetValue(); base_midi = freq_to_midi(self.Parent.data_store['pitch'])
        note_defs = [s.strip() for s in raw_text.split(',') if s.strip()]
        if len(note_defs) > 39:
            wx.MessageBox("Maximum of 39 notes allowed.", "Error", wx.OK | wx.ICON_ERROR); return
        
        new_packed_notes = []
        for note_def in note_defs:
            parts = note_def.split(':')
            if len(parts) != 2:
                wx.MessageBox(f"Invalid format for '{note_def}'.", "Error", wx.OK | wx.ICON_ERROR); return
            midi_val = note_name_to_midi(parts[0])
            if midi_val is None:
                wx.MessageBox(f"Invalid note name: '{parts[0]}'", "Error", wx.OK | wx.ICON_ERROR); return
            pitch_offset = int(round(midi_val - base_midi))
            if not (-128 <= pitch_offset <= 127):
                wx.MessageBox(f"Pitch offset for {parts[0]} is out of 8-bit range.", "Error", wx.OK | wx.ICON_ERROR); return
            try:
                dur_val = float(parts[1])
                closest_dur = min(DUR_VALUE_MAP.keys(), key=lambda k: abs(k - dur_val))
                dur_code = DUR_VALUE_MAP[closest_dur]
                new_packed_notes.append(struct.pack('>Bb', dur_code, pitch_offset))
            except ValueError:
                wx.MessageBox(f"Invalid duration: '{parts[1]}'", "Error", wx.OK | wx.ICON_ERROR); return
        self.notes_count = len(new_packed_notes); self.notes_data_bytes = b''.join(new_packed_notes)
        self.EndModal(wx.ID_OK)

    def get_values(self): return self.notes_count, self.notes_data_bytes

class WaveEditorDialog(wx.Dialog):
    def __init__(self, parent, wave_data_bytes, base_pitch):
        super().__init__(parent, title="Wave Table Editor", size=(600, 550))
        self.wave_data_bytes = wave_data_bytes
        self.base_pitch = base_pitch
        self.audio_stream = None; self.imported_audio_data = None; self.current_frame = 0; self.waveform = None

        panel = wx.Panel(self); vbox = wx.BoxSizer(wx.VERTICAL)
        current_box = wx.StaticBox(panel, label="Current Waveform (48 samples, unsigned 16-bit)")
        current_sizer = wx.StaticBoxSizer(current_box, wx.VERTICAL)
        self.play_button = wx.Button(panel, label="Play Current Waveform"); self.play_button.Bind(wx.EVT_BUTTON, self.on_play_stop)
        current_sizer.Add(self.play_button, 0, wx.ALL | wx.EXPAND, 5)
        self.wave_text_ctrl = wx.TextCtrl(panel, style=wx.TE_MULTILINE, size=(-1, 100))
        self.update_text_from_bytes()
        current_sizer.Add(self.wave_text_ctrl, 1, wx.EXPAND | wx.ALL, 5)
        apply_btn = wx.Button(panel, label="Apply Text Changes to Waveform"); apply_btn.Bind(wx.EVT_BUTTON, self.on_apply_text)
        current_sizer.Add(apply_btn, 0, wx.EXPAND | wx.ALL, 5)
        vbox.Add(current_sizer, 0, wx.EXPAND | wx.ALL, 5)
        
        import_box = wx.StaticBox(panel, label="Import New Waveform from Audio File")
        import_sizer = wx.StaticBoxSizer(import_box, wx.VERTICAL)
        import_btn = wx.Button(panel, label="Select WAV Audio File..."); import_btn.Bind(wx.EVT_BUTTON, self.on_import_audio)
        import_sizer.Add(import_btn, 0, wx.EXPAND | wx.ALL, 5)
        
        self.frame_selector_panel = wx.Panel(panel)
        frame_sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.frame_spin = wx.SpinCtrl(self.frame_selector_panel, min=0, max=0)
        self.frame_spin.Bind(wx.EVT_SPINCTRL, self.on_frame_selected)
        frame_sizer.Add(wx.StaticText(self.frame_selector_panel, label="Start Sample:"), 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5)
        frame_sizer.Add(self.frame_spin, 1, wx.EXPAND|wx.ALL, 5)
        self.frame_selector_panel.SetSizer(frame_sizer); self.frame_selector_panel.Hide()
        import_sizer.Add(self.frame_selector_panel, 0, wx.EXPAND)
        vbox.Add(import_sizer, 1, wx.EXPAND | wx.ALL, 5)

        btnsizer = wx.StdDialogButtonSizer(); ok_button = wx.Button(panel, wx.ID_OK); cancel_button = wx.Button(panel, wx.ID_CANCEL)
        btnsizer.AddButton(ok_button); btnsizer.AddButton(cancel_button); btnsizer.Realize()
        vbox.Add(btnsizer, 0, wx.ALL | wx.ALIGN_CENTER, 10)
        
        panel.SetSizer(vbox); ok_button.Bind(wx.EVT_BUTTON, self.on_ok); self.Bind(wx.EVT_CLOSE, self.on_close); self.Centre()

    def update_text_from_bytes(self):
        shorts = np.frombuffer(self.wave_data_bytes, dtype=np.uint16)
        self.wave_text_ctrl.SetValue(", ".join(map(str, shorts)))

    def on_apply_text(self, event):
        try:
            str_values = self.wave_text_ctrl.GetValue().split(',')
            new_shorts = [int(s.strip()) for s in str_values if s.strip()]
            if len(new_shorts) != 48:
                wx.MessageBox("Please enter exactly 48 numbers.", "Input Error", wx.OK | wx.ICON_ERROR); return
            new_array = np.array(new_shorts, dtype=np.uint16)
            self.wave_data_bytes = new_array.tobytes()
            wx.MessageBox("Waveform updated from text.", "Success", wx.OK | wx.ICON_INFORMATION)
        except Exception as e:
            wx.MessageBox(f"Error parsing text: {e}", "Error", wx.OK | wx.ICON_ERROR)

    def audio_callback(self, outdata, frames, time, status):
        if status:
            print(status, file=sys.stderr)
        
        indices = (self.current_frame + np.arange(frames)) % len(self.waveform)
        outdata[:] = self.waveform[indices].reshape(-1, 1)
        self.current_frame = (self.current_frame + frames) % len(self.waveform)

    def on_play_stop(self, event):
        if self.audio_stream and self.audio_stream.active:
            self.audio_stream.stop(); self.play_button.SetLabel("Play Current Waveform")
        else:
            try:
                shorts = np.frombuffer(self.wave_data_bytes, dtype=np.uint16).astype(np.float32)
                self.waveform = (shorts / 65535.0 - 0.5) * 2.0 * 0.25
                samplerate = self.base_pitch * 48
                self.current_frame = 0
                self.audio_stream = sd.OutputStream(samplerate=samplerate, channels=1, callback=self.audio_callback)
                self.audio_stream.start(); self.play_button.SetLabel("Stop")
            except Exception as e:
                wx.MessageBox(f"Error playing audio: {e}", "Error", wx.OK | wx.ICON_ERROR)

    def on_import_audio(self, event):
        with wx.FileDialog(self, "Select WAV file", wildcard="WAV files (*.wav)|*.wav", style=wx.FD_OPEN) as dlg:
            if dlg.ShowModal() == wx.ID_CANCEL: return
        fd, temp_path = tempfile.mkstemp(suffix=".pcm_u16le"); os.close(fd)
        try:
            cmd = ["ffmpeg", "-y", "-i", dlg.GetPath(), "-c:a", "pcm_u16le", "-ac", "1", "-f", "u16le", temp_path]
            subprocess.run(cmd, check=True, capture_output=True)
            with open(temp_path, "rb") as f:
                self.imported_audio_data = np.frombuffer(f.read(), dtype=np.uint16)
            self.frame_spin.SetMax(len(self.imported_audio_data) - 48); self.frame_selector_panel.Show(); self.Layout()
        except Exception as e: wx.MessageBox(f"Error converting audio: {e}", "Error", wx.OK | wx.ICON_ERROR)
        finally:
            if os.path.exists(temp_path): os.remove(temp_path)

    def on_frame_selected(self, event):
        if self.imported_audio_data is None: return
        start_sample = self.frame_spin.GetValue()
        frame_shorts = self.imported_audio_data[start_sample : start_sample + 48]
        self.wave_data_bytes = frame_shorts.tobytes()
        self.update_text_from_bytes()

    def on_ok(self, event): self.on_close(None); self.EndModal(wx.ID_OK)
    def get_value(self): return self.wave_data_bytes
    def on_close(self, event):
        if self.audio_stream: self.audio_stream.stop(); self.audio_stream.close()
        if event: self.Destroy()
        else: self.Hide()

# --- Main Application Frame ---
class VoiceEditorFrame(wx.Frame):
    def __init__(self, *args, **kw):
        super(VoiceEditorFrame, self).__init__(*args, **kw)
        self.original_filepath = None; self.full_data_bytes = None
        self.data_store = {}; self.field_metadata = []; self.dirty_flag = False
        self.InitUI(); self.Centre(); self.Show()

    def InitUI(self):
        self.SetTitle("MacinTalk Voice Editor"); self.SetSize((900, 700))
        panel = wx.Panel(self); main_hbox = wx.BoxSizer(wx.HORIZONTAL)
        left_vbox = wx.BoxSizer(wx.VERTICAL); top_controls_sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.open_button = wx.Button(panel, label="Open Voice File..."); self.open_button.Bind(wx.EVT_BUTTON, self.on_open_file)
        self.save_button = wx.Button(panel, label="Save As..."); self.save_button.Bind(wx.EVT_BUTTON, self.on_save_as_file); self.save_button.Disable()
        self.filter_checkbox = wx.CheckBox(panel, label="Hide non-essential fields"); self.filter_checkbox.SetValue(True); self.filter_checkbox.Bind(wx.EVT_CHECKBOX, self.on_filter_changed)
        top_controls_sizer.Add(self.open_button, 0, wx.ALL, 5); top_controls_sizer.Add(self.save_button, 0, wx.ALL, 5)
        top_controls_sizer.Add(self.filter_checkbox, 0, wx.ALL | wx.ALIGN_CENTER_VERTICAL, 5); left_vbox.Add(top_controls_sizer)
        self.list_ctrl = wx.ListCtrl(panel, style=wx.LC_REPORT | wx.LC_SINGLE_SEL)
        self.list_ctrl.InsertColumn(0, "Field Name", width=150); self.list_ctrl.InsertColumn(1, "Value", width=250)
        self.list_ctrl.Bind(wx.EVT_LIST_ITEM_SELECTED, self.on_list_item_selected); left_vbox.Add(self.list_ctrl, 1, wx.EXPAND | wx.ALL, 10)
        main_hbox.Add(left_vbox, 2, wx.EXPAND)
        right_vbox = wx.BoxSizer(wx.VERTICAL)
        self.description_text_ctrl = wx.TextCtrl(panel, style=wx.TE_MULTILINE | wx.TE_READONLY | wx.BORDER_SUNKEN)
        right_vbox.Add(wx.StaticText(panel, label="Description:"), 0, wx.LEFT | wx.TOP, 10)
        right_vbox.Add(self.description_text_ctrl, 1, wx.EXPAND | wx.ALL, 10)
        self.editor_panel = wx.Panel(panel); self.editor_sizer = wx.BoxSizer(wx.VERTICAL)
        self.editor_panel.SetSizer(self.editor_sizer); right_vbox.Add(self.editor_panel, 0, wx.EXPAND | wx.LEFT | wx.RIGHT, 10)
        self.create_editor_widgets()
        main_hbox.Add(right_vbox, 1, wx.EXPAND); panel.SetSizer(main_hbox)

    def create_editor_widgets(self):
        self.numeric_editor_sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.numeric_label = wx.StaticText(self.editor_panel, label="Value:")
        self.numeric_spin = wx.SpinCtrl(self.editor_panel, min=-32768, max=32767)
        self.numeric_spin.Bind(wx.EVT_SPINCTRL, self.on_numeric_value_change); self.numeric_spin.Bind(wx.EVT_TEXT, self.on_numeric_value_change)
        self.numeric_editor_sizer.Add(self.numeric_label, 0, wx.ALL | wx.ALIGN_CENTER_VERTICAL, 5); self.numeric_editor_sizer.Add(self.numeric_spin, 1, wx.EXPAND | wx.ALL, 5)
        self.editor_sizer.Add(self.numeric_editor_sizer)
        self.string_edit_button = wx.Button(self.editor_panel, label="Edit Text..."); self.string_edit_button.Bind(wx.EVT_BUTTON, self.on_edit_string)
        self.note_edit_button = wx.Button(self.editor_panel, label="Edit Note Data..."); self.note_edit_button.Bind(wx.EVT_BUTTON, self.on_edit_notes)
        self.wave_edit_button = wx.Button(self.editor_panel, label="Edit Wave Table..."); self.wave_edit_button.Bind(wx.EVT_BUTTON, self.on_edit_wave)
        self.editor_sizer.Add(self.string_edit_button, 0, wx.ALL, 5); self.editor_sizer.Add(self.note_edit_button, 0, wx.ALL, 5); self.editor_sizer.Add(self.wave_edit_button, 0, wx.ALL, 5)
        self.hide_all_editors()

    def hide_all_editors(self):
        self.numeric_editor_sizer.ShowItems(False); self.string_edit_button.Hide(); self.note_edit_button.Hide(); self.wave_edit_button.Hide()
        self.editor_panel.GetParent().Layout()

    def on_open_file(self, event):
        with wx.FileDialog(self, "Open VoiceDescription file", wildcard="All files (*.*)|*.*", style=wx.FD_OPEN | wx.FD_FILE_MUST_EXIST) as fileDialog:
            if fileDialog.ShowModal() == wx.ID_CANCEL: return
            self.original_filepath = fileDialog.GetPath()
        self.data_store = {}; self.field_metadata = []; self.dirty_flag = False
        try:
            with open(self.original_filepath, 'rb') as f: self.full_data_bytes = f.read()
            offset = 0; all_fields = VOICE_DESC_FIELDS + VOICE_DATA_FIELDS
            for name, fmt, size, is_pascal, is_array in all_fields:
                actual_size = min(size, len(self.full_data_bytes) - offset)
                if actual_size <= 0: continue
                raw_bytes = self.full_data_bytes[offset : offset + actual_size]; offset += actual_size
                val = None
                if is_pascal: val = raw_bytes[1:1+raw_bytes[0]].decode('mac_roman', errors='replace')
                elif is_array: val = raw_bytes
                else:
                    if len(raw_bytes) < struct.calcsize(f'>{fmt}'): continue
                    val = struct.unpack(f'>{fmt}', raw_bytes)[0]
                    if isinstance(val, bytes): val = val.decode('ascii', errors='replace').rstrip('\x00')
                self.data_store[name] = val
                self.field_metadata.append({'name': name, 'fmt': fmt, 'size': size, 'is_pascal': is_pascal, 'is_array': is_array, 'original_bytes': raw_bytes})
        except Exception as e:
            wx.MessageBox(f"Error loading file: {e}", "Error", wx.OK | wx.ICON_ERROR); return
        self.refresh_list_display(); self.save_button.Enable()

    def on_filter_changed(self, event): self.refresh_list_display()
    def refresh_list_display(self):
        self.list_ctrl.DeleteAllItems(); self.hide_all_editors(); self.description_text_ctrl.SetValue("")
        show_all = not self.filter_checkbox.IsChecked()
        for i, meta in enumerate(self.field_metadata):
            name = meta['name']
            if not show_all and name in HIDDEN_FIELDS: continue
            val = self.data_store.get(name)
            display_val = f"Array (size {len(val)})" if meta['is_array'] else str(val)
            idx = self.list_ctrl.InsertItem(self.list_ctrl.GetItemCount(), name)
            self.list_ctrl.SetItem(idx, 1, display_val); self.list_ctrl.SetItemData(idx, i)

    def on_list_item_selected(self, event):
        self.hide_all_editors(); meta_idx = self.list_ctrl.GetItemData(event.GetIndex())
        meta = self.field_metadata[meta_idx]; name = meta['name']; val = self.data_store.get(name)
        self.description_text_ctrl.SetValue(FIELD_DESCRIPTIONS.get(name, "No description available."))
        if name == "notes_data": self.note_edit_button.Show()
        elif name in ["vWave", "vWave1"]: self.wave_edit_button.Show()
        elif meta['is_pascal'] or meta['name'] == 'creator': self.string_edit_button.Show()
        elif not meta['is_array']:
            self.numeric_spin.Unbind(wx.EVT_SPINCTRL); self.numeric_spin.Unbind(wx.EVT_TEXT)
            self.numeric_spin.SetValue(int(val))
            self.numeric_spin.Bind(wx.EVT_SPINCTRL, self.on_numeric_value_change); self.numeric_spin.Bind(wx.EVT_TEXT, self.on_numeric_value_change)
            self.numeric_editor_sizer.ShowItems(True)
        self.editor_panel.GetParent().Layout()

    def on_numeric_value_change(self, event):
        selected_list_idx = self.list_ctrl.GetFirstSelected()
        if selected_list_idx == -1: return
        meta_idx = self.list_ctrl.GetItemData(selected_list_idx)
        name = self.field_metadata[meta_idx]['name']; new_val = self.numeric_spin.GetValue()
        self.data_store[name] = new_val; self.list_ctrl.SetItem(selected_list_idx, 1, str(new_val)); self.dirty_flag = True

    def _get_current_meta(self):
        idx = self.list_ctrl.GetFirstSelected()
        return self.field_metadata[self.list_ctrl.GetItemData(idx)] if idx != -1 else None

    def on_edit_string(self, event):
        meta = self._get_current_meta();
        if not meta: return
        with wx.TextEntryDialog(self, f"Edit {meta['name']}", value=str(self.data_store[meta['name']])) as dlg:
            if dlg.ShowModal() == wx.ID_OK:
                self.data_store[meta['name']] = dlg.GetValue(); self.list_ctrl.SetItem(self.list_ctrl.GetFirstSelected(), 1, dlg.GetValue()); self.dirty_flag = True

    def on_edit_notes(self, event):
        with NoteEditorDialog(self, self.data_store['notes_count'], self.data_store['notes_data'], self.data_store['pitch']) as dlg:
            if dlg.ShowModal() == wx.ID_OK:
                new_count, new_data = dlg.get_values()
                self.data_store['notes_count'] = new_count; self.data_store['notes_data'] = new_data
                self.list_ctrl.SetItem(self.list_ctrl.GetFirstSelected(), 1, f"Array (size {len(new_data)})")
                for i in range(self.list_ctrl.GetItemCount()):
                    if self.field_metadata[self.list_ctrl.GetItemData(i)]['name'] == 'notes_count':
                        self.list_ctrl.SetItem(i, 1, str(new_count)); break
                self.dirty_flag = True

    def on_edit_wave(self, event):
        meta = self._get_current_meta()
        if not meta: return
        with WaveEditorDialog(self, self.data_store[meta['name']], self.data_store['pitch']) as dlg:
            if dlg.ShowModal() == wx.ID_OK:
                new_data = dlg.get_value()
                self.data_store[meta['name']] = new_data
                self.list_ctrl.SetItem(self.list_ctrl.GetFirstSelected(), 1, f"Array (size {len(new_data)})"); self.dirty_flag = True

    def on_save_as_file(self, event):
        if not self.dirty_flag:
            wx.MessageBox("No changes to save.", "Info", wx.OK | wx.ICON_INFORMATION); return
        
        # --- FIX: Get the path *before* the dialog is destroyed ---
        save_filepath = ""
        with wx.FileDialog(self, "Save VoiceDescription File", defaultFile="VoiceDescription", 
                           wildcard="All files (*.*)|*.*", 
                           style=wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT) as dlg:
            if dlg.ShowModal() == wx.ID_OK:
                save_filepath = dlg.GetPath()
            else:
                return # User cancelled

        # Now, proceed with saving using the captured path
        try:
            modified_bytes = bytearray()
            for meta in self.field_metadata:
                name = meta['name']
                if name not in self.data_store:
                    modified_bytes.extend(meta['original_bytes']); continue
                val = self.data_store[name]; packed_data = b''
                if meta['is_array']:
                    packed_data = val
                    if len(packed_data) < meta['size']: packed_data += b'\x00' * (meta['size'] - len(packed_data))
                    packed_data = packed_data[:meta['size']]
                elif meta['is_pascal']:
                    encoded = val.encode('mac_roman', errors='replace'); p_len = min(len(encoded), meta['size'] - 1)
                    packed_data = struct.pack('>B', p_len) + encoded[:p_len]; packed_data += b'\x00' * (meta['size'] - len(packed_data))
                else:
                    if 's' in meta['fmt']: packed_data = val.encode('ascii').ljust(meta['size'], b'\x00')[:meta['size']]
                    else: packed_data = struct.pack(f">{meta['fmt']}", int(val))
                modified_bytes.extend(packed_data)
            if len(modified_bytes) < len(self.full_data_bytes): modified_bytes.extend(self.full_data_bytes[len(modified_bytes):])
            
            with open(save_filepath, 'wb') as f: f.write(modified_bytes)
            
            wx.MessageBox("File saved successfully!", "Success", wx.OK | wx.ICON_INFORMATION); self.dirty_flag = False
        except Exception as e:
            wx.MessageBox(f"Error saving file: {e}", "Error", wx.OK | wx.ICON_ERROR)

if __name__ == '__main__':
    app = wx.App(False)
    frame = VoiceEditorFrame(None)
    app.MainLoop()