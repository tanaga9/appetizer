program MiniLaunchBar;

{%TogetherDiagram 'ModelSupport_MiniLaunchBar\default.txaPackage'}

uses
  Forms,
  MainForm in 'MainForm.pas' {MainForm},
  WImageButton in 'Components\WImageButton.pas',
  pngextra in 'PNGButton\pngextra.pas',
  pngimage in 'PNGButton\pngimage.pas',
  Imaging in 'Components\Imaging.pas',
  WNineSlicesPanel in 'Components\WNineSlicesPanel.pas',
  WComponent in 'Components\WComponent.pas',
  FileSystemUtils in 'FileSystemUtils.pas',
  WFileIcon in 'Components\WFileIcon.pas',
  Main in 'Main.pas',
  StringUtils in 'StringUtils.pas',
  DebugWindow in 'DebugWindow.pas' {Form1},
  LocalizationUtils in 'LocalizationUtils.pas',
  ControlTest in 'ControlTest.pas' {Form2},
  WContainer in 'Components\WContainer.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TMainForm, Form1);
  Application.CreateForm(TForm2, Form2);
  Application.Run;
end.
