#!/usr/bin/env bash
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
COMPILER="${1:-${PROJECT_DIR}/build/tiecodec}"
STDLIB="${PROJECT_DIR}/stdlibs/安卓基本库/源代码"

if [[ ! -x "${COMPILER}" ]]; then
  echo "[FAIL] 缺少编译器: ${COMPILER}" >&2
  exit 1
fi
if [[ ! -d "${STDLIB}" ]]; then
  echo "[FAIL] 缺少标准库源文件: ${STDLIB}" >&2
  exit 1
fi

WORK="$(mktemp -d)"
trap 'rm -rf "${WORK}"' EXIT
mkdir -p "${WORK}/src"
cat > "${WORK}/src/启动窗口.t" <<'APP'
类 启动窗口 : 窗口
	变量 问候: 文本 = "你好"

	事件 启动窗口:创建完毕()
		调试输出(问候)
	结束 事件
结束 类
APP

mapfile -t SOURCES < <(find "${STDLIB}" "${WORK}/src" -name '*.t' | sort)
( cd "${WORK}" && "${COMPILER}" --platform android --release --friendly-name original \
    -o out "${SOURCES[@]}" > compile.log 2>&1 )
status=$?
if [[ ${status} -ne 0 ]]; then
  echo "[FAIL] 冒烟编译失败"
  sed -n '1,40p' "${WORK}/compile.log"
  exit 1
fi
if [[ ! -f "${WORK}/out/src/启动窗口.java" ]]; then
  echo "[FAIL] 冒烟编译未生成启动窗口.java"
  exit 1
fi
if ! grep -q "public void 创建完毕()" "${WORK}/out/src/启动窗口.java"; then
  echo "[FAIL] 冒烟编译输出缺少事件方法"
  exit 1
fi
echo "[PASS] 冒烟编译通过"
