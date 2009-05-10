/*
  Copyright (C) 2008 Laurent Cozic. All right reserved.
  Use of this source code is governed by a GNU/GPL license that can be
  found in the LICENSE file.
*/


/**
 * TODO
 *
 */	


#include "../stdafx.h"

#include "azPlugin.h"



//*****************************************************************
//
// LUNAR DATA
//
//*****************************************************************

const char azPlugin::className[] = "Preferences";

#define method(class, name) {#name, &class::name}

Lunar<azPlugin>::RegType azPlugin::methods[] = {
  method(azPlugin, getPath),
  {0,0}
};


//*****************************************************************
//
// NON-EXPORTED MEMBERS
//
//*****************************************************************


azPlugin::azPlugin(lua_State *L) {
  // throw exception
}


Plugin* azPlugin::Get() const {
  return NULL;
}


azPlugin::~azPlugin() {

}


//*****************************************************************
//
// EXPORTED MEMBERS
//
//*****************************************************************


int azPlugin::getPath(lua_State *L) {
  
  
  return 0;
}


//azPreferences::azPreferences(lua_State *L) {
//  luaL_error(L, "This object cannot be directly created. Instead use the global 'preferences' object.");
//}
//
//
//int azPreferences::registerPreferenceGroup(lua_State *L) {
//  wxString inputName = LuaUtil::GetStringFromTable(L, 1, _T("name"), false);
//  wxString inputTitle = LuaUtil::GetStringFromTable(L, 1, _T("title"));
//
//  PluginPreferenceGroup* group = new PluginPreferenceGroup();
//  group->Name = inputName;
//  group->Title = inputTitle;
//
//  Get()->RegisterPreferenceGroup(group);
//
//  return 0;
//}
//
//
//
//int azPreferences::registerPreference(lua_State *L) {
//  PluginPreference* preference = LuaUtil::ToPluginPreference(L, Get(), 1);
//  
//  Get()->RegisterPreference(preference);
//
//  return 0;
//}
//
//
//
//int azPreferences::getValue(lua_State *L) {
//  wxString inputName = LuaUtil::ToString(L, 1, false); 
//  
//  PluginPreference* preference = preferences_->GetPreference(inputName);
//  if (!preference) return 0;
//
//  if (preference->GetType() == PluginPreferenceType::Spinner) {
//    lua_pushinteger(L, preference->GetIntValue());
//    return 1;
//  }
//
//  if (preference->GetType() == PluginPreferenceType::CheckBox) {
//    lua_pushboolean(L, preference->GetBoolValue());
//    return 1;
//  }
//
//  lua_pushstring(L, preference->GetValue().ToUTF8());
//
//  return 1;
//}
//
//
//int azPreferences::setValue(lua_State *L) {
//  wxString inputName = LuaUtil::ToString(L, 1, false);
//  wxString inputValue = LuaUtil::ToString(L, 2, false); 
//  
//  PluginPreference* preference = preferences_->GetPreference(inputName);
//  if (!preference) return 0;
//
//  preference->SetValue(inputValue);
//
//  preferences_->ScheduleSave();
//
//  return 0;
//}