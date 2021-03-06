/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/

#include "stdafx.h"

#ifndef __FolderItemProcess_H
#define __FolderItemProcess_H


class FolderItemProcess : public wxProcess {

public:

  FolderItemProcess(wxEvtHandler * parent, int id = -1);
  bool IsTerminated();
  virtual void OnTerminate(int pid, int status); 

private:

  bool terminated_;

};


typedef std::vector<FolderItemProcess*> FolderItemProcessVector;


#endif // __FolderItemProcess_H