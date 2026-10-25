#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace tiecodec {
namespace test {
namespace {

namespace fs = std::filesystem;

std::string ReadFile(const fs::path& path) {
  std::ifstream input(path, std::ios::binary);
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

bool EndsWith(const std::string& text, const std::string& suffix) {
  return text.size() >= suffix.size() &&
         text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string Join(const std::vector<std::string>& parts) {
  std::string joined;
  for (const std::string& part : parts) {
    if (!joined.empty())
      joined += ", ";
    joined += part;
  }
  return joined;
}

struct Layer {
  std::string directory;
  std::vector<std::string> forbidden_headers;
};

std::vector<std::string> CollectIncludes(const fs::path& directory) {
  std::vector<std::string> headers;
  if (!fs::is_directory(directory))
    return headers;

  std::vector<fs::path> files;
  for (const fs::directory_entry& entry : fs::recursive_directory_iterator(directory)) {
    if (!entry.is_regular_file())
      continue;
    const std::string name = entry.path().filename().string();
    if (!EndsWith(name, ".cpp") && !EndsWith(name, ".h"))
      continue;
    files.push_back(entry.path());
  }
  std::sort(files.begin(), files.end());

  const std::regex include_pattern(R"re(#include\s+"([^"]+)")re");
  for (const fs::path& file : files) {
    const std::string content = ReadFile(file);
    for (std::sregex_iterator it(content.begin(), content.end(), include_pattern), end; it != end;
         ++it) {
      headers.push_back((*it)[1].str());
    }
  }
  return headers;
}

std::string NormalizeHeader(const std::string& include_path) {
  return fs::path(include_path).filename().string();
}

std::vector<fs::path> SourceFiles(const fs::path& path) {
  std::vector<fs::path> files;
  if (fs::is_regular_file(path)) {
    files.push_back(path);
    return files;
  }
  if (!fs::is_directory(path))
    return files;
  for (const fs::directory_entry& entry : fs::recursive_directory_iterator(path)) {
    if (!entry.is_regular_file())
      continue;
    const std::string name = entry.path().filename().string();
    if (EndsWith(name, ".cpp") || EndsWith(name, ".h"))
      files.push_back(entry.path());
  }
  std::sort(files.begin(), files.end());
  return files;
}

std::string TextViolations(const fs::path& path, const std::vector<std::string>& needles) {
  std::vector<std::string> hits;
  for (const fs::path& file : SourceFiles(path)) {
    const std::string content = ReadFile(file);
    for (const std::string& needle : needles) {
      if (content.find(needle) != std::string::npos)
        hits.push_back(file.filename().string() + " -> " + needle);
    }
  }
  std::sort(hits.begin(), hits.end());
  hits.erase(std::unique(hits.begin(), hits.end()), hits.end());
  return Join(hits);
}

std::string Violations(const fs::path& src_root, const Layer& layer) {
  std::vector<std::string> hits;
  for (const std::string& header : CollectIncludes(src_root / layer.directory)) {
    const std::string base = NormalizeHeader(header);
    if (std::find(layer.forbidden_headers.begin(), layer.forbidden_headers.end(), base) !=
        layer.forbidden_headers.end()) {
      hits.push_back(header);
    }
  }
  std::sort(hits.begin(), hits.end());
  hits.erase(std::unique(hits.begin(), hits.end()), hits.end());
  return Join(hits);
}

class LayeringTest : public ::testing::Test {
protected:
  void SetUp() override {
    src_root_ = fs::path(TIECODEC_SOURCE_DIR) / "src";
    ASSERT_TRUE(fs::is_directory(src_root_)) << "缺少源码目录: " << src_root_;
  }

  const fs::path& src_root() const {
    return src_root_;
  }

private:
  fs::path src_root_;
};

}

TEST_F(LayeringTest, CoreDoesNotDependOnUpperLayers) {
  const Layer layer{"core",
                    {"ast.h", "lexer.h", "parser.h", "token.h", "symbols.h", "symbol_resolution.h",
                     "usage_analysis.h", "target_config.h", "java_names.h", "java_type_mapper.h",
                     "android_platform_backend.h", "jvm_platform_backend.h", "target.h"}};
  EXPECT_EQ(Violations(src_root(), layer), "") << "core 依赖了上层模块";
}

TEST_F(LayeringTest, IrDoesNotDependOnFrontendOrUpperLayers) {
  const Layer layer{
      "ir",
      {"lexer.h", "parser.h", "token.h", "usage_analysis.h", "target_config.h", "java_names.h",
       "java_type_mapper.h", "android_platform_backend.h", "jvm_platform_backend.h", "target.h"}};
  EXPECT_EQ(Violations(src_root(), layer), "") << "ir 依赖了 frontend、analysis 或 targets";
}

TEST_F(LayeringTest, FrontendDoesNotDependOnAnalysisOrTargets) {
  const Layer layer{
      "frontend",
      {"symbol_resolution.h", "usage_analysis.h", "target_config.h", "java_names.h",
       "java_type_mapper.h", "android_platform_backend.h", "jvm_platform_backend.h", "target.h"}};
  EXPECT_EQ(Violations(src_root(), layer), "") << "frontend 依赖了 analysis 或 targets";
}

TEST_F(LayeringTest, AnalysisDoesNotDependOnFrontend) {
  const Layer layer{"analysis", {"lexer.h", "parser.h", "token.h"}};
  EXPECT_EQ(Violations(src_root(), layer), "") << "analysis 依赖了 frontend";
}

TEST_F(LayeringTest, AnalysisDoesNotDependOnTargets) {
  const Layer layer{
      "analysis",
      {"target.h", "target_registry.h", "target_config.h", "java_names.h", "java_type_mapper.h",
       "java_emitter.h", "android_platform_backend.h", "jvm_platform_backend.h"}};
  EXPECT_EQ(Violations(src_root(), layer), "") << "analysis 依赖了 targets";
}

TEST_F(LayeringTest, TargetsDoNotDependOnFrontendOrAnalysis) {
  const std::vector<std::string> directories = {"targets/api", "targets/java", "targets/android",
                                                "targets/jvm", "targets/builtin"};
  const std::vector<std::string> forbidden = {"lexer.h",
                                              "parser.h",
                                              "token.h",
                                              "program.h",
                                              "usage_analysis.h",
                                              "annotation_validation.h",
                                              "annotation_pipeline.h",
                                              "requirement_validation.h"};
  for (const std::string& directory : directories) {
    const Layer layer{directory, forbidden};
    EXPECT_EQ(Violations(src_root(), layer), "") << directory << " 依赖了 frontend 或 analysis";
  }
}

TEST_F(LayeringTest, JavaTargetDoesNotDependOnPlatformTargets) {
  const Layer layer{
      "targets/java",
      {"android_java_extension.h", "android_platform_backend.h", "android_resource_binder.h",
       "jvm_java_extension.h", "jvm_platform_backend.h"}};
  EXPECT_EQ(Violations(src_root(), layer), "") << "targets/java 依赖了平台目标";
}

TEST_F(LayeringTest, JavaTargetHasNoAndroidLiterals) {
  EXPECT_EQ(TextViolations(src_root() / "targets" / "java",
                           {"\"android.", "\"onInit\"", "\"onCreateComponent\"", "\"R."}),
            "")
      << "targets/java 仍包含 Android 字面量";
}

TEST_F(LayeringTest, UsageAnalysisHasNoAnnotationNames) {
  EXPECT_EQ(TextViolations(src_root() / "analysis" / "usage_analysis.cpp",
                           {"annotation.name", "annotation::"}),
            "")
      << "usage_analysis 仍按注解名判断";
}

TEST_F(LayeringTest, CliDoesNotDependOnTargetInternals) {
  const Layer layer{
      "cli",
      {"java_emitter.h", "java_expressions.h", "java_imports.h", "helpers.h",
       "android_platform_backend.h", "jvm_platform_backend.h", "java_language_backend.h"}};
  EXPECT_EQ(Violations(src_root(), layer), "") << "cli 依赖了目标内部实现";
}

}
}
