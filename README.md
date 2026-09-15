# Saem Physics — website

A one-page site for Saem Physics: admissions, programs (NEET, JEE, MIT
admissions, Olympiads), notes, and contact links. Plain HTML, CSS, and a
little vanilla JavaScript — no build step, no frameworks, ready for
GitHub Pages.

```
saem-physics-website/
├── index.html
├── css/
│   └── style.css
├── js/
│   └── script.js
└── README.md
```

## Publish it with GitHub Pages

1. Create a new repository on GitHub (e.g. `saem-physics`). Don't
   initialize it with a README — you already have one here.
2. Upload these files, keeping the folder structure above. Easiest way,
   from inside this folder:
   ```
   git init
   git add .
   git commit -m "Initial site"
   git branch -M main
   git remote add origin https://github.com/<your-username>/<repo-name>.git
   git push -u origin main
   ```
   (No git installed, or prefer a browser? Use GitHub's "Add file →
   Upload files" button on the repo page instead — drag in `index.html`,
   the `css` folder, and the `js` folder.)
3. On GitHub, go to your repo's **Settings → Pages**.
4. Under **Build and deployment → Source**, choose **Deploy from a
   branch**. Under **Branch**, choose `main` and folder `/ (root)`, then
   **Save**.
5. GitHub will publish the site at:
   `https://<your-username>.github.io/<repo-name>/`
   It usually takes a minute or two to go live the first time.

If you'd rather the site live at the shorter `https://<your-username>.github.io/`
(no repo name in the URL), name the repository exactly
`<your-username>.github.io` instead — GitHub treats that repo name as
your personal site automatically.

## Everything you'll want to edit is in `index.html`

All the outbound links (admission form, Telegram, Webex, email, maps)
are plain `href` attributes near the top and in the **Resources** and
**Contact** sections — search for the link text to find and change one.

## Customizing the look

Colors, fonts, and spacing are all defined once at the top of
`css/style.css` as CSS variables:

```css
:root {
  --bg: #141d1a;        /* page background */
  --ink: #f3efe2;       /* main text color */
  --gold: #ddb862;      /* accent — buttons, links, highlights */
  ...
}
```

Change a value there and it updates everywhere it's used.

## Optional: a custom domain

If you own a domain and want the site at, say, `www.saemphysics.com`
instead of the github.io address, add a `CNAME` file containing just
that domain to the repo root, then point your domain's DNS at GitHub
Pages (GitHub's Pages docs walk through the exact DNS records to add).
