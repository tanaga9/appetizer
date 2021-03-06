﻿/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include "../stdafx.h"

#include "IconGetter.h"
#include "StringUtil.h"

#ifdef __WINDOWS__
#include <shlobj.h>
#include <shlguid.h> 
#include <shellapi.h>
#include <commctrl.h>
#include <commoncontrols.h>
#endif // __WINDOWS__


wxString IconGetter::system32Path_ = wxEmptyString;
HINSTANCE IconGetter::shell32Library_ = NULL;
SHGetImageListType IconGetter::SHGetImageListFunction_ = NULL;


IconGetterIconHashMap IconGetter::defaultFileIcon_;
IconGetterIconHashMap IconGetter::defaultFolderIcon_;
IconGetterTypeIconHashMap IconGetter::defaultDefaultTypeIcon_;


void IconGetter::Destroy() {
  IconGetterIconHashMap::iterator it;
  for(it = defaultFileIcon_.begin(); it != defaultFileIcon_.end(); ++it) {
    wxIcon* icon = it->second;
    wxDELETE(icon);
  }
  defaultFileIcon_.clear();

  for(it = defaultFolderIcon_.begin(); it != defaultFolderIcon_.end(); ++it) {
    wxIcon* icon = it->second;
    wxDELETE(icon);
  }
  defaultFolderIcon_.clear();

  IconGetterTypeIconHashMap::iterator it2;
  for(it2 = defaultDefaultTypeIcon_.begin(); it2 != defaultDefaultTypeIcon_.end(); ++it2) {
    wxIcon* icon = it2->second;
    wxDELETE(icon);
  }
  defaultDefaultTypeIcon_.clear();

  if (shell32Library_) {
    FreeLibrary(shell32Library_);
    shell32Library_ = NULL;
  }
}


wxIcon* IconGetter::GetDefaultWebLinkIcon(int iconSize) {
  wxIcon* output = IconGetter::GetDefaultTypeIcon(iconSize, _T("html"));
  if (output) return output;

  output = IconGetter::GetDefaultTypeIcon(iconSize, _T("htm"));
  return output;
}



wxString IconGetter::GetSystem32Path() {
  if (system32Path_ == wxEmptyString) {
    // LPTSTR is wchar_t if UNICODE is enabled, or a char otherwise
    LPTSTR buffer = new TCHAR[MAX_PATH];
    int success = GetSystemDirectory(buffer, MAX_PATH);    

    // Convert the LPTSTR to a char*
    char cString[MAX_PATH];
    wcstombs(cString, buffer, MAX_PATH);
    wxDELETE(buffer);

    if (success) {
      system32Path_ = wxString::FromAscii(cString);
    } else {
      system32Path_ = _T("C:\\WINDOWS\\SYSTEM32");
    }
  }

  return system32Path_;
}


wxIcon* IconGetter::GetDefaultFileIcon(int iconSize) {
  if (defaultFileIcon_.find(iconSize) != defaultFileIcon_.end()) return new wxIcon(*(defaultFileIcon_[iconSize]));
  wxIcon* icon = GetExecutableIcon(GetSystem32Path() + _T("\\SHELL32.DLL"), iconSize, 0);  
  if (icon) {
    defaultFileIcon_[iconSize] = icon;
    return new wxIcon(*icon);
  }
  return NULL;
}


wxIcon* IconGetter::GetDefaultFolderIcon(int iconSize) {
  if (defaultFolderIcon_.find(iconSize) != defaultFolderIcon_.end()) return new wxIcon(*(defaultFolderIcon_[iconSize]));
  wxIcon* icon = GetExecutableIcon(GetSystem32Path() + _T("\\SHELL32.DLL"), iconSize, 4);  
  if (icon) {
    defaultFolderIcon_[iconSize] = icon;
    return new wxIcon(*icon);
  }
  return NULL;
}


wxIcon* IconGetter::GetDefaultTypeIcon(int iconSize, const wxString& extension) {
  wxString ext = extension.Lower();
  wxString key = wxString::Format(_T("%s-%d"), ext, iconSize);  

  if (defaultDefaultTypeIcon_.find(key) != defaultDefaultTypeIcon_.end()) return new wxIcon(*(defaultDefaultTypeIcon_[key]));

  wxIcon* icon = NULL;

  #ifdef __WINDOWS__

  // Get the file type from the file extension
  wxFileType* fileType = NULL;

  if (ext != wxEmptyString) {
    fileType = wxTheMimeTypesManager->GetFileTypeFromExtension(ext);
  }

  // Try to get the icon location from the file type
  wxIconLocation iconLocation;
  bool gotIconLocation = false; 
  if (fileType) {
    gotIconLocation = fileType->GetIcon(&iconLocation);
  }

  wxString shell32Path = IconGetter::GetSystem32Path() + _T("\\SHELL32.DLL");

  if (!fileType || !gotIconLocation) {

    // Below we try to get some default icon depending on the type

    wxString libFilePath = shell32Path;
    int iconIndex = -1;
    wxString e = ext;

    if (e == _T("inf") || e == _T("ini")) {
      iconIndex = 69;
    } else if (e == _T("txt")) {
      iconIndex = 70;
    } else if (e == _T("bat")) {
      iconIndex = 169;
    } else if (e == _T("jpg") || e == _T("jpeg") || e == _T("png") || e == _T("gif") || e == _T("bmp") || e == _T("tif") || e == _T("tiff")) {
      libFilePath = shell32Path + _T("\\mspaint.exe");
      iconIndex = 1;
    } else if (e == _T("wmv") || e == _T("avi") || e == _T("mpg") || e == _T("mpeg")) {
      iconIndex = 115;
    } else if (e == _T("mp3") || e == _T("wav") || e == _T("mid")) {
      iconIndex = 116;
    } else if (e == _T("dll") || e == _T("ocx") || e == _T("sys")) {
      iconIndex = 72;
    } else if (e == _T("html") || e == _T("htm")) {
      wxFileName ieFileName(_T("%ProgramFiles%\\Internet Explorer\\IEXPLORE.EXE"));
      ieFileName.Normalize();
      if (ieFileName.FileExists()) {
        libFilePath = ieFileName.GetFullPath();
        iconIndex = 1;
      }
    }

    if (iconIndex >= 0) icon = GetExecutableIcon(libFilePath, iconSize, iconIndex);
  }

  wxDELETE(fileType);


  if (gotIconLocation && !icon) {
    // Fixes a bug in wxWidgets: Sometime the icon is negative, in which case the wxIconLocation will be invalid
    if (iconLocation.GetIndex() < 0) iconLocation.SetIndex(0);

    // Fixes a bug in wxWidgets: The filename is sometime surrounded by quotes, and so the wxIconLocation will
    // again be invalid. We remove the quotes below:
    wxString iconLocFile = iconLocation.GetFileName();
    while (iconLocFile[0] == _T('"')) iconLocFile = iconLocFile.Mid(1, iconLocFile.Len());
    while (iconLocFile[iconLocFile.Len() - 1] == _T('"')) iconLocFile = iconLocFile.Mid(0, iconLocFile.Len() - 1);
    iconLocation.SetFileName(iconLocFile);
  }  

  if (gotIconLocation && !icon) {
    // NOTE: iconLocation.GetFileName() is not always a path, it may also be a filename 
    // to be looked for in the "system32" folder (example: "shimgvw.dll")
    wxLogNull logNull;
    wxIcon* icon = new wxIcon(iconLocation);
    if (!icon->IsOk()) {
      wxDELETE(icon);
    } else {
      icon->SetSize(iconSize, iconSize);
      return icon;
    }
  }


  // If we couldn't find the icon at this stage, look for it in the registry. The icon file path
  // may be stored in many different places. We need to look in:
  //
  // HKEY_CLASSES_ROOT\.<extension>\DefaultIcon\<defaultValue>
  // HKEY_CLASSES_ROOT\<documentType>\DefaultIcon\<defaultValue>
  // HKEY_CLASSES_ROOT\CLSID\<classId>\DefaultIcon\<defaultValue>
  //
  // The document type can be read at HKEY_CLASSES_ROOT\.<extension>\<defaultValue>
  // The class ID is (sometime) at HKEY_CLASSES_ROOT\<documentType>\CLSID\<defaultValue>
  // 
  // Note: all the DefaultIcon values already seem to be handled by wxWidgets so we
  // only care about the CLSID case below.
  // 
  // If there is no class ID, we could also look in:
  // HKEY_CLASSES_ROOT\<documentType>\shell\open\command
  // then get the associated executable from there and, finally, get the icon.
  // This is currently not implemented.
  //
  // Finally, some icons are displayed using an icon handler that can be found at:
  // HKEY_CLASSES_ROOT\<documentType>\ShellEx\IconHandler\<defaultValue>
  // This is a special module which displays the icon depending on the file content. It
  // may actually display two different icons for the same file extension. For example,
  // a C# .sln file is going to be displayed differently than a C++ .sln file. These cases
  // are hopefully rare enough to ignore them. Note: the SHGetImageList / SHGetFileInfo method
  // seems to make use of the icon handler.

  // Disable wxWidgets built-in error messages, such as the ones that
  // show up when a key can't be read.
  if (!icon) {

    wxLogNull* logNo = new wxLogNull();

    wxRegKey* regKey = new wxRegKey(_T("HKEY_CLASSES_ROOT\\.") + ext);
    wxString iconPath;    

    if (regKey->Exists()) {
      wxString extensionLink = regKey->QueryDefaultValue();    

      if (extensionLink != wxEmptyString) {
        wxDELETE(regKey);
        regKey = new wxRegKey(_T("HKEY_CLASSES_ROOT\\") + extensionLink + _T("\\CLSID"));

        if (regKey->Exists()) {
          wxString classIdLink = regKey->QueryDefaultValue();

          if (classIdLink != wxEmptyString) {
            wxDELETE(regKey);
            regKey = new wxRegKey(_T("HKEY_CLASSES_ROOT\\CLSID\\") + classIdLink + _T("\\DefaultIcon"));
            
            if (regKey->Exists()) {
              iconPath = regKey->QueryDefaultValue();
            }            
          }
        }   
      }
    }

    wxDELETE(logNo); // Re-enable wxWidgets built-in error messages
    wxDELETE(regKey);
   
    if (iconPath != wxEmptyString) {
      // The icon path may be specified with a coma followed by the icon index, as in:
      // c:\WINDOWS\Installer\{AC76BA86-7AD7-1033-7B44-A90000000001}\PDFFile_8.ico,0
      // So we need to split the string and extract the file path and index.

      wxArrayString splitted;

      if (iconPath.Find(_T(';')) != wxNOT_FOUND) {
        StringUtil::Split(iconPath, splitted, _T(";"));
      } else {
        StringUtil::Split(iconPath, splitted, _T(","));
      }
      
      iconPath = splitted[0];
      long iconIndex = 0;

      // If the conversion to "long" fails, we default to "0" which should be safe
      if (splitted.Count() > 1) splitted[1].ToLong(&iconIndex);
      
      // Sometime, the icon index is negative, for example for .resx files. Not sure
      // why since the icon actually displayed by Windows is at index 0.
      if (iconIndex < 0) iconIndex = 0;

      wxFileName f(iconPath);
      wxString e = f.GetExt().Lower();
      // Normally, it shouldn't be anything other than ico, exe or dll. If it is, we just return NULL for now.
      if (e == _T("ico") || e == _T("exe") || e == _T("dll") || e == _T("icl")) return IconGetter::GetExecutableIcon(iconPath, iconSize, iconIndex);
    }

  }

  if (!icon) icon = GetDefaultFileIcon(iconSize);
  
  #endif // __WINDOWS__


  if (icon) {
    defaultDefaultTypeIcon_[key] = icon;
    return new wxIcon(*icon);
  }

  return NULL;
}


wxIcon* IconGetter::GetFolderItemIcon(const wxString& filePath, int iconSize, bool alwaysReturnDefault) {

  wxIcon* output = NULL;

  bool isDrive = false;
  if (filePath.Len() <= 3 && filePath.Len() >= 2) isDrive = filePath[1] == _T(':');

  if (wxFileName::DirExists(filePath) || isDrive) {
    output = GetFolderIcon(filePath, iconSize);
  }

  if (output) return output;

  if (iconSize >= 48) {

    // Note: certain functions, like SHGetImageList don't exist in Windows 2000,
    // so we need to load them dynamically, otherwise we get this error and the app doesn't start:
    // "The ordinal 737 could not be located in the dynamic link library Shell32.dll"

    if (!shell32Library_) shell32Library_ = LoadLibrary(_T("SHELL32.DLL"));
    if (!SHGetImageListFunction_) SHGetImageListFunction_ = (SHGetImageListType)GetProcAddress(shell32Library_, "SHGetImageList");

    if (shell32Library_ && SHGetImageListFunction_) {

      // Get the icon index using SHGetFileInfo
      SHFILEINFOW sfi = {0};
      SHGetFileInfo(filePath, -1, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);

      // If iIcon is 0, we get a weird default icon representing a hand,
      // so don't continue.
      if (sfi.iIcon > 0) {
        // Retrieve the system image list.
        // To get the 48x48 icons, use SHIL_EXTRALARGE
        // To get the 256x256 icons (Vista only), use SHIL_JUMBO
        HIMAGELIST* imageList;
        //HRESULT hResult = SHGetImageList(iconSize == 48 ? SHIL_EXTRALARGE : SHIL_JUMBO, IID_IImageList, (void**)&imageList);
        HRESULT hResult = SHGetImageListFunction_(iconSize == 48 ? SHIL_EXTRALARGE : SHIL_JUMBO, IID_IImageList, (void**)&imageList);

        if (hResult == S_OK) {
          // Get the icon we need from the list. Note that the HIMAGELIST we retrieved
          // earlier needs to be casted to the IImageList interface before use.
          HICON hIcon;
          hResult = ((IImageList*)imageList)->GetIcon(sfi.iIcon, ILD_TRANSPARENT, &hIcon);

          if (hResult == S_OK) {
            wxIcon* icon = new wxIcon();
            icon->SetHICON((WXHICON)hIcon);
            icon->SetSize(iconSize, iconSize);
            return icon;
          }      
        }

      }

    }

  }

  wxFileName filename(filePath);

  if ((filename.GetExt().CmpNoCase(wxT("exe")) == 0) || (filename.GetExt().CmpNoCase(wxT("ico")) == 0)) {
    output = GetExecutableIcon(filePath, iconSize);
  } else {
    output = GetDocumentIcon(filePath, iconSize);
  }

  if (!output && alwaysReturnDefault) {
    return GetDefaultFileIcon(iconSize);
  }

  return output;
}


wxIcon* IconGetter::GetFolderIcon(const wxString& filePath, int iconSize) {
  #ifdef __WINDOWS__

  wxString desktopIniFilePath = filePath + wxT("/Desktop.ini");
  wxFileName filename(desktopIniFilePath);

  if (filename.FileExists()) {
    // If the folder contains a Desktop.ini file, try
    // to get the icon path from it
    CSimpleIniA ini(false, false, true);
    ini.LoadFile(desktopIniFilePath.c_str());
    const char* cIconFileValue = ini.GetValue(".ShellClassInfo", "IconFile", "");
    
    if (cIconFileValue != "") {
      wxString iconFileValue = wxString::FromAscii(cIconFileValue);
      // Normalize the path since the IconFile value is usually stored
      // as a path relative to the folder
      wxFileName iconFileName(iconFileValue);
      iconFileName.Normalize(wxPATH_NORM_ALL, filePath);
      iconFileValue = iconFileName.GetFullPath();
      
      return GetFolderItemIcon(iconFileValue, iconSize);
    }
  }

  SHFILEINFO fileInfo;
  int success = 0;

  if (filePath[filePath.Len() - 1] == _T(':')) {
    filename = wxFileName(filePath + _T("\\")); // Add "\" if it's a drive, otherwise SHGetFileInfo will fail
  } else {
    filename = wxFileName(filePath);
  }  
  
  wchar_t* cFullPath = new wchar_t[filename.GetFullPath().Len() + 2];
  mbstowcs(cFullPath, filename.GetFullPath().mb_str(), filename.GetFullPath().Len());
  cFullPath[filename.GetFullPath().Len()] = _T('\0');
  cFullPath[filename.GetFullPath().Len()+1] = _T('\0');

  if (iconSize == 32) {
    success = SHGetFileInfo(cFullPath, FILE_ATTRIBUTE_DIRECTORY, &fileInfo,
                            sizeof(fileInfo), SHGFI_ICON | SHGFI_LARGEICON);    
  } else if (iconSize == 16) {
    success = SHGetFileInfo(cFullPath, FILE_ATTRIBUTE_DIRECTORY, &fileInfo,
                            sizeof(fileInfo), SHGFI_ICON | SHGFI_SMALLICON);
  }

  wxDELETE(cFullPath);
 
  if (success != 0) {
    wxIcon* icon = new wxIcon();
    icon->SetHICON((WXHICON)fileInfo.hIcon);
    icon->SetSize(iconSize, iconSize);
    return icon;
  } else {
    if (filename.GetFullPath() == filename.GetPath()) { // if it's a drive
      UINT result = GetDriveType(filename.GetFullPath());   
      int iconIndex = -1;
      if (result == DRIVE_REMOVABLE) {
        iconIndex = 7;
      } else if (result == DRIVE_CDROM) {
        iconIndex = 11;        
      } else if (result == DRIVE_FIXED) {
        iconIndex = 8;
      }

      if (iconIndex >= 0) {
        if (iconSize >= 48) iconIndex++;
        return IconGetter::GetExecutableIcon(IconGetter::GetSystem32Path() + _T("\\shell32.dll"), iconSize, iconIndex);
      }

    }
  }

  #endif // __WINDOWS__

  return NULL;
}


wxIcon* IconGetter::GetDocumentIcon(const wxString& filePath, int iconSize) {
  return GetDefaultTypeIcon(iconSize, wxFileName(filePath).GetExt());
}


wxIcon* IconGetter::GetExecutableIcon(const wxString& filePath, int iconSize, int iconIndex) {  
  #ifdef __WINDOWS__

  if (iconSize <= 32) {
    HICON smallIcon;
    HICON largeIcon;
    int result;

    if (iconSize == 16) {
      result = ExtractIconEx(filePath.c_str(), iconIndex, NULL, &smallIcon, 1);	
    } else {
      result = ExtractIconEx(filePath.c_str(), iconIndex, &largeIcon, NULL, 1);	
    }

    // If the function succeeds, the return value is the handle to an icon.
    // If the file specified was not an executable file, DLL, or icon file,
    // the return value is 1. If no icons were found in the file, the return 
    // value is NULL. If the file didn't exist, the return value is < 0
    if (result > 0) {
      wxIcon* icon = new wxIcon();

      if (iconSize == 16) {
        icon->SetHICON((WXHICON)smallIcon);
        icon->SetSize(16, 16);
      } else {
        icon->SetHICON((WXHICON)largeIcon);
        icon->SetSize(32, 32);
      }
      return icon;
    } else {

    }

  } else {

    HINSTANCE hDll = ::LoadLibrary(filePath);
    if (hDll) {
      HANDLE handle = ::LoadImage(hDll, MAKEINTRESOURCE(iconIndex), IMAGE_ICON, iconSize, iconSize, LR_LOADTRANSPARENT);
      if (handle) {
        wxIcon* icon = new wxIcon();
        icon->SetHICON((WXHICON)handle);
        if (icon->IsOk()) {
          icon->SetSize(iconSize, iconSize);
          return icon;
        }
      }
    }

    return IconGetter::GetExecutableIcon(filePath, 32, iconIndex);
  }

  #endif // __WINDOWS__

  return NULL;
}