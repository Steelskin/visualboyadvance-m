#ifndef VBAM_WX_CONFIG_FILE_HISTORY_H_
#define VBAM_WX_CONFIG_FILE_HISTORY_H_

#include <functional>
#include <unordered_set>
#include <list>

#include <wx/filename.h>

namespace config {

class FileHistoryObserver;

class FileHistory {
public:
    static constexpr size_t kMaxFiles = 10;

    FileHistory();
    virtual ~FileHistory();

    void LoadConfig();

    // Adds a file to the history. If the file is already in the history, it is
    // moved to the front.
    void AddFileToHistory(const wxFileName& file);

    // Clears the history.
    void ClearHistory();

    // Returns the file at the given index.
    wxFileName GetHistoryFile(size_t i) const;

    const std::list<wxFileName>& history_files() const { return history_files_; }
    size_t size() const { return history_files_.size(); }
    bool is_empty() const { return history_files_.empty(); }

private:

    class Observer {
    public:
        virtual ~Observer() = default;

        // Called when the history changes.
        virtual void OnHistoryChanged(FileHistory* file_history) = 0;
    };

    // Registers an observer to be notified when the history changes.
    // `observer` must not be null and must outlive this object.
    void RegisterObserver(Observer* observer);

    // Unregisters an observer.
    // `observer` must have been previously registered. Will assert otherwise.
    void UnregisterObserver(Observer* observer);

    void OnHistoryChanged();

    std::list<wxFileName> history_files_;
    std::unordered_set<Observer*> observers_;

    // For Observer registration.
    friend class FileHistoryObserver;
};

class FileHistoryObserver : public FileHistory::Observer {
public:
    using FileHistoryCallback = std::function<void(FileHistory*)>;

    FileHistoryObserver(FileHistory* const file_history, FileHistoryCallback callback)
        : file_history_(file_history), callback_(std::move(callback)) {
        assert(file_history_);
        file_history_->RegisterObserver(this);
    }
    virtual ~FileHistoryObserver() { file_history_->UnregisterObserver(this); }

    // Disallow copy and assign.
    FileHistoryObserver(const FileHistoryObserver&) = delete;
    FileHistoryObserver& operator=(const FileHistoryObserver&) = delete;

private:
    // FileHistory::Observer implementation.
    void OnHistoryChanged(FileHistory* file_history) override { callback_(file_history); }

    FileHistory* const file_history_;
    const FileHistoryCallback callback_;
};

}  // namespace config

#endif  // VBAM_WX_CONFIG_FILE_HISTORY_H_
