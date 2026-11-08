# Saem Physics

The public front door for Saem Physics is a responsive, static website for
students exploring lessons, exam solutions, and physics through computation.
The design keeps the existing channel, playlist, admission, Telegram, and live
class links, with a searchable lesson library and short code examples in the
requested languages.

## Run locally

From the repository root:

```sh
python3 -m http.server 8000
```

Open `http://localhost:8000`. Stop the server with Ctrl+C.

## Publish with GitHub Pages

The [Pages workflow](.github/workflows/deploy-pages.yml) deploys the three
website files (`index.html`, `style.css`, and `script.js`) when changes are
pushed to `main`. It intentionally excludes the research books, archives, and
backend source from the Pages artifact. In GitHub, open **Settings → Pages**
and select **GitHub Actions** as the build and deployment source. No build
runtime is required to serve the site.

The public site is static: GitHub Pages does not execute the C17 backend. The
language examples are teaching sketches, not integrations. The intended
offline EdTech application and the current state of each language are described
in [the staged architecture](docs/SAEM_EDTECH_ARCHITECTURE.md).

## Project map

- [index.html](index.html), [style.css](style.css), [script.js](script.js):
  public website and browser interactions.
- [docs/qcos/](docs/qcos/): QCOS design and Work-Time Engine plan.
- [references/](references/): supplied books, archives, and source snapshot,
  kept separate from deployable site assets.
- [saem-unified/](saem-unified/): restored C17/C++ source, tests, and tools.
