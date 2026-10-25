#include "source_loader.h"

#include <iostream>
#include <utility>
#include <vector>

#include "ast.h"
#include "command_line.h"
#include "diagnostics.h"
#include "parser.h"
#include "program.h"
#include "source.h"

namespace tiecodec {

SourceLoadStatus LoadProgramSources(const CliCommand& command, Program* program,
                                    Diagnostics* diagnostics, std::string* error) {
  FileSystemSourceProvider provider;
  std::vector<std::string> files;
  for (const std::string& directory : command.source_dirs) {
    if (!provider.ScanDirectory(directory, &files, error))
      return SourceLoadStatus::kScanError;
  }
  files.insert(files.end(), command.sources.begin(), command.sources.end());
  if (files.empty())
    return SourceLoadStatus::kNoSources;

  program->units.reserve(files.size());
  for (const std::string& file : files) {
    std::cout << "添加源文件: " << file << "\n";
    SourceFile source_file;
    if (!provider.ReadFile(file, &source_file, error)) {
      diagnostics->Fatal(*error, "io.read");
      continue;
    }
    Unit unit;
    unit.file = file;
    Parser parser(std::move(source_file.content), file, diagnostics);
    parser.SetTopLevelMode(command.enable_toplevel_stmt);
    parser.ParseUnit(&unit);
    program->units.push_back(std::move(unit));
  }
  return SourceLoadStatus::kOk;
}

}
