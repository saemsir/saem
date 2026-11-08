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

  const channelGrid = document.getElementById("channel-video-grid");
  const channelStatus = document.getElementById("channel-feed-status");
  const channelRefresh = document.getElementById("channel-feed-refresh");
  function makeChannelVideo(video) {
    const card = document.createElement("article");
    card.className = "channel-video";

    const top = document.createElement("span");
    top.className = "channel-video-top";
    const source = document.createElement("span");
    source.textContent = "SAEM PHYSICS · VIDEO";
    const time = document.createElement("time");
    const published = new Date(video.published);
    if (Number.isNaN(published.getTime())) {
      time.textContent = "Recent upload";
    } else {
      time.dateTime = published.toISOString();
      time.textContent = new Intl.DateTimeFormat(undefined, { dateStyle: "medium" }).format(published);
    }
    top.append(source, time);

    const thumbnail = document.createElement("img");
    thumbnail.src = `https://i.ytimg.com/vi/${video.videoId}/hqdefault.jpg`;
    thumbnail.alt = "";
    thumbnail.loading = "lazy";
    thumbnail.decoding = "async";

    const playButton = document.createElement("button");
    playButton.className = "channel-video-play";
    playButton.type = "button";
    playButton.setAttribute("aria-label", `Play ${video.title} on this page`);
    const playLabel = document.createElement("span");
    playLabel.className = "channel-play-icon";
    playLabel.setAttribute("aria-hidden", "true");
    playLabel.textContent = "▶";
    playButton.append(thumbnail, playLabel);
    playButton.addEventListener("click", () => {
      const frame = document.createElement("iframe");
      frame.className = "channel-video-player";
      frame.src = `https://www.youtube-nocookie.com/embed/${video.videoId}?autoplay=1&rel=0`;
      frame.title = video.title;
      frame.loading = "lazy";
      frame.allow = "accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share";
      frame.referrerPolicy = "strict-origin-when-cross-origin";
      frame.allowFullscreen = true;
      playButton.replaceWith(frame);
    }, { once: true });

    const title = document.createElement("h3");
    title.textContent = video.title;

    const description = document.createElement("p");
    description.className = "channel-video-description";
    description.textContent = typeof video.description === "string" ? video.description : "";
    description.hidden = !description.textContent;

    const action = document.createElement("span");
    action.className = "channel-video-action";
    const playText = document.createElement("span");
    playText.textContent = "Play here";
    const external = document.createElement("a");
    external.href = `https://www.youtube.com/watch?v=${video.videoId}`;
    external.target = "_blank";
    external.rel = "noopener noreferrer";
    external.textContent = "YouTube ↗";
    action.append(playText, external);

    card.append(top, playButton, title, description, action);
    return card;
  }

  let channelLoadInProgress = false;
  async function loadChannelVideos() {
    if (channelLoadInProgress) return;
    channelLoadInProgress = true;
    channelRefresh.disabled = true;
    channelStatus.textContent = "Checking the latest channel feed…";
    try {
      const response = await fetch(`./channel-feed.json?check=${Date.now()}`, { cache: "no-store" });
      if (!response.ok) throw new Error(`Channel feed request failed (${response.status})`);
      const feed = await response.json();
      const videos = Array.isArray(feed.items)
        ? feed.items.filter((video) => video && /^[A-Za-z0-9_-]{11}$/.test(video.videoId) &&
          typeof video.title === "string" && video.title.trim() && typeof video.published === "string")
        : [];
      if (videos.length === 0) {
        throw new Error("The channel feed did not contain any videos");
      }
      channelGrid.replaceChildren(...videos.slice(0, 6).map(makeChannelVideo));
      const refreshed = new Date(feed.generatedAt);
      channelStatus.textContent = Number.isNaN(refreshed.getTime())
        ? "Latest upload updates · checked just now"
        : `Checked just now · channel snapshot ${new Intl.DateTimeFormat(undefined, { dateStyle: "medium", timeStyle: "short" }).format(refreshed)}`;
    } catch (error) {
      console.warn("Could not load the latest Saem Physics uploads:", error);
      channelStatus.textContent = "Recent uploads are temporarily unavailable. Visit the channel for the latest videos.";
      const fallback = document.createElement("a");
      fallback.className = "channel-fallback";
      fallback.href = "https://www.youtube.com/@SaemPhysics/videos";
      fallback.target = "_blank";
      fallback.rel = "noopener noreferrer";
      fallback.textContent = "Open the Saem Physics video archive ↗";
      channelGrid.replaceChildren(fallback);
    } finally {
      channelLoadInProgress = false;
      channelRefresh.disabled = false;
    }
  }
  channelRefresh.addEventListener("click", loadChannelVideos);
  loadChannelVideos();
  window.setInterval(() => {
    if (!document.hidden) loadChannelVideos();
  }, 15 * 60 * 1000);

  const liveButton = document.getElementById("channel-live-load");
  const livePlayer = document.getElementById("channel-live-player");
  const liveStatus = document.getElementById("channel-live-status");
  liveButton.addEventListener("click", () => {
    const frame = document.createElement("iframe");
    frame.className = "channel-live-frame";
    frame.src = "https://www.youtube-nocookie.com/embed/live_stream?channel=UCJlNb8kQgLWqb65htic2CkQ";
    frame.title = "Saem Physics live stream";
    frame.loading = "lazy";
    frame.allow = "accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share";
    frame.referrerPolicy = "strict-origin-when-cross-origin";
    frame.allowFullscreen = true;
    livePlayer.replaceChildren(frame);
    livePlayer.hidden = false;
    liveStatus.textContent = "The player is connected. If no public broadcast is running, browse the channel streams.";
  });

  const practiceWidget = document.getElementById("practice-widget");
  if (practiceWidget) {
    const studio = window.SaemStudentStudio;
    if (!studio) throw new Error("The student practice module did not load.");
    const stepLabel = document.getElementById("practice-step");
    const progress = document.getElementById("practice-progress");
    const questionHeading = document.getElementById("practice-question");
    const options = document.getElementById("practice-options");
    const feedback = document.getElementById("practice-feedback");
    const checkButton = document.getElementById("practice-check");
    const nextButton = document.getElementById("practice-next");
    const resetButton = document.getElementById("practice-reset");
    const storageStatus = document.getElementById("practice-storage-status");
    const progressKey = "saem-physics-practice-v1";
    let questionIndex = 0;
    let storageAvailable = true;
    let answeredCorrectly = false;

    function disableProgressStorage(message, error) {
      storageAvailable = false;
      storageStatus.textContent = message;
      if (error) console.warn("Student progress is not being stored:", error);
    }

    try {
      questionIndex = studio.restoreCompletedProgress(window.localStorage.getItem(progressKey));
    } catch (error) {
      disableProgressStorage("Browser storage is unavailable or invalid. Your answers stay in this session only.", error);
    }

    function saveProgress() {
      if (!storageAvailable) return;
      try {
        window.localStorage.setItem(progressKey, studio.serializeCompletedProgress(questionIndex));
        storageStatus.textContent = "Progress is stored only in this browser.";
      } catch (error) {
        disableProgressStorage("Browser storage is unavailable. Progress will reset when you leave this page.", error);
      }
    }

    function renderPractice() {
      const isComplete = questionIndex >= studio.questions.length;
      progress.value = questionIndex;
      resetButton.hidden = questionIndex === 0;
      feedback.className = "practice-feedback";
      feedback.textContent = "";
      checkButton.hidden = isComplete;
      nextButton.hidden = true;

      if (isComplete) {
        stepLabel.textContent = "Session complete";
        questionHeading.hidden = false;
        questionHeading.textContent = "Practice complete!";
        options.hidden = true;
        feedback.textContent = "You completed all three motion questions. Revisit the visual lab or start this practice again.";
        feedback.className = "practice-feedback is-correct";
        resetButton.hidden = false;
        nextButton.hidden = true;
        return;
      }

      questionHeading.hidden = false;
      options.hidden = false;
      stepLabel.textContent = `Question ${questionIndex + 1} of ${studio.questions.length}`;
      const question = studio.questions[questionIndex];
      questionHeading.textContent = question.prompt;
      const legend = options.querySelector("legend");
      const optionLabels = question.options.map((option, index) => {
        const label = document.createElement("label");
        label.className = "practice-option";
        const input = document.createElement("input");
        input.type = "radio";
        input.name = "practice-answer";
        input.value = String(index);
        const answerText = document.createElement("span");
        answerText.textContent = option;
        label.append(input, answerText);
        return label;
      });
      options.replaceChildren(legend, ...optionLabels);
      answeredCorrectly = false;
    }

    checkButton.addEventListener("click", () => {
      if (answeredCorrectly || questionIndex >= studio.questions.length) return;
      const selected = options.querySelector('input[name="practice-answer"]:checked');
      if (!selected) {
        feedback.className = "practice-feedback is-incorrect";
        feedback.textContent = "Choose an answer first. Try estimating before calculating.";
        return;
      }
      const question = studio.questions[questionIndex];
      if (Number(selected.value) === question.answerIndex) {
        answeredCorrectly = true;
        feedback.className = "practice-feedback is-correct";
        feedback.textContent = `That is right. ${question.explanation}`;
        checkButton.hidden = true;
        nextButton.hidden = false;
      } else {
        feedback.className = "practice-feedback is-incorrect";
        feedback.textContent = "Not quite. Check what each quantity means, then try another answer.";
      }
    });

    nextButton.addEventListener("click", () => {
      if (!answeredCorrectly) return;
      questionIndex += 1;
      saveProgress();
      renderPractice();
      questionHeading.focus();
    });

    resetButton.addEventListener("click", () => {
      questionIndex = 0;
      if (storageAvailable) {
        try {
          window.localStorage.removeItem(progressKey);
        } catch (error) {
          disableProgressStorage("Browser storage is unavailable. Progress will reset when you leave this page.", error);
        }
      }
      renderPractice();
      questionHeading.focus();
    });

    renderPractice();
  }

  const examWidget = document.getElementById("exam-test-widget");
  if (examWidget) {
    const studio = window.SaemStudentStudio;
    if (!studio) throw new Error("The student exam module did not load.");
    const startButton = document.getElementById("exam-start");
    const startPanel = document.getElementById("exam-test-start");
    const sessionPanel = document.getElementById("exam-test-session");
    const resultsPanel = document.getElementById("exam-test-results");
    const questionCount = document.getElementById("exam-question-count");
    const timerLabel = document.getElementById("exam-timer");
    const progress = document.getElementById("exam-progress");
    const questionHeading = document.getElementById("exam-question");
    const options = document.getElementById("exam-options");
    const status = document.getElementById("exam-test-status");
    const previousButton = document.getElementById("exam-previous");
    const nextButton = document.getElementById("exam-next");
    const submitButton = document.getElementById("exam-submit");
    const totalSeconds = studio.examConfig.durationSeconds;
    let answers;
    let questionIndex;
    let remainingSeconds;
    let timerId = null;
    let confirmIncomplete = false;

    function updateExamTimer() {
      const minutes = Math.floor(remainingSeconds / 60);
      const seconds = remainingSeconds % 60;
      timerLabel.textContent = `${String(minutes).padStart(2, "0")}:${String(seconds).padStart(2, "0")}`;
    }

    function renderExamQuestion() {
      const question = studio.examQuestions[questionIndex];
      questionCount.textContent = `Question ${questionIndex + 1} of ${studio.examQuestions.length}`;
      questionHeading.textContent = question.prompt;
      progress.value = answers.filter((answer) => answer !== null).length;
      previousButton.disabled = questionIndex === 0;
      nextButton.hidden = questionIndex === studio.examQuestions.length - 1;
      submitButton.hidden = questionIndex !== studio.examQuestions.length - 1;
      status.textContent = "";
      confirmIncomplete = false;
      submitButton.textContent = "Finish test";

      const legend = options.querySelector("legend");
      const labels = question.options.map((option, index) => {
        const label = document.createElement("label");
        label.className = "practice-option";
        const input = document.createElement("input");
        input.type = "radio";
        input.name = "exam-answer";
        input.value = String(index);
        input.checked = answers[questionIndex] === index;
        input.addEventListener("change", () => {
          answers[questionIndex] = index;
          progress.value = answers.filter((answer) => answer !== null).length;
          status.textContent = "";
          confirmIncomplete = false;
          submitButton.textContent = "Finish test";
        });
        const answerText = document.createElement("span");
        answerText.textContent = option;
        label.append(input, answerText);
        return label;
      });
      options.replaceChildren(legend, ...labels);
    }

    function finishExam(byTimer) {
      if (timerId !== null) window.clearInterval(timerId);
      timerId = null;
      const result = studio.scoreExam(answers);
      const heading = document.createElement("h4");
      heading.textContent = `Your score: ${result.score} / ${result.maximumScore}`;
      const summary = document.createElement("p");
      summary.textContent = `${byTimer ? "Time is up. " : ""}Marks: correct ${studio.examConfig.marksCorrect}, incorrect ${studio.examConfig.marksIncorrect}, unanswered ${studio.examConfig.marksUnanswered}.`;
      const topicHeading = document.createElement("h5");
      topicHeading.textContent = "Topic check";
      const topicList = document.createElement("ul");
      topicList.className = "exam-topic-results";
      result.topicScores.forEach((topic) => {
        const item = document.createElement("li");
        item.textContent = `${topic.topic}: ${topic.correct} of ${topic.total} correct`;
        topicList.append(item);
      });
      const reviewHeading = document.createElement("h5");
      reviewHeading.textContent = "Worked review";
      const reviewList = document.createElement("div");
      reviewList.className = "exam-review-list";
      studio.examQuestions.forEach((question, index) => {
        const details = document.createElement("details");
        details.className = "exam-review-item";
        const summaryLine = document.createElement("summary");
        const answer = answers[index] === null ? "Not answered" : question.options[answers[index]];
        summaryLine.textContent = `${index + 1}. ${question.topic} · ${answers[index] === question.answerIndex ? "Correct" : "Review"} · Your answer: ${answer}`;
        const correct = document.createElement("p");
        correct.textContent = `Answer: ${question.options[question.answerIndex]}. ${question.explanation}`;
        details.append(summaryLine, correct);
        reviewList.append(details);
      });
      const restart = document.createElement("button");
      restart.className = "button button-quiet";
      restart.type = "button";
      restart.textContent = "Start another attempt";
      restart.addEventListener("click", resetExam);
      resultsPanel.replaceChildren(heading, summary, topicHeading, topicList, reviewHeading, reviewList, restart);
      sessionPanel.hidden = true;
      resultsPanel.hidden = false;
    }

    function resetExam() {
      if (timerId !== null) window.clearInterval(timerId);
      timerId = null;
      answers = Array(studio.examQuestions.length).fill(null);
      questionIndex = 0;
      remainingSeconds = totalSeconds;
      updateExamTimer();
      resultsPanel.hidden = true;
      startPanel.hidden = false;
      sessionPanel.hidden = true;
    }

    startButton.addEventListener("click", () => {
      answers = Array(studio.examQuestions.length).fill(null);
      questionIndex = 0;
      remainingSeconds = totalSeconds;
      updateExamTimer();
      startPanel.hidden = true;
      resultsPanel.hidden = true;
      sessionPanel.hidden = false;
      renderExamQuestion();
      questionHeading.focus();
      timerId = window.setInterval(() => {
        remainingSeconds -= 1;
        updateExamTimer();
        if (remainingSeconds <= 0) finishExam(true);
      }, 1000);
    });
    previousButton.addEventListener("click", () => {
      if (questionIndex === 0) return;
      questionIndex -= 1;
      renderExamQuestion();
      questionHeading.focus();
    });
    nextButton.addEventListener("click", () => {
      if (questionIndex >= studio.examQuestions.length - 1) return;
      questionIndex += 1;
      renderExamQuestion();
      questionHeading.focus();
    });
    submitButton.addEventListener("click", () => {
      const unanswered = answers.filter((answer) => answer === null).length;
      if (unanswered && !confirmIncomplete) {
        confirmIncomplete = true;
        submitButton.textContent = "Submit with blanks";
        status.textContent = `${unanswered} question${unanswered === 1 ? "" : "s"} unanswered. Choose Submit with blanks to finish now.`;
        return;
      }
      finishExam(false);
    });
  }

  const paperHistoryInput = document.getElementById("paper-history-file");
  if (paperHistoryInput) {
    const studio = window.SaemStudentStudio;
    if (!studio) throw new Error("The topic outlook module did not load.");
    const historyStatus = document.getElementById("paper-history-status");
    const boardSelect = document.getElementById("outlook-board");
    const languageSelect = document.getElementById("outlook-language");
    const targetYear = document.getElementById("outlook-year");
    const runForecast = document.getElementById("outlook-run");
    const forecastStatus = document.getElementById("outlook-status");
    const forecastResults = document.getElementById("outlook-results");
    let paperHistory = [];
    targetYear.value = String(new Date().getFullYear());

    function selectOptions(select, values, placeholder) {
      const options = [new Option(placeholder, ""), ...values.map((value) => new Option(value, value))];
      select.replaceChildren(...options);
    }

    function updateAvailableLanguages() {
      const languages = [...new Set(paperHistory
        .filter((paper) => paper.board.toLocaleLowerCase() === boardSelect.value.toLocaleLowerCase())
        .map((paper) => paper.language))]
        .sort((a, b) => a.localeCompare(b));
      selectOptions(languageSelect, languages, languages.length ? "Choose language" : "No languages");
      languageSelect.disabled = languages.length === 0;
      languageSelect.value = languages[0] || "";
      runForecast.disabled = languages.length === 0;
    }

    boardSelect.addEventListener("change", updateAvailableLanguages);
    paperHistoryInput.addEventListener("change", async () => {
      const file = paperHistoryInput.files && paperHistoryInput.files[0];
      paperHistory = [];
      selectOptions(boardSelect, [], "Import history first");
      boardSelect.disabled = true;
      selectOptions(languageSelect, [], "Choose exam series first");
      languageSelect.disabled = true;
      runForecast.disabled = true;
      forecastResults.hidden = true;
      forecastStatus.textContent = "";
      if (!file) {
        historyStatus.textContent = "No paper history loaded. Your file is read locally and is not sent to Saem.";
        return;
      }
      if (file.size > 1_000_000) {
        historyStatus.textContent = "The paper-history file is too large (maximum 1 MB).";
        return;
      }
      try {
        const parsed = JSON.parse(await file.text());
        paperHistory = studio.parsePaperHistory(parsed);
        const boards = [...new Set(paperHistory.map((paper) => paper.board))].sort((a, b) => a.localeCompare(b));
        selectOptions(boardSelect, boards, "Choose exam series");
        boardSelect.disabled = false;
        boardSelect.value = boards[0];
        updateAvailableLanguages();
        historyStatus.textContent = `${paperHistory.length} cited paper records loaded locally. Teacher-review labels are user supplied and not independently verified.`;
      } catch (error) {
        paperHistory = [];
        historyStatus.textContent = `Could not load paper history: ${error.message}`;
        console.warn("Could not load local exam paper history:", error);
      }
    });

    runForecast.addEventListener("click", () => {
      forecastResults.hidden = true;
      forecastStatus.textContent = "";
      try {
        const outlook = studio.calculateTopicOutlook(
          paperHistory,
          boardSelect.value,
          languageSelect.value,
          Number(targetYear.value)
        );
        const summary = document.createElement("p");
        summary.textContent = `${outlook.board} · ${outlook.language} · ${outlook.paperCount} reviewed papers · ${outlook.years[0]}–${outlook.years[outlook.years.length - 1]} · target ${outlook.targetYear}. Only earlier papers were counted.`;
        const list = document.createElement("ol");
        list.className = "outlook-topic-list";
        outlook.topics.forEach((topic) => {
          const item = document.createElement("li");
          const label = document.createElement("span");
          label.textContent = topic.topic;
          const rate = document.createElement("strong");
          rate.textContent = `${(topic.probability * 100).toFixed(1)}%`;
          const evidence = document.createElement("small");
          evidence.textContent = `Tagged in ${topic.appearances} of ${topic.papers} papers`;
          item.append(label, rate, evidence);
          list.append(item);
        });
        const sources = document.createElement("details");
        const sourceSummary = document.createElement("summary");
        sourceSummary.textContent = "Paper sources used";
        const sourceList = document.createElement("ul");
        paperHistory
          .filter((paper) =>
            paper.board.toLocaleLowerCase() === outlook.board.toLocaleLowerCase() &&
            paper.language.toLocaleLowerCase() === outlook.language.toLocaleLowerCase() &&
            paper.year < outlook.targetYear
          )
          .sort((a, b) => a.year - b.year || a.paperId.localeCompare(b.paperId))
          .forEach((paper) => {
            const item = document.createElement("li");
            item.textContent = `${paper.year} · ${paper.paperId} · ${paper.source}`;
            sourceList.append(item);
          });
        sources.append(sourceSummary, sourceList);
        forecastResults.replaceChildren(summary, list, sources);
        forecastResults.hidden = false;
        forecastStatus.textContent = "Exploratory frequency only—not a calibrated exam forecast.";
      } catch (error) {
        forecastStatus.textContent = error.message;
      }
    });
  }

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

  const motionCanvas = document.getElementById("motion-canvas");
  if (motionCanvas) {
    // Keep this first explainer on 2D Canvas; the source flags costly graphics on budget phones (Saem Global EdTech, p. 59).
    const motionContext = motionCanvas.getContext("2d");
    if (motionContext) {
      const playButton = document.getElementById("motion-play");
      const timeSlider = document.getElementById("motion-time");
      const timeOutput = document.getElementById("motion-time-output");
      const accelerationSlider = document.getElementById("motion-acceleration");
      const accelerationOutput = document.getElementById("motion-acceleration-output");
      const motionStatus = document.getElementById("motion-status");
      const maxTime = Number(timeSlider.max);
      const initialVelocity = 3;
      let time = Number(timeSlider.value);
      let playing = false;
      let previousFrame = null;
      let frameId = 0;

      function drawArrow(ctx, x, y, value, color, label) {
        if (Math.abs(value) < 0.05) return;
        const direction = Math.sign(value);
        const length = Math.min(20 + Math.abs(value) * 11, 72);
        const endX = x + direction * length;
        ctx.strokeStyle = color;
        ctx.fillStyle = color;
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(x, y);
        ctx.lineTo(endX, y);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(endX, y);
        ctx.lineTo(endX - direction * 7, y - 4);
        ctx.lineTo(endX - direction * 7, y + 4);
        ctx.closePath();
        ctx.fill();
        ctx.font = "12px sans-serif";
        ctx.fillText(label, endX + direction * 8, y + 4);
      }

      function drawMotion() {
        const bounds = motionCanvas.getBoundingClientRect();
        const width = Math.max(bounds.width, 320);
        const height = Math.max(bounds.height, 240);
        const pixelRatio = window.devicePixelRatio || 1;
        const pixelWidth = Math.round(width * pixelRatio);
        const pixelHeight = Math.round(height * pixelRatio);
        if (motionCanvas.width !== pixelWidth || motionCanvas.height !== pixelHeight) {
          motionCanvas.width = pixelWidth;
          motionCanvas.height = pixelHeight;
        }
        motionContext.setTransform(pixelRatio, 0, 0, pixelRatio, 0, 0);
        motionContext.clearRect(0, 0, width, height);
        motionContext.fillStyle = "#101b19";
        motionContext.fillRect(0, 0, width, height);

        const acceleration = Number(accelerationSlider.value);
        const position = initialVelocity * time + 0.5 * acceleration * time * time;
        const velocity = initialVelocity + acceleration * time;
        const padding = 48;
        const trackWidth = width - padding * 2;
        const trackY = height * 0.36;
        const graphTop = height * 0.68;
        const graphBottom = height - 31;
        const maxPosition = 26;
        const particleX = padding + (position / maxPosition) * trackWidth;
        const graphX = (value) => padding + (value / maxTime) * trackWidth;
        const graphY = (value) => graphBottom - (value / maxPosition) * (graphBottom - graphTop);

        motionContext.strokeStyle = "rgba(245,242,233,.12)";
        motionContext.lineWidth = 1;
        for (let i = 0; i <= 4; i += 1) {
          const x = padding + (trackWidth * i) / 4;
          motionContext.beginPath();
          motionContext.moveTo(x, graphTop);
          motionContext.lineTo(x, graphBottom);
          motionContext.stroke();
        }

        motionContext.font = "11px sans-serif";
        motionContext.fillStyle = "rgba(245,242,233,.62)";
        motionContext.fillText("PARTICLE TRACK", padding, 24);
        motionContext.fillText("POSITION x (m)", padding, graphTop - 12);
        motionContext.fillText("TIME t (s)", width - padding - 64, height - 8);

        motionContext.strokeStyle = "rgba(245,242,233,.48)";
        motionContext.lineWidth = 1.5;
        motionContext.beginPath();
        motionContext.moveTo(padding, trackY);
        motionContext.lineTo(width - padding, trackY);
        motionContext.moveTo(padding, graphTop);
        motionContext.lineTo(padding, graphBottom);
        motionContext.lineTo(width - padding, graphBottom);
        motionContext.stroke();

        motionContext.strokeStyle = "#f0bf91";
        motionContext.lineWidth = 2.5;
        motionContext.beginPath();
        for (let step = 0; step <= 80; step += 1) {
          const sampleTime = (maxTime * step) / 80;
          const samplePosition = initialVelocity * sampleTime + 0.5 * acceleration * sampleTime * sampleTime;
          const x = graphX(sampleTime);
          const y = graphY(Math.max(0, Math.min(maxPosition, samplePosition)));
          if (step === 0) motionContext.moveTo(x, y);
          else motionContext.lineTo(x, y);
        }
        motionContext.stroke();

        const currentGraphX = graphX(time);
        motionContext.setLineDash([4, 5]);
        motionContext.strokeStyle = "rgba(155,216,223,.56)";
        motionContext.beginPath();
        motionContext.moveTo(currentGraphX, trackY + 13);
        motionContext.lineTo(currentGraphX, graphBottom);
        motionContext.stroke();
        motionContext.setLineDash([]);

        drawArrow(motionContext, particleX, trackY - 28, velocity, "#9bd8df", `v ${velocity.toFixed(1)}`);
        drawArrow(motionContext, particleX, trackY + 30, acceleration, "#c1d998", `a ${acceleration.toFixed(1)}`);

        motionContext.fillStyle = "#f0bf91";
        motionContext.beginPath();
        motionContext.arc(particleX, trackY, 8, 0, Math.PI * 2);
        motionContext.fill();
        motionContext.strokeStyle = "#f5f2e9";
        motionContext.lineWidth = 2;
        motionContext.stroke();

        motionContext.fillStyle = "#9bd8df";
        motionContext.beginPath();
        motionContext.arc(currentGraphX, graphY(position), 5, 0, Math.PI * 2);
        motionContext.fill();
      }

      function setTime(nextTime) {
        time = Math.max(0, Math.min(maxTime, nextTime));
        timeSlider.value = time.toFixed(2);
        timeOutput.textContent = `${time.toFixed(1)} s`;
        drawMotion();
      }

      function pauseMotion(message) {
        playing = false;
        previousFrame = null;
        if (frameId && typeof cancelAnimationFrame === "function") {
          cancelAnimationFrame(frameId);
        }
        frameId = 0;
        playButton.textContent = "Play motion";
        playButton.setAttribute("aria-pressed", "false");
        if (message) motionStatus.textContent = message;
      }

      function animate(timestamp) {
        if (!playing) return;
        if (previousFrame !== null) {
          time += Math.min(timestamp - previousFrame, 50) / 1000;
          if (time >= maxTime) {
            setTime(maxTime);
            pauseMotion("End of model. Press Play to run it again.");
            return;
          }
          setTime(time);
        }
        previousFrame = timestamp;
        frameId = requestAnimationFrame(animate);
      }

      timeSlider.addEventListener("input", () => {
        if (playing) pauseMotion("Paused. Scrub the time slider or press Play.");
        setTime(Number(timeSlider.value));
        motionStatus.textContent = "Time set with the slider.";
      });
      accelerationSlider.addEventListener("input", () => {
        const acceleration = Number(accelerationSlider.value);
        accelerationOutput.textContent = `${acceleration >= 0 ? "+" : ""}${acceleration.toFixed(1)} m/s²`;
        drawMotion();
        motionStatus.textContent = "Acceleration changed; the curve and arrows update.";
      });
      playButton.addEventListener("click", () => {
        if (playing) {
          pauseMotion("Paused. Scrub the time slider or press Play.");
          return;
        }
        if (time >= maxTime) setTime(0);
        playing = true;
        playButton.textContent = "Pause motion";
        playButton.setAttribute("aria-pressed", "true");
        motionStatus.textContent = "Playing the constant-acceleration model.";
        frameId = requestAnimationFrame(animate);
      });
      window.addEventListener("resize", drawMotion);
      accelerationOutput.textContent = `${Number(accelerationSlider.value) >= 0 ? "+" : ""}${Number(accelerationSlider.value).toFixed(1)} m/s²`;
      setTime(time);
    }
  }

  document.getElementById("current-year").textContent = String(new Date().getFullYear());
})();
