const assert = require("node:assert/strict");
const {
  calculateKinematics,
  questions,
  restoreCompletedProgress,
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
console.log("Student studio: kinematics and 3 guided questions pass");
