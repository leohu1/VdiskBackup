//
// Created by 26071 on 2024/10/19.
//

#include "BackupConfigTui.h"

void BackupConfigTui::ShowGUI() {
    auto screen = ScreenInteractive::TerminalOutput();
    auto cancel = true;

    std::string source_path_string,
            dest_path_string,
            min_compact_size_string = "5",
            min_merge_size_string = "5",
            buffer_size_string = "134217728";

    std::vector<std::string> toggle_entries = {
            "Disabled",
            "Enabled",
    };

    auto source_path = Input(&source_path_string, "path/to/vdisk.vhdx");
    source_path = Wrap("Source", source_path);

    auto dest_path = Input(&dest_path_string, "path/to/destination");
    dest_path = Wrap("Destination", dest_path);

    Component min_compact_size = Input(&min_compact_size_string, "Min compact size");
    min_compact_size |= CatchEvent([&](const Event &event) {
        try {
            return event.is_character() && std::stoull(min_compact_size_string) > 10;
        } catch (...) {
            return false;
        }
    });
    min_compact_size = Wrap("Min Compact Size", min_compact_size);

    Component min_merge_size = Input(&min_merge_size_string, "min merge size");
    min_merge_size |= CatchEvent([&](const Event &event) {
        try {
            return event.is_character() && std::stoull(min_merge_size_string) > 10;
        } catch (...) {
            return false;
        }
    });
    min_merge_size = Wrap("Min Merge Size", min_merge_size);

    Component buffer_size = Input(&buffer_size_string, "buffer size");
    buffer_size |= CatchEvent([&](const Event &event) {
        try {
            return event.is_character() && std::stoull(buffer_size_string) > 10;
        } catch (...) {
            return false;
        }
    });
    buffer_size = Wrap("Buffer Size", buffer_size);

    auto fs_aware_selected = 0;
    auto fs_aware_toggle = Toggle(&toggle_entries, &fs_aware_selected);
    fs_aware_toggle = Wrap("FS Aware Compact", fs_aware_toggle);

    auto fs_agnostic_selected = 0;
    auto fs_agnostic_toggle = Toggle(&toggle_entries, &fs_agnostic_selected);
    fs_agnostic_toggle = Wrap("FS Agnostic Compact", fs_agnostic_toggle);

    auto merge_selected = 0;
    auto merge_toggle = Toggle(&toggle_entries, &merge_selected);
    merge_toggle = Wrap("Merge Vdisk to Parent", merge_toggle);

    auto save_button = Button("Save", [&]() {
        cancel = false;
        screen.Exit(); }, BackupConfigTui::ButtonStyle());
    auto cancel_button = Button("Cancel", [&]() {
        cancel = true;
        screen.Exit(); }, BackupConfigTui::ButtonStyle());
    auto buttons = Container::Horizontal({save_button, cancel_button});
    auto layout = Container::Vertical({source_path,
                                       dest_path,
                                       fs_aware_toggle,
                                       fs_agnostic_toggle,
                                       min_compact_size,
                                       merge_toggle,
                                       min_merge_size,
                                       buttons});
    auto component = Renderer(layout, [&] {
        return vbox({source_path->Render(),
                     separator(),
                     dest_path->Render(),
                     separator(),
                     fs_aware_toggle->Render(),
                     separator(),
                     fs_agnostic_toggle->Render(),
                     separator(),
                     min_compact_size->Render(),
                     separator(),
                     merge_toggle->Render(),
                     separator(),
                     min_merge_size->Render(),
                     separator(),
                     hbox({save_button->Render(),
                           separator(),
                           cancel_button->Render()})}) |
               xflex | size(WIDTH, GREATER_THAN, 60) | border;
    });

    screen.Loop(component);

    VdiskBackupConfig config;
    config.source_path = source_path_string;
    config.destination_path = dest_path_string;
    config.min_compact_size = std::stoull(min_compact_size_string);
    config.min_merge_size = std::stoull(min_merge_size_string);
    config.buffer_size = std::stoull(buffer_size_string);
    config.enable_merge = merge_selected == 1;
    config.enable_fs_aware = fs_aware_selected == 1;
    config.enable_fs_agnostic = fs_agnostic_selected == 1;
    BackupConfigManager::WriteConfig(config);
}
