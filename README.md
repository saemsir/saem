# Saem Physics — channel website

This repository currently contains a static landing page for the Saem Physics
YouTube channel. It links to lessons, playlists, notes, admission, and live
classes. It uses plain HTML, CSS, and JavaScript; there is no build step or
backend in the root website.

The staged plan for the wider offline-first education product and the roles of
QCOS, RISC-V, Open Math Harness, and the other languages is in
[docs/SAEM_EDTECH_ARCHITECTURE.md](docs/SAEM_EDTECH_ARCHITECTURE.md).

## Preview locally

From the repository root:

```sh
python3 -m http.server 8000
```

Then open `http://localhost:8000`. Stop the server with Ctrl+C.

## Publish with GitHub Pages

This site is already at the repository root. In GitHub, open **Settings →
Pages**, choose **Deploy from a branch**, select the publishing branch and
`/(root)`, and save. The site will be published at
`https://<owner>.github.io/<repository>/`.

The page and assets use relative paths, so the repository can be published
from a project subpath. A custom domain can be configured later in GitHub Pages
settings after registering and configuring that domain.

## Edit the site

- Page sections and outbound links: [index.html](index.html)
- Lesson, topic, and playlist cards: [script.js](script.js)
- Colors, layout, and responsive styling: [style.css](style.css)

The PDFs, archives, and packed Saem source snapshot in this repository are
reference materials; they are not loaded by the static website.
