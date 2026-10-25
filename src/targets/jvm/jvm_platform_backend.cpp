#include "jvm_platform_backend.h"

#include <cstdint>
#include <cstdio>
#include <sstream>
#include <utility>
#include <vector>

#include "java_language_constants.h"
#include "json_util.h"
#include "jvm_constants.h"
#include "options.h"
#include "symbol_resolution.h"

namespace tiecodec {
namespace {

constexpr const char* kSourceDir = "src/";
constexpr const char* kMetadataFile = "metadata.json";
constexpr const char* kLauncherFile = "Main.java";

std::string PackageOf(const std::string& qualified_name) {
  const std::size_t dot = qualified_name.rfind('.');
  return dot == std::string::npos ? std::string() : qualified_name.substr(0, dot);
}

std::string MemberOutputName(const GeneratedProgram& program, const std::string& source_name) {
  for (const auto& entry : program.mapping_names) {
    if (entry.first == source_name && !entry.second.empty())
      return entry.second;
  }
  return {};
}

}

JvmPlatformBackend::JvmPlatformBackend(const GenerationInput& input) : options_(input.config) {}

TargetKey JvmPlatformBackend::Key() const {
  return TargetKey{kJvmPlatformName, kJavaLanguageName};
}

bool JvmPlatformBackend::Generate(const GenerationInput& input, LanguageBackend& language,
                                  Diagnostics& diagnostics, OutputSink& sink) {
  diagnostics_ = &diagnostics;
  GeneratedProgram program = language.Emit(input, diagnostics);
  if (diagnostics.has_errors())
    return false;
  if (input.entry == nullptr) {
    diagnostics.Fatal("当前环境中找不到入口类 \"" + std::string(kJvmEntryClassName) + "\"",
                      "output.entry.class-not-found");
    return false;
  }
  for (const GeneratedSource& source : program.sources) {
    if (!WriteFile(sink, std::string(kSourceDir) + source.file_name, source.content))
      return false;
  }
  if (!WriteFile(sink, std::string(kSourceDir) + kLauncherFile, LauncherSource(input, program)))
    return false;
  if (!WriteFile(sink, kMetadataFile, Metadata(input, program)))
    return false;
  if (!options_.Option(option::kLineMap).empty())
    WriteLineMap(input, program);
  return true;
}

bool JvmPlatformBackend::WriteFile(OutputSink& sink, const std::string& relative_path,
                                   const std::string& content) const {
  return WriteOutputFile(sink, relative_path, content, options_.output_dir, diagnostics_);
}

std::string JvmPlatformBackend::LauncherSource(const GenerationInput& input,
                                               const GeneratedProgram& program) const {
  const std::string entry_class =
      program.entry_class_name.empty() ? input.entry->declaration->name : program.entry_class_name;
  const std::string entry_qualified =
      program.entry_qualified_name.empty() ? entry_class : program.entry_qualified_name;
  const std::string package = PackageOf(entry_qualified);
  std::ostringstream out;
  if (!package.empty())
    out << "import " << entry_qualified << ";\n\n";
  out << "public final class Main {\n";
  out << "  public static void main(String[] args) {\n";
  out << "    " << entry_class << " app = new " << entry_class << "();\n";
  if (FindMemberInChain(input.symbols, *input.entry, kJvmEntryMethodName, nullptr) != nullptr) {
    const std::string mapped = MemberOutputName(program, kJvmEntryMethodName);
    out << "    app." << (mapped.empty() ? kJvmEntryMethodName : mapped) << "();\n";
  }
  out << "  }\n";
  out << "}\n";
  return out.str();
}

std::string JvmPlatformBackend::Metadata(const GenerationInput& input,
                                         const GeneratedProgram& program) const {
  const std::string entry = program.entry_qualified_name.empty() ? program.entry_class_name
                                                                 : program.entry_qualified_name;
  const std::string package = PackageOf(entry);
  std::ostringstream out;
  out << "{\n";
  out << "  \"platform\": \"" << kJvmDisplayName << "\",\n";
  out << "  \"language\": \"" << kJavaLanguageName << "\",\n";
  out << "  \"packageName\": \"" << JsonEscape(package.empty() ? options_.package_name : package)
      << "\",\n";
  out << "  \"entry\": \"" << JsonEscape(entry) << "\",\n";
  out << "  \"srcDir\": \"src\",\n";
  out << "  \"mainClass\": \"Main\"\n";
  out << "}\n";
  (void)input;
  return out.str();
}

void JvmPlatformBackend::WriteLineMap(const GenerationInput& input,
                                      const GeneratedProgram& program) const {
  const SourceMapping mapping = BuildSourceMapping(program);
  std::string error;
  if (!WriteSourceMapping(options_.Option(option::kLineMap), mapping, &error) &&
      diagnostics_ != nullptr) {
    diagnostics_->Fatal(error, "io.write");
  }
  (void)input;
}

}
