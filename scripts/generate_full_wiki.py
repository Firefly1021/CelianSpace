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
ASSET_VERSION = "20260630-code-display"

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


def split_latex_paragraphs(text: str) -> str:
    return re.sub(r"\\par(?![A-Za-z])", "\n\n", text)


def code_language(path: Path | None = None, lang: str = "") -> str:
    if lang:
        clean = re.sub(r"[^A-Za-z0-9_+#.-]+", "", lang).lower()
        return {"c++": "cpp", "py": "python", "sh": "shell", "bash": "shell", "text": "plaintext", "tex": "latex"}.get(clean, clean)
    if not path:
        return "plaintext"
    if path.name.lower() == "cmakelists.txt":
        return "cmake"
    ext = path.suffix.lower()
    return {
        ".cpp": "cpp",
        ".c": "c",
        ".h": "cpp",
        ".hpp": "cpp",
        ".py": "python",
        ".cmake": "cmake",
        ".md": "markdown",
        ".tex": "latex",
        ".bib": "bibtex",
        ".sty": "latex",
        ".csv": "csv",
        ".txt": "plaintext",
    }.get(ext, "plaintext")


def code_panel(code: str, title: str = "代码片段", lang: str = "plaintext") -> str:
    safe_lang = html.escape(lang or "plaintext")
    safe_title = html.escape(title)
    escaped_code = html.escape(code.rstrip() or "\n")
    return (
        '<div class="code-block">'
        '<div class="code-title">'
        f'<span>{safe_title}</span>'
        f'<div class="code-tools"><em>{safe_lang}</em><button type="button" class="copy-code" title="复制代码" aria-label="复制代码">复制</button></div>'
        '</div>'
        f'<pre><code class="language-{safe_lang}">{escaped_code}</code></pre>'
        "</div>"
    )


def strip_latex_comment(line: str) -> str:
    for index, char in enumerate(line):
        if char == "%" and (index == 0 or line[index - 1] != "\\"):
            return line[:index]
    return line


def tikz_render_source(code: str) -> str:
    lines = []
    for raw in code.splitlines():
        line = strip_latex_comment(raw).rstrip()
        if not line.strip():
            continue
        line = re.sub(r",\s*bend(?=[,\]])", "", line)
        line = re.sub(r"(?<=\[)\s*bend\s*,?", "", line)
        line = "".join(char for char in line if ord(char) < 128)
        lines.append(line)
    return "\n".join(lines)


def tikz_panel(code: str) -> str:
    renderable = "\\begin{axis}" not in code and "\\end{axis}" not in code
    tikz_libraries = "shapes.geometric,intersections,positioning,arrows.meta,bending,calc,decorations.markings,patterns"
    tikz_code = f"\\usetikzlibrary{{{tikz_libraries}}}\n" + tikz_render_source(code)
    tikz_code = tikz_code.replace("</script", "<\\/script")
    canvas = (
        f'<script type="text/tikz">{tikz_code}</script>'
        if renderable
        else '<div class="tikz-fallback">该图使用 pgfplots/axis，当前浏览器渲染器暂不支持在线图形化展示。</div>'
    )
    return (
        '<figure class="tikz-figure">'
        '<div class="tikz-canvas">'
        f'{canvas}'
        '</div>'
        '<figcaption>TikZ 图形</figcaption>'
        '</figure>'
    )


def table_html(lines) -> str:
    rows = []
    for raw in lines:
        line = strip_latex_comment(raw).strip()
        if not line or line.startswith("\\begin") or line.startswith("\\end"):
            continue
        line = re.sub(r"\\(hline|toprule|midrule|bottomrule)\b", "", line).strip()
        if not line:
            continue
        for row in re.split(r"\\\\", line):
            row = row.strip()
            if not row:
                continue
            cells = [inline_html(cell.strip()) for cell in row.split("&")]
            rows.append(cells)
    if not rows:
        return ""
    body = []
    for index, cells in enumerate(rows):
        tag = "th" if index == 0 else "td"
        body.append("<tr>" + "".join(f"<{tag}>{cell}</{tag}>" for cell in cells) + "</tr>")
    return '<div class="table-wrap"><table class="latex-table">' + "".join(body) + "</table></div>"


def list_html(items, ordered=False) -> str:
    tag = "ol" if ordered else "ul"
    body = "".join(f"<li>{inline_html(' '.join(item).strip())}</li>" for item in items if " ".join(item).strip())
    return f'<{tag} class="doc-list">{body}</{tag}>' if body else ""


def algorithm_html(lines) -> str:
    items = []
    for raw in lines:
        line = strip_latex_comment(raw).strip()
        if not line or line.startswith("\\begin") or line.startswith("\\end"):
            continue
        line = re.sub(r"\\State\b", "", line).strip()
        line = re.sub(r"\\While\s*\{(.+)\}", r"While: \1", line)
        line = re.sub(r"\\For\s*\{(.+)\}", r"For: \1", line)
        line = re.sub(r"\\If\s*\{(.+)\}", r"If: \1", line)
        line = re.sub(r"\\(EndWhile|EndFor|EndIf)\b", r"End", line)
        if line:
            items.append(inline_html(line))
    if not items:
        return ""
    return '<ol class="algorithm-list">' + "".join(f"<li>{item}</li>" for item in items) + "</ol>"


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
    text = split_latex_paragraphs(text)
    out, para = [], []
    env, env_lines = None, []
    in_code, code_lines = False, []
    in_tikz, tikz_lines = False, []
    in_tikz_setup, tikz_setup_lines, tikz_setup_depth = False, [], 0
    pending_tikz_lines = []
    in_table, table_lines = False, []
    in_algorithm, algorithm_lines = False, []
    list_kind, list_items, current_item = None, [], []
    in_display, display_lines = False, []
    note_envs = {"Definition", "Theorem", "Lemma", "Proposition", "Corollary", "Example", "Remark", "definition", "theorem", "lemma", "proposition", "corollary", "example", "remark"}
    math_envs = {"equation", "equation*", "align", "align*", "gather", "gather*", "cases", "matrix", "pmatrix", "bmatrix", "array", "aligned", "alignedat", "split"}

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

    def flush_list():
        nonlocal list_kind, list_items, current_item
        if list_kind:
            if current_item:
                list_items.append(current_item)
            html_list = list_html(list_items, ordered=(list_kind == "enumerate"))
            if html_list:
                out.append(html_list)
            list_kind, list_items, current_item = None, [], []

    for raw in text.splitlines():
        line = raw.strip()
        if in_tikz_setup:
            tikz_setup_lines.append(raw.rstrip())
            tikz_setup_depth += raw.count("{") - raw.count("}")
            if tikz_setup_depth <= 0:
                pending_tikz_lines.extend(tikz_setup_lines)
                in_tikz_setup, tikz_setup_lines, tikz_setup_depth = False, [], 0
            continue
        if not line or line.startswith("%"):
            flush_para()
            continue
        if in_table:
            table_lines.append(raw.rstrip())
            if re.match(r"\\end\{tabular\}", line):
                rendered_table = table_html(table_lines)
                if rendered_table:
                    out.append(rendered_table)
                in_table, table_lines = False, []
            continue
        if in_algorithm:
            algorithm_lines.append(raw.rstrip())
            if re.match(r"\\end\{algorithmic\}", line):
                rendered_algorithm = algorithm_html(algorithm_lines)
                if rendered_algorithm:
                    out.append(rendered_algorithm)
                in_algorithm, algorithm_lines = False, []
            continue
        if list_kind:
            if re.match(r"\\end\{" + list_kind + r"\}", line):
                flush_list()
                continue
            item = re.match(r"\\item(?:\[[^\]]*\])?\s*(.*)", line)
            if item:
                if current_item:
                    list_items.append(current_item)
                current_item = [item.group(1)]
            elif current_item:
                current_item.append(line)
            continue
        if in_tikz:
            tikz_lines.append(raw.rstrip())
            if re.match(r"\\end\{tikzpicture\}", line):
                out.append(tikz_panel(chr(10).join(tikz_lines)))
                in_tikz, tikz_lines = False, []
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
        if re.match(r"\\begin\{tabular\}", line) or re.match(r"\\begin\{tabular\}", line.replace("[t]", "")):
            flush_para()
            in_table = True
            table_lines = [raw.rstrip()]
            continue
        if re.match(r"\\begin\{algorithmic\}", line):
            flush_para()
            in_algorithm = True
            algorithm_lines = [raw.rstrip()]
            continue
        list_begin = re.match(r"\\begin\{(itemize|enumerate)\}", line)
        if list_begin:
            flush_para()
            list_kind, list_items, current_item = list_begin.group(1), [], []
            continue
        if line.startswith("\\tikzset"):
            flush_para()
            tikz_setup_lines = [raw.rstrip()]
            tikz_setup_depth = raw.count("{") - raw.count("}")
            if tikz_setup_depth <= 0:
                pending_tikz_lines.extend(tikz_setup_lines)
                tikz_setup_lines, tikz_setup_depth = [], 0
            else:
                in_tikz_setup = True
            continue
        if re.match(r"\\begin\{tikzpicture\}", line):
            flush_para()
            in_tikz = True
            tikz_lines = pending_tikz_lines + [raw.rstrip()]
            pending_tikz_lines = []
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
                out.append(f'<figure class="asset-figure"><img src="{href}" alt="{html.escape(Path(resolved).name)}" loading="lazy" /></figure>')
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
            flush_list()
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
    flush_list()
    if in_code and code_lines:
        out.append(code_panel(chr(10).join(code_lines), "LaTeX 代码", "tex"))
    if in_tikz and tikz_lines:
        out.append(tikz_panel(chr(10).join(tikz_lines)))
    if in_table and table_lines:
        rendered_table = table_html(table_lines)
        if rendered_table:
            out.append(rendered_table)
    if in_algorithm and algorithm_lines:
        rendered_algorithm = algorithm_html(algorithm_lines)
        if rendered_algorithm:
            out.append(rendered_algorithm)
    if env and env_lines:
        if env in note_envs:
            out.append(f'<aside class="wiki-note">{inline_html(" ".join(env_lines))}</aside>')
        else:
            push_math(env_lines, env)
    return "\n".join(out) or "<p>该部分主要包含样式、宏定义或参考条目，已作为支撑材料隐藏。</p>"


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


def search_excerpt(path: Path, kind: str) -> str:
    if kind in {"image", "pdf", "data"}:
        return path.name
    try:
        return re.sub(r"\s+", " ", read_text(path))[:1600]
    except Exception:
        return path.name


def nav_groups(math_pages):
    return [
        ("数学基础", [(p["title"], f'math/{p["slug"]}.html') for p in math_pages]),
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


MATHJAX = r"""<script>window.MathJax = { tex: { inlineMath: [['$', '$'], ['\\(', '\\)']], macros: { R: '\\mathbb{R}', N: '\\mathbb{N}', C: '\\mathbb{C}', K: '\\mathbb{K}', A: '\\boldsymbol{A}', m: 'm^*', tR: '\\tilde{\\mathcal{R}}', X: '(X,\\mathcal{M},\\mu)', ii: '\\mathrm{i}', dd: '\\mathrm{d}', pa: '\\partial', paa: '\\partial^{\\alpha}', Lp: 'L^p(\\Omega)', sbev: 'W^{k,p}(\\Omega)', sch: '\\mathcal{S}(\\mathbb{R}^n)', vecb: ['\\boldsymbol{#1}', 1], ttt: '\\mathcal{T}', dx: '\\,\\mathrm{d}x', curl: '\\operatorname{curl}', diverge: '\\operatorname{div}', E: '\\boldsymbol{E}', Hfield: '\\boldsymbol{H}', J: '\\boldsymbol{J}', n: '\\boldsymbol{n}', tvec: '\\boldsymbol{t}', re: '\\operatorname{Re}', im: '\\operatorname{Im}', supp: '\\operatorname{supp}', diam: '\\operatorname{diam}', dist: '\\operatorname{dist}', induct: ['\\left\\langle #1,#2\\right\\rangle', 2], norm: ['\\left\\|#1\\right\\|', 1], tnorm: ['|\\!|\\!|#1|\\!|\\!|', 1] } }, svg: { fontCache: 'global' } };</script>"""


def page(title: str, body: str, current: str, depth: int, math_pages, description="") -> str:
    pre = "../" * depth
    tikz_assets = ""
    code_style_assets = ""
    code_script_assets = ""
    if 'class="code-block"' in body:
        code_style_assets = '\n  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/highlight.js@11.11.1/styles/github.min.css" />'
        code_script_assets = '\n  <script defer src="https://cdn.jsdelivr.net/npm/highlight.js@11.11.1/lib/common.min.js"></script>'
    if 'type="text/tikz"' in body:
        tikz_assets = '\n  <link rel="stylesheet" type="text/css" href="https://tikzjax.com/v1/fonts.css" />\n  <script src="https://tikzjax.com/v1/tikzjax.js"></script>'
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
  <link rel="icon" href="data:," />
  {code_style_assets}
  <link rel="stylesheet" href="{pre}assets/wiki.css?v={ASSET_VERSION}" />
  <script defer src="{pre}assets/search-index.js?v={ASSET_VERSION}"></script>
  <script defer src="{pre}assets/wiki.js?v={ASSET_VERSION}"></script>
  {code_script_assets}
  {MATHJAX}
  <script defer src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-svg.js"></script>
  {tikz_assets}
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


def group_folder_key(record) -> str:
    parts = record["material_rel"].split("/")
    if len(parts) <= 2:
        return KIND_LABELS.get(record["kind"], "文件")
    return parts[1]


def folded_topic_panels(items, mode: str = "folder", open_first: bool = True) -> str:
    buckets = {}
    for record in items:
        key = record["group"] if mode == "group" else group_folder_key(record)
        buckets.setdefault(key, []).append(record)
    parts = []
    for index, key in enumerate(sorted(buckets, key=lambda k: (k != "Section", k.lower()))):
        subset = buckets[key]
        open_attr = " open" if open_first and index == 0 else ""
        stats = {}
        for record in subset:
            label = KIND_LABELS.get(record["kind"], "文件")
            stats[label] = stats.get(label, 0) + 1
        stat_text = "，".join(f"{name} {count}" for name, count in sorted(stats.items()))
        stat_chips = "".join(f"<span>{html.escape(name)}：{count}</span>" for name, count in sorted(stats.items()))
        parts.append(
            f'<details class="fold-panel"{open_attr}>'
            f'<summary><strong>{html.escape(key)}</strong><span>{html.escape(stat_text)}</span></summary>'
            f'<div class="topic-summary"><p>该模块已纳入站内整理，底层支撑文件入口已隐藏。</p><div class="kind-summary">{stat_chips}</div></div>'
            '</details>'
        )
    return '<div class="fold-list">' + "".join(parts) + "</div>"


def display_title(record) -> str:
    parts = record["material_rel"].split("/")
    if len(parts) <= 1:
        return record["title"]
    return "/".join(parts[1:])


def displayable_record(record) -> bool:
    return record["kind"] in {"code", "latex", "markdown", "text", "csv", "image"}


def render_display_record(record, depth: int) -> str:
    title = display_title(record)
    kind = record["kind"]
    path = record["path"]
    if kind == "markdown":
        return f'<section class="content-entry"><h3>{html.escape(title)}</h3>{render_markdown(read_text(path))}</section>'
    if kind == "csv":
        text = read_text(path)
        lines = text.splitlines()
        shown = "\n".join(lines[:80])
        omitted = max(0, len(lines) - 80)
        note = f'<p class="topic-note">CSV 数据预览，显示前 {min(len(lines), 80)} 行' + (f'，其余 {omitted} 行未展开。' if omitted else '。') + '</p>'
        return f'<section class="content-entry"><h3>{html.escape(title)}</h3>{note}{code_panel(shown, title, "csv")}</section>'
    if kind == "image":
        href = raw_href(record["material_rel"], depth)
        return (
            '<section class="content-entry media-entry">'
            f'<h3>{html.escape(title)}</h3>'
            f'<figure class="asset-figure"><img src="{href}" alt="{html.escape(record["title"])}" loading="lazy" /></figure>'
            '</section>'
        )
    if kind in {"code", "latex", "text", "csv"}:
        text = read_text(path)
        if not text.strip():
            return f'<section class="content-entry"><h3>{html.escape(title)}</h3><p class="topic-note">该文件为空。</p></section>'
        return code_panel(text, title, code_language(path))
    return ""


def content_topic_panels(items, depth: int, open_first: bool = True) -> str:
    buckets = {}
    for record in items:
        buckets.setdefault(group_folder_key(record), []).append(record)
    parts = []
    for index, key in enumerate(sorted(buckets, key=lambda k: (k != "Section", k.lower()))):
        subset = buckets[key]
        open_attr = " open" if open_first and index == 0 else ""
        display_items = [record for record in subset if displayable_record(record)]
        hidden_count = len(subset) - len(display_items)
        stats = {}
        for record in subset:
            label = KIND_LABELS.get(record["kind"], "文件")
            stats[label] = stats.get(label, 0) + 1
        stat_text = "，".join(f"{name} {count}" for name, count in sorted(stats.items()))
        visible_count = f"已展开 {len(display_items)} 项"
        note = f'<p class="topic-note">另有 {hidden_count} 个 PDF、二进制数据或辅助文件未在页面中展开。</p>' if hidden_count else ""
        rendered = "".join(render_display_record(record, depth) for record in display_items)
        if not rendered:
            rendered = '<p class="topic-note">该模块主要包含 PDF、数据集或辅助资源，未直接展开为代码。</p>'
        parts.append(
            f'<details class="fold-panel code-topic"{open_attr}>'
            f'<summary><strong>{html.escape(key)}</strong><span>{html.escape(stat_text)} · {visible_count}</span></summary>'
            f'<div class="topic-summary">{note}{rendered}</div>'
            '</details>'
        )
    return '<div class="fold-list code-fold-list">' + "".join(parts) + "</div>"


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

    for index, item in enumerate(math_pages):
        prev_item = math_pages[index - 1] if index else None
        next_item = math_pages[index + 1] if index + 1 < len(math_pages) else None
        pn = '<nav class="prev-next">'
        pn += f'<a href="{prev_item["slug"]}.html">上一页：{html.escape(prev_item["title"])}</a>' if prev_item else "<span></span>"
        pn += f'<a href="{next_item["slug"]}.html">下一页：{html.escape(next_item["title"])}</a>' if next_item else "<span></span>"
        pn += "</nav>"
        body = article_header(item["title"], item["description"], [item["category"], "完整正文"])
        body += f'<section class="doc-section">{item["html"]}</section>{pn}{close_article()}'
        write(WIKI / "math" / f'{item["slug"]}.html', page(item["title"], body, f'math/{item["slug"]}.html', 1, math_pages, item["description"]))

    for out, current, title, subtitle, group in [
        (WIKI / "templates" / "index.html", "templates/index.html", "模板总览", "课程报告与 Beamer 模板按组成模块整理展示。", "各种模板"),
        (WIKI / "code" / "index.html", "code/index.html", "代码实验", "LearningCode 中的 Python、C++、数据与数值实验结果按模块整理。", "LearningCode"),
        (WIKI / "docs" / "index.html", "docs/index.html", "学习文档", "code 学习文档按笔记、示例与参考材料整理。", "code学习文档"),
    ]:
        subset = [r for r in records if r["group"] == group]
        displayed = sum(1 for record in subset if displayable_record(record))
        body = article_header(title, subtitle, [f"{len(subset)} 个材料条目", f"展开 {displayed} 项"])
        body += f'<section class="doc-section"><p>本页按模块直接展示代码、配置、Markdown、CSV 预览与图片预览；PDF 和二进制数据只作为支撑材料参与整理。</p>{content_topic_panels(subset, 1)}</section>{close_article()}'
        write(out, page(title, body, current, 1, math_pages, subtitle))

    category_cards = []
    for cat in ["分析基础", "代数与几何", "PDE 与分析", "数值计算与有限元", "数值分析与 PDE", "基础结构"]:
        pages = [p for p in math_pages if p["category"] == cat]
        if pages:
            links = "".join(f'<a href="math/{p["slug"]}.html"><strong>{html.escape(p["title"])}</strong><span>{html.escape(p["description"])}</span></a>' for p in pages)
            category_cards.append(f'<section class="category-card"><h2>{html.escape(cat)}</h2>{links}</section>')
    home_cards = "".join([
        '<a href="math/real-analysis.html"><strong>数学基础</strong><span>11 个 Section 按方向分类，全文渲染。</span></a>',
        '<a href="templates/index.html"><strong>模板</strong><span>报告与 Beamer 模板源码直接展示。</span></a>',
        '<a href="code/index.html"><strong>代码实验</strong><span>Python、C++、CSV 预览和结果图按模块展开。</span></a>',
        '<a href="docs/index.html"><strong>学习文档</strong><span>代码学习笔记与示例源码一起展示。</span></a>',
    ])
    body = article_header("CelianSpace 文档站", "面向数学、代码、模板和学习文档的内容型网页。", ["多页面", "内容整理", "公式渲染", "搜索"])
    body += f'<section class="doc-section"><div class="wiki-hero-grid">{home_cards}</div><h2>数学基础分类</h2><div class="category-grid">{"".join(category_cards)}</div></section>{close_article()}'
    write(WIKI / "index.html", page("CelianSpace 文档站", body, "index.html", 0, math_pages, "CelianSpace 多页面文档站"))

    search_items = [{"title": "CelianSpace 文档站", "url": "index.html", "category": "入口", "text": "数学基础 模板 代码 学习文档"}]
    for p in math_pages:
        text = re.sub(r"\s+", " ", re.sub(r"<[^>]+>", " ", p["html"]))[:1600]
        search_items.append({"title": p["title"], "url": f'math/{p["slug"]}.html', "category": p["category"], "text": p["description"] + " " + text})
    for title, url, group in [("模板总览", "templates/index.html", "各种模板"), ("代码实验", "code/index.html", "LearningCode"), ("学习文档", "docs/index.html", "code学习文档")]:
        subset = [record for record in records if record["group"] == group and displayable_record(record)]
        text = " ".join(f'{display_title(record)} {record["search_text"]}' for record in subset)[:5000]
        search_items.append({"title": title, "url": url, "category": "专题入口", "text": f"{title} {text}"})
    write(ASSETS / "search-index.json", json.dumps(search_items, ensure_ascii=False, indent=2))
    write(ASSETS / "search-index.js", "window.WIKI_SEARCH_INDEX = " + json.dumps(search_items, ensure_ascii=False, indent=2) + ";\n")

    css = r''':root{--bg:#f6f8fb;--surface:#fff;--line:#d9e0e8;--text:#1f2937;--muted:#64748b;--brand:#128276;--brand-soft:#e0f4f0;--code:#172724;--shadow:0 18px 55px rgba(30,45,70,.08);--radius:8px;--mono:"Cascadia Code",Consolas,monospace;--sans:Inter,"Segoe UI",system-ui,sans-serif;--serif:Georgia,"Noto Serif SC","Songti SC",serif}*{box-sizing:border-box}html{scroll-behavior:smooth}body{margin:0;overflow-x:hidden;background:var(--bg);color:var(--text);font-family:var(--sans);line-height:1.75}.wiki-topbar{position:sticky;top:0;z-index:30;display:grid;grid-template-columns:auto auto minmax(260px,560px) auto;gap:16px;align-items:center;height:58px;padding:0 24px;border-bottom:1px solid var(--line);background:rgba(255,255,255,.94);backdrop-filter:blur(16px)}.top-brand{font-weight:800;color:var(--brand);text-decoration:none}.top-link{justify-self:end;color:var(--muted);font-size:.9rem;text-decoration:none}.menu-toggle{display:none;border:1px solid var(--line);border-radius:6px;background:#fff;padding:7px 10px}.wiki-search{display:grid;grid-template-columns:auto 1fr;gap:10px;align-items:center;color:var(--muted);font-size:.82rem}.wiki-search input,.material-tools input{height:36px;border:1px solid var(--line);border-radius:999px;padding:0 14px;font:inherit;background:#fff}.search-results{position:fixed;top:60px;left:50%;z-index:40;width:min(680px,calc(100% - 32px));max-height:420px;overflow:auto;transform:translateX(-50%);border:1px solid var(--line);border-radius:var(--radius);background:#fff;box-shadow:var(--shadow)}.search-results a{display:block;padding:12px 14px;border-bottom:1px solid var(--line);color:var(--text);text-decoration:none}.search-results small{display:block;color:var(--brand);font-family:var(--mono)}.wiki-shell{display:grid;grid-template-columns:280px minmax(0,1fr) 220px;gap:28px;max-width:1500px;margin:0 auto;padding:28px 26px}.wiki-sidebar{position:sticky;top:82px;align-self:start;max-height:calc(100vh - 100px);overflow:auto;padding:14px;border:1px solid var(--line);border-radius:var(--radius);background:#fff}.sidebar-home{display:block;margin-bottom:12px;color:var(--brand);font-weight:800;text-decoration:none}.wiki-sidebar details{border-top:1px solid var(--line);padding:10px 0}.wiki-sidebar summary{cursor:pointer;font-weight:700}.wiki-sidebar nav{display:grid;gap:2px;margin-top:8px}.wiki-sidebar a{display:block;padding:7px 9px;border-radius:6px;color:var(--muted);font-size:.92rem;text-decoration:none}.wiki-sidebar a:hover,.wiki-sidebar a.active{background:var(--brand-soft);color:var(--brand)}.wiki-main{min-width:0}.doc-article{border:1px solid var(--line);border-radius:var(--radius);background:var(--surface);box-shadow:var(--shadow)}.doc-header{padding:42px 48px 28px;border-bottom:1px solid var(--line);background:linear-gradient(135deg,#fff,#eefaf7)}.breadcrumb,.inline-source{margin:0;color:var(--muted);font-family:var(--mono);font-size:.78rem}.doc-header h1{margin:12px 0 0;font-family:var(--serif);font-size:clamp(2.2rem,5vw,4.6rem);line-height:1.05;font-weight:500;overflow-wrap:anywhere}.doc-subtitle{max-width:840px;color:var(--muted);font-size:1.08rem;overflow-wrap:anywhere}.doc-chips,.kind-summary{display:flex;flex-wrap:wrap;gap:8px;margin-top:16px}.doc-chips span,.kind-summary span{padding:5px 10px;border:1px solid rgba(18,130,118,.2);border-radius:999px;background:#fff;color:var(--brand);font-family:var(--mono);font-size:.74rem}.doc-section{padding:34px 48px}.doc-section h2{margin:36px 0 12px;padding-top:4px;font-family:var(--serif);font-size:2rem;line-height:1.18}.doc-section h3{margin:30px 0 10px;font-size:1.35rem}.doc-section h4{margin:24px 0 8px}.doc-section p{margin:12px 0}.doc-section p code{padding:2px 5px;border-radius:4px;background:#eef3f6}.caption{color:var(--muted);font-size:.9rem}.math-display{overflow-x:auto;margin:12px 0;padding:2px 0;text-align:center}.wiki-note .math-display{margin:10px 0;padding:0}.wiki-note,blockquote{margin:18px 0;padding:14px 16px;border-left:4px solid var(--brand);background:var(--brand-soft);color:#22413d}.wiki-note strong{display:block;margin-bottom:6px}.doc-list{padding-left:22px}.tikz-source{margin:18px 0;border:1px solid var(--line);border-radius:var(--radius);background:#fbfdfc}.tikz-source>summary{cursor:pointer;padding:10px 14px;color:var(--brand);font-weight:700}.tikz-source .code-block{margin:0;border:0;border-top:1px solid #203c36;border-radius:0 0 var(--radius) var(--radius)}.code-block{overflow:hidden;margin:18px 0;border:1px solid #203c36;border-radius:var(--radius);background:var(--code)}.code-title{display:flex;justify-content:space-between;gap:12px;align-items:center;padding:8px 13px;border-bottom:1px solid rgba(255,255,255,.08);color:#c8eee6;font:0.78rem/1.4 var(--mono)}.code-title span{overflow-wrap:anywhere}.code-title em{font-style:normal;color:#8ccfc3}.doc-section pre{overflow:auto;max-height:78vh;margin:0;padding:18px;background:transparent;color:#effaf6;font:0.86rem/1.7 var(--mono)}.wiki-hero-grid,.category-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:14px}.wiki-hero-grid a,.category-card{padding:20px;border:1px solid var(--line);border-radius:var(--radius);background:#fff;text-decoration:none;color:var(--text)}.wiki-hero-grid strong,.category-card strong{display:block;font-family:var(--serif);font-size:1.35rem;font-weight:500}.wiki-hero-grid span,.category-card span{display:block;color:var(--muted)}.category-card a{display:block;margin-top:12px;padding-top:12px;border-top:1px solid var(--line);text-decoration:none;color:var(--text)}.fold-list{display:grid;gap:12px}.fold-panel{border:1px solid var(--line);border-radius:var(--radius);background:#fff}.fold-panel summary{display:flex;justify-content:space-between;gap:16px;align-items:center;cursor:pointer;padding:15px 18px;color:var(--text)}.fold-panel summary strong{font-family:var(--serif);font-size:1.2rem;font-weight:500}.fold-panel summary span{color:var(--muted);font-size:.9rem}.fold-panel[open] summary{border-bottom:1px solid var(--line);background:#fbfdfc}.file-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(240px,1fr));gap:12px;margin:18px}.file-card{display:grid;gap:6px;min-height:116px;padding:16px;border:1px solid var(--line);border-radius:var(--radius);background:#fff;color:var(--text);text-decoration:none}.file-card:hover{border-color:rgba(18,130,118,.45);box-shadow:0 12px 35px rgba(30,45,70,.08)}.file-card strong{overflow-wrap:anywhere}.file-card small{color:var(--muted);font-family:var(--mono);font-size:.74rem;overflow-wrap:anywhere}.file-kind{width:max-content;padding:2px 8px;border-radius:999px;background:var(--brand-soft);color:var(--brand);font-family:var(--mono);font-size:.72rem}.material-tools{display:grid;gap:10px;margin:4px 0 18px}.file-actions{display:flex;flex-wrap:wrap;gap:10px;margin-bottom:18px}.button-link{display:inline-block;padding:8px 12px;border:1px solid var(--line);border-radius:6px;background:#fff;color:var(--brand);text-decoration:none}.file-info{margin-top:28px;border:1px solid var(--line);border-radius:var(--radius);background:#fbfdfc}.file-info summary{cursor:pointer;padding:11px 14px;color:var(--brand);font-weight:700}.file-info-body{display:flex;flex-wrap:wrap;gap:10px 14px;align-items:center;padding:0 14px 14px;color:var(--muted);font-size:.88rem}.file-info-body span{overflow-wrap:anywhere}.asset-figure{margin:18px 0;padding:14px;border:1px solid var(--line);border-radius:var(--radius);background:#fff}.asset-figure img{display:block;max-width:100%;height:auto;margin:auto}.asset-figure.large img{max-height:78vh}.asset-figure figcaption{margin-top:8px;color:var(--muted);font-family:var(--mono);font-size:.75rem;overflow-wrap:anywhere}.pdf-frame{height:72vh;border:1px solid var(--line);border-radius:var(--radius);overflow:hidden;background:#fff}.pdf-frame iframe{width:100%;height:100%;border:0}.wiki-toc{position:sticky;top:82px;align-self:start;max-height:calc(100vh - 100px);overflow:auto;color:var(--muted);font-size:.88rem}.wiki-toc p{margin:0 0 10px;color:var(--text);font-weight:700}.wiki-toc a{display:block;padding:5px 0;color:var(--muted);text-decoration:none}.wiki-toc a:hover{color:var(--brand)}.prev-next{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-top:18px}.prev-next a,.prev-next span{padding:16px;border:1px solid var(--line);border-radius:var(--radius);background:#fff;color:var(--brand);text-decoration:none}mjx-container[display="true"]{display:block;max-width:100%;overflow-x:auto;overflow-y:hidden}mjx-container[display="true"] svg{max-width:100%;height:auto}@media(max-width:1100px){.wiki-shell{grid-template-columns:240px minmax(0,1fr)}.wiki-toc{display:none}}@media(max-width:760px){.wiki-topbar{grid-template-columns:auto 1fr auto;height:auto;min-height:58px;padding:10px 14px}.menu-toggle{display:inline-block}.wiki-search{grid-column:1/-1;grid-template-columns:1fr}.wiki-search span{display:none}.top-link{display:none}.wiki-shell{display:block;padding:14px}.wiki-sidebar{position:fixed;top:0;bottom:0;left:0;z-index:60;width:min(86vw,320px);max-height:none;transform:translateX(-105%);transition:transform .2s ease;border-radius:0}.wiki-sidebar.open{transform:translateX(0)}.doc-header{padding:30px 22px 22px}.doc-section{padding:24px 22px}.wiki-hero-grid,.category-grid,.prev-next{grid-template-columns:1fr}.doc-header h1{font-size:2.35rem}.doc-section p{overflow-x:auto}.file-grid{grid-template-columns:1fr;margin:14px}.fold-panel summary{align-items:flex-start;flex-direction:column;gap:2px}}'''
    css += ".tikz-figure{margin:20px 0;text-align:center}.tikz-canvas{display:flex;justify-content:center;align-items:center;min-height:70px;overflow-x:auto;padding:10px 0}.tikz-canvas>div{margin:0 auto}.tikz-canvas svg{max-width:100%;height:auto}.tikz-figure figcaption{color:var(--muted);font-size:.82rem}.tikz-fallback{display:inline-block;max-width:680px;padding:14px 16px;border:1px dashed var(--line);border-radius:var(--radius);background:#fbfdfc;color:var(--muted);font-size:.9rem}.table-wrap{overflow-x:auto;margin:16px 0}.latex-table{width:max-content;min-width:60%;margin:0 auto;border-collapse:collapse;background:#fff}.latex-table th,.latex-table td{border:1px solid var(--line);padding:8px 12px;text-align:center;vertical-align:middle}.latex-table th{background:var(--brand-soft);color:var(--brand);font-weight:700}.algorithm-list{margin:14px 0;padding-left:28px}.algorithm-list li{margin:5px 0}.topic-summary{padding:0 18px 16px;color:var(--muted)}.topic-summary ul{columns:2;gap:28px;margin:12px 0 0;padding-left:20px}.topic-summary li{break-inside:avoid;margin:4px 0}@media(max-width:760px){.topic-summary ul{columns:1}.latex-table{min-width:100%}}"
    css += r''':root{--mono:"JetBrains Mono","Maple Mono","Fira Code","Cascadia Code","SFMono-Regular","Menlo","Consolas",monospace;--code:#f8fafc}.code-fold-list{gap:16px}.code-topic .topic-summary{padding:18px;background:#fbfcfe;color:var(--text)}.code-topic .topic-summary ul{columns:1}.topic-note{margin:0 0 14px;color:var(--muted);font-size:.94rem}.content-entry{margin:18px 0;padding:16px 18px;border:1px solid #dbe4ee;border-radius:var(--radius);background:#fff}.content-entry h3{margin:0 0 12px;font-family:var(--mono);font-size:.92rem;line-height:1.45;color:#334155;overflow-wrap:anywhere}.content-entry .code-block{margin-top:12px}.media-entry .asset-figure{margin:10px 0 0;border-color:#e0e7ef;background:#f8fafc}.code-block{overflow:hidden;margin:20px 0;border:1px solid #d4dee9;border-radius:8px;background:#f8fafc;box-shadow:0 10px 28px rgba(15,23,42,.06)}.code-title{display:flex;justify-content:space-between;gap:14px;align-items:center;padding:9px 12px;border-bottom:1px solid #dbe4ee;background:#fff;color:#334155;font:600 .8rem/1.35 var(--mono)}.code-title span{overflow-wrap:anywhere}.code-tools{display:flex;gap:8px;align-items:center;flex:0 0 auto}.code-title em{padding:2px 7px;border:1px solid #cae2de;border-radius:999px;background:#eefaf7;color:#0f766e;font-style:normal;font-weight:700}.copy-code{height:26px;padding:0 9px;border:1px solid #cbd5e1;border-radius:6px;background:#fff;color:#475569;font:600 .76rem/1 var(--sans);cursor:pointer}.copy-code:hover{border-color:#128276;color:#128276;background:#f0fdfa}.copy-code.copied{border-color:#128276;background:#e0f4f0;color:#0f766e}.doc-section pre{overflow:auto;max-height:68vh;margin:0;padding:16px 18px;background:#f8fafc;color:#1f2937;font:0.9rem/1.7 var(--mono);font-feature-settings:"liga" 1,"calt" 1;tab-size:2}.doc-section pre code,.doc-section pre code.hljs{display:block;min-width:max-content;padding:0;background:transparent;color:inherit;font:inherit}.hljs-comment,.hljs-quote{color:#64748b;font-style:italic}.hljs-keyword,.hljs-selector-tag,.hljs-subst{color:#7c3aed}.hljs-built_in,.hljs-title.function_{color:#0369a1}.hljs-string,.hljs-attr{color:#047857}.hljs-number,.hljs-literal{color:#b45309}.hljs-title,.hljs-section{color:#be123c}.hljs-meta{color:#475569}@media(max-width:760px){.code-topic .topic-summary{padding:14px}.content-entry{padding:14px}.code-title{align-items:flex-start;flex-direction:column}.code-tools{width:100%;justify-content:space-between}.doc-section pre{max-height:62vh;font-size:.84rem}}'''
    css += r'''.code-block,.fold-panel,.topic-summary,.content-entry{min-width:0;max-width:100%}.code-title{min-width:0}.code-title span{min-width:0;max-width:100%;word-break:break-word}.doc-section pre{max-width:100%;min-width:0}.doc-section pre code,.doc-section pre code.hljs{min-width:100%;width:max-content;max-width:none}@media(max-width:760px){.code-block{width:100%}.code-title span{word-break:break-all}}'''
    write(ASSETS / "wiki.css", css + "\n")
    js = r'''const depth=Number(document.body.dataset.depth||0);const prefix='../'.repeat(depth);const sidebar=document.querySelector('#wiki-sidebar');document.querySelector('.menu-toggle')?.addEventListener('click',()=>sidebar.classList.toggle('open'));document.addEventListener('click',event=>{if(innerWidth<760&&sidebar?.classList.contains('open')&&!sidebar.contains(event.target)&&!event.target.closest('.menu-toggle'))sidebar.classList.remove('open')});const toc=document.querySelector('#page-toc');[...document.querySelectorAll('.doc-section h2, .doc-section h3')].filter(heading=>!heading.closest('.content-entry')).forEach((heading,index)=>{if(!heading.id)heading.id=`section-${index+1}`;const a=document.createElement('a');a.href=`#${heading.id}`;a.textContent=heading.textContent;if(heading.tagName==='H3')a.style.paddingLeft='12px';toc?.appendChild(a)});const input=document.querySelector('#wiki-search');const box=document.querySelector('#search-results');let searchIndex=window.WIKI_SEARCH_INDEX||[];if(!searchIndex.length)fetch(`${prefix}assets/search-index.json`).then(r=>r.json()).then(data=>{searchIndex=data}).catch(()=>{});input?.addEventListener('input',()=>{const q=input.value.trim().toLowerCase();if(!q){box.hidden=true;box.innerHTML='';return}const hits=searchIndex.filter(item=>`${item.title} ${item.category} ${item.text}`.toLowerCase().includes(q)).slice(0,12);box.innerHTML=hits.length?hits.map(item=>`<a href="${prefix}${item.url}"><small>${item.category}</small>${item.title}</a>`).join(''):'<a>没有找到匹配内容</a>';box.hidden=false});input?.addEventListener('blur',()=>setTimeout(()=>{box.hidden=true},180));const materialFilter=document.querySelector('#materialFilter');materialFilter?.addEventListener('input',()=>{const q=materialFilter.value.trim().toLowerCase();document.querySelectorAll('.file-card').forEach(card=>{card.hidden=!!q&&!card.textContent.toLowerCase().includes(q)&&!(card.dataset.kind||'').toLowerCase().includes(q)&&!(card.dataset.group||'').toLowerCase().includes(q)});document.querySelectorAll('.fold-panel').forEach(panel=>{const cards=[...panel.querySelectorAll('.file-card')];const hasVisible=cards.some(card=>!card.hidden);panel.hidden=q&&!hasVisible;if(q&&hasVisible)panel.open=true})});window.addEventListener('load',()=>setTimeout(()=>document.querySelectorAll('mjx-assistive-mml').forEach(node=>node.remove()),600));'''
    js += r'''if(toc&&!toc.children.length){[...document.querySelectorAll('.code-topic>summary strong')].forEach((heading,index)=>{const panel=heading.closest('.code-topic');if(!panel.id)panel.id=`module-${index+1}`;const a=document.createElement('a');a.href=`#${panel.id}`;a.textContent=heading.textContent;toc.appendChild(a)})}window.addEventListener('load',()=>{window.hljs?.highlightAll()});function fallbackCopy(text,codeNode){const area=document.createElement('textarea');area.value=text;area.setAttribute('readonly','');area.style.position='fixed';area.style.left='-9999px';document.body.appendChild(area);area.select();let ok=false;try{ok=document.execCommand('copy')}catch{}area.remove();if(!ok&&codeNode){const range=document.createRange();range.selectNodeContents(codeNode);const selection=window.getSelection();selection.removeAllRanges();selection.addRange(range);return 'selected'}return ok?'copied':'failed'}document.addEventListener('click',async event=>{const button=event.target.closest('.copy-code');if(!button)return;const codeNode=button.closest('.code-block')?.querySelector('code');const code=codeNode?.textContent||'';let state='failed';try{if(navigator.clipboard&&window.isSecureContext){await navigator.clipboard.writeText(code);state='copied'}}catch{}if(state==='failed')state=fallbackCopy(code,codeNode);button.textContent=state==='copied'?'已复制':state==='selected'?'已选中':'复制失败';button.classList.toggle('copied',state!=='failed');setTimeout(()=>{button.textContent='复制';button.classList.remove('copied')},1200)});'''
    js += r'''function codeEscape(text){return text.replace(/[&<>]/g,char=>({'&':'&amp;','<':'&lt;','>':'&gt;'}[char]))}function simpleHighlight(code){const lang=[...code.classList].find(name=>name.startsWith('language-'))?.replace('language-','')||'';if(lang==='csv'||lang==='plaintext')return;const keywords={python:'and as assert break class continue def del elif else except False finally for from global if import in is lambda None nonlocal not or pass raise return True try while with yield',cpp:'alignas alignof auto bool break case catch char class const constexpr continue default delete do double else enum explicit extern false float for friend if inline int long namespace new noexcept nullptr operator private protected public return short signed sizeof static struct switch template this throw true try typedef typename union unsigned using virtual void volatile while',cmake:'add_executable add_library cmake_minimum_required endif find_package foreach function if include link_libraries message project return set target_link_libraries target_include_directories',shell:'cd echo export for if in then else fi do done git python pip conda'};const set=new Set((keywords[lang]||'').split(' ').filter(Boolean));const raw=code.textContent;const token=/\/\*[\s\S]*?\*\/|\/\/[^\n]*|#[^\n]*|"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'|\\[A-Za-z]+\*?|\b\d+(?:\.\d+)?\b|\b[A-Za-z_]\w*\b/g;let html='',last=0;raw.replace(token,(match,offset)=>{html+=codeEscape(raw.slice(last,offset));let cls='';if(/^\/\*/.test(match)||/^\/\//.test(match))cls='hljs-comment';else if(/^#/.test(match))cls=lang==='cpp'?'hljs-meta':'hljs-comment';else if(/^["']/.test(match))cls='hljs-string';else if(/^\\[A-Za-z]/.test(match))cls='hljs-keyword';else if(/^\d/.test(match))cls='hljs-number';else if(set.has(match))cls='hljs-keyword';html+=cls?`<span class="${cls}">${codeEscape(match)}</span>`:codeEscape(match);last=offset+match.length});html+=codeEscape(raw.slice(last));code.innerHTML=html}window.addEventListener('load',()=>{if(window.hljs)return;document.querySelectorAll('pre code[class*="language-"]').forEach(simpleHighlight)});'''
    write(ASSETS / "wiki.js", js + "\n")

    print(json.dumps({"records": len(records), "math_pages": len(math_pages), "search_items": len(search_items)}, ensure_ascii=False))


if __name__ == "__main__":
    main()
