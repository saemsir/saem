const assert = require("node:assert/strict");
const {
  calculateKinematics,
  calculateCorpusOutlook,
  calculateTopicOutlook,
  examConfig,
  examQuestions,
  questions,
  parsePaperHistory,
  parseExamCorpus,
  restoreCompletedProgress,
  scoreExam,
  serializeCompletedProgress
} = require("../student-studio.js");

assert.deepEqual(calculateKinematics(0, 2, 3), { displacement: 9, velocity: 6 });
assert.deepEqual(calculateKinematics(6, -2, 2), { displacement: 8, velocity: 2 });
assert.deepEqual(calculateKinematics(3, 0, 4), { displacement: 12, velocity: 3 });
assert.throws(() => calculateKinematics(1, 2, -1), RangeError);
assert.throws(() => calculateKinematics(Number.NaN, 2, 1), RangeError);
assert.equal(restoreCompletedProgress(null), 0);
assert.equal(restoreCompletedProgress('{"version":1,"completed":2}'), 2);
assert.equal(restoreCompletedProgress(serializeCompletedProgress(3)), 3);
assert.throws(() => restoreCompletedProgress('{"version":2,"completed":1}'), TypeError);
assert.throws(() => restoreCompletedProgress('{"version":1,"completed":4}'), TypeError);
assert.throws(() => serializeCompletedProgress(-1), RangeError);

assert.equal(questions.length, 3);
for (const question of questions) {
  assert.ok(question.prompt.length > 10);
  assert.ok(question.options.length >= 2);
  assert.ok(question.answerIndex >= 0 && question.answerIndex < question.options.length);
  assert.ok(question.explanation.length > question.options[question.answerIndex].length);
}
assert.equal(questions[0].options[questions[0].answerIndex], "9 m");
assert.equal(questions[1].options[questions[1].answerIndex], "The object's velocity");
assert.equal(questions[2].options[questions[2].answerIndex], "2 m/s");

assert.equal(examQuestions.length, 8);
assert.deepEqual(examConfig, { version: 1, durationSeconds: 720, marksCorrect: 1, marksIncorrect: 0, marksUnanswered: 0 });
for (const question of examQuestions) {
  assert.ok(question.topic.length > 0);
  assert.ok(question.options.length >= 2);
  assert.ok(question.answerIndex >= 0 && question.answerIndex < question.options.length);
  assert.ok(question.explanation.length > question.options[question.answerIndex].length);
}
const fullScore = scoreExam(examQuestions.map((question) => question.answerIndex));
assert.equal(fullScore.correct, examQuestions.length);
assert.equal(fullScore.total, examQuestions.length);
assert.equal(fullScore.score, 8);
assert.equal(fullScore.maximumScore, 8);
assert.ok(fullScore.topicScores.every((topic) => topic.correct === topic.total));
const partialScore = scoreExam(examQuestions.map((question, index) => index === 0 ? question.answerIndex : null));
assert.equal(partialScore.correct, 1);
assert.equal(partialScore.incorrect, 0);
assert.equal(partialScore.unanswered, examQuestions.length - 1);
assert.throws(() => scoreExam([]), TypeError);
assert.throws(() => scoreExam(examQuestions.map(() => 99)), RangeError);

const papers = parsePaperHistory([
  { board: "JEE Main", paperId: "2020-A", year: 2020, language: "English", source: "Official paper 2020-A", reviewed: true, topics: ["Mechanics", "Optics"] },
  { board: "JEE Main", paperId: "2021-A", year: 2021, language: "English", source: "Official paper 2021-A", reviewed: true, topics: ["Mechanics"] },
  { board: "JEE Main", paperId: "2022-A", year: 2022, language: "English", source: "Official paper 2022-A", reviewed: true, topics: ["Mechanics", "Optics"] },
  { board: "JEE Main", paperId: "2023-A", year: 2023, language: "English", source: "Official paper 2023-A", reviewed: true, topics: ["Mechanics", "Waves"] },
  { board: "JEE Main", paperId: "2024-A", year: 2024, language: "English", source: "Official paper 2024-A", reviewed: true, topics: ["Optics"] },
  { board: "JEE Main", paperId: "2025-A", year: 2025, language: "English", source: "Official paper 2025-A", reviewed: true, topics: ["Mechanics"] },
  { board: "NEET", paperId: "2024-B", year: 2024, language: "Hindi", source: "Official paper 2024-B", reviewed: true, topics: ["Electricity"] }
]);
const outlook = calculateTopicOutlook(papers, "JEE Main", "English", 2025);
assert.equal(outlook.paperCount, 5);
assert.deepEqual(outlook.years, [2020, 2021, 2022, 2023, 2024]);
assert.equal(outlook.topics.find((item) => item.topic === "Mechanics").probability, 5 / 7);
assert.equal(outlook.topics.find((item) => item.topic === "Optics").appearances, 3);
assert.ok(outlook.topics.every((item) => item.probability > 0 && item.probability < 1));
assert.equal(outlook.topics.find((item) => item.topic === "Mechanics").appearances, 4);
assert.throws(() => calculateTopicOutlook(papers, "JEE Main", "English", 2024), RangeError);
assert.throws(() => calculateTopicOutlook(papers, "NEET", "Hindi", 2025), RangeError);
assert.throws(() => parsePaperHistory([{ ...papers[0], reviewed: false }]), TypeError);
assert.throws(() => parsePaperHistory([papers[0], papers[0]]), TypeError);
assert.equal(parsePaperHistory([papers[0], { ...papers[0], language: "Hindi" }]).length, 2);
assert.throws(() => parsePaperHistory(Array(501).fill(papers[0])), RangeError);

const corpus = parseExamCorpus({
  corpusVersion: 1,
  indexMethodVersion: 1,
  papers: Array.from({ length: 8 }, (_, index) => ({
    id: `jee-${index}`,
    examSeries: "jee_main",
    sourceName: `source-${index}.pdf`,
    sha256: "a".repeat(64),
    ownerVerificationStatus: index === 6 ? "not_reviewed" : "owner_verified",
    year: index === 7 ? 2024 : index === 6 ? 2023 : 2020 + Math.floor(index / 2),
    yearSource: "title_text_candidate",
    pages: 10,
    searchableCharacters: 1200,
    extractionStatus: index === 5 ? "partial" : "text_extracted",
    topicReviewStatus: "machine_candidates_unreviewed",
    topicSignals: index === 5
      ? [{ subject: "physics", label: "Partial-only", keywordMatches: 4 }]
      : index === 6
      ? [{ subject: "physics", label: "Unreviewed-only", keywordMatches: 5 }]
      : index === 7
      ? [{ subject: "physics", label: "Future-only", keywordMatches: 9 }]
      : index < 5
        ? [{ subject: "physics", label: "Mechanics", keywordMatches: 3 }]
      : []
  }))
});
const corpusOutlook = calculateCorpusOutlook(corpus, "jee_main", 2024);
assert.equal(corpusOutlook.eligiblePaperFiles, 6);
assert.equal(corpusOutlook.unreviewedPaperFiles, 1);
assert.equal(corpusOutlook.analyzedPaperFiles, 5);
assert.deepEqual(corpusOutlook.years, [2020, 2021, 2022]);
assert.equal(corpusOutlook.topics[0].appearances, 5);
assert.equal(corpusOutlook.topics[0].historicalDetectionRate, 1);
assert.ok(!corpusOutlook.topics.some((topic) => topic.topic === "Partial-only"));
assert.ok(!corpusOutlook.topics.some((topic) => topic.topic === "Unreviewed-only"));
assert.ok(!corpusOutlook.topics.some((topic) => topic.topic === "Future-only"));
assert.throws(() => calculateCorpusOutlook(corpus, "jee_main", 2021), RangeError);
assert.throws(() => parseExamCorpus({
  corpusVersion: 1,
  papers: [{ ...corpus[0], year: 2024, yearSource: "invented" }]
}), TypeError);
assert.throws(() => parseExamCorpus({
  corpusVersion: 1,
  papers: [corpus[0], corpus[0]]
}), TypeError);

console.log("Student studio: practice, timed-test scoring, and past-paper outlook pass");
