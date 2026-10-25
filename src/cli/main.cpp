#include <iostream>
#include <string>

#include "command_line.h"
#include "compilation_session.h"
#include "constants.h"
#include "register_targets.h"
#include "retrace.h"

using namespace tiecodec;

int main(int argc, char** argv) {
  TargetRegistry targets;
  std::string error;
  if (!RegisterBuiltinTargets(targets, &error)) {
    std::cerr << "错误: " << error << "\n";
    return 2;
  }

  CliCommand command;
  if (!ParseCommandLine(argc, argv, targets, &command, &error)) {
    std::cerr << "错误: " << error << "\n";
    return 1;
  }
  if (command.help) {
    std::cout << UsageText(targets);
    return 0;
  }
  if (command.friendly_name != kFriendlyNameOriginal &&
      command.friendly_name != kFriendlyNamePinyin &&
      command.friendly_name != kFriendlyNameRandom) {
    std::cerr << "错误: 无效的命名模式: " << command.friendly_name
              << "（可选: random, pinyin, original）\n";
    return 2;
  }
  if (!command.retrace_batch.empty())
    return RunRetraceBatch(command.retrace_batch);
  if (!command.retrace_map.empty()) {
    RetraceRequest request;
    request.mapping_path = command.retrace_map;
    request.file = command.retrace_file;
    request.line = command.retrace_line;
    return RunRetrace(request);
  }

  CompilationSession session(command, targets);
  return session.Run();
}
