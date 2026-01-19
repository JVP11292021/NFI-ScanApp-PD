#pragma once

#include "endian.h"
#include "logging.h"
#include "types.h"

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#define THROW_CHECK_FILE_EXISTS(path) \
  THROW_CHECK(ExistsFile(path)) << "File " << (path) << " does not exist."

#define THROW_CHECK_DIR_EXISTS(path) \
  THROW_CHECK(ExistsDir(path)) << "Directory " << (path) << " does not exist."

#define THROW_CHECK_PATH_OPEN(path)                           \
  THROW_CHECK(std::ofstream(path, std::ios::trunc).is_open()) \
      << "Could not open " << (path)                          \
      << ". Is the path a directory or does the parent dir not exist?"

#define THROW_CHECK_FILE_OPEN(file, path) \
  THROW_CHECK((file).is_open())           \
      << "Could not open " << (path)      \
      << ". Is the path a directory or does the parent dir not exist?"

#define THROW_CHECK_HAS_FILE_EXTENSION(path, ext)                        \
  THROW_CHECK(HasFileExtension(path, ext))                               \
      << "Path " << (path) << " does not match file extension " << (ext) \
      << "."

namespace colmap {

enum class CopyType { COPY, HARD_LINK, SOFT_LINK };

// Append trailing slash to string if it does not yet end with a slash.
std::string EnsureTrailingSlash(const std::string& str);

// Check whether file name has the file extension (case insensitive).
bool HasFileExtension(const std::string& file_name, const std::string& ext);

// Split the path into its root and extension, for example,
// "dir/file.jpg" into "dir/file" and ".jpg".
void SplitFileExtension(const std::string& path,
                        std::string* root,
                        std::string* ext);

// Copy or link file from source to destination path
void FileCopy(const std::string& src_path,
              const std::string& dst_path,
              CopyType type = CopyType::COPY);

// Check if the path points to an existing file.
bool ExistsFile(const std::string& path);

// Check if the path points to an existing directory.
bool ExistsDir(const std::string& path);

// Check if the path points to an existing file or directory.
bool ExistsPath(const std::string& path);

// Create the directory if it does not exist.
void CreateDirIfNotExists(const std::string& path, bool recursive = false);

// Extract the base name of a path, e.g., "image.jpg" for "/dir/image.jpg".
std::string GetPathBaseName(const std::string& path);

// Get the path of the parent directory for the given path.
std::string GetParentDir(const std::string& path);

// Join multiple paths into one path.
template <typename... T>
std::string JoinPaths(T const&... paths);

// Return list of files in directory.
std::vector<std::string> GetFileList(const std::string& path);

// Return list of files, recursively in all sub-directories.
std::vector<std::string> GetRecursiveFileList(const std::string& path);

// Return list of directories, recursively in all sub-directories.
std::vector<std::string> GetDirList(const std::string& path);

// Return list of directories, recursively in all sub-directories.
std::vector<std::string> GetRecursiveDirList(const std::string& path);

// Get the size in bytes of a file.
size_t GetFileSize(const std::string& path);

// Gets current user's home directory from environment variables.
// Returns null if it cannot be resolved.
std::optional<std::filesystem::path> HomeDir();

// Read contiguous binary blob from file.
void ReadBinaryBlob(const std::string& path, std::vector<char>* data);

// Write contiguous binary blob to file.
void WriteBinaryBlob(const std::string& path, const span<const char>& data);

// Read each line of a text file into a separate element. Empty lines are
// ignored and leading/trailing whitespace is removed.
std::vector<std::string> ReadTextFileLines(const std::string& path);

// Detect if given string is a URI
// (i.e., starts with http://, https://, file://).
bool IsURI(const std::string& uri);

#ifdef COLMAP_DOWNLOAD_ENABLED

// Download file from server. Supports any protocol suppported by Curl.
// Automatically follows redirects. Returns null in case of failure. Notice that
// this function is not suitable for large files that don't fit easily into
// memory. If such a use case emerges in the future, we want to instead stream
// the downloaded data chunks to disk instead of accumulating them in memory.
std::optional<std::string> DownloadFile(const std::string& url);

// Computes SHA256 digest for given string.
std::string ComputeSHA256(const std::string_view& str);

// Downloads and caches file from given URI. The URI must take the format
// "<url>;<name>;<sha256>". The file will be cached under
// $HOME/.cache/colmap/<sha256>-<name>. File integrity is checked against the
// provided SHA256 digest. Throws exception if the digest does not match.
// Returns the path to the cached file.
std::string DownloadAndCacheFile(const std::string& uri);

// Overwrites the default download cache directory at $HOME/.cache/colmap/.
void OverwriteDownloadCacheDir(std::filesystem::path path);

#endif  // COLMAP_DOWNLOAD_ENABLED

// If the given URI is a local filesystem path, returns the input path. If the
// URI matches the "<url>;<name>;<sha256>" format, calls DownloadAndCacheFile().
// Throws runtime exception if download is not supported.
std::string MaybeDownloadAndCacheFile(const std::string& uri);

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

template <typename... T>
std::string JoinPaths(T const&... paths) {
  std::filesystem::path result;
  int unpack[]{0, (result = result / std::filesystem::path(paths), 0)...};
  static_cast<void>(unpack);
  return result.string();
}

}  // namespace colmap
