//
// Created by 26071 on 2024/10/19.
//

#ifndef VDISKBACKUP_BACKUPCONFIGTUI_H
#define VDISKBACKUP_BACKUPCONFIGTUI_H
#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/component.hpp"         // for Slider, Checkbox, Vertical, Renderer, Button, Input, Menu, Radiobox, Toggle
#include "ftxui/component/component_base.hpp"    // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"// for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"
#include "ftxui/util/ref.hpp"
#include "VdiskBackupManager.h"
using namespace ftxui;

class BackupConfigTui {
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
    static Component Wrap(const std::string& name, const Component& component) {
        return Renderer(component, [name, component] {
            return hbox({
                           text(name),
                           separator(),
                           component->Render(),
                   });
        });
    }
public:
    void ShowGUI();
};


#endif//VDISKBACKUP_BACKUPCONFIGTUI_H
