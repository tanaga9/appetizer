#include _1_Initialize.ahk

; ****************************************************************
; Copy the required files and folders in the release folder
; Basically eveything that is needed to run Appetizer minus the
; Settings folders, .svn folders, temporary files, etc.
; ****************************************************************

FileCreateDir %releaseDirectory%

FileCopy Appetizer.exe, %releaseDirectory%
FileCopyDir Data, %releaseDirectory%\Data

Loop %releaseDirectory%\.svn, 2, 1
{
  FileRemoveDir %A_LoopFileFullPath%, 1
}

FileDelete %releaseDirectory%\Data\Plugins\_BuildPlugins.ahk
FileDelete %releaseDirectory%\Data\Plugins\_gettext.bat
FileDelete %releaseDirectory%\Data\Plugins\*.zpl
FileRemoveDir %releaseDirectory%\Data\Plugins\TestUnit, 1
FileRemoveDir %releaseDirectory%\Data\Plugins\Securizer, 1
FileRemoveDir %releaseDirectory%\Data\Plugins\AddToGroupOnRightClick, 1
FileRemoveDir %releaseDirectory%\Data\Plugins\CloseAfterLaunchingAnApp, 1
FileRemoveDir %releaseDirectory%\Data\Plugins\RevealShortcutTarget, 1
FileRemoveDir %releaseDirectory%\Data\Plugins\TrayIconExtras, 1
FileRemoveDir %releaseDirectory%\Data\Skin\NightBlue, 1
FileRemoveDir %releaseDirectory%\Data\Skin\NightRed, 1
FileRemoveDir %releaseDirectory%\Data\Skin\PAMClassic, 1
FileRemoveDir %releaseDirectory%\Data\Skin\DarkBlue, 1
FileRemoveDir %releaseDirectory%\Data\Skin\Greydows, 1
FileRemoveDir %releaseDirectory%\Data\Skin\Sonix, 1
FileRemoveDir %releaseDirectory%\Data\Skin\LCD, 1
FileRemoveDir %releaseDirectory%\Data\Skin\LCD pink, 1
FileRemoveDir %releaseDirectory%\Data\Skin\GreenGlass, 1
FileRemoveDir %releaseDirectory%\Data\Skin\iAppetizer, 1
FileRemoveDir %releaseDirectory%\Data\Skin\OrangeGlass, 1
FileRemoveDir %releaseDirectory%\Data\Skin\Logo, 1
FileRemoveDir %releaseDirectory%\Data\Skin\DarkGlass, 1
FileRemoveDir %releaseDirectory%\Data\Settings, 1
FileRemoveDir %releaseDirectory%\Data\IconCache, 1

Loop %releaseDirectory%\Data\Help\*.html, 0, 1
{
  FileDelete %A_LoopFileFullPath%
}

Loop %releaseDirectory%\Data\Help\*.whc, 0, 1
{
  FileDelete %A_LoopFileFullPath%
}

Loop %releaseDirectory%\Data\*.po~, 0, 1
{
  FileDelete %A_LoopFileFullPath%
}

Loop %releaseDirectory%\Data\*.po, 0, 1
{
  FileDelete %A_LoopFileFullPath%
}

Loop %releaseDirectory%\Data\Help\*.mo, 0, 1
{
  FileDelete %A_LoopFileFullPath%
}

Loop %releaseDirectory%\Data\*.pot, 0, 1
{
  FileDelete %A_LoopFileFullPath%
}

Loop %releaseDirectory%\Data\Help\images, 2, 1
{
  FileRemoveDir %A_LoopFileFullPath%, 1
}

command = %upxExe% -9 %releaseDirectory%\Appetizer.exe
RunWait %command%

zipFileName = Appetizer_%appVersion%.zip
command = %sevenZipExe% a -tzip %zipFileName% *
RunWait %command%, %releaseDirectory%

FileMove %releaseDirectory%\%zipFileName%, %releaseDir%\%zipFileName%, 1