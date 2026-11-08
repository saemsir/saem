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

  return Object.freeze({
    questions,
    calculateKinematics,
    restoreCompletedProgress,
    serializeCompletedProgress
  });
});
