"use strict";

const form = document.getElementById("calculation-form");
const kernelStatus = document.getElementById("kernel-status");
const result = document.getElementById("result");
const calculationStatus = document.getElementById("calculation-status");
const submitButton = form.querySelector("button[type='submit']");

async function checkKernel() {
  try {
    const response = await fetch("/api/v1/health", { headers: { Accept: "application/json" } });
    const payload = await response.json();
    if (!response.ok || payload.status !== "ok" || payload.apiVersion !== 1) {
      throw new Error("The local kernel API did not report a ready v1 service.");
    }
    kernelStatus.textContent = "C17 kernel connected";
    kernelStatus.dataset.state = "ready";
  } catch (error) {
    kernelStatus.textContent = "Kernel connection unavailable";
    kernelStatus.dataset.state = "error";
    calculationStatus.textContent = error.message;
    submitButton.disabled = true;
  }
}

form.addEventListener("submit", async (event) => {
  event.preventDefault();
  result.textContent = "…";
  calculationStatus.textContent = "Checking the exact expression…";
  submitButton.disabled = true;
  try {
    const response = await fetch("/api/v1/kernel/rational", {
      method: "POST",
      headers: { "Content-Type": "application/json", Accept: "application/json" },
      body: JSON.stringify({
        operation: document.getElementById("operation").value,
        left: document.getElementById("left").value.trim(),
        right: document.getElementById("right").value.trim()
      })
    });
    const payload = await response.json();
    if (!response.ok) {
      throw new Error(payload.message || `Kernel request failed (${response.status}).`);
    }
    result.textContent = payload.result;
    calculationStatus.textContent = "Returned by the C17 exact-rational kernel.";
  } catch (error) {
    result.textContent = "No exact result";
    calculationStatus.textContent = error.message;
  } finally {
    submitButton.disabled = false;
  }
});

checkKernel();
