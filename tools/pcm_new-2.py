import wx
import struct
import subprocess
import os
import tempfile
import numpy as np
import sounddevice as sd
import math
from functools import partial

# --- Constants ---
OUTPUT_SAMPLE_RATE = 11025 # Hz
PREVIEW_DURATION_SEC = 0.20 # 200ms

# --- Dialogs (Unchanged) ---
class HeaderViewFrame(wx.Frame):
    def __init__(self, parent, header_info_text):
        super().__init__(parent, title="PCMWave Header Information", size=(500, 400))
        panel = wx.Panel(self)
        main_sizer = wx.BoxSizer(wx.VERTICAL)
        info_text_ctrl = wx.TextCtrl(panel, style=wx.TE_MULTILINE | wx.TE_READONLY | wx.HSCROLL)
        info_text_ctrl.SetValue(header_info_text)
        main_sizer.Add(info_text_ctrl, 1, wx.EXPAND | wx.ALL, 5)
        close_btn = wx.Button(panel, label="Close")
        close_btn.Bind(wx.EVT_BUTTON, lambda event: self.Close())
        main_sizer.Add(close_btn, 0, wx.ALL, 5)
        panel.SetSizer(main_sizer)
        self.CenterOnParent()
        self.Show()

class PCMWaveGenFrame(wx.Frame):
    def __init__(self):
        super().__init__(None, title="PCMWave file generator", size=(700, 650))
        self.panel = wx.ScrolledWindow(self, -1)
        self.panel.SetScrollRate(0, 20)
        
        self.main_sizer = wx.BoxSizer(wx.VERTICAL)
        self.markers = []
        self.current_audio_path = ""
        self.current_pcm_data_for_preview = None
        self.current_audio_duration_sec = 0.0
        self.selected_marker_index = -1
        # --- NEW: State for cursor position ---
        self.cursor_time_sec = 0.0
        
        # --- NEW: ID for new shortcut ---
        self.ID_ADD_MARKER_AT_CURSOR = wx.NewIdRef()

        self.InitUI()
        self.panel.SetSizer(self.main_sizer)
        self.Layout()
        self.Show()

        # --- NEW: Added Ctrl+M shortcut ---
        accel_tbl = wx.AcceleratorTable([
            (wx.ACCEL_CTRL, ord('P'), wx.ID_PREVIEW),
            (wx.ACCEL_CTRL, ord('M'), self.ID_ADD_MARKER_AT_CURSOR)
        ])
        self.SetAcceleratorTable(accel_tbl)
        self.Bind(wx.EVT_MENU, self.OnPreviewMarker, id=wx.ID_PREVIEW)
        self.Bind(wx.EVT_MENU, self.OnAddMarkerAtCursor, id=self.ID_ADD_MARKER_AT_CURSOR)

    def InitUI(self):
        gen_input_box = wx.StaticBox(self.panel, label="Generate New PCMWave")
        gen_input_sizer = wx.StaticBoxSizer(gen_input_box, wx.VERTICAL)

        file_sizer = wx.BoxSizer(wx.HORIZONTAL)
        select_audio_btn = wx.Button(self.panel, label="Select Input Audio...")
        select_audio_btn.Bind(wx.EVT_BUTTON, self.OnSelectInputAudio)
        self.selected_audio_label = wx.StaticText(self.panel, label="No file selected.")
        file_sizer.Add(select_audio_btn, 0, wx.ALL, 5)
        file_sizer.Add(self.selected_audio_label, 1, wx.ALL, 5)
        gen_input_sizer.Add(file_sizer, 0, wx.EXPAND | wx.ALL, 5)
        
        marker_box = wx.StaticBox(self.panel, label="Audio Markers")
        marker_sizer = wx.StaticBoxSizer(marker_box, wx.VERTICAL)
        
        self.marker_list_ctrl = wx.ListCtrl(self.panel, style=wx.LC_REPORT | wx.LC_SINGLE_SEL, size=(-1, 150))
        self.marker_list_ctrl.InsertColumn(0, "Time (s)", width=120)
        self.marker_list_ctrl.Bind(wx.EVT_LIST_ITEM_SELECTED, self.OnMarkerSelected)
        self.marker_list_ctrl.Bind(wx.EVT_LIST_ITEM_DESELECTED, self.OnMarkerDeselected)
        marker_sizer.Add(self.marker_list_ctrl, 1, wx.EXPAND | wx.ALL, 5)

        marker_btn_sizer = wx.BoxSizer(wx.HORIZONTAL)
        # --- NEW: Changed 'Add Markers' button ---
        add_marker_btn = wx.Button(self.panel, label="Add Marker at Cursor (Ctrl+M)")
        add_marker_btn.Bind(wx.EVT_BUTTON, self.OnAddMarkerAtCursor)
        
        remove_marker_btn = wx.Button(self.panel, label="Remove Selected")
        remove_marker_btn.Bind(wx.EVT_BUTTON, self.OnRemoveMarker)
        remove_all_markers_btn = wx.Button(self.panel, label="Remove All Markers")
        remove_all_markers_btn.Bind(wx.EVT_BUTTON, self.OnRemoveAllMarkers)
        self.preview_marker_btn = wx.Button(self.panel, label="Preview (Ctrl+P)")
        self.preview_marker_btn.Bind(wx.EVT_BUTTON, self.OnPreviewMarker)
        self.preview_marker_btn.Disable()
        marker_btn_sizer.Add(add_marker_btn, 0, wx.ALL, 5)
        marker_btn_sizer.Add(remove_marker_btn, 0, wx.ALL, 5)
        marker_btn_sizer.Add(remove_all_markers_btn, 0, wx.ALL, 5)
        marker_btn_sizer.Add(self.preview_marker_btn, 0, wx.ALL, 5)
        marker_sizer.Add(marker_btn_sizer, 0)

        self.marker_editor_panel = wx.Panel(self.panel)
        editor_sizer = wx.BoxSizer(wx.HORIZONTAL)
        
        # --- NEW: Label for the spin control is now a separate widget ---
        self.time_spin_label = wx.StaticText(self.marker_editor_panel, label="Cursor Time (s):", size=(100,-1))
        
        self.time_spin_ctrl = wx.SpinCtrlDouble(self.marker_editor_panel, min=0.0, max=9999.0, inc=0.001)
        self.time_spin_ctrl.SetDigits(3)
        self.time_spin_ctrl.Bind(wx.EVT_SPINCTRLDOUBLE, self.OnMarkerTimeChange)
        self.time_spin_ctrl.Bind(wx.EVT_TEXT, self.OnMarkerTimeChange)

        btn_minus_100ms = wx.Button(self.marker_editor_panel, label="-100ms", size=(70, -1))
        btn_plus_100ms = wx.Button(self.marker_editor_panel, label="+100ms", size=(70, -1))
        btn_go_start = wx.Button(self.marker_editor_panel, label="Go to Start")
        btn_go_end = wx.Button(self.marker_editor_panel, label="Go to End")

        btn_minus_100ms.Bind(wx.EVT_BUTTON, partial(self.OnAdjustMarkerTime, -0.1))
        btn_plus_100ms.Bind(wx.EVT_BUTTON, partial(self.OnAdjustMarkerTime, +0.1))
        btn_go_start.Bind(wx.EVT_BUTTON, self.OnGoToStart)
        btn_go_end.Bind(wx.EVT_BUTTON, self.OnGoToEnd)
        
        editor_sizer.Add(self.time_spin_label, 0, wx.ALL | wx.ALIGN_CENTER_VERTICAL, 5)
        editor_sizer.Add(self.time_spin_ctrl, 1, wx.EXPAND | wx.ALL, 5)
        editor_sizer.Add(btn_minus_100ms, 0, wx.ALL, 5)
        editor_sizer.Add(btn_plus_100ms, 0, wx.ALL, 5)
        editor_sizer.Add(btn_go_start, 0, wx.ALL, 5)
        editor_sizer.Add(btn_go_end, 0, wx.ALL, 5)
        
        self.marker_editor_panel.SetSizer(editor_sizer)
        self.marker_editor_panel.Hide()
        marker_sizer.Add(self.marker_editor_panel, 0, wx.EXPAND | wx.ALL, 5)
        
        gen_input_sizer.Add(marker_sizer, 0, wx.EXPAND | wx.ALL, 5)
        generate_btn = wx.Button(self.panel, label="Generate PCMWave File")
        generate_btn.Bind(wx.EVT_BUTTON, self.OnGenerate)
        gen_input_sizer.Add(generate_btn, 0, wx.ALL, 5)
        self.main_sizer.Add(gen_input_sizer, 0, wx.EXPAND | wx.ALL, 5)

        view_box = wx.StaticBox(self.panel, label="View Existing PCMWave")
        view_sizer = wx.StaticBoxSizer(view_box, wx.VERTICAL)
        view_btn = wx.Button(self.panel, label="View PCMWave Header...")
        view_btn.Bind(wx.EVT_BUTTON, self.OnViewPCMWave)
        view_sizer.Add(view_btn, 0, wx.ALL, 5)
        self.main_sizer.Add(view_sizer, 0, wx.EXPAND | wx.ALL, 5)
        
        self.status_text = wx.StaticText(self.panel, label="Status: Idle")
        self.main_sizer.Add(self.status_text, 0, wx.ALL | wx.EXPAND | wx.TOP, 10)

    def OnSelectInputAudio(self, event):
        with wx.FileDialog(self, "Select Input Audio File", wildcard="Audio Files (*.wav;*.mp3;*.ogg)|*.wav;*.mp3;*.ogg|All files (*.*)|*.*",
                           style=wx.FD_OPEN | wx.FD_FILE_MUST_EXIST) as fileDialog:
            if fileDialog.ShowModal() == wx.ID_CANCEL: return
            audio_path = fileDialog.GetPath()

        self.current_audio_path = audio_path
        self.current_pcm_data_for_preview = None
        self.current_audio_duration_sec = 0.0
        self.preview_marker_btn.Disable()
        self.marker_editor_panel.Hide() # Hide while processing
        self.markers = [] 
        self.UpdateMarkerList() 
        self.selected_marker_index = -1 
        self.panel.Layout()
        
        if audio_path:
            self.selected_audio_label.SetLabel(f"Selected: {os.path.basename(audio_path)}")
            self.status_text.SetLabel(f"Status: Converting '{os.path.basename(audio_path)}'...")
            wx.Yield()
            # ... (ffmpeg conversion remains the same) ...
            temp_pcm_path_preview = ""
            try:
                fd, temp_pcm_path_preview = tempfile.mkstemp(suffix=".pcm_u8_preview")
                os.close(fd)
                ffmpeg_cmd = ["ffmpeg", "-y", "-i", audio_path, "-c:a", "pcm_u8", "-ac", "1", "-ar", str(OUTPUT_SAMPLE_RATE), "-f", "u8", temp_pcm_path_preview]
                process = subprocess.Popen(ffmpeg_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                stdout, stderr = process.communicate()
                if process.returncode == 0:
                    with open(temp_pcm_path_preview, "rb") as f:
                        self.current_pcm_data_for_preview = f.read()
                    self.current_audio_duration_sec = len(self.current_pcm_data_for_preview) / OUTPUT_SAMPLE_RATE
                    self.time_spin_ctrl.SetMax(self.current_audio_duration_sec)
                    self.status_text.SetLabel(f"Status: Audio ready (Duration: {self.current_audio_duration_sec:.3f}s).")
                    
                    # --- NEW: Show editor panel in cursor mode ---
                    self.cursor_time_sec = 0.0
                    self.time_spin_label.SetLabel("Cursor Time (s):")
                    self._update_spinctrl_value(self.cursor_time_sec)
                    self.marker_editor_panel.Show()
                    self.panel.Layout()
                else:
                    self.status_text.SetLabel(f"Status: Error converting audio.")
                    wx.MessageBox(f"ffmpeg conversion error:\n{stderr.decode(errors='ignore')}", "Error", wx.OK | wx.ICON_ERROR)
            except Exception as e:
                self.status_text.SetLabel(f"Status: Error during preview conversion: {e}")
            finally:
                if temp_pcm_path_preview and os.path.exists(temp_pcm_path_preview):
                    try: os.remove(temp_pcm_path_preview)
                    except: pass
        else:
            self.selected_audio_label.SetLabel("No file selected.")
            self.status_text.SetLabel("Status: No audio file selected.")

    def OnMarkerSelected(self, event):
        self.selected_marker_index = event.GetIndex()
        if self.current_pcm_data_for_preview: self.preview_marker_btn.Enable()
            
        if 0 <= self.selected_marker_index < len(self.markers):
            marker_time = self.markers[self.selected_marker_index]
            # --- NEW: Switch spin control to marker edit mode ---
            self.time_spin_label.SetLabel("Marker Time (s):")
            self._update_spinctrl_value(marker_time)
            self.marker_editor_panel.Show() # Ensure it's shown
            self.panel.Layout()
        else:
            self.preview_marker_btn.Disable()
            self.panel.Layout()

    def OnMarkerDeselected(self, event):
        if self.marker_list_ctrl.GetSelectedItemCount() == 0:
            # --- NEW: Switch spin control back to cursor mode ---
            self.time_spin_label.SetLabel("Cursor Time (s):")
            self._update_spinctrl_value(self.cursor_time_sec)
            
            self.preview_marker_btn.Disable()
            self.selected_marker_index = -1
            self.panel.Layout()

    def _update_spinctrl_value(self, value):
        self.time_spin_ctrl.Unbind(wx.EVT_SPINCTRLDOUBLE); self.time_spin_ctrl.Unbind(wx.EVT_TEXT)
        self.time_spin_ctrl.SetValue(value)
        self.time_spin_ctrl.Bind(wx.EVT_SPINCTRLDOUBLE, self.OnMarkerTimeChange)
        self.time_spin_ctrl.Bind(wx.EVT_TEXT, self.OnMarkerTimeChange)

    def _update_marker_time(self, new_time):
        if self.selected_marker_index == -1 or self.selected_marker_index >= len(self.markers): return
        if self.current_audio_duration_sec > 0 and new_time > self.current_audio_duration_sec:
            new_time = self.current_audio_duration_sec
        if new_time < 0: new_time = 0.0
        self.markers[self.selected_marker_index] = new_time
        self.markers.sort()
        self.UpdateMarkerList(new_time)
        
    def OnMarkerTimeChange(self, event):
        new_time = self.time_spin_ctrl.GetValue()
        
        # --- NEW: Logic to handle both modes ---
        if self.selected_marker_index != -1:
            # We are in Marker Edit Mode
            self._update_marker_time(new_time)
        else:
            # We are in Cursor Mode
            self.cursor_time_sec = new_time
            # Clamp value to audio duration
            if self.current_audio_duration_sec > 0 and self.cursor_time_sec > self.current_audio_duration_sec:
                self.cursor_time_sec = self.current_audio_duration_sec
            if self.cursor_time_sec < 0: self.cursor_time_sec = 0.0
            self._update_spinctrl_value(self.cursor_time_sec) # Update visually if clamped
        
    def OnAdjustMarkerTime(self, adjustment, event):
        current_time = self.time_spin_ctrl.GetValue()
        new_time = current_time + adjustment
        self._update_spinctrl_value(new_time)
        # Manually trigger the update logic
        self.OnMarkerTimeChange(None)

    def OnGoToStart(self, event):
        self._update_spinctrl_value(0.0); self.OnMarkerTimeChange(None)
    def OnGoToEnd(self, event):
        self._update_spinctrl_value(self.current_audio_duration_sec); self.OnMarkerTimeChange(None)

    def OnPreviewMarker(self, event=None): 
        current_selection = self.marker_list_ctrl.GetFirstSelected()
        if current_selection == -1:
            wx.MessageBox("Please select a marker to preview.", "Info", wx.OK | wx.ICON_INFORMATION)
            return
        # ... (Rest of the preview logic is unchanged) ...
        self.selected_marker_index = current_selection
        if not self.current_pcm_data_for_preview: return
        marker_time_sec = self.markers[self.selected_marker_index]
        sample_offset = int(marker_time_sec * OUTPUT_SAMPLE_RATE)
        try:
            audio_array_u8 = np.frombuffer(self.current_pcm_data_for_preview, dtype=np.uint8)
            audio_array_float = (audio_array_u8 / 255.0) * 2.0 - 1.0 
            if sample_offset < len(audio_array_float):
                preview_duration_samples = int(PREVIEW_DURATION_SEC * OUTPUT_SAMPLE_RATE)
                preview_chunk = audio_array_float[sample_offset : sample_offset + preview_duration_samples]
                if len(preview_chunk) > 0:
                    self.status_text.SetLabel(f"Status: Playing preview from {marker_time_sec:.3f}s...")
                    wx.Yield()
                    sd.play(preview_chunk, samplerate=OUTPUT_SAMPLE_RATE, blocking=False)
                    self.status_text.SetLabel(f"Status: Preview started.")
                else: self.status_text.SetLabel(f"Status: Marker at or near end of audio.")
            else: wx.MessageBox("Marker time is beyond the end of the audio.", "Error", wx.OK | wx.ICON_ERROR)
        except Exception as e:
            wx.MessageBox(f"Error during audio preview: {e}", "Error", wx.OK | wx.ICON_ERROR)

    # --- NEW: Replaces OnAddMarkers ---
    def OnAddMarkerAtCursor(self, event):
        if not self.current_pcm_data_for_preview:
            wx.MessageBox("Please select an input audio file first.", "No Audio", wx.OK | wx.ICON_INFORMATION)
            return
        
        new_marker_time = self.cursor_time_sec
        self.markers.append(new_marker_time)
        self.markers.sort()
        # Update list and automatically select the new marker
        self.UpdateMarkerList(new_marker_time)
    
    # --- OnRemoveMarker and OnRemoveAllMarkers are unchanged ---
    def OnRemoveMarker(self, event):
        selected_idx = self.marker_list_ctrl.GetFirstSelected()
        if selected_idx != -1:
            del self.markers[selected_idx]
            self.selected_marker_index = -1 
            self.UpdateMarkerList() 
            
    def OnRemoveAllMarkers(self, event): 
        if self.markers:
            self.markers = []
            self.selected_marker_index = -1
            self.UpdateMarkerList()

    def UpdateMarkerList(self, value_to_reselect=None):
        self.marker_list_ctrl.DeleteAllItems()
        for marker_time in self.markers:
            self.marker_list_ctrl.Append([f"{marker_time:.3f}"])
        
        new_selection_idx = -1
        if value_to_reselect is not None:
            try: new_selection_idx = self.markers.index(value_to_reselect)
            except ValueError: pass 
        
        if 0 <= new_selection_idx < self.marker_list_ctrl.GetItemCount():
            self.marker_list_ctrl.Select(new_selection_idx)
            self.marker_list_ctrl.Focus(new_selection_idx)
            # The OnMarkerSelected event will handle the rest
        else: 
            self.selected_marker_index = -1
            if self.current_pcm_data_for_preview:
                self.time_spin_label.SetLabel("Cursor Time (s):")
                self._update_spinctrl_value(self.cursor_time_sec)
            self.preview_marker_btn.Disable()
        self.panel.Layout()

    # --- OnGenerate and OnViewPCMWave are unchanged ---
    def OnGenerate(self, event):
        if not self.current_audio_path:
            wx.MessageBox("Please select an input audio file.", "Error", wx.OK | wx.ICON_ERROR); return
        with wx.FileDialog(self, "Save PCMWave File", defaultFile="PCMWave", wildcard="All Files (*.*)|*.*",
                           style=wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT) as fileDialog:
            if fileDialog.ShowModal() == wx.ID_CANCEL: return
            output_pcm_path = fileDialog.GetPath()
        self.status_text.SetLabel("Status: Generating...")
        wx.Yield() 
        pcm_data_to_write = self.current_pcm_data_for_preview
        if pcm_data_to_write is None:
            self.status_text.SetLabel("Status: Error - No PCM data available."); return
        sample_length = len(pcm_data_to_write)
        self.status_text.SetLabel(f"Status: Packing PCMWave data...")
        wx.Yield()
        try:
            pcmw_data = bytearray()
            pcmw_data.extend(struct.pack(">I", sample_length))
            pcmw_data.extend(struct.pack(">I", len(self.markers)))
            for marker_time_sec in self.markers:
                marker_offset_bytes = int(marker_time_sec * OUTPUT_SAMPLE_RATE)
                if marker_offset_bytes >= sample_length: marker_offset_bytes = sample_length - 1 if sample_length > 0 else 0
                if marker_offset_bytes < 0: marker_offset_bytes = 0
                pcmw_data.extend(struct.pack(">I", marker_offset_bytes))
            pcmw_data.extend(pcm_data_to_write)
            with open(output_pcm_path, "wb") as f: f.write(pcmw_data)
            self.status_text.SetLabel(f"Status: PCMWave file '{os.path.basename(output_pcm_path)}' generated!")
        except Exception as e:
            wx.MessageBox(f"An error occurred during file writing: {e}", "Error", wx.OK | wx.ICON_ERROR)
    
    def OnViewPCMWave(self, event):
        with wx.FileDialog(self, "Select PCMWave File to View", wildcard="All Files (*.*)|*.*",
                           style=wx.FD_OPEN | wx.FD_FILE_MUST_EXIST) as fileDialog:
            if fileDialog.ShowModal() == wx.ID_CANCEL: return
            filepath = fileDialog.GetPath()
        try:
            with open(filepath, "rb") as f:
                sample_length = struct.unpack(">I", f.read(4))[0]
                num_markers = struct.unpack(">I", f.read(4))[0]
                info_str = f"PCMWave File: {os.path.basename(filepath)}\n--------------------------------------\n"
                info_str += f"Sample Length (bytes/samples): {sample_length}\nNumber of Markers: {num_markers}\n\n"
                if num_markers > 0:
                    info_str += "Marker Offsets (bytes from start of PCM data):\n"
                    for i in range(num_markers):
                        offset = struct.unpack(">I", f.read(4))[0]
                        time_sec = offset / float(OUTPUT_SAMPLE_RATE) if OUTPUT_SAMPLE_RATE > 0 else 0
                        info_str += f"  Marker {i+1}: Offset = {offset} bytes (approx {time_sec:.3f} s)\n"
                HeaderViewFrame(self, info_str)
                self.status_text.SetLabel(f"Status: Displayed header for {os.path.basename(filepath)}")
        except Exception as e:
            wx.MessageBox(f"An error occurred while viewing file: {e}", "Error", wx.OK | wx.ICON_ERROR)
            self.status_text.SetLabel(f"Status: Error - {e}")

if __name__ == '__main__':
    app = wx.App(False)
    frame = PCMWaveGenFrame()
    app.MainLoop()