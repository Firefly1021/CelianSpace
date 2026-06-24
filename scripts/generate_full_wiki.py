from pathlib import Path
from urllib.parse import quote
import hashlib
import html
import json
import re

SITE = Path(__file__).resolve().parents[1]
MATERIALS = SITE / "materials"
WIKI = SITE / "wiki"
ASSETS = WIKI / "assets"
ASSET_VERSION = "20260624-latex-env-fix"

TEXT_EXTS = {".tex", ".sty", ".bib", ".md", ".cpp", ".c", ".h", ".hpp", ".py", ".txt", ".cmake"}
CODE_EXTS = {".cpp", ".c", ".h", ".hpp", ".py", ".cmake"}
IMAGE_EXTS = {".png", ".jpg", ".jpeg", ".webp", ".gif", ".svg"}
PDF_EXTS = {".pdf"}
CSV_EXTS = {".csv"}
DATA_EXTS = {".gz", ".ubyte"}

MATH_META = {
    "Algebra_NumberTheory.tex": ("algebra-number-theory", "代数与数论", "代数与几何", "群、环、数论结构与代数对象。"),
    "Classical_Calculus.tex": ("classical-calculus", "经典微积分", "分析基础", "微分、积分与经典分析结果。"),
    "Complex_Analysis.tex": ("complex-analysis", "复变函数", "分析基础", "复数、全纯函数、积分表示与级数展开。"),
    "Finite_Element.tex": ("finite-element", "有限元方法初步", "数值计算与有限元", "变分问题、有限元空间、混合有限元与计算流程。"),
    "Foutier_Analysis.tex": ("pde-fourier-analysis", "偏微分方程与傅里叶分析", "PDE 与分析", "古典 PDE、广义函数、Sobolev 空间与椭圆方程。"),
    "Function_Analysis.tex": ("functional-analysis", "泛函分析", "分析基础", "Banach 空间、线性算子、对偶空间与复习选题。"),
    "Machine_Learning.tex": ("machine-learning", "机器学习基础", "数值计算与有限元", "线性代数、机器学习基本要素与线性模型。"),
    "Numerical_Analysis.tex": ("numerical-analysis", "数值分析", "数值分析与 PDE", "守恒律、PDE 数值解、离散极值原理与 Helmholtz 方程。"),
    "Real_Analysis.tex": ("real-analysis", "实分析", "分析基础", "Lebesgue 测度、抽象测度和积分。"),
    "Set_Axiomatics.tex": ("set-axiomatics", "集合与公理化", "基础结构", "公理化思想与集合论基础。"),
    "Topology_Geometry.tex": ("topology-geometry", "拓扑与几何", "代数与几何", "拓扑学基本知识和微分几何。"),
}


def read_text(path: Path) -> str:
    data = path.read_bytes()
    for enc in ("utf-8-sig", "utf-8", "gb18030", "latin1"):
        try:
            return data.decode(enc)
        except UnicodeDecodeError:
            pass
    return data.decode("utf-8", errors="replace")


def write(path: Path, text: str):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def material_rel(path: Path) -> str:
    return path.relative_to(MATERIALS).as_posix()


def qpath(path: str) -> str:
    return "/".join(quote(part) for part in path.split("/"))


def raw_href(material_path: str, depth: int) -> str:
    return "../" * (depth + 1) + "materials/" + qpath(material_path)


def wiki_href(url: str, depth: int) -> str:
    return "../" * depth + url


def page_id(material_path: str) -> str:
    digest = hashlib.sha1(material_path.encode("utf-8")).hexdigest()[:12]
    stem = re.sub(r"[^A-Za-z0-9]+", "-", material_path).strip("-").lower()[:54] or "file"
    return f"files/{digest}-{stem}.html"


def group_name(material_path: str) -> str:
    return material_path.split("/")[0] if "/" in material_path else "资料说明"


def file_kind(path: Path) -> str:
    ext = path.suffix.lower()
    if ext in IMAGE_EXTS:
        return "image"
    if ext in PDF_EXTS:
        return "pdf"
    if ext in CSV_EXTS:
        return "csv"
    if ext == ".md":
        return "markdown"
    if ext in {".tex", ".sty", ".bib"}:
        return "latex"
    if ext in CODE_EXTS:
        return "code"
    if ext in DATA_EXTS:
        return "data"
    if ext in TEXT_EXTS:
        return "text"
    return "file"


def human_size(size: int) -> str:
    value = float(size)
    for unit in ["B", "KB", "MB", "GB"]:
        if value < 1024 or unit == "GB":
            return f"{value:.1f} {unit}" if unit != "B" else f"{int(value)} B"
        value /= 1024


def clean_latex(text: str) -> str:
    text = re.sub(r"%.*$", "", text).strip()
    for old, new in [
        ("\\newpage", ""),
        ("\\quad", " "),
        ("\\qquad", " "),
        ("\\,", " "),
        ("~", " "),
    ]:
        text = text.replace(old, new)
    text = re.sub(r"\\label\{[^}]+\}", "", text)
    text = re.sub(r"\\ref\{[^}]+\}", "相关式", text)
    text = re.sub(r"\\eqref\{[^}]+\}", "相关方程", text)
    text = re.sub(r"\\cite\{[^}]+\}", "", text)
    return text.strip()


def code_language(path: Path | None = None, lang: str = "") -> str:
    if lang:
        return re.sub(r"[^A-Za-z0-9_+#.-]+", "", lang).lower()
    if not path:
        return "text"
    ext = path.suffix.lower()
    return {
        ".cpp": "cpp",
        ".c": "c",
        ".h": "cpp",
        ".hpp": "cpp",
        ".py": "python",
        ".cmake": "cmake",
        ".md": "markdown",
        ".tex": "tex",
        ".bib": "bibtex",
        ".sty": "tex",
        ".csv": "csv",
        ".txt": "text",
    }.get(ext, "text")


def code_panel(code: str, title: str = "代码片段", lang: str = "text") -> str:
    safe_lang = html.escape(lang or "text")
    return (
        '<div class="code-block">'
        f'<div class="code-title"><span>{html.escape(title)}</span><em>{safe_lang}</em></div>'
        f'<pre><code class="language-{safe_lang}">{html.escape(code)}</code></pre>'
        "</div>"
    )


def inline_html(text: str) -> str:
    text = clean_latex(text)
    text = re.sub(r"\\(textbf|emph|textit)\{([^{}]*)\}", lambda m: f"@@{m.group(1)}:{m.group(2)}@@", text)
    text = re.sub(r"\\(title|author|date|frametitle)\{([^{}]*)\}", r"\2", text)
    out = html.escape(text).replace("\\\\", "<br>")
    out = re.sub(r"@@textbf:(.*?)@@", r"<strong>\1</strong>", out)
    out = re.sub(r"@@emph:(.*?)@@", r"<em>\1</em>", out)
    out = re.sub(r"@@textit:(.*?)@@", r"<em>\1</em>", out)
    return out


def resolve_image(current_file: Path, expr: str):
    expr = expr.strip().strip("{}")
    for candidate in [current_file.parent / expr, current_file.parent.parent / expr, MATERIALS / expr]:
        if candidate.exists():
            return material_rel(candidate)
    return None


def render_latex(text: str, current_file: Path, depth: int) -> str:
    text = text.replace("\\par", "\n\n")
    out, para = [], []
    env, env_lines = None, []
    in_code, code_lines = False, []
    in_display, display_lines = False, []
    note_envs = {"Definition", "Theorem", "Lemma", "Proposition", "Corollary", "Example", "Remark", "definition", "theorem", "lemma", "proposition", "corollary", "example", "remark"}
    math_envs = {"equation", "equation*", "align", "align*", "gather", "gather*", "cases", "matrix", "pmatrix", "bmatrix"}

    def flush_para():
        nonlocal para
        if para:
            text = " ".join(para).strip()
            if text:
                out.append(f"<p>{inline_html(text)}</p>")
            para = []

    def push_math(lines, kind=""):
        formula = clean_latex("\n".join(lines))
        if not formula:
            return
        if kind in {"align", "align*"} and not formula.startswith("\\begin"):
            formula = "\\begin{aligned}\n" + formula + "\n\\end{aligned}"
        if kind == "cases" and not formula.startswith("\\begin"):
            formula = "\\begin{cases}\n" + formula + "\n\\end{cases}"
        out.append(f'<div class="math-display">\\[{html.escape(formula)}\\]</div>')

    for raw in text.splitlines():
        line = raw.strip()
        if not line or line.startswith("%"):
            flush_para()
            continue
        if in_code:
            if re.match(r"\\end\{(lstlisting|minted|verbatim)\}", line):
                out.append(code_panel(chr(10).join(code_lines), "LaTeX 代码", "tex"))
                in_code, code_lines = False, []
            else:
                code_lines.append(raw.rstrip())
            continue
        if in_display:
            marker_line = line.replace("\\]", "$$")
            if "$$" in marker_line:
                before = marker_line.split("$$", 1)[0].strip()
                if before and before not in {".", ",", ";"}:
                    display_lines.append(before)
                push_math(display_lines)
                in_display, display_lines = False, []
            else:
                display_lines.append(line)
            continue
        if re.match(r"\\begin\{(lstlisting|minted|verbatim)\}", line):
            flush_para()
            in_code = True
            code_lines = []
            continue
        if env:
            if re.match(r"\\end\{" + re.escape(env) + r"\}", line):
                if env in note_envs:
                    out.append(f'<aside class="wiki-note">{inline_html(" ".join(env_lines))}</aside>')
                else:
                    push_math(env_lines, env)
                env, env_lines = None, []
            else:
                env_lines.append(line)
            continue
        if line.startswith("$$") or line.startswith("\\["):
            flush_para()
            rest = line[2:].strip()
            marker_rest = rest.replace("\\]", "$$")
            if "$$" in marker_rest and marker_rest.split("$$", 1)[0].strip():
                push_math([marker_rest.split("$$", 1)[0].strip()])
            else:
                in_display, display_lines = True, ([rest] if rest else [])
            continue
        heading = re.match(r"\\(section|subsection|subsubsection)\*?\{(.+)\}", line)
        if heading:
            flush_para()
            tag = {"section": "h2", "subsection": "h3", "subsubsection": "h4"}[heading.group(1)]
            out.append(f"<{tag}>{inline_html(heading.group(2))}</{tag}>")
            continue
        frame = re.match(r"\\frametitle\{(.+)\}", line)
        if frame:
            flush_para()
            out.append(f"<h2>{inline_html(frame.group(1))}</h2>")
            continue
        caption = re.match(r"\\caption\{(.+)\}", line)
        if caption:
            flush_para()
            out.append(f'<p class="caption">{inline_html(caption.group(1))}</p>')
            continue
        image = re.search(r"\\includegraphics(?:\[[^\]]*\])?\{([^}]+)\}", line)
        if image:
            flush_para()
            resolved = resolve_image(current_file, image.group(1))
            if resolved:
                href = raw_href(resolved, depth)
                out.append(f'<figure class="asset-figure"><img src="{href}" alt="{html.escape(Path(resolved).name)}" loading="lazy" /><figcaption>{html.escape(resolved)}</figcaption></figure>')
            continue
        if line.startswith("\\begin{tcolorbox"):
            flush_para()
            title = re.search(r"title\s*=\s*\\textbf\{([^}]*)\}", line)
            out.append('<aside class="wiki-note">')
            if title:
                out.append(f'<strong>{inline_html(title.group(1))}</strong>')
            continue
        if line.startswith("\\end{tcolorbox"):
            flush_para()
            out.append("</aside>")
            continue
        begin = re.match(r"\\begin\{([^}]+)\}", line)
        if begin:
            name = begin.group(1)
            if name in math_envs:
                flush_para()
                env, env_lines = name, []
            elif name in note_envs:
                flush_para()
                env, env_lines = name, []
            else:
                flush_para()
            continue
        if line.startswith("\\end{"):
            flush_para()
            continue
        if line.startswith(("\\documentclass", "\\usepackage", "\\newcommand", "\\renewcommand", "\\set", "\\bibliography", "\\bibliographystyle")):
            continue
        if line in {"\\begin{document}", "\\end{document}", "\\maketitle", "\\cover", "\\reference"}:
            continue
        if line.startswith(("\\input", "\\include")):
            out.append(f'<p class="inline-source">{inline_html(line)}</p>')
            continue
        para.append(line)
    flush_para()
    if in_code and code_lines:
        out.append(code_panel(chr(10).join(code_lines), "LaTeX 代码", "tex"))
    if env and env_lines:
        if env in note_envs:
            out.append(f'<aside class="wiki-note">{inline_html(" ".join(env_lines))}</aside>')
        else:
            push_math(env_lines, env)
    return "\n".join(out) or "<p>该文件主要包含样式、宏定义或参考条目，请使用原始文件查看。</p>"


def render_markdown(text: str) -> str:
    out, para, code, code_lines, code_lang = [], [], False, [], "text"

    def flush():
        nonlocal para
        if para:
            out.append("<p>" + html.escape(" ".join(para)) + "</p>")
            para = []

    for raw in text.splitlines():
        line = raw.rstrip()
        if line.strip().startswith("```"):
            if code:
                out.append(code_panel(chr(10).join(code_lines), "Markdown 代码块", code_lang))
                code, code_lines, code_lang = False, [], "text"
            else:
                flush()
                code = True
                code_lang = code_language(lang=line.strip()[3:].strip() or "text")
            continue
        if code:
            code_lines.append(line)
            continue
        if not line.strip():
            flush()
            continue
        h = re.match(r"^(#{1,4})\s+(.+)", line)
        if h:
            flush()
            tag = "h" + str(min(len(h.group(1)) + 1, 4))
            out.append(f"<{tag}>{html.escape(h.group(2))}</{tag}>")
            continue
        if line.strip().startswith(("- ", "* ")):
            flush()
            out.append(f'<ul class="doc-list"><li>{html.escape(line.strip()[2:])}</li></ul>')
            continue
        para.append(line.strip())
    flush()
    if code and code_lines:
        out.append(code_panel(chr(10).join(code_lines), "Markdown 代码块", code_lang))
    return "\n".join(out)


KIND_LABELS = {"latex": "TeX", "code": "代码", "markdown": "Markdown", "image": "图片", "pdf": "PDF", "csv": "CSV", "data": "数据", "text": "文本", "file": "文件"}


def folder_label(record) -> str:
    parts = record["material_rel"].split("/")
    if len(parts) <= 2:
        return record["group"]
    return " / ".join(parts[1:-1])


def file_details(record, depth: int) -> str:
    raw = raw_href(record["material_rel"], depth)
    return (
        '<details class="file-info">'
        '<summary>文件入口</summary>'
        '<div class="file-info-body">'
        f'<a class="button-link" href="{raw}">打开原始文件</a>'
        f'<span>分类：{html.escape(record["group"])}</span>'
        f'<span>类型：{html.escape(KIND_LABELS.get(record["kind"], "文件"))}</span>'
        f'<span>大小：{human_size(record["size"])}</span>'
        f'<span>位置：{html.escape(record["material_rel"])}</span>'
        '</div></details>'
    )


def render_file(path: Path, record, depth: int) -> str:
    raw = raw_href(record["material_rel"], depth)
    kind = record["kind"]
    if kind == "image":
        return f'<figure class="asset-figure large"><img src="{raw}" alt="{html.escape(path.name)}" loading="lazy" /><figcaption>{html.escape(path.stem)}</figcaption></figure>'
    if kind == "pdf":
        return f'<div class="pdf-frame"><iframe src="{raw}" title="{html.escape(path.name)}"></iframe></div>'
    if kind == "csv":
        lines = read_text(path).splitlines()
        return f'<p>CSV 共约 {len(lines)} 行。页面显示前 300 行。</p>{code_panel(chr(10).join(lines[:300]), path.name, "csv")}'
    if kind == "data":
        return f'<p>这是数据文件或压缩数据包，大小 {human_size(path.stat().st_size)}。为保持网页可读，页面不展开二进制内容。</p>'
    if kind == "markdown":
        return render_markdown(read_text(path))
    if kind == "latex":
        return render_latex(read_text(path), path, depth)
    if kind == "code":
        text = read_text(path)
        if not text.strip():
            return '<p>该源码文件当前为空。</p>'
        return code_panel(text, path.name, code_language(path))
    if path.suffix.lower() in TEXT_EXTS:
        text = read_text(path)
        if not text.strip():
            return '<p>该文本文件当前为空。</p>'
        return code_panel(text, path.name, code_language(path))
    return '<p>该文件无法作为文本展开，请使用原始文件链接查看。</p>'


def search_excerpt(path: Path, kind: str) -> str:
    if kind in {"image", "pdf", "data"}:
        return path.name
    try:
        return re.sub(r"\s+", " ", read_text(path))[:1600]
    except Exception:
        return path.name


def category_url(group: str) -> str:
    mapping = {
        "数学基础": "materials/math-source.html",
        "各种模板": "materials/templates-source.html",
        "LearningCode": "materials/learning-code-source.html",
        "code学习文档": "materials/code-docs-source.html",
        "资料说明": "materials/index.html",
    }
    return mapping[group]


def nav_groups(math_pages):
    return [
        ("数学基础", [(p["title"], f'math/{p["slug"]}.html') for p in math_pages]),
        ("完整资料", [("资料总览", "materials/index.html"), ("数学基础源文件", category_url("数学基础")), ("各种模板", category_url("各种模板")), ("LearningCode", category_url("LearningCode")), ("code学习文档", category_url("code学习文档"))]),
        ("专题入口", [("模板总览", "templates/index.html"), ("代码实验", "code/index.html"), ("学习文档", "docs/index.html")]),
    ]


def sidebar(current: str, depth: int, math_pages) -> str:
    pre = "../" * depth
    parts = [f'<a class="sidebar-home" href="{pre}index.html">CelianSpace Wiki</a>']
    for name, links in nav_groups(math_pages):
        parts.append(f"<details open><summary>{html.escape(name)}</summary><nav>")
        for title, url in links:
            active = " active" if url == current else ""
            parts.append(f'<a class="{active.strip()}" href="{pre}{url}">{html.escape(title)}</a>')
        parts.append("</nav></details>")
    return "\n".join(parts)


MATHJAX = r"""<script>window.MathJax = { tex: { inlineMath: [['$', '$'], ['\\(', '\\)']], macros: { R: '\\mathbb{R}', N: '\\mathbb{N}', C: '\\mathbb{C}', K: '\\mathbb{K}', A: '\\boldsymbol{A}', m: 'm^*', tR: '\\tilde{\\mathcal{R}}', X: '(X,\\mathcal{M},\\mu)', ii: '\\mathrm{i}', dd: '\\mathrm{d}', pa: '\\partial', paa: '\\partial^{\\alpha}', Lp: 'L^p(\\Omega)', sbev: 'W^{k,p}(\\Omega)', sch: '\\mathcal{S}(\\mathbb{R}^n)', vecb: ['\\boldsymbol{#1}', 1], ttt: '\\mathcal{T}', dx: '\\,\\mathrm{d}x', curl: '\\operatorname{curl}', diverge: '\\operatorname{div}', E: '\\boldsymbol{E}', Hfield: '\\boldsymbol{H}', J: '\\boldsymbol{J}', n: '\\boldsymbol{n}', tvec: '\\boldsymbol{t}', norm: ['\\left\\|#1\\right\\|', 1], tnorm: ['|\\!|\\!|#1|\\!|\\!|', 1] } }, svg: { fontCache: 'global' } };</script>"""


def page(title: str, body: str, current: str, depth: int, math_pages, description="") -> str:
    pre = "../" * depth
    return f"""<!doctype html>
<html lang="zh-CN">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate" />
  <meta http-equiv="Pragma" content="no-cache" />
  <meta http-equiv="Expires" content="0" />
  <meta name="description" content="{html.escape(description or title)}" />
  <title>{html.escape(title)} | CelianSpace Wiki</title>
  <link rel="stylesheet" href="{pre}assets/wiki.css?v={ASSET_VERSION}" />
  <script defer src="{pre}assets/search-index.js?v={ASSET_VERSION}"></script>
  <script defer src="{pre}assets/wiki.js?v={ASSET_VERSION}"></script>
  {MATHJAX}
  <script defer src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-svg.js"></script>
</head>
<body data-depth="{depth}">
  <header class="wiki-topbar">
    <button class="menu-toggle" type="button" aria-label="打开目录">目录</button>
    <a class="top-brand" href="{pre}index.html">CelianSpace Wiki</a>
    <label class="wiki-search"><span>搜索</span><input id="wiki-search" type="search" placeholder="搜索数学、代码、模板、文件..." autocomplete="off" /></label>
    <a class="top-link" href="{pre}../index.html">个人主页</a>
  </header>
  <div class="search-results" id="search-results" hidden></div>
  <div class="wiki-shell">
    <aside class="wiki-sidebar" id="wiki-sidebar">{sidebar(current, depth, math_pages)}</aside>
    <main class="wiki-main">{body}</main>
    <aside class="wiki-toc"><p>本文目录</p><nav id="page-toc"></nav></aside>
  </div>
</body>
</html>"""


def article_header(title: str, subtitle="", chips=None, source="", source_url="") -> str:
    chips = chips or []
    chip_html = "".join(f"<span>{html.escape(c)}</span>" for c in chips)
    subtitle_html = f'<p class="doc-subtitle">{html.escape(subtitle)}</p>' if subtitle else ""
    chips_html = f'<div class="doc-chips">{chip_html}</div>' if chip_html else ""
    return f'<article class="doc-article"><header class="doc-header"><p class="breadcrumb">CelianSpace Wiki</p><h1>{html.escape(title)}</h1>{subtitle_html}{chips_html}</header>'


def close_article() -> str:
    return "</article>"


def file_card(record, depth: int) -> str:
    return f'''<a class="file-card" data-kind="{record["kind"]}" data-group="{html.escape(record["group"])}" href="{wiki_href(record["page_url"], depth)}">
  <span class="file-kind">{KIND_LABELS.get(record["kind"], "文件")}</span><strong>{html.escape(record["title"])}</strong><small>{html.escape(folder_label(record))}</small>
</a>'''


def group_folder_key(record) -> str:
    parts = record["material_rel"].split("/")
    if len(parts) <= 2:
        return KIND_LABELS.get(record["kind"], "文件")
    return parts[1]


def folded_file_panels(items, depth: int, mode: str = "folder", open_first: bool = True) -> str:
    buckets = {}
    for record in items:
        key = record["group"] if mode == "group" else group_folder_key(record)
        buckets.setdefault(key, []).append(record)
    parts = []
    for index, key in enumerate(sorted(buckets, key=lambda k: (k != "Section", k.lower()))):
        subset = buckets[key]
        open_attr = " open" if open_first and index == 0 else ""
        cards = "".join(file_card(r, depth) for r in subset)
        parts.append(
            f'<details class="fold-panel"{open_attr}>'
            f'<summary><strong>{html.escape(key)}</strong><span>{len(subset)} 个条目</span></summary>'
            f'<div class="file-grid">{cards}</div>'
            '</details>'
        )
    return '<div class="fold-list">' + "".join(parts) + "</div>"


def main():
    for d in ["files", "materials", "math", "templates", "code", "docs", "assets"]:
        (WIKI / d).mkdir(parents=True, exist_ok=True)
    for folder in [WIKI / "files", WIKI / "materials", WIKI / "math", WIKI / "templates", WIKI / "code", WIKI / "docs"]:
        for child in folder.rglob("*"):
            if child.is_file():
                child.unlink()

    records = []
    for path in sorted([p for p in MATERIALS.rglob("*") if p.is_file()], key=lambda p: material_rel(p).lower()):
        mrel = material_rel(path)
        kind = file_kind(path)
        records.append({
            "path": path,
            "material_rel": mrel,
            "title": path.name,
            "group": group_name(mrel),
            "kind": kind,
            "size": path.stat().st_size,
            "page_url": page_id(mrel),
            "search_text": search_excerpt(path, kind),
        })
    records_by_rel = {r["material_rel"]: r for r in records}

    math_pages = []
    for filename, (slug, title, category, desc) in MATH_META.items():
        path = MATERIALS / "数学基础" / "Section" / filename
        if path.exists():
            math_pages.append({
                "slug": slug,
                "title": title,
                "category": category,
                "description": desc,
                "source": material_rel(path),
                "html": render_latex(read_text(path), path, 1),
            })

    for record in records:
        kind_label = KIND_LABELS.get(record["kind"], "文件")
        body = article_header(record["title"], f'{record["group"]} 中的{kind_label}资料。', [record["group"], kind_label])
        body += f'<section class="doc-section"><div class="file-actions"><a class="button-link" href="{wiki_href(category_url(record["group"]), 1)}">返回分类</a></div>{render_file(record["path"], record, 1)}{file_details(record, 1)}</section>{close_article()}'
        write(WIKI / record["page_url"], page(record["title"], body, record["page_url"], 1, math_pages, record["material_rel"]))

    for index, item in enumerate(math_pages):
        prev_item = math_pages[index - 1] if index else None
        next_item = math_pages[index + 1] if index + 1 < len(math_pages) else None
        pn = '<nav class="prev-next">'
        pn += f'<a href="{prev_item["slug"]}.html">上一页：{html.escape(prev_item["title"])}</a>' if prev_item else "<span></span>"
        pn += f'<a href="{next_item["slug"]}.html">下一页：{html.escape(next_item["title"])}</a>' if next_item else "<span></span>"
        pn += "</nav>"
        source_details = file_details(records_by_rel[item["source"]], 1) if item["source"] in records_by_rel else ""
        body = article_header(item["title"], item["description"], [item["category"], "完整正文"])
        body += f'<section class="doc-section">{item["html"]}{source_details}</section>{pn}{close_article()}'
        write(WIKI / "math" / f'{item["slug"]}.html', page(item["title"], body, f'math/{item["slug"]}.html', 1, math_pages, item["description"]))

    def group_page(group: str, title: str, subtitle: str):
        subset = [r for r in records if r["group"] == group]
        stats = {}
        for r in subset:
            label = KIND_LABELS.get(r["kind"], "文件")
            stats[label] = stats.get(label, 0) + 1
        stat_html = "".join(f"<span>{html.escape(k)}：{v}</span>" for k, v in sorted(stats.items()))
        body = article_header(title, subtitle, [f"{len(subset)} 个文件", "完整索引"])
        body += f'<section class="doc-section"><div class="material-tools"><input id="materialFilter" type="search" placeholder="筛选本页文件..." /><div class="kind-summary">{stat_html}</div></div>{folded_file_panels(subset, 1, "folder")}</section>{close_article()}'
        write(WIKI / category_url(group), page(title, body, category_url(group), 1, math_pages, subtitle))

    group_page("数学基础", "数学基础源文件", "LaTeX 主文件、11 个完整 Section、样式文件、参考文献和配图。")
    group_page("各种模板", "各种模板", "课程报告模板与 Beamer 模板的源码、PDF、样式、图片和参考文献。")
    group_page("LearningCode", "LearningCode", "Python、C++、数值实验、MNIST 压缩数据和结果图。")
    group_page("code学习文档", "code学习文档", "Markdown、LaTeX、PDF 和 C++ 示例代码。")

    group_cards = "".join(f'<a href="{category_url(g)}"><strong>{html.escape(g)}</strong><span>{sum(1 for r in records if r["group"] == g)} 个文件</span></a>' for g in ["数学基础", "各种模板", "LearningCode", "code学习文档"])
    body = article_header("完整资料库", "全部有效资料已按主题收纳，可搜索、筛选、预览或进入正文阅读。", [f"{len(records)} 个文件", "完整覆盖", "可展开目录"])
    body += f'<section class="doc-section"><div class="wiki-hero-grid">{group_cards}</div><h2>资料目录</h2><div class="material-tools"><input id="materialFilter" type="search" placeholder="输入文件名、主题或类型筛选..." /><div class="kind-summary"><span>TeX / 代码 / Markdown 正文展示</span><span>图片 / PDF 嵌入预览</span><span>数据文件提供入口</span></div></div>{folded_file_panels(records, 1, "group", open_first=False)}</section>{close_article()}'
    write(WIKI / "materials" / "index.html", page("完整资料库", body, "materials/index.html", 1, math_pages, "完整资料库"))

    for out, current, title, subtitle, group in [
        (WIKI / "templates" / "index.html", "templates/index.html", "模板总览", "课程报告和 Beamer 模板的全部源码、样式、PDF 与图片。", "各种模板"),
        (WIKI / "code" / "index.html", "code/index.html", "代码实验", "LearningCode 中的 Python、C++、数据、结果图和数值实验输出。", "LearningCode"),
        (WIKI / "docs" / "index.html", "docs/index.html", "学习文档", "code学习文档 中的 Markdown、LaTeX、PDF 和示例代码。", "code学习文档"),
    ]:
        subset = [r for r in records if r["group"] == group]
        body = article_header(title, subtitle, [f"{len(subset)} 个文件", "专题入口"])
        body += f'<section class="doc-section"><div class="material-tools"><input id="materialFilter" type="search" placeholder="筛选本页文件..." /></div>{folded_file_panels(subset, 1, "folder")}</section>{close_article()}'
        write(out, page(title, body, current, 1, math_pages, subtitle))

    category_cards = []
    for cat in ["分析基础", "代数与几何", "PDE 与分析", "数值计算与有限元", "数值分析与 PDE", "基础结构"]:
        pages = [p for p in math_pages if p["category"] == cat]
        if pages:
            links = "".join(f'<a href="math/{p["slug"]}.html"><strong>{html.escape(p["title"])}</strong><span>{html.escape(p["description"])}</span></a>' for p in pages)
            category_cards.append(f'<section class="category-card"><h2>{html.escape(cat)}</h2>{links}</section>')
    home_cards = "".join([
        f'<a href="materials/index.html"><strong>完整资料库</strong><span>{len(records)} 个材料文件全部纳入网页。</span></a>',
        '<a href="math/real-analysis.html"><strong>数学基础</strong><span>11 个 Section 按方向分类，全文渲染。</span></a>',
        '<a href="templates/index.html"><strong>模板</strong><span>报告、Beamer、样式、图片和 PDF。</span></a>',
        '<a href="code/index.html"><strong>代码实验</strong><span>Python、C++、数据和结果图。</span></a>',
    ])
    body = article_header("CelianSpace 文档站", "面向数学、代码、模板和学习文档的完整资料型网页。", ["多页面", "完整资料", "公式渲染", "搜索"])
    body += f'<section class="doc-section"><div class="wiki-hero-grid">{home_cards}</div><h2>数学基础分类</h2><div class="category-grid">{"".join(category_cards)}</div></section>{close_article()}'
    write(WIKI / "index.html", page("CelianSpace 文档站", body, "index.html", 0, math_pages, "CelianSpace 多页面文档站"))

    search_items = [{"title": "CelianSpace 文档站", "url": "index.html", "category": "入口", "text": "完整资料库 数学基础 模板 代码 学习文档"}]
    for p in math_pages:
        text = re.sub(r"\s+", " ", re.sub(r"<[^>]+>", " ", p["html"]))[:1600]
        search_items.append({"title": p["title"], "url": f'math/{p["slug"]}.html', "category": p["category"], "text": p["description"] + " " + p["source"] + " " + text})
    for r in records:
        search_items.append({"title": r["title"], "url": r["page_url"], "category": r["group"] + " / " + r["kind"], "text": r["material_rel"] + " " + r["search_text"]})
    for title, url in [("完整资料库", "materials/index.html"), ("数学基础源文件", category_url("数学基础")), ("各种模板", category_url("各种模板")), ("LearningCode", category_url("LearningCode")), ("code学习文档", category_url("code学习文档"))]:
        search_items.append({"title": title, "url": url, "category": "完整资料", "text": title})
    write(ASSETS / "search-index.json", json.dumps(search_items, ensure_ascii=False, indent=2))
    write(ASSETS / "search-index.js", "window.WIKI_SEARCH_INDEX = " + json.dumps(search_items, ensure_ascii=False, indent=2) + ";\n")

    css = r''':root{--bg:#f6f8fb;--surface:#fff;--line:#d9e0e8;--text:#1f2937;--muted:#64748b;--brand:#128276;--brand-soft:#e0f4f0;--code:#172724;--shadow:0 18px 55px rgba(30,45,70,.08);--radius:8px;--mono:"Cascadia Code",Consolas,monospace;--sans:Inter,"Segoe UI",system-ui,sans-serif;--serif:Georgia,"Noto Serif SC","Songti SC",serif}*{box-sizing:border-box}html{scroll-behavior:smooth}body{margin:0;overflow-x:hidden;background:var(--bg);color:var(--text);font-family:var(--sans);line-height:1.75}.wiki-topbar{position:sticky;top:0;z-index:30;display:grid;grid-template-columns:auto auto minmax(260px,560px) auto;gap:16px;align-items:center;height:58px;padding:0 24px;border-bottom:1px solid var(--line);background:rgba(255,255,255,.94);backdrop-filter:blur(16px)}.top-brand{font-weight:800;color:var(--brand);text-decoration:none}.top-link{justify-self:end;color:var(--muted);font-size:.9rem;text-decoration:none}.menu-toggle{display:none;border:1px solid var(--line);border-radius:6px;background:#fff;padding:7px 10px}.wiki-search{display:grid;grid-template-columns:auto 1fr;gap:10px;align-items:center;color:var(--muted);font-size:.82rem}.wiki-search input,.material-tools input{height:36px;border:1px solid var(--line);border-radius:999px;padding:0 14px;font:inherit;background:#fff}.search-results{position:fixed;top:60px;left:50%;z-index:40;width:min(680px,calc(100% - 32px));max-height:420px;overflow:auto;transform:translateX(-50%);border:1px solid var(--line);border-radius:var(--radius);background:#fff;box-shadow:var(--shadow)}.search-results a{display:block;padding:12px 14px;border-bottom:1px solid var(--line);color:var(--text);text-decoration:none}.search-results small{display:block;color:var(--brand);font-family:var(--mono)}.wiki-shell{display:grid;grid-template-columns:280px minmax(0,1fr) 220px;gap:28px;max-width:1500px;margin:0 auto;padding:28px 26px}.wiki-sidebar{position:sticky;top:82px;align-self:start;max-height:calc(100vh - 100px);overflow:auto;padding:14px;border:1px solid var(--line);border-radius:var(--radius);background:#fff}.sidebar-home{display:block;margin-bottom:12px;color:var(--brand);font-weight:800;text-decoration:none}.wiki-sidebar details{border-top:1px solid var(--line);padding:10px 0}.wiki-sidebar summary{cursor:pointer;font-weight:700}.wiki-sidebar nav{display:grid;gap:2px;margin-top:8px}.wiki-sidebar a{display:block;padding:7px 9px;border-radius:6px;color:var(--muted);font-size:.92rem;text-decoration:none}.wiki-sidebar a:hover,.wiki-sidebar a.active{background:var(--brand-soft);color:var(--brand)}.wiki-main{min-width:0}.doc-article{border:1px solid var(--line);border-radius:var(--radius);background:var(--surface);box-shadow:var(--shadow)}.doc-header{padding:42px 48px 28px;border-bottom:1px solid var(--line);background:linear-gradient(135deg,#fff,#eefaf7)}.breadcrumb,.inline-source{margin:0;color:var(--muted);font-family:var(--mono);font-size:.78rem}.doc-header h1{margin:12px 0 0;font-family:var(--serif);font-size:clamp(2.2rem,5vw,4.6rem);line-height:1.05;font-weight:500;overflow-wrap:anywhere}.doc-subtitle{max-width:840px;color:var(--muted);font-size:1.08rem;overflow-wrap:anywhere}.doc-chips,.kind-summary{display:flex;flex-wrap:wrap;gap:8px;margin-top:16px}.doc-chips span,.kind-summary span{padding:5px 10px;border:1px solid rgba(18,130,118,.2);border-radius:999px;background:#fff;color:var(--brand);font-family:var(--mono);font-size:.74rem}.doc-section{padding:34px 48px}.doc-section h2{margin:36px 0 12px;padding-top:4px;font-family:var(--serif);font-size:2rem;line-height:1.18}.doc-section h3{margin:30px 0 10px;font-size:1.35rem}.doc-section h4{margin:24px 0 8px}.doc-section p{margin:12px 0}.doc-section p code{padding:2px 5px;border-radius:4px;background:#eef3f6}.caption{color:var(--muted);font-size:.9rem}.math-display{overflow-x:auto;margin:18px 0;padding:18px;border:1px solid var(--line);border-radius:var(--radius);background:#fbfdfc}.wiki-note,blockquote{margin:18px 0;padding:14px 16px;border-left:4px solid var(--brand);background:var(--brand-soft);color:#22413d}.wiki-note strong{display:block;margin-bottom:6px}.doc-list{padding-left:22px}.code-block{overflow:hidden;margin:18px 0;border:1px solid #203c36;border-radius:var(--radius);background:var(--code)}.code-title{display:flex;justify-content:space-between;gap:12px;align-items:center;padding:8px 13px;border-bottom:1px solid rgba(255,255,255,.08);color:#c8eee6;font:0.78rem/1.4 var(--mono)}.code-title span{overflow-wrap:anywhere}.code-title em{font-style:normal;color:#8ccfc3}.doc-section pre{overflow:auto;max-height:78vh;margin:0;padding:18px;background:transparent;color:#effaf6;font:0.86rem/1.7 var(--mono)}.wiki-hero-grid,.category-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:14px}.wiki-hero-grid a,.category-card{padding:20px;border:1px solid var(--line);border-radius:var(--radius);background:#fff;text-decoration:none;color:var(--text)}.wiki-hero-grid strong,.category-card strong{display:block;font-family:var(--serif);font-size:1.35rem;font-weight:500}.wiki-hero-grid span,.category-card span{display:block;color:var(--muted)}.category-card a{display:block;margin-top:12px;padding-top:12px;border-top:1px solid var(--line);text-decoration:none;color:var(--text)}.fold-list{display:grid;gap:12px}.fold-panel{border:1px solid var(--line);border-radius:var(--radius);background:#fff}.fold-panel summary{display:flex;justify-content:space-between;gap:16px;align-items:center;cursor:pointer;padding:15px 18px;color:var(--text)}.fold-panel summary strong{font-family:var(--serif);font-size:1.2rem;font-weight:500}.fold-panel summary span{color:var(--muted);font-size:.9rem}.fold-panel[open] summary{border-bottom:1px solid var(--line);background:#fbfdfc}.file-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(240px,1fr));gap:12px;margin:18px}.file-card{display:grid;gap:6px;min-height:116px;padding:16px;border:1px solid var(--line);border-radius:var(--radius);background:#fff;color:var(--text);text-decoration:none}.file-card:hover{border-color:rgba(18,130,118,.45);box-shadow:0 12px 35px rgba(30,45,70,.08)}.file-card strong{overflow-wrap:anywhere}.file-card small{color:var(--muted);font-family:var(--mono);font-size:.74rem;overflow-wrap:anywhere}.file-kind{width:max-content;padding:2px 8px;border-radius:999px;background:var(--brand-soft);color:var(--brand);font-family:var(--mono);font-size:.72rem}.material-tools{display:grid;gap:10px;margin:4px 0 18px}.file-actions{display:flex;flex-wrap:wrap;gap:10px;margin-bottom:18px}.button-link{display:inline-block;padding:8px 12px;border:1px solid var(--line);border-radius:6px;background:#fff;color:var(--brand);text-decoration:none}.file-info{margin-top:28px;border:1px solid var(--line);border-radius:var(--radius);background:#fbfdfc}.file-info summary{cursor:pointer;padding:11px 14px;color:var(--brand);font-weight:700}.file-info-body{display:flex;flex-wrap:wrap;gap:10px 14px;align-items:center;padding:0 14px 14px;color:var(--muted);font-size:.88rem}.file-info-body span{overflow-wrap:anywhere}.asset-figure{margin:18px 0;padding:14px;border:1px solid var(--line);border-radius:var(--radius);background:#fff}.asset-figure img{display:block;max-width:100%;height:auto;margin:auto}.asset-figure.large img{max-height:78vh}.asset-figure figcaption{margin-top:8px;color:var(--muted);font-family:var(--mono);font-size:.75rem;overflow-wrap:anywhere}.pdf-frame{height:72vh;border:1px solid var(--line);border-radius:var(--radius);overflow:hidden;background:#fff}.pdf-frame iframe{width:100%;height:100%;border:0}.wiki-toc{position:sticky;top:82px;align-self:start;max-height:calc(100vh - 100px);overflow:auto;color:var(--muted);font-size:.88rem}.wiki-toc p{margin:0 0 10px;color:var(--text);font-weight:700}.wiki-toc a{display:block;padding:5px 0;color:var(--muted);text-decoration:none}.wiki-toc a:hover{color:var(--brand)}.prev-next{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-top:18px}.prev-next a,.prev-next span{padding:16px;border:1px solid var(--line);border-radius:var(--radius);background:#fff;color:var(--brand);text-decoration:none}mjx-container[display="true"]{display:block;max-width:100%;overflow-x:auto;overflow-y:hidden}mjx-container[display="true"] svg{max-width:100%;height:auto}@media(max-width:1100px){.wiki-shell{grid-template-columns:240px minmax(0,1fr)}.wiki-toc{display:none}}@media(max-width:760px){.wiki-topbar{grid-template-columns:auto 1fr auto;height:auto;min-height:58px;padding:10px 14px}.menu-toggle{display:inline-block}.wiki-search{grid-column:1/-1;grid-template-columns:1fr}.wiki-search span{display:none}.top-link{display:none}.wiki-shell{display:block;padding:14px}.wiki-sidebar{position:fixed;top:0;bottom:0;left:0;z-index:60;width:min(86vw,320px);max-height:none;transform:translateX(-105%);transition:transform .2s ease;border-radius:0}.wiki-sidebar.open{transform:translateX(0)}.doc-header{padding:30px 22px 22px}.doc-section{padding:24px 22px}.wiki-hero-grid,.category-grid,.prev-next{grid-template-columns:1fr}.doc-header h1{font-size:2.35rem}.doc-section p{overflow-x:auto}.file-grid{grid-template-columns:1fr;margin:14px}.fold-panel summary{align-items:flex-start;flex-direction:column;gap:2px}}'''
    write(ASSETS / "wiki.css", css + "\n")
    js = r'''const depth=Number(document.body.dataset.depth||0);const prefix='../'.repeat(depth);const sidebar=document.querySelector('#wiki-sidebar');document.querySelector('.menu-toggle')?.addEventListener('click',()=>sidebar.classList.toggle('open'));document.addEventListener('click',event=>{if(innerWidth<760&&sidebar?.classList.contains('open')&&!sidebar.contains(event.target)&&!event.target.closest('.menu-toggle'))sidebar.classList.remove('open')});const toc=document.querySelector('#page-toc');[...document.querySelectorAll('.doc-section h2, .doc-section h3')].forEach((heading,index)=>{if(!heading.id)heading.id=`section-${index+1}`;const a=document.createElement('a');a.href=`#${heading.id}`;a.textContent=heading.textContent;if(heading.tagName==='H3')a.style.paddingLeft='12px';toc?.appendChild(a)});const input=document.querySelector('#wiki-search');const box=document.querySelector('#search-results');let searchIndex=window.WIKI_SEARCH_INDEX||[];if(!searchIndex.length)fetch(`${prefix}assets/search-index.json`).then(r=>r.json()).then(data=>{searchIndex=data}).catch(()=>{});input?.addEventListener('input',()=>{const q=input.value.trim().toLowerCase();if(!q){box.hidden=true;box.innerHTML='';return}const hits=searchIndex.filter(item=>`${item.title} ${item.category} ${item.text}`.toLowerCase().includes(q)).slice(0,12);box.innerHTML=hits.length?hits.map(item=>`<a href="${prefix}${item.url}"><small>${item.category}</small>${item.title}</a>`).join(''):'<a>没有找到匹配内容</a>';box.hidden=false});input?.addEventListener('blur',()=>setTimeout(()=>{box.hidden=true},180));const materialFilter=document.querySelector('#materialFilter');materialFilter?.addEventListener('input',()=>{const q=materialFilter.value.trim().toLowerCase();document.querySelectorAll('.file-card').forEach(card=>{card.hidden=!!q&&!card.textContent.toLowerCase().includes(q)&&!(card.dataset.kind||'').toLowerCase().includes(q)&&!(card.dataset.group||'').toLowerCase().includes(q)});document.querySelectorAll('.fold-panel').forEach(panel=>{const cards=[...panel.querySelectorAll('.file-card')];const hasVisible=cards.some(card=>!card.hidden);panel.hidden=q&&!hasVisible;if(q&&hasVisible)panel.open=true})});window.addEventListener('load',()=>setTimeout(()=>document.querySelectorAll('mjx-assistive-mml').forEach(node=>node.remove()),600));'''
    write(ASSETS / "wiki.js", js + "\n")

    readme = SITE / "README.md"
    text = readme.read_text(encoding="utf-8")
    if "完整资料索引" not in text:
        text += "\n\n## 资料库生成\n\n`wiki/` 现在包含 `materials/` 中全部有效文件的完整资料索引：TeX、Markdown 和代码以网页形式展示，图片与 PDF 直接预览，数据文件提供摘要和原始文件入口。\n"
        readme.write_text(text, encoding="utf-8")

    print(json.dumps({"records": len(records), "math_pages": len(math_pages), "search_items": len(search_items)}, ensure_ascii=False))


if __name__ == "__main__":
    main()
