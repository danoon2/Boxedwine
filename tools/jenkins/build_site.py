#!/usr/bin/env python3
import argparse
import html
import json
import os
import re
import shutil
from datetime import datetime, timezone
from pathlib import Path


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
    rows = []
    for branch in branches:
        latest = branch["builds"][0] if branch["builds"] else {}
        status = latest.get("result", "UNKNOWN").lower()
        branch_name = html.escape(branch["name"])
        rows.append(f"""
        <section class="branch">
          <div class="branch-header">
            <div>
              <h2>{branch_name}</h2>
              <p>Latest build #{html.escape(str(latest.get("buildNumber", "n/a")))} on {html.escape(latest.get("builtAt", "unknown"))}</p>
            </div>
            <span class="status {html.escape(status)}">{html.escape(latest.get("result", "UNKNOWN"))}</span>
          </div>
          <div class="build-list">
            {render_builds(branch["builds"])}
          </div>
        </section>
        """)

    content = "\n".join(rows) if rows else "<p class=\"empty\">No builds have been published yet.</p>"
    html_page = f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>{html.escape(title)}</title>
  <style>
    :root {{
      color-scheme: light;
      --bg: #f5f7fa;
      --panel: #ffffff;
      --text: #17202a;
      --muted: #647184;
      --line: #d9e0e8;
      --success: #117a37;
      --failure: #bd2626;
      --running: #936a00;
      --unknown: #5d6573;
      --link: #155cc1;
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
      padding: 28px 24px;
      border-bottom: 4px solid #4ba36f;
    }}
    header .inner, main {{
      max-width: 1120px;
      margin: 0 auto;
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
    main {{
      padding: 24px;
    }}
    .branch {{
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 8px;
      margin-bottom: 18px;
      overflow: hidden;
      box-shadow: 0 1px 2px rgba(23, 32, 42, 0.05);
    }}
    .branch-header {{
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 16px;
      padding: 18px 20px;
      border-bottom: 1px solid var(--line);
      background: #fbfcfe;
    }}
    h2 {{
      margin: 0;
      font-size: 20px;
      letter-spacing: 0;
    }}
    .branch-header p {{
      margin: 4px 0 0;
      color: var(--muted);
      font-size: 14px;
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
    }}
    .success {{ background: var(--success); }}
    .failure, .failed, .unstable, .aborted {{ background: var(--failure); }}
    .running, .building {{ background: var(--running); }}
    .unknown, .not_built {{ background: var(--unknown); }}
    .build {{
      display: grid;
      grid-template-columns: 120px 1fr auto;
      gap: 16px;
      padding: 14px 20px;
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
      min-width: 220px;
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
    @media (max-width: 760px) {{
      header {{ padding: 22px 16px; }}
      main {{ padding: 16px; }}
      .branch-header {{
        align-items: flex-start;
        flex-direction: column;
      }}
      .build {{
        grid-template-columns: 1fr;
        gap: 8px;
      }}
      .artifact-list {{
        justify-content: flex-start;
        min-width: 0;
      }}
    }}
  </style>
</head>
<body>
  <header>
    <div class="inner">
      <h1>{html.escape(title)}</h1>
      <p>Last updated {html.escape(generated_at)}</p>
    </div>
  </header>
  <main>
    {content}
  </main>
</body>
</html>
"""
    (site_dir / "index.html").write_text(html_page, encoding="utf-8")


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
        },
    )

    prune_old_builds(branch_dir, args.keep)
    branches = load_builds(site_dir)
    write_json(site_dir / "builds.json", {"generatedAt": built_at, "branches": branches})
    render_html(site_dir, branches, args.title)


if __name__ == "__main__":
    main()
