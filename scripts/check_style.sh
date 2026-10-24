#!/usr/bin/env bash
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
SCAN_DIRS=("${REPO_ROOT}/src" "${REPO_ROOT}/tests")
CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
status=0

if ! command -v "${CLANG_FORMAT}" >/dev/null 2>&1; then
  echo "[FAIL] 缺少 clang-format (建议 23.1.3)"
  exit 1
fi

FILE_LIST="$(find "${SCAN_DIRS[@]}" \( -name '*.cpp' -o -name '*.h' \) ! -name 'pinyin_table.cpp' | sort)"
FILES=()
if [[ -n "${FILE_LIST}" ]]; then
  while IFS= read -r f; do FILES+=("$f"); done <<< "${FILE_LIST}"
fi
if [[ ${#FILES[@]} -eq 0 ]]; then
  echo "[FAIL] 未找到源文件"
  exit 1
fi
if ! "${CLANG_FORMAT}" --dry-run --Werror --style=file "${FILES[@]}" > /tmp/tiecodec-format.log 2>&1; then
  echo "[FAIL] clang-format 检查未通过"
  head -40 /tmp/tiecodec-format.log
  status=1
else
  echo "[PASS] clang-format 检查通过"
fi

if grep -rnP '[ \t]+$' "${FILES[@]}" > /tmp/tiecodec-trailing.log 2>/dev/null; then
  echo "[FAIL] 存在行尾空白"
  head -20 /tmp/tiecodec-trailing.log
  status=1
else
  echo "[PASS] 无行尾空白"
fi
if grep -rnP '\t' "${FILES[@]}" > /tmp/tiecodec-tab.log 2>/dev/null; then
  echo "[FAIL] 存在制表符缩进"
  head -20 /tmp/tiecodec-tab.log
  status=1
else
  echo "[PASS] 无制表符缩进"
fi
exit ${status}
