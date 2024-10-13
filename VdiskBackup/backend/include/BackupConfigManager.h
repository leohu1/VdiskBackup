//
// Created by 26071 on 2024/10/6.
//

#ifndef VDISKBACKUP_BACKUPCONFIGMANAGER_H
#define VDISKBACKUP_BACKUPCONFIGMANAGER_H

#include "VdiskBackupManager.h"// for separator, gauge, text, Element, operator|, vbox, border
#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"         // for Slider, Checkbox, Vertical, Renderer, Button, Input, Menu, Radiobox, Toggle
#include "ftxui/component/component_base.hpp"    // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"// for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"
#include "ftxui/util/ref.hpp"
#include "yaml-cpp/yaml.h"
#include <memory>
#include <string>

using namespace ftxui;
class BackupConfigManager {
private:
    static ButtonOption ButtonStyle() {
        auto option = ButtonOption::Animated();
        option.transform = [](const EntryState& s) {
            auto element = text(s.label);
            if (s.focused) {
                element |= bold;
            }
            return element | center | borderEmpty | flex;
        };
        return option;
    }
public:
    void ShowGUI();
};


#endif//VDISKBACKUP_BACKUPCONFIGMANAGER_H
