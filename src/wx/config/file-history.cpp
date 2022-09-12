#include "wx/config/file-history.h"

#include <cassert>

#include <wx/confbase.h>
#include <wx/filename.h>
#include <wx/string.h>

namespace config {

FileHistory::FileHistory() = default;
FileHistory::~FileHistory() = default;

void FileHistory::AddFileToHistory(const wxFileName& file) {
    // We only store absolute paths internally.
    wxFileName file_to_add(file);
    file_to_add.Normalize(wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG);
    if (!file_to_add.IsFileReadable()) {
        return;
    }

    for (auto iter = history_files_.begin(); iter != history_files_.end(); ++iter) {
        if (*iter == file) {
            // Remove file from history.
            history_files_.erase(iter);
            break;
        }
    }

    // Add the file to the front of the list.
    history_files_.push_front(file);

    if (history_files_.size() > kMaxFiles) {
        // Remove the last element.
        history_files_.pop_back();
    }

    OnHistoryChanged();
}

void FileHistory::ClearHistory() {
    if (history_files_.empty()) {
        return;
    }

    history_files_.clear();
    OnHistoryChanged();
}

wxFileName FileHistory::GetHistoryFile(size_t i) const {
    if (i >= history_files_.size()) {
        return wxString();
    }

    auto iter = history_files_.begin();
    std::advance(iter, i);
    return *iter;
}


void FileHistory::LoadConfig() {
    const wxConfigBase* config = wxConfigBase::Get();

    history_files_.clear();
    for (size_t i = 0; i < kMaxFiles; ++i) {
        const wxString key = wxString::Format("/Recent/file%zu", i + 1);
        wxString file_string;
        if (!config->Read(key, &file_string)) {
            break;
        }

        wxFileName file_name(file_string);
        file_name.Normalize(wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG);
        if (!file_name.FileExists() || !file_name.IsFileReadable()) {
            continue;
        }

        // Don't add duplicates.
        bool found = false;
        for (const wxFileName& file : history_files_) {
            if (file == file_name) {
                found = true;
                break;
            }
        }
        if (found) {
            continue;
        }

        // Finally, add the file.
        history_files_.push_back(file_name);
    }

    OnHistoryChanged();
}

void FileHistory::RegisterObserver(Observer* observer) {
    assert(observer);
    assert(observers_.find(observer) == observers_.end());
    observers_.insert(observer);
}

void FileHistory::UnregisterObserver(Observer* observer) {
    assert(observer);
    auto iter = observers_.find(observer);
    assert(iter != observers_.end());
    observers_.erase(iter);
}

void FileHistory::OnHistoryChanged() {
    // First, update the config.
    wxConfigBase* config = wxConfigBase::Get();
    assert(config);

    size_t i = 0;
    for (const wxFileName& file : history_files_) {
        const wxString key = wxString::Format("/Recent/file%zu", i + 1);
        config->Write(key, file.GetFullPath());
        i++;
    }

    // Clear out any remaining keys.
    for (; i < kMaxFiles; ++i) {
        const wxString key = wxString::Format("/Recent/file%zu", i + 1);
        config->Write(key, "");
    }

    config->Flush();

    // Call the observers.
    for (Observer* observer : observers_) {
        observer->OnHistoryChanged(this);
    }
}

}  // namespace config
