#!/usr/bin/env python3
"""yuleASR 需求追溯自动化工具 — @req 注解 ↔ 验收矩阵 交叉核对。

功能:
  1. 扫描源码 (src/) 与测试 (tests/) 目录中的 ``@req SWS_*`` / ``@req SHALL_*``
     注解, 汇总每个需求 ID 的"实现文件"与"测试文件"清单;
  2. 解析 ``.yuleosh/audit/acceptance-matrix.md`` 验收矩阵 (Req ID → 验证方法 →
     测试文件), 校验矩阵引用的测试文件与 ``::测试函数`` 是否真实存在;
  3. 交叉引用: 对每条矩阵需求, 汇总其测试文件中携带的 ``@req`` 注解, 标记
     "无注解支撑"的矩阵行; 对注解侧, 统计 SWS ID 的实现↔测试双向追溯率;
  4. 输出覆盖率统计与未覆盖需求列表, 格式: JSON 报告 + 控制台摘要。

用法:
  python3 tools/traceability/trace_requirements.py                     # 默认参数
  python3 tools/traceability/trace_requirements.py --fail-under 90     # CI 门禁
  python3 tools/traceability/trace_requirements.py -o my-report.json   # 自定义输出

退出码:
  0 = 成功 (覆盖率 ≥ --fail-under 阈值, 默认 0)
  1 = 覆盖率低于阈值 (CI 门禁失败)
  2 = 输入错误 (矩阵文件缺失 / 目录不存在)
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from collections import defaultdict
from datetime import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

DEFAULT_MATRIX = ".yuleosh/audit/acceptance-matrix.md"
DEFAULT_OUTPUT = ".yuleosh/audit/trace-requirements-report.json"

# @req 注解正则: 兼容 /* @req X */ 与 /** @req X */ 及一行多个 @req
REQ_RE = re.compile(r"@req\s+([A-Za-z0-9_]+)")

# 测试文件单元格中的路径 Token (可带 ::function 后缀)
FILE_TOKEN_RE = re.compile(
    r"(?P<path>(?:[A-Za-z0-9_.\-]+/)*[A-Za-z0-9_.\-]+\.(?:c|h|py|md|txt|json|sh|cfg|yaml|yml))"
    r"(?:::(?P<func>[A-Za-z0-9_]+))?"
)

# 扫描时跳过的目录 (构建产物 / 缓存)
SKIP_DIRS = {"build", "build-*", "__pycache__", ".git", "CMakeFiles", "node_modules"}

# 参与注解扫描的文件后缀
SCAN_SUFFIXES = {".c", ".h", ".py"}


def _is_skipped(name: str) -> bool:
    """判断目录名是否应被扫描跳过 (支持 build-* 通配)。"""
    if name in SKIP_DIRS and "*" not in name:
        return True
    return name.startswith("build-") or name == "build"


def scan_annotations(dirs: list[Path], bucket: str) -> dict[str, list[str]]:
    """扫描目录树, 返回 {req_id: [文件相对路径, ...]} 注解索引。

    参数:
        dirs:   待扫描的目录列表 (存在者生效)
        bucket: 'src' 或 'tests', 用于调用方分类归档
    返回:
        req_id → 出现该注解的文件相对路径列表 (排序去重)
    """
    index: dict[str, list[str]] = defaultdict(list)
    for base in dirs:
        if not base.is_dir():
            continue
        for path in sorted(base.rglob("*")):
            if not path.is_file() or path.suffix not in SCAN_SUFFIXES:
                continue
            if any(_is_skipped(p) for p in path.relative_to(ROOT).parts[:-1]):
                continue
            try:
                text = path.read_text(encoding="utf-8", errors="ignore")
            except OSError:
                continue
            for req_id in set(REQ_RE.findall(text)):
                rel = str(path.relative_to(ROOT))
                if rel not in index[req_id]:
                    index[req_id].append(rel)
    return index


def parse_matrix(matrix_path: Path) -> list[dict]:
    """解析 acceptance-matrix.md 表格, 返回需求行列表。

    返回字段: req_id / requirement / method / evidence (原始单元格) /
    confidence / status_ok / refs (解析后的文件+函数引用列表)
    """
    rows: list[dict] = []
    unnamed = 0
    in_table = False
    for line in matrix_path.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()
        if not stripped.startswith("|"):
            in_table = False
            continue
        cells = [c.strip() for c in stripped.strip("|").split("|")]
        if cells and cells[0] == "Req ID":
            in_table = True
            continue
        if not in_table or set("".join(cells)) <= set(":- "):
            continue  # 分隔行 |:---|:---|
        if len(cells) < 5:
            continue
        req_id = cells[0]
        if req_id in ("None", "—", "-", ""):
            unnamed += 1
            req_id = f"UNNAMED-{unnamed:02d}"
        refs = _parse_evidence_cell(cells[4])
        rows.append(
            {
                "req_id": req_id,
                "requirement": cells[2] if len(cells) > 2 else cells[1],
                "method": cells[3] if len(cells) > 3 else "",
                "evidence_raw": cells[4],
                "confidence": cells[6] if len(cells) > 6 else "",
                "status_ok": "✅" in (cells[7] if len(cells) > 7 else stripped),
                "refs": refs,
            }
        )
    return rows


def _parse_evidence_cell(cell: str) -> list[dict]:
    """解析测试文件单元格 → [{'path': ..., 'func': ...|None}, ...]。

    支持: ``path.c::func``、``a.c::f1 + f2`` (裸函数名沿用上一文件)、
    ``a.md + b.json`` 等组合。
    """
    refs: list[dict] = []
    last_path: str | None = None
    for part in cell.split("+"):
        part = part.strip().strip("`")
        if not part:
            continue
        m = FILE_TOKEN_RE.search(part)
        if m and m.group("path"):
            last_path = m.group("path")
            refs.append({"path": last_path, "func": m.group("func")})
        elif part.startswith(".") or ("/" in part and " " not in part):
            # 无扩展名的路径 Token (如 .misra_config)
            last_path = part
            refs.append({"path": last_path, "func": None})
        elif re.fullmatch(r"[A-Za-z0-9_]+", part) and last_path:
            # 裸函数名 → 归属上一个文件
            if not refs or refs[-1]["func"] != part:
                refs.append({"path": last_path, "func": part})
    return refs


def verify_matrix_rows(rows: list[dict]) -> tuple[list[dict], list[dict], list[dict]]:
    """校验矩阵每行引用的文件/函数是否存在。

    返回: (uncovered, broken_files, broken_functions) 三个列表。
    """
    uncovered: list[dict] = []
    broken_files: list[dict] = []
    broken_functions: list[dict] = []
    for row in rows:
        if not row["refs"]:
            if not row["status_ok"]:
                uncovered.append(
                    {"req_id": row["req_id"], "requirement": row["requirement"]}
                )
            continue
        for ref in row["refs"]:
            fpath = ROOT / ref["path"]
            if not fpath.is_file():
                broken_files.append(
                    {
                        "req_id": row["req_id"],
                        "missing_file": ref["path"],
                    }
                )
                continue
            if ref["func"] and fpath.suffix in {".c", ".h", ".py"}:
                content = fpath.read_text(encoding="utf-8", errors="ignore")
                if ref["func"] not in content:
                    broken_functions.append(
                        {
                            "req_id": row["req_id"],
                            "file": ref["path"],
                            "missing_function": ref["func"],
                        }
                    )
    return uncovered, broken_files, broken_functions


def module_of(req_id: str) -> str:
    """从注解 ID 推断模块名: SWS_SecOC_00010 → SecOC, SHALL_CDD → CDD。"""
    parts = req_id.split("_")
    return parts[1] if len(parts) >= 2 else req_id


def build_report(
    rows: list[dict],
    src_index: dict[str, list[str]],
    test_index: dict[str, list[str]],
    scanned: dict[str, int],
) -> dict:
    """汇总 JSON 报告结构 (矩阵核对 + 注解追溯 + 模块统计)。"""
    uncovered, broken_files, broken_functions = verify_matrix_rows(rows)

    # 注解双向追溯
    all_ids = sorted(set(src_index) | set(test_index))
    sws_ids = [i for i in all_ids if i.startswith("SWS_")]
    other_ids = [i for i in all_ids if not i.startswith("SWS_")]
    both = [i for i in all_ids if i in src_index and i in test_index]
    src_only = [i for i in all_ids if i in src_index and i not in test_index]
    test_only = [i for i in all_ids if i not in src_index and i in test_index]

    by_module: dict[str, dict] = defaultdict(
        lambda: {"ids": 0, "with_tests": 0, "src_only": 0}
    )
    for i in all_ids:
        mod = module_of(i)
        by_module[mod]["ids"] += 1
        if i in test_index:
            by_module[mod]["with_tests"] += 1
        elif i in src_index:
            by_module[mod]["src_only"] += 1

    # 矩阵行 ↔ 测试文件注解交叉引用
    row_details = []
    rows_without_annotation = []
    for row in rows:
        anns: list[str] = []
        for ref in row["refs"]:
            fpath = ROOT / ref["path"]
            if fpath.is_file() and fpath.suffix in SCAN_SUFFIXES:
                content = fpath.read_text(encoding="utf-8", errors="ignore")
                anns.extend(sorted(set(REQ_RE.findall(content))))
        detail = dict(row)
        detail["annotations_in_evidence"] = anns
        row_details.append(detail)
        if row["refs"] and not anns and row["method"] == "Unit Test":
            rows_without_annotation.append(
                {"req_id": row["req_id"], "evidence": row["evidence_raw"]}
            )

    covered = sum(1 for r in rows if r["refs"] and r["status_ok"])
    matrix_pct = (covered / len(rows) * 100.0) if rows else 0.0
    trace_pct = (len(both) / len(all_ids) * 100.0) if all_ids else 0.0

    return {
        "generated": datetime.now().isoformat(timespec="seconds"),
        "tool": "tools/traceability/trace_requirements.py",
        "matrix": {
            "path": str(DEFAULT_MATRIX),
            "total_requirements": len(rows),
            "covered": covered,
            "coverage_pct": round(matrix_pct, 1),
            "uncovered": uncovered,
            "broken_file_links": broken_files,
            "broken_function_links": broken_functions,
        },
        "annotations": {
            "files_scanned": scanned,
            "total_ids": len(all_ids),
            "sws_ids": len(sws_ids),
            "other_ids": len(other_ids),
            "implementation_and_test": both,
            "src_only": src_only,
            "test_only": test_only,
            "test_trace_pct": round(trace_pct, 1),
            "by_module": {
                m: {
                    "ids": v["ids"],
                    "with_tests": v["with_tests"],
                    "src_only": v["src_only"],
                }
                for m, v in sorted(by_module.items())
            },
        },
        "matrix_rows": row_details,
        "rows_without_req_annotation": rows_without_annotation,
    }


def print_summary(report: dict, output_path: Path) -> None:
    """打印控制台摘要 (覆盖率统计 + 未覆盖需求列表)。"""
    m = report["matrix"]
    a = report["annotations"]
    line = "=" * 64
    print(line)
    print(" yuleASR 需求追溯报告 — trace_requirements.py")
    print(line)
    print(f"[1] 验收矩阵核对 ({m['path']})")
    print(f"    需求总数:      {m['total_requirements']}")
    print(f"    已关联证据:    {m['covered']} ({m['coverage_pct']}%)")
    print(f"    未覆盖需求:    {len(m['uncovered'])}")
    print(f"    失效文件引用:  {len(m['broken_file_links'])}")
    print(f"    失效函数引用:  {len(m['broken_function_links'])}")
    if m["uncovered"]:
        for item in m["uncovered"][:20]:
            print(f"      ❌ {item['req_id']}: {item['requirement'][:60]}")
        if len(m["uncovered"]) > 20:
            print(f"      ... 其余 {len(m['uncovered']) - 20} 条见 JSON 报告")
    if m["broken_file_links"]:
        for item in m["broken_file_links"][:10]:
            print(f"      ⛔ {item['req_id']}: 文件缺失 {item['missing_file']}")
    if m["broken_function_links"]:
        for item in m["broken_function_links"][:10]:
            print(
                f"      ⚠️  {item['req_id']}: 函数缺失 "
                f"{item['missing_function']} @ {item['file']}"
            )

    print(f"[2] @req 注解扫描 (src + tests)")
    scanned = a["files_scanned"]
    print(f"    扫描文件:      src={scanned.get('src', 0)}, tests={scanned.get('tests', 0)}")
    print(f"    注解 ID 总数:  {a['total_ids']} (SWS_*: {a['sws_ids']}, 其他: {a['other_ids']})")
    print(f"    实现↔测试追溯: {len(a['implementation_and_test'])} ({a['test_trace_pct']}%)")
    print(f"    仅实现侧注解:  {len(a['src_only'])}  ← 需补充测试侧 @req 注解")
    print(f"    仅测试侧注解:  {len(a['test_only'])}")

    print("[3] 模块级 SWS 注解覆盖 (按数量排序, Top 15)")
    modules = sorted(
        a["by_module"].items(), key=lambda kv: kv[1]["ids"], reverse=True
    )[:15]
    for mod, v in modules:
        pct = (v["with_tests"] / v["ids"] * 100.0) if v["ids"] else 0.0
        print(
            f"    {mod:<12} {v['ids']:>4} ids | "
            f"测试追溯 {v['with_tests']:>3} ({pct:.0f}%) | 仅实现 {v['src_only']:>3}"
        )

    no_ann = report["rows_without_req_annotation"]
    print(f"[4] 矩阵行注解支撑 (Unit Test 行无 @req 注解的): {len(no_ann)}")
    if no_ann:
        for item in no_ann[:10]:
            print(f"      • {item['req_id']}: {item['evidence'][:60]}")

    print(line)
    print(f"JSON 报告: {output_path}")
    print(line)


def main(argv: list[str] | None = None) -> int:
    """CLI 入口: 参数解析 → 扫描 → 交叉核对 → 输出报告。"""
    global ROOT  # noqa: PLW0603 — CLI 允许重定向根目录
    parser = argparse.ArgumentParser(
        description="yuleASR 需求追溯: @req 注解 ↔ acceptance-matrix 交叉核对"
    )
    parser.add_argument(
        "--matrix",
        default=DEFAULT_MATRIX,
        help=f"验收矩阵路径 (默认: {DEFAULT_MATRIX})",
    )
    parser.add_argument(
        "-o", "--output",
        default=DEFAULT_OUTPUT,
        help=f"JSON 报告输出路径 (默认: {DEFAULT_OUTPUT})",
    )
    parser.add_argument(
        "--src",
        action="append",
        default=["src"],
        help="源码扫描目录 (可多次指定, 默认: src)",
    )
    parser.add_argument(
        "--tests",
        action="append",
        default=["tests"],
        help="测试扫描目录 (可多次指定, 默认: tests)",
    )
    parser.add_argument(
        "--fail-under",
        type=float,
        default=0.0,
        metavar="PCT",
        help="矩阵覆盖率门禁百分比, 低于则退出码 1 (默认: 0)",
    )
    parser.add_argument(
        "--root",
        default=str(ROOT),
        help="项目根目录 (默认: 脚本所在仓库根)",
    )
    args = parser.parse_args(argv)

    ROOT = Path(args.root).resolve()

    matrix_path = ROOT / args.matrix
    if not matrix_path.is_file():
        print(f"[ERROR] 验收矩阵不存在: {matrix_path}", file=sys.stderr)
        return 2

    src_dirs = [ROOT / d for d in args.src]
    test_dirs = [ROOT / d for d in args.tests]
    for d in src_dirs + test_dirs:
        if not d.is_dir():
            print(f"[WARN] 扫描目录不存在, 已跳过: {d}", file=sys.stderr)

    rows = parse_matrix(matrix_path)
    if not rows:
        print(f"[ERROR] 矩阵中未解析到需求行: {matrix_path}", file=sys.stderr)
        return 2

    src_index = scan_annotations(src_dirs, "src")
    test_index = scan_annotations(test_dirs, "tests")

    def count_files(dirs: list[Path]) -> int:
        n = 0
        for base in dirs:
            if base.is_dir():
                n += sum(
                    1
                    for p in base.rglob("*")
                    if p.is_file()
                    and p.suffix in SCAN_SUFFIXES
                    and not any(_is_skipped(x) for x in p.relative_to(ROOT).parts[:-1])
                )
        return n

    scanned = {"src": count_files(src_dirs), "tests": count_files(test_dirs)}
    report = build_report(rows, src_index, test_index, scanned)

    output_path = ROOT / args.output
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(
        json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )

    print_summary(report, output_path)

    if report["matrix"]["coverage_pct"] < args.fail_under:
        print(
            f"[GATE] 覆盖率 {report['matrix']['coverage_pct']}% < 阈值 {args.fail_under}% → FAIL",
            file=sys.stderr,
        )
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
