// Saem Physics — small site behaviours, no dependencies.

document.addEventListener('DOMContentLoaded', function () {

  // Mobile nav toggle
  var toggle = document.getElementById('nav-toggle');
  var nav = document.getElementById('site-nav');

  if (toggle && nav) {
    toggle.addEventListener('click', function () {
      var isOpen = nav.classList.toggle('is-open');
      toggle.setAttribute('aria-expanded', String(isOpen));
    });

    // Close the menu after a link is chosen, so it doesn't stay
    // open over the section the person just jumped to.
    nav.querySelectorAll('a').forEach(function (link) {
      link.addEventListener('click', function () {
        nav.classList.remove('is-open');
        toggle.setAttribute('aria-expanded', 'false');
      });
    });

    // Close the menu on outside click.
    document.addEventListener('click', function (event) {
      var clickedInsideNav = nav.contains(event.target) || toggle.contains(event.target);
      if (!clickedInsideNav && nav.classList.contains('is-open')) {
        nav.classList.remove('is-open');
        toggle.setAttribute('aria-expanded', 'false');
      }
    });
  }

  // Keep the footer year correct without editing it by hand every January.
  var yearEl = document.getElementById('year');
  if (yearEl) {
    yearEl.textContent = String(new Date().getFullYear());
  }

});
