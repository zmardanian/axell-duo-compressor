#!/usr/bin/env bash
# Builds the GitHub-ready public repository (~/axell-duo-compressor-public): sources, tests, docs, validation tools and
# licences - no dev notes (CLAUDE.md, PROGRESS, plans), build output, downloaded tools or generated audio. A new
# repository gets one clean commit; an existing one is updated in place ("Axell Duo Compressor v<ver>", tag v<ver>).
# Sets the remote, never pushes.
set -euo pipefail
cd "$(dirname "$0")/.."
PUB="${DUO_PUBLIC_DIR:-$HOME/axell-duo-compressor-public}"
VER=$(sed -n 's/^project(AxellDuoCompressor VERSION \([0-9.]*\)).*/\1/p' CMakeLists.txt)
FINAL="$PUB"; UPDATE=0
if [ -d "$PUB/.git" ]; then UPDATE=1; PUB="$(mktemp -d)/stage"
elif [ -e "$PUB" ]; then echo "$PUB exists but is not a git repository - remove it first."; exit 1; fi
mkdir -p "$PUB"
for f in CMakeLists.txt LICENSE README.md CHANGELOG.md THIRD_PARTY_NOTICES.md; do cp "$f" "$PUB/"; done
cp -r Resources Source scripts "$PUB/"
mkdir -p "$PUB/tests/independent" "$PUB/docs" "$PUB/tools/validation" "$PUB/tools/listening" "$PUB/tools/benchmark"
cp tests/*.cpp tests/*.h tests/lsan.supp tests/report.md tests/report-asan.md "$PUB/tests/"
cp tests/independent/*.cpp tests/independent/*.py tests/independent/README.md tests/independent/audit-report.txt "$PUB/tests/independent/"
cp docs/ENGINEERING_SPEC.md docs/DECISIONS.md docs/pluginval-log.txt "$PUB/docs/"
cp -r docs/research docs/screenshots "$PUB/docs/"
cp tools/duo_render.cpp tools/retune_presets.cpp "$PUB/tools/"
cp tools/validation/*.py tools/validation/*.json "$PUB/tools/validation/"
cp tools/listening/index.html tools/listening/make_stimuli.py "$PUB/tools/listening/"
cp tools/benchmark/*.py "$PUB/tools/benchmark/"
cat > "$PUB/.gitignore" <<'IGN'
build/
build-*/
tools/pluginval*
tools/vst3-validator*
tools/listening/stimuli/
__pycache__/
*.user
IGN
if [ "$UPDATE" = 1 ]; then
  git -C "$FINAL" diff --quiet && git -C "$FINAL" diff --cached --quiet || { echo "$FINAL has uncommitted changes."; exit 1; }
  git -C "$FINAL" rev-parse -q --verify "refs/tags/v$VER" >/dev/null && { echo "tag v$VER already exists in $FINAL."; exit 1; }
  rsync -a --delete --exclude .git "$PUB/" "$FINAL/"; rm -rf "$(dirname "$PUB")"
  cd "$FINAL"; git add -A; git commit -qm "Axell Duo Compressor v$VER"; git tag "v$VER"
  echo "Public repo updated: $FINAL (tag v$VER, NOT pushed)"; exit 0
fi
cd "$PUB"
git init -q -b main
git config user.name "zmardanian"
git config user.email "333182917+zmardanian@users.noreply.github.com"
git add -A
git commit -qm "Axell Duo Compressor v$VER"
git tag "v$VER"
git remote add origin "https://github.com/zmardanian/axell-duo-compressor.git"
echo "Public repo ready: $PUB (commit $(git rev-parse --short HEAD), tag v$VER, remote origin set, NOT pushed)"
