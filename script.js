(() => {
  const lessons = [
    { category: "exam", tag: "JEE ADVANCED", title: "JEE Advanced Physics Paper Solution", description: "Detailed solutions with the reasoning behind each step.", url: "https://www.youtube.com/live/3TT1crLy9PQ?si=gyVhZIM5InkcNF4o" },
    { category: "exam", tag: "JEE MAINS", title: "JEE Mains Physics Paper Solution", description: "Work through the paper and understand why each answer works.", url: "https://www.youtube.com/live/3IbsP-95MRE?si=BYujEr7TzCAeNRiB" },
    { category: "exam", tag: "NEET", title: "NEET Physics Paper Solution", description: "A clear, step-by-step discussion of a NEET physics paper.", url: "https://www.youtube.com/live/Ya75UNbuVaI?si=JN63seRXFpHVkw8d" },
    { category: "concept", tag: "MECHANICS", title: "Newton's Laws of Motion", description: "Connect force, acceleration, and motion with a physical picture.", url: "https://www.youtube.com/live/ARLQwOAdqd0?si=HOgtkaaQIqB0UaC2" },
    { category: "concept", tag: "ROTATIONAL MOTION", title: "Angular Momentum", description: "Explore rotational motion through intuition and equations.", url: "https://youtu.be/iGLd6pqGu4w?si=UUK1ZToThHkY8fD_" },
    { category: "concept", tag: "OPTICS", title: "Refraction of Light", description: "Understand why light changes direction between materials.", url: "https://youtu.be/dD5CMx5GeSM?si=uSyKlizkeR7II3kT" },
    { category: "revision", tag: "CBSE CLASS 12", title: "CBSE Class 12 Physics PYQs", description: "Practice previous-year questions concept-first.", url: "https://www.youtube.com/live/FvA6a8dzL0o?si=KaM5WR5qZ9bzRz5C" },
    { category: "revision", tag: "BIHAR BOARD", title: "Bihar Board Class 12 PYQs", description: "Prepare with guided previous-year question solutions.", url: "https://www.youtube.com/live/3RmLjiBsL1Q?si=8xGlEimTatNyZGV0" },
    { category: "concept", tag: "NCERT", title: "NCERT Physics, Made Clear", description: "Build connections between core NCERT ideas and problems.", url: "https://www.youtube.com/live/fpqUMPcvvmc?si=3dFFE43w-sMJs3x9" },
    { category: "concept", tag: "MATHEMATICAL TOOLS", title: "Units, Dimensions & Errors", description: "Use measurement as a tool for checking physical reasoning.", url: "https://youtube.com/playlist?list=PLjEkQ_dvlIGk-_9QojNMbg6vhRrk1C0Yg&si=YWpAnwJcX90NDFev" },
    { category: "concept", tag: "WORK & ENERGY", title: "Work, Energy & Power", description: "Solve questions by choosing the right physical viewpoint.", url: "https://www.youtube.com/live/94KWCWIpTWM?si=GXJzfDrZw8iYHCC8" },
    { category: "revision", tag: "QUICK REVISION", title: "Electrostatics", description: "Review electric fields, force, and central ideas.", url: "https://youtu.be/OzuS2LwJwzI?si=531yUfZV0wWUEuSS" },
    { category: "revision", tag: "QUICK REVISION", title: "Gravitation", description: "Revisit gravity, orbits, and central forces.", url: "https://youtube.com/playlist?list=PLjEkQ_dvlIGlDLayMTKe7kAf28RswrXAc&si=aWb3DhyKGrZKYCxZ" },
    { category: "concept", tag: "MODERN PHYSICS", title: "Modern Physics", description: "Explore the ideas beyond classical intuition.", url: "https://youtube.com/playlist?list=PLjEkQ_dvlIGmYGbJoXdyKCa3eN6w0OrL0&si=FMKiyUA__LxuaSLx" },
  ];

  const examples = [
    {
      name: "C17 · exact core",
      result: "For u=4, a=2, t=3: displacement = 21/1 exactly.",
      code: `#include "cos_rat.h"

CosRat u = {4, 1}, a = {2, 1}, t = {3, 1};
CosRat half = {1, 2}, ut, t2, at2, half_at2, s = {0, 1};
CosStatus st = cos_rat_mul(u, t, &ut);
if (st == COS_OK) st = cos_rat_mul(t, t, &t2);
if (st == COS_OK) st = cos_rat_mul(a, t2, &at2);
if (st == COS_OK) st = cos_rat_mul(at2, half, &half_at2);
if (st == COS_OK) st = cos_rat_add(ut, half_at2, &s);`
    },
    {
      name: "C",
      result: "For these small integer inputs, s = 21.",
      code: `#include <stdint.h>

int main(void) {
  const int64_t u = 4, a = 2, t = 3;
  const int64_t s = u * t + (a * t * t) / 2;
  return s == 21 ? 0 : 1;
}`
    },
    {
      name: "C++",
      result: "For these small integer inputs, s = 21.",
      code: `#include <cstdint>

int main() {
  constexpr std::int64_t u = 4, a = 2, t = 3;
  constexpr auto s = u * t + (a * t * t) / 2;
  return s == 21 ? 0 : 1;
}`
    },
    {
      name: "Python",
      result: "Python integers keep this integer example exact.",
      code: `u, a, t = 4, 2, 3
s = u * t + (a * t * t) // 2
print(s)  # 21`
    },
    {
      name: "Rust",
      result: "For these small integer inputs, s = 21.",
      code: `fn main() {
    let (u, a, t): (i64, i64, i64) = (4, 2, 3);
    let s = u * t + (a * t * t) / 2;
    println!("{s}");
}`
    },
    {
      name: "R",
      result: "R prints the displacement as 21.",
      code: `u <- 4
a <- 2
t <- 3
s <- u * t + (a * t^2) / 2
print(s)  # 21`
    },
    {
      name: "Julia",
      result: "Integer division is exact here because the numerator is even.",
      code: `u, a, t = 4, 2, 3
s = u * t + (a * t^2) ÷ 2
println(s)  # 21`
    },
    {
      name: "Fortran",
      result: "For these small integer inputs, s = 21.",
      code: `program motion
  implicit none
  integer :: u, a, t, s
  u = 4; a = 2; t = 3
  s = u * t + (a * t * t) / 2
  print *, s
end program motion`
    },
    {
      name: "Lisp",
      result: "Common Lisp evaluates this integer expression to 21.",
      code: `(let ((u 4) (a 2) (t 3))
  (+ (* u t)
     (/ (* a t t) 2)))`
    },
  ];

  const lessonGrid = document.getElementById("lesson-grid");
  const search = document.getElementById("lesson-search");
  const count = document.getElementById("results-count");
  const emptyState = document.getElementById("empty-state");
  const filters = document.getElementById("lesson-filters");
  let activeFilter = "all";

  function makeLessonCard(lesson) {
    const link = document.createElement("a");
    link.className = "lesson-card";
    link.href = lesson.url;
    link.target = "_blank";
    link.rel = "noopener noreferrer";

    const topline = document.createElement("span");
    topline.className = "lesson-topline";
    topline.textContent = lesson.tag;

    const title = document.createElement("h3");
    title.textContent = lesson.title;

    const description = document.createElement("p");
    description.textContent = lesson.description;

    const action = document.createElement("span");
    action.className = "lesson-link";
    action.append("Watch lesson", document.createTextNode("↗"));

    link.append(topline, title, description, action);
    return link;
  }

  function renderLessons() {
    const query = search.value.trim().toLocaleLowerCase();
    const visible = lessons.filter((lesson) => {
      const matchesFilter = activeFilter === "all" || lesson.category === activeFilter;
      const matchesQuery = `${lesson.tag} ${lesson.title} ${lesson.description}`.toLocaleLowerCase().includes(query);
      return matchesFilter && matchesQuery;
    });

    lessonGrid.replaceChildren(...visible.map(makeLessonCard));
    count.textContent = `${visible.length} ${visible.length === 1 ? "lesson" : "lessons"}`;
    emptyState.hidden = visible.length !== 0;
  }

  search.addEventListener("input", renderLessons);
  filters.addEventListener("click", (event) => {
    const button = event.target.closest("button[data-filter]");
    if (!button) return;
    activeFilter = button.dataset.filter;
    filters.querySelectorAll("button[data-filter]").forEach((item) => {
      const selected = item === button;
      item.classList.toggle("is-active", selected);
      item.setAttribute("aria-pressed", String(selected));
    });
    renderLessons();
  });
  document.addEventListener("keydown", (event) => {
    if (event.key === "/" && !event.ctrlKey && !event.metaKey && !event.altKey && !["INPUT", "TEXTAREA"].includes(document.activeElement.tagName)) {
      event.preventDefault();
      search.focus();
    }
  });
  renderLessons();

  const tabs = document.getElementById("language-tabs");
  const code = document.getElementById("code-example");
  const codeLanguage = document.getElementById("code-language");
  const codeResult = document.getElementById("code-result");
  const copyButton = document.getElementById("copy-code");
  let selectedExample = 0;

  function selectExample(index) {
    selectedExample = index;
    const example = examples[index];
    code.textContent = example.code;
    codeLanguage.textContent = example.name;
    codeResult.textContent = example.result;
    tabs.querySelectorAll('[role="tab"]').forEach((tab, tabIndex) => {
      const selected = tabIndex === index;
      tab.setAttribute("aria-selected", String(selected));
      tab.tabIndex = selected ? 0 : -1;
    });
    document.getElementById("code-panel").setAttribute("aria-labelledby", `language-tab-${index}`);
    copyButton.textContent = "Copy code";
  }

  examples.forEach((example, index) => {
    const tab = document.createElement("button");
    tab.className = "language-tab";
    tab.type = "button";
    tab.id = `language-tab-${index}`;
    tab.setAttribute("role", "tab");
    tab.setAttribute("aria-controls", "code-panel");
    tab.setAttribute("aria-selected", String(index === selectedExample));
    tab.tabIndex = index === selectedExample ? 0 : -1;
    tab.textContent = example.name;
    tab.addEventListener("click", () => selectExample(index));
    tab.addEventListener("keydown", (event) => {
      if (!["ArrowLeft", "ArrowRight", "Home", "End"].includes(event.key)) return;
      event.preventDefault();
      const next = event.key === "Home" ? 0
        : event.key === "End" ? examples.length - 1
          : (index + (event.key === "ArrowRight" ? 1 : -1) + examples.length) % examples.length;
      selectExample(next);
      tabs.children[next].focus();
    });
    tabs.append(tab);
  });
  selectExample(selectedExample);

  copyButton.addEventListener("click", async () => {
    try {
      await navigator.clipboard.writeText(examples[selectedExample].code);
      copyButton.textContent = "Copied";
    } catch (error) {
      copyButton.textContent = "Select code to copy";
    }
  });

  const menuToggle = document.getElementById("menu-toggle");
  const primaryNav = document.getElementById("primary-nav");
  function closeMenu() {
    menuToggle.setAttribute("aria-expanded", "false");
    menuToggle.setAttribute("aria-label", "Open navigation");
    primaryNav.classList.remove("is-open");
  }
  menuToggle.addEventListener("click", () => {
    const open = menuToggle.getAttribute("aria-expanded") !== "true";
    menuToggle.setAttribute("aria-expanded", String(open));
    menuToggle.setAttribute("aria-label", open ? "Close navigation" : "Open navigation");
    primaryNav.classList.toggle("is-open", open);
  });
  primaryNav.querySelectorAll("a").forEach((link) => link.addEventListener("click", closeMenu));
  document.addEventListener("click", (event) => {
    if (!primaryNav.contains(event.target) && !menuToggle.contains(event.target)) closeMenu();
  });
  document.addEventListener("keydown", (event) => {
    if (event.key === "Escape") closeMenu();
  });

  document.getElementById("current-year").textContent = String(new Date().getFullYear());
})();
