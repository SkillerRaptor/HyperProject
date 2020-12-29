#pragma once

#include <functional>
#include <string>
#include <vector>

namespace HyperUtilities
{
	class FileUtilities
	{
	public:
		static void WriteFile(const std::string& file, const std::vector<std::string>& lines);

		static void ReadFile(const std::string& file, const typename std::common_type<std::function<void(const std::string&)>>::type function);
		static void ReadFile(const std::string& file, const typename std::common_type<std::function<void(std::vector<std::string>)>>::type function);
		static std::vector<std::string> ReadFile(const std::string& file);

		static void GetFiles(const std::string& directory, const typename std::common_type<std::function<void(const std::string&)>>::type function);
		static void GetFiles(const std::string& directory, const typename std::common_type<std::function<void(std::vector<std::string>)>>::type function);
		static void GetFiles(const std::string& directory, std::vector<std::string>& files);
		static std::vector<std::string> GetFiles(const std::string& directory);

		static void GetDirectories(const std::string& directory, const typename std::common_type<std::function<void(const std::string&)>>::type function);
		static void GetDirectories(const std::string& directory, const typename std::common_type<std::function<void(std::vector<std::string>)>>::type function);
		static void GetDirectories(const std::string& directory, std::vector<std::string>& directories);
		static std::vector<std::string> GetDirectories(const std::string& directory);

		static void GetEntry(const std::string& directory, const typename std::common_type<std::function<void(const std::string&)>>::type function);
		static void GetEntry(const std::string& directory, const typename std::common_type<std::function<void(std::vector<std::string>)>>::type function);
		static void GetEntry(const std::string& directory, std::vector<std::string>& entries);
		static std::vector<std::string> GetEntry(const std::string& directory);

		static bool Exists(const std::string& path);
		static bool IsFile(const std::string& path);
		static bool IsDirectory(const std::string& path);
	}
}