#include "FileUtils.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace HyperUtilities
{
	void FileUtilities::WriteFile(const std::string& file, const std::vector<std::string>& lines)
	{
		if (IsDirectory(file))
		{
			std::cerr << "[HyperUtilities] Path was not a file!" << std::endl;
			__debugbreak();
		}

		std::ofstream fileStream(file);

		for (std::string line : lines)
			fileStream << line;

		fileStream.close();
	}

	void FileUtilities::ReadFile(const std::string& file, const typename std::common_type<std::function<void(const std::string&)>>::type function)
	{
		if (!Exists(file))
		{
			std::cerr << "[HyperUtilities] File was not found!" << std::endl;
			__debugbreak();
		}

		if (IsDirectory(file))
		{
			std::cerr << "[HyperUtilities] Path was not a file!" << std::endl;
			__debugbreak();
		}

		std::ifstream fileStream(file);

		std::string line;
		while (std::getline(fileStream, line))
			function(line);

		fileStream.close();
	}

	void FileUtilities::ReadFile(const std::string& file, const typename std::common_type<std::function<void(std::vector<std::string>)>>::type function)
	{
		if (!Exists(file))
		{
			std::cerr << "[HyperUtilities] File was not found!" << std::endl;
			__debugbreak();
		}

		if (IsDirectory(file))
		{
			std::cerr << "[HyperUtilities] Path was not a file!" << std::endl;
			__debugbreak();
		}

		std::ifstream fileStream(file);

		std::vector<std::string> lines;

		std::string line;
		while (std::getline(fileStream, line))
			lines.push_back(line);

		fileStream.close();

		function(lines);
	}

	std::vector<std::string> FileUtilities::ReadFile(const std::string& file)
	{
		if (!Exists(file))
		{
			std::cerr << "[HyperUtilities] File was not found!" << std::endl;
			__debugbreak();
		}

		if (IsDirectory(file))
		{
			std::cerr << "[HyperUtilities] Path was not a file!" << std::endl;
			__debugbreak();
		}

		std::ifstream fileStream(file);

		std::vector<std::string> lines;

		std::string line;
		while (std::getline(fileStream, line))
			lines.push_back(line);

		return lines;
	}

	void FileUtilities::GetFiles(const std::string& directory, const typename std::common_type<std::function<void(const std::string&)>>::type function)
	{
		if (!Exists(directory))
		{
			std::cerr << "[HyperUtilities] Directory was not found!" << std::endl;
			__debugbreak();
		}

		if (IsFile(directory))
		{
			std::cerr << "[HyperUtilities] Path was not a directory!" << std::endl;
			__debugbreak();
		}

		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			std::string entryPath = entry.path().string();
			if (IsFile(entryPath))
			{
				std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
				function(entryPath);
			}
		}
	}

	void FileUtilities::GetFiles(const std::string& directory, const typename std::common_type<std::function<void(std::vector<std::string>)>>::type function)
	{
		if (!Exists(directory))
		{
			std::cerr << "[HyperUtilities] Directory was not found!" << std::endl;
			__debugbreak();
		}

		if (IsFile(directory))
		{
			std::cerr << "[HyperUtilities] Path was not a directory!" << std::endl;
			__debugbreak();
		}

		std::vector<std::string> files;
		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			std::string entryPath = entry.path().string();
			if (IsFile(entryPath))
			{
				std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
				files.push_back(entryPath);
			}
		}
		function(files);
	}

	void FileUtilities::GetFiles(const std::string& directory, std::vector<std::string>& files)
	{
		if (!Exists(directory))
		{
			std::cerr << "[HyperUtilities] Directory was not found!" << std::endl;
			__debugbreak();
		}

		if (IsFile(directory))
		{
			std::cerr << "[HyperUtilities] Path was not a directory!" << std::endl;
			__debugbreak();
		}

		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			std::string entryPath = entry.path().string();
			if (IsFile(entryPath))
			{
				std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
				files.push_back(entryPath);
			}
		}
	}

	std::vector<std::string> FileUtilities::GetFiles(const std::string& directory)
	{
		if (!Exists(directory))
		{
			std::cerr << "[HyperUtilities] Directory was not found!" << std::endl;
			__debugbreak();
		}

		if (IsFile(directory))
		{
			std::cerr << "[HyperUtilities] Path was not a directory!" << std::endl;
			__debugbreak();
		}

		std::vector<std::string> files;
		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			std::string entryPath = entry.path().string();
			if (IsFile(entryPath))
			{
				std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
				files.push_back(entryPath);
			}
		}
		return files;
	}

	void FileUtilities::GetDirectories(const std::string& directory, const typename std::common_type<std::function<void(const std::string&)>>::type function)
	{
		if (!Exists(directory))
		{
			std::cerr << "[HyperUtilities] Directory was not found!" << std::endl;
			__debugbreak();
		}

		if (IsFile(directory))
		{
			std::cerr << "[HyperUtilities] Path was not a directory!" << std::endl;
			__debugbreak();
		}

		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			std::string entryPath = entry.path().string();
			if (IsDirectory(entryPath))
			{
				std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
				function(entryPath);
			}
		}
	}

	void FileUtilities::GetDirectories(const std::string& directory, const typename std::common_type<std::function<void(std::vector<std::string>)>>::type function)
	{
		if (!Exists(directory))
		{
			std::cerr << "[HyperUtilities] Directory was not found!" << std::endl;
			__debugbreak();
		}

		if (IsFile(directory))
		{
			std::cerr << "[HyperUtilities] Path was not a directory!" << std::endl;
			__debugbreak();
		}

		std::vector<std::string> directories;
		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			std::string entryPath = entry.path().string();
			if (IsDirectory(entryPath))
			{
				std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
				directories.push_back(entryPath);
			}
		}
		function(directories);
	}

	void FileUtilities::GetDirectories(const std::string& directory, std::vector<std::string>& directories)
	{
		if (!Exists(directory))
		{
			std::cerr << "[HyperUtilities] Directory was not found!" << std::endl;
			__debugbreak();
		}

		if (IsFile(directory))
		{
			std::cerr << "[HyperUtilities] Path was not a directory!" << std::endl;
			__debugbreak();
		}

		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			std::string entryPath = entry.path().string();
			if (IsDirectory(entryPath))
			{
				std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
				directories.push_back(entryPath);
			}
		}
	}

	std::vector<std::string> FileUtilities::GetDirectories(const std::string& directory)
	{
		if (!Exists(directory))
		{
			std::cerr << "[HyperUtilities] Directory was not found!" << std::endl;
			__debugbreak();
		}

		if (IsFile(directory))
		{
			std::cerr << "[HyperUtilities] Path was not a directory!" << std::endl;
			__debugbreak();
		}

		std::vector<std::string> directories;
		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			std::string entryPath = entry.path().string();
			if (IsDirectory(entryPath))
			{
				std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
				directories.push_back(entryPath);
			}
		}
		return directories;
	}

	void FileUtilities::GetEntry(const std::string& directory, const typename std::common_type<std::function<void(const std::string&)>>::type function)
	{
		if (!Exists(directory))
		{
			std::cerr << "[HyperUtilities] Directory was not found!" << std::endl;
			__debugbreak();
		}

		if (IsFile(directory))
		{
			std::cerr << "[HyperUtilities] Path was not a directory!" << std::endl;
			__debugbreak();
		}

		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			std::string entryPath = entry.path().string();
			std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
			function(entryPath);
		}
	}

	void FileUtilities::GetEntry(const std::string& directory, const typename std::common_type<std::function<void(std::vector<std::string>)>>::type function)
	{
		if (!Exists(directory))
		{
			std::cerr << "[HyperUtilities] Directory was not found!" << std::endl;
			__debugbreak();
		}

		if (IsFile(directory))
		{
			std::cerr << "[HyperUtilities] Path was not a directory!" << std::endl;
			__debugbreak();
		}

		std::vector<std::string> entries;
		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			std::string entryPath = entry.path().string();
			std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
			entries.push_back(entryPath);
		}
		function(entries);
	}

	void FileUtilities::GetEntry(const std::string& directory, std::vector<std::string>& entries)
	{
		if (!Exists(directory))
		{
			std::cerr << "[HyperUtilities] Directory was not found!" << std::endl;
			__debugbreak();
		}

		if (IsFile(directory))
		{
			std::cerr << "[HyperUtilities] Path was not a directory!" << std::endl;
			__debugbreak();
		}

		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			std::string entryPath = entry.path().string();
			std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
			entries.push_back(entryPath);
		}
	}

	std::vector<std::string> FileUtilities::GetEntry(const std::string& directory)
	{
		if (!Exists(directory))
		{
			std::cerr << "[HyperUtilities] Directory was not found!" << std::endl;
			__debugbreak();
		}

		if (IsFile(directory))
		{
			std::cerr << "[HyperUtilities] Path was not a directory!" << std::endl;
			__debugbreak();
		}

		std::vector<std::string> entries;
		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			std::string entryPath = entry.path().string();
			std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
			entries.push_back(entryPath);
		}
		return entries;
	}

	bool FileUtilities::Exists(const std::string& path)
	{
		return std::filesystem::exists(path);
	}

	bool FileUtilities::IsFile(const std::string& path)
	{
		return !std::filesystem::is_directory(path);
	}

	bool FileUtilities::IsDirectory(const std::string& path)
	{
		return std::filesystem::is_directory(path);
	}
}