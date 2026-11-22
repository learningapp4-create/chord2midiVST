; Inno Setup script. ISCC is invoked with /DVst3Dir=<path to Chord2MIDI.vst3>.
#ifndef Vst3Dir
  #define Vst3Dir "Chord2MIDI.vst3"
#endif

[Setup]
AppId={{7E4A1C2B-9F30-4D6A-8B15-C2D400C42D01}
AppName=Chord2MIDI
AppVersion=0.3.0
AppPublisher=Chord2MIDI
DefaultDirName={commoncf}\VST3
DisableProgramGroupPage=yes
DisableDirPage=no
OutputDir=output
OutputBaseFilename=Chord2MIDI-windows
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin
WizardStyle=modern
UninstallDisplayName=Chord2MIDI VST3

[Files]
Source: "{#Vst3Dir}\*"; DestDir: "{app}\Chord2MIDI.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[Messages]
FinishedLabel=Chord2MIDI.vst3 is in the VST3 folder. Rescan plugins in your DAW. The plugin is unsigned, and the JUCE splash screen stays on.
