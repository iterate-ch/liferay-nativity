#pragma once

// C-Std
#include <memory>
#include <string>
#include <vector>

// Windows
#include <Unknwn.h>
#include <Windows.h>
#include <ShlObj.h>
#include <ShObjIdl.h>
#include <winrt/base.h>
#include <gdiplus.h>

// Utils
#include <FileUtil.h>
#include <json/json.h>
#include <NativityMessage.h>
#include <RegistryUtil.h>
#include <StringUtil.h>

// Local
#include "CommunicationSocket.h"
#include "ContextMenuAction.h"
#include "ContextMenuConstants.h"
#include "ContextMenuFactory.h"
#include "ContextMenuItem.h"
#include "ContextMenuUtil.h"
