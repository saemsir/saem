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
  const MAX_CORPUS_PAPERS = 10000;
  const MAX_SOURCE_LENGTH = 500;
  const EXAM_SERIES = Object.freeze(["neet", "jee_main", "jee_advanced"]);

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
    const yearlyTopics = new Map();
    for (const paper of eligible) {
      if (!yearlyTopics.has(paper.year)) yearlyTopics.set(paper.year, new Set());
      for (const topic of paper.topics) yearlyTopics.get(paper.year).add(topic.toLocaleLowerCase());
    }
    const years = [...yearlyTopics.keys()].sort((a, b) => a - b);
    if (eligible.length < 5 || years.length < 3) {
      throw new RangeError("At least five reviewed papers across three earlier years are required for an outlook.");
    }

    const topicLabels = new Map();
    for (const paper of eligible) {
      for (const label of paper.topics) {
        const key = label.toLocaleLowerCase();
        if (!topicLabels.has(key)) topicLabels.set(key, label);
      }
    }

    const outOfFoldSamples = [];
    for (let holdoutIndex = 3; holdoutIndex < years.length; holdoutIndex += 1) {
      const trainingYears = years.slice(0, holdoutIndex);
      const trainingPapers = eligible.filter((paper) => paper.year < years[holdoutIndex]);
      if (trainingPapers.length < 5) continue;
      const trainingTopics = new Set(
        trainingPapers.flatMap((paper) => paper.topics.map((topic) => topic.toLocaleLowerCase()))
      );
      for (const topic of trainingTopics) {
        const presentYears = trainingYears.filter((year) => yearlyTopics.get(year).has(topic)).length;
        outOfFoldSamples.push({
          targetYear: years[holdoutIndex],
          rawProbability: (presentYears + 1) / (trainingYears.length + 2),
          observed: yearlyTopics.get(years[holdoutIndex]).has(topic)
        });
      }
    }

    const oofYears = [...new Set(outOfFoldSamples.map((sample) => sample.targetYear))];
    const validationSamples = [];
    for (const validationYear of oofYears) {
      const calibrationTraining = outOfFoldSamples.filter((sample) => sample.targetYear < validationYear);
      const calibrationYears = new Set(calibrationTraining.map((sample) => sample.targetYear));
      const holdout = outOfFoldSamples.filter((sample) => sample.targetYear === validationYear);
      if (calibrationTraining.length < 30 || calibrationYears.size < 3) continue;
      const calibrator = fitProbabilityCalibrator(calibrationTraining);
      if (!calibrator) continue;
      const baselineRate =
        calibrationTraining.reduce((sum, sample) => sum + Number(sample.observed), 0) /
        calibrationTraining.length;
      holdout.forEach((sample) => validationSamples.push({
        ...sample,
        calibratedProbability: calibrateProbability(sample.rawProbability, calibrator),
        baselineProbability: baselineRate
      }));
    }

    const validationYears = [...new Set(validationSamples.map((sample) => sample.targetYear))];
    const finalCalibrator = fitProbabilityCalibrator(outOfFoldSamples);
    const calibratedBrier = brierScore(validationSamples, (sample) => sample.calibratedProbability);
    const rawBrier = brierScore(validationSamples, (sample) => sample.rawProbability);
    const baselineBrier = brierScore(validationSamples, (sample) => sample.baselineProbability);
    const expectedCalibrationError = calibrationError(
      validationSamples,
      (sample) => sample.calibratedProbability
    );
    const calibrated =
      finalCalibrator !== null &&
      validationSamples.length >= 30 &&
      validationYears.length >= 3 &&
      calibratedBrier < baselineBrier &&
      expectedCalibrationError <= 0.1;
    const calibration = Object.freeze({
      status: calibrated ? "validated" : "not_validated",
      outOfFoldYears: oofYears.length,
      outOfFoldSamples: outOfFoldSamples.length,
      validationYears: validationYears.length,
      validationSamples: validationSamples.length,
      calibratedBrier,
      rawBrier,
      baselineBrier,
      expectedCalibrationError
    });

    const topics = [...topicLabels.entries()]
      .map(([key, topic]) => {
        const appearances = years.filter((year) => yearlyTopics.get(year).has(key)).length;
        const rawProbability = (appearances + 1) / (years.length + 2);
        return Object.freeze({
          topic,
          appearances,
          papers: eligible.length,
          yearsObserved: years.length,
          rawProbability,
          probability: calibrated
            ? calibrateProbability(rawProbability, finalCalibrator)
            : null
        });
      })
      .sort((left, right) =>
        (right.probability ?? right.rawProbability) - (left.probability ?? left.rawProbability) ||
        left.topic.localeCompare(right.topic)
      );

    return Object.freeze({
      board: selectedBoard,
      language: selectedLanguage,
      targetYear,
      paperCount: eligible.length,
      years: Object.freeze(years),
      calibration,
      topics: Object.freeze(topics)
    });
  }

  function fitProbabilityCalibrator(samples) {
    if (samples.length < 30 || !samples.some((sample) => sample.observed) ||
        !samples.some((sample) => !sample.observed)) return null;

    let intercept = 0;
    let slope = 1;
    for (let iteration = 0; iteration < 80; iteration += 1) {
      let gradientIntercept = 0;
      let gradientSlope = 0;
      let hessianIntercept = 1e-6;
      let hessianCross = 0;
      let hessianSlope = 1e-3;
      for (const sample of samples) {
        const logit = Math.log(sample.rawProbability / (1 - sample.rawProbability));
        const linear = Math.max(-30, Math.min(30, intercept + slope * logit));
        const prediction = 1 / (1 + Math.exp(-linear));
        const residual = prediction - Number(sample.observed);
        const weight = prediction * (1 - prediction);
        gradientIntercept += residual;
        gradientSlope += residual * logit;
        hessianIntercept += weight;
        hessianCross += weight * logit;
        hessianSlope += weight * logit * logit;
      }
      gradientSlope += 1e-3 * (slope - 1);
      const determinant = hessianIntercept * hessianSlope - hessianCross * hessianCross;
      if (determinant < 1e-12) return null;
      const interceptStep =
        (hessianSlope * gradientIntercept - hessianCross * gradientSlope) / determinant;
      const slopeStep =
        (hessianIntercept * gradientSlope - hessianCross * gradientIntercept) / determinant;
      const nextIntercept = Math.max(-8, Math.min(8, intercept - interceptStep));
      const nextSlope = Math.max(0, Math.min(8, slope - slopeStep));
      if (Math.abs(nextIntercept - intercept) + Math.abs(nextSlope - slope) < 1e-7) {
        intercept = nextIntercept;
        slope = nextSlope;
        break;
      }
      intercept = nextIntercept;
      slope = nextSlope;
    }
    return Object.freeze({ intercept, slope });
  }

  function calibrateProbability(probability, calibrator) {
    const safeProbability = Math.max(1e-6, Math.min(1 - 1e-6, probability));
    const logit = Math.log(safeProbability / (1 - safeProbability));
    const linear = Math.max(-30, Math.min(30, calibrator.intercept + calibrator.slope * logit));
    return 1 / (1 + Math.exp(-linear));
  }

  function brierScore(samples, probabilityFor) {
    if (samples.length === 0) return 1;
    return samples.reduce((sum, sample) => {
      const difference = probabilityFor(sample) - Number(sample.observed);
      return sum + difference * difference;
    }, 0) / samples.length;
  }

  function calibrationError(samples, probabilityFor) {
    if (samples.length === 0) return 1;
    const sorted = [...samples].sort((left, right) =>
      probabilityFor(left) - probabilityFor(right)
    );
    const binCount = Math.min(5, sorted.length);
    let weightedError = 0;
    for (let bin = 0; bin < binCount; bin += 1) {
      const start = Math.floor(bin * sorted.length / binCount);
      const end = Math.floor((bin + 1) * sorted.length / binCount);
      const members = sorted.slice(start, end);
      if (members.length === 0) continue;
      const predicted = members.reduce((sum, sample) => sum + probabilityFor(sample), 0) / members.length;
      const observed = members.reduce((sum, sample) => sum + Number(sample.observed), 0) / members.length;
      weightedError += members.length * Math.abs(predicted - observed);
    }
    return weightedError / sorted.length;
  }

  function parsePaperAnswerKey(data) {
    if (!data || typeof data !== "object" || Array.isArray(data) ||
        data.schemaVersion !== 1 || typeof data.paperId !== "string" ||
        !data.paperId.trim() || !EXAM_SERIES.includes(data.examSeries) ||
        !Number.isInteger(data.year) || data.year < 1900 || data.year > 2100 ||
        typeof data.source !== "string" || !data.source.trim() ||
        !/^[a-f0-9]{64}$/.test(data.sourceSha256) ||
        data.teacherReviewed !== true ||
        typeof data.reviewedBy !== "string" || !data.reviewedBy.trim() ||
        !Array.isArray(data.questions) || data.questions.length < 1 ||
        data.questions.length > 300) {
      throw new TypeError("The local answer key is missing a valid paper identity, source hash, review record, or question list.");
    }

    const marking = data.marking;
    const parseMarking = (value, label) => {
      if (!value || typeof value !== "object" ||
          ![value.correct, value.incorrect, value.unanswered].every(Number.isFinite) ||
          value.correct <= 0 || value.correct > 100 ||
          value.incorrect < -100 || value.incorrect > 100 ||
          value.unanswered < -100 || value.unanswered > 100) {
        throw new RangeError(`${label} needs finite marking values within the supported range.`);
      }
      return Object.freeze({
        correct: value.correct,
        incorrect: value.incorrect,
        unanswered: value.unanswered
      });
    };
    const defaultMarking = parseMarking(marking, "The answer key");

    const questionNumbers = new Set();
    const questions = data.questions.map((question, index) => {
      if (!question || typeof question !== "object" || Array.isArray(question) ||
          !Number.isInteger(question.number) || question.number < 1 ||
          question.number > data.questions.length || questionNumbers.has(question.number) ||
          !["choice", "numeric"].includes(question.answerType) ||
          typeof question.solution !== "string" || question.solution.trim().length < 10 ||
          question.solution.length > 4000) {
        throw new TypeError(`Answer-key question ${index + 1} has an invalid number, answer type, or worked solution.`);
      }
      questionNumbers.add(question.number);
      if (question.answerType === "choice" &&
          (typeof question.answer !== "string" || !question.answer.trim() || question.answer.length > 40)) {
        throw new TypeError(`Answer-key question ${question.number} needs a choice answer.`);
      }
      if (question.answerType === "numeric" &&
          (!Number.isFinite(question.answer) || !Number.isFinite(question.tolerance ?? 0) ||
           (question.tolerance ?? 0) < 0 || (question.tolerance ?? 0) > 1e6)) {
        throw new TypeError(`Answer-key question ${question.number} needs a numeric answer and non-negative tolerance.`);
      }
      const questionMarking = question.marking === undefined
        ? defaultMarking
        : parseMarking(question.marking, `Question ${question.number}`);
      return Object.freeze({
        number: question.number,
        answerType: question.answerType,
        answer: question.answerType === "choice" ? question.answer.trim() : question.answer,
        tolerance: question.tolerance ?? 0,
        solution: question.solution.trim(),
        marking: questionMarking
      });
    });
    if (questionNumbers.size !== questions.length ||
        questions.some((question, index) => question.number !== index + 1)) {
      throw new TypeError("Answer-key question numbers must be a complete sequence from 1 to the paper's question count.");
    }

    return Object.freeze({
      schemaVersion: 1,
      paperId: data.paperId.trim(),
      examSeries: data.examSeries,
      year: data.year,
      source: data.source.trim(),
      sourceSha256: data.sourceSha256,
      teacherReviewed: true,
      reviewedBy: data.reviewedBy.trim(),
      marking: defaultMarking,
      questions: Object.freeze(questions)
    });
  }

  function scorePaperResponses(responses, answerKey) {
    if (!Array.isArray(responses) || !answerKey ||
        !Array.isArray(answerKey.questions) || responses.length !== answerKey.questions.length) {
      throw new TypeError("Paper responses must contain one entry per answer-key question.");
    }
    let correct = 0;
    let incorrect = 0;
    let unanswered = 0;
    const perQuestion = responses.map((response, index) => {
      const question = answerKey.questions[index];
      const marking = question.marking || answerKey.marking;
      const normalized = typeof response === "string" ? response.trim() : "";
      if (!normalized) {
        unanswered += 1;
        return Object.freeze({ result: "unanswered", score: marking.unanswered });
      }
      const isCorrect = question.answerType === "choice"
        ? normalized.toLocaleUpperCase() === question.answer.toLocaleUpperCase()
        : Number.isFinite(Number(normalized)) &&
          Math.abs(Number(normalized) - question.answer) <= question.tolerance;
      if (isCorrect) {
        correct += 1;
        return Object.freeze({ result: "correct", score: marking.correct });
      }
      incorrect += 1;
      return Object.freeze({ result: "incorrect", score: marking.incorrect });
    });
    return Object.freeze({
      correct,
      incorrect,
      unanswered,
      total: responses.length,
      score: perQuestion.reduce((sum, item) => sum + item.score, 0),
      maximumScore: answerKey.questions.reduce((sum, question) =>
        sum + (question.marking || answerKey.marking).correct, 0),
      perQuestion: Object.freeze(perQuestion)
    });
  }

  function parseExamCorpus(corpus) {
    if (!corpus || typeof corpus !== "object" || corpus.corpusVersion !== 1 ||
        corpus.indexMethodVersion !== 1 ||
        !Array.isArray(corpus.papers) || corpus.papers.length < 1 ||
        corpus.papers.length > MAX_CORPUS_PAPERS) {
      throw new TypeError("The bundled exam corpus has an unsupported format.");
    }
    const identifiers = new Set();
    const papers = corpus.papers.map((paper, index) => {
      if (!paper || typeof paper !== "object" || Array.isArray(paper) ||
          typeof paper.id !== "string" || !paper.id.trim() ||
          identifiers.has(paper.id) ||
          !EXAM_SERIES.includes(paper.examSeries) ||
          typeof paper.sourceName !== "string" ||
          !/^[a-f0-9]{64}$/.test(paper.sha256) ||
          !["owner_verified", "not_reviewed"].includes(paper.ownerVerificationStatus) ||
          (paper.year !== null && (!Number.isInteger(paper.year) || paper.year < 1900 || paper.year > 2100)) ||
          !["filename_hint", "title_text_candidate", "needs_mapping"].includes(paper.yearSource) ||
          !Number.isInteger(paper.pages) || paper.pages < 0 ||
          !Number.isInteger(paper.searchableCharacters) || paper.searchableCharacters < 0 ||
          !["text_extracted", "low_text", "partial"].includes(paper.extractionStatus) ||
          paper.topicReviewStatus !== "machine_candidates_unreviewed" ||
          !Array.isArray(paper.topicSignals)) {
        throw new TypeError(`The bundled exam corpus has an invalid paper record at ${index + 1}.`);
      }
      identifiers.add(paper.id);
      const topicLabels = new Set();
      const topicSignals = paper.topicSignals.map((signal) => {
        if (!signal || typeof signal !== "object" ||
            !["physics", "chemistry", "mathematics", "biology"].includes(signal.subject) ||
            typeof signal.label !== "string" || !signal.label.trim() ||
            !Number.isInteger(signal.keywordMatches) || signal.keywordMatches < 2) {
          throw new TypeError(`Paper record ${index + 1} has an invalid topic candidate.`);
        }
        const key = `${signal.subject}:${signal.label.toLocaleLowerCase()}`;
        if (topicLabels.has(key)) throw new TypeError(`Paper record ${index + 1} repeats a topic candidate.`);
        topicLabels.add(key);
        return Object.freeze({
          subject: signal.subject,
          label: signal.label,
          keywordMatches: signal.keywordMatches
        });
      });
      return Object.freeze({
        id: paper.id,
        examSeries: paper.examSeries,
        sourceName: paper.sourceName,
        sha256: paper.sha256,
        ownerVerificationStatus: paper.ownerVerificationStatus,
        year: paper.year,
        yearSource: paper.yearSource,
        pages: paper.pages,
        searchableCharacters: paper.searchableCharacters,
        extractionStatus: paper.extractionStatus,
        topicReviewStatus: paper.topicReviewStatus,
        topicSignals: Object.freeze(topicSignals)
      });
    });
    return Object.freeze(papers);
  }

  function calculateCorpusOutlook(papers, examSeries, targetYear) {
    if (!Array.isArray(papers) || papers.length < 1) {
      throw new TypeError("Load the bundled exam corpus before estimating a topic outlook.");
    }
    if (!EXAM_SERIES.includes(examSeries)) {
      throw new TypeError("Choose NEET, JEE Main, or JEE Advanced.");
    }
    if (!Number.isInteger(targetYear) || targetYear < 1900 || targetYear > 2100) {
      throw new RangeError("Target year must be a whole year between 1900 and 2100.");
    }
    const selected = papers.filter((paper) =>
      paper.examSeries === examSeries && paper.year !== null && paper.year < targetYear
    );
    const eligible = selected.filter((paper) => paper.ownerVerificationStatus === "owner_verified");
    const analyzed = eligible.filter((paper) =>
      paper.searchableCharacters >= 200 && paper.extractionStatus === "text_extracted"
    );
    const years = [...new Set(analyzed.map((paper) => paper.year))].sort((a, b) => a - b);
    if (analyzed.length < 5 || years.length < 3) {
      throw new RangeError(
        `Only ${analyzed.length} earlier text-extracted papers across ${years.length} years are available; at least five papers across three years are required.`
      );
    }

    const topicCounts = new Map();
    for (const paper of analyzed) {
      for (const signal of paper.topicSignals) {
        const key = `${signal.subject}:${signal.label.toLocaleLowerCase()}`;
        const entry = topicCounts.get(key) || {
          subject: signal.subject,
          topic: signal.label,
          appearances: 0,
          keywordMatches: 0
        };
        entry.appearances += 1;
        entry.keywordMatches += signal.keywordMatches;
        topicCounts.set(key, entry);
      }
    }
    const topics = [...topicCounts.values()]
      .map((item) => Object.freeze({
        ...item,
        papers: analyzed.length,
        historicalDetectionRate: item.appearances / analyzed.length
      }))
      .sort((left, right) =>
        right.historicalDetectionRate - left.historicalDetectionRate ||
        left.subject.localeCompare(right.subject) ||
        left.topic.localeCompare(right.topic)
      );
    return Object.freeze({
      examSeries,
      targetYear,
      eligiblePaperFiles: eligible.length,
      unreviewedPaperFiles: selected.length - eligible.length,
      analyzedPaperFiles: analyzed.length,
      years: Object.freeze(years),
      topics: Object.freeze(topics)
    });
  }

  return Object.freeze({
    questions,
    examConfig,
    examQuestions,
    parseExamCorpus,
    calculateCorpusOutlook,
    calculateTopicOutlook,
    parsePaperAnswerKey,
    scorePaperResponses,
    calculateKinematics,
    parsePaperHistory,
    restoreCompletedProgress,
    scoreExam,
    serializeCompletedProgress
  });
});
