# Saem Physics — website

A four-page site for Saem Physics: Home, About, Programs (NEET, JEE, MIT
admissions, Olympiads), and Contact (admission form, notes, resources, and
every way to reach Saem). Plain HTML, CSS, and a little vanilla
JavaScript — no build step, no frameworks, ready for GitHub Pages.

```
saem-physics-website/
├── index.html       Home
├── about.html        About
├── programs.html      Programs
├── contact.html        Contact, resources, FAQ
├── css/
│   └── style.css     shared stylesheet
├── js/
│   └── script.js     shared script (nav menu, footer year)
└── README.md
```

All four pages share the same header, footer, and stylesheet, so the look
stays consistent — updating a color or font in `css/style.css` updates
every page at once.

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
   Upload files" button on the repo page instead — drag in all four
   `.html` files, the `css` folder, and the `js` folder.)
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

## Everything you'll want to edit

- **Links** (admission form, Telegram, Webex, email, maps) are plain
  `href` attributes — mainly in `contact.html`, with a couple repeated
  on `index.html`. Search for the link text to find and change one.
- **Programs copy** lives in `programs.html` (short version on
  `index.html` too).
- **About page**: `about.html` has an HTML comment marking exactly where
  a personal note from Saem can go — open the file in a text editor to
  find it, or just send the details and it can be written in properly.

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

Change a value there and it updates everywhere it's used, on every page.

## A note on the contact form

Earlier drafts of this kind of site sometimes use a `mailto:` form
action (`<form action="mailto:...">`) for a "contact form." It's left
out here on purpose — it's unreliable across browsers and mobile mail
apps, and can silently fail. A direct `mailto:` link plus the Google
Form (both already in `contact.html`) work everywhere without a
backend.

## Optional: a logo or photos

There's no `assets/` folder yet since no images were provided. To add a
logo or photos later, drop image files into a new `assets/` folder and
reference them in the HTML (e.g. `<img src="assets/logo.png" alt="Saem
Physics logo">`) — or share them and they can be wired in directly.

## Optional: a custom domain

If you own a domain and want the site at, say, `www.saemphysics.com`
instead of the github.io address, add a `CNAME` file containing just
that domain to the repo root, then point your domain's DNS at GitHub
Pages (GitHub's Pages docs walk through the exact DNS records to add).
