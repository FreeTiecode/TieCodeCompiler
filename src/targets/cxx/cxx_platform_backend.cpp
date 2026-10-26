#include "cxx_platform_backend.h"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "cxx_effect_types.h"
#include "cxx_language_constants.h"
#include "file_search.h"
#include "json_util.h"
#include "line_map.h"
#include "options.h"
#include "symbol_resolution.h"
#include "text_util.h"

namespace tiecodec {
namespace {

namespace fs = std::filesystem;

constexpr const char* kSrcDir = "src/";
constexpr const char* kMetadataFile = "metadata.json";
constexpr const char* kNamespaceSeparator = "::";

bool IsHeaderFile(const std::string& name) {
  return EndsWith(name, ".h") || EndsWith(name, ".hpp") || EndsWith(name, ".hh") ||
         EndsWith(name, ".hxx");
}

bool IsSourceFile(const std::string& name) {
  return EndsWith(name, ".c") || EndsWith(name, ".cpp") || EndsWith(name, ".cxx") ||
         EndsWith(name, ".cc");
}

std::string BasenameOf(const std::string& path) {
  const fs::path p(path);
  return p.filename().string();
}

bool IsCxxReferenceShape(const TargetConfig& config) {
  return config.Flag(option::kDisableFtcFeatures);
}

std::string MemberOutputName(const GeneratedProgram& program, const std::string& source_name) {
  for (const auto& entry : program.mapping_names) {
    if (entry.first == source_name && !entry.second.empty())
      return entry.second;
  }
  return {};
}

}

CxxPlatformBackend::CxxPlatformBackend(const GenerationInput& input) : config_(input.config) {}

TargetKey CxxPlatformBackend::Key() const {
  return TargetKey{kCxxPlatformName, kCxxLanguageName};
}

bool CxxPlatformBackend::Generate(const GenerationInput& input, LanguageBackend& language,
                                  Diagnostics& diagnostics, OutputSink& sink) {
  diagnostics_ = &diagnostics;
  const bool reference_shape = IsCxxReferenceShape(config_);
  GeneratedProgram program = language.Emit(input, diagnostics);
  if (diagnostics.has_errors())
    return false;
  if (input.entry == nullptr) {
    diagnostics.Fatal("当前环境中找不到入口类 \"" + std::string(kCxxEntryClassName) + "\"",
                      "output.entry.class-not-found");
    return false;
  }

  for (const GeneratedSource& source : program.sources) {
    if (!WriteFile(sink, kSrcDir + source.file_name, source.content))
      return false;
  }

  std::set<std::string> external_headers;
  std::set<std::string> external_sources;
  std::set<std::string> external_libraries;
  CollectExternalSources(input, reference_shape, input.plan.classes, &external_headers,
                         &external_sources, &external_libraries, &sink);

  std::vector<std::string> headers;
  std::vector<std::string> sources;
  for (const GeneratedSource& source : program.sources) {
    if (EndsWith(source.file_name, std::string(".") + kCxxHeaderExtension))
      headers.push_back(BasenameOf(source.file_name));
    else
      sources.push_back(BasenameOf(source.file_name));
  }
  for (const std::string& header : external_headers)
    headers.push_back(header);
  for (const std::string& source : external_sources)
    sources.push_back(source);
  headers.insert(headers.end(), kCxxFoundationHeader);
  sources.insert(sources.end(), kCxxFoundationSource);
  std::sort(headers.begin(), headers.end());
  headers.erase(std::unique(headers.begin(), headers.end()), headers.end());
  std::sort(sources.begin(), sources.end());
  sources.erase(std::unique(sources.begin(), sources.end()), sources.end());
  std::vector<std::string> libraries(external_libraries.begin(), external_libraries.end());

  const std::string launcher = LauncherSource(input, program, reference_shape);
  if (reference_shape) {
    diagnostics.ErrorGlobal(
        "禁用 FreeTiecodec 特性时，启动器的参考形态缺少线程头包含，产物不可编译",
        "compat.ftc.launcher-header");
    return false;
  }
  if (!WriteFile(sink, kSrcDir + std::string(kCxxLauncherFile), launcher))
    return false;
  if (!WriteFile(sink, kSrcDir + std::string(kCxxCMakeFile),
                 CMakeScript(headers, sources, libraries)))
    return false;
  if (!WriteFile(sink, kSrcDir + std::string(kCxxGccScriptFile), GccScript(sources, libraries)))
    return false;
  if (!WriteFile(sink, kSrcDir + std::string(kCxxMsvcScriptFile), MsvcScript(sources, libraries)))
    return false;
  if (!WriteFile(sink, kMetadataFile, Metadata(input, program, reference_shape)))
    return false;
  const std::string line_map_path = config_.Option(option::kLineMap);
  if (!line_map_path.empty()) {
    std::string error;
    const SourceMapping mapping = BuildSourceMapping(program);
    if (!WriteSourceMapping(line_map_path, mapping, &error) && diagnostics_ != nullptr)
      diagnostics_->Fatal(error, "io.write");
  }
  return true;
}

bool CxxPlatformBackend::WriteFile(OutputSink& sink, const std::string& relative_path,
                                   const std::string& content) const {
  return WriteOutputFile(sink, relative_path, content, config_.output_dir, diagnostics_);
}

void CxxPlatformBackend::CollectExternalSources(const GenerationInput& input, bool reference_shape,
                                                const std::vector<const ClassInfo*>& output_classes,
                                                std::set<std::string>* headers,
                                                std::set<std::string>* sources,
                                                std::set<std::string>* libraries,
                                                OutputSink* sink) const {
  std::vector<const ClassInfo*> classes;
  if (reference_shape) {
    classes = output_classes;
  } else {
    for (const auto& entry : input.symbols.classes())
      classes.push_back(&entry.second);
  }
  const std::vector<std::string> prefixes = config_.ListOption(option::kSearchSourcePrefix);
  for (const ClassInfo* klass : classes) {
    if (klass == nullptr || klass->declaration == nullptr)
      continue;
    const CxxExternalSource* list =
        input.effects.Get<CxxExternalSource>(static_cast<const void*>(klass->declaration));
    if (list != nullptr) {
      for (const std::string& raw : list->files) {
        const fs::path resolved = file_search::ResolvePath(raw, klass->declaration->file, prefixes);
        std::error_code error;
        if (fs::is_directory(resolved, error)) {
          for (const fs::directory_entry& entry :
               fs::recursive_directory_iterator(resolved, error)) {
            if (!entry.is_regular_file())
              continue;
            const std::string name = entry.path().filename().string();
            if (!IsHeaderFile(name) && !IsSourceFile(name))
              continue;
            if (IsHeaderFile(name) && headers->count(name) == 0) {
              headers->insert(name);
              sink->CopyFile(entry.path().string(), kSrcDir + name);
            } else if (IsSourceFile(name) && sources->count(name) == 0) {
              sources->insert(name);
              sink->CopyFile(entry.path().string(), kSrcDir + name);
            }
          }
          continue;
        }
        const std::string name = BasenameOf(resolved.string());
        if (IsHeaderFile(name) && headers->count(name) == 0) {
          headers->insert(name);
          sink->CopyFile(resolved.string(), kSrcDir + name);
        } else if (IsSourceFile(name) && sources->count(name) == 0) {
          sources->insert(name);
          sink->CopyFile(resolved.string(), kSrcDir + name);
        }
      }
    }
    const CxxExternalDependency* dependencies =
        input.effects.Get<CxxExternalDependency>(static_cast<const void*>(klass->declaration));
    if (dependencies != nullptr) {
      for (const std::string& library : dependencies->libraries)
        libraries->insert(BasenameOf(library));
    }
    for (const Member& member : klass->declaration->members) {
      const CxxExternalDependency* member_dependencies =
          input.effects.Get<CxxExternalDependency>(static_cast<const void*>(&member));
      if (member_dependencies == nullptr)
        continue;
      for (const std::string& library : member_dependencies->libraries)
        libraries->insert(BasenameOf(library));
    }
  }
}

std::string CxxPlatformBackend::EntryQualified(const GeneratedProgram& program,
                                               bool reference_shape) const {
  const std::string source = program.entry_qualified_name.empty() ? program.entry_class_name
                                                                  : program.entry_qualified_name;
  const std::size_t dot = source.rfind('.');
  if (dot == std::string::npos)
    return source;
  std::string package = source.substr(0, dot);
  const std::string class_name = source.substr(dot + 1);
  if (!reference_shape) {
    std::string qualified_package;
    for (std::size_t i = 0; i < package.size(); ++i) {
      if (package[i] == '.')
        qualified_package += kNamespaceSeparator;
      else
        qualified_package += package[i];
    }
    return qualified_package + kNamespaceSeparator + class_name;
  }
  return package + kNamespaceSeparator + class_name;
}

std::string CxxPlatformBackend::LauncherSource(const GenerationInput& input,
                                               const GeneratedProgram& program,
                                               bool reference_shape) const {
  const std::string entry_class =
      program.entry_class_name.empty() ? input.entry->declaration->name : program.entry_class_name;
  const std::string entry_qualified = EntryQualified(program, reference_shape);
  std::ostringstream out;
  out << "#include \"" << entry_class << "." << kCxxHeaderExtension << "\"\n";
  out << "#include \"" << kCxxThreadHeader << "\"\n";
  out << "\nint main() {\n";
  out << "  GetMainLooper().PreScheduleMessage(0, []{\n";
  out << "   " << entry_qualified << " entry;\n";
  const Member* member =
      FindMemberInChain(input.symbols, *input.entry, kCxxEntryMethodName, nullptr);
  if (member != nullptr) {
    std::string mapped = MemberOutputName(program, kCxxEntryMethodName);
    if (mapped.empty())
      mapped = kCxxEntryMethodName;
    out << "   entry." << mapped << "();\n";
  }
  out << "  });\n";
  out << "  GetMainLooper().Run();\n";
  out << "  return 0;\n";
  out << "}\n";
  return out.str();
}

std::string CxxPlatformBackend::CMakeScript(const std::vector<std::string>& headers,
                                            const std::vector<std::string>& sources,
                                            const std::vector<std::string>& libraries) const {
  std::ostringstream out;
  out << "cmake_minimum_required(VERSION 3.10)\n";
  out << "project(main)\n";
  out << "set(CMAKE_CXX_STANDARD 17)\n";
  std::string files;
  for (const std::string& header : headers)
    files += " " + header;
  for (const std::string& source : sources)
    files += " " + source;
  files += " " + std::string(kCxxLauncherFile);
  out << "add_executable(main " << files << ")\n";
  if (!libraries.empty()) {
    out << "target_link_libraries(main PRIVATE ";
    bool first = true;
    for (const std::string& library : libraries) {
      if (!first)
        out << " ";
      first = false;
      out << library;
    }
    out << ")\n";
  }
  return out.str();
}

std::string CxxPlatformBackend::GccScript(const std::vector<std::string>& sources,
                                          const std::vector<std::string>& libraries) const {
  std::ostringstream out;
  out << "g++";
  for (const std::string& source : sources)
    out << " " << source;
  out << " " << kCxxLauncherFile;
  out << " -Wno-deprecated-declarations";
  for (const std::string& library : libraries)
    out << " " << library;
  out << " -o main.exe\n";
  return out.str();
}

std::string CxxPlatformBackend::MsvcScript(const std::vector<std::string>& sources,
                                           const std::vector<std::string>& libraries) const {
  std::ostringstream cpp_files;
  for (const std::string& source : sources)
    cpp_files << " " << source;
  cpp_files << " " << kCxxLauncherFile;
  std::ostringstream link_libs;
  link_libs << "ws2_32.lib";
  for (const std::string& library : libraries)
    link_libs << " " << library;

  std::ostringstream out;
  out << "@echo off\n";
  out << "setlocal enabledelayedexpansion\n\n";
  out << ":: 设置代码页为UTF-8\n";
  out << "chcp 65001 >nul\n";
  out << ":: 输出的exe文件名\n";
  out << "set OUTPUT_FILE=main.exe\n";
  out << ":: msvc编译选项\n";
  out << "set COMPILE_FLAGS=/Zc:__cplusplus /utf-8 /MP /EHsc /std:c++17 /W4 /O2 /I. "
         "/D_CRT_SECURE_NO_WARNINGS /D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING\n";
  out << ":: .cpp 文件路径\n";
  out << "set CPP_FILES=" << cpp_files.str() << "\n";
  out << ":: 需要链接的库\n";
  out << "set LINK_LIBS=" << link_libs.str() << "\n\n";
  out << "cl %COMPILE_FLAGS% %CPP_FILES% /Fe\"%OUTPUT_FILE%\" /link %LINK_LIBS%\n";
  out << "\nendlocal\n";
  return out.str();
}

std::string CxxPlatformBackend::Metadata(const GenerationInput& input,
                                         const GeneratedProgram& program,
                                         bool reference_shape) const {
  const std::string entry = EntryQualified(program, reference_shape);
  std::string package = program.entry_qualified_name;
  const std::size_t dot = package.rfind('.');
  if (dot != std::string::npos)
    package = package.substr(0, dot);
  else
    package.clear();
  if (package.empty())
    package = config_.namespace_name;
  std::ostringstream out;
  out << "{\n";
  out << "  \"platform\": \"" << kCxxDisplayName << "\",\n";
  out << "  \"language\": \"" << kCxxLanguageName << "\",\n";
  out << "  \"packageName\": \"" << JsonEscape(package) << "\",\n";
  out << "  \"entry\": \"" << JsonEscape(entry) << "\",\n";
  out << "  \"srcDir\": \"src\",\n";
  out << "  \"launcher\": \"src/" << kCxxLauncherFile << "\"\n";
  out << "}\n";
  (void)input;
  return out.str();
}

}