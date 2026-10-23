#!/usr/bin/env bash
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$(cd "${SCRIPT_DIR}/../src" && pwd)"
status=0

fail() { echo "[FAIL] $1"; status=1; }

if grep -rn '#include "\(ast\|lexer\|parser\|token\|symbols\|symbol_resolution\|usage_analysis\|environment_contract\|backend\|java_names\|java_type_mapper\|android_backend\)\.h"' "${SRC_DIR}/core" >/dev/null 2>&1; then
  fail "core 依赖了上层模块"
fi
if grep -rn '#include "\(symbols\|symbol_resolution\|usage_analysis\|environment_contract\|backend\|java_names\|java_type_mapper\|android_backend\)\.h"' "${SRC_DIR}/frontend" >/dev/null 2>&1; then
  fail "frontend 依赖了 analysis 或 backend"
fi
if grep -rn '#include "\(backend\|java_names\|java_type_mapper\|android_backend\)\.h"' "${SRC_DIR}/analysis" >/dev/null 2>&1; then
  fail "analysis 依赖了 backend"
fi
if grep -rn '#include "android_backend\.h"' "${SRC_DIR}/backend/java" >/dev/null 2>&1; then
  fail "backend/java 依赖了 android 后端"
fi
if grep -rn '#include "\(java_emitter\|java_expressions\|java_imports\|internal/helpers\)\.h"' "${SRC_DIR}/cli" >/dev/null 2>&1; then
  fail "cli 依赖了后端内部实现"
fi
if [[ ${status} -eq 0 ]]; then
  echo "[PASS] 分层依赖约束满足"
fi
exit ${status}
