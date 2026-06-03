#!/usr/bin/env python3
import argparse
import html
import json
import os
import re
import shutil
import subprocess
import zipfile
from datetime import datetime, timezone
from pathlib import Path
from urllib.parse import quote


def slugify(value):
    value = value.strip().replace("/", "__")
    value = re.sub(r"[^A-Za-z0-9._-]+", "-", value)
    return value.strip("-") or "unknown"


def read_json(path, default):
    try:
        with path.open("r", encoding="utf-8") as file:
            return json.load(file)
    except FileNotFoundError:
        return default


def write_json(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8") as file:
        json.dump(data, file, indent=2, sort_keys=True)
        file.write("\n")


def copy_artifacts(artifact_paths, build_dir):
    artifact_dir = build_dir / "artifacts"
    artifacts = []
    for artifact_path in artifact_paths:
        source = Path(artifact_path)
        if not source.exists():
            continue
        artifact_dir.mkdir(parents=True, exist_ok=True)
        destination = artifact_dir / source.name
        shutil.copy2(source, destination)
        artifacts.append(
            {
                "name": source.name,
                "path": destination.relative_to(build_dir.parents[3]).as_posix(),
                "size": destination.stat().st_size,
            }
        )
    return artifacts


def copy_log(log_path, build_dir):
    if not log_path:
        return None

    source = Path(log_path)
    if not source.exists():
        return None

    destination = build_dir / "console.log"
    shutil.copy2(source, destination)
    return {
        "name": destination.name,
        "path": destination.relative_to(build_dir.parents[3]).as_posix(),
        "size": destination.stat().st_size,
    }


def copy_tree_contents(source, destination, skip_zip=False):
    source = Path(source)
    if not source.exists():
        return
    destination.mkdir(parents=True, exist_ok=True)
    for item in source.iterdir():
        if skip_zip and item.is_file() and item.suffix.lower() == ".zip":
            continue
        target = destination / item.name
        if item.is_dir():
            if target.exists():
                shutil.rmtree(target)
            shutil.copytree(item, target)
        else:
            shutil.copy2(item, target)


def link_or_copy(source, destination):
    source = Path(source)
    destination = Path(destination)
    if destination.exists() or destination.is_symlink():
        destination.unlink()
    try:
        relative_source = os.path.relpath(source, destination.parent)
        destination.symlink_to(relative_source)
    except OSError:
        shutil.copy2(source, destination)


def find_demo_program(zip_path):
    with zipfile.ZipFile(zip_path) as archive:
        names = [name for name in archive.namelist() if not name.endswith("/")]

    programs = [name for name in names if name.lower().endswith((".bat", ".exe"))]
    if not programs:
        return ""

    bat_files = [name for name in programs if name.lower().endswith(".bat")]
    if bat_files:
        return sorted(bat_files, key=lambda name: (name.count("/"), len(name), name.lower()))[0]

    stem = zip_path.stem.lower()
    exact_stem_matches = [
        name for name in programs
        if Path(name).stem.lower() == stem
    ]
    if exact_stem_matches:
        return sorted(exact_stem_matches, key=lambda name: (name.count("/"), len(name), name.lower()))[0]

    top_level = [name for name in programs if "/" not in name]
    if top_level:
        return sorted(top_level, key=lambda name: (len(name), name.lower()))[0]

    return sorted(programs, key=lambda name: (name.count("/"), len(name), name.lower()))[0]


def is_overlay_demo(zip_path):
    with zipfile.ZipFile(zip_path) as archive:
        return any(name.startswith("home/username/.wine/drive_c/") for name in archive.namelist())


def normalize_demo_program(program):
    prefix = "home/username/.wine/drive_c/"
    if program.startswith(prefix):
        return "c:/" + program[len(prefix):]
    return program


def title_from_zip(zip_path):
    name = zip_path.stem
    name = re.sub(r"[-_]+", " ", name)
    name = re.sub(r"(?<=[a-z])(?=[A-Z0-9])", " ", name)
    return name.title()


def load_demo_manifest(demo_source):
    manifest = read_json(Path(demo_source) / "demos.json", {})
    if isinstance(manifest, list):
        return {
            item.get("zip", ""): item
            for item in manifest
            if isinstance(item, dict) and item.get("zip")
        }
    if isinstance(manifest, dict):
        demos = manifest.get("demos", manifest)
        if isinstance(demos, list):
            return {
                item.get("zip", ""): item
                for item in demos
                if isinstance(item, dict) and item.get("zip")
            }
        if isinstance(demos, dict):
            return {
                key: value
                for key, value in demos.items()
                if isinstance(value, dict)
            }
    return {}


def get_remote_branch_slugs():
    try:
        output = subprocess.check_output(
            ["git", "ls-remote", "--heads", "origin"],
            stderr=subprocess.DEVNULL,
            text=True,
            timeout=30,
        )
    except Exception:
        return set()

    slugs = set()
    for line in output.splitlines():
        if "refs/heads/" not in line:
            continue
        branch = line.split("refs/heads/", 1)[1].strip()
        if branch:
            slugs.add(slugify(branch))
    return slugs


def load_builds(site_dir):
    branches_dir = site_dir / "branches"
    branches = []
    if not branches_dir.exists():
        return branches

    for branch_dir in sorted(branches_dir.iterdir()):
        if not branch_dir.is_dir():
            continue
        branch_meta = read_json(branch_dir / "branch.json", {})
        builds = []
        for build_json in branch_dir.glob("builds/*/build.json"):
            builds.append(read_json(build_json, {}))
        builds.sort(key=lambda build: int(build.get("buildNumber", 0)), reverse=True)
        branches.append(
            {
                "name": branch_meta.get("name", branch_dir.name),
                "slug": branch_dir.name,
                "builds": builds,
            }
        )
    branches.sort(key=lambda branch: branch["name"].lower())
    return branches


def prune_old_builds(branch_dir, keep):
    build_dirs = [path for path in (branch_dir / "builds").glob("*") if path.is_dir()]
    build_dirs.sort(key=lambda path: int(path.name) if path.name.isdigit() else -1, reverse=True)
    for old_build_dir in build_dirs[keep:]:
        shutil.rmtree(old_build_dir)


def prune_removed_demo_branches(site_dir):
    active_branch_slugs = get_remote_branch_slugs()
    if not active_branch_slugs:
        return

    branches_dir = site_dir / "branches"
    if branches_dir.exists():
        for branch_dir in branches_dir.iterdir():
            if branch_dir.is_dir() and branch_dir.name not in active_branch_slugs:
                shutil.rmtree(branch_dir)

    demo_build_dir = site_dir / "demos" / "build"
    if demo_build_dir.exists():
        for branch_dir in demo_build_dir.iterdir():
            if branch_dir.is_dir() and branch_dir.name not in active_branch_slugs:
                shutil.rmtree(branch_dir)


def prune_old_demo_builds(site_dir, branch_slug, keep):
    branch_dir = site_dir / "demos" / "build" / branch_slug
    if not branch_dir.exists():
        return
    build_dirs = [path for path in branch_dir.iterdir() if path.is_dir()]
    build_dirs.sort(key=lambda path: int(path.name) if path.name.isdigit() else -1, reverse=True)
    for old_build_dir in build_dirs[keep:]:
        shutil.rmtree(old_build_dir)


def format_size(size):
    units = ["B", "KB", "MB", "GB"]
    value = float(size)
    for unit in units:
        if value < 1024 or unit == units[-1]:
            if unit == "B":
                return f"{int(value)} {unit}"
            return f"{value:.1f} {unit}"
        value /= 1024
    return f"{size} B"


def render_html(site_dir, branches, title):
    generated_at = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    safe_branches = json.dumps(branches, separators=(",", ":")).replace("</", "<\\/")
    branch_count = len(branches)
    build_count = sum(len(branch["builds"]) for branch in branches)

    html_page = f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>{html.escape(title)}</title>
  <style>
    :root {{
      color-scheme: light;
      --bg: #f3f5f7;
      --panel: #ffffff;
      --panel-soft: #f8fafc;
      --text: #17202a;
      --muted: #647184;
      --line: #d9e0e8;
      --success: #117a37;
      --failure: #bd2626;
      --running: #936a00;
      --unknown: #5d6573;
      --link: #155cc1;
      --selected: #e8f1ff;
      --selected-line: #86aee8;
    }}
    * {{ box-sizing: border-box; }}
    body {{
      margin: 0;
      background: var(--bg);
      color: var(--text);
      font: 15px/1.45 system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
    }}
    header {{
      background: #202a35;
      color: #fff;
      padding: 24px 28px;
      border-bottom: 4px solid #4ba36f;
    }}
    h1 {{
      margin: 0;
      font-size: 28px;
      font-weight: 700;
      letter-spacing: 0;
    }}
    header p {{
      margin: 6px 0 0;
      color: #c9d3df;
    }}
    .layout {{
      display: grid;
      grid-template-columns: minmax(230px, 320px) minmax(0, 1fr);
      min-height: calc(100vh - 92px);
    }}
    .branch-index {{
      border-right: 1px solid var(--line);
      background: var(--panel);
      min-height: 100%;
    }}
    .index-header {{
      position: sticky;
      top: 0;
      z-index: 1;
      padding: 18px 18px 12px;
      background: var(--panel);
      border-bottom: 1px solid var(--line);
    }}
    .index-title {{
      margin: 0;
      font-size: 16px;
      font-weight: 700;
    }}
    .index-meta {{
      margin: 4px 0 0;
      color: var(--muted);
      font-size: 13px;
    }}
    .branch-list {{
      display: flex;
      flex-direction: column;
      padding: 8px;
    }}
    .branch-button {{
      display: grid;
      grid-template-columns: minmax(0, 1fr) auto;
      gap: 10px;
      width: 100%;
      border: 1px solid transparent;
      border-radius: 6px;
      background: transparent;
      color: var(--text);
      cursor: pointer;
      font: inherit;
      padding: 10px 9px;
      text-align: left;
    }}
    .branch-button:hover {{
      background: var(--panel-soft);
      border-color: var(--line);
    }}
    .branch-button.selected {{
      background: var(--selected);
      border-color: var(--selected-line);
    }}
    .branch-name {{
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
      font-weight: 650;
    }}
    .branch-count {{
      color: var(--muted);
      font-size: 13px;
    }}
    main {{
      min-width: 0;
      padding: 24px 28px;
    }}
    .branch-summary {{
      display: flex;
      align-items: flex-start;
      justify-content: space-between;
      gap: 16px;
      margin-bottom: 18px;
    }}
    h2 {{
      margin: 0;
      font-size: 24px;
      letter-spacing: 0;
      overflow-wrap: anywhere;
    }}
    .summary-meta {{
      margin: 5px 0 0;
      color: var(--muted);
    }}
    .status {{
      display: inline-flex;
      align-items: center;
      justify-content: center;
      min-width: 88px;
      border-radius: 999px;
      padding: 5px 10px;
      color: #fff;
      font-size: 12px;
      font-weight: 700;
      text-transform: uppercase;
      white-space: nowrap;
    }}
    .success {{ background: var(--success); }}
    .failure {{ background: var(--failure); }}
    .running {{ background: var(--running); }}
    .unknown {{ background: var(--unknown); }}
    .build-list {{
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 8px;
      overflow: hidden;
      box-shadow: 0 1px 2px rgba(23, 32, 42, 0.05);
    }}
    .build {{
      display: grid;
      grid-template-columns: 120px minmax(0, 1fr) minmax(220px, auto);
      gap: 16px;
      padding: 15px 18px;
      border-bottom: 1px solid var(--line);
      align-items: start;
    }}
    .build:last-child {{ border-bottom: 0; }}
    .build-number {{
      font-weight: 700;
    }}
    .meta {{
      color: var(--muted);
      font-size: 14px;
      overflow-wrap: anywhere;
    }}
    .commit {{
      font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
    }}
    .artifact-list {{
      display: flex;
      flex-wrap: wrap;
      justify-content: flex-end;
      gap: 8px;
      min-width: 0;
    }}
    a {{
      color: var(--link);
      text-decoration: none;
      font-weight: 600;
    }}
    a:hover {{ text-decoration: underline; }}
    .artifact {{
      border: 1px solid var(--line);
      border-radius: 6px;
      padding: 6px 8px;
      background: #fff;
      white-space: nowrap;
    }}
    .empty {{
      padding: 24px;
      color: var(--muted);
    }}
    @media (max-width: 820px) {{
      header {{ padding: 20px 16px; }}
      .layout {{
        grid-template-columns: 1fr;
      }}
      .branch-index {{
        border-right: 0;
        border-bottom: 1px solid var(--line);
      }}
      .index-header {{
        position: static;
      }}
      .branch-list {{
        flex-direction: row;
        overflow-x: auto;
        padding: 8px 8px 10px;
      }}
      .branch-button {{
        min-width: 190px;
      }}
      main {{
        padding: 18px 16px;
      }}
      .branch-summary {{
        flex-direction: column;
      }}
      .build {{
        grid-template-columns: 1fr;
        gap: 8px;
      }}
      .artifact-list {{
        justify-content: flex-start;
      }}
    }}
  </style>
</head>
<body>
  <header>
    <h1>{html.escape(title)}</h1>
    <p>Last updated {html.escape(generated_at)}</p>
  </header>
  <div class="layout">
    <aside class="branch-index" aria-label="Branch index">
      <div class="index-header">
        <p class="index-title">Branches</p>
        <p class="index-meta">{branch_count} branches, {build_count} retained builds</p>
      </div>
      <nav id="branch-list" class="branch-list"></nav>
    </aside>
    <main id="build-pane">
      <p class="empty">No builds have been published yet.</p>
    </main>
  </div>
  <script>
    const branches = {safe_branches};

    function escapeHtml(value) {{
      return String(value ?? "").replace(/[&<>"']/g, (character) => ({{
        "&": "&amp;",
        "<": "&lt;",
        ">": "&gt;",
        '"': "&quot;",
        "'": "&#39;"
      }}[character]));
    }}

    function statusClass(status) {{
      const normalized = String(status || "UNKNOWN").toLowerCase();
      if (["failure", "failed", "unstable", "aborted"].includes(normalized)) return "failure";
      if (["running", "building"].includes(normalized)) return "running";
      if (normalized === "success") return "success";
      return "unknown";
    }}

    function formatSize(size) {{
      const units = ["B", "KB", "MB", "GB"];
      let value = Number(size || 0);
      for (const unit of units) {{
        if (value < 1024 || unit === units[units.length - 1]) {{
          return unit === "B" ? `${{Math.round(value)}} ${{unit}}` : `${{value.toFixed(1)}} ${{unit}}`;
        }}
        value /= 1024;
      }}
      return `${{size}} B`;
    }}

    function buildUrl(url, label, className = "") {{
      if (!url) return escapeHtml(label);
      return `<a${{className ? ` class="${{className}}"` : ""}} href="${{escapeHtml(url)}}">${{escapeHtml(label)}}</a>`;
    }}

    function renderArtifacts(build) {{
      const artifacts = build.artifacts || [];
      const links = artifacts.map((artifact) => (
        `<a class="artifact" href="${{escapeHtml(artifact.path)}}">` +
        `${{escapeHtml(artifact.name)}} <span class="meta">${{escapeHtml(formatSize(artifact.size))}}</span>` +
        "</a>"
      ));
      if (build.log && statusClass(build.result) === "failure") {{
        links.push(
          `<a class="artifact" href="${{escapeHtml(build.log.path)}}">` +
          `Error Log <span class="meta">${{escapeHtml(formatSize(build.log.size))}}</span>` +
          "</a>"
        );
      }} else if (build.branchSlug && build.buildNumber) {{
        links.push(
          `<a class="artifact" href="demos/build/${{escapeHtml(build.branchSlug)}}/${{escapeHtml(build.buildNumber)}}/">Demos</a>`
        );
      }}
      return links.length ? links.join("") : '<span class="meta">No artifact</span>';
    }}

    function renderBranch(branch) {{
      const builds = branch.builds || [];
      const latest = builds[0] || {{}};
      const latestStatus = latest.result || "UNKNOWN";
      const latestNumber = latest.buildNumber ?? "n/a";
      const visibleBuilds = builds.slice(0, 5);
      const buildRows = visibleBuilds.length ? visibleBuilds.map((build) => {{
        const shortCommit = build.commit ? String(build.commit).slice(0, 12) : "unknown";
        return `
          <article class="build">
            <div class="build-number">${{buildUrl(build.buildUrl, `#${{build.buildNumber ?? "n/a"}}`)}}</div>
            <div>
              <div>${{escapeHtml(build.result || "UNKNOWN")}} on ${{escapeHtml(build.builtAt || "unknown")}}</div>
              <div class="meta">Commit ${{buildUrl(build.commitUrl, shortCommit, "commit")}}</div>
            </div>
            <div class="artifact-list">${{renderArtifacts(build)}}</div>
          </article>
        `;
      }}).join("") : '<p class="empty">No builds for this branch.</p>';

      document.getElementById("build-pane").innerHTML = `
        <section class="branch-summary">
          <div>
            <h2>${{escapeHtml(branch.name)}}</h2>
            <p class="summary-meta">Latest build #${{escapeHtml(latestNumber)}} on ${{escapeHtml(latest.builtAt || "unknown")}}. Showing the last ${{visibleBuilds.length}} builds.</p>
          </div>
          <span class="status ${{statusClass(latestStatus)}}">${{escapeHtml(latestStatus)}}</span>
        </section>
        <section class="build-list">${{buildRows}}</section>
      `;

      document.querySelectorAll(".branch-button").forEach((button) => {{
        button.classList.toggle("selected", button.dataset.slug === branch.slug);
      }});
      if (location.hash !== `#${{branch.slug}}`) {{
        history.replaceState(null, "", `#${{branch.slug}}`);
      }}
    }}

    function renderBranchIndex() {{
      const list = document.getElementById("branch-list");
      list.innerHTML = branches.map((branch, index) => `
        <button class="branch-button" type="button" data-slug="${{escapeHtml(branch.slug)}}" data-index="${{index}}">
          <span class="branch-name" title="${{escapeHtml(branch.name)}}">${{escapeHtml(branch.name)}}</span>
          <span class="branch-count">${{(branch.builds || []).length}}</span>
        </button>
      `).join("");
      list.addEventListener("click", (event) => {{
        const button = event.target.closest(".branch-button");
        if (!button) return;
        renderBranch(branches[Number(button.dataset.index)]);
      }});
    }}

    renderBranchIndex();
    if (branches.length) {{
      const hashSlug = decodeURIComponent(location.hash.replace(/^#/, ""));
      renderBranch(branches.find((branch) => branch.slug === hashSlug) || branches[0]);
    }}
  </script>
</body>
</html>
"""
    (site_dir / "index.html").write_text(html_page, encoding="utf-8")


def render_demos_html(site_dir, branch, branch_slug, build_number, demos):
    demos_dir = site_dir / "demos"
    generated_at = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    branch_html = html.escape(branch)
    build_html = html.escape(str(build_number))
    content = render_demo_cards(
        demos,
        lambda demo, mode: demo_launch_url(branch_slug, build_number, mode, demo),
        "images",
    )
    page = render_demo_page(
        "Boxedwine Demos",
        f"Branch {branch_html}, build {build_html}. Last updated {html.escape(generated_at)}",
        "Each demo can run against the current single threaded or multi threaded web build.",
        "../",
        content,
    )
    demos_dir.mkdir(parents=True, exist_ok=True)
    (demos_dir / "index.html").write_text(page, encoding="utf-8")


def render_build_demo_html(build_dir, branch, build_number, demos):
    generated_at = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    branch_html = html.escape(branch)
    build_html = html.escape(str(build_number))
    content = render_demo_cards(
        demos,
        lambda demo, mode: build_demo_launch_url(mode, demo),
        "images",
    )
    page = render_demo_page(
        "Boxedwine Demos",
        f"Branch {branch_html}, build {build_html}. Last updated {html.escape(generated_at)}",
        "Each demo can run against this build's single threaded or multi threaded web runner.",
        "../../../../",
        content,
    )
    (build_dir / "index.html").write_text(page, encoding="utf-8")


def render_demo_cards(demos, url_builder, image_prefix):
    cards = []
    for demo in demos:
        image = html.escape(f"{image_prefix}/{demo['stem']}.png")
        title = html.escape(demo["title"])
        zip_name = html.escape(demo["zip"])
        program = html.escape(demo["program"] or "Select executable")
        description = html.escape(demo.get("description", ""))
        st_url = html.escape(url_builder(demo, "st"))
        mt_url = html.escape(url_builder(demo, "mt"))
        cards.append(f"""
        <article class="demo-card">
          <div class="screenshot">
            <img src="{image}" alt="{title} screenshot" loading="lazy">
          </div>
          <div class="demo-body">
            <h2>{title}</h2>
            <p>{zip_name}</p>
            <p class="program">{program}</p>
            {f'<p class="description">{description}</p>' if description else ''}
            <div class="actions">
              <a href="{st_url}">Single Threaded</a>
              <a href="{mt_url}">Multi Threaded</a>
            </div>
          </div>
        </article>
        """)

    return "\n".join(cards) if cards else "<p class=\"empty\">No demos are available.</p>"


def render_demo_page(title, subtitle, intro, back_href, content):
    return f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>{html.escape(title)}</title>
  <style>
    :root {{
      --bg: #f4f6f8;
      --panel: #ffffff;
      --text: #17202a;
      --muted: #667487;
      --line: #d9e0e8;
      --link: #155cc1;
      --button: #1f6f43;
      --button-alt: #244f82;
    }}
    * {{ box-sizing: border-box; }}
    body {{
      margin: 0;
      background: var(--bg);
      color: var(--text);
      font: 15px/1.45 system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
    }}
    header {{
      background: #202a35;
      color: #fff;
      border-bottom: 4px solid #4ba36f;
      padding: 26px 22px;
    }}
    header .inner, main {{
      max-width: 1180px;
      margin: 0 auto;
    }}
    h1 {{
      margin: 0;
      font-size: 28px;
      letter-spacing: 0;
    }}
    header p {{
      margin: 6px 0 0;
      color: #c9d3df;
    }}
    main {{
      padding: 24px;
    }}
    .toolbar {{
      display: flex;
      justify-content: space-between;
      gap: 16px;
      align-items: center;
      margin-bottom: 18px;
      color: var(--muted);
    }}
    .toolbar a {{
      color: var(--link);
      font-weight: 700;
      text-decoration: none;
    }}
    .grid {{
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(260px, 1fr));
      gap: 18px;
    }}
    .demo-card {{
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 8px;
      overflow: hidden;
      box-shadow: 0 1px 2px rgba(23, 32, 42, 0.05);
      display: flex;
      flex-direction: column;
    }}
    .screenshot {{
      aspect-ratio: 16 / 10;
      background: #e8edf3;
      border-bottom: 1px solid var(--line);
    }}
    .screenshot img {{
      width: 100%;
      height: 100%;
      display: block;
      object-fit: cover;
    }}
    .demo-body {{
      padding: 14px;
      display: flex;
      flex: 1;
      flex-direction: column;
    }}
    h2 {{
      margin: 0;
      font-size: 18px;
      letter-spacing: 0;
    }}
    .demo-body p {{
      margin: 5px 0 0;
      color: var(--muted);
      overflow-wrap: anywhere;
    }}
    .program {{
      font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
      font-size: 13px;
    }}
    .description {{
      min-height: 42px;
    }}
    .actions {{
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 8px;
      margin-top: auto;
      padding-top: 14px;
    }}
    .actions a {{
      display: inline-flex;
      justify-content: center;
      align-items: center;
      min-height: 38px;
      border-radius: 6px;
      background: var(--button);
      color: #fff;
      font-weight: 700;
      text-decoration: none;
      padding: 8px 10px;
      text-align: center;
    }}
    .actions a + a {{
      background: var(--button-alt);
    }}
    .empty {{
      color: var(--muted);
    }}
    @media (max-width: 640px) {{
      header {{ padding: 22px 16px; }}
      main {{ padding: 16px; }}
      .toolbar {{
        align-items: flex-start;
        flex-direction: column;
      }}
    }}
  </style>
</head>
<body>
  <header>
    <div class="inner">
      <h1>{html.escape(title)}</h1>
      <p>{subtitle}</p>
    </div>
  </header>
  <main>
    <div class="toolbar">
      <span>{html.escape(intro)}</span>
      <a href="{html.escape(back_href)}">Build Status</a>
    </div>
    <div class="grid">
      {content}
    </div>
  </main>
</body>
</html>
"""


def demo_launch_url(branch_slug, build_number, mode, demo):
    params = {
        "root": "boxedwine.zip",
        demo.get("zipParam", "app"): demo["zip"],
    }
    if demo["program"]:
        params["p"] = demo["program"]
    query = "&".join(f"{key}={quote(value, safe='/:._-')}" for key, value in params.items())
    return f"build/{quote(branch_slug)}/{quote(str(build_number))}/{mode}/boxedwine.html?{query}"


def build_demo_launch_url(mode, demo):
    params = {
        "root": "boxedwine.zip",
        demo.get("zipParam", "app"): demo["zip"],
    }
    if demo["program"]:
        params["p"] = demo["program"]
    query = "&".join(f"{key}={quote(value, safe='/:._-')}" for key, value in params.items())
    return f"{mode}/boxedwine.html?{query}"


def update_demos(site_dir, branch, branch_slug, build_number, demo_source, single_threaded_dir, multi_threaded_dir, keep):
    demo_source = Path(demo_source)
    if not demo_source.exists():
        return

    demos_dir = site_dir / "demos"
    apps_dir = demos_dir / "apps"
    images_dir = demos_dir / "images"
    apps_dir.mkdir(parents=True, exist_ok=True)
    images_dir.mkdir(parents=True, exist_ok=True)

    same_apps_dir = demo_source.resolve() == apps_dir.resolve()
    if not same_apps_dir:
        for old_zip in apps_dir.glob("*.zip"):
            old_zip.unlink()

    manifest = load_demo_manifest(demo_source)
    demos = []
    for zip_path in sorted(demo_source.glob("*.zip"), key=lambda path: path.name.lower()):
        if not same_apps_dir:
            shutil.copy2(zip_path, apps_dir / zip_path.name)
        if zip_path.name.lower() == "boxedwine.zip":
            continue
        manifest_entry = manifest.get(zip_path.name, {})
        program = manifest_entry.get("exe") or manifest_entry.get("program") or find_demo_program(zip_path)
        demos.append(
            {
                "zip": zip_path.name,
                "stem": zip_path.stem,
                "title": manifest_entry.get("title") or title_from_zip(zip_path),
                "description": manifest_entry.get("description", ""),
                "program": normalize_demo_program(program),
                "zipParam": "overlay" if is_overlay_demo(zip_path) else "app",
            }
        )

    source_images_dir = demo_source / "images"
    if source_images_dir.exists():
        for image_path in source_images_dir.glob("*.png"):
            shutil.copy2(image_path, images_dir / image_path.name)

    build_dir = demos_dir / "build" / branch_slug / str(build_number)
    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)

    boxedwine_zip = apps_dir / "boxedwine.zip"
    if boxedwine_zip.exists():
        link_or_copy(boxedwine_zip, build_dir / "boxedwine.zip")

    copy_tree_contents(single_threaded_dir, build_dir / "st", skip_zip=True)
    copy_tree_contents(multi_threaded_dir, build_dir / "mt", skip_zip=True)
    build_images_dir = build_dir / "images"
    build_images_dir.mkdir(parents=True, exist_ok=True)
    for demo in demos:
        image_path = images_dir / f"{demo['stem']}.png"
        if image_path.exists():
            link_or_copy(image_path, build_images_dir / image_path.name)
    if boxedwine_zip.exists():
        link_or_copy(boxedwine_zip, build_dir / "st" / "boxedwine.zip")
        link_or_copy(boxedwine_zip, build_dir / "mt" / "boxedwine.zip")
    for demo in demos:
        app_zip = apps_dir / demo["zip"]
        if app_zip.exists():
            link_or_copy(app_zip, build_dir / "st" / demo["zip"])
            link_or_copy(app_zip, build_dir / "mt" / demo["zip"])

    render_build_demo_html(build_dir, branch, build_number, demos)
    prune_old_demo_builds(site_dir, branch_slug, keep)
    render_demos_html(site_dir, branch, branch_slug, build_number, demos)


def render_builds(builds):
    if not builds:
        return "<p class=\"empty\">No builds for this branch.</p>"

    rendered = []
    for build in builds[:5]:
        commit = build.get("commit", "")
        short_commit = commit[:12] if commit else "unknown"
        commit_html = html.escape(short_commit)
        if build.get("commitUrl"):
            commit_html = f"<a class=\"commit\" href=\"{html.escape(build['commitUrl'])}\">{commit_html}</a>"

        build_link = f"#{html.escape(str(build.get('buildNumber', 'n/a')))}"
        if build.get("buildUrl"):
            build_link = f"<a href=\"{html.escape(build['buildUrl'])}\">{build_link}</a>"

        artifacts = []
        for artifact in build.get("artifacts", []):
            artifacts.append(
                f"<a class=\"artifact\" href=\"{html.escape(artifact['path'])}\">"
                f"{html.escape(artifact['name'])} <span class=\"meta\">{html.escape(format_size(artifact.get('size', 0)))}</span>"
                "</a>"
            )
        failed = str(build.get("result", "")).lower() in ("failure", "failed", "unstable", "aborted")
        if failed and build.get("log"):
            log = build["log"]
            artifacts.append(
                f"<a class=\"artifact\" href=\"{html.escape(log['path'])}\">"
                f"Error Log <span class=\"meta\">{html.escape(format_size(log.get('size', 0)))}</span>"
                "</a>"
            )
        elif build.get("branchSlug") and build.get("buildNumber"):
            artifacts.append(
                f"<a class=\"artifact\" href=\"demos/build/{html.escape(build['branchSlug'])}/{html.escape(str(build['buildNumber']))}/\">"
                "Demos</a>"
            )
        artifact_html = "\n".join(artifacts) if artifacts else "<span class=\"meta\">No artifact</span>"

        rendered.append(f"""
        <article class="build">
          <div class="build-number">{build_link}</div>
          <div>
            <div>{html.escape(build.get("result", "UNKNOWN"))} on {html.escape(build.get("builtAt", "unknown"))}</div>
            <div class="meta">Commit {commit_html}</div>
          </div>
          <div class="artifact-list">{artifact_html}</div>
        </article>
        """)
    return "\n".join(rendered)


def main():
    parser = argparse.ArgumentParser(description="Generate the public Boxedwine build status site.")
    parser.add_argument("--site-dir", required=True)
    parser.add_argument("--title", default="Boxedwine Builds")
    parser.add_argument("--branch", default=os.environ.get("BRANCH_NAME", "unknown"))
    parser.add_argument("--build-number", default=os.environ.get("BUILD_NUMBER", "0"))
    parser.add_argument("--result", default=os.environ.get("BUILD_RESULT", os.environ.get("currentBuild.currentResult", "SUCCESS")))
    parser.add_argument("--commit", default=os.environ.get("GIT_COMMIT", ""))
    parser.add_argument("--commit-url", default=os.environ.get("GIT_URL", ""))
    parser.add_argument("--build-url", default=os.environ.get("BUILD_URL", ""))
    parser.add_argument("--artifact", action="append", default=[])
    parser.add_argument("--log")
    parser.add_argument("--demo-source")
    parser.add_argument("--single-threaded-dir")
    parser.add_argument("--multi-threaded-dir")
    parser.add_argument("--keep", type=int, default=5)
    args = parser.parse_args()

    site_dir = Path(args.site_dir)
    site_dir.mkdir(parents=True, exist_ok=True)

    branch_slug = slugify(args.branch)
    branch_dir = site_dir / "branches" / branch_slug
    build_dir = branch_dir / "builds" / str(args.build_number)
    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)

    artifacts = copy_artifacts(args.artifact, build_dir)
    log = copy_log(args.log, build_dir)
    built_at = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    commit_url = ""
    if args.commit and args.commit_url:
        repo_url = args.commit_url[:-4] if args.commit_url.endswith(".git") else args.commit_url
        commit_url = f"{repo_url}/commit/{args.commit}"

    write_json(branch_dir / "branch.json", {"name": args.branch, "slug": branch_slug})
    write_json(
        build_dir / "build.json",
        {
            "branch": args.branch,
            "branchSlug": branch_slug,
            "buildNumber": int(args.build_number) if str(args.build_number).isdigit() else args.build_number,
            "result": args.result,
            "commit": args.commit,
            "commitUrl": commit_url,
            "buildUrl": args.build_url,
            "builtAt": built_at,
            "artifacts": artifacts,
            "log": log,
        },
    )

    if args.demo_source and args.single_threaded_dir and args.multi_threaded_dir:
        update_demos(
            site_dir,
            args.branch,
            branch_slug,
            args.build_number,
            args.demo_source,
            args.single_threaded_dir,
            args.multi_threaded_dir,
            args.keep,
        )

    prune_removed_demo_branches(site_dir)
    prune_old_builds(branch_dir, args.keep)
    branches = load_builds(site_dir)
    write_json(site_dir / "builds.json", {"generatedAt": built_at, "branches": branches})
    render_html(site_dir, branches, args.title)


if __name__ == "__main__":
    main()
