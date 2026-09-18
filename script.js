document.addEventListener('DOMContentLoaded', () => {
  const lectures = [
    ['JEE ADVANCED', 'JEE Advanced Physics Paper Solution', 'Detailed solution and reasoning for advanced physics problems.', 'https://www.youtube.com/live/3TT1crLy9PQ?si=gyVhZIM5InkcNF4o'],
    ['JEE MAINS', 'JEE Mains Physics Paper Solution', 'Work through the paper and understand each answer.', 'https://www.youtube.com/live/3IbsP-95MRE?si=BYujEr7TzCAeNRiB'],
    ['NEET', 'NEET Physics Paper Solution', 'A clear, step-by-step paper discussion.', 'https://www.youtube.com/live/Ya75UNbuVaI?si=JN63seRXFpHVkw8d'],
    ['MECHANICS', "Newton's Law of Motion", 'See how force, motion and mechanics fit together.', 'https://www.youtube.com/live/ARLQwOAdqd0?si=HOgtkaaQIqB0UaC2'],
    ['ROTATIONAL MOTION', 'Angular Momentum', 'Explore rotational motion through intuition and equations.', 'https://youtu.be/iGLd6pqGu4w?si=UUK1ZToThHkY8fD_'],
    ['OPTICS', 'Refraction of Light', 'Understand why light bends between materials.', 'https://youtu.be/dD5CMx5GeSM?si=uSyKlizkeR7II3kT'],
    ['CBSE CLASS 12', 'CBSE 12th Physics PYQ', 'Practice previous-year questions concept-first.', 'https://www.youtube.com/live/FvA6a8dzL0o?si=KaM5WR5qZ9bzRz5C'],
    ['BIHAR BOARD', 'Bihar Board 12th PYQ', 'Prepare with guided previous-year solutions.', 'https://www.youtube.com/live/3RmLjiBsL1Q?si=8xGlEimTatNyZGV0'],
    ['NCERT', 'NCERT Clear', 'Make NCERT concepts clear and connected.', 'https://www.youtube.com/live/fpqUMPcvvmc?si=3dFFE43w-sMJs3x9']
  ];

  const playlists = [
    ['JEE ADVANCE', 'JEE Advance Paper Solution', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGmJCQv5QKkfGWBBCLtzgQr1&si=I7vTInci3mQOs9N2'],
    ['NEET', 'NEET Exam Paper Solution', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGm7bdSxNp61GfEgAgpcb7c3&si=EcrB9UqsoHXF4uZG'],
    ['CLASS 12', '12th DEEP Physics', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGmz5Nu8Eo3tbW7C6TWxg47M&si=w7Tp_LEO3NaNfjpT'],
    ['CLASS 11', '11th Morning Physics', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGnJopwUTNwm4zB88Stdj8sc&si=HwBloKVuYDkOz8OG'],
    ['CLASS 11', '11th Deep Physics', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGkgKaiK8vYlJZRkklOYwrZ8&si=hmjmuwMQQu10lkGX'],
    ['DAILY LEARNING', 'Evening Physics', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGm5vX6_yZjSzkwPIv5BOr7d&si=ZVGB_tkm3_Lu_-vY'],
    ['REVISION', 'Quick Revision: System of Particles', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGldL1j2dvWCqR-9J_d1rQTv&si=5iBtkozfmJv2Pw53'],
    ['STRATEGY', "Topper's Strategy", 'https://youtube.com/playlist?list=PLjEkQ_dvlIGlbjHCLVfFG1LOmz6DhCpOB&si=YqUhwj7uVVRb-o9a'],
    ['JEE', 'Clear JEE Physics', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGnUR6SCSdM0oApUY7jldn1R&si=oW1Z8Fck_6wSgLFZ'],
    ['NEET', 'Clear NEET Physics', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGnoVXe4df8IAa1Ka-wvy9qr&si=jX-Sus6vzYBF-GnI'],
    ['CLASS 10', '10th Quick Revision', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGnrLTOGX75hlPRQE2mMDkUT&si=0ppHTD3-aq98uUrV'],
    ['CLASS 9', '9th Quick Revision', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGk-Y6nr-ldTnnCr8uRftGiC&si=9ygXAY1V3050SBG5'],
    ['CBSE 12', 'CBSE 12th Board exam', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGlHZ0f8U3R3_i1t9YFURQex&si=J-_FE2Sh3OdO0_aR'],
    ['BIHAR BOARD', 'Bihar Board Exam Guess', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGk0GPyRclNXoJYx7kQ5OQLV&si=aBSlHE8EMinmehJV']
  ];

  const makeCard = (item, playlist, index) => {
    const card = document.createElement('a');
    card.className = 'card';
    card.href = playlist ? item[2] : item[3];
    card.target = '_blank';
    card.rel = 'noopener';
    card.innerHTML = `<small>${item[0]} · ${String(index + 1).padStart(2, '0')}</small><h3>${item[1]}</h3><p>${playlist ? 'A focused Saem Physics learning path.' : item[2]}</p><b>${playlist ? 'Open playlist' : 'Watch on YouTube'} ↗</b>`;
    return card;
  };

  const lectureCards = document.getElementById('lecture-cards');
  const playlistCards = document.getElementById('playlist-cards');
  if (lectureCards) lectures.forEach((item, index) => lectureCards.appendChild(makeCard(item, false, index)));
  if (playlistCards) playlists.forEach((item, index) => playlistCards.appendChild(makeCard(item, true, index)));

  // Add the second featured lesson without disturbing the original layout.
  const feature = document.querySelector('#lectures .feature');
  if (feature) {
    feature.insertAdjacentHTML('afterend', `<div class="feature-secondary"><a class="feature-mini" href="https://youtu.be/sy70l6lQ7p8?si=jRQS-N0R-76PNis7" target="_blank" rel="noopener"><span class="mini-play">▶</span><span><small>MATHEMATICS FOR PHYSICS</small><strong>Quadratic Equations</strong></span><b>Watch ↗</b></a><p class="feature-note">Maths for Physics · Quadratic video — the algebra toolkit behind physics.</p></div>`);
  }

  // Keep useful resource links visible in the header in a compact utility row.
  const nav = document.getElementById('nav');
  if (nav) {
    const utility = document.createElement('div');
    utility.className = 'utility-links';
    utility.innerHTML = `<a href="https://t.me/SaemPhysics" target="_blank" rel="noopener">Notes</a><a href="https://forms.gle/xGCHZt8vsd7rDf9k9" target="_blank" rel="noopener">Admission</a><a href="https://meet170.webex.com/meet/saem" target="_blank" rel="noopener">Live class</a>`;
    nav.appendChild(utility);
  }

  // Point the library cards to the requested lectures and add three more paths.
  const topicLinks = [
    ['https://youtube.com/playlist?list=PLjEkQ_dvlIGmj-GBjRI7WC_vzYn4-7Si5&si=hqmCPs2kr-OM5-mF', 'Mechanics', 'NCERT Questions: Laws of Motion'],
    ['https://youtube.com/playlist?list=PLjEkQ_dvlIGk-_9QojNMbg6vhRrk1C0Yg&si=YWpAnwJcX90NDFev', 'Mathematical tools', 'NCERT Book: Units, Dimensions & Errors'],
    ['https://www.youtube.com/live/94KWCWIpTWM?si=GXJzfDrZw8iYHCC8', 'Problem-solving', 'Work, Energy & Power']
  ];
  document.querySelectorAll('#topics .topic').forEach((card, index) => {
    if (!topicLinks[index]) return;
    card.href = topicLinks[index][0];
    const title = card.querySelector('h3');
    const description = card.querySelector('p');
    if (title) title.textContent = topicLinks[index][1];
    if (description) description.textContent = topicLinks[index][2];
  });
  const topicGrid = document.querySelector('#topics .topic-grid');
  if (topicGrid) {
    [['QUICK REVISION', 'Electrostatics', 'https://youtu.be/OzuS2LwJwzI?si=531yUfZV0wWUEuSS'], ['QUICK REVISION', 'Gravitation', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGlDLayMTKe7kAf28RswrXAc&si=aWb3DhyKGrZKYCxZ'], ['MODERN PHYSICS', 'Modern Physics', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGmYGbJoXdyKCa3eN6w0OrL0&si=FMKiyUA__LxuaSLx']].forEach((item, index) => {
      const card = document.createElement('a');
      card.className = `topic ${['blue', 'gold', 'pink-card'][index]}`;
      card.href = item[2]; card.target = '_blank'; card.rel = 'noopener';
      card.innerHTML = `<span>0${index + 4}</span><h3>${item[1]}</h3><p>${item[0]} lesson and guided revision.</p><b>Open lecture ↗</b>`;
      topicGrid.appendChild(card);
    });
  }

  const year = document.getElementById('year');
  if (year) year.textContent = new Date().getFullYear();

  const menu = document.getElementById('menu');
  if (menu && nav) {
    menu.addEventListener('click', () => {
      const open = nav.classList.toggle('open');
      menu.setAttribute('aria-expanded', String(open));
    });
    nav.querySelectorAll('a').forEach(link => link.addEventListener('click', () => {
      nav.classList.remove('open');
      menu.setAttribute('aria-expanded', 'false');
    }));
  }

  const revealItems = document.querySelectorAll('.reveal');
  if ('IntersectionObserver' in window) {
    const observer = new IntersectionObserver(entries => entries.forEach(entry => {
      if (entry.isIntersecting) { entry.target.classList.add('visible'); observer.unobserve(entry.target); }
    }), { threshold: 0.1 });
    revealItems.forEach(item => observer.observe(item));
  } else revealItems.forEach(item => item.classList.add('visible'));

  const canvas = document.getElementById('space');
  if (canvas && !window.matchMedia('(prefers-reduced-motion: reduce)').matches) {
    const ctx = canvas.getContext('2d'); let particles = [];
    const resize = () => {
      canvas.width = innerWidth; canvas.height = innerHeight;
      particles = Array.from({ length: Math.min(70, Math.floor(innerWidth / 18)) }, () => ({ x: Math.random() * canvas.width, y: Math.random() * canvas.height, vx: (Math.random() - .5) * .35, vy: (Math.random() - .5) * .35, r: Math.random() * 1.5 + .4 }));
    };
    const draw = () => {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      particles.forEach((p, i) => {
        p.x = (p.x + p.vx + canvas.width) % canvas.width; p.y = (p.y + p.vy + canvas.height) % canvas.height;
        ctx.fillStyle = 'rgba(104,217,243,.4)'; ctx.beginPath(); ctx.arc(p.x, p.y, p.r, 0, Math.PI * 2); ctx.fill();
        for (let j = i + 1; j < particles.length; j++) { const q = particles[j], d = Math.hypot(p.x - q.x, p.y - q.y); if (d < 115) { ctx.strokeStyle = `rgba(104,217,243,${.1 * (1 - d / 115)})`; ctx.beginPath(); ctx.moveTo(p.x, p.y); ctx.lineTo(q.x, q.y); ctx.stroke(); } }
      });
      requestAnimationFrame(draw);
    };
    addEventListener('resize', resize); resize(); draw();
  }

  const style = document.createElement('style');
  style.textContent = `.utility-links{display:flex;align-items:center;gap:.55rem;margin-left:.35rem;padding-left:.7rem;border-left:1px solid var(--line)}.utility-links a{color:var(--muted);font-size:.68rem;letter-spacing:.04em;white-space:nowrap}.utility-links a:hover{color:var(--yellow)}.feature-secondary{margin-top:1rem;padding:1rem 1.2rem;border:1px solid var(--line);border-left:3px solid var(--yellow);border-radius:14px;background:rgba(29,51,80,.55)}.feature-mini{display:flex;align-items:center;gap:.9rem}.feature-mini small{display:block;color:var(--blue);font-size:.68rem;font-weight:800;letter-spacing:.1em}.feature-mini strong{display:block;font:1.3rem var(--serif);margin-top:.15rem}.feature-mini b{margin-left:auto;color:var(--yellow);font-size:.78rem;white-space:nowrap}.mini-play{display:grid;place-items:center;width:38px;height:38px;border:1px solid var(--blue);border-radius:50%;color:var(--blue);font-size:.8rem}.feature-note{color:var(--muted);font-size:.8rem;margin:.65rem 0 0 3rem}.topic-grid .topic:nth-child(n+4){margin-top:1rem}@media(max-width:720px){.utility-links{margin:.5rem 0 0;padding:.6rem 0 0;border-left:0;border-top:1px solid var(--line);justify-content:space-between}.feature-mini{align-items:flex-start}.feature-mini b{margin-left:auto}.feature-note{margin-left:0}.topic-grid .topic:nth-child(n+4){margin-top:0}}`;
  document.head.appendChild(style);
});
