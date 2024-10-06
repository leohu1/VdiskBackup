//
// Created by 26071 on 2024/10/6.
//

#include "BackupConfigManager.h"

using namespace ftxui;

// Display a component nicely with a title on the left.
Component Wrap(const std::string& name, const Component& component) {
    return Renderer(component, [name, component] {
        return hbox({
                       text(name) | size(WIDTH, EQUAL, 8),
                       separator(),
                       component->Render() | xflex,
               }) |
               xflex;
    });
}

void BackupConfigManager::ShowGUI() {
    auto screen = ScreenInteractive::TerminalOutput();
    auto cancel = true;

    std::string source_path_string,
            dest_path_string,
            min_compact_size_string="5",
            min_merge_size_string="5",
            buffer_size_string="134217728";

    std::vector<std::string> toggle_entries = {
            "Disabled",
            "Enabled",
    };

    auto source_path = Input(&source_path_string, "path/to/vdisk.vhdx");
    source_path = Wrap("Source Path", source_path);

    auto dest_path = Input(&dest_path_string, "path/to/destination");
    dest_path = Wrap("Destination Path", dest_path);

    Component min_compact_size = Input(&min_compact_size_string, "min compact size");
    min_compact_size |= CatchEvent([&](const Event& event) {
        try{
            return event.is_character() && std::stoull(min_compact_size_string) > 10;
        } catch (...) {
            return false;
        }
    });
    min_compact_size = Wrap("Min Compact Size", min_compact_size);

    Component min_merge_size = Input(&min_merge_size_string, "min merge size");
    min_merge_size |= CatchEvent([&](const Event& event) {
        try{
            return event.is_character() && std::stoull(min_merge_size_string) > 10;
        } catch (...) {
            return false;
        }
    });
    min_merge_size = Wrap("Min Merge Size", min_merge_size);

    Component buffer_size = Input(&buffer_size_string, "buffer size");
    buffer_size |= CatchEvent([&](const Event& event) {
        try{
            return event.is_character() && std::stoull(buffer_size_string) > 10;
        } catch (...) {
            return false;
        }
    });
    buffer_size = Wrap("Buffer Size", buffer_size);

    auto fs_aware_selected = 0;
    auto fs_aware_toggle = Toggle(&toggle_entries, &fs_aware_selected);
    fs_aware_toggle = Wrap("FileSystem Aware Compact", fs_aware_toggle);

    auto fs_agnostic_selected = 0;
    auto fs_agnostic_toggle = Toggle(&toggle_entries, &fs_agnostic_selected);
    fs_agnostic_toggle = Wrap("FileSystem Agnostic Compact", fs_agnostic_toggle);

    auto merge_selected = 0;
    auto merge_toggle = Toggle(&toggle_entries, &merge_selected);
    merge_toggle = Wrap("Merge Vdisk to Parent", merge_toggle);

    auto save_button = Button("Save", [&](){
        cancel = false;
        screen.Exit();
    }, BackupConfigManager::ButtonStyle());
    auto cancel_button = Button("Cancel", [&](){
        cancel = true;
        screen.Exit();
    }, BackupConfigManager::ButtonStyle());
    auto buttons = Container::Horizontal({save_button, cancel_button});
    auto layout = Container::Vertical({
            source_path,
            dest_path,
            fs_aware_toggle,
            fs_agnostic_toggle,
            min_compact_size,
            merge_toggle,
            min_merge_size,
            buttons
    });
    auto component = Renderer(layout, [&] {
        return vbox({
                       source_path->Render(),
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
                       hbox({
                               save_button->Render(),
                               separator(),
                               cancel_button->Render()
                       })
               }) |
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
    YAML::Emitter source_out;
    source_out << YAML::BeginMap << YAML::Key << "backups" << YAML::BeginSeq << YAML::BeginMap;
    source_out << YAML::Key << "id" << YAML::Value << "A";
    source_out << YAML::Key << "source" << YAML::Value << fs::relative(config.source_path, config.source_path.root_path()).string();
    source_out << YAML::Key << "min_compact_size" << YAML::Value << config.min_compact_size;
    source_out << YAML::Key << "min_merge_size" << YAML::Value << config.min_merge_size;
    source_out << YAML::Key << "copy_buffer_size" << YAML::Value << config.buffer_size;
    source_out << YAML::Key << "enable_file_system_aware_compact" << YAML::Value << config.enable_fs_aware;
    source_out << YAML::Key << "enable_file_system_agnostic_compact" << YAML::Value << config.enable_fs_agnostic;
    source_out << YAML::Key << "enable_merge" << YAML::Value << config.enable_merge;
    std::ofstream source_of((config.source_path.root_path() / VdiskBackupManager::ConfigName).string());
    source_of << source_out.c_str();
    source_of.close();

    YAML::Emitter dest_out;
    dest_out << YAML::BeginMap << YAML::Key << "backups" << YAML::BeginSeq << YAML::BeginMap;
    dest_out << YAML::Key << "id" << YAML::Value << "A";
    dest_out << YAML::Key << "destination" << YAML::Value << fs::relative(config.destination_path, config.destination_path.root_path()).string();
    dest_out << YAML::Key << "min_compact_size" << YAML::Value << config.min_compact_size;
    dest_out << YAML::Key << "min_merge_size" << YAML::Value << config.min_merge_size;
    dest_out << YAML::Key << "copy_buffer_size" << YAML::Value << config.buffer_size;
    dest_out << YAML::Key << "enable_file_system_aware_compact" << YAML::Value << config.enable_fs_aware;
    dest_out << YAML::Key << "enable_file_system_agnostic_compact" << YAML::Value << config.enable_fs_agnostic;
    dest_out << YAML::Key << "enable_merge" << YAML::Value << config.enable_merge;
    std::ofstream dest_of((config.destination_path.root_path() / VdiskBackupManager::ConfigName).string());
    dest_of << dest_out.c_str();
    dest_of.close();
}
