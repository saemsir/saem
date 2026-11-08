(function (root, factory) {
  const studentStudio = factory();
  if (typeof module === "object" && module.exports) module.exports = studentStudio;
  if (root) root.SaemStudentStudio = studentStudio;
})(typeof window === "undefined" ? null : window, function () {
  "use strict";

  const questions = Object.freeze([
    Object.freeze({
      prompt: "An object starts from rest and accelerates at 2 m/s² for 3 s. How far does it travel?",
      options: Object.freeze(["6 m", "9 m", "12 m"]),
      answerIndex: 1,
      explanation: "Use x = ut + ½at². Here u = 0, so x = 0 + ½ × 2 × 3² = 9 m."
    }),
    Object.freeze({
      prompt: "On a position-versus-time graph, what does the slope at one instant tell you?",
      options: Object.freeze(["The object's velocity", "The object's acceleration", "The force on the object"]),
      answerIndex: 0,
      explanation: "Velocity is the rate of change of position: v = dx/dt. On an x–t graph, that is the tangent's slope."
    }),
    Object.freeze({
      prompt: "A runner has velocity 6 m/s. An acceleration of −2 m/s² acts for 2 s. What is the final velocity?",
      options: Object.freeze(["2 m/s", "10 m/s", "−4 m/s"]),
      answerIndex: 0,
      explanation: "Use v = u + at: v = 6 + (−2 × 2) = 2 m/s. Negative acceleration does not instantly mean negative velocity."
    })
  ]);

  const examQuestions = Object.freeze([
    Object.freeze({
      topic: "Kinematics",
      prompt: "A car starts from rest and accelerates uniformly at 3 m/s² for 4 s. What is its final speed?",
      options: Object.freeze(["7 m/s", "12 m/s", "24 m/s"]),
      answerIndex: 1,
      explanation: "Use v = u + at. With u = 0, v = 0 + 3 × 4 = 12 m/s."
    }),
    Object.freeze({
      topic: "Newton's laws",
      prompt: "A book rests on a table. Which statement about the forces on it is correct?",
      options: Object.freeze(["There are no forces on the book", "The upward normal force balances its weight", "The normal force is larger than its weight"]),
      answerIndex: 1,
      explanation: "The book is at rest, so its net force is zero. The table's upward normal force balances the book's downward weight."
    }),
    Object.freeze({
      topic: "Graphs of motion",
      prompt: "What does the slope of a velocity-versus-time graph represent?",
      options: Object.freeze(["Displacement", "Acceleration", "Momentum"]),
      answerIndex: 1,
      explanation: "The slope is change in velocity divided by change in time, which is acceleration."
    }),
    Object.freeze({
      topic: "Work and energy",
      prompt: "A 5 N force moves a box 4 m in the force's direction. How much work is done?",
      options: Object.freeze(["1.25 J", "9 J", "20 J"]),
      answerIndex: 2,
      explanation: "For a constant force parallel to the displacement, W = Fd = 5 × 4 = 20 J."
    }),
    Object.freeze({
      topic: "Work and energy",
      prompt: "If an object's speed doubles, what happens to its kinetic energy?",
      options: Object.freeze(["It doubles", "It becomes four times as large", "It is unchanged"]),
      answerIndex: 1,
      explanation: "Kinetic energy is K = ½mv². Doubling v multiplies v², and therefore K, by four."
    }),
    Object.freeze({
      topic: "Waves",
      prompt: "A wave travels at the same speed, but its frequency doubles. What happens to its wavelength?",
      options: Object.freeze(["It doubles", "It halves", "It stays the same"]),
      answerIndex: 1,
      explanation: "The wave relation is v = fλ. At fixed speed, doubling frequency halves the wavelength."
    }),
    Object.freeze({
      topic: "Electricity",
      prompt: "A 6 Ω resistor is connected across a 12 V ideal source. What current flows?",
      options: Object.freeze(["0.5 A", "2 A", "72 A"]),
      answerIndex: 1,
      explanation: "Ohm's law gives I = V/R = 12/6 = 2 A."
    }),
    Object.freeze({
      topic: "Ray optics",
      prompt: "A ray enters glass from air and bends toward the normal. Which is true?",
      options: Object.freeze(["Light travels more slowly in the glass", "Light travels faster in the glass", "Its frequency must increase"]),
      answerIndex: 0,
      explanation: "Bending toward the normal on entering glass indicates a higher refractive index and lower light speed. The frequency does not change at the boundary."
    })
  ]);

  const examConfig = Object.freeze({
    version: 1,
    durationSeconds: 12 * 60,
    marksCorrect: 1,
    marksIncorrect: 0,
    marksUnanswered: 0
  });

  const MAX_PAPER_RECORDS = 500;
  const MAX_SOURCE_LENGTH = 500;

  function calculateKinematics(initialVelocity, acceleration, time) {
    if (![initialVelocity, acceleration, time].every(Number.isFinite) || time < 0) {
      throw new RangeError("Kinematics inputs must be finite, and time cannot be negative.");
    }
    return Object.freeze({
      displacement: initialVelocity * time + 0.5 * acceleration * time * time,
      velocity: initialVelocity + acceleration * time
    });
  }

  function restoreCompletedProgress(saved) {
    if (saved === null) return 0;
    const state = JSON.parse(saved);
    if (!state || state.version !== 1 || !Number.isInteger(state.completed) ||
        state.completed < 0 || state.completed > questions.length) {
      throw new TypeError("Saved practice progress has an unsupported format.");
    }
    return state.completed;
  }

  function serializeCompletedProgress(completed) {
    if (!Number.isInteger(completed) || completed < 0 || completed > questions.length) {
      throw new RangeError("Completed question count is outside the practice range.");
    }
    return JSON.stringify({ version: 1, completed });
  }

  function scoreExam(answers) {
    if (!Array.isArray(answers) || answers.length !== examQuestions.length) {
      throw new TypeError("Exam answers must contain one entry per question.");
    }

    const topicScores = new Map();
    let correct = 0;
    let incorrect = 0;
    let unanswered = 0;
    answers.forEach((answer, index) => {
      const question = examQuestions[index];
      if (answer !== null && (!Number.isInteger(answer) || answer < 0 || answer >= question.options.length)) {
        throw new RangeError(`Exam answer ${index + 1} is outside its option range.`);
      }
      const topicScore = topicScores.get(question.topic) || { topic: question.topic, correct: 0, total: 0 };
      topicScore.total += 1;
      if (answer === question.answerIndex) {
        correct += 1;
        topicScore.correct += 1;
      } else if (answer === null) {
        unanswered += 1;
      } else {
        incorrect += 1;
      }
      topicScores.set(question.topic, topicScore);
    });

    return Object.freeze({
      correct,
      incorrect,
      unanswered,
      total: examQuestions.length,
      score: correct * examConfig.marksCorrect +
        incorrect * examConfig.marksIncorrect +
        unanswered * examConfig.marksUnanswered,
      maximumScore: examQuestions.length * examConfig.marksCorrect,
      topicScores: Object.freeze([...topicScores.values()].map((item) => Object.freeze(item)))
    });
  }

  function parsePaperHistory(records) {
    if (!Array.isArray(records) || records.length < 1 || records.length > MAX_PAPER_RECORDS) {
      throw new RangeError(`Paper history must contain between 1 and ${MAX_PAPER_RECORDS} records.`);
    }

    const identifiers = new Set();
    return Object.freeze(records.map((record, index) => {
      if (!record || typeof record !== "object" || Array.isArray(record)) {
        throw new TypeError(`Paper record ${index + 1} must be an object.`);
      }
      const board = typeof record.board === "string" ? record.board.trim() : "";
      const paperId = typeof record.paperId === "string" ? record.paperId.trim() : "";
      const language = typeof record.language === "string" ? record.language.trim() : "";
      const source = typeof record.source === "string" ? record.source.trim() : "";
      if (!board || board.length > 100 || !paperId || paperId.length > 100 ||
          !language || language.length > 40 ||
          !source || source.length > MAX_SOURCE_LENGTH) {
        throw new TypeError(`Paper record ${index + 1} needs a board, paperId, language, and source citation.`);
      }
      if (!Number.isInteger(record.year) || record.year < 1900 || record.year > 2100) {
        throw new RangeError(`Paper record ${index + 1} has an invalid year.`);
      }
      if (record.reviewed !== true) {
        throw new TypeError(`Paper record ${index + 1} must be teacher-reviewed before use.`);
      }
      if (!Array.isArray(record.topics) || record.topics.length < 1 || record.topics.length > 30) {
        throw new TypeError(`Paper record ${index + 1} needs between 1 and 30 topic labels.`);
      }

      const identity = `${board.toLocaleLowerCase()}\u0000${language.toLocaleLowerCase()}\u0000${paperId.toLocaleLowerCase()}`;
      if (identifiers.has(identity)) throw new TypeError(`Duplicate paperId in record ${index + 1}.`);
      identifiers.add(identity);

      const topicKeys = new Set();
      const topics = record.topics.map((topic) => {
        if (typeof topic !== "string" || !topic.trim() || topic.trim().length > 80) {
          throw new TypeError(`Paper record ${index + 1} contains an invalid topic label.`);
        }
        const label = topic.trim();
        const key = label.toLocaleLowerCase();
        if (topicKeys.has(key)) throw new TypeError(`Paper record ${index + 1} repeats a topic label.`);
        topicKeys.add(key);
        return label;
      });

      return Object.freeze({
        board,
        paperId,
        year: record.year,
        language,
        source,
        reviewed: true,
        topics: Object.freeze(topics)
      });
    }));
  }

  function calculateTopicOutlook(papers, board, language, targetYear) {
    const selectedBoard = typeof board === "string" ? board.trim() : "";
    if (!selectedBoard) throw new TypeError("Choose an exam series from the imported papers.");
    const selectedLanguage = typeof language === "string" ? language.trim() : "";
    if (!selectedLanguage) throw new TypeError("Choose a language from the imported papers.");
    if (!Number.isInteger(targetYear) || targetYear < 1900 || targetYear > 2100) {
      throw new RangeError("Target year must be a whole year between 1900 and 2100.");
    }

    const eligible = papers.filter((paper) =>
      paper.board.toLocaleLowerCase() === selectedBoard.toLocaleLowerCase() &&
      paper.language.toLocaleLowerCase() === selectedLanguage.toLocaleLowerCase() &&
      paper.reviewed && paper.year < targetYear
    );
    const years = [...new Set(eligible.map((paper) => paper.year))].sort((a, b) => a - b);
    if (eligible.length < 5 || years.length < 3) {
      throw new RangeError("At least five reviewed papers across three earlier years are required for an outlook.");
    }

    const topicCounts = new Map();
    for (const paper of eligible) {
      for (const label of paper.topics) {
        const key = label.toLocaleLowerCase();
        const item = topicCounts.get(key) || { topic: label, appearances: 0 };
        item.appearances += 1;
        topicCounts.set(key, item);
      }
    }

    const topics = [...topicCounts.values()]
      .map((item) => Object.freeze({
        topic: item.topic,
        appearances: item.appearances,
        papers: eligible.length,
        probability: (item.appearances + 1) / (eligible.length + 2)
      }))
      .sort((left, right) => right.probability - left.probability || left.topic.localeCompare(right.topic));

    return Object.freeze({
      board: selectedBoard,
      language: selectedLanguage,
      targetYear,
      paperCount: eligible.length,
      years: Object.freeze(years),
      topics: Object.freeze(topics)
    });
  }

  return Object.freeze({
    questions,
    examConfig,
    examQuestions,
    calculateTopicOutlook,
    calculateKinematics,
    parsePaperHistory,
    restoreCompletedProgress,
    scoreExam,
    serializeCompletedProgress
  });
});
