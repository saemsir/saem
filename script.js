// Saem Physics — small site behaviours, no dependencies.
document.addEventListener('DOMContentLoaded', function () {
  const lectures = [
    { tag: 'JEE ADVANCED', title: 'JEE Advanced Physics Paper Solution', desc: 'Detailed solution and reasoning for advanced physics problems.', url: 'https://www.youtube.com/live/3TT1crLy9PQ?si=gyVhZIM5InkcNF4o' },
    { tag: 'JEE MAINS', title: 'JEE Mains Physics Paper Solution', desc: 'Work through the paper and understand each answer.', url: 'https://www.youtube.com/live/3IbsP-95MRE?si=BYujEr7TzCAeNRiB' },
    { tag: 'NEET', title: 'NEET Physics Paper Solution', desc: 'A clear, step-by-step paper discussion.', url: 'https://www.youtube.com/live/Ya75UNbuVaI?si=JN63seRXFpHVkw8d' },
    { tag: 'MECHANICS', title: "Newton's Law of Motion", desc: 'See how force, motion and mechanics fit together.', url: 'https://www.youtube.com/live/ARLQwOAdqd0?si=HOgtkaaQIqB0UaC2' },
    { tag: 'ROTATIONAL MOTION', title: 'Angular Momentum', desc: 'Explore rotational motion through intuition and equations.', url: 'https://youtu.be/iGLd6pqGu4w?si=UUK1ZToThHkY8fD_' },
    { tag: 'OPTICS', title: 'Refraction of Light', desc: 'Understand why light bends between materials.', url: 'https://youtu.be/dD5CMx5GeSM?si=uSyKlizkeR7II3kT' },
    { tag: 'CBSE CLASS 12', title: 'CBSE 12th Physics PYQ', desc: 'Practice previous-year questions concept-first.', url: 'https://www.youtube.com/live/FvA6a8dzL0o?si=KaM5WR5qZ9bzRz5C' },
    { tag: 'BIHAR BOARD', title: 'Bihar Board 12th PYQ', desc: 'Prepare with guided previous-year solutions.', url: 'https://www.youtube.com/live/3RmLjiBsL1Q?si=8xGlEimTatNyZGV0' },
    { tag: 'NCERT', title: 'NCERT Clear', desc: 'Make NCERT concepts clear and connected.', url: 'https://www.youtube.com/live/fpqUMPcvvmc?si=3dFFE43w-sMJs3x9' }
  ];

  const playlists = [
    { tag: 'JEE ADVANCE', title: 'JEE Advance Paper Solution', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGmJCQv5QKkfGWBBCLtzgQr1&si=I7vTInci3mQOs9N2' },
    { tag: 'NEET', title: 'NEET Exam Paper Solution', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGm7bdSxNp61GfEgAgpcb7c3&si=EcrB9UqsoHXF4uZG' },
    { tag: 'CLASS 12', title: '12th DEEP Physics', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGmz5Nu8Eo3tbW7C6TWxg47M&si=w7Tp_LEO3NaNfjpT' },
    { tag: 'CLASS 11', title: '11th Morning Physics', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGnJopwUTNwm4zB88Stdj8sc&si=HwBloKVuYDkOz8OG' },
    { tag: 'CLASS 11', title: '11th Deep Physics', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGkgKaiK8vYlJZRkklOYwrZ8&si=hmjmuwMQQu10lkGX' },
    { tag: 'DAILY LEARNING', title: 'Evening Physics', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGm5vX6_yZjSzkwPIv5BOr7d&si=ZVGB_tkm3_Lu_-vY' },
    { tag: 'REVISION', title: 'Quick Revision: System of Particles', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGldL1j2dvWCqR-9J_d1rQTv&si=5iBtkozfmJv2Pw53' },
    { tag: 'STRATEGY', title: "Topper's Strategy", url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGlbjHCLVfFG1LOmz6DhCpOB&si=YqUhwj7uVVRb-o9a' },
    { tag: 'JEE', title: 'Clear JEE Physics', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGnUR6SCSdM0oApUY7jldn1R&si=oW1Z8Fck_6wSgLFZ' },
    { tag: 'NEET', title: 'Clear NEET Physics', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGnoVXe4df8IAa1Ka-wvy9qr&si=jX-Sus6vzYBF-GnI' },
    { tag: 'CLASS 10', title: '10th Quick Revision', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGnrLTOGX75hlPRQE2mMDkUT&si=0ppHTD3-aq98uUrV' },
    { tag: 'CLASS 9', title: '9th Quick Revision', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGk-Y6nr-ldTnnCr8uRftGiC&si=9ygXAY1V3050SBG5' },
    { tag: 'CBSE 12', title: 'CBSE 12th Board exam', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGlHZ0f8U3R3_i1t9YFURQex&si=J-_FE2Sh3OdO0_aR' },
    { tag: 'BIHAR BOARD', title: 'Bihar Board Exam Guess', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGk0GPyRclNXoJYx7kQ5OQLV&si=aBSlHE8EMinmehJV' }
  ];

  const topics = [
    { tag: 'Mechanics', title: 'NCERT Questions: Laws of Motion', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGmj-GBjRI7WC_vzYn4-7Si5&si=hqmCPs2kr-OM5-mF', blurb: 'Motion, forces, and the deep logic behind everyday acceleration.' },
    { tag: 'Mathematical tools', title: 'NCERT Book: Units, Dimensions & Errors', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGk-_9QojNMbg6vhRrk1C0Yg&si=YWpAnwJcX90NDFev', blurb: 'The language of measurement, scaling, and physical reasoning.' },
    { tag: 'Problem-solving', title: 'Work Energy Power', url: 'https://www.youtube.com/live/94KWCWIpTWM?si=GXJzfDrZw8iYHCC8', blurb: 'Question-first problem solving with the right intuition.' },
    { tag: 'Quick revision', title: 'Electrostatics', url: 'https://youtu.be/OzuS2LwJwzI?si=531yUfZV0wWUEuSS', blurb: 'Electric fields, force, and central ideas in one clean pass.' },
    { tag: 'Quick revision', title: 'Gravitation', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGlDLayMTKe7kAf28RswrXAc&si=aWb3DhyKGrZKYCxZ', blurb: 'Gravity, orbits, and the structure of central forces.' },
    { tag: 'Modern physics', title: 'Modern Physics', url: 'https://youtube.com/playlist?list=PLjEkQ_dvlIGmYGbJoXdyKCa3eN6w0OrL0&si=FMKiyUA__LxuaSLx', blurb: 'The world as it looks beyond classical intuition.' }
  ];

  const utilityLinks = [
    ['Notes', 'https://t.me/SaemPhysics'],
    ['Admission', 'https://forms.gle/xGCHZt8vsd7rDf9k9'],
    ['Live class', 'https://meet170.webex.com/meet/saem']
  ];

  function buildCard(item, type) {
    const card = document.createElement('a');
    card.className = type === 'lecture' ? 'card lecture-card' : type === 'playlist' ? 'card playlist-card' : 'card topic-card';
    card.href = item.url;
    card.target = '_blank';
    card.rel = 'noopener';

    const meta = type === 'topic' ? item.tag : item.tag;
    card.innerHTML = `
      <small>${meta}</small>
      <h3>${item.title}</h3>
      ${type === 'lecture' ? `<p>${item.desc}</p>` : type === 'topic' ? `<p>${item.blurb}</p>` : '<p>One guided playlist for a complete learning arc.</p>'}
      <span>Open ↗</span>
    `;
    return card;
  }

  const lectureGrid = document.getElementById('lecture-grid');
  if (lectureGrid) {
    lectures.forEach((item) => lectureGrid.appendChild(buildCard(item, 'lecture')));
  }

  const playlistGrid = document.getElementById('playlist-grid');
  if (playlistGrid) {
    playlists.forEach((item) => playlistGrid.appendChild(buildCard(item, 'playlist')));
  }

  const topicGrid = document.getElementById('topic-grid');
  if (topicGrid) {
    topics.forEach((item) => topicGrid.appendChild(buildCard(item, 'topic')));
  }

  const utility = document.getElementById('utility-links');
  if (utility) {
    utilityLinks.forEach(([label, url]) => {
      const link = document.createElement('a');
      link.href = url;
      link.target = '_blank';
      link.rel = 'noopener';
      link.textContent = label;
      utility.appendChild(link);
    });
  }

  const toggle = document.getElementById('nav-toggle');
  const nav = document.getElementById('site-nav');
  if (toggle && nav) {
    toggle.addEventListener('click', function () {
      const isOpen = nav.classList.toggle('is-open');
      toggle.setAttribute('aria-expanded', String(isOpen));
    });

    nav.querySelectorAll('a').forEach(function (link) {
      link.addEventListener('click', function () {
        nav.classList.remove('is-open');
        toggle.setAttribute('aria-expanded', 'false');
      });
    });

    document.addEventListener('click', function (event) {
      const clickedInside = nav.contains(event.target) || toggle.contains(event.target);
      if (!clickedInside && nav.classList.contains('is-open')) {
        nav.classList.remove('is-open');
        toggle.setAttribute('aria-expanded', 'false');
      }
    });
  }

  const year = document.getElementById('year');
  if (year) {
    year.textContent = String(new Date().getFullYear());
  }

  const revealItems = document.querySelectorAll('.reveal');
  if ('IntersectionObserver' in window) {
    const observer = new IntersectionObserver((entries) => {
      entries.forEach((entry) => {
        if (entry.isIntersecting) {
          entry.target.classList.add('visible');
          observer.unobserve(entry.target);
        }
      });
    }, { threshold: 0.12 });

    revealItems.forEach((item) => observer.observe(item));
  } else {
    revealItems.forEach((item) => item.classList.add('visible'));
  }
});

