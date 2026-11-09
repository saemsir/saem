const assert = require("node:assert/strict");
const {
  calculateKinematics,
  calculateCorpusOutlook,
  calculateTopicOutlook,
  examConfig,
  examQuestions,
  questions,
  parsePaperAnswerKey,
  parsePaperHistory,
  parseExamCorpus,
  restoreCompletedProgress,
  scoreExam,
  scorePaperResponses,
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
assert.equal(outlook.topics.find((item) => item.topic === "Mechanics").rawProbability, 5 / 7);
assert.equal(outlook.topics.find((item) => item.topic === "Mechanics").probability, null);
assert.equal(outlook.topics.find((item) => item.topic === "Optics").appearances, 3);
assert.ok(outlook.topics.every((item) => item.probability === null));
assert.equal(outlook.topics.find((item) => item.topic === "Mechanics").appearances, 4);
assert.equal(outlook.calibration.status, "not_validated");
assert.throws(() => calculateTopicOutlook(papers, "JEE Main", "English", 2024), RangeError);
assert.throws(() => calculateTopicOutlook(papers, "NEET", "Hindi", 2025), RangeError);
assert.throws(() => parsePaperHistory([{ ...papers[0], reviewed: false }]), TypeError);
assert.throws(() => parsePaperHistory([papers[0], papers[0]]), TypeError);
assert.equal(parsePaperHistory([papers[0], { ...papers[0], language: "Hindi" }]).length, 2);
assert.throws(() => parsePaperHistory(Array(501).fill(papers[0])), RangeError);

let randomSeed = 17;
function nextRandom() {
  randomSeed = (randomSeed * 48271) % 2147483647;
  return randomSeed / 2147483647;
}
const calibrationHistory = [];
const topicRates = [
  ["Mechanics", 0.8],
  ["Optics", 0.5],
  ["Thermal physics", 0.3],
  ["Probability", 0.1]
];
for (let year = 1980; year < 2015; year += 1) {
  for (let session = 0; session < 4; session += 1) {
    let topics = topicRates
      .filter(([, rate]) => nextRandom() < rate)
      .map(([topic]) => topic);
    if (topics.length === 0) topics = ["Mechanics"];
    calibrationHistory.push({
      board: "JEE Main",
      paperId: `${year}-${session}`,
      year,
      language: "English",
      source: `Reviewed source ${year}-${session}`,
      reviewed: true,
      topics
    });
  }
}
const calibratedOutlook = calculateTopicOutlook(
  parsePaperHistory(calibrationHistory),
  "JEE Main",
  "English",
  2015
);
assert.equal(calibratedOutlook.calibration.status, "validated");
assert.ok(calibratedOutlook.calibration.validationYears >= 3);
assert.ok(calibratedOutlook.calibration.validationSamples >= 30);
assert.ok(calibratedOutlook.calibration.calibratedBrier <
  calibratedOutlook.calibration.baselineBrier);
assert.ok(calibratedOutlook.topics.every((item) =>
  Number.isFinite(item.probability) && item.probability > 0 && item.probability < 1
));

const sourceSha256 = "b".repeat(64);
const answerKey = parsePaperAnswerKey({
  schemaVersion: 1,
  paperId: "neet-2024-paper-1",
  examSeries: "neet",
  year: 2024,
  source: "Teacher-checked paper section",
  sourceSha256,
  teacherReviewed: true,
  reviewedBy: "Test reviewer",
  marking: { correct: 4, incorrect: -1, unanswered: 0 },
  questions: [
    { number: 1, answerType: "choice", answer: "B", solution: "Apply the conservation law and compare the two states." },
    { number: 2, answerType: "numeric", answer: 2.5, tolerance: 0.01, solution: "Substitute in the relation, then check the units." },
    {
      number: 3,
      answerType: "choice",
      answer: "C",
      solution: "Use the definition to eliminate the distractor.",
      marking: { correct: 5, incorrect: -2, unanswered: 0 }
    }
  ]
});
const paperScore = scorePaperResponses(["b", "2.505", ""], answerKey);
assert.deepEqual(
  {
    correct: paperScore.correct,
    incorrect: paperScore.incorrect,
    unanswered: paperScore.unanswered,
    score: paperScore.score,
    maximumScore: paperScore.maximumScore
  },
  { correct: 2, incorrect: 0, unanswered: 1, score: 8, maximumScore: 13 }
);
const negativeScore = scorePaperResponses(["A", "1.5", "C"], answerKey);
assert.equal(negativeScore.incorrect, 2);
assert.equal(negativeScore.score, 3);
assert.throws(() => parsePaperAnswerKey({
  schemaVersion: 1,
  paperId: "bad",
  examSeries: "neet",
  year: 2024,
  source: "citation",
  sourceSha256: "wrong",
  teacherReviewed: true,
  reviewedBy: "Reviewer",
  marking: { correct: 4, incorrect: -1, unanswered: 0 },
  questions: []
}), TypeError);
assert.throws(() => scorePaperResponses([], answerKey), TypeError);

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
